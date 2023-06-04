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
#include <condition_variable>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <queue>

namespace Elvis
{
  class IQueue
  {
  public:
    using wlock = std::unique_lock<std::shared_mutex>;
    using rlock = std::shared_lock<std::shared_mutex>;

    virtual ~IQueue() = default;

    virtual bool PickTask(std::future<void> &task) = 0;

    virtual void CreateTask(std::future<void> event, std::string taskType) = 0;
  };

  class ConcurrentQueue final : public IQueue
  {
  public:
    ConcurrentQueue(int const capacity) : m_Capacity(capacity) {}

    virtual bool PickTask(std::future<void> &task) override
    {
      // std::lock_guard<std::mutex> lock(m_ConcurrentQueueLock);
      wlock lock(m_SharedMutex);
      if (!m_ConcurrentQueue.empty())
      {
        auto pair = std::move(m_ConcurrentQueue.front());
#ifdef DEBUG
        std::cout << "ConcurrentQueue::PickTask " << pair.second << "\n";
#endif
        task = std::move(pair.first);
        m_ConcurrentQueue.pop_front();
        return true;
      }
      return false;
    }

    virtual void CreateTask(std::future<void> task,
                            std::string taskType) override
    {
      // std::lock_guard<std::mutex> lock(m_ConcurrentQueueLock);
      wlock lock(m_SharedMutex);
#ifdef DEBUG
      std::cout << "ConcurrentQueue::CreateTask " << taskType << "\n";
#endif
      m_ConcurrentQueue.emplace_back(std::make_pair(std::move(task), taskType));
    }

  private:
    std::mutex m_ConcurrentQueueLock;
    mutable std::shared_mutex m_SharedMutex;
    std::list<std::pair<std::future<void>, std::string>> m_ConcurrentQueue;
    const int m_Capacity;
  };

  class AsyncQueue final : public IQueue
  {
  public:
    AsyncQueue() : m_Quit{false}
    {
      std::unique_lock<std::mutex> lock(m_Lock);
      do
      {
        m_CV.wait(lock, [this]
                  { return (m_Quit || m_AsyncQueue.size()); });
        if (m_Quit)
        {
          break;
        }
        std::future<void> task;
        auto hasTask = PickTask(task);
        if (hasTask)
        {
          lock.unlock();
          task.wait();
          lock.lock();
        }
      } while (!m_Quit);
    }

    ~AsyncQueue()
    {
      // Signal to dispatch threads that it's time to wrap up
      {
        std::lock_guard<std::mutex> lock(m_Lock);
        m_Quit = true;
      }
      m_CV.notify_all();
    }

    virtual bool PickTask(std::future<void> &task)
    {
      if (m_AsyncQueue.size())
      {
        auto pair = std::move(m_AsyncQueue.front());
#ifdef DEBUG
        std::cout << "AsyncQueue::PickTask " << pair.second << "\n";
#endif
        task = std::move(pair.first);
        m_AsyncQueue.pop();
        return true;
      }
      return false;
    }

    virtual void CreateTask(std::future<void> task, std::string taskType)
    {
      std::unique_lock<std::mutex> lock(m_Lock);
      m_AsyncQueue.emplace(std::make_pair(std::move(task), taskType));
      lock.unlock();
      m_CV.notify_one();
    }

  private:
    bool m_Quit;
    std::queue<std::pair<std::future<void>, std::string>> m_AsyncQueue;
    std::mutex m_Lock;
    std::condition_variable m_CV;
  };

} // namespace Elvis
#endif // QUEUE_H
