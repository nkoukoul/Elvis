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

App* App::app_instance_{nullptr};
std::mutex App::app_mutex_;

void App::Configure(
    std::string ipaddr,
    int port,
    int threadNumber,
    std::shared_ptr<RouteManager> routeManager,
    std::string logfile,
    LogLevel loglevel = LogLevel::DETAIL)
{
    std::lock_guard<std::mutex> guard(app_mutex_);
#ifdef USE_CRYPTO
    m_CryptoManager = std::make_shared<cryptocpp_crypto_manager>();
#else
    m_CryptoManager = std::make_shared<MockCryptoManager>();
#endif
    m_Logger = std::make_shared<Logger>(logfile, loglevel);
    m_ConcurrentQueue = std::make_shared<ConcurrentQueue>(100, threadNumber);
#ifdef DEBUG
    m_ConnectionMonitor = std::make_shared<ConnectionMonitor>(1);
#else
    m_ConnectionMonitor = std::make_shared<MockConnectionMonitor>();
#endif
    m_TCPServer =
        CreateTCPServer(ipaddr, port, routeManager, m_CryptoManager, m_ConcurrentQueue, m_Logger, m_ConnectionMonitor);
    m_JSONContext = std::make_unique<JSONContext>();
    m_CacheManager = std::make_unique<CacheManager<LRUCache<std::string, std::string>>>(3);
#ifdef USE_POSTGRES
    m_DBEngine = std::make_unique<PGEngine>(threadNumber);
#else
    m_DBEngine = std::make_unique<MockEngine>(threadNumber);
#endif
}

void App::Run()
{
    m_ConnectionMonitor->Run();
    m_TCPServer->Run();
    m_ConcurrentQueue->Run();
}

void App::Cache(std::string key, std::string data)
{
    m_CacheManager->GetCache<LRUCache<std::string, std::string>>()->Insert(key, data);
}

std::string App::GetCacheData(std::string key) const
{
    return (*m_CacheManager->GetCache<LRUCache<std::string, std::string>>())[key];
}

std::list<std::unordered_map<std::string, std::string>> App::JSONDeserialize(std::string&& serializedData) const
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

App* App::GetInstance()
{
    std::lock_guard<std::mutex> guard(app_mutex_);
    if (app_instance_ == nullptr)
    {
        app_instance_ = new App();
    }
    return app_instance_;
}
