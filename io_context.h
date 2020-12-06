//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
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
#include <sys/epoll.h>
#include <fcntl.h>
#include "request_context.h"
#include "response_context.h"

class app;

class io_context{
public:
  io_context() = default;
  virtual void run() = 0;
  virtual void do_read(int const client_socket) = 0;
  virtual void do_write(int const client_socket, std::string && output_data, bool close_connection) = 0;
};

class tcp_server: public io_context{
public:
  tcp_server(std::string ipaddr, int port, std::unique_ptr<i_request_context> req, std::unique_ptr<i_response_context> res, app * ac);  
  void run() override;
  void do_read(int const client_socket) override;
  void do_write(int const client_socket, std::string && output_data, bool close_connection) override;
  
  std::unique_ptr<i_request_context> req_;
  std::unique_ptr<i_response_context> res_;
private:
  app * ac_;
  void handle_connections();
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
};

//here will be used with the above tcp server so no need for listening to sockets
class websocket_server: public io_context{
public:
  websocket_server(std::string ipaddr, int port, std::unique_ptr<i_request_context> req, std::unique_ptr<i_response_context> res, app * ac);  
  void run() override;
  void do_read(int const client_socket) override;
  void do_write(int const client_socket, std::string && output_data, bool close_connection) override;
  void register_socket(int const client_socket);

  std::unique_ptr<i_request_context> req_;
  std::unique_ptr<i_response_context> res_;
private:
  app * ac_;
  std::mutex socket_state_mutex_;
  void handle_connections();
  //std::unordered_map<int, std::string> socket_state_;
  int epoll_fd;
  struct epoll_event event;
  struct epoll_event * events;
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
  static const int MAXEVENTS = 64;
};


#endif // IO_CONTEXT_H
