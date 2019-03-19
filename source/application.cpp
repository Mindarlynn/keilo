#include "keilo.hpp"

using json = nlohmann::json;

namespace keilo {

	result_t application::create_database(const std::string& database_name) {
		std::unique_lock<std::mutex> lock{this->mutex};

		if(hashed_databases[database_name])
			return result_t::already_exist;

		hashed_databases[database_name] = true;
		databases.emplace_back(database_name);

		return result_t::success;
	}

	result_t application::add_database(const database& database) {
		std::unique_lock<std::mutex> lock(mutex);

		if(hashed_databases[database.get_name()])
			return result_t::already_exist;

		hashed_databases[database.get_name()] = true;
		databases.emplace_back(database);

		return result_t::success;
	}

	result_t application::drop_database(const std::string& database_name) {
		std::unique_lock<std::mutex> lock{this->mutex};

		if (!hashed_databases[database_name])
			return result_t::cannot_find;

		const auto it = std::find_if(databases.begin(), databases.end(), [database_name](const database& database) {
			return database.get_name() == database_name;
		});

		hashed_databases[it->get_name()] = false;
		databases.erase(it);

		return result_t::success;
	}

	database* application::select_database(const std::string& database_name) {
		std::unique_lock<std::mutex> lock{this->mutex};

		if (!hashed_databases[database_name])
			return nullptr;

		const auto it = std::find_if(databases.begin(), databases.end(), [database_name](const database& database) {
			return database.get_name() == database_name;
		});

		return &*it;
	}

	result_t application::import_file(const std::string& file_name) {
		if (file_name.find(".json") == std::string::npos)
			return result_t::cannot_find;

		std::stringstream file_path;

		file_path << std::experimental::filesystem::current_path().generic_string() << "/database/" << file_name;

		std::ifstream file(file_path.str());

		if (!file)
			return result_t::cannot_find;

		std::unique_lock<std::mutex> mutex_lock{this->mutex};
		databases.emplace_back(file);

		return result_t::success;
	}

	result_t application::export_database(const std::string& database_name, const std::string& file_name) {
		auto database = select_database(database_name);
		try {
			json js;
			js[database->get_name()] = json::array();
			for (auto& table : database->get_tables()) {
				json tb;
				tb["name"] = table.get_name();
				tb["key"] = table.get_key();
				auto rc_arr = json::array();
				for (auto& record : table.get_records()) {
					json rc;
					for (auto& instance : record()) {
						if (instance.identifier[0] >= '0' && instance.identifier[0] <= '9')
							rc[instance.identifier] = std::stoi(instance.value);
						else
							rc[instance.identifier] = instance.value;
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
		catch (std::exception& e) { return result_t::fail; }
		return result_t::success;
	}

	std::list<database> application::get_databases() {
		std::unique_lock<std::mutex> lock{this->mutex};
		return this->databases;
	}
}
