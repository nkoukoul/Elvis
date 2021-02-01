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
#include "cache.h"
#include "db_connector.h"

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

class i_event_queue
{
public:
  virtual ~i_event_queue(){};

  template <class D>
  D consume_event();

  template <class D, class U>
  void produce_event(U &&data);

  virtual bool empty() const = 0;

  virtual int size() const = 0;

  virtual i_cache *access_cache_() = 0;

  virtual db_connector *access_connector() = 0;
};

template <class D>
class event_queue : public i_event_queue
{
public:
  event_queue(
      int const capacity)
      : capacity_(capacity)
  {
    executor_cache_ = std::make_unique<t_cache<std::string, std::string>>(5);
    pg_connector_ = std::make_unique<pg_connector>(
        "test_db", "test", "test", "127.0.0.1", "5432");
  }

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

  i_cache *access_cache_()
  {
    return executor_cache_.get();
  }

  db_connector *access_connector()
  {
    return pg_connector_.get();
  }

private:
  std::unique_ptr<i_cache> executor_cache_;
  std::mutex executor_lock_;
  std::unique_ptr<db_connector> pg_connector_;
  std::queue<std::unique_ptr<base_event>> e_q_;
  const int capacity_;
};

template <class D>
D i_event_queue::consume_event()
{
  return dynamic_cast<event_queue<D> &>(*this).consume_event();
}

template <class D, class U>
void i_event_queue::produce_event(U &&data)
{
  return dynamic_cast<event_queue<D> &>(*this).produce_event(std::move(data));
}

#endif // EVENT_QUEUE_H
