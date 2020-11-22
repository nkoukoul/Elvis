#include "app_context.h"
#include "tcp_server.h"
#include "json_utils.h"
#include "route_manager.h"

int main()
{
  route_manager * rm = new route_manager();
  rm->set_route("/file", "GET");
  rm->set_route("/file", "POST");
  app * my_app = app::get_instance(new tcp_server("127.0.0.1", 8589), new json_util_context(), rm);
  my_app->run();
  return 0;
}
