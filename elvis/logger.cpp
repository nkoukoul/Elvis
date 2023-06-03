//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#include "logger.h"
#include <cassert>
#include <iostream>

Elvis::Logger::Logger(const std::string& filename, Elvis::LogLevel level): m_Filename{filename}, m_Level{level}
{
    assert(!m_Filename.empty() && "Log filename is not empty");
    m_LogFile.open(filename, std::ios::out | std::ios::binary | std::ios::ate | std::ios::app);
}

Elvis::Logger::~Logger()
{
    m_LogFile.close();
}

inline const char *ToString(Elvis::LogLevel level)
{
    switch (level)
    {
    case Elvis::LogLevel::DEBUG:
        return "Debug";
    case Elvis::LogLevel::INFO:
        return "Info";
    case Elvis::LogLevel::WARNING:
        return "Warning";
    default:
        return "Error";
    }
}

void Elvis::Logger::Log(Elvis::LogLevel logLevel, std::string message)
{
    std::lock_guard<std::mutex> guard(m_LogLock);
#ifdef CONSOLE
    std::cout << daytime_() << " - " << ToString(logLevel) << " - "
              << message << "\n";
#else
    if (!(logLevel < m_Level))
    {
        m_LogFile << daytime_() << " - " << ToString(logLevel) << " - "
                  << message << std::endl;
    }
#endif
}
