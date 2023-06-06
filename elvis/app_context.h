//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "cache.h"
#include "common_headers.h"
#include "crypto_manager.h"
#include "db_connector.h"
#include "io_context.h"
#include "json_utils.h"
#include "logger.h"
#include "queue.h"
#include "request_context.h"
#include "route_manager.h"
#include "monitor.h"
#include "utils.h"

namespace Elvis
{
  class App
  {
  public:
    // Singletons should not be cloneable.
    App(App &other) = delete;

    // Singletons should not be assignable.
    void operator=(const App &) = delete;

    // This is the static method that controls the access to the singleton
    static App *GetInstance();

    void Configure(std::string ipaddr, int port, int threadNumber, std::shared_ptr<RouteManager> routeManager, std::string logfile, LogLevel loglevel);

    void Cache(std::string key, std::string data);

    std::string GetCacheData(std::string key) const;

    std::list<std::unordered_map<std::string, std::string>> JSONDeserialize(std::string &&serializedData) const;

    void CreateModel(std::string query);

    void RetrieveModel(std::string query);

    void Run();

    static std::mutex app_mutex_;
    std::vector<int> broadcast_fd_list;

  protected:
    App() = default;
    ~App() = default;

  private:
    static App *app_instance_;
    std::vector<std::thread> thread_pool_;
    std::shared_ptr<IQueue> m_ConcurrentQueue;
    std::shared_ptr<TCPContext> m_IOContext;
    std::shared_ptr<HttpRequestContext> m_HTTPRequestContext;
    std::unique_ptr<WebsocketRequestContext> m_WSRequestContext;
    std::unique_ptr<IJSONContext> m_JSONContext;
    std::unique_ptr<IDBEngine> m_DBEngine;
    std::unique_ptr<ICacheManager> m_CacheManager;
    std::shared_ptr<ILogger> m_Logger;
    std::shared_ptr<ICryptoManager> m_CryptoManager;
    std::shared_ptr<IConnectionMonitor> m_ConnectionMonitor;

    friend class TCPContext;
  };
}

#endif // APP_CONTEXT_H
