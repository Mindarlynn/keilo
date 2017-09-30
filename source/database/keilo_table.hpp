#pragma once

#include "keilo_core.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

class keilo_table
{
public:
	keilo_table(std::string _name);
	keilo_table(std::string _name, std::list<keilo_record> _records);
	keilo_table(const keilo_table& _other);

public:
	keilo_table join(keilo_table* _other);

	keilo_record select_record(keilo_instance _instance);
	std::string insert_record(keilo_record& _record);
	std::string update_record(keilo_instance _destination, keilo_instance _source);
	std::string remove_record(keilo_instance _instance);

public:
	std::list<keilo_record> get_records();
	int count();
	std::string get_name() const;

private:
	void set_name(std::string _name);

private:
	std::string m_name;

	std::mutex m_mutex;
	std::list<keilo_record> m_records;
};
