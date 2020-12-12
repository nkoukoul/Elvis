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
  
  //Here we create an event queue
  std::unique_ptr<event_queue<std::function<void()>>> e_q = std::make_unique<event_queue<std::function<void()>>>(4000, my_app);

  //Here we create a cache
  std::unique_ptr<t_cache<std::string, std::string>> app_cache = std::make_unique<t_cache<std::string, std::string>>(5);

  //Here we create a tcp server and inject it with an http request/response context
  std::unique_ptr<tcp_server> http_server = std::make_unique<tcp_server>
    (ipaddr, port, std::move(std::make_unique<http_request_context>(my_app)), std::move(std::make_unique<http_response_context>(my_app)), my_app);

  //Here we create a handler for websocket connections and inject it with a websocket request/response context
  std::unique_ptr<websocket_server> ws = std::make_unique<websocket_server>
    (ipaddr, port, std::move(std::make_unique<websocket_request_context>(my_app)), std::move(std::make_unique<websocket_response_context>(my_app)), my_app);
  
  //Route manager is used to connect endpoints with controllers
  std::unique_ptr<route_manager> rm = std::make_unique<route_manager>();
  rm->set_route("/file", "GET", std::move(std::make_unique<file_get_controller>()));
  rm->set_route("/file", "POST", std::move(std::make_unique<file_post_controller>()));
  rm->set_route("/triggers", "POST", std::move(std::make_unique<trigger_post_controller>()));

  //Application context is injected with the tcp_server, the websocket handler, the utils the route manager the event queue and the cache
  my_app->configure(std::move(http_server),
		    std::move(ws), 
		    std::move(std::make_unique<json_util_context>()),
		    std::move(std::make_unique<utils>()),  
		    std::move(rm),
		    std::move(e_q),
		    std::move(app_cache));
  
  std::cout << "server accepting connections on " << ipaddr << ":" << port << "\n";

  my_app->run(thread_number);
  
  
  return 0;
}
