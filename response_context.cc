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
#include <bitset>
#include <vector>
#include <openssl/sha.h>

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

int binary_to_decimal(std::string binary_num)
{
  int dec_value = 0;
 
  int base = 1;
 
  for (int i = binary_num.length() - 1; i >= 0; i--) {
    if (binary_num[i] == '1')
      dec_value += base;
    base = base * 2;
  }
    
  return dec_value;
}

std::string base_64_encode(std::string input){
  
  std::vector<char> base64_lookup = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
  };
  
  std::string bstring;
  for (int i = 0; i < input.size(); i++){
    std::bitset<8> bb(input[i]);
    bstring += bb.to_string();
  }
  
  std::string padding;

  if (bstring.size()%24 == 16){
    bstring += "00";
    padding += "=";
  }

  if (bstring.size()%24 == 8){
    bstring += "0000";
    padding += "==";
  }

  std::string output;
  
  for (int i = 0; i < bstring.size()/6; i++){
    std::string b_sub = bstring.substr(i * 6, 6);
    output += base64_lookup[binary_to_decimal(b_sub)];
  }
  output += padding;
  return output;
}

std::string generate_ws_key(std::string ws_client_key){
  
  std::string magic_ws_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  std::string concatenated_string = ws_client_key + magic_ws_string;

  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1((unsigned char*)concatenated_string.c_str(), concatenated_string.size(), hash);
  std::string input;
  for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
    input += (char)hash[i];
  }
  return base_64_encode(input);
}

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
    std::cout << "WEBSOCKET KEY IS " << deserialized_input_data["Sec-WebSocket-Key"] << "\n";
    sec_websocket_key = generate_ws_key(deserialized_input_data["Sec-WebSocket-Key"]);
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
  response += "Date: " +  daytime_() + "\r\n";
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
