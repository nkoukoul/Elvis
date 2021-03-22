//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef RESPONSE_CONTEXT_H
#define RESPONSE_CONTEXT_H

#include <string>
#include <memory>
#include <unordered_map>
#include <bitset>
#include "event_queue.h"

class app;
namespace elvis::io_context { class client_context; }

class response_creator
{
public:
  response_creator() = default;

  virtual void create_response(std::shared_ptr<elvis::io_context::client_context> c_ctx) const = 0;
};

class http_response_creator : public response_creator
{
public:
  http_response_creator(app *application_context = nullptr);

  void create_response(std::shared_ptr<elvis::io_context::client_context> c_ctx) const override;

private:
  app *application_context_;
};

class websocket_response_creator : public response_creator
{
public:
  websocket_response_creator(app *application_context = nullptr);

  void create_response(std::shared_ptr<elvis::io_context::client_context> c_ctx) const override;

private:
  app *application_context_;
};

class i_response_context
{
public:
  i_response_context() = default;

  void set_response_context(std::unique_ptr<response_creator> response)
  {
    response_ = std::move(response);
  }

  void do_create_response(std::shared_ptr<elvis::io_context::client_context> c_ctx) const
  {
    return response_->create_response(c_ctx);
  }

protected:
  std::unique_ptr<response_creator> response_;
};

class http_response_context : public i_response_context
{
public:
  http_response_context(app *application_context = nullptr)
  {
    this->response_ = std::make_unique<http_response_creator>(application_context); //default for now
  }
};

class websocket_response_context : public i_response_context
{
public:
  websocket_response_context(app *application_context = nullptr)
  {
    this->response_ = std::make_unique<websocket_response_creator>(application_context); //default for now
  }
};

#endif //RESPONSE_CONTEXT_H
