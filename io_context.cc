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
#include <csignal>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "io_context.h"
#include "app_context.h"


int socket_timeout = 3000;

tcp_server::tcp_server(std::string ipaddr, int port, app * ac): 
  ipaddr_(ipaddr), port_(port), ac_(ac)
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

void tcp_server::run(){
  handle_connections();
}
  
void tcp_server::handle_connections(){
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
    
    do_read(client_socket);
  }
}

void tcp_server::do_read(int client_socket){
  int in = client_socket;  
  std::string input_data;
  char inbuffer[MAXBUF], *p = inbuffer;

  // Read data from client
  int bytes_read = read(in, inbuffer, MAXBUF);
  if ( bytes_read <= 0 ){
    close(client_socket);
    return; //client closed connection
  }

  for (int i = 0; i < bytes_read; i++){
    input_data += inbuffer[i];
  }
  input_data += '\n'; //add end of line for getline
 
  return ac_->req_->do_parse(client_socket, std::move(input_data));
}


void tcp_server::do_write(int client_socket, std::string && output_data, bool close_connection){
  int out = client_socket;
  write(out, output_data.c_str(), output_data.size());
  
  //close socket if http
  if (close_connection){
    close(client_socket);
  }
  
  return;
}

websocket_server::websocket_server(std::string ipaddr, int port, app * ac): 
  ipaddr_(ipaddr), port_(port), ac_(ac){}

void websocket_server::run(){
  std::signal(SIGPIPE, SIG_IGN);
  handle_connections();
}

void websocket_server::handle_connections(){
  while (true){
    if (socket_state_.empty()) 
      std::this_thread::yield();
    for (auto sd_pair : socket_state_){
      std::string output_data = "hello";
      do_write(sd_pair.first, std::move(output_data), true);
    }
  }
}

void websocket_server::register_socket(int client_socket, std::unordered_map<std::string, std::string>  && deserialized_input_data){
  std::lock_guard<std::mutex> guard(socket_state_mutex_);
  socket_state_.insert(std::make_pair(client_socket, std::move(deserialized_input_data)));
  return;
}

void websocket_server::do_read(int client_socket){
  return;
}

void websocket_server::do_write(int client_socket, std::string && output_data, bool close_connection){
  int out = client_socket;
  if (write(out, output_data.c_str(), output_data.size()) < 0){
    close(client_socket);
    socket_state_.erase(client_socket);
  }
  return;
}
