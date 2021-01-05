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

class request_parser
{
public:
  request_parser() = default;

  virtual void parse(
      int const client_socket,
      std::string &&input_data,
      std::shared_ptr<i_event_queue> executor) const = 0;
};

class http_request_parser : public request_parser
{
public:
  http_request_parser(app *application_context = nullptr);

  void parse(
      int const client_socket,
      std::string &&input_data,
      std::shared_ptr<i_event_queue> executor) const override;

private:
  app *application_context_;
};

class websocket_request_parser : public request_parser
{
public:
  websocket_request_parser(app *application_context = nullptr);

  void parse(
      int const client_socket,
      std::string &&input_data,
      std::shared_ptr<i_event_queue> executor) const override;

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

  void do_parse(
      int const client_socket,
      std::string input_data,
      std::shared_ptr<i_event_queue> executor) const
  {
    return request_->parse(client_socket, std::move(input_data), executor);
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
