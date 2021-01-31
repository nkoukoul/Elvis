//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "response_context.h"
#include "app_context.h"
#include "io_context.h"

http_response_creator::http_response_creator(app *application_context) : application_context_(application_context) {}

void http_response_creator::create_response(
    std::shared_ptr<client_context> c_ctx,
    std::shared_ptr<i_event_queue> executor) const
{
  std::string status;
  std::string controller_data;
  std::string sec_websocket_key;
  bool close_connection;

  if (c_ctx->http_headers_.find("Connection") != c_ctx->http_headers_.end() && 
      c_ctx->http_headers_["Connection"] == "Upgrade" && 
      c_ctx->http_headers_.find("Upgrade") != c_ctx->http_headers_.end() && 
      c_ctx->http_headers_["Upgrade"] == "websocket")
  {
    //wsconnection
    status = "101 Switching Protocols";
    sec_websocket_key = application_context_->uc_->generate_ws_key(c_ctx->http_headers_["Sec-WebSocket-Key"]);
    c_ctx->close_connection_ = false;
    c_ctx->is_websocket_ = true;
    c_ctx->handshake_completed_ = false;
  }
  else
  {
    //http connection
    status = c_ctx->http_headers_["status"];
    controller_data = c_ctx->http_headers_["controller_data"];
    c_ctx->close_connection_ = true;
  }

  c_ctx->http_response_.reserve(controller_data.size() + 1024);
  c_ctx->http_response_ += "HTTP/1.1 " + status + "\r\n";
  c_ctx->http_response_ += "Date: " + application_context_->uc_->daytime_() + "\r\n";
  if (c_ctx->is_websocket_)
  {
    c_ctx->http_response_ += "Upgrade: websocket\r\n";
    c_ctx->http_response_ += "Connection: Upgrade\r\n";
    c_ctx->http_response_ += "Sec-WebSocket-Accept: " + sec_websocket_key + "\r\n";
  }
  else
  {
    c_ctx->http_response_ += "Connection: close\r\n";
  }

  if (!controller_data.empty())
  {
    c_ctx->http_response_ += "Content-Type: text/html\r\n";
    c_ctx->http_response_ += "Content-Length: " + std::to_string(controller_data.size() + 2) + "\r\n";
  }

  c_ctx->http_response_ += "\r\n";
  if (!controller_data.empty())
  {
    c_ctx->http_response_ += controller_data + "\r\n";
  }
  executor->produce_event<std::function<void()>>(
      std::move(
          std::bind(
              &io_context::do_write,
              application_context_->ioc_.get(),
              c_ctx,
              executor)));
}

websocket_response_creator::websocket_response_creator(app *application_context) : application_context_(application_context) {}

void websocket_response_creator::create_response(
    std::shared_ptr<client_context> c_ctx,
    std::shared_ptr<i_event_queue> executor) const
{
  int payload_len;
  bool close_connection = false;
  if (c_ctx->websocket_data_["Connection"] == "close")
  {
    c_ctx->websocket_response_ += 0x88; //fin = 1, rsv1,2,3 = 0 text frame opcode = 8
    c_ctx->close_connection_ = true;
  }
  else
  {
    c_ctx->websocket_response_ += 0x81; //fin = 1, rsv1,2,3 = 0 text frame opcode = 1
    c_ctx->close_connection_ = false;
  }

  if (c_ctx->websocket_data_["data"].size() <= 125)
  {
    payload_len = c_ctx->websocket_data_["data"].size();
    c_ctx->websocket_response_ += (payload_len) & 0x7F;
  }
  else if (c_ctx->websocket_data_["data"].size() <= 65535)
  {
    payload_len = 126;
    c_ctx->websocket_response_ += (payload_len) & 0x7F;
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size());
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size() >> 8);
  }
  else
  {
    payload_len = 127;
    c_ctx->websocket_response_ += (payload_len) & 0x7F;
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size());
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size() >> 8);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size() >> 16);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size() >> 24);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size() >> 32);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size() >> 40);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size() >> 48);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_["data"].size() >> 54);
  }
  c_ctx->websocket_response_ += c_ctx->websocket_data_["data"];

  executor->produce_event<std::function<void()>>(
      std::move(
          std::bind(
              &io_context::do_write,
              application_context_->ioc_.get(),
              c_ctx,
              executor)));
}
