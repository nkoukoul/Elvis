//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include <string>
#include <vector>
#include <thread>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <fcntl.h>
#include "request_context.h"
#include "response_context.h"
#include "event_queue.h"

class app;

class io_context
{
public:
  io_context() = default;

  void run(app * ac, std::shared_ptr<i_event_queue> executor);

  virtual void handle_connections(std::shared_ptr<i_event_queue> executor) = 0;

  virtual void do_read(int const client_socket, std::shared_ptr<i_event_queue> executor) = 0;

  virtual void do_write(
      int const client_socket,
      std::string output_data,
      bool close_connection,
      std::shared_ptr<i_event_queue> executor) = 0;
};

class tcp_handler : public io_context
{
public:
  tcp_handler(std::string ipaddr, int port, std::unique_ptr<i_request_context> req, std::unique_ptr<i_response_context> res, app *ac);

  void handle_connections(std::shared_ptr<i_event_queue> executor) override;

  void do_read(int const client_socket, std::shared_ptr<i_event_queue> executor) override;

  void do_write(
      int const client_socket,
      std::string output_data,
      bool close_connection, std::shared_ptr<i_event_queue> executor) override;

  app *ac_;
  std::unique_ptr<i_request_context> req_;
  std::unique_ptr<i_response_context> res_;

private:
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
};

//here will be used with the above tcp server so no need for listening to sockets
class websocket_handler : public io_context
{
public:
  websocket_handler(
      std::string ipaddr,
      int port, std::unique_ptr<i_request_context> req,
      std::unique_ptr<i_response_context> res,
      app *ac);

  void handle_connections(std::shared_ptr<i_event_queue> executor) override;

  void do_read(int const client_socket, std::shared_ptr<i_event_queue> executor) override;

  void do_write(
      int const client_socket,
      std::string output_data,
      bool close_connection,
      std::shared_ptr<i_event_queue> executor) override;

  void register_socket(int const client_socket, std::shared_ptr<i_event_queue> executor);

  app *ac_;
  std::unique_ptr<i_request_context> req_;
  std::unique_ptr<i_response_context> res_;

private:
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
};

#endif // IO_CONTEXT_H
