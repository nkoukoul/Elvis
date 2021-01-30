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
                    std::unique_ptr<http_request_context> http_req,
                    std::unique_ptr<http_response_context> http_res,
                    std::unique_ptr<websocket_request_context> ws_req,
                    std::unique_ptr<websocket_response_context> ws_res,
                    std::unique_ptr<i_json_util_context> juc,
                    std::unique_ptr<utils> uc,
                    std::unique_ptr<route_manager> rm)
{
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (http_ioc)
    ioc_ = std::move(http_ioc);
  http_req_ = std::move(http_req);
  http_res_ = std::move(http_res);
  ws_req_ = std::move(ws_req);
  ws_res_ = std::move(ws_res);
  if (juc)
    juc_ = std::move(juc);
  if (uc)
    uc_ = std::move(uc);
  if (rm)
    rm_ = std::move(rm);
  return;
}

void app::run(int thread_number)
{
  broadcast_fd_list.resize(256, {0, ""});
  if (ioc_)
  {
    thread_pool_.reserve(thread_number - 1);
    for (auto i = thread_number - 1; i > 0; --i)
    {
      thread_pool_.emplace_back(
          [&http_ioc = (this->ioc_)] {
            http_ioc->run(
                app_instance_,
                std::make_shared<event_queue<std::function<void()>>>(4000));
          });
    }
    ioc_->run(
        app_instance_,
        std::make_shared<event_queue<std::function<void()>>>(4000));
  }
  // Block until all the threads exit
  for (auto &t : thread_pool_)
    if (t.joinable())
      t.join();
  return;
}

void app::add_route(std::string key, std::string value)
{
  route_manager_table_.insert(std::make_pair(key, value));
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
