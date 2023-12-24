//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include "controllers.h"
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Elvis
{
class RouteManager final
{
public:
    RouteManager() = default;

    IController* GetController(std::string url, std::string request_type)
    {
        // std::lock_guard<std::mutex> guard(m_RouteManagerLock);
        if (GetRoute(url, request_type))
        {
            return m_RouteMap[url][request_type].get();
        }
        else
        {
            std::size_t index = url.find_last_of("/");
            url = url.substr(0, index > 0 ? index : 0);
            if (GetRoute(url, request_type))
            {
                return m_RouteMap[url][request_type].get();
            }
        }
        return {};
    }

    void SetRoute(std::string url, std::string request_type, std::unique_ptr<IController> controller)
    {
        if (m_RouteMap.find(url) == m_RouteMap.end())
        {
            std::unordered_map<std::string, std::unique_ptr<IController>> request_types;
            request_types.insert(std::move(std::make_pair(request_type, std::move(controller))));
            m_RouteMap.insert(std::move(std::make_pair(url, std::move(request_types))));
        }
        else if (m_RouteMap[url].find(request_type) == m_RouteMap[url].end())
        {
            m_RouteMap[url].insert(std::move(std::make_pair(request_type, std::move(controller))));
        }
        else
        {
            std::cout << url << " has already a request type " << request_type << " registered\n";
        }
        return;
    }

    bool GetRoute(std::string url, std::string request_type)
    {
        return m_RouteMap.find(url) != m_RouteMap.end() && m_RouteMap[url].find(request_type) != m_RouteMap[url].end();
    }

private:
    std::mutex m_RouteManagerLock;
    std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<IController>>> m_RouteMap;
};
} // namespace Elvis
#endif // ROUTE_MANAGER_H
