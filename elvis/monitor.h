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
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace Elvis
{
class IConnectionMonitor
{
public:
    virtual ~IConnectionMonitor() = default;

    virtual void Run() = 0;
    virtual void AddConnection(std::shared_ptr<ClientContext> connection) = 0;
    virtual void RemoveConnection(std::shared_ptr<ClientContext> connection) = 0;
    virtual void DisplayOpenConnections() const = 0;
};

class ConnectionMonitor final : public IConnectionMonitor
{
public:
    ConnectionMonitor(size_t intervalInSeconds);
    ConnectionMonitor() = delete;
    ~ConnectionMonitor();

    virtual void Run() override;
    virtual void AddConnection(std::shared_ptr<ClientContext> connection) override;
    virtual void RemoveConnection(std::shared_ptr<ClientContext> connection) override;
    virtual void DisplayOpenConnections() const override;

private:
    std::unordered_set<std::shared_ptr<ClientContext>> m_ActiveConnections;
    mutable std::mutex m_ConnectionMonitorLock;
    std::thread m_Thread;
    size_t m_Interval;
    std::condition_variable m_CV;
    bool m_Quit;
};

class MockConnectionMonitor final : public IConnectionMonitor
{
public:
    MockConnectionMonitor() = default;
    ~MockConnectionMonitor() = default;

    virtual void Run() override
    {
    }

    virtual void AddConnection(std::shared_ptr<ClientContext> connection) override
    {
    }

    virtual void RemoveConnection(std::shared_ptr<ClientContext> connection) override
    {
    }

    virtual void DisplayOpenConnections() const override
    {
    }
};
} // namespace Elvis

#endif // MONITOR_H