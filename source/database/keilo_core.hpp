#pragma once

#include <string>
#include <list>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

typedef std::pair<std::string, std::string> keilo_instance;
typedef std::list<keilo_instance> keilo_record;

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
