#include "monitor.h"
#include <iostream>

using namespace Elvis;
using namespace std::chrono_literals;

ConnectionMonitor::ConnectionMonitor(size_t intervalInSeconds) : m_Interval{intervalInSeconds}, m_Quit{false}
{
}

void ConnectionMonitor::Run()
{
    m_Thread = std::thread([this]()
                           {
        std::unique_lock<std::mutex> lock(this->m_ConnectionMonitorLock);
        do 
        {
             m_CV.wait_for(lock, this->m_Interval * 1s, [this]
             {
                return (this->m_Quit);
             });
            if (!this->m_Quit)
            {
                this->DisplayOpenConnections();
            }
        } while (!this->m_Quit);
        std::cout << "************ Connection Monitor: Quitting ************\n"; });
}

ConnectionMonitor::~ConnectionMonitor()
{
    std::unique_lock<std::mutex> lock(this->m_ConnectionMonitorLock);
    m_Quit = true;
    m_CV.notify_one();

    if (m_Thread.joinable())
    {
        m_Thread.join();
    }
}

void ConnectionMonitor::AddConnection(std::shared_ptr<ClientContext> connection)
{
    {
        std::lock_guard<std::mutex> lock(m_ConnectionMonitorLock);
        m_ActiveConnections.insert(connection);
    }
    m_CV.notify_one();
}

void ConnectionMonitor::RemoveConnection(std::shared_ptr<ClientContext> connection)
{
    {
        std::lock_guard<std::mutex> lock(m_ConnectionMonitorLock);
        m_ActiveConnections.erase(connection);
    }
    m_CV.notify_one();
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

void ConnectionMonitor::DisplayOpenConnections() const
{
    std::cout << "**************** Connection Monitor *****************\n";
    for (const auto &connection : m_ActiveConnections)
    {
        std::cout << "Socket " << connection->m_ClientSocket << " on State " << ToString(connection->m_State) << "\n";
    }
    std::cout << "*****************************************************\n";
}