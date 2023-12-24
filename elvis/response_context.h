//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef RESPONSE_CONTEXT_H
#define RESPONSE_CONTEXT_H

#include "client_context.h"
#include "crypto_manager.h"
#include "io_context.h"
#include "queue.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Elvis
{
class ResponseCreator
{
public:
    virtual ~ResponseCreator() = default;

    virtual void CreateResponse(std::shared_ptr<ClientContext> c_ctx) const = 0;
};

class HttpResponseCreator final : public std::enable_shared_from_this<HttpResponseCreator>, public ResponseCreator
{
public:
    HttpResponseCreator(
        std::shared_ptr<OutputContext> tcpOutputContext,
        std::shared_ptr<IQueue> concurrentQueue,
        std::shared_ptr<ICryptoManager> cryptoManager);

    virtual void CreateResponse(std::shared_ptr<ClientContext> c_ctx) const override;

private:
    std::shared_ptr<IQueue> m_ConcurrentQueue;
    std::shared_ptr<ICryptoManager> m_CryptoManager;
    std::shared_ptr<OutputContext> m_TCPOutputContext;
};

class WebsocketResponseCreator final : public std::enable_shared_from_this<WebsocketResponseCreator>,
                                       public ResponseCreator
{
public:
    WebsocketResponseCreator(std::shared_ptr<OutputContext> tcpOutputContext, std::shared_ptr<IQueue> concurrentQueue);

    virtual void CreateResponse(std::shared_ptr<ClientContext> c_ctx) const override;

private:
    std::shared_ptr<OutputContext> m_TCPOutputContext;
    std::shared_ptr<IQueue> m_ConcurrentQueue;
};

class IResponseContext
{
public:
    virtual ~IResponseContext() = default;

    void setResponseCreator(std::shared_ptr<ResponseCreator> responseCreator)
    {
        m_ResponseCreator = responseCreator;
    }

    void DoCreateResponse(std::shared_ptr<ClientContext> c_ctx) const
    {
        return m_ResponseCreator->CreateResponse(c_ctx);
    }

protected:
    std::shared_ptr<ResponseCreator> m_ResponseCreator;
};

class HttpResponseContext final : public IResponseContext
{
public:
    HttpResponseContext(
        std::shared_ptr<OutputContext> tcpOutputContext,
        std::shared_ptr<IQueue> concurrentQueue,
        std::shared_ptr<ICryptoManager> cryptoManager)
    {
        this->m_ResponseCreator =
            std::make_shared<HttpResponseCreator>(tcpOutputContext, concurrentQueue, cryptoManager); // default for now
    }
};

class WebsocketResponseContext final : public IResponseContext
{
public:
    WebsocketResponseContext(std::shared_ptr<OutputContext> tcpOutputContext, std::shared_ptr<IQueue> concurrentQueue)
    {
        this->m_ResponseCreator =
            std::make_shared<WebsocketResponseCreator>(tcpOutputContext, concurrentQueue); // default for now
    }
};
} // namespace Elvis

#endif // RESPONSE_CONTEXT_H
