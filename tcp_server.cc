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
#include "models.h"

int socket_timeout = 3000;

tcp_server::tcp_server(std::string ipaddr, int port, i_json_util_context * juc, route_manager * rm): ipaddr_(ipaddr), port_(port), juc_(juc), rm_(rm)
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

tcp_server::~tcp_server(){
  delete juc_;
  delete rm_;
}

void tcp_server::set_json_util_context(i_json_util_context * juc){
  delete juc_;
  this->juc_ = juc;
}

void tcp_server::set_route_manager(route_manager * rm){
  delete rm_;
  this->rm_ = rm;
}
  
void tcp_server::accept_connections(){
  struct sockaddr_in client; 
  socklen_t client_len;
  
  client_len = sizeof client;
  
  //return accept4(server_sock_, (struct sockaddr *)&client, &client_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  std::cout << "server accepting connections on " << ipaddr_ << ":" << port_ << "\n";
  while(true){
    int client_socket = accept(server_sock_, (struct sockaddr *)&client, &client_len);
    if (client_socket < 0){
      std::cout << "Error while accepting connection";
      continue;
    }
    std::thread t(&tcp_server::handle_request, this, std::move(client_socket));
    client_threads_.push_back(std::move(t));
  }

  quit();
}

void tcp_server::quit(){
  for (std::thread & t : client_threads_){
    if (t.joinable())
      t.join();
  }
  return;
}

int tcp_server::handle_request(int && client_socket){
  int in = client_socket;  
  int out = client_socket;
  std::string input_data, output_data;
  char inbuffer[MAXBUF], *p = inbuffer;

  // Read data from client
  int bytes_read = read(in, inbuffer, MAXBUF);
  if ( bytes_read <= 0 )
    return -1; //client closed connection

  for (int i = 0; i < bytes_read; i++) input_data += inbuffer[i];
  input_data += '\n'; //add end of line for getline
  std::cout << "received data: \n" << input_data << "\n" << "on fd= "
  	    << in << "\n";

  std::istringstream ss(input_data);
  std::string request_type, url, protocol, line;
  
  std::getline(ss, line);
  std::istringstream first_line(line);
  first_line >> request_type >> url >> protocol;
  while (line.size() > 1){
    //headers here
    //std::cout << "line " << line << "\n";
    std::getline(ss, line);
  }

  //  std::cout << "request: " << request_type << "\n"
  //	    << "url: " << url << "\n"
  //	    << "protocol: " << protocol << "\n";
  
  if (request_type == "GET"){
    output_data = "get hi there";
  }else if (request_type == "POST"){
    output_data = "post hi there";
    std::string deserialized_data;
    //json for post
    std::getline(ss, line);
    while (line.size() > 1){
      deserialized_data += line;
      std::getline(ss, line);
    }
    //model is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
    file_model * fm = new file_model();
    //std::cout << deserialize_data << "\n";
    juc_->do_deserialize(std::move(deserialized_data), fm);
    fm->repr();
    delete fm;
  }

  write(out, output_data.c_str(), output_data.size());
  close(client_socket);
  return 0;
}
