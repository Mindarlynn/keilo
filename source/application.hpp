#pragma once
#include "core.hpp"

namespace keilo {
	class application {
	public:
		result_t create_database(const std::string& database_name);
		result_t add_database(const database& database);
		result_t drop_database(const std::string& database_name);
		database* select_database(const std::string& database_name);
		result_t import_file(const std::string& file_name);
		result_t export_database(const std::string& database_name, const std::string& file_name);
		std::list<database> get_databases();

	private:
		std::mutex mutex;
		std::list<database> databases;
	};
}
