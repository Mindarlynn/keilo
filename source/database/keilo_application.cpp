#include "keilo_application.hpp"
#include "keilo_database.hpp"

#include <experimental/filesystem>
#include <sstream>
#include <fstream>
#include <mutex>
#include <exception>



std::string keilo_application::create_database(std::string _name)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);

	for (const auto& database : m_databases) {
		if (database.get_name() == _name)
			return "Database that was named \"" + _name + "\" already exist in application.";
		//throw std::exception(("Database \"" + _name + "\" already exist in application.").c_str());
	}

	m_databases.push_back(keilo_database(_name));
	return "Successfully create database that was named \"" + _name + "\".";
}

keilo_database* keilo_application::select_database(std::string _name)
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);

	for (auto& database : m_databases) 
	{
		if (database.get_name() != _name) 
			continue;

		return &database;
	}

	return nullptr;
}

std::string keilo_application::import_file(std::string file_name)
{
	if (file_name.find(".klo") == std::string::npos)
		return "This program support only *.klo files.";
	//throw std::exception("This program support only *.klo files.");

	std::stringstream file_path;

	file_path << std::experimental::filesystem::current_path().generic_string() << "/database/" << file_name;

	if (std::ifstream file(file_path.str()); file) {
		m_databases.clear();
		m_mutex.lock();
		m_databases.push_back(keilo_database(file));
		m_mutex.unlock();

		return "Sucessfully impotred file that was named \"" + file_name + "\".";
	}
	else
		return "File that was named \"" + file_name + "\" does not exist.";
}

std::string keilo_application::export_database(std::string database_name, std::string file_name)
{
	auto database = select_database(database_name);
	try {
		std::stringstream file_path;

		file_path << std::experimental::filesystem::current_path().generic_string() << "/database/" << file_name;

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
	catch (std::exception& e) {
		return e.what();
	}
	return "Successfully exported database that was named \"" + database_name + "\".";
}

std::list<keilo_database> keilo_application::get_databases()
{
	std::lock_guard<std::mutex> mutex_guard(m_mutex);
	return m_databases;
}
