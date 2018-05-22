#pragma once
#include <string>
#include <winsock2.h>

#include "string_process.h"

#pragma warning(disable:4996)

class tcp_socket
{
public:
	explicit tcp_socket(string_process** const processer = nullptr);
	tcp_socket(const char*& ip, const u_short& port, string_process** const processer = nullptr);
	tcp_socket(const u_long& ip, const u_short& port, string_process** const processer = nullptr);
	~tcp_socket();

	void operator=(const tcp_socket& other)
	{
		this->socket_ = other.socket_;
		this->address_ = other.address_;
		this->addr_len_ = other.addr_len_;
		this->is_initialized_ = other.is_initialized_;
		this->is_bound_ = other.is_bound_;
	}

private:
	bool initialize_socket();
	bool bind_socket(const char*& ip, const u_short& port);
	bool bind_socket(const u_long& ip, const u_short& port);
	std::string receive_data();
	bool send_data(const std::string& data);

public:
	bool start() const;
	bool connect_to(const std::string& ip, const u_short& port);
	void accept_client(tcp_socket** const socket) const;

	bool send(const std::string& data);
	std::string recv();


	auto get_port() const
	{
		return ntohs(address_.sin_port);
	}

	std::string get_ip() const
	{
		return inet_ntoa(address_.sin_addr);
	}

	void stop();

private:
	SOCKET socket_;
	SOCKADDR_IN address_{};
	int addr_len_;

	bool is_bound_ = false;
	bool is_initialized_ = false;

	string_process* processer_;
};

