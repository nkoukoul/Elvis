#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <vector>
#include <thread>

class tcp_server{
public:
  tcp_server(std::string ipaddr, int port);
  void accept_connections();
private:
  void quit();
  static int handle_request(int && client_socket);
  std::vector<std::thread> client_threads_;
  std::string ipaddr_;
  int port_;
  int server_sock_;
  static const int MAXBUF = 1024;
};


#endif // TCP_SERVER_H
