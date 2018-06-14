#include "keilo_application.hpp"
#include "keilo_database.hpp"
#include "json.hpp"

#include <filesystem>
#include <sstream>
#include <fstream>
#include <mutex>
#include <exception>

using json = nlohmann::json;

std::string keilo_application::create_database(const std::string& name) {
	std::lock_guard<std::mutex> mutex_guard(mutex);

	for (const auto& database : databases)
		if (database.get_name() == name)
			return "Database that was named " + name + " already exist in application";
	databases.emplace_back(name);

	return "Successfully create database that was named " + name;
}

keilo_database* keilo_application::select_database(const std::string& name) {
	std::lock_guard<std::mutex> mutex_guard(mutex);

	for (auto& database : databases)
		if (database.get_name() == name) return &database;

	return nullptr;
}

std::string keilo_application::import_file(const std::string& file_name) {
	if (file_name.find(".json") == std::string::npos)
		return "This program support only *.json files.";

	std::stringstream file_path;

	file_path << std::experimental::filesystem::current_path().generic_string()
		<< "/database/" << file_name;

	std::ifstream file(file_path.str());

	if (!file)
		return "File named " + file_name + " does not exist";

	std::lock_guard<std::mutex> mutex_lock(mutex);
	databases.emplace_back(&file);

	return "Successfully impotred file that was named " + file_name;
}

std::string keilo_application::export_database(const std::string& database_name,
                                               const std::string& file_name) {
	auto database = select_database(database_name);
	try {
		json js;
		js[database_name] = json::array();
		for (auto& table : database->get_tables()) {
			json tb;
			tb["name"] = table.get_name();
			auto rc_arr = json::array();
			for (auto& record : table.get_records()) {
				json rc;
				for (auto& field : record) {
					if (field.second[0] >= '0' && field.second[0] <= '9')
						rc[field.first] = stoi(field.second);
					else
						rc[field.first] = field.second;
				}
				rc_arr += rc;
			}
			tb["value"] = rc_arr;
			js[database_name] += tb;
		}

		std::stringstream file_path;
		file_path << std::experimental::filesystem::current_path().generic_string()
			<< "/database/" << file_name;
		std::ofstream file(file_path.str());

		file << js;
		file.flush();
		file.close();
	}
	catch (std::exception& e) { return e.what(); }
	return "Successfully exported database that was named " + database_name;
}

std::list<keilo_database> keilo_application::get_databases() {
	std::lock_guard<std::mutex> mutex_guard(mutex);
	return databases;
}
