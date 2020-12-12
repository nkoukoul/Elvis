#include "app_context.h"

app * app::app_instance_{nullptr};
std::mutex app::app_mutex_;


void app::configure(std::unique_ptr<tcp_server> http_ioc,
		    std::unique_ptr<websocket_server> ws_ioc, 
		    std::unique_ptr<i_json_util_context> juc, 
		    std::unique_ptr<utils> uc,
		    std::unique_ptr<route_manager> rm,
		    std::unique_ptr<i_event_queue> e_q,
		    std::unique_ptr<i_cache> app_cache){
  
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (http_ioc)
    http_ioc_ = std::move(http_ioc);
  if (ws_ioc)
    ws_ioc_ = std::move(ws_ioc);
  if (juc)
    juc_ = std::move(juc);
  if (uc)
    uc_ = std::move(uc);
  if(rm)
    rm_ = std::move(rm);
  if(e_q)
    e_q_ = std::move(e_q);
  if(app_cache)
    app_cache_ = std::move(app_cache);
  return;
}

void app::run(int thread_number){
  if (e_q_){
    thread_pool_.reserve(thread_number - 1);
    for(auto i = thread_number - 3; i > 0; --i){
      thread_pool_.emplace_back(
				[&e_q = (this->e_q_)]
				{
				  while (true){
				    while (e_q->empty()){
				      std::this_thread::yield();
				    }
				    auto async_func = e_q->consume_event<std::function<void()>>();
				    try{
				      async_func();
				    }
				    catch (const std::bad_function_call& e){
				      ;;
				    }
				  }
				});
    }
      
    if (ws_ioc_){
      thread_pool_.emplace_back(
				[&ws_ioc = (this->ws_ioc_)]
				{
				  ws_ioc->run();
				});
    }
    
  }
    
  //main io_context
  if (http_ioc_){
    thread_pool_.emplace_back(
			      [&http_ioc = (this->http_ioc_)]
			      {
				http_ioc->run();
			      });

    http_ioc_->run();
  }
    

  // Block until all the threads exit
  for(auto& t : thread_pool_)
    if (t.joinable())
      t.join();

  return;
}  

app * app::get_instance(){
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (app_instance_ == nullptr) {
    app_instance_ = new app();
  }
  return app_instance_;
}
