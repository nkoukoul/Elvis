#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <vector>
#include <thread>
#include <memory>
#include "request_context.h"
#include "route_manager.h"
#include "json_utils.h"
#include "models.h"
#include "response_context.h"

class tcp_server{
public:
  tcp_server(std::string ipaddr, int port, 
	       std::shared_ptr<i_json_util_context> juc = nullptr, 
	       std::shared_ptr<route_manager> rm = nullptr,
	       std::shared_ptr<i_request_context> rc = nullptr);
  
  void set_json_util_context(std::shared_ptr<i_json_util_context> juc);
  void set_route_manager(std::shared_ptr<route_manager> rm);
  void set_request_context(std::shared_ptr<i_request_context> rc);
  void accept_connections();
private:
  void quit();
  int handle_request(int && client_socket);
  std::vector<std::thread> client_threads_;
  std::vector<int> available_sockets_;
  std::string ipaddr_;
  int port_;
  int server_sock_;
  std::shared_ptr<i_json_util_context> juc_;
  std::shared_ptr<route_manager> rm_;
  std::shared_ptr<i_request_context> rc_;
  static const int MAXBUF = 1024;
};


#endif // TCP_SERVER_H
