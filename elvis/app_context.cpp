//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//
#include "app_context.h"
#ifdef USE_POSTGRES
#include "pg_db_connector.h"
#endif

app *app::app_instance_{nullptr};
std::mutex app::app_mutex_;

void app::configure(std::unique_ptr<Elvis::TCPContext> http_ioc,
                    std::unique_ptr<Elvis::HttpRequestContext> httpRequestContext,
                    std::unique_ptr<Elvis::HttpResponseContext> httpResponseContext,
                    std::unique_ptr<Elvis::WebsocketRequestContext> wsRequestContext,
                    std::unique_ptr<Elvis::WebsocketResponseContext> wsResponseContext,
                    std::unique_ptr<Elvis::IJSONContext> jsonContext,
                    std::unique_ptr<Elvis::RouteManager> routeManager)
{
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (http_ioc) 
  {
    ioc_ = std::move(http_ioc);
  }
  m_HttpRequestContext = std::move(httpRequestContext);
  m_HttpResponseContext = std::move(httpResponseContext);
  m_WSRequestContext = std::move(wsRequestContext);
  m_WSResponseContext = std::move(wsResponseContext);
  if (jsonContext)
  {
    m_JSONContext = std::move(jsonContext);
  }
  if (routeManager)
  {
    m_RouteManager = std::move(routeManager);
  }
  return;
}

void app::run(int thread_number)
{
  m_CacheManager = std::make_unique<Elvis::CacheManager<Elvis::LRUCache<std::string, std::string>>>(3);
#ifdef USE_POSTGRES
   dbEngine = std::make_unique<Elvis::PGEngine>(thread_number);
#else
  dbEngine = std::make_unique<Elvis::MockEngine>(thread_number);
#endif
#ifdef USE_CRYPTO
  cryptoManager = std::make_shared<cryptocpp_crypto_manager>();
#else
  cryptoManager = std::make_shared<Elvis::MockCryptoManager>();
#endif
  m_AsyncQueue = std::make_unique<Elvis::AsyncQueue>(10);
  m_Logger = std::make_unique<Logger>("server.log");
  if (ioc_)
  {
    thread_pool_.reserve(thread_number - 1);
    for (auto i = thread_number - 1; i > 0; --i)
    {
      thread_pool_.emplace_back(
          [&http_ioc = (this->ioc_)] {
            http_ioc->Run();
          });
    }
    ioc_->Run();
  }
  // Block until all the threads exit
  for (auto &t : thread_pool_)
    if (t.joinable())
      t.join();
  return;
}

app *app::get_instance()
{
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (app_instance_ == nullptr)
  {
    app_instance_ = new app();
  }
  return app_instance_;
}
