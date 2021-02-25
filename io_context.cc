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

tcp_handler::tcp_handler(
    std::string ipaddr,
    int port,
    app *ac) : ipaddr_(ipaddr), port_(port), ac_(ac)
{
  struct sockaddr_in server;

  server_sock_ = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
  non_block_socket(server_sock_);
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

void tcp_handler::run()
{
  auto executor = ac_->sm_->access_strand<event_queue<std::function<void()>>>();
  executor->produce_event(
      std::move(
          std::bind(
              &tcp_handler::handle_connections,
              ac_->ioc_.get())));

  while (true)
  {
    if (!executor->empty())
    {
      auto async_func = executor->consume_event();
      async_func();
    }
  }
}

void tcp_handler::handle_connections()
{
  struct sockaddr_in client;
  socklen_t client_len;
  auto executor = ac_->sm_->access_strand<event_queue<std::function<void()>>>();
  client_len = sizeof client;
  int client_socket = accept(server_sock_, (struct sockaddr *)&client, &client_len);
  if (client_socket <= 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      ;
      ; //no incoming connection for non-blocking sockets
    }
    else
    {
      std::cout << "Error while accepting connection\n";
    }
  }
  else
  {
    non_block_socket(client_socket);
    std::shared_ptr<client_context> c_ctx = std::make_shared<client_context>();
    c_ctx->client_socket_ = client_socket;
    executor->produce_event(
        std::move(
            std::bind(
                &io_context::do_read,
                ac_->ioc_.get(),
                c_ctx)));
  }

  executor->produce_event(
      std::move(
          std::bind(
              &tcp_handler::handle_connections,
              ac_->ioc_.get())));
}

void tcp_handler::do_read(std::shared_ptr<client_context> c_ctx)
{
  auto executor = ac_->sm_->access_strand<event_queue<std::function<void()>>>();
  char inbuffer[MAXBUF], *p = inbuffer;
  // Read data from client
  int bytes_read = read(c_ctx->client_socket_, inbuffer, MAXBUF);
  // Client closed connection
  if (bytes_read == 0)
  {
    close(c_ctx->client_socket_);
    return;
  }
  if (bytes_read < 0)
  {
    // Client read block
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      //if is websocket and has data proceed to parsing
      if (c_ctx->is_websocket_ && c_ctx->websocket_message_.size())
      {
        executor->produce_event(
            std::move(
                std::bind(
                    &i_request_context::do_parse,
                    ac_->ws_req_.get(),
                    c_ctx)));
      } //if is http and has data proceed to parsing
      else if (!c_ctx->is_websocket_ && c_ctx->http_message_.size())
      {
        c_ctx->http_message_ += '\n'; //add end of line for getline
        executor->produce_event(
            std::move(
                std::bind(
                    &i_request_context::do_parse,
                    ac_->http_req_.get(),
                    c_ctx)));
      } // read simply blocked with no data try again
      else
      {
        executor->produce_event(
            std::move(
                std::bind(
                    &io_context::do_read,
                    ac_->ioc_.get(),
                    c_ctx)));
      }
      return;
    }
    else
    {
      // TCP read error
      close(c_ctx->client_socket_);
      return;
    }
  }
  else // We have data to read
  {
    if (c_ctx->is_websocket_)
    {
      for (int i = 0; i < bytes_read; i++)
      {
        c_ctx->websocket_message_ += inbuffer[i];
      }
    }
    else
    {
      for (int i = 0; i < bytes_read; i++)
      {
        c_ctx->http_message_ += inbuffer[i];
      }
    }
    // maybe there some more data so add another read to the queue
    executor->produce_event(
        std::move(
            std::bind(
                &io_context::do_read,
                ac_->ioc_.get(),
                c_ctx)));
  }
}

void tcp_handler::do_write(std::shared_ptr<client_context> c_ctx)
{
  auto executor = ac_->sm_->access_strand<event_queue<std::function<void()>>>();
  int bytes_write;
  if (c_ctx->is_websocket_ && c_ctx->handshake_completed_)
  {
    size_t bytes_to_send;
    if (MAXBUF > (c_ctx->websocket_response_.size() - c_ctx->websocket_bytes_send_))
    {
      bytes_to_send = c_ctx->websocket_response_.size() - c_ctx->websocket_bytes_send_;
    }
    else
    {
      bytes_to_send = MAXBUF;
    }
    bytes_write = write(c_ctx->client_socket_, c_ctx->websocket_response_.c_str() + c_ctx->websocket_bytes_send_, bytes_to_send);
  }
  else
  {
    size_t bytes_to_send;
    if (MAXBUF > (c_ctx->http_response_.size() - c_ctx->http_bytes_send_))
    {
      bytes_to_send = c_ctx->http_response_.size() - c_ctx->http_bytes_send_;
    }
    else
    {
      bytes_to_send = MAXBUF;
    }
    
    bytes_write = write(c_ctx->client_socket_, c_ctx->http_response_.c_str() + c_ctx->http_bytes_send_, bytes_to_send);
  }
  
  // Client closed connection
  if (bytes_write == 0)
  {
    close(c_ctx->client_socket_);
    return;
  }
  
  // Error handling
  if (bytes_write < 0)
  { // Write block try again
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      executor->produce_event(
          std::move(
              std::bind(
                  &io_context::do_write,
                  ac_->ioc_.get(),
                  c_ctx)));
    }
    else
    {
      // TCP write error
      close(c_ctx->client_socket_);
    }
    return;
  }

  // Check if we need to transfer more data
  if (c_ctx->is_websocket_ && c_ctx->handshake_completed_)
  {
    c_ctx->websocket_bytes_send_ += bytes_write;
  }
  else
  {
    c_ctx->http_bytes_send_ += bytes_write;
  }
  
  if ((!c_ctx->is_websocket_ && c_ctx->http_bytes_send_ < c_ctx->http_response_.size()) ||
      (c_ctx->is_websocket_ && c_ctx->handshake_completed_ && c_ctx->websocket_bytes_send_ < c_ctx->websocket_response_.size()))
  {
    executor->produce_event(
        std::move(
            std::bind(
                &io_context::do_write,
                ac_->ioc_.get(),
                c_ctx)));
    return;
  }

  // Check if socket should close
  if (c_ctx->close_connection_)
  {
    close(c_ctx->client_socket_);
    return;
  }

  // if socket is still open with http keep-alive or ws read again
  if (c_ctx->is_websocket_)
  {
    if (!c_ctx->handshake_completed_)
    {
      ac_->broadcast_fd_list.push_back(c_ctx->client_socket_);
      c_ctx->handshake_completed_ = true;
    }
    c_ctx->websocket_message_.clear();
    c_ctx->websocket_data_.clear();
    c_ctx->websocket_response_.clear();
    c_ctx->websocket_bytes_send_ = 0;
  }
  else
  {
    c_ctx->http_message_.clear();
  }
  

  executor->produce_event(
      std::move(
          std::bind(
              &tcp_handler::do_read,
              ac_->ioc_.get(),
              c_ctx)));
  return;
}
