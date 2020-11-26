//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//


#include <memory>
#include <thread>
#include <string>
#include "app_context.h"

int main()
{
  int thread_number = 5;
  int port = 8589;
  std::string ipaddr = "127.0.0.1";

  std::unique_ptr<route_manager> rm = std::make_unique<route_manager>();
  rm->set_route("/file", "GET");
  rm->set_route("/file", "POST");
  app * my_app = app::get_instance(
				   std::move(std::make_unique<tcp_server>(ipaddr, port)), 
				   std::move(std::make_unique<json_util_context>()), 
				   std::move(rm),
				   std::move(std::make_unique<http_request_context>()),
				   std::move(std::make_unique<http_response_context>()));
  
  std::cout << "server accepting connections on " << ipaddr << ":" << port << "\n";

  my_app->run(thread_number);
  
  
  return 0;
}
