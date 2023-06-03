//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef QUEUE_H
#define QUEUE_H

#include <algorithm>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>

namespace Elvis
{
  class IQueue
  {
  public:
    using wlock = std::unique_lock<std::shared_mutex>;
    using rlock = std::shared_lock<std::shared_mutex>;

    virtual ~IQueue() = default;

    virtual bool RunTask(std::future<void> &task) = 0;

    virtual void CreateTask(std::future<void> event, std::string taskType) = 0;
  };

  class ConcurrentQueue final : public IQueue
  {
  public:
    ConcurrentQueue(int const capacity) : m_Capacity(capacity) {}

    virtual bool RunTask(std::future<void> &task) override
    {
      // std::lock_guard<std::mutex> lock(m_ConcurrentQueueLock);
      wlock lock(m_SharedMutex);
      if (!empty())
      {
        auto pair = std::move(m_ConcurrentQueue.front());
#ifdef DEBUG
        std::cout << "ConcurrentQueue::RunTask " << pair.second << "\n";
#endif
        task = std::move(pair.first);
        m_ConcurrentQueue.pop_front();
        return true;
      }
      return false;
    }

    virtual void CreateTask(std::future<void> event,
                            std::string taskType) override
    {
      // std::lock_guard<std::mutex> lock(m_ConcurrentQueueLock);
      wlock lock(m_SharedMutex);
#ifdef DEBUG
      std::cout << "ConcurrentQueue::CreateTask " << taskType << "\n";
#endif
      m_ConcurrentQueue.emplace_back(std::make_pair(std::move(event), taskType));
      return;
    }

    bool empty() const
    {
      // rlock lock(m_SharedMutex);
      return m_ConcurrentQueue.empty();
    }

  private:
    std::mutex m_ConcurrentQueueLock;
    mutable std::shared_mutex m_SharedMutex;
    std::list<std::pair<std::future<void>, std::string>> m_ConcurrentQueue;
    const int m_Capacity;
  };
} // namespace Elvis
#endif // QUEUE_H
