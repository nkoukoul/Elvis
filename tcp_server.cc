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
#include "tcp_server.h"
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
    
  listen(server_sock_, 50);
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
    handle_request(std::move(client_socket), ac);
    //std::thread t(&tcp_server::handle_request, this, std::move(client_socket));
  }
}

int tcp_server::handle_request(int && client_socket, app * ac){
  int in = client_socket;  
  int out = client_socket;
  std::string input_data, output_data;
  char inbuffer[MAXBUF], *p = inbuffer;

  // Read data from client
  int bytes_read = read(in, inbuffer, MAXBUF);
  if ( bytes_read <= 0 ){
    close(client_socket);
    return -1; //client closed connection
  }

  for (int i = 0; i < bytes_read; i++) input_data += inbuffer[i];
  input_data += '\n'; //add end of line for getline
  //std::cout << "received data: \n" << input_data << "\n" << "on fd= "
  //	    << in << "\n";

  std::unordered_map<std::string, std::string>  input_request = ac->req_->do_parse(std::move(input_data));
  
  if (!ac->rm_->get_route(input_request["url"], input_request["request_type"])){
    input_request.insert(std::make_pair("status", "400"));
  }
  
  if (input_request["status"] != "400"){//temporary
    std::string bla = "bla";
    //nkou_response_creator * nkou = new nkou_response_creator();
    //nkour->create_response
    output_data = ac->res_->do_create_response(std::move(bla));
    if (input_request["request_type"] == "POST"){
      //model is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
      std::unique_ptr<file_model> fm = std::make_unique<file_model>();
      //std::cout << deserialize_data << "\n";      
      fm->model_map(std::move(ac->juc_->do_deserialize(std::move(input_request["data"]))));
      fm->repr();
    }
  }else{
    output_data = "bad request";
  }

  write(out, output_data.c_str(), output_data.size());
  //std::cout << "closing socket " << client_socket << "\n";
  close(client_socket);
  return 0;
}
