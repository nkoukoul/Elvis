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

void http_response_creator::create_response(int const client_socket, std::unordered_map<std::string, std::string> && deserialized_input_data) const {
  
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

  //std::cout << "response is : \n" << response << "\n";
  std::cout << "socket " << client_socket << " response created" << std::endl;
  return application_context_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&io_context::do_write, application_context_->http_ioc_.get(), client_socket, std::move(response), close_connection)));
    //return application_context_->http_ioc_->do_write(client_socket, std::move(response), close_connection);
}

websocket_response_creator::websocket_response_creator(app * application_context):application_context_(application_context){}

void websocket_response_creator::create_response(int const client_socket, std::unordered_map<std::string, std::string> && deserialized_input_data) const {
  bool close_connection = false;
  if (deserialized_input_data["Connection"] == "close"){
    close_connection = true;
  }
  //std::string dammy_data = "bla bla";
  unsigned int len = deserialized_input_data["data"].size();
  std::string extra_len = "";
  std::string response;
  std::bitset<16> bs;
  std::bitset<4> b_opcode(1); 
  bs[15] = 1; //fin
  bs[14] = 0; //RSV1
  bs[13] = 0; //RSV2
  bs[12] = 0; //RSV3
  bs[11] = b_opcode[3];//opcode
  bs[10] = b_opcode[2];
  bs[9] = b_opcode[1];
  bs[8] = b_opcode[0];
  bs[7] = 0; //mask
  if (len <= 125){
    std::bitset<7> b_len(len);
    for (int i = 0; i < 7; i++){
      bs[6-i] = b_len[6-i];
    }
  }else if (len <= 65535){
    std::bitset<7> b_len(126);
    for (int i = 0; i < 7; i++){
      bs[6-i] = b_len[6-i];
    }
    extra_len += (len & 0x0000ff00) >> 8; //MSB    
    extra_len += len & 0x000000ff; //LSB    
  }else{
    std::bitset<7> b_len(127);
    /*for (int i = 0; i < 7; i++){
      bs[6-i] = b_len[6-i];
    }
    extra_len += (len & 0xff00000000000000) >> 256; //8 byte
    extra_len += (len & 0x00ff000000000000) >> 128; //7 byte    
    extra_len += (len & 0x0000ff0000000000) >> 64; //6 byte   
    extra_len += (len & 0x000000ff00000000) >> 32; //5 byte    
    extra_len += (len & 0x00000000ff000000) >> 24; //4 byte    
    extra_len += (len & 0x0000000000ff0000) >> 16; //3 byte    
    extra_len += (len & 0x000000000000ff00) >> 8; //2 byte        
    extra_len += len & 0x00000000000000ff; //1 byte    */
  }
  //std::cout << "first part " << bs.to_string() << "\n";
  unsigned int first_part = application_context_->uc_->binary_to_decimal(bs.to_string());
  response += (first_part & 0x0000ff00) >> 8; //MSB
  response += first_part & 0x000000ff; //LSB
  response += extra_len;
  response += deserialized_input_data["data"];
  //std::cout << response << "\n";
  return application_context_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&io_context::do_write, application_context_->ws_ioc_.get(), client_socket, std::move(response), close_connection)));
}
