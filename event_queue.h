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
#include <string>
#include <memory>
#include <mutex>
#include <iostream>
#include <algorithm>

class app;

class base_event{
public:
  //base_event() = default;
  virtual ~base_event(){};
  template<class T> const T & get_data() const;
  template<class T, class U> void set_data(U && data);
};

template<class T>
class event : public base_event{
public:
  event(int priority, T && data):priority_(priority), data_(std::move(data)){};
  const T & get_data() const{ return data_ ;}
  void set_data(T && data){ data_ = std::move(data);}
  
  bool operator < (event const & m_event) const{
    return priority_ < m_event.priority_;
  }

private:
  int priority_;
  T data_;
};

template<class T> const T & base_event::get_data() const
{ return dynamic_cast<const event<T>&>(*this).get_data(); }

template<class T, class U> void base_event::set_data(U && data)
{ return dynamic_cast<event<T>>(*this).set_value(std::move(data)); }

template <class D>
class event_queue{
public:
  event_queue(int const capacity, app * ac);
  ~event_queue();
  int size() const;
  bool empty() const;
  D consume_event();
  void produce_event(D && data);
  void print_queue_elements() const;
private:
  app * ac_;
  std::mutex queue_lock_;
  int get_priority(std::string const action) const;
  std::vector<std::unique_ptr<base_event>> e_q_;
  const int capacity_;
};


#endif // EVENT_QUEUE_H
