#pragma once

#include "keilo_core.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

class keilo_table
{
public:
	keilo_table(std::string _name, std::list<keilo_record> _records);
	keilo_table(const keilo_table& _other);

public:
	keilo_record select_record(keilo_instance _instance);
	keilo_table join(keilo_table* _other);
	void insert(keilo_record& _record);
	void update(keilo_instance _destination, keilo_instance _source);
	void remove(keilo_instance _instance);

public:
	inline std::list<keilo_record> get_records() 
	{
		std::lock_guard<std::mutex> mutex_guard(m_mutex);
		return m_records;
	}
	inline int count() 
	{
		std::lock_guard<std::mutex> mutex_guard(m_mutex);
		return m_records.size();
	}
	inline std::string get_name() const
	{
		return m_name;
	}

private:
	inline void set_name(std::string _name) {
		m_name = _name;
	}

private:
	std::string m_name;

	std::mutex m_mutex;
	std::list<keilo_record> m_records;
};
