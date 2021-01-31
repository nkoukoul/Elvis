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

void io_context::run(app *ac, std::shared_ptr<i_event_queue> executor)
{
  executor->produce_event<std::function<void()>>(
      std::move(
          std::bind(
              &tcp_handler::handle_connections,
              ac->ioc_.get(),
              executor)));

  while (true)
  {
    if (!executor->empty())
    {
      auto async_func = executor->consume_event<std::function<void()>>();
      async_func();
    }
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

void tcp_handler::handle_connections(std::shared_ptr<i_event_queue> executor)
{
  struct sockaddr_in client;
  socklen_t client_len;

  client_len = sizeof client;
  int client_socket = accept(server_sock_, (struct sockaddr *)&client, &client_len);
  if (client_socket < 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      ;
      ; //no incoming connection for non-blocking sockets
    }
    else
    {
      std::cout << "Error while accepting connection";
    }
  }
  else
  {
    non_block_socket(client_socket);
    std::shared_ptr<client_context> c_ctx = std::make_shared<client_context>();
    c_ctx->client_socket_ = client_socket;
    executor->produce_event<std::function<void()>>(
        std::move(
            std::bind(
                &io_context::do_read,
                ac_->ioc_.get(),
                c_ctx,
                executor)));
  }
  executor->produce_event<std::function<void()>>(
      std::move(
          std::bind(
              &tcp_handler::handle_connections,
              ac_->ioc_.get(),
              executor)));
}

void tcp_handler::do_read(std::shared_ptr<client_context> c_ctx, std::shared_ptr<i_event_queue> executor)
{
  char inbuffer[MAXBUF], *p = inbuffer;

  // Read data from client
  int bytes_read = read(c_ctx->client_socket_, inbuffer, MAXBUF);
  if (bytes_read <= 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      //client read block try again
      executor->produce_event<std::function<void()>>(
          std::move(
              std::bind(
                  &io_context::do_read,
                  ac_->ioc_.get(),
                  c_ctx,
                  executor)));
      return;
    }
    else
    {
      //client closed connection
      close(c_ctx->client_socket_);
      return;
    }
  }
  else
  {
    if (c_ctx->is_websocket_)
    {
      for (int i = 0; i < bytes_read; i++)
      {
        std::bitset<8> bb(inbuffer[i]);
        c_ctx->websocket_message_ += bb.to_string();
      }
      executor->produce_event<std::function<void()>>(
          std::move(
              std::bind(
                  &i_request_context::do_parse,
                  ac_->ws_req_.get(),
                  c_ctx,
                  executor)));
    }
    else
    {
      for (int i = 0; i < bytes_read; i++)
      {
        c_ctx->http_message_ += inbuffer[i];
      }
      c_ctx->http_message_ += '\n'; //add end of line for getline
      executor->produce_event<std::function<void()>>(
          std::move(
              std::bind(
                  &i_request_context::do_parse,
                  ac_->http_req_.get(),
                  c_ctx,
                  executor)));
    }
  }
}

void tcp_handler::do_write(
    std::shared_ptr<client_context> c_ctx,
    std::shared_ptr<i_event_queue> executor)
{
  int status;
  if (c_ctx->is_websocket_ && c_ctx->handshake_completed_)
  {
    status = write(c_ctx->client_socket_, c_ctx->websocket_response_.c_str(), c_ctx->websocket_response_.size());
  }
  else
  {
    status = write(c_ctx->client_socket_, c_ctx->http_response_.c_str(), c_ctx->http_response_.size());
    if (c_ctx->is_websocket_)
    {
      c_ctx->handshake_completed_ = true;
    }    
  }
  
  //close socket if http
  if (status < 0 || c_ctx->close_connection_)
  {//to check eagain ewblock for nonblock sockets
    close(c_ctx->client_socket_);
  }
  else
  {
    if(c_ctx->is_websocket_)
    {
      c_ctx->websocket_message_.clear();
      c_ctx->websocket_data_.clear();
      c_ctx->websocket_response_.clear();
      executor->produce_event<std::function<void()>>(
          std::move(
              std::bind(
                  &tcp_handler::do_read,
                  ac_->ioc_.get(),
                  c_ctx,
                  executor)));
    }
  }
  return;
}
