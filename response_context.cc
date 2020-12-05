//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#include "response_context.h"
#include "app_context.h"
#include <iostream>

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

http_response_creator::http_response_creator(app * application_context):application_context_(application_context){}

void http_response_creator::create_response(int client_socket, std::unordered_map<std::string, std::string> && deserialized_input_data){
  
  std::string status;
  std::string controller_data;
  bool close_connection;

  if (deserialized_input_data.find("Connection")!= deserialized_input_data.end() && deserialized_input_data["Connection"] == "Upgrade"
    && deserialized_input_data.find("Upgrade")!= deserialized_input_data.end() && deserialized_input_data["Upgrade"] == "websocket"){
    //wsconnection
    status = "101 Switching Protocols";
    close_connection = false;
    //output_data = ac_->res_->do_create_response(std::move(controller_data), std::move(status), deserialized_input_data["Sec-WebSocket-Key"]);
    //ac_->ws_ioc_->register_socket(client_socket, std::move(deserialized_input_data));
  } else {
    //http connection
    i_controller * ic = application_context_->rm_->get_controller(deserialized_input_data["url"], deserialized_input_data["request_type"]);
  
    if (ic){
      status = "200 OK";
      controller_data = ic->run(std::move(deserialized_input_data), application_context_);
    }else{
      status = "400 Bad Request";
      controller_data = "Url or method not supported";
    }
    close_connection = true;
  }
  
  std::string response;
  response.reserve(controller_data.size()+1024);
  response += "HTTP/1.1 "+ status +"\r\n";
  response += "Date: " +  daytime_() + "\r\n";
  if (status == "101 Switching Protocols"){
    response += "Connection: Upgrade\r\n";
    response += "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n";
  }else{
    response += "Connection: close\r\n";
  }
  
  if (!controller_data.empty()){
    response += "Content-Type: text/html\r\n"; 
  }
  
  if (!controller_data.empty()){
    response += "Content-Length: " + std::to_string(controller_data.size()+2) + "\r\n";
  }
  response += "\r\n";
  if (!controller_data.empty())
    response += controller_data + "\r\n";

  return application_context_->http_ioc_->do_write(client_socket, std::move(response), close_connection);
}
