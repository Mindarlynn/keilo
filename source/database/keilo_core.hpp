#pragma once

#include <string>
#include <list>

typedef std::pair<std::string, std::string> keilo_instance;
typedef std::list<keilo_instance> keilo_record;

typedef struct {
	SOCKET sock;
	SOCKADDR_IN addr;
} client;