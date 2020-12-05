//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#include "request_context.h"
#include "app_context.h"
#include <sstream>

http_request_parser::http_request_parser(app * application_context):application_context_(application_context){}

void http_request_parser::parse(int client_socket, std::string && input_data){
  
  std::unordered_map<std::string, std::string> deserialized_input_data;
  
  std::istringstream ss(input_data);
  std::string request_type, url, protocol, line;
  
  //first line
  std::getline(ss, line);
  std::istringstream first_line(line);
  first_line >> request_type >> url >> protocol;
  
  deserialized_input_data.insert(std::make_pair("request_type", std::move(request_type)));
  deserialized_input_data.insert(std::make_pair("url", std::move(url)));
  deserialized_input_data.insert(std::make_pair("protocol", std::move(protocol)));
  
  //headers here
  while (line.size() > 1){
    std::getline(ss, line);
    std::size_t column_index = line.find_first_of(":");
    if (column_index != std::string::npos)
      deserialized_input_data.insert(std::make_pair(line.substr(0, column_index), line.substr(column_index + 2, line.size()- 3 - column_index)));
  }

  if (deserialized_input_data["request_type"] == "POST"){
    std::string deserialized_data;
    //json for post
    std::getline(ss, line);
    while (line.size() > 1){
      deserialized_data += line;
      std::getline(ss, line);
    }
    deserialized_input_data.insert(std::make_pair("data", std::move(deserialized_data)));
  }
  
  for (auto elem : deserialized_input_data)
    std::cout << "key " << elem.first << " value " << elem.second << " with size " << elem.second.size() << "\n";
  
  return application_context_->res_->do_create_response(client_socket, std::move(deserialized_input_data));
}
