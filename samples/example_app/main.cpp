//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "my_controllers.h"
#include <elvis/app_context.h>
#include <functional>
#include <memory>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cout << "Usage: app hostname port thread_number\n";
    return -1;
  }

  std::string ipaddr = argv[1];
  int port = std::stoi(argv[2]);
  int thread_number = std::max<int>(1, std::stoi(argv[3]));
  app *my_app = app::get_instance();

  // Route manager is used to connect endpoints with controllers
  std::shared_ptr<Elvis::RouteManager> routeManager =
      std::make_shared<Elvis::RouteManager>();
  routeManager->SetRoute(
      "/file", "GET", std::move(std::make_unique<FileGetController>(my_app)));
  routeManager->SetRoute(
      "/file", "POST", std::move(std::make_unique<FilePostController>(my_app)));
  routeManager->SetRoute(
      "/triggers", "POST",
      std::move(std::make_unique<trigger_post_controller>(my_app)));
  routeManager->SetRoute(
      "/login", "POST",
      std::move(std::make_unique<user_post_controller>(my_app)));
  auto concurrentQueue = std::make_shared<Elvis::ConcurrentQueue>(100);
  // Application context is injected with the http request and response handler,
  // the websocket request and response handler the route manager and json/utils
  my_app->configure(
      ipaddr, port, concurrentQueue,
      std::move(std::make_unique<Elvis::HttpRequestContext>(
          std::make_unique<Elvis::HttpResponseContext>(my_app), concurrentQueue,
          routeManager)),
      std::move(std::make_unique<Elvis::WebsocketRequestContext>(
          std::make_unique<Elvis::WebsocketResponseContext>(my_app),
          concurrentQueue)),
      std::move(std::make_unique<Elvis::JSONContext>()));

  std::cout << "server accepting connections on " << ipaddr << ":" << port
            << "\n";

  my_app->run(thread_number);
  return 0;
}
