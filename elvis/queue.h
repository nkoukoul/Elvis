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
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <queue>
#include <vector>
#include <thread>

namespace Elvis
{
  class IQueue
  {
  public:
    using wlock = std::unique_lock<std::shared_mutex>;
    using rlock = std::shared_lock<std::shared_mutex>;

    virtual ~IQueue() = default;

    virtual void Run() = 0;

    virtual void DispatchAsync(std::function<void()> callback, std::string description) = 0;
  };

  class ConcurrentQueue final : public IQueue
  {
  public:
    ConcurrentQueue(const int capacity, const size_t threadNumber) : m_ThreadNumber{threadNumber}, m_Capacity{capacity}, m_Quit{false}
    {
      m_ThreadPool.reserve(m_ThreadNumber);
    }

    ~ConcurrentQueue()
    {
      // Signal to dispatch threads that it's time to wrap up
      {
        std::lock_guard<std::mutex> lock(m_ConcurrentQueueLock);
        m_Quit = true;
      }
      m_CV.notify_all();
    }

    virtual void Run() override
    {
      for (auto i = 0; i < m_ThreadNumber; i++)
      {
        m_ThreadPool.emplace_back([this]()
                                  {
          std::unique_lock<std::mutex> lock(this->m_ConcurrentQueueLock);
          do
          {
            m_CV.wait(lock, [this]
                      { return (this->m_Quit || this->m_ConcurrentQueue.size()); });
            if (!this->m_Quit)
            {
              auto pair = this->m_ConcurrentQueue.front();
              this->m_ConcurrentQueue.pop_front();
#ifdef DEBUG
              // std::cout << "ConcurrentQueue::PickTask " << pair.second << "\n";
#endif
              auto callback = pair.first;
              lock.unlock();
              callback();
              lock.lock();
            }
          } while (!m_Quit); });
      }

      // Block until all the threads exit
      for (auto &t : m_ThreadPool)
      {
        if (t.joinable())
        {
          t.join();
        }
      }
    }

    virtual void DispatchAsync(std::function<void()> callback, std::string description) override
    {
      {
        std::lock_guard<std::mutex> lock(m_ConcurrentQueueLock);
#ifdef DEBUG
        // std::cout << "ConcurrentQueue::DispatchAsync " << description << "\n";
#endif
        m_ConcurrentQueue.emplace_back(std::make_pair(callback, description));
      }
      m_CV.notify_one();
    }

  private:
    bool m_Quit;
    const int m_Capacity;
    size_t m_ThreadNumber;
    std::vector<std::thread> m_ThreadPool;
    std::mutex m_ConcurrentQueueLock;
    std::list<std::pair<std::function<void()>, std::string>> m_ConcurrentQueue;
    std::condition_variable m_CV;
  };
} // namespace Elvis
#endif // QUEUE_H
