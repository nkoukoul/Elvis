//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef RESPONSE_CONTEXT_H
#define RESPONSE_CONTEXT_H

#include "queue.h"
#include <string>
#include <memory>
#include <unordered_map>

class app;
namespace Elvis
{

  // Forward Declaration
  class ClientContext;

  class ResponseCreator
  {
  public:
    virtual ~ResponseCreator() = default;

    virtual void CreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const = 0;
  };

  class HttpResponseCreator final : public ResponseCreator
  {
  public:
    HttpResponseCreator(app *application_context = nullptr);

    virtual void CreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const override;

  private:
    app *application_context_;
  };

  class WebsocketResponseCreator final : public ResponseCreator
  {
  public:
    WebsocketResponseCreator(app *application_context = nullptr);

    virtual void CreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const override;

  private:
    app *application_context_;
  };

  class IResponseContext
  {
  public:
    virtual ~IResponseContext() = default;

    void setResponseCreator(std::unique_ptr<ResponseCreator> responseCreator)
    {
      m_ResponseCreator = std::move(responseCreator);
    }

    void DoCreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const
    {
      return m_ResponseCreator->CreateResponse(c_ctx);
    }

  protected:
    std::unique_ptr<ResponseCreator> m_ResponseCreator;
  };

  class HttpResponseContext final : public IResponseContext
  {
  public:
    HttpResponseContext(app *application_context = nullptr)
    {
      this->m_ResponseCreator = std::make_unique<HttpResponseCreator>(application_context); // default for now
    }
  };

  class WebsocketResponseContext final : public IResponseContext
  {
  public:
    WebsocketResponseContext(app *application_context = nullptr)
    {
      this->m_ResponseCreator = std::make_unique<WebsocketResponseCreator>(application_context); // default for now
    }
  };
}

#endif // RESPONSE_CONTEXT_H
