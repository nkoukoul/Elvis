//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef QUEUE_H
#define QUEUE_H

#include <vector>
#include <list>
#include <future>
#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include <algorithm>

namespace Elvis
{
  class IQueue
  {
  public:
    virtual ~IQueue() = default;

    virtual std::future<void> RunTask() = 0;

    virtual void CreateTask(std::future<void> event, std::string taskType) = 0;
  };

  class AsyncQueue final : public IQueue
  {
  public:
    AsyncQueue(int const capacity) : m_Capacity(capacity) {}

    std::future<void> RunTask() override
    {
      
      std::lock_guard<std::mutex> lock(m_AsyncQueueLock);
      if (!empty())
      {
        auto pair = std::move(m_AsyncQueue.front());
#ifdef DEBUG
        std::cout << "AsyncQueue::RunTask " << pair.second << "\n";
#endif
        m_AsyncQueue.pop_front();
        return std::move(pair.first);
      }
      std::future<void> event;
      return event;
    }

    void CreateTask(std::future<void> event, std::string taskType) override
    {
      std::lock_guard<std::mutex> lock(m_AsyncQueueLock);
#ifdef DEBUG
      std::cout << "AsyncQueue::CreateTask " << taskType << "\n";
#endif
      m_AsyncQueue.emplace_back(std::make_pair(std::move(event), taskType));
      return;
    }

    bool empty() const { return m_AsyncQueue.empty(); }

  private:
    std::mutex m_AsyncQueueLock;
    std::list<std::pair<std::future<void>, std::string >> m_AsyncQueue;
    const int m_Capacity;
  };
}
#endif // QUEUE_H
