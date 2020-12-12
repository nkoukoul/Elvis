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
#include <bitset>
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

void non_block_socket(int sd){
  /* set O_NONBLOCK on fd */
  int flags = fcntl(sd, F_GETFL, 0);
  if (flags == -1) {
    //perror("fcntl()");
    return;
  }
  if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1) {
    //perror("fcntl()");
  }
}

tcp_server::tcp_server(std::string ipaddr, int port, std::unique_ptr<i_request_context> req, std::unique_ptr<i_response_context> res, app * ac): 
  ipaddr_(ipaddr), port_(port), req_(std::move(req)), res_(std::move(res)), ac_(ac)
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
    
    //do_read(client_socket);
    ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&io_context::do_read, ac_->http_ioc_.get(), client_socket)));
  }
}

void tcp_server::do_read(int const client_socket){
  std::string input_data;
  char inbuffer[MAXBUF], *p = inbuffer;

  // Read data from client
  int bytes_read = read(client_socket, inbuffer, MAXBUF);
  if ( bytes_read <= 0 ){
    close(client_socket);
    return; //client closed connection
  }

  for (int i = 0; i < bytes_read; i++){
    input_data += inbuffer[i];
  }
  input_data += '\n'; //add end of line for getline
  return ac_->e_q_->produce_event<std::function<void()>>(std::move(std::bind(&i_request_context::do_parse, ac_->http_ioc_->req_.get(), client_socket, std::move(input_data))));
  //return req_->do_parse(client_socket, std::move(input_data));
}


void tcp_server::do_write(int const client_socket, std::string output_data, bool close_connection){
  write(client_socket, output_data.c_str(), output_data.size());
  //close socket if http
  if (close_connection){
    close(client_socket);
  }else{
    //websocket connection for now
    ac_->ws_ioc_->register_socket(client_socket);
  }
  
  return;
}

websocket_server::websocket_server(std::string ipaddr, int port, std::unique_ptr<i_request_context> req, std::unique_ptr<i_response_context> res, app * ac): 
  ipaddr_(ipaddr), port_(port), req_(std::move(req)), res_(std::move(res)), ac_(ac){
  broadcast_fd_list.resize(256, 0);
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    //perror("epoll_create1()");
    //exit;
  }
  memset(&event, 0, sizeof(event));
  events = (struct epoll_event *)calloc(MAXEVENTS, sizeof(struct epoll_event));
}

void websocket_server::run(){
  std::signal(SIGPIPE, SIG_IGN);
  handle_connections();
}

void websocket_server::handle_connections(){
  while (true){
    //std::cout << "handling ws connections\n";
    int nevents = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
    //if (nevents == -1) {
    //perror("epoll_wait()");
    //return 1;
    //}
    for (int i = 0; i < nevents; i++) {
      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
        // error case
        fprintf(stderr, "epoll error\n");
        close(events[i].data.fd);
        continue;
      }else if (events[i].events & EPOLLIN){
	do_read(events[i].data.fd);
      }/*else if (events[i].events & EPOLLOUT){
	if (!message_to_deliver.empty()){
	  
	}
	}*/
    }
  }
}

void websocket_server::register_socket(int const client_socket){
  std::lock_guard<std::mutex> guard(socket_state_mutex_);
  if (client_socket > 255) return;
  broadcast_fd_list[client_socket] = client_socket;
  non_block_socket(client_socket);
  event.data.fd = client_socket;
  event.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
    //perror("epoll_ctl()");
    //return 1;
  }
  return;
}

void websocket_server::do_read(int const client_socket){
  char inbuffer[MAXBUF];
  std::string input_websocket_frame_in_bits;
  // Read data from client
  while(true){
    int bytes_read = read(client_socket, inbuffer, MAXBUF);
    if (bytes_read == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
	//std::cout << "finished reading data from client\n";
	break;
      } else {
	//perror("read()");
	return;
      }
    } else if (bytes_read == 0){
      std::cout << "ws client disconnected \n";
      broadcast_fd_list[client_socket] = 0;
      close(client_socket);
    }else {
      //std::cout << "read " << bytes_read << " bytes \n";
      for (int i = 0; i < bytes_read; i++){
	std::bitset<8> bb(inbuffer[i]);
	input_websocket_frame_in_bits += bb.to_string();
      }
    }
  }
  return req_->do_parse(client_socket, std::move(input_websocket_frame_in_bits));
}

void websocket_server::do_write(int const client_socket, std::string output_data, bool close_connection){
  if (write(client_socket, output_data.c_str(), output_data.size()) < 0 || close_connection){
    std::cout << "error during write\n";
    close(client_socket);
    broadcast_fd_list[client_socket] = 0;
  }
  return;
}
