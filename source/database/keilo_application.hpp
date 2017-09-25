#pragma once

#include "keilo_core.hpp"
#include "keilo_database.hpp"

#include <string>
#include <memory.h>
#include <mutex>

class keilo_application
{
public:
	std::string create_database(std::string _name);
	keilo_database* select_database(std::string _name);
	
public:
	std::string import_file(std::string file_name);
	std::string export_database(std::string database_name, std::string file_name);

public:
	std::list<keilo_database> get_databases();
	
private:
	std::mutex m_mutex;
	std::list<keilo_database> m_databases;
}; 