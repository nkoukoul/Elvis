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
#include "t_cache.h"

class app{
public:
  
  // Singletons should not be cloneable.  
  app(app &other) = delete;

  // Singletons should not be assignable.
  void operator=(const app &) = delete;

  // This is the static method that controls the access to the singleton
  static app * get_instance();

  void configure(std::unique_ptr<tcp_handler> http_ioc = nullptr,
		 std::unique_ptr<websocket_handler> ws_ioc = nullptr, 
		 std::unique_ptr<i_json_util_context> juc = nullptr,
		 std::unique_ptr<utils> uc = nullptr, 
		 std::unique_ptr<route_manager> rm = nullptr);
  
  void run(int thread_number);

  std::unique_ptr<tcp_handler> http_ioc_;
  std::unique_ptr<websocket_handler> ws_ioc_;
  std::unique_ptr<i_json_util_context> juc_; 
  std::unique_ptr<utils> uc_; 
  std::unique_ptr<route_manager> rm_;
  std::unordered_map<std::thread::id, std::unique_ptr<i_cache>> app_cache_pool_;
  static std::mutex app_mutex_;
protected:
  app() = default;
  ~app(){};
  
private:
  static app * app_instance_;
  std::vector<std::thread> thread_pool_;
};

#endif //APP_CONTEXT_H
