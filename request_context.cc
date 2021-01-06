//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "request_context.h"
#include "app_context.h"
#include <sstream>

http_request_parser::http_request_parser(app *application_context) : application_context_(application_context) {}

void http_request_parser::parse(
    int const client_socket,
    std::string &&input_data,
    std::shared_ptr<i_event_queue> executor) const
{

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
  while (line.size() > 1)
  {
    std::getline(ss, line);
    std::size_t column_index = line.find_first_of(":");
    if (column_index != std::string::npos)
      deserialized_input_data.insert(std::make_pair(line.substr(0, column_index), line.substr(column_index + 2, line.size() - 3 - column_index)));
  }

  if (deserialized_input_data["request_type"] == "POST")
  {
    std::string deserialized_data;
    //json for post
    std::getline(ss, line);
    while (line.size() > 1)
    {
      deserialized_data += line;
      std::getline(ss, line);
    }
    deserialized_input_data.insert(std::make_pair("data", std::move(deserialized_data)));
  }

  i_controller *ic = application_context_->rm_->get_controller(
      deserialized_input_data["url"],
      deserialized_input_data["request_type"]);
  
  if (ic)
  {
    deserialized_input_data["status"] = "200 OK";
    deserialized_input_data["controller_data"] = "";
    executor->produce_event<std::function<void()>>(
      std::move(
          std::bind(
              &i_controller::run,
              ic,
              std::move(deserialized_input_data),
              client_socket,
              application_context_,
              executor)));  
  }
  else
  {
    deserialized_input_data["status"] = "400 Bad Request";
    deserialized_input_data["controller_data"] = "Url or method not supported";
    
    executor->produce_event<std::function<void()>>(
        std::move(
            std::bind(
                &i_response_context::do_create_response,
                application_context_->http_ioc_->res_.get(),
                client_socket,
                std::move(deserialized_input_data),
                executor)));
  }
}

websocket_request_parser::websocket_request_parser(app *application_context) : application_context_(application_context) {}

void websocket_request_parser::parse(
    int const client_socket,
    std::string &&input_data,
    std::shared_ptr<i_event_queue> executor) const
{
  unsigned int payload_length = application_context_->uc_->binary_to_decimal(input_data.substr(9, 7));
  std::string mask_key;
  std::string unmasked_payload_data;
  std::string masked_payload_data;
  std::string connection = "open";
  int opcode = application_context_->uc_->binary_to_decimal(input_data.substr(4, 4));
  if (opcode == 8)
  { //close received
    connection = "close";
  }
  if (payload_length == 126)
  {
    payload_length = application_context_->uc_->binary_to_decimal(input_data.substr(16, 16));
    mask_key = input_data.substr(32, 32);
    masked_payload_data = input_data.substr(64, payload_length * 8);
  }
  else if (payload_length == 127)
  {
    payload_length = application_context_->uc_->binary_to_decimal(input_data.substr(16, 64));
    mask_key = input_data.substr(80, 32);
    masked_payload_data = input_data.substr(112, payload_length * 8);
  }
  else
  {
    mask_key = input_data.substr(16, 32);
    masked_payload_data = input_data.substr(48, payload_length * 8);
  }

  for (int i = 0; i < masked_payload_data.size() / 8; i++)
  {
    std::string masked_sub_data = masked_payload_data.substr(i * 8, 8);
    std::string mask_sub_key = mask_key.substr((i % 4) * 8, 8);
    unmasked_payload_data += static_cast<char>(application_context_->uc_->binary_to_decimal(masked_sub_data) ^ application_context_->uc_->binary_to_decimal(mask_sub_key));
  }
  //echo functinality for now
  std::unordered_map<std::string, std::string> deserialized_input_data({{"data", unmasked_payload_data}, {"Connection", connection}});
  executor->produce_event<std::function<void()>>(
      std::move(
          std::bind(
              &i_response_context::do_create_response,
              application_context_->ws_ioc_->res_.get(),
              client_socket,
              std::move(deserialized_input_data),
              executor)));
}
