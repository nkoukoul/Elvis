#include "my_controllers.h"
#include <elvis/app_context.h>
#include <elvis/route_manager.h>
#include <functional>
#include <memory>
#include <string>
#include <thread>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage: app hostname port thread_number\n";
        return -1;
    }

    std::string ipaddr = argv[1];
    int port = std::stoi(argv[2]);
    int thread_number = std::max<int>(1, std::stoi(argv[3]));
    Elvis::App *elvis = Elvis::App::GetInstance();

    // Route manager is used to connect endpoints with controllers
    std::shared_ptr<Elvis::RouteManager> routeManager =
        std::make_shared<Elvis::RouteManager>();
    routeManager->SetRoute(
        "/file", "GET", std::move(std::make_unique<FileGetController>()));
    routeManager->SetRoute(
        "/file", "POST", std::move(std::make_unique<FilePostController>()));
    routeManager->SetRoute(
        "/triggers", "POST",
        std::move(std::make_unique<TriggerPostController>()));
    routeManager->SetRoute(
        "/login", "POST",
        std::move(std::make_unique<UserPostController>()));
    // Application context is injected with the http request and response handler,
    // the websocket request and response handler the route manager and json/utils
    elvis->Configure(ipaddr, port, thread_number, routeManager, "server.log", Elvis::LogLevel::INFO);

    std::cout << "server accepting connections on " << ipaddr << ":" << port
              << "\n";

    elvis->Run();
    return 0;
}
