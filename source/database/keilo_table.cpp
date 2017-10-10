#include "keilo_table.hpp"

#include <mutex>

keilo_table::keilo_table(std::string _name) : m_name(_name), m_records(std::list<keilo_record>())
{
}

keilo_table::keilo_table(std::string _name, std::list<keilo_record> _rows) : m_name(_name), m_records(_rows)
{
}

keilo_table::keilo_table(const keilo_table & _other) : m_name(_other.m_name), m_records(_other.m_records)
{
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
	return keilo_record();
	//throw std::exception(("Could not find record that has identifier \"" + _instance.first +"\" or value\"" + _instance.second + "\".").c_str());
}

std::string keilo_table::insert_record(keilo_record & _record)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	int pos = 0;
	std::string index;

	for (auto& i_instance : _record) 
	{
		if (i_instance.first != "index") 
			continue;

		for (const auto& j_record : m_records) 
			for (const auto& j_instance : j_record) 
				if (i_instance != j_instance)
					continue;
				else
				return "Record that has index \"" + i_instance.second + "\" is already exist in table \"" + get_name() + "\".";
		
		index = i_instance.second;
		pos = atoi(index.c_str());
		break;
	}

	auto count = 0;
	std::list<keilo_record>::iterator it = m_records.begin();

	while (it != m_records.end() && ++count != pos) 
		it++;

	m_records.insert(it, _record);
	return "Successfully inserted record that has index\"" + index + "\" to table \"" + get_name() + "\".";
}

std::string keilo_table::update_record(keilo_instance _where, keilo_instance _new)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);

	keilo_record* found_record = nullptr;
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
		return "Record that has " + _where.first + " \"" + _where.second + "\" does not exist in table \"" + get_name() + "\".";
		//throw std::exception((_where.first + " \"" + _where.second + "\" does not exist in table \"" + get_name() + "\".").c_str());

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
		return "Identifier \"" + _new.first + "\" does not exist in table \"" + get_name() + "\".";
		//throw std::exception(("Identifier \"" + _new.first + "\" does not exist in table \"" + get_name() + "\".").c_str());
	return  "Successfully updated record that has " + _where.first + " \"" + _where.second +"\" in table \"" + get_name() + "\".";
}

std::string keilo_table::remove_record(keilo_instance _instance)
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
	if (!found)
		return "Record that has" + _instance.first + " \"" + _instance.second + "\" does not exist in table \"" + get_name() + "\".";
		//throw std::exception((_instance.first + " \"" + _instance.second + "\" does not exist in table \"" + get_name() + "\".").c_str());

	m_records.erase(it);
	return "Successfully removed record that has " + _instance.first + " \"" + _instance.second + "\" in table \"" + get_name() + "\"";
}

std::list<keilo_record> keilo_table::get_records()
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	return m_records;
}

int keilo_table::count()
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	return m_records.size();
}

std::string keilo_table::get_name() const
{
	return m_name;
}

void keilo_table::set_name(std::string _name)
{
	m_name = _name;
}
