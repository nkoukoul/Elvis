//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//
#include "app_context.h"
#ifdef USE_POSTGRES
#include "pg_db_connector.h"
#endif

App *App::app_instance_{nullptr};
std::mutex App::app_mutex_;

void App::Configure(std::string ipaddr, int port, std::shared_ptr<Elvis::RouteManager> routeManager)
{
  std::lock_guard<std::mutex> guard(app_mutex_);
#ifdef USE_CRYPTO
  m_CryptoManager = std::make_shared<cryptocpp_crypto_manager>();
#else
  m_CryptoManager = std::make_shared<Elvis::MockCryptoManager>();
#endif
  m_Logger = std::make_unique<Logger>("server.log");
  m_ConcurrentQueue = std::make_shared<Elvis::ConcurrentQueue>(100);
  m_IOContext = std::make_shared<Elvis::TCPContext>(ipaddr, port, m_ConcurrentQueue);
  auto httpResponseContext = std::make_unique<Elvis::HttpResponseContext>(m_IOContext, m_ConcurrentQueue, m_CryptoManager);
  m_HTTPRequestContext = std::make_unique<Elvis::HttpRequestContext>(std::move(httpResponseContext), m_ConcurrentQueue, routeManager);
  auto wsResponseContext = std::make_unique<Elvis::WebsocketResponseContext>(m_IOContext, m_ConcurrentQueue);
  m_WSRequestContext = std::make_unique<Elvis::WebsocketRequestContext>(std::move(wsResponseContext), m_ConcurrentQueue);
  m_JSONContext = std::make_unique<Elvis::JSONContext>();
  m_CacheManager = std::make_unique<Elvis::CacheManager<Elvis::LRUCache<std::string, std::string>>>(3);
  return;
}

void App::Run(int thread_number)
{
#ifdef USE_POSTGRES
  dbEngine = std::make_unique<Elvis::PGEngine>(thread_number);
#else
  dbEngine = std::make_unique<Elvis::MockEngine>(thread_number);
#endif
  if (m_IOContext)
  {
    thread_pool_.reserve(thread_number - 1);
    for (auto i = thread_number - 1; i > 0; --i)
    {
      thread_pool_.emplace_back(
          [&ioContext = (this->m_IOContext)]
          { ioContext->Run(); });
    }
    m_IOContext->Run();
  }
  // Block until all the threads exit
  for (auto &t : thread_pool_)
    if (t.joinable())
      t.join();
  return;
}

std::shared_ptr<Elvis::IQueue> App::GetAppConcurrentQueueSharedInstance()
{
  return m_ConcurrentQueue;
}

App *App::GetInstance()
{
  std::lock_guard<std::mutex> guard(app_mutex_);
  if (app_instance_ == nullptr)
  {
    app_instance_ = new App();
  }
  return app_instance_;
}
