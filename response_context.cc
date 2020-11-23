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

std::string nkou_response_creator::create_response(std::string && input_data){

  std::string page = "<!doctype html>"
    "<html>"
    "<head>"
    "<title>This is the title of the webpage!</title>"
    "</head>"
    "<body>"
    "<p>This is an example paragraph. Anything in the <strong>body</strong> tag will appear on the page, just like this <strong>p</strong> tag and its contents.</p>"
    "</body>"
    "</html>";

  std::string response;
  response.reserve(page.size()+1024);
  response += "HTTP/1.1 200 OK\r\n";
  response += "Date: " +  daytime_() + "\r\n"; 
  response += "Content-Type: text/html\r\n"; 
  response += "Content-Length: " + std::to_string(page.size()+2) + "\r\n";
  response += "\r\n";
  response += page;
  response += "\r\n";
  return response;
}
