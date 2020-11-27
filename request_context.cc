//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#include "request_context.h"
#include <sstream>


std::unordered_map<std::string, std::string> http_request_parser::parse(std::string && input_data){
  std::unordered_map<std::string, std::string> input_request;
  
  std::istringstream ss(input_data);
  std::string request_type, url, protocol, line;
  
  std::getline(ss, line);
  std::istringstream first_line(line);
  first_line >> request_type >> url >> protocol;
  
  input_request.insert(std::make_pair("request_type", std::move(request_type)));
  input_request.insert(std::make_pair("url", std::move(url)));
  input_request.insert(std::make_pair("protocol", std::move(protocol)));

  while (line.size() > 1){
    //headers here
    std::getline(ss, line);
  }

  if (input_request["request_type"] == "POST"){
    std::string deserialized_data;
    //json for post
    std::getline(ss, line);
    while (line.size() > 1){
      deserialized_data += line;
      std::getline(ss, line);
    }
    input_request.insert(std::make_pair("data", std::move(deserialized_data)));
  }
  return input_request;
}
