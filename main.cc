#include <memory>
#include <thread>
#include <string>
#include "app_context.h"
#include "tcp_server.h"
#include "json_utils.h"
#include "route_manager.h"
#include "request_context.h"
#include "response_context.h"

int main()
{
  int threads = 20;
  int port = 8589;
  std::string ipaddr = "127.0.0.1";

  std::unique_ptr<route_manager> rm = std::make_unique<route_manager>();
  rm->set_route("/file", "GET");
  rm->set_route("/file", "POST");
  app * my_app = app::get_instance(
				   std::move(std::make_unique<tcp_server>(ipaddr, port)), 
				   std::move(std::make_unique<json_util_context>()), 
				   std::move(rm),
				   std::move(std::make_unique<http_request_context>()));
  std::cout << "server accepting connections on " << ipaddr << ":" << port << "\n";

  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for(auto i = threads - 1; i > 0; --i)
    v.emplace_back(
		   [&my_app]
		   {
		     my_app->run();
		   });
  my_app->run();
  
  // Block until all the threads exit
  for(auto& t : v)
    if (t.joinable())
      t.join();

  return 0;
}
