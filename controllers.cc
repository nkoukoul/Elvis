//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//
#include "controllers.h"
#include "app_context.h"
#include "models.h"

std::string file_get_controller::run(std::unordered_map<std::string, std::string>  && deserialized_input_data, app * ac){
  std::string page = "<!doctype html>"
    "<html>"
    "<head>"
    "<title>This is the title of the webpage!</title>"
    "</head>"
    "<body>"
    "<p>This is an example paragraph. Anything in the <strong>body</strong> tag will appear on the page, just like this <strong>p</strong> tag and its contents.</p>"
    "</body>"
    "</html>";
  return page;
}

std::string file_post_controller::run(std::unordered_map<std::string, std::string>  && deserialized_input_data, app * ac){
  //eg model is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
  std::unique_ptr<file_model> fm = std::make_unique<file_model>();
  // this is json data so further deserialization is needed
  fm->model_map(std::move(ac->juc_->do_deserialize(std::move(deserialized_input_data["data"]))));
  fm->repr();
  if (!ac->app_cache_->find(fm->get_filename())){
    ac->app_cache_->insert(std::make_pair(fm->get_filename(), fm->get_md5sum()));
  } else { 
    auto test_pair = (*(ac->app_cache_))[fm->get_filename()];
    std::cout << "key " << test_pair.first << " found with value " << test_pair.second << "\n";
  }
  ac->app_cache_->state();
  return {};
}
