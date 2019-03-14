#include "keilo.hpp"

using json = nlohmann::json;

namespace keilo {
	database::database(std::string database_name) : name(std::move(database_name)) { }

	database::database(std::ifstream& file) { parse_file(file); }

	database::database(const database& other) : name(other.name) {
		for (const auto& table : other.tables)
			this->tables.emplace_back(table);
	}

	result_t database::create_table(const std::string& table_name, const std::string& key) {
		std::unique_lock<std::mutex> lock(mutex);

		if (tables.cend() != std::find_if(tables.begin(), tables.end(),
		                                  [table_name](const table& table) { return table.get_name() == table_name; })
		)
			return result_t::already_exist;

		tables.emplace_back(table_name, key);
		return result_t::success;
	}

	result_t database::add_table(const table& other) {
		std::unique_lock<std::mutex> lock(mutex);

		if (tables.cend() != std::find_if(tables.begin(), tables.end(),
		                                  [other](const table& table) { return table.get_name() == other.get_name(); })
		)
			return result_t::already_exist;

		tables.emplace_back(other);
		return result_t::success;
	}

	result_t database::drop_table(const std::string& table_name) {
		std::unique_lock<std::mutex> lock(mutex);

		const auto it = std::find_if(tables.begin(), tables.end(), [table_name](const table& table) {
			return table_name == table.get_name();
		});

		if (it == tables.end())
			return result_t::cannot_find;

		tables.erase(it);
		return result_t::success;
	}

	table* database::select_table(const std::string& table_name) {
		std::unique_lock<std::mutex> lock(mutex);

		const auto it = std::find_if(tables.begin(), tables.end(), [table_name](const table& table) {
			return table_name == table.get_name();
		});

		return it == tables.end() ? nullptr : &*it;
	}

	void database::parse_file(std::ifstream& file) {
		json js;
		file >> js;

		for (auto db = js.cbegin(); db != js.cend(); ++db) {
			set_name(db.key());

			for (const auto& tb : *db) {
				std::list<record> records;
				for (const auto& rc : tb["value"]) {
					record record{};
					for (auto it = rc.cbegin(); it != rc.cend(); ++it) {
						auto val = it.value().dump();

						if (it.value().is_string()) {
							size_t strpos = 0;
							const std::string from = R"(")";

							while ((strpos = val.find(from, strpos)) != std::string::npos) {
								val.replace(strpos, from.length(), "");
								strpos += from.length();
							}
						}
						record.instances.emplace_back(it.key(), val);
						if (tb["key"].get<std::string>() == it.key())
							record.key = *--record.instances.end();
					}
					records.emplace_back(record);
				}
				mutex.lock();
				tables.emplace_back(tb["name"].get<std::string>(), tb["key"].get<std::string>(), records);
				mutex.unlock();
			}
		}
	}

	std::list<table> database::get_tables() {
		std::unique_lock<std::mutex> lock(mutex);
		return tables;
	}

	std::string database::get_name() const { return name; }

	void database::set_name(const std::string& name) { this->name = name; }

}
