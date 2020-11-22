#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <vector>
#include <thread>
#include "route_manager.h"
#include "json_utils.h"

class tcp_server{
public:
  tcp_server(std::string ipaddr, int port, i_json_util_context * juc = nullptr, route_manager * rm = nullptr);
  ~tcp_server();
  void set_json_util_context(i_json_util_context * juc);
  void set_route_manager(route_manager * rm);
  void accept_connections();
private:
  void quit();
  int handle_request(int && client_socket);
  std::vector<std::thread> client_threads_;
  std::string ipaddr_;
  int port_;
  int server_sock_;
  i_json_util_context * juc_;
  route_manager * rm_;
  static const int MAXBUF = 1024;
};


#endif // TCP_SERVER_H
