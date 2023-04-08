//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef REQUEST_CONTEXT_H
#define REQUEST_CONTEXT_H

#include "queue.h"
#include "response_context.h"
#include "route_manager.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Elvis
{
  class ClientContext;

  class IRequestParser
  {
  public:
    virtual ~IRequestParser() = default;
    virtual void Parse(std::shared_ptr<ClientContext> c_ctx) const = 0;
  };

  class HttpRequestParser final : public IRequestParser
  {
  public:
    HttpRequestParser(std::unique_ptr<HttpResponseContext> httpResponseContext,
                      std::shared_ptr<Elvis::IQueue> concurrentQueue,
                      std::shared_ptr<Elvis::RouteManager> routeManager);

    virtual void Parse(std::shared_ptr<ClientContext> c_ctx) const override;

  private:
    std::unique_ptr<HttpResponseContext> m_HttpResponseContext;
    std::shared_ptr<RouteManager> m_RouteManager;
    std::shared_ptr<Elvis::IQueue> m_ConcurrentQueue;
  };

  class WebsocketRequestParser final : public IRequestParser
  {
  public:
    WebsocketRequestParser(
        std::unique_ptr<WebsocketResponseContext> wsResponseContext,
        std::shared_ptr<Elvis::IQueue> concurrentQueue);

    virtual void Parse(std::shared_ptr<ClientContext> c_ctx) const override;

  private:
    std::unique_ptr<WebsocketResponseContext> m_WSResponseContext;
    std::shared_ptr<Elvis::IQueue> m_ConcurrentQueue;
  };

  class IRequestContext
  {
  public:
    virtual ~IRequestContext() = default;

    void setRequestParser(std::unique_ptr<IRequestParser> request)
    {
      m_RequestParser = std::move(request);
    }

    void DoParse(std::shared_ptr<ClientContext> c_ctx) const
    {
      return m_RequestParser->Parse(c_ctx);
    }

  protected:
    std::unique_ptr<IRequestParser> m_RequestParser;
  };

  class HttpRequestContext final : public IRequestContext
  {
  public:
    HttpRequestContext(std::unique_ptr<HttpResponseContext> httpResponseContext,
                       std::shared_ptr<Elvis::IQueue> concurrentQueue,
                       std::shared_ptr<Elvis::RouteManager> routeManager)
    {
      this->m_RequestParser = std::make_unique<HttpRequestParser>(
          std::move(httpResponseContext), concurrentQueue,
          routeManager); // default for now
    }
  };

  class WebsocketRequestContext final : public IRequestContext
  {
  public:
    WebsocketRequestContext(
        std::unique_ptr<WebsocketResponseContext> wsResponseContext,
        std::shared_ptr<Elvis::IQueue> concurrentQueue)
    {
      this->m_RequestParser = std::make_unique<WebsocketRequestParser>(
          std::move(wsResponseContext), concurrentQueue); // default for now
    }
  };
} // namespace Elvis
#endif // REQUEST_CONTEXT_H
