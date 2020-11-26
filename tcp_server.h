//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
// 
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <vector>
#include <thread>
#include <memory>

class app;

class tcp_server{
public:
  tcp_server(std::string ipaddr, int port);  
  void accept_connections(app * ac);
private:
  int handle_request(int && client_socket, app * ac);
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
};


#endif // TCP_SERVER_H
