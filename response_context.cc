//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

#include "response_context.h"
#include "app_context.h"

http_response_creator::http_response_creator(app * application_context):application_context_(application_context){}

void http_response_creator::create_response(int client_socket, std::unordered_map<std::string, std::string> && deserialized_input_data){
  
  std::string status;
  std::string controller_data;
  std::string sec_websocket_key;
  bool close_connection;

  if (deserialized_input_data.find("Connection")!= deserialized_input_data.end() && deserialized_input_data["Connection"] == "Upgrade"
    && deserialized_input_data.find("Upgrade")!= deserialized_input_data.end() && deserialized_input_data["Upgrade"] == "websocket"){
    //wsconnection
    status = "101 Switching Protocols";
    sec_websocket_key = application_context_->uc_->generate_ws_key(deserialized_input_data["Sec-WebSocket-Key"]);
    close_connection = false;
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
  response += "Date: " +  application_context_->uc_->daytime_() + "\r\n";
  if (status == "101 Switching Protocols"){
    response += "Upgrade: websocket\r\n";
    response += "Connection: Upgrade\r\n";
    response += "Sec-WebSocket-Accept: " + sec_websocket_key + "\r\n";
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

  std::cout << "response is : \n" << response << "\n";
  
  return application_context_->http_ioc_->do_write(client_socket, std::move(response), close_connection);
}

websocket_response_creator::websocket_response_creator(app * application_context):application_context_(application_context){}

void websocket_response_creator::create_response(int client_socket, std::unordered_map<std::string, std::string> && deserialized_input_data){}
