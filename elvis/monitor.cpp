#include "monitor.h"
#include <iostream>

using namespace Elvis;

ConnectionMonitor::ConnectionMonitor()
{}

void ConnectionMonitor::AddConnection(std::shared_ptr<ClientContext> connection)
{
    std::lock_guard<std::mutex> lock(m_ConnectionMonitorLock);
    m_ActiveConnections.insert(connection);
    DisplayOpenConnections();
    return;
}

void ConnectionMonitor::RemoveConnection(std::shared_ptr<ClientContext> connection)
{
    std::lock_guard<std::mutex> lock(m_ConnectionMonitorLock);
    m_ActiveConnections.erase(connection);
    DisplayOpenConnections();
    return;
}

inline const char *ToString(SocketState state)
{
    switch (state)
    {
    case SocketState::CONNECTED:
        return "Connected";
    case SocketState::READING:
        return "Reading";
    case SocketState::WRITING:
        return "Writng";
    default:
        return "Unknown";
    }
}

void ConnectionMonitor::DisplayOpenConnections()
{
    // std::lock_guard<std::mutex> lock(m_ConnectionMonitorLock);
    std::cout << "**************** Connection Monitor *****************\n";
    for (const auto &connection : m_ActiveConnections) 
    {
        std::cout << "Socket " << connection->m_ClientSocket << " on State " << ToString(connection->m_State) << "\n";
    }
    std::cout << "******************************************************\n";
}