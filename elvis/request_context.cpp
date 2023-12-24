//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//
#include "request_context.h"
#include <sstream>

using namespace Elvis;
HttpRequestParser::HttpRequestParser(
    std::unique_ptr<HttpResponseContext> httpResponseContext,
    std::shared_ptr<IQueue> concurrentQueue,
    std::shared_ptr<RouteManager> routeManager)
    : m_HttpResponseContext{std::move(httpResponseContext)}
    , m_ConcurrentQueue{concurrentQueue}
    , m_RouteManager{routeManager}
{
}

void HttpRequestParser::Parse(std::shared_ptr<ClientContext> c_ctx) const
{
#ifdef DEBUG
    std::cout << "HttpRequestParser::Parse: " << c_ctx->m_HttpMessage << "\n";
#endif
    std::istringstream ss(c_ctx->m_HttpMessage);
    std::string request_type, url, protocol, line;

    // first line
    std::getline(ss, line);
    std::istringstream first_line(line);
    first_line >> request_type >> url >> protocol;

    c_ctx->m_HttpHeaders.insert(std::make_pair("request_type", std::move(request_type)));
    c_ctx->m_HttpHeaders.insert(std::make_pair("url", std::move(url)));
    c_ctx->m_HttpHeaders.insert(std::make_pair("protocol", std::move(protocol)));

    // headers here, read until first empty line.
    while (line.size() > 1)
    {
        std::getline(ss, line);
        std::size_t column_index = line.find_first_of(":");
        if (column_index != std::string::npos)
            c_ctx->m_HttpHeaders.insert(std::make_pair(
                line.substr(0, column_index),
                line.substr(column_index + 2, line.size() - 3 - column_index)));
    }

    if (c_ctx->m_HttpHeaders["request_type"] == "POST")
    {
        std::string deserialized_data;
        // json for post
        while (std::getline(ss, line))
        {
            deserialized_data += line;
        }
        c_ctx->m_HttpHeaders.insert(std::make_pair("data", std::move(deserialized_data)));
    }

    IController* ic = m_RouteManager->GetController(c_ctx->m_HttpHeaders["url"], c_ctx->m_HttpHeaders["request_type"]);

    if (ic)
    {
        c_ctx->m_HttpHeaders["status"] = "200 OK";
        c_ctx->m_HttpHeaders["controller_data"] = "";
        // Run Controller
        ic->Run(c_ctx);
        ////////////////
    }
    else
    {
        c_ctx->m_HttpHeaders["status"] = "400 Bad Request";
        c_ctx->m_HttpHeaders["controller_data"] = "Url or method not supported";
    }
    auto weakSelf = weak_from_this();
    m_ConcurrentQueue->DispatchAsync(
        [weakSelf, c_ctx]() {
            auto self = weakSelf.lock();
            if (self)
            {
                self->m_HttpResponseContext->DoCreateResponse(c_ctx);
            }
        },
        "HTTPRequestParser::Parse -> HTTPResponseContext::DoCreateResponse");
}

WebsocketRequestParser::WebsocketRequestParser(
    std::unique_ptr<WebsocketResponseContext> wsResponseContext,
    std::shared_ptr<IQueue> concurrentQueue)
    : m_WSResponseContext{std::move(wsResponseContext)}
    , m_ConcurrentQueue{concurrentQueue}
{
}

void WebsocketRequestParser::Parse(std::shared_ptr<ClientContext> c_ctx) const
{
    int fin, rsv1, rsv2, rsv3, opcode, mask, i = 0;
    uint64_t payload_len;
    unsigned char mask_key[4];
    uint8_t octet = (uint8_t)c_ctx->m_WSMessage[i++]; // 8 bit char
    fin = (octet >> 7) & 0x01;
    rsv1 = (octet >> 6) & 0x01;
    rsv2 = (octet >> 5) & 0x01;
    rsv3 = (octet >> 4) & 0x01;
    opcode = octet & 0x0F;
    octet = (uint8_t)c_ctx->m_WSMessage[i++]; // 8 bit char
    mask = (octet >> 7) & 0x01;
    payload_len = (octet)&0x7F;
    std::string unmasked_payload_data;
    std::string masked_payload_data;
    if (opcode == 8)
    { // close received
        c_ctx->m_ShouldCloseConnection = true;
    }
    if (payload_len == 126)
    {
        payload_len = ((uint16_t)c_ctx->m_WSMessage[i] << 8 & 0xff00) + ((uint16_t)c_ctx->m_WSMessage[i + 1] & 0x00ff);
        i += 2;
    }
    else if (payload_len == 127)
    {
        payload_len = ((uint64_t)c_ctx->m_WSMessage[i] << 56 & 0xff00000000000000) +
                      ((uint64_t)c_ctx->m_WSMessage[i + 1] << 48 & 0x00ff000000000000) +
                      ((uint64_t)c_ctx->m_WSMessage[i + 2] << 40 & 0x0000ff0000000000) +
                      ((uint64_t)c_ctx->m_WSMessage[i + 3] << 32 & 0x000000ff00000000) +
                      ((uint64_t)c_ctx->m_WSMessage[i + 4] << 24 & 0x00000000ff000000) +
                      ((uint64_t)c_ctx->m_WSMessage[i + 5] << 16 & 0x0000000000ff0000) +
                      ((uint64_t)c_ctx->m_WSMessage[i + 6] << 8 & 0x000000000000ff00) +
                      ((uint64_t)c_ctx->m_WSMessage[i + 7] & 0x00000000000000ff);
        i += 8;
    }
    if (mask == 1)
    {
        mask_key[0] = c_ctx->m_WSMessage[i];
        mask_key[1] = c_ctx->m_WSMessage[i + 1];
        mask_key[2] = c_ctx->m_WSMessage[i + 2];
        mask_key[3] = c_ctx->m_WSMessage[i + 3];
        i += 4;
    }
    masked_payload_data = c_ctx->m_WSMessage.substr(i);
    for (uint64_t j = 0; j < payload_len; j++)
    {
        unmasked_payload_data += masked_payload_data[j] ^ (mask_key[(j % 4)]);
    }
    // echo functionality for now
    c_ctx->m_WSData = std::move(unmasked_payload_data);
    auto weakSelf = weak_from_this();
    m_ConcurrentQueue->DispatchAsync(
        [weakSelf, c_ctx]() {
            auto self = weakSelf.lock();
            if (self)
            {
                self->m_WSResponseContext->DoCreateResponse(c_ctx);
            }
        },
        "WebsocketRequestParser::Parse -> WebsocketResponseContext::DoCreateResponse");
}
