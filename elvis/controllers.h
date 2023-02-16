//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include <unordered_map>
#include <string>
#include <memory>

class app;

namespace Elvis
{
  class ClientContext;

  class IController
  {
  public:
    virtual ~IController() = default;

    void Run(std::shared_ptr<Elvis::ClientContext> c_ctx, app *ac);

    virtual void DoStuff(std::unordered_map<std::string, std::string> &deserialized_input_data, app *ac) = 0;
  };
}
#endif // CONTROLLERS_H
