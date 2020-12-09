//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
//
// repository: https://github.com/nkoukoul/Elvis
//


#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "common_headers.h"
#include "io_context.h"
#include "request_context.h"
#include "route_manager.h"
#include "json_utils.h"
#include "utils.h"
#include "response_context.h"
#include "t_cache.h"
#include "event_queue.h"

class app{
public:
  
  // Singletons should not be cloneable.  
  app(app &other) = delete;

  // Singletons should not be assignable.
  void operator=(const app &) = delete;

  // This is the static method that controls the access to the singleton
  static app * get_instance();

  void configure(std::unique_ptr<tcp_server> http_ioc = nullptr,
		 std::unique_ptr<websocket_server> ws_ioc = nullptr, 
		 std::unique_ptr<i_json_util_context> juc = nullptr,
		 std::unique_ptr<utils> uc = nullptr, 
		 std::unique_ptr<route_manager> rm = nullptr,
		 std::unique_ptr<i_event_queue> e_q = nullptr,
		 std::unique_ptr<i_cache> app_cache = nullptr);
  
  void run(int thread_number){
    if (http_ioc_){
      io_context_threads_.reserve(thread_number - 1);
      for(auto i = thread_number - 2; i > 0; --i){
        io_context_threads_.emplace_back(
					 [&http_ioc = (this->http_ioc_)]
					 {
					   http_ioc->run();
					 });
      }
      
      if (ws_ioc_){
	io_context_threads_.emplace_back(
					 [&ws_ioc = (this->ws_ioc_)]
					 {
					   ws_ioc->run();
					 });
      }else{
	io_context_threads_.emplace_back(
					 [&http_ioc = (this->http_ioc_)]
					 {
					   http_ioc->run();
					 });
      }
    
    }
    
    //main loop event queue
    if (e_q_){
      std::string broadcast_message;
      while (true){
	while (e_q_->empty()){
	  std::this_thread::yield();
	}
	broadcast_message = e_q_->consume_event<std::string>();
	for (auto fd : ws_ioc_->broadcast_fd_list){
	  if (fd){
	    ws_ioc_->res_->do_create_response(fd, {{"data", broadcast_message}, {"Connection", "open"}});
	  }
	}
      }
    }
    

    // Block until all the threads exit
    for(auto& t : io_context_threads_)
      if (t.joinable())
	t.join();

    return;
  }

  std::unique_ptr<tcp_server> http_ioc_;
  std::unique_ptr<websocket_server> ws_ioc_;
  std::unique_ptr<i_json_util_context> juc_; 
  std::unique_ptr<utils> uc_; 
  std::unique_ptr<route_manager> rm_;
  std::unique_ptr<i_event_queue> e_q_;
  std::unique_ptr<i_cache> app_cache_;
  static std::mutex app_mutex_;
protected:
  app() = default;
  ~app(){};
  
private:
  static app * app_instance_;
  std::vector<std::thread> io_context_threads_;
};

#endif //APP_CONTEXT_H
