#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <memory>
#include <mutex>
#include <thread>
#include "tcp_server.h"
#include "route_manager.h"
#include "json_utils.h"
#include "request_context.h"

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
			      std::unique_ptr<i_request_context> rc = nullptr);
  
  void run(){
//std::thread t;
    if (ts_){
      ts_->set_json_util_context(juc_);
      ts_->set_route_manager(rm_);
      ts_->set_request_context(rc_);
      ts_->accept_connections();
//t = std::thread(&tcp_server::accept_connections, ts_);
    }
    // while(true){
    //   ;;
    // }
    // if (t.joinable())
    //   t.join();
    
    return;
  }

protected:
  app(std::shared_ptr<tcp_server> ts, 
	std::shared_ptr<i_json_util_context> juc, 
	std::shared_ptr<route_manager> rm,
	std::shared_ptr<i_request_context> rc)
    :ts_(ts), juc_(juc), rm_(rm), rc_(rc){}
  ~app(){}
  
private:
  static app * app_instance_;
  static std::mutex app_mutex_;
  std::shared_ptr<tcp_server> ts_;
  std::shared_ptr<i_json_util_context> juc_; 
  std::shared_ptr<route_manager> rm_;
  std::shared_ptr<i_request_context> rc_;
};

app * app::app_instance_{nullptr};
std::mutex app::app_mutex_;

app * app::get_instance(std::unique_ptr<tcp_server> ts, 
			  std::unique_ptr<i_json_util_context> juc, 
			  std::unique_ptr<route_manager> rm,
			  std::unique_ptr<i_request_context> rc){

  std::lock_guard<std::mutex> guard(app_mutex_);
  if (app_instance_ == nullptr) {
    app_instance_ = new app(std::move(ts), std::move(juc), std::move(rm), std::move(rc));
  }
  return app_instance_;
}

#endif //APP_CONTEXT_H
