#include "app_context.h"

app * app::app_instance_{nullptr};
std::mutex app::app_mutex_;


void app::configure(std::unique_ptr<io_context> http_ioc,
		    std::unique_ptr<websocket_server> ws_ioc, 
		    std::unique_ptr<i_json_util_context> juc, 
		    std::unique_ptr<route_manager> rm,
		    std::unique_ptr<i_request_context> req,
		    std::unique_ptr<i_response_context> res){
  
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (http_ioc)
    http_ioc_ = std::move(http_ioc);
  if (ws_ioc)
    ws_ioc_ = std::move(ws_ioc);
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
    app_instance_->app_cache_ = std::make_unique<t_cache<std::string, std::string>>(5);
  }
  return app_instance_;
}
