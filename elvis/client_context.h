//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

#include <string>
#include <unordered_map>

namespace Elvis
{
enum class SocketState
{
    CONNECTED = 0,
    READING = 1,
    WRITING = 2
};

struct ClientContext
{
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
    SocketState m_State;
};
} // namespace Elvis
#endif // CLIENT_CONTEXT_H