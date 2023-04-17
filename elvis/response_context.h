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

    virtual void
    CreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const = 0;
  };

  class HttpResponseCreator final : public ResponseCreator
  {
  public:
    HttpResponseCreator(
        std::shared_ptr<Elvis::IOContext> ioContext,
        std::shared_ptr<Elvis::IQueue> concurrentQueue,
        std::shared_ptr<Elvis::ICryptoManager> cryptoManager);

    virtual void
    CreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const override;

  private:
    std::shared_ptr<Elvis::IQueue> m_ConcurrentQueue;
    std::shared_ptr<Elvis::ICryptoManager> m_CryptoManager;
    std::shared_ptr<Elvis::IOContext> m_IOContext;
  };

  class WebsocketResponseCreator final : public ResponseCreator
  {
  public:
    WebsocketResponseCreator(std::shared_ptr<Elvis::IOContext> ioContext, std::shared_ptr<Elvis::IQueue> concurrentQueue);

    virtual void
    CreateResponse(std::shared_ptr<Elvis::ClientContext> c_ctx) const override;

  private:
    std::shared_ptr<Elvis::IOContext> m_IOContext;
    std::shared_ptr<Elvis::IQueue> m_ConcurrentQueue;
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
    HttpResponseContext(
        std::shared_ptr<Elvis::IOContext> ioContext,
        std::shared_ptr<Elvis::IQueue> concurrentQueue,
        std::shared_ptr<Elvis::ICryptoManager> cryptoManager)
    {
      this->m_ResponseCreator = std::make_unique<HttpResponseCreator>(ioContext, concurrentQueue, cryptoManager); // default for now
    }
  };

  class WebsocketResponseContext final : public IResponseContext
  {
  public:
    WebsocketResponseContext(std::shared_ptr<Elvis::IOContext> ioContext, std::shared_ptr<Elvis::IQueue> concurrentQueue)
    {
      this->m_ResponseCreator = std::make_unique<WebsocketResponseCreator>(ioContext, concurrentQueue); // default for now
    }
  };
} // namespace Elvis

#endif // RESPONSE_CONTEXT_H
