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

#include "client_context.h"
#include "context_delegate.h"
#include "queue.h"
#include "response_context.h"
#include "route_manager.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Elvis
{
class IRequestParser
{
public:
    virtual ~IRequestParser() = default;
    virtual void Parse(std::shared_ptr<ClientContext> c_ctx) const = 0;
};

class HttpRequestParser final : public std::enable_shared_from_this<HttpRequestParser>, public IRequestParser
{
public:
    explicit HttpRequestParser(
        std::unique_ptr<HttpResponseContext> httpResponseContext,
        std::shared_ptr<IQueue> concurrentQueue,
        std::shared_ptr<RouteManager> routeManager);

    virtual void Parse(std::shared_ptr<ClientContext> c_ctx) const override;

private:
    std::unique_ptr<HttpResponseContext> m_HttpResponseContext;
    std::shared_ptr<RouteManager> m_RouteManager;
    std::shared_ptr<IQueue> m_ConcurrentQueue;
};

class WebsocketRequestParser final : public std::enable_shared_from_this<WebsocketRequestParser>, public IRequestParser
{
public:
    WebsocketRequestParser(
        std::unique_ptr<WebsocketResponseContext> wsResponseContext,
        std::shared_ptr<IQueue> concurrentQueue);

    virtual void Parse(std::shared_ptr<ClientContext> c_ctx) const override;

private:
    std::unique_ptr<WebsocketResponseContext> m_WSResponseContext;
    std::shared_ptr<IQueue> m_ConcurrentQueue;
};

class IRequestContext
{
public:
    virtual ~IRequestContext() = default;

    IRequestContext(std::shared_ptr<IRequestParser> requestParser)
        : m_RequestParser{requestParser}
    {
    }

    void DoParse(std::shared_ptr<ClientContext> c_ctx) const
    {
        return m_RequestParser->Parse(c_ctx);
    }

private:
    std::shared_ptr<IRequestParser> m_RequestParser;
};

class HttpRequestContext final : public IRequestContext
{

public:
    explicit HttpRequestContext(
        std::unique_ptr<HttpResponseContext> httpResponseContext,
        std::shared_ptr<IQueue> concurrentQueue,
        std::shared_ptr<RouteManager> routeManager)
        : IRequestContext(
              std::make_shared<HttpRequestParser>(std::move(httpResponseContext), concurrentQueue, routeManager))
    {
    }
};

class WebsocketRequestContext final : public IRequestContext
{
public:
    WebsocketRequestContext(
        std::unique_ptr<WebsocketResponseContext> wsResponseContext,
        std::shared_ptr<IQueue> concurrentQueue)
        : IRequestContext(std::make_shared<WebsocketRequestParser>(std::move(wsResponseContext), concurrentQueue))
    {
    }
};
} // namespace Elvis
#endif // REQUEST_CONTEXT_H
