//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include "event_queue.h"

class app;

class i_controller
{
public:
  i_controller() = default;
  
  void run(
      std::unordered_map<std::string, std::string> deserialized_input_data,
      int client_socket,
      app *ac,
      std::shared_ptr<i_event_queue> executor);

  virtual void do_stuff(
      std::unordered_map<std::string, std::string> &deserialized_input_data,
      app *ac,
      std::shared_ptr<i_event_queue> executor) = 0;
};

class file_get_controller : public i_controller
{
public:
  file_get_controller() = default;
  
  void do_stuff(
      std::unordered_map<std::string, std::string> &deserialized_input_data,
      app *ac,
      std::shared_ptr<i_event_queue> executor) override;
  
};

class file_post_controller : public i_controller
{
public:
  file_post_controller() = default;
  
  void do_stuff(
      std::unordered_map<std::string, std::string> &deserialized_input_data,
      app *ac,
      std::shared_ptr<i_event_queue> executor) override;

};

class trigger_post_controller : public i_controller
{
public:
  trigger_post_controller() = default;
  void do_stuff(
      std::unordered_map<std::string, std::string> &deserialized_input_data,
      app *ac,
      std::shared_ptr<i_event_queue> executor) override;
};

#endif //CONTROLLERS_H
