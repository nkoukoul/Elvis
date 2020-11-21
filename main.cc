#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tcp_server.h"
#include "json_utils.h"


int main()
{
  json_util_context juc = json_util_context();
  tcp_server * ts = new tcp_server("127.0.0.1", 8589, std::move(juc));
  std::cout << "server accepting connections on 127.0.0.1:8589\n";
  ts->accept_connections();
  return 0;
}
