//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#include "controllers.h"

std::string file_get_controller::run(std::unordered_map<std::string, std::string>  && deserialized_input_data){
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

std::string file_post_controller::run(std::unordered_map<std::string, std::string>  && deserialized_input_data){

  return{};
}
