//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot
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

class ILogger
{
public:
  virtual ~ILogger() = default;

  virtual void Log(std::string caller, std::string log_level,
                   std::string message) = 0;
};

class Logger final : public ILogger
{
public:
  Logger(std::string filename)
      : m_LogFile(filename, std::ios::out | std::ios::binary | std::ios::ate |
                                std::ios::app){};

  void Log(std::string caller, std::string log_level,
           std::string message) override
  {
    std::lock_guard<std::mutex> guard(m_LogLock);
    m_LogFile << daytime_() << " - " << log_level << " - " << caller << " - "
              << message << std::endl;
  }

private:
  std::mutex m_LogLock;
  std::ofstream m_LogFile;
  std::string Filename;
};

#endif // LOGGER_H