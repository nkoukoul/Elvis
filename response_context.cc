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

void http_response_creator::create_response(std::shared_ptr<client_context> c_ctx) const
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
    if (c_ctx->http_headers_["status"] == "400 Bad Request" ||
        c_ctx->http_headers_.find("Connection") == c_ctx->http_headers_.end() ||
        c_ctx->http_headers_["Connection"] == "close")
    {
      c_ctx->close_connection_ = true;
    }
    else
    { // keep connection open for more
      c_ctx->close_connection_ = false;
    }
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
  else if (c_ctx->close_connection_)
  {
    c_ctx->http_response_ += "Connection: close\r\n";
  }
  else
  {
    c_ctx->http_response_ += "Connection: keep-alive\r\n";
    c_ctx->http_response_ += "Keep-Alive: timeout=2, max=1000\r\n";
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
  auto executor = application_context_->sm_->access_strand<event_queue<std::function<void()>>>();
  executor->produce_event(
      std::move(
          std::bind(
              &io_context::do_write,
              application_context_->ioc_.get(),
              c_ctx)));
}

websocket_response_creator::websocket_response_creator(app *application_context) : application_context_(application_context) {}

void websocket_response_creator::create_response(std::shared_ptr<client_context> c_ctx) const
{
  int payload_len;
  bool close_connection = false;
  if (c_ctx->close_connection_)
  {
    c_ctx->websocket_response_ += 0x88; //fin = 1, rsv1,2,3 = 0 text frame opcode = 8
  }
  else
  {
    c_ctx->websocket_response_ += 0x81; //fin = 1, rsv1,2,3 = 0 text frame opcode = 1
  }

  if (c_ctx->websocket_data_.size() <= 125)
  {
    payload_len = c_ctx->websocket_data_.size();
    c_ctx->websocket_response_ += (payload_len) & 0x7F;
  }
  else if (c_ctx->websocket_data_.size() <= 65535)
  {
    payload_len = 126;
    c_ctx->websocket_response_ += (payload_len) & 0x7F;
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size() >> 8);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size());
  }
  else
  {
    payload_len = 127;
    c_ctx->websocket_response_ += (payload_len) & 0x7F;
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size() >> 54);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size() >> 48);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size() >> 40);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size() >> 32);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size() >> 24);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size() >> 16);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size() >> 8);
    c_ctx->websocket_response_ += (unsigned char)(c_ctx->websocket_data_.size());
  }
  c_ctx->websocket_response_ += c_ctx->websocket_data_;
  auto executor = application_context_->sm_->access_strand<event_queue<std::function<void()>>>();
  executor->produce_event(
      std::move(
          std::bind(
              &io_context::do_write,
              application_context_->ioc_.get(),
              c_ctx)));
}
