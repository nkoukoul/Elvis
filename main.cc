#include <memory>
#include "app_context.h"
#include "tcp_server.h"
#include "json_utils.h"
#include "route_manager.h"
#include "request_context.h"

int main()
{
  std::unique_ptr<route_manager> rm = std::make_unique<route_manager>();
  rm->set_route("/file", "GET");
  rm->set_route("/file", "POST");
  app * my_app = app::get_instance(
				   std::move(std::make_unique<tcp_server>("127.0.0.1", 8589)), 
				   std::move(std::make_unique<json_util_context>()), 
				   std::move(rm),
				   std::move(std::make_unique<http_context>()));
  my_app->run();
  return 0;
}
