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
#include "models.h"

void i_controller::run(std::shared_ptr<client_context> c_ctx, app *ac)
{
  do_stuff(c_ctx->http_headers_, ac);
  ac->executor_->produce_event<std::function<void()>>(
      std::move(
          std::bind(
              &i_response_context::do_create_response,
              ac->http_res_.get(),
              c_ctx)));
}

//route is /file/{filename}
void file_get_controller::do_stuff(
    std::unordered_map<std::string,
                       std::string> &deserialized_input_data,
    app *ac)
{
  //check for file existense should be added
  std::size_t index = deserialized_input_data["url"].find_last_of("/");
  std::string filename = deserialized_input_data["url"].substr(index + 1);
  auto fm = std::make_unique<file_model>();
  fm->filename_.set(filename);
  fm->retrieve_model(ac);
  //fm->repr();
  std::string controller_data = ac->cache_->operator[]<std::string, std::string>(filename);
  if (controller_data.empty())
  {
    controller_data = ac->uc_->read_from_file("", filename);
    ac->cache_->insert<std::string, std::string>(
        std::make_pair(filename, controller_data));
  }
  deserialized_input_data["controller_data"] = ac->uc_->read_from_file("", filename);
}

// route is /file body is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
// used to refresh or add to cache content
void file_post_controller::do_stuff(
    std::unordered_map<std::string,
                       std::string> &deserialized_input_data,
    app *ac)
{
  //eg model is {"filename": "test.txt",  "md5sum_": "5f7f11f4b89befa92c9451ffa5c81184"}
  auto fm = std::make_unique<file_model>();
  // this is json data so further deserialization is needed
  auto model = ac->juc_->do_deserialize(std::move(deserialized_input_data["data"])).front();
  fm->filename_.set(model["filename"]);
  fm->md5sum_.set(model["md5sum"]);
  //fm->repr();
  fm->insert_model(ac);
  ac->cache_->insert<std::string, std::string>(
      std::make_pair(fm->filename_.get(),
                     std::move(ac->uc_->read_from_file("", fm->filename_.get()))));
  ac->cache_->state();
}

// http request is used to trigger a broadcast to all the ws clients
void trigger_post_controller::do_stuff(
    std::unordered_map<std::string, std::string> &deserialized_input_data,
    app *ac)
{
  // this is json data so further deserialization is needed
  std::unordered_map<std::string, std::string> input_args =
      {{"data", ac->uc_->read_from_file("", "index.html")}, {"Connection", "open"}};
  for (auto fd : ac->broadcast_fd_list)
  {

    // executor->produce_event<std::function<void()>>(
    // std::move(
    //     std::bind(
    //         &i_response_context::do_create_response,
    //         ac->ws_ioc_->res_.get(),
    //         fd_pair.first,
    //         input_args,
    //         executor)));
  }
}
