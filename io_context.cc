//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
//
// repository: https://github.com/nkoukoul/Elvis
//

#include <iostream>
#include <sstream>
#include <bitset>
#include <stdio.h>
#include <string.h>
#include <csignal>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "io_context.h"
#include "app_context.h"

int socket_timeout = 3000;

void non_block_socket(int sd)
{
  /* set O_NONBLOCK on fd */
  int flags = fcntl(sd, F_GETFL, 0);
  if (flags == -1)
  {
    //perror("fcntl()");
    return;
  }
  if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1)
  {
    //perror("fcntl()");
  }
}

void io_context::run(app * ac)
{
  while (true)
  {
    auto async_func = ac->e_q_->consume_event<std::function<void()>>();
    try
    {
      async_func();
    }
    catch (const std::bad_function_call &e)
    {
      ;
    }
  }
}

tcp_handler::tcp_handler(std::string ipaddr, int port, std::unique_ptr<i_request_context> req, std::unique_ptr<i_response_context> res, app *ac) : ipaddr_(ipaddr), port_(port), req_(std::move(req)), res_(std::move(res)), ac_(ac)
{
  struct sockaddr_in server;

  server_sock_ = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
  //non_block_socket(server_sock_);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ipaddr_.c_str());
  server.sin_port = htons(port_);

  if (bind(server_sock_, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    std::cout << "bind failed\n";
    exit(1);
  }

  listen(server_sock_, 5);
}

void tcp_handler::handle_connections()
{
  struct sockaddr_in client;
  socklen_t client_len;

  client_len = sizeof client;
  int client_socket = accept(server_sock_, (struct sockaddr *)&client, &client_len);
  if (client_socket < 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      ;;//no incoming connection for non-blocking sockets
    }
    else
    {
      std::cout << "Error while accepting connection";
    }
  } else {
    ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&io_context::do_read, ac_->http_ioc_.get(), client_socket)));
  }
  return ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&tcp_handler::handle_connections, ac_->http_ioc_.get())));
}

void tcp_handler::do_read(int const client_socket)
{
  std::string input_data;
  char inbuffer[MAXBUF], *p = inbuffer;

  // Read data from client
  int bytes_read = read(client_socket, inbuffer, MAXBUF);
  if (bytes_read <= 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      //client block
      ;;
    } else {
      //client closed connection
      close(client_socket);
      return;
    }
  }

  for (int i = 0; i < bytes_read; i++)
  {
    input_data += inbuffer[i];
  }
  input_data += '\n'; //add end of line for getline
  return ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&i_request_context::do_parse, ac_->http_ioc_->req_.get(), client_socket, std::move(input_data))));
}

void tcp_handler::do_write(int const client_socket, std::string output_data, bool close_connection)
{
  write(client_socket, output_data.c_str(), output_data.size());
  //close socket if http
  if (close_connection)
  {
    close(client_socket);
  }
  else
  {
    //websocket connection for now
    ac_->ws_ioc_->register_socket(client_socket);
  }
  return;
}

websocket_handler::websocket_handler(std::string ipaddr, int port, std::unique_ptr<i_request_context> req, std::unique_ptr<i_response_context> res, app *ac) : ipaddr_(ipaddr), port_(port), req_(std::move(req)), res_(std::move(res)), ac_(ac)
{
  std::signal(SIGPIPE, SIG_IGN);
  broadcast_fd_list.resize(256, {0, ""});
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1)
  {
    //perror("epoll_create1()");
    //exit;
  }
  memset(&event, 0, sizeof(event));
  events = (struct epoll_event *)calloc(MAXEVENTS, sizeof(struct epoll_event));
}

void websocket_handler::register_socket(int const client_socket)
{
  std::lock_guard<std::mutex> guard(socket_state_mutex_);
  if (client_socket > 255)
    return;
  broadcast_fd_list[client_socket].first = client_socket;
  non_block_socket(client_socket);
  event.data.fd = client_socket;
  event.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1)
  {
    //perror("epoll_ctl()");
    //return 1;
  }
  return;
}

void websocket_handler::handle_connections()
{
  std::lock_guard<std::mutex> guard(socket_state_mutex_);
  int nevents = epoll_wait(epoll_fd, events, MAXEVENTS, 10);
  if (nevents == -1) {
    //perror("epoll_wait()");
    //return 1;
  }
  for (int i = 0; i < nevents; i++)
  {
    if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
    {
      // error case
      fprintf(stderr, "epoll error\n");
      close(events[i].data.fd);
      broadcast_fd_list[events[i].data.fd].first = 0;
      continue;
    }
    else if (events[i].events & EPOLLIN)
    {
      int client_socket = events[i].data.fd;
      broadcast_fd_list[client_socket].second.clear();
      ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&websocket_handler::do_read, ac_->ws_ioc_.get(), client_socket)));
    }
  }
  return ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&websocket_handler::handle_connections, ac_->ws_ioc_.get())));
}

void websocket_handler::do_read(int const client_socket)
{
  char inbuffer[MAXBUF];
  std::string input_websocket_frame_in_bits;
  // Read data from client
  int bytes_read = read(client_socket, inbuffer, MAXBUF);
  if (bytes_read <= 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      //read block so finished reading data from client
      input_websocket_frame_in_bits = broadcast_fd_list[client_socket].second;
      return ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&i_request_context::do_parse, ac_->ws_ioc_->req_.get(), client_socket, std::move(input_websocket_frame_in_bits))));
    }
    else
    {
      //perror("read()");
      std::cout << "ws client disconnected or error\n";
      close(client_socket);
      broadcast_fd_list[client_socket].first = 0;
      return;
    }
  }
  else
  {
    for (int i = 0; i < bytes_read; i++)
    {
      std::bitset<8> bb(inbuffer[i]);
      broadcast_fd_list[client_socket].second += bb.to_string();
    }
  }
  //if the call hasnt block more data is available add a read to the queue
  return ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&websocket_handler::do_read, ac_->ws_ioc_.get(), client_socket)));
}

void websocket_handler::do_write(int const client_socket, std::string output_data, bool close_connection)
{
  if (write(client_socket, output_data.c_str(), output_data.size()) < 0 || close_connection)
  {
    std::cout << "error during write\n";
    close(client_socket);
    broadcast_fd_list[client_socket].first = 0;
  }
  return;
}
