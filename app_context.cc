//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
//
// repository: https://github.com/nkoukoul/Elvis
//
#include "app_context.h"

app *app::app_instance_{nullptr};
std::mutex app::app_mutex_;

void app::configure(std::unique_ptr<tcp_handler> http_ioc,
                    std::unique_ptr<websocket_handler> ws_ioc,
                    std::unique_ptr<i_json_util_context> juc,
                    std::unique_ptr<utils> uc,
                    std::unique_ptr<route_manager> rm,
                    std::unique_ptr<i_event_queue> e_q)
{

  std::lock_guard<std::mutex> guard(app_mutex_);
  if (http_ioc)
    http_ioc_ = std::move(http_ioc);
  if (ws_ioc)
    ws_ioc_ = std::move(ws_ioc);
  if (juc)
    juc_ = std::move(juc);
  if (uc)
    uc_ = std::move(uc);
  if (rm)
    rm_ = std::move(rm);
  if (e_q)
    e_q_ = std::move(e_q);
  return;
}

void app::run(int thread_number)
{
  if (http_ioc_)
  {
    thread_pool_.reserve(thread_number - 1);
    for (auto i = thread_number - 1; i > 0; --i)
    {
      thread_pool_.emplace_back(
          [&http_ioc = (this->http_ioc_)] {
            http_ioc->run();
          });
      //Here we add a cache
      std::thread::id ti = thread_pool_.back().get_id();
      app_cache_pool.emplace(std::make_pair(ti, std::make_unique<t_cache<std::string, std::string>>(5)));
    }
    app_cache_pool.emplace(std::make_pair(std::this_thread::get_id(), std::make_unique<t_cache<std::string, std::string>>(5)));
    if (ws_ioc_)
    {
      ws_ioc_->run();
    }
    else
    {      
      http_ioc_->run();
    }
    
  }
  // Block until all the threads exit
  for (auto &t : thread_pool_)
    if (t.joinable())
      t.join();
  return;
}

app *app::get_instance()
{
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (app_instance_ == nullptr)
  {
    app_instance_ = new app();
  }
  return app_instance_;
}
