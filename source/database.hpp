#pragma once
#include "keilo.hpp"

namespace keilo {
	class database {
	public:
		explicit database(std::string database_name);
		explicit database(std::ifstream& file);
		database(const database& other);

		result_t create_table(const std::string& table_name, const std::string& key);
		result_t add_table(const table& other);
		result_t drop_table(const std::string& table_name);
		table* select_table(const std::string& table_name);

	private:
		void parse_file(std::ifstream& file);

	public:
		std::list<table> get_tables();
		std::string get_name() const;

	private:
		void set_name(const std::string& name);
		std::string name;
		std::mutex mutex;
		std::list<table> tables = std::list<table>();
	};

}
