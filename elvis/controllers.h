//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include "client_context.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Elvis
{
class IController
{
public:
    virtual ~IController() = default;

    void Run(std::shared_ptr<ClientContext> c_ctx);

    virtual void DoStuff(std::unordered_map<std::string, std::string>& deserialized_input_data) = 0;
};
} // namespace Elvis
#endif // CONTROLLERS_H
