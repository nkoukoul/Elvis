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
#include <memory>

class app;

class io_context{
public:
  io_context() = default;
  virtual void run(app * ac) = 0;
private:
  virtual std::string do_read(int client_socket, app * ac) = 0;
  virtual void do_write(int client_socket, app * ac, std::string && input_data) = 0;
};

class tcp_server: public io_context{
public:
  tcp_server(std::string ipaddr, int port);  
  void run(app * ac) override;
private:
  void accept_connections(app * ac);
  std::string do_read(int client_socket, app * ac);
  void do_write(int client_socket, app * ac, std::string && input_data);
//int handle_request(int && client_socket, app * ac);
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
};


#endif // IO_CONTEXT_H
