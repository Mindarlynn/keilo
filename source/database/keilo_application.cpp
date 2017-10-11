#include "keilo_application.hpp"
#include "keilo_database.hpp"
#include "json.hpp"

#include <experimental/filesystem>
#include <sstream>
#include <fstream>
#include <mutex>
#include <exception>

using json = nlohmann::json;

std::string keilo_application::create_database(const std::string name)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	for (const auto& database : databases_)
	{
		if (database.get_name() == name)
			return "Database that was named \"" + name + "\" already exist in application.";
	}

	databases_.push_back(keilo_database(name));
	return "Successfully create database that was named \"" + name + "\".";
}

keilo_database* keilo_application::select_database(const std::string name)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	for (auto& database : databases_)
	{
		if (database.get_name() != name)
			continue;

		return &database;
	}

	return nullptr;
}

std::string keilo_application::import_file(std::string file_name)
{
	if (file_name.find(".json") == std::string::npos)
		return "This program support only *.json files.";

	std::stringstream file_path;

	file_path << std::experimental::filesystem::current_path().generic_string() << "/database/" << file_name;

	if (std::ifstream file(file_path.str()); file)
	{
		std::lock_guard<std::mutex> mutex_lock(mutex_);
		databases_.push_back(keilo_database(file));

		return "Successfully impotred file that was named \"" + file_name + "\".";
	}
	return "File that was named \"" + file_name + "\" does not exist.";
}

std::string keilo_application::export_database(const std::string database_name, const std::string file_name)
{
	auto database = select_database(database_name);
	try
	{
		json js;
		js[database->get_name()] = json::array();
		for (auto& table : database->get_tables())
		{
			json tb;
			tb["name"] = table.get_name();
			json rc_arr = json::array();
			for (auto& record : table.get_records())
			{
				json rc;
				for (auto& instance : record)
				{
					if (instance.second[0] >= '0' && instance.second[0] <= '9')
						rc[instance.first] = atoi(instance.second.c_str());
					else
					{
						int pos = 0;
						std::string from = "\"";

						while ((pos = instance.second.find(from, pos)) != std::string::npos)
						{
							instance.second.replace(pos, from.length(), "");
							pos += from.length();
						}

						rc[instance.first] = instance.second;
					}
				}
				rc_arr += rc;
			}
			tb["value"] = rc_arr;
			js[database->get_name()] += tb;
		}

		std::stringstream file_path;
		file_path << std::experimental::filesystem::current_path().generic_string() << "/database/" << file_name;
		std::ofstream file(file_path.str());

		file << js;
		file.flush();
		file.close();
	}
	catch (std::exception& e)
	{
		return e.what();
	}
	return "Successfully exported database that was named \"" + database_name + "\".";
}

std::list<keilo_database> keilo_application::get_databases()
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	return databases_;
}
