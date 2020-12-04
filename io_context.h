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

class app;

class io_context{
public:
  io_context() = default;
  virtual void run(app * ac) = 0;
  virtual std::string do_read(int client_socket, app * ac) = 0;
  virtual void do_write(int client_socket, app * ac, std::string && output_data) = 0;
};

class tcp_server: public io_context{
public:
  tcp_server(std::string ipaddr, int port);  
  void run(app * ac) override;
  std::string do_read(int client_socket, app * ac) override;
  void do_write(int client_socket, app * ac, std::string && output_data) override;
private:
  void accept_connections(app * ac);
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
};

//here will be used with the above tcp server so no need for listening to sockets
class websocket_server: public io_context{
public:
  websocket_server(std::string ipaddr, int port);  
  void run(app * ac) override;
  std::string do_read(int client_socket, app * ac) override;
  void do_write(int client_socket, app * ac, std::string && output_data) override;
  void register_socket(int client_socket, std::unordered_map<std::string, std::string>  && deserialized_input_data);
private:
  std::mutex socket_state_mutex_;
  void handle_connections(app * ac);
  std::unordered_map<int, std::unordered_map<std::string, std::string>> socket_state_;
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
};


#endif // IO_CONTEXT_H
