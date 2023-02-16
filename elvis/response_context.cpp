//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "response_context.h"
#include "app_context.h"
#include "io_context.h"

Elvis::HttpResponseCreator::HttpResponseCreator(app *application_context) : application_context_(application_context) {}

void Elvis::HttpResponseCreator::CreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const
{
  std::string status;
  std::string controller_data;
  std::string sec_websocket_key;
  bool close_connection;
#ifdef DEBUG
  std::cout << "HttpResponseCreator::CreateResponse\n";
#endif
  if (c_ctx->m_HttpHeaders.find("Connection") != c_ctx->m_HttpHeaders.end() &&
      c_ctx->m_HttpHeaders["Connection"] == "Upgrade" &&
      c_ctx->m_HttpHeaders.find("Upgrade") != c_ctx->m_HttpHeaders.end() &&
      c_ctx->m_HttpHeaders["Upgrade"] == "websocket")
  {
    // wsconnection
    status = "101 Switching Protocols";
    sec_websocket_key = generate_ws_key(c_ctx->m_HttpHeaders["Sec-WebSocket-Key"], application_context_->cryptoManager);
    c_ctx->m_ShouldCloseConnection = false;
    c_ctx->m_IsWebsocketConnection = true;
    c_ctx->m_IsHandshakeCompleted = false;
  }
  else
  {
    // http connection
    status = c_ctx->m_HttpHeaders["status"];
    controller_data = c_ctx->m_HttpHeaders["controller_data"];
    if (c_ctx->m_HttpHeaders["status"] == "400 Bad Request" ||
        c_ctx->m_HttpHeaders.find("Connection") == c_ctx->m_HttpHeaders.end() ||
        c_ctx->m_HttpHeaders["Connection"] == "close")
    {
      c_ctx->m_ShouldCloseConnection = true;
    }
    else
    { // keep connection open for more
      c_ctx->m_ShouldCloseConnection = false;
    }
  }

  c_ctx->m_HttpResponse.reserve(controller_data.size() + 1024);
  c_ctx->m_HttpResponse += "HTTP/1.1 " + status + "\r\n";
  c_ctx->m_HttpResponse += "Date: " + daytime_() + "\r\n";
  if (c_ctx->m_IsWebsocketConnection)
  {
    c_ctx->m_HttpResponse += "Upgrade: websocket\r\n";
    c_ctx->m_HttpResponse += "Connection: Upgrade\r\n";
    c_ctx->m_HttpResponse += "Sec-WebSocket-Accept: " + sec_websocket_key + "\r\n";
  }
  else if (c_ctx->m_ShouldCloseConnection)
  {
    c_ctx->m_HttpResponse += "Connection: close\r\n";
  }
  else
  {
    c_ctx->m_HttpResponse += "Connection: keep-alive\r\n";
    c_ctx->m_HttpResponse += "Keep-Alive: timeout=2, max=1000\r\n";
  }

  if (!controller_data.empty())
  {
    c_ctx->m_HttpResponse += "Content-Type: text/html\r\n";
    c_ctx->m_HttpResponse += "Content-Length: " + std::to_string(controller_data.size() + 2) + "\r\n";
  }

  c_ctx->m_HttpResponse += "\r\n";
  if (!controller_data.empty())
  {
    c_ctx->m_HttpResponse += controller_data + "\r\n";
  }

  c_ctx->m_HttpBytesSend = 0;
  std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::DoWrite, application_context_->ioc_.get(), c_ctx);
  application_context_->m_AsyncQueue->CreateTask(std::move(event), "HttpResponseCreator::CreateResponse -> IOContext::DoWrite");
}

Elvis::WebsocketResponseCreator::WebsocketResponseCreator(app *application_context) : application_context_(application_context) {}

void Elvis::WebsocketResponseCreator::CreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const
{
  int payload_len;
  bool close_connection = false;
  if (c_ctx->m_ShouldCloseConnection)
  {
    c_ctx->m_WSResponse += 0x88; // fin = 1, rsv1,2,3 = 0 text frame opcode = 8
  }
  else
  {
    c_ctx->m_WSResponse += 0x81; // fin = 1, rsv1,2,3 = 0 text frame opcode = 1
  }

  if (c_ctx->m_WSData.size() <= 125)
  {
    payload_len = c_ctx->m_WSData.size();
    c_ctx->m_WSResponse += (payload_len)&0x7F;
  }
  else if (c_ctx->m_WSData.size() <= 65535)
  {
    payload_len = 126;
    c_ctx->m_WSResponse += (payload_len)&0x7F;
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size() >> 8);
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size());
  }
  else
  {
    payload_len = 127;
    c_ctx->m_WSResponse += (payload_len)&0x7F;
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size() >> 54);
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size() >> 48);
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size() >> 40);
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size() >> 32);
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size() >> 24);
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size() >> 16);
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size() >> 8);
    c_ctx->m_WSResponse += (unsigned char)(c_ctx->m_WSData.size());
  }
  c_ctx->m_WSResponse += c_ctx->m_WSData;
  std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::DoWrite, application_context_->ioc_.get(), c_ctx);
  application_context_->m_AsyncQueue->CreateTask(std::move(event), "");
}
