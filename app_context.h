//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "common_headers.h"
#include "event_queue.h"
#include "io_context.h"
#include "request_context.h"
#include "route_manager.h"
#include "json_utils.h"
#include "utils.h"
#include "response_context.h"
#include "cache.h"
#include "db_connector.h"

class app
{
public:
  // Singletons should not be cloneable.
  app(app &other) = delete;

  // Singletons should not be assignable.
  void operator=(const app &) = delete;

  // This is the static method that controls the access to the singleton
  static app *get_instance();

  void configure(std::unique_ptr<tcp_handler> http_ioc = nullptr,
                 std::unique_ptr<http_request_context> http_req = nullptr,
                 std::unique_ptr<http_response_context> http_res = nullptr,
                 std::unique_ptr<websocket_request_context> ws_req = nullptr,
                 std::unique_ptr<websocket_response_context> ws_res = nullptr,
                 std::unique_ptr<i_json_util_context> juc = nullptr,
                 std::unique_ptr<utils> uc = nullptr,
                 std::unique_ptr<route_manager> rm = nullptr);

  void run(int thread_number);

  std::shared_ptr<i_event_queue> executor_;
  std::unique_ptr<tcp_handler> ioc_;
  std::unique_ptr<http_request_context> http_req_;
  std::unique_ptr<http_response_context> http_res_;
  std::unique_ptr<websocket_request_context> ws_req_;
  std::unique_ptr<websocket_response_context> ws_res_;
  std::unique_ptr<i_json_util_context> juc_;
  std::unique_ptr<utils> uc_;
  std::unique_ptr<route_manager> rm_;
  std::unique_ptr<i_db_manager> dbm_;
  std::unique_ptr<i_cache_manager> cm_;
  static std::mutex app_mutex_;
  std::mutex db_lock_;
  std::vector<int> broadcast_fd_list;

protected:
  app() = default;
  ~app(){};

private:
  static app *app_instance_;
  std::vector<std::thread> thread_pool_;
};

#endif //APP_CONTEXT_H
