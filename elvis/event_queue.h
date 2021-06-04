//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <vector>
#include <list>
#include <future>
#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include <algorithm>

class i_strand_manager
{
public:
  virtual std::future<void> consume_event() = 0;

  virtual void produce_event(std::future<void> event) = 0;

};

class strand_manager : public i_strand_manager
{
public:
  strand_manager(int const capacity): capacity_(capacity){}

  std::future<void> consume_event() override
  {
      std::future<void> event;
      std::lock_guard<std::mutex> lock(executor_lock_);
      if (!empty())
      {
          event = std::move(e_q_.front());
          e_q_.pop_front();
      }
      return event;
  }

  void produce_event(std::future<void> event) override
  {
      std::lock_guard<std::mutex> lock(executor_lock_);
      e_q_.emplace_back(std::move(event));
      return;
  }

  bool empty() const { return e_q_.empty(); }

private:
    std::mutex executor_lock_;
    std::list<std::future<void>> e_q_;
    const int capacity_;
};
#endif // EVENT_QUEUE_H
