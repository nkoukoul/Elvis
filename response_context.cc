//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#include "response_context.h"

std::string daytime_()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,80,"%a %b %d %H:%M:%S %Y",timeinfo);
    std::string time(buffer);

    return time;
}

std::string http_response_creator::create_response(std::string && input_data){
  std::string response;
  response.reserve(input_data.size()+1024);
  response += "HTTP/1.1 200 OK\r\n";
  response += "Date: " +  daytime_() + "\r\n"; 
  response += "Content-Type: text/html\r\n"; 
  response += "Content-Length: " + std::to_string(input_data.size()+2) + "\r\n";
  response += "\r\n";
  response += input_data;
  response += "\r\n";
  return response;
}
