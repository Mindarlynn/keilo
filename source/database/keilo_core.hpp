#pragma once

#include <string>
#include <list>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

typedef std::pair<std::string, std::string> keilo_instance;
typedef std::list<keilo_instance> keilo_record;

typedef struct {
	SOCKET sock;
	SOCKADDR_IN addr;
} client;