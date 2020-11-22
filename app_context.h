#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <mutex>
#include <thread>
#include "tcp_server.h"

class app{
public:
  
  // Singletons should not be cloneable.  
  app(app &other) = delete;

  // Singletons should not be assignable.
  void operator=(const app &) = delete;

  //
  // This is the static method that controls the access to the singleton
  static app * get_instance(tcp_server * ts = nullptr);
  
  void run(){
    std::thread t;
    if (ts_){
      t = std::thread(&tcp_server::accept_connections,ts_);
    }
    while(true){
      ;;
    }
    if (t.joinable())
      t.join();
    return;
  }

protected:
  app(tcp_server * ts):ts_(ts){}
  ~app(){}
  
private:
  static app * app_instance_;
  static std::mutex app_mutex_;
  tcp_server * ts_;
};

app * app::app_instance_{nullptr};
std::mutex app::app_mutex_;

app * app::get_instance(tcp_server * ts){
  std::lock_guard<std::mutex> lock(app_mutex_);
  if (app_instance_ == nullptr) {
    app_instance_ = new app(ts);
  }
  return app_instance_;
}

#endif //APP_CONTEXT_H
