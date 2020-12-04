//
// Copyright (c) 2020 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md) 
// 
// repository: https://github.com/nkoukoul/Elvis
//

//#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "io_context.h"
#include "app_context.h"

int socket_timeout = 3000;

tcp_server::tcp_server(std::string ipaddr, int port): 
  ipaddr_(ipaddr), port_(port)
{
  struct sockaddr_in server; 
    
  server_sock_ = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ipaddr_.c_str());
  server.sin_port = htons(port_);

  if (bind(server_sock_, (struct sockaddr *) &server, sizeof(server))<0){
    std::cout << "bind failed\n";
    exit(1);
  }
    
  listen(server_sock_, 5);
}

void tcp_server::run(app * ac){
  accept_connections(ac);
}
  
void tcp_server::accept_connections(app * ac){
  struct sockaddr_in client; 
  socklen_t client_len;
  
  client_len = sizeof client;
  
  //return accept4(server_sock_, (struct sockaddr *)&client, &client_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  while(true){
    int client_socket = accept(server_sock_, (struct sockaddr *)&client, &client_len);
    if (client_socket < 0){
      std::cout << "Error while accepting connection";
      continue;
    }
    
    std::string input_data = do_read(client_socket, ac);
    if (input_data.empty()){
      continue;
    }
    
    std::unordered_map<std::string, std::string>  deserialized_input_data = ac->req_->do_parse(std::move(input_data));
    for (auto elem : deserialized_input_data)
      std::cout << "key " << elem.first << " value " << elem.second << " with size " << elem.second.size() << "\n";
    //wsconnection
    if (deserialized_input_data.find("Connection")!= deserialized_input_data.end() && deserialized_input_data["Connection"] == "Upgrade"
	&& deserialized_input_data.find("Upgrade")!= deserialized_input_data.end() && deserialized_input_data["Upgrade"] == "websocket"){
      ac->ws_ioc_->register_socket(client_socket, std::move(deserialized_input_data));
      continue; //do not close the socket
    }
      
    i_controller * ic = ac->rm_->get_controller(deserialized_input_data["url"], deserialized_input_data["request_type"]);
    
    std::string output_data;
    std::string controller_data;
    std::string status;
    if (ic){
      status = "200 OK";
      controller_data = ic->run(std::move(deserialized_input_data), ac);
    }else{
      status = "400 Bad Request";
      controller_data = "Url or method not supported";
    }
    output_data = ac->res_->do_create_response(std::move(controller_data), std::move(status));
    do_write(client_socket, ac, std::move(output_data));
  }
}

std::string tcp_server::do_read(int client_socket, app * ac){
  int in = client_socket;  
  std::string input_data;
  char inbuffer[MAXBUF], *p = inbuffer;

  // Read data from client
  int bytes_read = read(in, inbuffer, MAXBUF);
  if ( bytes_read <= 0 ){
    close(client_socket);
    return {}; //client closed connection
  }

  for (int i = 0; i < bytes_read; i++) input_data += inbuffer[i];
  input_data += '\n'; //add end of line for getline
  //std::cout << "received data: \n" << input_data << "\n" << "on fd= "
  //	    << in << "\n";

  return input_data;
}

void tcp_server::do_write(int client_socket, app * ac, std::string && output_data){
  int out = client_socket;
  write(out, output_data.c_str(), output_data.size());
  close(client_socket);
  return;
}

websocket_server::websocket_server(std::string ipaddr, int port): 
  ipaddr_(ipaddr), port_(port){}

void websocket_server::run(app * ac){
  handle_connections(ac);
}

void websocket_server::handle_connections(app * ac){
  while (true){
    if (socket_state_.empty()) 
      std::this_thread::yield();
    for (auto sd_pair : socket_state_){
      std::string output_data = "hello";
      do_write(sd_pair.first, ac, std::move(output_data));
    }
  }
}

void websocket_server::register_socket(int client_socket, std::unordered_map<std::string, std::string>  && deserialized_input_data){
  std::lock_guard<std::mutex> guard(socket_state_mutex_);
  socket_state_.insert(std::make_pair(client_socket, std::move(deserialized_input_data)));
  return;
}

std::string websocket_server::do_read(int client_socket, app * ac){
  return {};
}

void websocket_server::do_write(int client_socket, app * ac, std::string && output_data){
  int out = client_socket;
  write(out, output_data.c_str(), output_data.size());
  return;
}
