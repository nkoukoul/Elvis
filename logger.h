//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

class i_logger
{
public:
    i_logger() = default;

    virtual void log(std::string caller, std::string log_level, std::string message) = 0;
};

class logger : public i_logger
{
public:
    logger(std::string filename):log_file_(filename, std::ios::out|std::ios::binary|std::ios::ate|std::ios::app){};

    void log(std::string caller, std::string log_level, std::string message) override
    {
        std::lock_guard<std::mutex> guard(log_lock_);
        log_file_ << daytime_() << " - " << log_level << " - " << caller << " - " << message << std::endl; 
    }
private:
    std::mutex log_lock_;
    std::ofstream log_file_;
    std::string filename_;
};

#endif // LOGGER_H