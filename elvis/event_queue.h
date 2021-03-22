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
#include <queue>
#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include <algorithm>

class base_event
{
public:
  virtual ~base_event(){};

  template <class T>
  const T &get_data() const;

  template <class T, class U>
  void set_data(U &&data);
};

template <class T>
class event : public base_event
{
public:
  event(int priority, T &&data) : priority_(priority), data_(std::move(data)){};

  const T &get_data() const { return data_; }

  void set_data(T &&data) { data_ = std::move(data); }

  bool operator<(event const &m_event) const
  {
    return priority_ < m_event.priority_;
  }

private:
  int priority_;
  T data_;
};

template <class T>
const T &base_event::get_data() const
{
  return dynamic_cast<const event<T> &>(*this).get_data();
}

template <class T, class U>
void base_event::set_data(U &&data)
{
  return dynamic_cast<event<T> &>(*this).set_value(std::move(data));
}

template <class E, class D>
class i_event_queue
{
public:
  virtual ~i_event_queue(){};

  D consume_event();

  void produce_event(D &&data);

  virtual bool empty() const = 0;

  virtual int size() const = 0;
};

template <class D>
class event_queue : public i_event_queue<event_queue<D>, D>
{
public:
  event_queue(int const capacity) : capacity_(capacity) {}

  int size() const { return e_q_.size(); }

  bool empty() const { return e_q_.empty(); }

  D consume_event()
  {
    D data;
    std::lock_guard<std::mutex> lock(executor_lock_);
    if (!empty())
    {
      data = e_q_.front()->get_data<D>();
      e_q_.pop();
    }
    return data;
  }

  void produce_event(D &&data)
  {
    std::lock_guard<std::mutex> lock(executor_lock_);
    e_q_.emplace(std::make_unique<event<D>>(100, std::move(data)));
    return;
  }

private:
  std::mutex executor_lock_;
  std::queue<std::unique_ptr<base_event>> e_q_;
  const int capacity_;
};

class i_strand_manager
{
public:
  virtual ~i_strand_manager(){};

  template<class I_STRAND>
  I_STRAND *access_strand();

};

template <class STRAND>
class strand_manager : public i_strand_manager
{
public:
  strand_manager(int size)
  {
    strand_ = std::make_unique<STRAND>(size);
  }

  STRAND *access_strand()
  {
    return strand_.get();
  }

private:
  std::unique_ptr<STRAND> strand_;
};

template <class I_STRAND>
I_STRAND *i_strand_manager::access_strand()
{
  return static_cast<strand_manager<I_STRAND> *>(this)->access_strand();
}

#endif // EVENT_QUEUE_H
