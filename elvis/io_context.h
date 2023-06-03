//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include "client_context.h"
#include "queue.h"
#include "logger.h"
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace Elvis
{
  class IOContext
  {
  public:
    virtual ~IOContext() = default;

    virtual void Run() = 0;

    virtual void HandleConnections() = 0;

    virtual void DoRead(std::shared_ptr<ClientContext> c_ctx) = 0;

    virtual void DoWrite(std::shared_ptr<ClientContext> c_ctx) = 0;
  };

  class TCPContext final : public IOContext
  {
  public:
    TCPContext(std::string ipaddr, int port,
               std::shared_ptr<Elvis::IQueue> concurrentQueue, std::shared_ptr<Elvis::ILogger> logger);

    void Run() override;

    void HandleConnections() override;

    void DoRead(std::shared_ptr<ClientContext> c_ctx) override;

    void DoWrite(std::shared_ptr<ClientContext> c_ctx) override;

  private:
    std::shared_ptr<Elvis::IQueue> m_ConcurrentQueue;
    std::shared_ptr<Elvis::ILogger> m_Logger;
    std::string ipaddr_;
    int port_;
    int server_sock_;
    static const int MAXBUF = 1024;
  };
} // namespace Elvis
#endif // IO_CONTEXT_H
