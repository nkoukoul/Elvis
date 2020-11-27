#include "app_context.h"

app * app::app_instance_{nullptr};
std::mutex app::app_mutex_;


void app::configure(std::unique_ptr<io_context> ioc, 
			std::unique_ptr<i_json_util_context> juc, 
			std::unique_ptr<route_manager> rm,
			std::unique_ptr<i_request_context> req,
			std::unique_ptr<i_response_context> res){
  
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (ioc)
    ioc_ = std::move(ioc);
  if (juc)
    juc_ = std::move(juc);
  if(rm)
    rm_ = std::move(rm);
  if (req)
    req_ = std::move(req);
  if (res)
    res_ = std::move(res);
  return;
}
  

app * app::get_instance(){
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (app_instance_ == nullptr) {
    app_instance_ = new app();
  }
  return app_instance_;
}
