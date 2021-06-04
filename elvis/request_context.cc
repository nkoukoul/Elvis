//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//
#include "io_context.h"
#include "request_context.h"
#include "app_context.h"
#include <sstream>

http_request_parser::http_request_parser(app *application_context) : application_context_(application_context) {}

void http_request_parser::parse(std::shared_ptr<elvis::io_context::client_context> c_ctx) const
{
  //auto executor = application_context_->sm_->access_strand<event_queue<std::function<void()>>>();
  std::istringstream ss(c_ctx->http_message_);
  std::string request_type, url, protocol, line;

  //first line
  std::getline(ss, line);
  std::istringstream first_line(line);
  first_line >> request_type >> url >> protocol;

  c_ctx->http_headers_.insert(std::make_pair("request_type", std::move(request_type)));
  c_ctx->http_headers_.insert(std::make_pair("url", std::move(url)));
  c_ctx->http_headers_.insert(std::make_pair("protocol", std::move(protocol)));

  //headers here
  while (line.size() > 1)
  {
    std::getline(ss, line);
    std::size_t column_index = line.find_first_of(":");
    if (column_index != std::string::npos)
      c_ctx->http_headers_.insert(std::make_pair(line.substr(0, column_index), line.substr(column_index + 2, line.size() - 3 - column_index)));
  }

  if (c_ctx->http_headers_["request_type"] == "POST")
  {
    std::string deserialized_data;
    //json for post
    std::getline(ss, line);
    while (line.size() > 1)
    {
      deserialized_data += line;
      std::getline(ss, line);
    }
    c_ctx->http_headers_.insert(std::make_pair("data", std::move(deserialized_data)));
  }

  i_controller *ic = application_context_->rm_->get_controller(
      c_ctx->http_headers_["url"],
      c_ctx->http_headers_["request_type"]);

  if (ic)
  {
    c_ctx->http_headers_["status"] = "200 OK";
    c_ctx->http_headers_["controller_data"] = "";
    std::future<void> event = std::async(std::launch::deferred, &i_controller::run, ic, c_ctx, application_context_);
    application_context_->sm_->produce_event(std::move(event));
  }
  else
  {
    c_ctx->http_headers_["status"] = "400 Bad Request";
    c_ctx->http_headers_["controller_data"] = "Url or method not supported";
    std::future<void> event = std::async(std::launch::deferred, &i_response_context::do_create_response, application_context_->http_res_.get(), c_ctx);
    application_context_->sm_->produce_event(std::move(event));
  }
}

websocket_request_parser::websocket_request_parser(app *application_context) : application_context_(application_context) {}

void websocket_request_parser::parse(std::shared_ptr<elvis::io_context::client_context> c_ctx) const
{
  int fin, rsv1, rsv2, rsv3, opcode, mask, i = 0;
  uint64_t payload_len;
  unsigned char mask_key[4];
  uint8_t octet = (uint8_t)c_ctx->websocket_message_[i++]; //8 bit char
  fin = (octet >> 7) & 0x01;
  rsv1 = (octet >> 6) & 0x01;
  rsv2 = (octet >> 5) & 0x01;
  rsv3 = (octet >> 4) & 0x01;
  opcode = octet & 0x0F;
  octet = (uint8_t)c_ctx->websocket_message_[i++]; //8 bit char
  mask = (octet >> 7) & 0x01;
  payload_len = (octet)&0x7F;
  std::string unmasked_payload_data;
  std::string masked_payload_data;
  if (opcode == 8)
  { //close received
    c_ctx->close_connection_ = true;
  }
  if (payload_len == 126)
  {
    payload_len = ((uint16_t)c_ctx->websocket_message_[i] << 8 & 0xff00) + ((uint16_t)c_ctx->websocket_message_[i + 1] & 0x00ff);
    i += 2;
  }
  else if (payload_len == 127)
  {
    payload_len = ((uint64_t)c_ctx->websocket_message_[i] << 56 & 0xff00000000000000) 
                  + ((uint64_t)c_ctx->websocket_message_[i + 1] << 48 & 0x00ff000000000000)
                  + ((uint64_t)c_ctx->websocket_message_[i + 2] << 40 & 0x0000ff0000000000)
                  + ((uint64_t)c_ctx->websocket_message_[i + 3] << 32 & 0x000000ff00000000)
                  + ((uint64_t)c_ctx->websocket_message_[i + 4] << 24 & 0x00000000ff000000)
                  + ((uint64_t)c_ctx->websocket_message_[i + 5] << 16 & 0x0000000000ff0000)
                  + ((uint64_t)c_ctx->websocket_message_[i + 6] << 8 & 0x000000000000ff00)
                  + ((uint64_t)c_ctx->websocket_message_[i + 7] & 0x00000000000000ff);
    i += 8;
  }
  if (mask == 1)
  {
    mask_key[0] = c_ctx->websocket_message_[i];
    mask_key[1] = c_ctx->websocket_message_[i + 1];
    mask_key[2] = c_ctx->websocket_message_[i + 2];
    mask_key[3] = c_ctx->websocket_message_[i + 3];
    i += 4;
  }
  masked_payload_data = c_ctx->websocket_message_.substr(i);
  for (uint64_t j = 0; j < payload_len; j++)
  {
    unmasked_payload_data += masked_payload_data[j] ^ (mask_key[(j % 4)]);
  }
  //echo functionality for now
  c_ctx->websocket_data_ = std::move(unmasked_payload_data);
  std::future<void> event = std::async(std::launch::deferred, &i_response_context::do_create_response, application_context_->ws_res_.get(), c_ctx);
  application_context_->sm_->produce_event(std::move(event));
}
