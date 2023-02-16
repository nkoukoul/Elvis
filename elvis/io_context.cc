//
// Copyright (c) 2020-2021 Nikolaos Koukoulas (koukoulas dot nikos at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE.md)
//
// repository: https://github.com/nkoukoul/Elvis
//

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

void non_block_socket(int sd)
{
	/* set O_NONBLOCK on fd */
	int flags = fcntl(sd, F_GETFL, 0);
	if (flags == -1)
	{
		//perror("fcntl()");
		return;
	}
	if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		//perror("fcntl()");
	}
}

void non_block_with_timeout(int sd)
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100;

	if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
		std::cout << "sock timeout failed\n";
}

Elvis::TCPContext::TCPContext(
	std::string ipaddr,
	int port,
	app* ac) : ipaddr_(ipaddr), port_(port), ac_(ac)
{
	struct sockaddr_in server;

	server_sock_ = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
	non_block_with_timeout(server_sock_);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ipaddr_.c_str());
	server.sin_port = htons(port_);

	if (bind(server_sock_, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		std::cout << "bind failed\n";
		exit(1);
	}

	listen(server_sock_, 5);
}

void Elvis::TCPContext::Run()
{
	std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::HandleConnections, ac_->ioc_.get());
	ac_->m_AsyncQueue->CreateTask(std::move(event));
	while (true)
	{
		auto event = ac_->m_AsyncQueue->RunTask();
		event.wait();
	}
}

void Elvis::TCPContext::HandleConnections()
{
	struct sockaddr_in client;
	socklen_t client_len;
	client_len = sizeof client;
	int client_socket = accept(server_sock_, (struct sockaddr*)&client, &client_len);
	if (client_socket <= 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			;
			; //no incoming connection for non-blocking sockets
		}
		else
		{
			std::cout << "Error while accepting connection\n";
		}
	}
	else
	{
		non_block_with_timeout(client_socket);
		std::shared_ptr<ClientContext> c_ctx = std::make_shared<ClientContext>();
		c_ctx->m_ClientSocket = client_socket;
		std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::DoRead, ac_->ioc_.get(), c_ctx);
		ac_->m_AsyncQueue->CreateTask(std::move(event));
	}
	std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::HandleConnections, ac_->ioc_.get());
	ac_->m_AsyncQueue->CreateTask(std::move(event));
}

void Elvis::TCPContext::DoRead(std::shared_ptr<ClientContext> c_ctx)
{
	//auto executor = ac_->m_AsyncQueue->access_strand<event_queue<std::function<void()>>>();
	char inbuffer[MAXBUF], * p = inbuffer;
	// Read data from client
	int bytes_read = read(c_ctx->m_ClientSocket, inbuffer, MAXBUF);
	// Client closed connection
	if (bytes_read == 0)
	{
		close(c_ctx->m_ClientSocket);
		return;
	}
	if (bytes_read < 0)
	{
		// Client read block
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			//if is websocket and has data proceed to parsing
			if (c_ctx->m_IsWebsocketConnection && c_ctx->m_WSMessage.size())
			{
				std::future<void> event = std::async(std::launch::deferred, &IRequestContext::DoParse, ac_->m_WSRequestContext.get(), c_ctx);
				ac_->m_AsyncQueue->CreateTask(std::move(event));
			} //if is http and has data proceed to parsing
			else if (!c_ctx->m_IsWebsocketConnection && c_ctx->m_HttpMessage.size())
			{
				c_ctx->m_HttpMessage += '\n'; //add end of line for getline
				std::future<void> event = std::async(std::launch::deferred, &IRequestContext::DoParse, ac_->m_HttpRequestContext.get(), c_ctx);
				ac_->m_AsyncQueue->CreateTask(std::move(event));
			} // read simply blocked with no data try again
			else
			{
				std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::DoRead, ac_->ioc_.get(), c_ctx);
				ac_->m_AsyncQueue->CreateTask(std::move(event));
			}
			return;
		}
		else
		{
			// TCP read error
			close(c_ctx->m_ClientSocket);
			return;
		}
	}
	else // We have data to read
	{
		if (c_ctx->m_IsWebsocketConnection)
		{
			for (int i = 0; i < bytes_read; i++)
			{
				c_ctx->m_WSMessage += inbuffer[i];
			}
		}
		else
		{
			for (int i = 0; i < bytes_read; i++)
			{
				c_ctx->m_HttpMessage += inbuffer[i];
			}
		}
		// maybe there some more data so add another read to the queue
		std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::DoRead, ac_->ioc_.get(), c_ctx);
		ac_->m_AsyncQueue->CreateTask(std::move(event));
	}
}

void Elvis::TCPContext::DoWrite(std::shared_ptr<ClientContext> c_ctx)
{
	int bytes_write;
	if (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted)
	{
		size_t bytes_to_send;
		if (MAXBUF > (c_ctx->m_WSResponse.size() - c_ctx->m_WSBytesSend))
		{
			bytes_to_send = c_ctx->m_WSResponse.size() - c_ctx->m_WSBytesSend;
		}
		else
		{
			bytes_to_send = MAXBUF;
		}
		bytes_write = write(c_ctx->m_ClientSocket, c_ctx->m_WSResponse.c_str() + c_ctx->m_WSBytesSend, bytes_to_send);
	}
	else
	{
		size_t bytes_to_send;
		if (MAXBUF > (c_ctx->m_HttpResponse.size() - c_ctx->m_HttpBytesSend))
		{
			bytes_to_send = c_ctx->m_HttpResponse.size() - c_ctx->m_HttpBytesSend;
		}
		else
		{
			bytes_to_send = MAXBUF;
		}

		bytes_write = write(c_ctx->m_ClientSocket, c_ctx->m_HttpResponse.c_str() + c_ctx->m_HttpBytesSend, bytes_to_send);
	}

	// Client closed connection
	if (bytes_write == 0)
	{
		close(c_ctx->m_ClientSocket);
		return;
	}

	// Error handling
	if (bytes_write < 0)
	{ // Write block try again
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::DoWrite, ac_->ioc_.get(), c_ctx);
			ac_->m_AsyncQueue->CreateTask(std::move(event));
		}
		else
		{
			// TCP write error
			close(c_ctx->m_ClientSocket);
		}
		return;
	}

	// Check if we need to transfer more data
	if (c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted)
	{
		c_ctx->m_WSBytesSend += bytes_write;
	}
	else
	{
		c_ctx->m_HttpBytesSend += bytes_write;
	}

	if ((!c_ctx->m_IsWebsocketConnection && c_ctx->m_HttpBytesSend < c_ctx->m_HttpResponse.size()) ||
		(c_ctx->m_IsWebsocketConnection && c_ctx->m_IsHandshakeCompleted && c_ctx->m_WSBytesSend < c_ctx->m_WSResponse.size()))
	{
		std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::DoWrite, ac_->ioc_.get(), c_ctx);
		ac_->m_AsyncQueue->CreateTask(std::move(event));
		return;
	}
	// Check if socket should close
	if (c_ctx->m_ShouldCloseConnection)
	{
		close(c_ctx->m_ClientSocket);
		return;
	}

	// if socket is still open with http keep-alive or ws read again
	if (c_ctx->m_IsWebsocketConnection)
	{
		if (!c_ctx->m_IsHandshakeCompleted)
		{
			ac_->broadcast_fd_list.push_back(c_ctx->m_ClientSocket);
			c_ctx->m_IsHandshakeCompleted = true;
		}
		c_ctx->m_WSMessage.clear();
		c_ctx->m_WSData.clear();
		c_ctx->m_WSResponse.clear();
		c_ctx->m_WSBytesSend = 0;
	}
	else
	{
		c_ctx->m_HttpMessage.clear();
		c_ctx->m_HttpBytesSend = 0;
	}
	std::future<void> event = std::async(std::launch::deferred, &Elvis::IOContext::DoRead, ac_->ioc_.get(), c_ctx);
	ac_->m_AsyncQueue->CreateTask(std::move(event));
	return;
}
