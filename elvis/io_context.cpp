//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "io_context.h"
#include "app_context.h"
#include "request_context.h"
#include <arpa/inet.h>
#include <bitset>
#include <csignal>
#include <future>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Elvis
{
inline void non_block_socket(int sd)
{
    /* set O_NONBLOCK on fd */
    int flags = fcntl(sd, F_GETFL, 0);
    if (flags == -1)
    {
        std::cout << "Socket flags set failed\n";
        std::exit(EXIT_FAILURE);
    }
    if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        std::cout << "Socket set to NonBlock failed\n";
        std::exit(EXIT_FAILURE);
    }
}

inline void non_block_with_timeout(int sd)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;

    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
    {
        std::cout << "Socket set timeout failed\n";
        std::exit(EXIT_FAILURE);
    }
}

class TCPServer final : public std::enable_shared_from_this<TCPServer>, public IServer
{
private:
    std::string m_IPAddress;
    int m_Port;
    std::shared_ptr<InputContext> m_TCPInputContext;
    std::shared_ptr<IQueue> m_SerialQueue;
    std::shared_ptr<ILogger> m_Logger;
    std::shared_ptr<IConnectionMonitor> m_ConnectionMonitor;
    int m_ServerSocket{-1};
    static const int MAXBUF = 1024;

public:
    TCPServer(
        std::string ipAddress,
        int port,
        std::shared_ptr<InputContext> tcpInputContext,
        std::shared_ptr<IQueue> serialQueue,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConnectionMonitor> connectionMonitor)
        : m_IPAddress{ipAddress}
        , m_Port{port}
        , m_TCPInputContext{tcpInputContext}
        , m_SerialQueue{serialQueue}
        , m_Logger{logger}
        , m_ConnectionMonitor{connectionMonitor}
    {
    }

    virtual void Run() override
    {
        struct sockaddr_in server;
        m_ServerSocket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
#ifdef __APPLE__
        non_block_socket(m_ServerSocket);
#else
        non_block_with_timeout(m_ServerSocket);
#endif
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(m_IPAddress.c_str());
        server.sin_port = htons(m_Port);
        if (bind(m_ServerSocket, (struct sockaddr*)&server, sizeof(server)) < 0)
        {
            m_Logger->Log(LogLevel::ERROR, "Server failed to bind socket, terminating application…");
            exit(1);
        }
        listen(m_ServerSocket, 10);
        auto weakSelf = weak_from_this();
        m_SerialQueue->DispatchAsync(
            [weakSelf]() {
                auto self = weakSelf.lock();
                if (self != nullptr)
                {
                    self->HandleConnections();
                }
            },
            "TCPContext::HandleConnections -> TCPContext::HandleConnections");
    }

    virtual void HandleConnections() override
    {
        struct sockaddr_in client;
        socklen_t clientSocketLength;
        clientSocketLength = sizeof client;
        int clientSocket = accept(m_ServerSocket, (struct sockaddr*)&client, &clientSocketLength);
        if (clientSocket <= 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                m_Logger->Log(LogLevel::DETAIL, "No incoming connections.");
            }
            else
            {
                m_Logger->Log(LogLevel::ERROR, "Error while accepting connection.");
            }
        }
        else
        {
            m_Logger->Log(LogLevel::INFO, "Incoming connection on socket descriptor " + std::to_string(clientSocket));
            non_block_with_timeout(clientSocket);
            std::shared_ptr<ClientContext> c_ctx = std::make_shared<ClientContext>();
            c_ctx->m_ClientSocket = clientSocket;
            c_ctx->m_State = SocketState::CONNECTED;
            m_ConnectionMonitor->AddConnection(c_ctx);
            auto weakSelf = weak_from_this();
            m_SerialQueue->DispatchAsync(
                [weakSelf, c_ctx]() {
                    auto self = weakSelf.lock();
                    if (self != nullptr)
                    {
                        self->m_TCPInputContext->DoRead(c_ctx);
                    }
                },
                "TCPContext::HandleConnections -> TCPContext::DoRead");
        }
        auto weakSelf = weak_from_this();
        m_SerialQueue->DispatchAsync(
            [weakSelf]() {
                auto self = weakSelf.lock();
                if (self != nullptr)
                {
                    self->HandleConnections();
                }
            },
            "TCPContext::HandleConnections -> TCPContext::HandleConnections");
    }
};

class TCPInputContext final : public std::enable_shared_from_this<TCPInputContext>,
                              public InputContext,
                              public InputContextDelegate
{
private:
    std::shared_ptr<IRequestContext> m_HTTPRequestContext;
    std::shared_ptr<IRequestContext> m_WSRequestContext;
    std::shared_ptr<IQueue> m_SerialQueue;
    std::shared_ptr<ILogger> m_Logger;
    std::shared_ptr<IConnectionMonitor> m_ConnectionMonitor;
    static const int MAXBUF = 1024;

public:
    TCPInputContext(
        std::shared_ptr<IRequestContext> httpRequestContext,
        std::shared_ptr<IRequestContext> wsRequestContext,
        std::shared_ptr<IQueue> serialQueue,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConnectionMonitor> connectionMonitor)
        : m_HTTPRequestContext{httpRequestContext}
        , m_WSRequestContext{wsRequestContext}
        , m_SerialQueue{serialQueue}
        , m_Logger{logger}
        , m_ConnectionMonitor{connectionMonitor}
    {
    }

    virtual void DoRead(std::shared_ptr<ClientContext> c_ctx) override
    {
        c_ctx->m_State = SocketState::READING;
        char inbuffer[MAXBUF], *p = inbuffer;
        // Read data from client
        int bytes_read = read(c_ctx->m_ClientSocket, inbuffer, MAXBUF);
        m_Logger->Log(
            LogLevel::DETAIL,
            "TCPContext::DoRead: Incoming Socket byte count " + std::to_string(bytes_read) + " on socket " +
                std::to_string(c_ctx->m_ClientSocket));
        // Client closed connection
        if (bytes_read == 0)
        {
            m_Logger->Log(
                LogLevel::DETAIL,
                "TCPContext::DoRead: No bytes closing socket " + std::to_string(c_ctx->m_ClientSocket));
            close(c_ctx->m_ClientSocket);
            m_ConnectionMonitor->RemoveConnection(c_ctx);
            return;
        }
        if (bytes_read < 0)
        {
            // Client read block
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // if is websocket and has data proceed to parsing
                if (c_ctx->m_IsWebsocketConnection && c_ctx->m_WSMessage.size())
                {
                    m_Logger->Log(LogLevel::DETAIL, "TCPContext::DoRead: Websocket Read Finished with data.");
                    m_WSRequestContext->DoParse(c_ctx);
                    return;
                } // if is http and has data proceed to parsing
                else if (!c_ctx->m_IsWebsocketConnection && c_ctx->m_HttpMessage.size())
                {
                    m_Logger->Log(LogLevel::DETAIL, "TCPContext::DoRead: HTTP Read Finished with data");
                    c_ctx->m_HttpMessage += '\n'; // add end of line for getline
                    m_HTTPRequestContext->DoParse(c_ctx);
                    return;
                } // read simply blocked with no data try again
                else
                {
                    m_Logger->Log(LogLevel::DETAIL, "TCPContext::DoRead: Read Blocked will try again");
                    auto weakSelf = weak_from_this();
                    m_SerialQueue->DispatchAsync(
                        [weakSelf, c_ctx]() {
                            auto self = weakSelf.lock();
                            if (self)
                            {
                                self->DoRead(c_ctx);
                            }
                        },
                        "TCPContext::DoRead -> TCPContext::DoRead");
                }
                return;
            }
            else
            {
                // TCP read error
                m_Logger->Log(
                    LogLevel::ERROR,
                    "TCPContext::DoRead: Read Error, closing socket " + std::to_string(c_ctx->m_ClientSocket));
                close(c_ctx->m_ClientSocket);
                m_ConnectionMonitor->RemoveConnection(c_ctx);
                return;
            }
        }
        else // We have data to read
        {
            if (c_ctx->m_IsWebsocketConnection)
            {
                m_Logger->Log(
                    LogLevel::INFO,
                    "TCPContext::DoRead: Websocket " + std::to_string(c_ctx->m_ClientSocket) + " has " +
                        std::to_string(bytes_read) + " bytes to read");
                for (int i = 0; i < bytes_read; i++)
                {
                    c_ctx->m_WSMessage += inbuffer[i];
                }
            }
            else
            {
                m_Logger->Log(
                    LogLevel::INFO,
                    "TCPContext::DoRead: HTTP socket " + std::to_string(c_ctx->m_ClientSocket) + " has " +
                        std::to_string(bytes_read) + " bytes to read");
                for (int i = 0; i < bytes_read; i++)
                {
                    c_ctx->m_HttpMessage += inbuffer[i];
                }
            }
            // maybe there is some more data so add another read to the queue
            auto weakSelf = weak_from_this();
            m_SerialQueue->DispatchAsync(
                [weakSelf, c_ctx]() {
                    auto self = weakSelf.lock();
                    if (self)
                    {
                        self->DoRead(c_ctx);
                    }
                },
                "TCPContext::DoRead -> TCPContext::DoRead");
        }
    }

#pragma mark InputContextDelegate
    virtual void Read(std::shared_ptr<ClientContext> c_ctx) override
    {
        DoRead(c_ctx);
    }
};

class TCPOutputContext final : public std::enable_shared_from_this<TCPOutputContext>, public OutputContext
{
private:
    std::shared_ptr<IQueue> m_SerialQueue;
    std::shared_ptr<ILogger> m_Logger;
    std::shared_ptr<IConnectionMonitor> m_ConnectionMonitor;
    std::weak_ptr<InputContextDelegate> m_TCPInputDelegate;
    static const int MAXBUF = 1024;

public:
    TCPOutputContext(
        std::shared_ptr<IQueue> serialQueue,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IConnectionMonitor> connectionMonitor)
        : m_SerialQueue{serialQueue}
        , m_Logger{logger}
        , m_ConnectionMonitor{connectionMonitor}
    {
    }

    virtual void SetTCPInputDelegate(std::weak_ptr<InputContextDelegate> inputDelegate) override
    {
        m_TCPInputDelegate = inputDelegate;
    }

    virtual void DoWrite(std::shared_ptr<ClientContext> c_ctx) override
    {
        c_ctx->m_State = SocketState::WRITING;
        int bytes_write;

        if (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted)
        {

            m_Logger->Log(
                LogLevel::DETAIL,
                "TCPContext::DoWrite: WS Response " + c_ctx->m_WSResponse + "\n with size " +
                    std::to_string(c_ctx->m_WSResponse.size()));
            size_t bytes_to_send;
            if (MAXBUF > (c_ctx->m_WSResponse.size() - c_ctx->m_WSBytesSend))
            {
                bytes_to_send = c_ctx->m_WSResponse.size() - c_ctx->m_WSBytesSend;
            }
            else
            {
                bytes_to_send = MAXBUF;
            }
            bytes_write =
                write(c_ctx->m_ClientSocket, c_ctx->m_WSResponse.c_str() + c_ctx->m_WSBytesSend, bytes_to_send);
            m_Logger->Log(
                LogLevel::INFO,
                "TCPContext::DoWrite: WS Response wrote " + std::to_string(bytes_write) + " bytes on socket " +
                    std::to_string(c_ctx->m_ClientSocket));
        }
        else
        {
            m_Logger->Log(
                LogLevel::DETAIL,
                "TCPContext::DoWrite: HTTP Response " + c_ctx->m_HttpResponse + "\n with size " +
                    std::to_string(c_ctx->m_HttpResponse.size()));
            size_t bytes_to_send;

            if (MAXBUF > (c_ctx->m_HttpResponse.size() - c_ctx->m_HttpBytesSend))
            {
                bytes_to_send = c_ctx->m_HttpResponse.size() - c_ctx->m_HttpBytesSend;
            }
            else
            {
                bytes_to_send = MAXBUF;
            }
            bytes_write =
                write(c_ctx->m_ClientSocket, c_ctx->m_HttpResponse.c_str() + c_ctx->m_HttpBytesSend, bytes_to_send);
            m_Logger->Log(
                LogLevel::INFO,
                "TCPContext::DoWrite: HTTP Response wrote " + std::to_string(bytes_write) + " bytes on socket " +
                    std::to_string(c_ctx->m_ClientSocket));
        }

        // Client closed connection
        if (bytes_write == 0)
        {
            m_Logger->Log(
                LogLevel::INFO,
                "TCPContext::DoWrite: No bytes written, Client closed "
                "connection, closing connection on socket " +
                    std::to_string(c_ctx->m_ClientSocket));
            close(c_ctx->m_ClientSocket);
            m_ConnectionMonitor->RemoveConnection(c_ctx);
            return;
        }

        // Error handling
        if (bytes_write < 0)
        { // Write block try again
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                m_Logger->Log(
                    LogLevel::DETAIL,
                    "TCPContext::DoWrite: Write block will try again on socket " +
                        std::to_string(c_ctx->m_ClientSocket));
                auto weakSelf = weak_from_this();
                m_SerialQueue->DispatchAsync(
                    [weakSelf, c_ctx]() {
                        auto self = weakSelf.lock();
                        if (self)
                        {
                            self->DoWrite(c_ctx);
                        }
                    },
                    "TCPContext::DoWrite -> TCPContext::DoWrite");
            }
            else
            {
                // TCP write error
                m_Logger->Log(
                    LogLevel::ERROR,
                    "TCPContext::DoWrite: Write error closing connection on socket " +
                        std::to_string(c_ctx->m_ClientSocket));
                close(c_ctx->m_ClientSocket);
                m_ConnectionMonitor->RemoveConnection(c_ctx);
            }
            return;
        }

        // Check if we need to transfer more data
        if (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted)
        {
            c_ctx->m_WSBytesSend += bytes_write;
        }
        else
        {
            c_ctx->m_HttpBytesSend += bytes_write;
        }

        if ((!c_ctx->m_IsWebsocketConnection && c_ctx->m_HttpBytesSend < c_ctx->m_HttpResponse.size()) ||
            (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted &&
             c_ctx->m_WSBytesSend < c_ctx->m_WSResponse.size()))
        {
            m_Logger->Log(
                LogLevel::INFO,
                "TCPContext::DoWrite: There is more data to write on socket " + std::to_string(c_ctx->m_ClientSocket));
            auto weakSelf = weak_from_this();
            m_SerialQueue->DispatchAsync(
                [weakSelf, c_ctx]() {
                    auto self = weakSelf.lock();
                    if (self)
                    {
                        self->DoWrite(c_ctx);
                    }
                },
                "TCPContext::DoWrite -> TCPContext::DoWrite");
            return;
        }
        // Check if socket should close
        if (c_ctx->m_ShouldCloseConnection)
        {
            m_Logger->Log(
                LogLevel::INFO,
                "TCPContext::DoWrite: Should close socket " + std::to_string(c_ctx->m_ClientSocket));
            close(c_ctx->m_ClientSocket);
            m_ConnectionMonitor->RemoveConnection(c_ctx);
            return;
        }

        // if socket is still open with http keep-alive or ws read again
        if (c_ctx->m_IsWebsocketConnection)
        {
            m_Logger->Log(
                LogLevel::INFO,
                "TCPContext::DoWrite: WS connection on socket " + std::to_string(c_ctx->m_ClientSocket) +
                    " will schedule another read");
            if (!c_ctx->m_IsHandshakeCompleted)
            {
                // Add socket to broadcast list, disabled atm.
                // ac_->broadcast_fd_list.push_back(c_ctx->m_ClientSocket);
                c_ctx->m_IsHandshakeCompleted = true;
            }
            c_ctx->m_WSMessage.clear();
            c_ctx->m_WSData.clear();
            c_ctx->m_WSResponse.clear();
            c_ctx->m_WSBytesSend = 0;
        }
        else
        {
            m_Logger->Log(
                LogLevel::INFO,
                "TCPContext::DoWrite: HTTP with keepAlive connection on socket " +
                    std::to_string(c_ctx->m_ClientSocket) + " will schedule another read");
            c_ctx->m_HttpMessage.clear();
            c_ctx->m_HttpBytesSend = 0;
        }
        auto weakSelf = weak_from_this();
        m_SerialQueue->DispatchAsync(
            [weakSelf, c_ctx]() {
                auto self = weakSelf.lock();
                if (self)
                {
                    auto tcpInputDelegate = self->m_TCPInputDelegate.lock();
                    if (tcpInputDelegate != nullptr)
                    {
                        tcpInputDelegate->Read(c_ctx);
                    }
                }
            },
            "TCPContext::DoWrite -> TCPContext::DoRead");
        return;
    }
};

std::shared_ptr<IServer> CreateTCPServer(
    std::string ipAddress,
    int port,
    std::shared_ptr<RouteManager> routeManager,
    std::shared_ptr<ICryptoManager> cryptoManager,
    std::shared_ptr<IQueue> concurrentQueue,
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IConnectionMonitor> connectionMonitor)
{
    auto serialQueue = std::make_shared<SerialQueue>();
    auto tcpOutputContext = std::make_shared<TCPOutputContext>(serialQueue, logger, connectionMonitor);
    auto httpResponseContext = std::make_unique<HttpResponseContext>(tcpOutputContext, cryptoManager);
    auto httpRequestContext = std::make_shared<HttpRequestContext>(std::move(httpResponseContext), routeManager);
    auto wsResponseContext = std::make_unique<WebsocketResponseContext>(tcpOutputContext);
    auto wsRequestContext = std::make_shared<WebsocketRequestContext>(std::move(wsResponseContext));
    auto tcpInputContext =
        std::make_shared<TCPInputContext>(httpRequestContext, wsRequestContext, serialQueue, logger, connectionMonitor);
    tcpOutputContext->SetTCPInputDelegate(tcpInputContext);
    return std::make_shared<TCPServer>(ipAddress, port, tcpInputContext, serialQueue, logger, connectionMonitor);
}
} // namespace Elvis
