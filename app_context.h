#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <mutex>
#include <thread>
#include "tcp_server.h"
#include "route_manager.h"
#include "json_utils.h"

class app{
public:
  
  // Singletons should not be cloneable.  
  app(app &other) = delete;

  // Singletons should not be assignable.
  void operator=(const app &) = delete;

  // This is the static method that controls the access to the singleton
  static app * get_instance(tcp_server * ts = nullptr, i_json_util_context * juc = nullptr, route_manager * rm = nullptr);
  
  void run(){
    std::thread t;
    if (ts_){
      ts_->set_json_util_context(juc_);
      ts_->set_route_manager(rm_);
      t = std::thread(&tcp_server::accept_connections,ts_);
    }
    while(true){
      ;;
    }
    if (t.joinable())
      t.join();
    
    delete ts_; 
    return;
  }

protected:
  app(tcp_server * ts, i_json_util_context * juc, route_manager * rm):ts_(ts), juc_(juc), rm_(rm){}
  
  ~app(){}
  
private:
  static app * app_instance_;
  static std::mutex app_mutex_;
  tcp_server * ts_;
  i_json_util_context * juc_; 
  route_manager * rm_;
};

app * app::app_instance_{nullptr};
std::mutex app::app_mutex_;

app * app::get_instance(tcp_server * ts, i_json_util_context * juc, route_manager * rm){
  std::lock_guard<std::mutex> lock(app_mutex_);
  if (app_instance_ == nullptr) {
    app_instance_ = new app(ts, juc, rm);
  }
  return app_instance_;
}

#endif //APP_CONTEXT_H
