//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//
#include "controllers.h"
#include "app_context.h"
#include "models.h"

//route is /file/{filename}
std::string file_get_controller::run(std::unordered_map<std::string, std::string> &&deserialized_input_data, app *ac)
{
  //check for file existense should be added
  std::size_t index = deserialized_input_data["url"].find_last_of("/");
  std::string filename = deserialized_input_data["url"].substr(index + 1);
  //if (!ac->app_cache_pool_[std::this_thread::get_id()]->find<std::string, std::string>(filename))
  //{
    //ac->app_cache_pool_[std::this_thread::get_id()]->insert<std::string, std::string>(std::make_pair(filename, std::move(ac->uc_->read_from_file("", filename))));
  //}
  //std::pair<std::string, std::string> k_v_pair = ac->app_cache_->[filename];
  return ac->uc_->read_from_file("", filename);
  //return (*(ac->app_cache_pool_[std::this_thread::get_id()])).operator[]<std::string, std::string>(filename).second;
  //return {};
}

//route is /file body is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
std::string file_post_controller::run(std::unordered_map<std::string, std::string> &&deserialized_input_data, app *ac)
{
  //eg model is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
  std::unique_ptr<file_model> fm = std::make_unique<file_model>();
  // this is json data so further deserialization is needed
  fm->model_map(std::move(ac->juc_->do_deserialize(std::move(deserialized_input_data["data"]))));
  fm->repr();
  ac->app_cache_pool_[std::this_thread::get_id()]->insert<std::string, std::string>(std::make_pair(fm->get_filename(), std::move(ac->uc_->read_from_file("", fm->get_filename()))));
  ac->app_cache_pool_[std::this_thread::get_id()]->state();
  return {};
}

std::string trigger_post_controller::run(std::unordered_map<std::string, std::string> &&deserialized_input_data, app *ac)
{
  std::unique_ptr<file_model> fm = std::make_unique<file_model>();
  // this is json data so further deserialization is needed
  fm->model_map(std::move(ac->juc_->do_deserialize(std::move(deserialized_input_data["data"]))));
  std::unordered_map<std::string, std::string> input_args = {{"data", ac->uc_->read_from_file("", fm->get_filename())}, {"Connection", "open"}};
  for (auto fd_pair : ac->ws_ioc_->broadcast_fd_list)
  {
    if (fd_pair.first)
    {
      //ac->executor_pool_[std::this_thread::get_id()]->produce_event<std::function<void()>>(std::move(std::bind(&i_response_context::do_create_response, ac->ws_ioc_->res_.get(), fd_pair.first, input_args)));
    }
  }
  return {};
}
