#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "app_context.h"
#include "tcp_server.h"
#include "json_utils.h"


int main()
{
  app * my_app = app::get_instance(new tcp_server("127.0.0.1", 8589,new json_util_context()));
  my_app->run();
  return 0;
}
