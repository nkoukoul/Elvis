//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef REQUEST_CONTEXT_H
#define REQUEST_CONTEXT_H

#include "queue.h"
#include <string>
#include <memory>
#include <unordered_map>

class app;
namespace Elvis
{
  class ClientContext;

  class IRequestParser
  {
  public:
    virtual ~IRequestParser() = default;
    virtual void Parse(
        std::shared_ptr<ClientContext> c_ctx) const = 0;
  };

  class HttpRequestParser final : public IRequestParser
  {
  public:
    HttpRequestParser(app *application_context = nullptr);

    virtual void Parse(std::shared_ptr<Elvis::ClientContext> c_ctx) const override;

  private:
    app *application_context_;
  };

  class WebsocketRequestParser final : public IRequestParser
  {
  public:
    WebsocketRequestParser(app *application_context = nullptr);

    virtual void Parse(std::shared_ptr<Elvis::ClientContext> c_ctx) const override;

  private:
    app *application_context_;
  };

  class IRequestContext
  {
  public:
    virtual ~IRequestContext() = default;

    void setRequestParser(std::unique_ptr<IRequestParser> request)
    {
      m_RequestParser = std::move(request);
    }

    void DoParse(std::shared_ptr<Elvis::ClientContext> c_ctx) const
    {
      return m_RequestParser->Parse(c_ctx);
    }

  protected:
    std::unique_ptr<IRequestParser> m_RequestParser;
  };

  class HttpRequestContext final : public IRequestContext
  {
  public:
    HttpRequestContext(app *application_context = nullptr)
    {
      this->m_RequestParser = std::make_unique<HttpRequestParser>(application_context); // default for now
    }
  };

  class WebsocketRequestContext final : public IRequestContext
  {
  public:
    WebsocketRequestContext(app *application_context = nullptr)
    {
      this->m_RequestParser = std::make_unique<WebsocketRequestParser>(application_context); // default for now
    }
  };
}
#endif // REQUEST_CONTEXT_H
