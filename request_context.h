//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef REQUEST_CONTEXT_H
#define REQUEST_CONTEXT_H

#include <string>
#include <memory>
#include <unordered_map>
#include "event_queue.h"

class app;
class client_context;

class request_parser
{
public:
  request_parser() = default;

  virtual void parse(
      std::shared_ptr<client_context> c_ctx) const = 0;
};

class http_request_parser : public request_parser
{
public:
  http_request_parser(app *application_context = nullptr);

  void parse(std::shared_ptr<client_context> c_ctx) const override;

private:
  app *application_context_;
};

class websocket_request_parser : public request_parser
{
public:
  websocket_request_parser(app *application_context = nullptr);

  void parse(std::shared_ptr<client_context> c_ctx) const override;

private:
  app *application_context_;
};

class i_request_context
{
public:
  i_request_context() = default;

  void set_request_context(std::unique_ptr<request_parser> request)
  {
    request_ = std::move(request);
  }

  void do_parse(std::shared_ptr<client_context> c_ctx) const
  {
    return request_->parse(c_ctx);
  }

protected:
  std::unique_ptr<request_parser> request_;
};

class http_request_context : public i_request_context
{
public:
  http_request_context(app *application_context = nullptr)
  {
    this->request_ = std::make_unique<http_request_parser>(application_context); //default for now
  }
};

class websocket_request_context : public i_request_context
{
public:
  websocket_request_context(app *application_context = nullptr)
  {
    this->request_ = std::make_unique<websocket_request_parser>(application_context); //default for now
  }
};

#endif //REQUEST_CONTEXT_H
