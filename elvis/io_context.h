//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include <string>
#include <vector>
#include <thread>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <fcntl.h>
#include "request_context.h"
#include "response_context.h"
#include "queue.h"

class app;

namespace Elvis
{
  class ClientContext
  {
  public:
    ClientContext() = default;
    int m_ClientSocket;
    bool m_ShouldCloseConnection;
    bool m_IsWebsocketConnection;
    bool m_IsHandshakeCompleted;
    size_t m_HttpBytesSend;
    size_t m_WSBytesSend;
    std::string m_HttpMessage;
    std::string m_HttpResponse;
    std::string m_WSMessage;
    std::string m_WSResponse;
    std::unordered_map<std::string, std::string> m_HttpHeaders;
    std::string m_WSData;
  };

  class IOContext
  {
  public:
    virtual ~IOContext() = default;

    virtual void Run() = 0;

    virtual void HandleConnections() = 0;

    virtual void DoRead(std::shared_ptr<ClientContext> c_ctx) = 0;

    virtual void DoWrite(std::shared_ptr<ClientContext> c_ctx) = 0;
  };

  class TCPContext final: public IOContext
  {
  public:
    TCPContext(std::string ipaddr, int port, app *ac);

    void Run() override;

    void HandleConnections() override;

    void DoRead(std::shared_ptr<ClientContext> c_ctx) override;

    void DoWrite(std::shared_ptr<ClientContext> c_ctx) override;

    app *ac_;

  private:
    std::string ipaddr_;
    int port_;
    int server_sock_;
    static const int MAXBUF = 1024;
  };
}
#endif // IO_CONTEXT_H
