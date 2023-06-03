//
// Copyright (c) 2020-2023 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
// com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef LOGGER_H
#define LOGGER_H

#include "utils.h"
#include <fstream>
#include <mutex>
#include <string>

namespace Elvis
{
  enum class LogLevel
  {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
  };

  class ILogger
  {
  public:
    virtual ~ILogger() = default;

    virtual void Log(LogLevel logLevel, std::string message) = 0;
  };

  class Logger final : public ILogger
  {
  public:
    Logger() = delete;
    Logger(const std::string& filename, LogLevel level);
    ~Logger();

    virtual void Log(LogLevel logLevel, std::string message) override;

  private:
    LogLevel m_Level;
    std::mutex m_LogLock;
    std::ofstream m_LogFile;
    const std::string m_Filename;
  };
} // namespace Elvis
#endif // LOGGER_H