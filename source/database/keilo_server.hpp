#pragma once

#include "keilo_database.hpp"

#include <string>
#include <mutex>

class keilo_server
{
public:
	void run();

public:
	void import_file(std::string file_name);
	keilo_database* select_database(std::string _name);
	void export_database(std::string database_name, std::string file_name);

public:
	inline std::list<keilo_database> get_databases() 
	{
		std::lock_guard<std::mutex> mutex_guard(m_mutex);
		return m_databases;
	}
	
private:
	std::mutex m_mutex;
	std::list<keilo_database> m_databases;
}; 