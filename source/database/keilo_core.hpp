#pragma once

#include <string>
#include <list>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using keilo_instance = std::pair<std::string, std::string>;
using keilo_record = std::list<keilo_instance>;

static bool operator==(const SOCKADDR_IN a, const SOCKADDR_IN b)
{
	return a.sin_addr.S_un.S_addr == b.sin_addr.S_un.S_addr &&
		a.sin_port == b.sin_port;
}

typedef struct
{
	/**
	 * \brief Client's socket.
	 */
	SOCKET socket;
	/**
	 * \brief Client's address.
	 */
	SOCKADDR_IN address;
} client;
