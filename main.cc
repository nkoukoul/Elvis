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
#include "controllers.h"

int main(int argc, char * argv[])
{
  if (argc != 4){
    std::cout << "Usage: app hostname port thread_number\n";
    return -1;
  }

  std::string ipaddr = argv[1];
  int port = std::stoi(argv[2]);
  int thread_number = std::max<int>(1,std::stoi(argv[3]));
  app * my_app = app::get_instance();
  
  std::unique_ptr<websocket_server> ws = std::make_unique<websocket_server>
    (ipaddr, port, std::move(std::make_unique<websocket_request_context>(my_app)), std::move(std::make_unique<websocket_response_context>(my_app)), my_app);
  

  std::unique_ptr<route_manager> rm = std::make_unique<route_manager>();
  rm->set_route("/file", "GET", std::move(std::make_unique<file_get_controller>()));
  rm->set_route("/file", "POST", std::move(std::make_unique<file_post_controller>()));
  my_app->configure(
		    std::move(std::make_unique<tcp_server>(ipaddr, port, my_app)),
		    std::move(ws), 
		    std::move(std::make_unique<json_util_context>()),
		    std::move(std::make_unique<utils>()),  
		    std::move(rm),
		    std::move(std::make_unique<http_request_context>(my_app)),
		    std::move(std::make_unique<http_response_context>(my_app)));
  
  std::cout << "server accepting connections on " << ipaddr << ":" << port << "\n";

  my_app->run(thread_number);
  
  
  return 0;
}
