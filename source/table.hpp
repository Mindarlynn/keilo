#pragma once
#include "keilo.hpp"

namespace keilo {
	class table {
	public:
		table(std::string table_name, std::string key);
		table(std::string table_name, std::string key, std::list<record> instances);
		table(const table& other);

		table& operator=(const table& other);

		result_t insert_record(const record& record);
		result_t update_record(const std::list<instance>& conditions, const std::list<instance>& replacements);
		result_t remove_record(const std::list<instance>& conditions);
		std::list<record> select_record(const std::list<instance>& conditions) const;

		table join(table&);

		void sort(const bool& order = true);
		void sort(const char* sort_key, const bool& order = true);


		std::list<record> get_records();
		std::string get_name() const;
		std::string get_key() const;
		size_t count();

	private:
		void set_name(std::string name);
		std::string name;
		std::string key;
		std::mutex mutex;
		std::list<record> records;
	};
}
