#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tcp_server.h"



int main()
{
  tcp_server * ts = new tcp_server("127.0.0.1", 8589);
  std::cout << "server accepting connections on 127.0.0.1:8589\n";
  ts->accept_connections();
  return 0;
}
