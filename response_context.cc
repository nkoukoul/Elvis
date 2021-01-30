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
  bool close_connection = false;
  std::bitset<4> b_opcode;
  if (c_ctx->websocket_data_["Connection"] == "close")
  {
    std::bitset<4> t_opcode(8);
    b_opcode = t_opcode;
    close_connection = true;
  }
  else
  {
    std::bitset<4> t_opcode(1);
    b_opcode = t_opcode;
  }
  unsigned int len = c_ctx->websocket_data_["data"].size();
  std::string extra_len = "";
  std::bitset<16> bs;
  bs[15] = 1;           //fin
  bs[14] = 0;           //RSV1
  bs[13] = 0;           //RSV2
  bs[12] = 0;           //RSV3
  bs[11] = b_opcode[3]; //opcode
  bs[10] = b_opcode[2];
  bs[9] = b_opcode[1];
  bs[8] = b_opcode[0];
  bs[7] = 0; //mask
  if (len <= 125)
  {
    std::bitset<7> b_len(len);
    for (int i = 0; i < 7; i++)
    {
      bs[6 - i] = b_len[6 - i];
    }
  }
  else if (len <= 65535)
  {
    std::bitset<7> b_len(126);
    for (int i = 0; i < 7; i++)
    {
      bs[6 - i] = b_len[6 - i];
    }
    extra_len += (len & 0x0000ff00) >> 8; //MSB
    extra_len += len & 0x000000ff;        //LSB
  }
  else
  {
    std::bitset<7> b_len(127);
    /*for (int i = 0; i < 7; i++){
      bs[6-i] = b_len[6-i];
    }
    extra_len += (len & 0xff00000000000000) >> 256; //8 byte
    extra_len += (len & 0x00ff000000000000) >> 128; //7 byte    
    extra_len += (len & 0x0000ff0000000000) >> 64; //6 byte   
    extra_len += (len & 0x000000ff00000000) >> 32; //5 byte    
    extra_len += (len & 0x00000000ff000000) >> 24; //4 byte    
    extra_len += (len & 0x0000000000ff0000) >> 16; //3 byte    
    extra_len += (len & 0x000000000000ff00) >> 8; //2 byte        
    extra_len += len & 0x00000000000000ff; //1 byte    */
  }
  unsigned int first_part = application_context_->uc_->binary_to_decimal(bs.to_string());
  c_ctx->websocket_response_ += (first_part & 0x0000ff00) >> 8; //MSB
  c_ctx->websocket_response_ += first_part & 0x000000ff;        //LSB
  c_ctx->websocket_response_ += extra_len;
  c_ctx->websocket_response_ += c_ctx->websocket_data_["data"];
  executor->produce_event<std::function<void()>>(
      std::move(
          std::bind(
              &io_context::do_write,
              application_context_->ioc_.get(),
              c_ctx,
              executor)));
}
