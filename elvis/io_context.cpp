//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "io_context.h"
#include <arpa/inet.h>
#include <bitset>
#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void non_block_socket(int sd)
{
  /* set O_NONBLOCK on fd */
  int flags = fcntl(sd, F_GETFL, 0);
  if (flags == -1)
  {
    // perror("fcntl()");
    return;
  }
  if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1)
  {
    // perror("fcntl()");
  }
}

void non_block_with_timeout(int sd)
{
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100;

  if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0)
    std::cout << "sock timeout failed\n";
}

Elvis::TCPContext::TCPContext(
    std::string ipaddr, int port,
    std::unique_ptr<Elvis::HttpRequestContext> httpRequestContext,
    std::unique_ptr<Elvis::WebsocketRequestContext> wsRequestContext,
    std::shared_ptr<Elvis::IQueue> concurrentQueue)
    : ipaddr_(ipaddr), port_(port),
      m_HTTPRequestContext(std::move(httpRequestContext)),
      m_WSRequestContext(std::move(wsRequestContext))
{
  m_ConcurrentQueue = concurrentQueue;
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
    std::cout << "bind failed\n";
    exit(1);
  }

  listen(server_sock_, 5);
}

void Elvis::TCPContext::Run()
{
  // std::future<void> task = std::async(std::launch::deferred,
  // &Elvis::IOContext::HandleConnections, this);
  m_ConcurrentQueue->CreateTask(
      std::move(std::async(std::launch::deferred,
                           &Elvis::IOContext::HandleConnections, this)),
      "TCPContext::Run -> IOContext::HandleConnections");
  while (true)
  {
    // std::cout << "Looping\n";
    std::future<void> task;
    auto hasTask = m_ConcurrentQueue->RunTask(task);
    if (hasTask)
    {
      task.wait();
    }
    else
    {
      std::cout << "No task\n";
    }
  }
}

void Elvis::TCPContext::HandleConnections()
{
  struct sockaddr_in client;
  socklen_t client_len;
  client_len = sizeof client;
  int client_socket =
      accept(server_sock_, (struct sockaddr *)&client, &client_len);
  if (client_socket <= 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      ;
      ; // no incoming connection for non-blocking sockets
    }
    else
    {
      std::cout << "Error while accepting connection\n";
    }
  }
  else
  {
#ifdef DEBUG
    std::cout << "Incoming Connection\n";
#endif
    non_block_with_timeout(client_socket);
    std::shared_ptr<ClientContext> c_ctx = std::make_shared<ClientContext>();
    c_ctx->m_ClientSocket = client_socket;
    std::future<void> task = std::async(std::launch::deferred,
                                        &Elvis::IOContext::DoRead, this, c_ctx);
    m_ConcurrentQueue->CreateTask(
        std::move(task), "TCPContext::HandleConnections -> IOContext::DoRead");
  }
  std::future<void> task = std::async(
      std::launch::deferred, &Elvis::IOContext::HandleConnections, this);
  m_ConcurrentQueue->CreateTask(
      std::move(task),
      "TCPContext::HandleConnections -> IOContext::HandleConnections");
}

void Elvis::TCPContext::DoRead(std::shared_ptr<ClientContext> c_ctx)
{
  char inbuffer[MAXBUF], *p = inbuffer;
  // Read data from client
  int bytes_read = read(c_ctx->m_ClientSocket, inbuffer, MAXBUF);
#ifdef DEBUG
  std::cout << "TCPContext::DoRead: Incoming Socket byte count " << bytes_read
            << "\n";
#endif
  // Client closed connection
  if (bytes_read == 0)
  {
#ifdef DEBUG
    std::cout << "TCPContext::DoRead: No bytes closing socket " << bytes_read
              << "\n";
#endif
    close(c_ctx->m_ClientSocket);
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
        std::cout << "TCPContext::DoRead: Websocket Read Finished\n";
        std::future<void> task =
            std::async(std::launch::deferred, &IRequestContext::DoParse,
                       m_WSRequestContext.get(), c_ctx);
        m_ConcurrentQueue->CreateTask(
            std::move(task), "TCPContext::DoRead -> IRequestContext::DoParse");
      } // if is http and has data proceed to parsing
      else if (!c_ctx->m_IsWebsocketConnection && c_ctx->m_HttpMessage.size())
      {
#ifdef DEBUG
        std::cout << "TCPContext::DoRead: HTTP Read Finished\n";
#endif
        c_ctx->m_HttpMessage += '\n'; // add end of line for getline
        std::future<void> task =
            std::async(std::launch::deferred, &IRequestContext::DoParse,
                       m_HTTPRequestContext.get(), c_ctx);
        m_ConcurrentQueue->CreateTask(
            std::move(task), "TCPContext::DoRead -> IRequestContext::DoParse");
      } // read simply blocked with no data try again
      else
      {
#ifdef DEBUG
        std::cout << "TCPContext::DoRead: Read Blocked will try again\n";
#endif
        std::future<void> task = std::async(
            std::launch::deferred, &Elvis::IOContext::DoRead, this, c_ctx);
        m_ConcurrentQueue->CreateTask(
            std::move(task), "TCPContext::DoRead -> IOContext::DoRead");
      }
      return;
    }
    else
    {
      // TCP read error
      std::cout << "TCPContext::DoRead: Read Error\n";
      close(c_ctx->m_ClientSocket);
      return;
    }
  }
  else // We have data to read
  {
    if (c_ctx->m_IsWebsocketConnection)
    {
#ifdef DEBUG
      std::cout << "TCPContext::DoRead: We have ws data to Read\n";
#endif
      for (int i = 0; i < bytes_read; i++)
      {
        c_ctx->m_WSMessage += inbuffer[i];
      }
    }
    else
    {
#ifdef DEBUG
      std::cout << "TCPContext::DoRead: We have http to Read\n";
#endif
      for (int i = 0; i < bytes_read; i++)
      {
        c_ctx->m_HttpMessage += inbuffer[i];
      }
    }
    // maybe there some more data so add another read to the queue
    std::future<void> task = std::async(std::launch::deferred,
                                        &Elvis::IOContext::DoRead, this, c_ctx);
    m_ConcurrentQueue->CreateTask(std::move(task),
                                  "TCPContext::DoRead -> IOContext::DoRead");
  }
}

void Elvis::TCPContext::DoWrite(std::shared_ptr<ClientContext> c_ctx)
{
  int bytes_write;
  if (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted)
  {
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
  }
  else
  {
    size_t bytes_to_send;
#ifdef DEBUG
    std::cout << "TCPContext::DoWrite: HTTP Response " << c_ctx->m_HttpResponse
              << "\n with size " << c_ctx->m_HttpResponse.size() << "\n";
#endif
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
  }

  // Client closed connection
  if (bytes_write == 0)
  {
#ifdef DEBUG
    std::cout << "TCPContext::DoWrite: No bytes written, Client closed "
                 "connection, closing connection\n";
#endif
    close(c_ctx->m_ClientSocket);
    return;
  }

  // Error handling
  if (bytes_write < 0)
  { // Write block try again
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
#ifdef DEBUG
      std::cout << "TCPContext::DoWrite: Write block will try again\n";
#endif
      std::future<void> task = std::async(
          std::launch::deferred, &Elvis::IOContext::DoWrite, this, c_ctx);
      m_ConcurrentQueue->CreateTask(std::move(task), "");
    }
    else
    {
      // TCP write error
      std::cout << "TCPContext::DoWrite: Write error closing connection\n";
      close(c_ctx->m_ClientSocket);
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
#ifdef DEBUG
    std::cout << "TCPContext::DoWrite: HTTP there is data to write.\n";
#endif
    c_ctx->m_HttpBytesSend += bytes_write;
  }

  if ((!c_ctx->m_IsWebsocketConnection &&
       c_ctx->m_HttpBytesSend < c_ctx->m_HttpResponse.size()) ||
      (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted &&
       c_ctx->m_WSBytesSend < c_ctx->m_WSResponse.size()))
  {
    std::future<void> task = std::async(
        std::launch::deferred, &Elvis::IOContext::DoWrite, this, c_ctx);
    m_ConcurrentQueue->CreateTask(std::move(task), "");
    return;
  }
  // Check if socket should close
  if (c_ctx->m_ShouldCloseConnection)
  {
#ifdef DEBUG
    std::cout << "TCPContext::DoWrite: Socket should close\n";
#endif
    close(c_ctx->m_ClientSocket);
    return;
  }

  // if socket is still open with http keep-alive or ws read again
  if (c_ctx->m_IsWebsocketConnection)
  {
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
    c_ctx->m_HttpMessage.clear();
    c_ctx->m_HttpBytesSend = 0;
  }
  std::future<void> task =
      std::async(std::launch::deferred, &Elvis::IOContext::DoRead, this, c_ctx);
  m_ConcurrentQueue->CreateTask(std::move(task), "");
  return;
}
