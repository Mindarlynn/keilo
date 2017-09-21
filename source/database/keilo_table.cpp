#include "keilo_table.hpp"

#include <mutex>

keilo_table::keilo_table(std::string _name, std::list<keilo_record> _rows) : m_records(_rows), m_name(_name)
{
}

keilo_table::keilo_table(const keilo_table & _other) : m_records(_other.m_records), m_name(_other.m_name)
{
}


keilo_record keilo_table::select_record(keilo_instance _instance)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);

	for (auto record : m_records) 
	{
		for (auto instance : record) 
		{
			if (instance != _instance)
				continue;

			return record;
		}
	}
	throw std::exception(("Could not find identifier \"" + _instance.first +"\" or value\"" + _instance.second + "\".").c_str());
}

keilo_table keilo_table::join(keilo_table* _other)
{
	std::list<keilo_record> joined_table{ get_records() };
	auto other_table = _other->get_records();

	for (auto& i_record : joined_table) 
	{
		keilo_instance i_instance;

		for (const auto& instance : i_record) 
		{
			if (instance.first != "index") 
				continue;

			i_instance = instance;
			break;
		}

		bool found = false;

		for (auto& j_record : other_table) 
		{
			for (const auto& j_instance : j_record) 
			{
				if (j_instance.first == "index") 
				{
					if (i_instance.second != j_instance.second) 
						break;

					found = true;
				}
				else 
					i_record.push_back(keilo_instance{ j_instance.first, j_instance.second });
			}
			if (found) 
				break;
		}
	}
	return keilo_table(get_name() + "+" + _other->get_name(), joined_table);
}

void keilo_table::insert(keilo_record & _record)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	int pos;

	for (auto& i_instance : _record) 
	{
		if (i_instance.first != "index") 
			continue;

		for (const auto& j_record : m_records) 
		{
			for (const auto& j_instance : j_record) 
			{
				if (i_instance != j_instance)
					continue;

				throw std::exception(("Element that has index \"" + i_instance.second + "\" is already exist in table \"" + get_name() + "\".").c_str());
			}
		}
		pos = atoi(i_instance.second.c_str());
		break;
	}

	auto count = 0;
	std::list<keilo_record>::iterator it = m_records.begin();

	while (it != m_records.end() && ++count != pos) 
		it++;

	m_records.insert(it, _record);
}

void keilo_table::update(keilo_instance _where, keilo_instance _new)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);

	keilo_record* found_record;
	auto found = false;

	for (auto& record : m_records) 
	{
		for (const auto& instance : record) 
		{
			if (instance != _where) 
				continue;

			found_record = &record;
			found = true;
			break;
		}
		if (found)
			break;
	}
	if (!found) 
		throw std::exception((_where.first + " \"" + _where.second + "\" does not exist in table \"" + get_name() + "\".").c_str());

 	auto changed = false;

	for (auto& instance : *found_record) 
	{
		if (instance.first != _new.first) 
			continue;

		instance.second = _new.second;
		changed = true;
		break;
	}
	if (!changed) 
		throw std::exception(("Identifier \"" + _new.first + "\" does not exist in table \"" + get_name() + "\".").c_str());
}
void keilo_table::remove(keilo_instance _instance)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	
	std::list<keilo_record>::iterator it;
	auto found = false;

	for (auto record = m_records.begin(); record != m_records.end(); ++record) 
	{
		for (auto instance = record->begin(); instance != record->end(); ++instance) 
		{
			if (*instance != _instance)
				continue;

			it = record;
			found = true;
			break;
		}
		if (found) 
			break;
	}
	if(!found)
		throw std::exception((_instance.first + " \"" + _instance.second + "\" does not exist in table \"" + get_name() + "\".").c_str());

	m_records.erase(it);
}
