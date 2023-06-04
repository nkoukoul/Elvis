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

using namespace Elvis;

App *App::app_instance_{nullptr};
std::mutex App::app_mutex_;

void App::Configure(std::string ipaddr, int port, std::shared_ptr<RouteManager> routeManager, std::string logfile, LogLevel loglevel = LogLevel::DETAIL)
{
  std::lock_guard<std::mutex> guard(app_mutex_);
#ifdef USE_CRYPTO
  m_CryptoManager = std::make_shared<cryptocpp_crypto_manager>();
#else
  m_CryptoManager = std::make_shared<MockCryptoManager>();
#endif
  m_Logger = std::make_shared<Logger>(logfile, loglevel);
  m_ConcurrentQueue = std::make_shared<ConcurrentQueue>(100);
  m_ConnectionMonitor = std::make_shared<ConnectionMonitor>();
  m_IOContext = std::make_shared<TCPContext>(ipaddr, port, m_ConcurrentQueue, m_Logger, m_ConnectionMonitor);
  auto httpResponseContext = std::make_unique<HttpResponseContext>(m_IOContext, m_ConcurrentQueue, m_CryptoManager);
  m_HTTPRequestContext = std::make_shared<HttpRequestContext>(std::move(httpResponseContext), m_ConcurrentQueue, routeManager);
  auto wsResponseContext = std::make_unique<WebsocketResponseContext>(m_IOContext, m_ConcurrentQueue);
  m_WSRequestContext = std::make_unique<WebsocketRequestContext>(std::move(wsResponseContext), m_ConcurrentQueue);
  m_JSONContext = std::make_unique<JSONContext>();
  m_CacheManager = std::make_unique<CacheManager<LRUCache<std::string, std::string>>>(3);
  return;
}

void App::Run(int thread_number)
{
#ifdef USE_POSTGRES
  m_DBEngine = std::make_unique<PGEngine>(thread_number);
#else
  m_DBEngine = std::make_unique<MockEngine>(thread_number);
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

void App::Cache(std::string key, std::string data)
{
  m_CacheManager->GetCache<LRUCache<std::string, std::string>>()->Insert(key, data);
}

std::string App::GetCacheData(std::string key) const
{
  return (*m_CacheManager->GetCache<LRUCache<std::string, std::string>>())[key];
}

std::list<std::unordered_map<std::string, std::string>> App::JSONDeserialize(std::string &&serializedData) const
{
  return m_JSONContext->DoDeserialize(std::move(serializedData));
}

void App::CreateModel(std::string query)
{
  m_DBEngine->CreateModel(query);
}

void App::RetrieveModel(std::string query)
{
  m_DBEngine->RetrieveModel(query);
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
