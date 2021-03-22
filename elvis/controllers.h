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


class app;
namespace elvis::io_context { class client_context; }

class i_controller
{
public:
  i_controller() = default;
  
  void run(std::shared_ptr<elvis::io_context::client_context> c_ctx, app *ac);

  virtual void do_stuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac) = 0;
};

#endif //CONTROLLERS_H
