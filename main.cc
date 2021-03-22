//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
//
// repository: https://github.com/nkoukoul/Elvis
//

#include <memory>
#include <thread>
#include <string>
#include <functional>
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

  //Here we create a tcp connection handler
  std::unique_ptr<elvis::io_context::tcp_handler> tcp_server = std::make_unique<elvis::io_context::tcp_handler>
    (ipaddr, port, my_app);

  //Route manager is used to connect endpoints with controllers
  std::unique_ptr<route_manager> rm = std::make_unique<route_manager>();
  rm->set_route("/file", "GET", std::move(std::make_unique<file_get_controller>()));
  rm->set_route("/file", "POST", std::move(std::make_unique<file_post_controller>()));
  rm->set_route("/triggers", "POST", std::move(std::make_unique<trigger_post_controller>()));
  rm->set_route("/login", "POST", std::move(std::make_unique<user_post_controller>()));
  //Application context is injected with the http request and response handler, 
  //the websocket request and response handler the route manager and json/utils
  my_app->configure(std::move(tcp_server),
        std::move(std::make_unique<http_request_context>(my_app)),
        std::move(std::make_unique<http_response_context>(my_app)),
        std::move(std::make_unique<websocket_request_context>(my_app)),
        std::move(std::make_unique<websocket_response_context>(my_app)), 
		    std::move(std::make_unique<json_util_context>()),
		    std::move(rm));
  
  std::cout << "server accepting connections on " << ipaddr << ":" << port << "\n";

  my_app->run(thread_number);
  return 0;
}
