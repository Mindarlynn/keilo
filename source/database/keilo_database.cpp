#include "keilo_database.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <unordered_map>

#include <boost/algorithm/string.hpp>


keilo_database::keilo_database(std::string _name) : m_name(_name), m_tables(std::list<keilo_table>())
{
}

keilo_database::keilo_database(std::ifstream& _file) : m_tables(std::list<keilo_table>())
{
	parse_file(_file);
}

keilo_database::keilo_database(const keilo_database & _other) : m_name(_other.m_name), m_tables(_other.m_tables)
{
}

std::string keilo_database::create_table(std::string _name)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);

	for (const auto& table : m_tables) {
		if (table.get_name() == _name)
			return "Table that was named \"" + _name + "\" already exist in database.";
		//throw std::exception(("Table \"" + _name + "\" already exist in database.").c_str());
	}

	m_tables.push_back(keilo_table(_name));
	return "Successfully create table that was named \"" + _name + "\".";
}

std::string keilo_database::add_table(keilo_table & _table)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	for (const auto& table : m_tables) 
	{
		if (table.get_name() != _table.get_name()) 
			continue;

		return "Table \"" + _table.get_name() + "\" already exist in database \"" + get_name() + "\".";
		//throw std::exception(("Table \"" + _table.get_name() + "\" already exist in database \"" + get_name() + "\".").c_str());
	}
	m_tables.push_back(_table);
	return "Successfully added table that was named \"" + _table.get_name() + "\".";
}

keilo_table* keilo_database::select_table(std::string _name)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	for (auto& table : m_tables) {
		if (table.get_name() != _name)
			continue;

		return &table;
	}

	return nullptr;
	//throw std::exception(("Could not find table that was named \"" + _name + "\".").c_str());
}

std::string keilo_database::drop_table(std::string _name)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	auto it = m_tables.end();

	for (auto table = m_tables.begin(); table != m_tables.end(); ++table) 
	{
		if (table->get_name() != _name) 
			continue;

		it = table;
		break;
	}

	if (it == m_tables.end())
		return "Table that was named \"" + _name + "\" dose not exist in database \"" + get_name() + "\".";
		//throw std::exception(("Table \"" + _name + "\" dose not exist in database \"" + get_name() + "\".").c_str());

	m_tables.erase(it);
	return "Successfully droped table that was named \"" + _name + "\".";
}

void keilo_database::parse_file(std::ifstream& _file)
{
	std::string file_content;
	std::stringstream content;

	while (!_file.eof())
	{
		std::string line;

		_file >> line;
		content << line;
	}

	_file.close();
	file_content = content.str();

	int i;
	std::stringstream db_name;

	for (i = 0; file_content[i] != '['; ++i) 
		db_name << file_content[i];
	i += 1;
	set_name(db_name.str());

	while (file_content[i] != ']') 
	{
		std::stringstream table_name;

		for (; file_content[i] != '{'; ++i) 
			table_name << file_content[i];
		i += 2;

		std::list<keilo_record> records;

		while (file_content[i] != '}') 
		{
			keilo_record record;

			while (file_content[i] != ')') 
			{
				std::stringstream identifier;

				for (; file_content[i] != ':'; ++i) 
					identifier << file_content[i];
				i += 1;

				std::stringstream value;

				for (; file_content[i] != ';'; ++i) 
					value << file_content[i];
				i += 1;

				record.push_back(keilo_instance{ identifier.str(), value.str() });
			}
			file_content[i + 1] == '(' ? i += 2 : i += 1;

			int pos;

			for (const auto& instance : record) 
			{
				if (instance.first != "index") 
					continue;

				pos = atoi(instance.second.c_str());
				break;
			}

			auto count = 0;
			std::list<keilo_record>::iterator it = records.begin();

			while (it != records.end() && ++count != pos)
				it++;

			records.insert(it, record);
		}
		i += 1;

		m_mutex.lock();
		m_tables.push_back(keilo_table(table_name.str(), records));
		m_mutex.unlock();
	}
}

std::list<keilo_table> keilo_database::get_tables()
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	return m_tables;
}

std::string keilo_database::get_name() const
{
	return m_name;
}

void keilo_database::set_name(std::string _name)
{
	m_name = _name;
}
