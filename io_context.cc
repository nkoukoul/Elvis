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
#include "models.h"

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
    do_write(client_socket, ac, std::move(do_read(client_socket, ac)));
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

void tcp_server::do_write(int client_socket, app * ac, std::string && input_data){
  
  int out = client_socket;
  std::string output_data;
  std::unordered_map<std::string, std::string>  deserialized_input_data = ac->req_->do_parse(std::move(input_data));

  i_controller * ic = ac->rm_->get_controller(deserialized_input_data["url"],deserialized_input_data["request_type"]);
  if (ic){
    std::string controller_data = ic->run(std::move(deserialized_input_data));
    output_data = ac->res_->do_create_response(std::move(controller_data));
  }else{
    output_data = "Bad request";
  }
  
  write(out, output_data.c_str(), output_data.size());
  //std::cout << "closing socket " << client_socket << "\n";
  close(client_socket);
  return;
}
