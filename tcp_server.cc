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

tcp_server::tcp_server(std::string ipaddr, int port, 
		       std::shared_ptr<i_json_util_context> juc, 
		       std::shared_ptr<route_manager> rm,
		       std::shared_ptr<i_request_context> rc): 
  ipaddr_(ipaddr), port_(port), juc_(juc), rm_(rm), rc_(rc)
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

void tcp_server::set_json_util_context(std::shared_ptr<i_json_util_context> juc){
  this->juc_ = juc;
}

void tcp_server::set_route_manager(std::shared_ptr<route_manager> rm){
  this->rm_ = rm;
}

void tcp_server::set_request_context(std::shared_ptr<i_request_context> rc){
  this->rc_ = rc;
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

  std::unordered_map<std::string, std::string>  input_request = rc_->do_parse(std::move(input_data));
  
  if (!rm_->get_route(input_request["url"], input_request["request_type"])){
    input_request.insert(std::make_pair("status", "400"));
  }
  
  if (input_request["status"] != "400"){//temporary
    output_data = "succesfull request";
    if (input_request["request_type"] == "POST"){
      //model is {"filename": "test.txt",  "md5": "5f7f11f4b89befa92c9451ffa5c81184"}
      file_model * fm = new file_model();
      //std::cout << deserialize_data << "\n";
      juc_->do_deserialize(std::move(input_request["data"]), fm);
      fm->repr();
      delete fm;
    }
  }else{
    output_data = "bad request";
  }
  write(out, output_data.c_str(), output_data.size());
  close(client_socket);
  return 0;
}
