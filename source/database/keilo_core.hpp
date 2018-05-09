#pragma once

#include <string>
#include <list>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using keilo_instance = std::pair<std::string, std::string>;
using keilo_record = std::list<keilo_instance>;