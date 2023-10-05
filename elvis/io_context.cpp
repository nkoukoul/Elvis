//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "io_context.h"
#include "app_context.h"
#include "request_context.h"
#include <arpa/inet.h>
#include <bitset>
#include <csignal>
#include <iostream>
#include <future>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace Elvis;

inline void non_block_socket(int sd)
{
  /* set O_NONBLOCK on fd */
  int flags = fcntl(sd, F_GETFL, 0);
  if (flags == -1)
  {
    std::cout << "Socket flags set failed\n";
    std::exit(EXIT_FAILURE);
  }
  if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1)
  {
    std::cout << "Socket set to NonBlock failed\n";
    std::exit(EXIT_FAILURE);
  }
}

inline void non_block_with_timeout(int sd)
{
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100;

  if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
  {
    std::cout << "Socket set timeout failed\n";
    std::exit(EXIT_FAILURE);
  }
}

TCPContext::TCPContext(std::string ipaddr, int port,
                       std::shared_ptr<IQueue> concurrentQueue,
                       std::shared_ptr<ILogger> logger,
                       std::shared_ptr<IConnectionMonitor> connectionMonitor)
    : ipaddr_(ipaddr), port_(port), m_ConcurrentQueue{concurrentQueue}, m_Logger{logger}, m_ConnectionMonitor{connectionMonitor}
{
  struct sockaddr_in server;
  server_sock_ = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
#ifdef __APPLE__
  non_block_socket(server_sock_);
#else
  non_block_with_timeout(server_sock_);
#endif
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ipaddr_.c_str());
  server.sin_port = htons(port_);

  if (bind(server_sock_, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    m_Logger->Log(LogLevel::ERROR, "Server failed to bind socket, terminating application…");
    exit(1);
  }

  listen(server_sock_, 5);
}

void TCPContext::SetHTTPInputDelegate(std::weak_ptr<InputContextDelegate> inputDelegate)
{
  m_HTTPInputDelegate = inputDelegate;
}

void TCPContext::Run()
{
  auto weakSelf = weak_from_this();
  m_ConcurrentQueue->DispatchAsync([weakSelf]()
  {
    auto self = weakSelf.lock();
    if (self)
    {
      self->HandleConnections();
    }
  },
  "TCPContext::Run -> TCPContext::HandleConnections");
}

void TCPContext::HandleConnections()
{
  struct sockaddr_in client;
  socklen_t client_len;
  client_len = sizeof client;
  int client_socket =
      accept(server_sock_, (struct sockaddr *)&client, &client_len);
  if (client_socket > 0)
  {
    m_Logger->Log(LogLevel::INFO, "Incoming connection on socket descriptor " + std::to_string(client_socket));
    non_block_with_timeout(client_socket);
    std::shared_ptr<ClientContext> c_ctx = std::make_shared<ClientContext>();
    c_ctx->m_ClientSocket = client_socket;
    c_ctx->m_State = SocketState::CONNECTED;
    m_ConnectionMonitor->AddConnection(c_ctx);
    auto weakSelf = weak_from_this();
    m_ConcurrentQueue->DispatchAsync([weakSelf, c_ctx]()
    {
      auto self = weakSelf.lock();
      if (self)
      {
        self->DoRead(c_ctx);
      }
    },
    "TCPContext::HandleConnections -> TCPContext::DoRead");
  }
  if (errno == EAGAIN || errno == EWOULDBLOCK)
  {
    m_Logger->Log(LogLevel::DETAIL, "No incoming connections.");
  }
  else
  {
    m_Logger->Log(LogLevel::ERROR, "Error while accepting connection.");
  }
  auto weakSelf = weak_from_this();
  m_ConcurrentQueue->DispatchAsync([weakSelf]()
  {
    auto self = weakSelf.lock();
    if (self)
    {
      self->HandleConnections();
    }
  },
  "TCPContext::HandleConnections -> TCPContext::HandleConnections");  
}

void TCPContext::DoRead(std::shared_ptr<ClientContext> c_ctx)
{
  c_ctx->m_State = SocketState::READING;
  char inbuffer[MAXBUF], *p = inbuffer;
  // Read data from client
  int bytes_read = read(c_ctx->m_ClientSocket, inbuffer, MAXBUF);
  m_Logger->Log(LogLevel::DETAIL,
                "TCPContext::DoRead: Incoming Socket byte count " + std::to_string(bytes_read) + " on socket " + std::to_string(c_ctx->m_ClientSocket));
  // Client closed connection
  if (bytes_read == 0)
  {
    m_Logger->Log(LogLevel::DETAIL, "TCPContext::DoRead: No bytes closing socket " + std::to_string(c_ctx->m_ClientSocket));
    close(c_ctx->m_ClientSocket);
    m_ConnectionMonitor->RemoveConnection(c_ctx);
    return;
  }
  if (bytes_read < 0)
  {
    // Client read block
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      // if is websocket and has data proceed to parsing
      if (c_ctx->m_IsWebsocketConnection && c_ctx->m_WSMessage.size())
      {
        m_Logger->Log(LogLevel::DETAIL, "TCPContext::DoRead: Websocket Read Finished with data.");
        m_ConcurrentQueue->DispatchAsync([c_ctx]()
        {
          auto app = App::GetInstance();
          app->m_WSRequestContext->DoParse(c_ctx);
        },
        "TCPContext::DoRead -> WSRequestContext::DoParse");
        return;
      } // if is http and has data proceed to parsing
      else if (!c_ctx->m_IsWebsocketConnection && c_ctx->m_HttpMessage.size())
      {
        m_Logger->Log(LogLevel::DETAIL, "TCPContext::DoRead: HTTP Read Finished with data");
        c_ctx->m_HttpMessage += '\n'; // add end of line for getline
        auto inputDelegate = m_HTTPInputDelegate.lock();
        if (inputDelegate)
        {
          inputDelegate->DidRead(c_ctx);
        }
        return;
      } // read simply blocked with no data try again
      else
      {
        m_Logger->Log(LogLevel::DETAIL, "TCPContext::DoRead: Read Blocked will try again");
        auto weakSelf = weak_from_this();
        m_ConcurrentQueue->DispatchAsync([weakSelf, c_ctx]()
        {
          auto self = weakSelf.lock();
          if (self)
          {
            self->DoRead(c_ctx);
          }
        },
        "TCPContext::DoRead -> TCPContext::DoRead");
      }
      return;
    }
    else
    {
      // TCP read error
      m_Logger->Log(LogLevel::ERROR, "TCPContext::DoRead: Read Error, closing socket " + std::to_string(c_ctx->m_ClientSocket));
      close(c_ctx->m_ClientSocket);
      m_ConnectionMonitor->RemoveConnection(c_ctx);
      return;
    }
  }
  else // We have data to read
  {
    if (c_ctx->m_IsWebsocketConnection)
    {
      m_Logger->Log(LogLevel::INFO,
                    "TCPContext::DoRead: Websocket " + std::to_string(c_ctx->m_ClientSocket) + " has " + std::to_string(bytes_read) + " bytes to read");
      for (int i = 0; i < bytes_read; i++)
      {
        c_ctx->m_WSMessage += inbuffer[i];
      }
    }
    else
    {
      m_Logger->Log(LogLevel::INFO,
                    "TCPContext::DoRead: HTTP socket " + std::to_string(c_ctx->m_ClientSocket) + " has " + std::to_string(bytes_read) + " bytes to read");
      for (int i = 0; i < bytes_read; i++)
      {
        c_ctx->m_HttpMessage += inbuffer[i];
      }
    }
    // maybe there some more data so add another read to the queue
    auto weakSelf = weak_from_this();
    m_ConcurrentQueue->DispatchAsync([weakSelf, c_ctx]()
    {
      auto self = weakSelf.lock();
      if (self)
      {
        self->DoRead(c_ctx);
      }
    },
    "TCPContext::DoRead -> TCPContext::DoRead");
    
  }
}

void TCPContext::DoWrite(std::shared_ptr<ClientContext> c_ctx)
{
  c_ctx->m_State = SocketState::WRITING;
  int bytes_write;
  if (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted)
  {
    m_Logger->Log(LogLevel::DETAIL,
                  "TCPContext::DoWrite: WS Response " + c_ctx->m_WSResponse + "\n with size " + std::to_string(c_ctx->m_WSResponse.size()));
    size_t bytes_to_send;
    if (MAXBUF > (c_ctx->m_WSResponse.size() - c_ctx->m_WSBytesSend))
    {
      bytes_to_send = c_ctx->m_WSResponse.size() - c_ctx->m_WSBytesSend;
    }
    else
    {
      bytes_to_send = MAXBUF;
    }
    bytes_write = write(c_ctx->m_ClientSocket,
                        c_ctx->m_WSResponse.c_str() + c_ctx->m_WSBytesSend,
                        bytes_to_send);
    m_Logger->Log(LogLevel::INFO,
                  "TCPContext::DoWrite: WS Response wrote " + std::to_string(bytes_write) + " bytes on socket " + std::to_string(c_ctx->m_ClientSocket));
  }
  else
  {
    m_Logger->Log(LogLevel::DETAIL,
                  "TCPContext::DoWrite: HTTP Response " + c_ctx->m_HttpResponse + "\n with size " + std::to_string(c_ctx->m_HttpResponse.size()));
    size_t bytes_to_send;
    if (MAXBUF > (c_ctx->m_HttpResponse.size() - c_ctx->m_HttpBytesSend))
    {
      bytes_to_send = c_ctx->m_HttpResponse.size() - c_ctx->m_HttpBytesSend;
    }
    else
    {
      bytes_to_send = MAXBUF;
    }
    bytes_write = write(c_ctx->m_ClientSocket,
                        c_ctx->m_HttpResponse.c_str() + c_ctx->m_HttpBytesSend,
                        bytes_to_send);
    m_Logger->Log(LogLevel::INFO,
                  "TCPContext::DoWrite: HTTP Response wrote " + std::to_string(bytes_write) + " bytes on socket " + std::to_string(c_ctx->m_ClientSocket));
  }

  // Client closed connection
  if (bytes_write == 0)
  {
    m_Logger->Log(LogLevel::INFO,
                  "TCPContext::DoWrite: No bytes written, Client closed connection, closing connection on socket " + std::to_string(c_ctx->m_ClientSocket));
    close(c_ctx->m_ClientSocket);
    m_ConnectionMonitor->RemoveConnection(c_ctx);
    return;
  }

  // Error handling
  if (bytes_write < 0)
  { // Write block try again
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      m_Logger->Log(LogLevel::DETAIL, "TCPContext::DoWrite: Write block will try again on socket " + std::to_string(c_ctx->m_ClientSocket));
      auto weakSelf = weak_from_this();
      m_ConcurrentQueue->DispatchAsync([weakSelf, c_ctx]()
      {
        auto self = weakSelf.lock();
        if (self)
        {
          self->DoWrite(c_ctx);
        }
      },
      "TCPContext::DoWrite -> TCPContext::DoWrite");
    }
    else
    {
      // TCP write error
      m_Logger->Log(LogLevel::ERROR, "TCPContext::DoWrite: Write error closing connection on socket " + std::to_string(c_ctx->m_ClientSocket));
      close(c_ctx->m_ClientSocket);
      m_ConnectionMonitor->RemoveConnection(c_ctx);
    }
    return;
  }

  // Check if we need to transfer more data
  if (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted)
  {
    c_ctx->m_WSBytesSend += bytes_write;
  }
  else
  {
    c_ctx->m_HttpBytesSend += bytes_write;
  }

  if ((!c_ctx->m_IsWebsocketConnection &&
       c_ctx->m_HttpBytesSend < c_ctx->m_HttpResponse.size()) ||
      (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted &&
       c_ctx->m_WSBytesSend < c_ctx->m_WSResponse.size()))
  {
    m_Logger->Log(LogLevel::INFO, "TCPContext::DoWrite: There is more data to write on socket " + std::to_string(c_ctx->m_ClientSocket));
    auto weakSelf = weak_from_this();
    m_ConcurrentQueue->DispatchAsync([weakSelf, c_ctx]()
    {
      auto self = weakSelf.lock();
      if (self)
      {
        self->DoWrite(c_ctx);
      }
    },
    "TCPContext::DoWrite -> TCPContext::DoWrite");
    return;
  }
  // Check if socket should close
  if (c_ctx->m_ShouldCloseConnection)
  {
    m_Logger->Log(LogLevel::INFO, "TCPContext::DoWrite: Should close socket " + std::to_string(c_ctx->m_ClientSocket));
    close(c_ctx->m_ClientSocket);
    m_ConnectionMonitor->RemoveConnection(c_ctx);
    return;
  }

  // if socket is still open with http keep-alive or ws read again
  if (c_ctx->m_IsWebsocketConnection)
  {
    m_Logger->Log(LogLevel::INFO, "TCPContext::DoWrite: WS connection on socket " + std::to_string(c_ctx->m_ClientSocket) + " will schedule another read");
    if (!c_ctx->m_IsHandshakeCompleted)
    {
      // Add socket to broadcast list, disabled atm.
      // ac_->broadcast_fd_list.push_back(c_ctx->m_ClientSocket);
      c_ctx->m_IsHandshakeCompleted = true;
    }
    c_ctx->m_WSMessage.clear();
    c_ctx->m_WSData.clear();
    c_ctx->m_WSResponse.clear();
    c_ctx->m_WSBytesSend = 0;
  }
  else
  {
    m_Logger->Log(LogLevel::INFO,
                  "TCPContext::DoWrite: HTTP with keepAlive connection on socket " + std::to_string(c_ctx->m_ClientSocket) + " will schedule another read");
    c_ctx->m_HttpMessage.clear();
    c_ctx->m_HttpBytesSend = 0;
  }
  auto weakSelf = weak_from_this();
  m_ConcurrentQueue->DispatchAsync([weakSelf, c_ctx]()
  {
    auto self = weakSelf.lock();
    if (self)
    {
      self->DoRead(c_ctx);
    }
  },
  "TCPContext::DoWrite -> TCPContext::DoRead");
  return;
}
