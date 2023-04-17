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
#include "utils.h"

class App
{
public:
  // Singletons should not be cloneable.
  App(App &other) = delete;

  // Singletons should not be assignable.
  void operator=(const App &) = delete;

  // This is the static method that controls the access to the singleton
  static App *GetInstance();

  void Configure(std::string ipaddr, int port, std::shared_ptr<Elvis::RouteManager> routeManager);

  void Run(int thread_number);

  std::shared_ptr<Elvis::IQueue> GetAppConcurrentQueueSharedInstance();
  std::shared_ptr<Elvis::IQueue> m_ConcurrentQueue;
  std::shared_ptr<Elvis::TCPContext> m_IOContext;
  std::unique_ptr<Elvis::HttpRequestContext> m_HTTPRequestContext;
  std::unique_ptr<Elvis::WebsocketRequestContext> m_WSRequestContext;
  std::unique_ptr<Elvis::IJSONContext> m_JSONContext;
  std::unique_ptr<Elvis::IDBEngine> dbEngine;
  std::unique_ptr<Elvis::ICacheManager> m_CacheManager;
  std::unique_ptr<ILogger> m_Logger;
  std::shared_ptr<Elvis::ICryptoManager> m_CryptoManager;
  static std::mutex app_mutex_;
  std::vector<int> broadcast_fd_list;

protected:
  App() = default;
  ~App(){};

private:
  static App *app_instance_;
  std::vector<std::thread> thread_pool_;
};

#endif // APP_CONTEXT_H
