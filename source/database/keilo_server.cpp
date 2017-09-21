#include "keilo_server.hpp"
#include "keilo_database.hpp"

#include <boost/filesystem.hpp>

#include <sstream>
#include <fstream>
#include <exception>

void keilo_server::import_file(std::string file_name)
{	
	if (file_name.find(".klo") == std::string::npos)
		throw std::exception("This program support only *.klo files.");
	
	std::stringstream file_path;

	file_path << boost::filesystem::current_path().generic_string() << "/database/" << file_name;

	std::ifstream file(file_path.str());

	if (!file) 
		throw std::exception(("File \"" + file_name + "\" does not exist.").c_str());
	file.close();

	m_mutex.lock();
	m_databases.push_back(keilo_database(file_path.str()));
	m_mutex.unlock();
}

keilo_database* keilo_server::
select_database(std::string _name)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);

	for (auto& database : m_databases) 
	{
		if (database.get_name() != _name) 
			continue;

		return &database;
	}

	throw std::exception("Could not find database.");
}

void keilo_server::export_database(std::string database_name, std::string file_name)
{
	auto database = select_database(database_name);
	std::stringstream file_path;

	file_path << boost::filesystem::current_path().generic_string() << "/database/" << file_name;

	std::ofstream file(file_path.str());
	
	file << database->get_name() << std::endl;
	file << "[" << std::endl;
	for (auto table : database->get_tables()) 
	{
		file << "	" << table.get_name() << std::endl;
		file << "	{" << std::endl;
		for (const auto& record : table.get_records()) 
		{
			file << "		(" << std::endl;
			for (const auto& instance : record) 
				file << "		" << instance.first << ":" << instance.second << ";" << std::endl;
			file << "		)" << std::endl;
		}
		file << "	}" << std::endl;
	}
	file << "]";

	file.flush();
	file.close();
}

void keilo_server::run()
{
}
