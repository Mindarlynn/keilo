#pragma once

#include "keilo_table.hpp"
#include "keilo_core.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <mutex>

class keilo_database
{
public:
	keilo_database(std::string _name);
	keilo_database(std::ifstream& _file);
	keilo_database(const keilo_database& _other);

public:
	std::string create_table(std::string _name);
	std::string add_table(keilo_table& _table);
	keilo_table* select_table(std::string _name);
	std::string drop_table(std::string _name);

private:
	void parse_file(std::ifstream& _file);
	
public:
	std::list<keilo_table> get_tables();
	std::string get_name() const;

private:
	void set_name(std::string _name);

private:
	std::string m_name;

	std::mutex m_mutex;
	std::list<keilo_table> m_tables;
};

