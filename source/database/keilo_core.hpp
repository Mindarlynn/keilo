#pragma once

#include <string>
#include <list>
#include <winsock2.h>
#include <tcp_socket.hpp>

#pragma comment(lib, "ws2_32.lib")

using keilo_field = std::pair<std::string, std::string>;
using keilo_record = std::list<keilo_field>;

inline bool operator==(tcp_socket a, tcp_socket b)
{
	return a.get_ip() == b.get_ip() &&
		a.get_port() == b.get_port();
}