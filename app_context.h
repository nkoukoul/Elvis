//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
//
// repository: https://github.com/nkoukoul/Elvis
//


#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include "tcp_server.h"
#include "request_context.h"
#include "route_manager.h"
#include "json_utils.h"
#include "response_context.h"


class app{
public:
  
  // Singletons should not be cloneable.  
  app(app &other) = delete;

  // Singletons should not be assignable.
  void operator=(const app &) = delete;

  // This is the static method that controls the access to the singleton
  static app * get_instance(std::unique_ptr<tcp_server> ts = nullptr, 
			    std::unique_ptr<i_json_util_context> juc = nullptr, 
			    std::unique_ptr<route_manager> rm = nullptr,
			    std::unique_ptr<i_request_context> req = nullptr,
			    std::unique_ptr<i_response_context> res = nullptr);
  
  void run(int thread_number){

    if (ts_){
      io_context_threads_.reserve(thread_number - 1);
      for(auto i = thread_number - 1; i > 0; --i)
        io_context_threads_.emplace_back(
					 [&ioc = (this->ts_)]
					 {
					   ioc->accept_connections(app_instance_);
					 });
  
      ts_->accept_connections(app_instance_);

    }

    // Block until all the threads exit
    for(auto& t : io_context_threads_)
      if (t.joinable())
	t.join();

    return;
  }

  std::unique_ptr<tcp_server> ts_;
  std::unique_ptr<i_json_util_context> juc_; 
  std::unique_ptr<route_manager> rm_;
  std::unique_ptr<i_request_context> req_;
  std::unique_ptr<i_response_context> res_;

protected:
  app(std::unique_ptr<tcp_server> ts, 
      std::unique_ptr<i_json_util_context> juc, 
      std::unique_ptr<route_manager> rm,
      std::unique_ptr<i_request_context> req,
      std::unique_ptr<i_response_context> res)
    :ts_(std::move(ts)), juc_(std::move(juc)), rm_(std::move(rm)), req_(std::move(req)), res_(std::move(res)){}
  ~app(){}
  
private:
  static app * app_instance_;
  static std::mutex app_mutex_;
  std::vector<std::thread> io_context_threads_;
};

#endif //APP_CONTEXT_H
