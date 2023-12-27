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
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

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
private:
    size_t m_ThreadNumber;
    const int m_Capacity;
    bool m_Quit{false};
    std::vector<std::thread> m_ThreadPool;
    std::mutex m_ConcurrentQueueLock;
    std::list<std::pair<std::function<void()>, std::string>> m_ConcurrentQueue;
    std::condition_variable m_CV;

public:
    ConcurrentQueue(const int capacity, const size_t threadNumber)
        : m_ThreadNumber{threadNumber}
        , m_Capacity{capacity}
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
            m_ThreadPool.emplace_back([this]() {
                std::unique_lock<std::mutex> lock(this->m_ConcurrentQueueLock);
                do
                {
                    m_CV.wait(lock, [this] {
                        return (this->m_Quit || this->m_ConcurrentQueue.size());
                    });
                    if (!this->m_Quit)
                    {
                        auto pair = this->m_ConcurrentQueue.front();
                        this->m_ConcurrentQueue.pop_front();
#ifdef DEBUG
                        std::cout << "ConcurrentQueue::PickTask " << pair.second << "\n";
#endif
                        auto callback = pair.first;
                        lock.unlock();
                        callback();
                        lock.lock();
                    }
                } while (!this->m_Quit);
            });
        }

        // Block until all the threads exit
        for (auto& t : m_ThreadPool)
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
            std::cout << "ConcurrentQueue::DispatchAsync " << description << "\n";
#endif
            m_ConcurrentQueue.emplace_back(std::make_pair(callback, description));
        }
        m_CV.notify_one();
    }
};

class SerialQueue final : public IQueue
{
private:
    bool m_Quit{false};
    const int m_Capacity{100};
    std::mutex m_SerialQueueLock;
    std::thread m_Thread;
    std::deque<std::pair<std::function<void()>, std::string>> m_SerialQueue;

public:
    SerialQueue()
    {
        Run();
    }

    ~SerialQueue()
    {
        {
            std::lock_guard<std::mutex> lock(m_SerialQueueLock);
            m_Quit = true;
        }
        if (m_Thread.joinable())
        {
            m_Thread.join();
        }
    }

    virtual void Run() override
    {
        m_Thread = std::thread([this]() {
            do
            {
                std::lock_guard<std::mutex> lock(m_SerialQueueLock);
                if (!this->m_Quit && !this->m_SerialQueue.empty())
                {
                    auto pair = this->m_SerialQueue.front();
                    this->m_SerialQueue.pop_front();
#ifdef DEBUG
                    std::cout << "SerialQueue::PickTask " << pair.second << "\n";
#endif
                    auto callback = pair.first;
                    callback();
                }
            } while (!this->m_Quit);
        });
    }

    virtual void DispatchAsync(std::function<void()> callback, std::string description) override
    {
#ifdef DEBUG
        std::cout << "SerialQueue::DispatchAsync " << description << "\n";
#endif
        if (std::this_thread::get_id() == m_Thread.get_id())
        {
            m_SerialQueue.emplace_back(std::make_pair(callback, description));
            return;
        }
        // If outside of strand aquire lock and check for capacity.
        std::lock_guard<std::mutex> lock(m_SerialQueueLock);
        while (m_Capacity < m_SerialQueue.size())
        {
            // stall calling thread until there is room in this queue.
            sleep(1);
        }
        m_SerialQueue.emplace_back(std::make_pair(callback, description));
    }
};
} // namespace Elvis
#endif // QUEUE_H
