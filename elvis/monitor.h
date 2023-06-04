//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef MONITOR_H
#define MONITOR_H

#include "client_context.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>

namespace Elvis
{
    class IConnectionMonitor
    {
    public:
        virtual ~IConnectionMonitor() = default;
        virtual void AddConnection(std::shared_ptr<ClientContext> connection) = 0;
        virtual void RemoveConnection(std::shared_ptr<ClientContext> connection) = 0;

        virtual void DisplayOpenConnections() = 0;
    };

    class ConnectionMonitor final: public IConnectionMonitor
    {
    public:
        ConnectionMonitor();

        virtual void AddConnection(std::shared_ptr<ClientContext> connection) override;
        virtual void RemoveConnection(std::shared_ptr<ClientContext> connection) override;

        virtual void DisplayOpenConnections() override;
    private:
        std::unordered_set<std::shared_ptr<ClientContext>> m_ActiveConnections;
        mutable std::mutex m_ConnectionMonitorLock;
    };
}

#endif // MONITOR_H