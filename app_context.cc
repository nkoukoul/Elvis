#include "app_context.h"

app * app::app_instance_{nullptr};
std::mutex app::app_mutex_;

app * app::get_instance(std::unique_ptr<tcp_server> ts, 
			std::unique_ptr<i_json_util_context> juc, 
			std::unique_ptr<route_manager> rm,
			std::unique_ptr<i_request_context> req,
			std::unique_ptr<i_response_context> res){

  std::lock_guard<std::mutex> guard(app_mutex_);
  if (app_instance_ == nullptr) {
    app_instance_ = new app(std::move(ts), std::move(juc), std::move(rm), std::move(req), std::move(res));
  }
  return app_instance_;
}
