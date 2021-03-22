//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//
#include "controllers.h"
#include "app_context.h"
#include "response_context.h"
#include "event_queue.h"

void i_controller::run(std::shared_ptr<elvis::io_context::client_context> c_ctx, app *ac)
{
  do_stuff(c_ctx->http_headers_, ac);
  auto executor = ac->sm_->access_strand<event_queue<std::function<void()>>>();
  executor->produce_event(
      std::move(
          std::bind(
              &i_response_context::do_create_response,
              ac->http_res_.get(),
              c_ctx)));
}
