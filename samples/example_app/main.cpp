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
#include <elvis/app_context.h>
#include "my_controllers.h"

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cout << "Usage: app hostname port thread_number\n";
		return -1;
	}

	std::string ipaddr = argv[1];
	int port = std::stoi(argv[2]);
	int thread_number = std::max<int>(1, std::stoi(argv[3]));
	app* my_app = app::get_instance();

	//Here we create a tcp connection handler
	std::unique_ptr<Elvis::TCPContext> tcp_server = std::make_unique<Elvis::TCPContext>
		(ipaddr, port, my_app);

	//Route manager is used to connect endpoints with controllers
	std::unique_ptr<Elvis::RouteManager> routeManager = std::make_unique<Elvis::RouteManager>();
	routeManager->SetRoute("/file", "GET", std::move(std::make_unique<file_get_controller>()));
	routeManager->SetRoute("/file", "POST", std::move(std::make_unique<FilePostController>()));
	routeManager->SetRoute("/triggers", "POST", std::move(std::make_unique<trigger_post_controller>()));
	routeManager->SetRoute("/login", "POST", std::move(std::make_unique<user_post_controller>()));
	//Application context is injected with the http request and response handler, 
	//the websocket request and response handler the route manager and json/utils
	my_app->configure(std::move(tcp_server),
		std::move(std::make_unique<Elvis::HttpRequestContext>(my_app)),
		std::move(std::make_unique<Elvis::HttpResponseContext>(my_app)),
		std::move(std::make_unique<Elvis::WebsocketRequestContext>(my_app)),
		std::move(std::make_unique<Elvis::WebsocketResponseContext>(my_app)),
		std::move(std::make_unique<Elvis::JSONContext>()),
		std::move(routeManager));

	std::cout << "server accepting connections on " << ipaddr << ":" << port << "\n";

	my_app->run(thread_number);
	return 0;
}
