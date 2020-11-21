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


int socket_timeout = 3000;

tcp_server::tcp_server(std::string ipaddr, int port, json_util_context && juc): ipaddr_(ipaddr), port_(port), juc_(std::move(juc))
{
  struct sockaddr_in server; 
    
  server_sock_ = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ipaddr_.c_str());
  server.sin_port = htons(port_);

  if (bind(server_sock_, (struct sockaddr *) &server, sizeof(server))<0)
    {
      std::cout << "bind failed\n";
      exit(1);
    }
    
  listen(server_sock_, 5);
}
  
void tcp_server::accept_connections(){
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

  /*int total_bytes_read = 0;
  int bytes_read;
  while ((bytes_read = read(in, inbuffer, MAXBUF)) > 0){
    std::cout << "bytes read " << bytes_read << "\n";
    total_bytes_read += bytes_read;
    for (int i = 0; i < bytes_read; i++) input_data += inbuffer[i];
    }*/

  for (int i = 0; i < bytes_read; i++) input_data += inbuffer[i];
  std::cout << "received data: \n" << input_data << "\n" << "on fd= "
  	    << in << "\n";

  std::istringstream ss(input_data);
  std::string request_type, url, protocol;
  ss >> request_type >> url >> protocol;
  
  std::cout << "request: " << request_type << "\n"
	    << "url: " << url << "\n"
	    << "protocol: " << protocol << "\n";
  
  if (request_type == "GET"){
    output_data = "get hi there";
  }else if (request_type == "POST"){
    output_data = "post hi there";
    juc_.do_deserialize(input_data);
  }

  write(out, output_data.c_str(), output_data.size());
  close(client_socket);
  return 0;
}
