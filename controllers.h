//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include <unordered_map>
#include <string>

class app;

class i_controller{
public:
  i_controller() = default;
  virtual std::string run(std::unordered_map<std::string, std::string>  && deserialized_input_data, app * ac) = 0;
};

class file_get_controller: public i_controller{
public:
  file_get_controller() = default;
  std::string run(std::unordered_map<std::string, std::string>  && deserialized_input_data, app * ac) override;
};

class file_post_controller: public i_controller{
public:
  file_post_controller() = default;
  std::string run(std::unordered_map<std::string, std::string>  && deserialized_input_data, app * ac) override;
};

class trigger_post_controller: public i_controller{
public:
  trigger_post_controller() = default;
  std::string run(std::unordered_map<std::string, std::string>  && deserialized_input_data, app * ac) override;
};

#endif //CONTROLLERS_H
