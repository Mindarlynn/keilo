#pragma once

#include "keilo_table.hpp"
#include "keilo_core.hpp"

#include <string>
#include <vector>
#include <mutex>

class keilo_database
{
public:
	keilo_database(std::string file_path);
	keilo_database(const keilo_database& _other);

public:
	keilo_table* select_table(std::string _name);
	void add_table(keilo_table& _table);
	void drop_table(std::string _name);
	
public:
	inline std::list<keilo_table> get_tables() 
	{
		std::lock_guard<std::mutex> mutex_guard(m_mutex);
		return m_tables;
	}
	inline std::string get_name() const
	{
		return m_name;
	}

private:
	void parse_file();

private:
	inline void set_name(std::string _name) 
	{
		m_name = _name;
	}

private:
	std::string m_name;
	std::string m_file_path;

	std::mutex m_mutex;
	std::list<keilo_table> m_tables;
};

