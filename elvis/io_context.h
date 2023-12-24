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
#include "context_delegate.h"
#include "crypto_manager.h"
#include "logger.h"
#include "monitor.h"
#include "queue.h"
#include "route_manager.h"
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace Elvis
{
struct IServer
{
    virtual ~IServer() = default;
    virtual void Run() = 0;
    virtual void HandleConnections() = 0;
};

struct InputContext
{
    virtual ~InputContext() = default;
    virtual void DoRead(std::shared_ptr<ClientContext> c_ctx) = 0;
};

struct OutputContext
{
    virtual ~OutputContext() = default;
    virtual void DoWrite(std::shared_ptr<ClientContext> c_ctx) = 0;
    virtual void SetTCPInputDelegate(std::weak_ptr<InputContextDelegate> inputDelegate) = 0;
};

std::shared_ptr<IServer> CreateTCPServer(
    std::string ipAddress,
    int port,
    std::shared_ptr<RouteManager> routeManager,
    std::shared_ptr<ICryptoManager> cryptoManager,
    std::shared_ptr<IQueue> concurrentQueue,
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IConnectionMonitor> connectionMonitor);
} // namespace Elvis
#endif // IO_CONTEXT_H
