//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "db_connector.h"
#include "cache.h"
#include "common_headers.h"
#include "crypto_manager.h"
#include "queue.h"
#include "io_context.h"
#include "json_utils.h"
#include "logger.h"
#include "response_context.h"
#include "request_context.h"
#include "route_manager.h"
#include "utils.h"

class app
{
public:
  // Singletons should not be cloneable.
  app(app &other) = delete;

  // Singletons should not be assignable.
  void operator=(const app &) = delete;

  // This is the static method that controls the access to the singleton
  static app *get_instance();

  void configure(std::unique_ptr<Elvis::TCPContext> http_ioc = nullptr,
                 std::unique_ptr<Elvis::HttpRequestContext> httpRequestContext = nullptr,
                 std::unique_ptr<Elvis::HttpResponseContext> httpResponseContext = nullptr,
                 std::unique_ptr<Elvis::WebsocketRequestContext> wsRequestContext = nullptr,
                 std::unique_ptr<Elvis::WebsocketResponseContext> wsResponseContext = nullptr,
                 std::unique_ptr<Elvis::IJSONContext> jsonContext = nullptr,
                 std::unique_ptr<Elvis::RouteManager> routeManager = nullptr);

  void run(int thread_number);

  std::unique_ptr<Elvis::IQueue> m_AsyncQueue;
  std::unique_ptr<Elvis::TCPContext> ioc_;
  std::unique_ptr<Elvis::HttpRequestContext> m_HttpRequestContext;
  std::unique_ptr<Elvis::HttpResponseContext> m_HttpResponseContext;
  std::unique_ptr<Elvis::WebsocketRequestContext> m_WSRequestContext;
  std::unique_ptr<Elvis::WebsocketResponseContext> m_WSResponseContext;
  std::unique_ptr<Elvis::IJSONContext> m_JSONContext;
  std::unique_ptr<Elvis::RouteManager> m_RouteManager;
  std::unique_ptr<Elvis::IDBEngine> dbEngine;
  std::unique_ptr<Elvis::ICacheManager> m_CacheManager;
  std::unique_ptr<ILogger> m_Logger;
  std::shared_ptr<Elvis::ICryptoManager> cryptoManager;
  static std::mutex app_mutex_;
  std::vector<int> broadcast_fd_list;

protected:
  app() = default;
  ~app(){};

private:
  static app *app_instance_;
  std::vector<std::thread> thread_pool_;
};

#endif //APP_CONTEXT_H
