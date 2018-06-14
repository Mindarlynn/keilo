#include "keilo_database.hpp"
#include "json.hpp"

#include <string>
#include <fstream>

using json = nlohmann::json;

keilo_database::keilo_database(const std::string& name)
	: name(name), tables(std::list<keilo_table>()) {}

keilo_database::keilo_database(std::ifstream* const file)
	: tables(std::list<keilo_table>()) { parse_file(file); }

keilo_database::keilo_database(const keilo_database& other)
	: name(other.name), tables(other.tables) {}

std::string keilo_database::create_table(const std::string& name) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);

	for (const auto& table : tables)
		if (table.get_name() == name)
			return "Table that was named " + name + " already exist in database";

	tables.emplace_back(name);
	return "Successfully create table that was named " + name;
}

std::string keilo_database::add_table(const keilo_table& other) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);
	for (const auto& table : tables)
		if (table.get_name() == other.get_name())
			return "Table " + other.get_name() +
			" already exist in database " + get_name();

	tables.push_back(other);
	return "Successfully added table that was named " + other.get_name();
}

keilo_table* keilo_database::select_table(const std::string& name) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);
	for (auto& table : tables)
		if (table.get_name() == name) return &table;

	return nullptr;
}

std::string keilo_database::drop_table(const std::string& name) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);
	auto it = tables.end();

	for (auto table = tables.begin(); table != tables.end(); ++table)
		if (table->get_name() == name) {
			it = table;
			break;
		}

	if (it == tables.end())
		return "Table that was named " + name +
		" dose not exist in database " + get_name();

	tables.erase(it);
	return "Successfully droped table that was named " + name;
}

void keilo_database::parse_file(std::ifstream* const file) {
	json js;
	*file >> js;

	for (auto db = js.cbegin(); db != js.cend(); ++db) {
		set_name(db.key());

		for (const auto& tb : *db) {
			std::list<keilo_record> records;
			for (const auto& rc : tb["value"]) {
				keilo_record record;
				auto pos = 0;
				for (auto it = rc.cbegin(); it != rc.cend(); ++it) {
					auto val = it.value().dump();

					if (it.value().is_string()) {
						size_t strpos = 0;
						const std::string from = "\"";

						while ((strpos = val.find(from, strpos)) != std::string::npos) {
							val.replace(strpos, from.length(), "");
							strpos += from.length();
						}
					}

					record.emplace_back(it.key(), val);
					if (it.key() == "index") pos = it.value();
				}

				auto it = records.cbegin();
				auto count = 0;
				while (it != records.cend() && ++count != pos) ++it;
				records.insert(it, record);
			}
			mutex.lock();
			tables.emplace_back(tb["name"], records);
			mutex.unlock();
		}
	}
}

std::list<keilo_table> keilo_database::get_tables() {
	const std::lock_guard<std::mutex> mutex_guard(mutex);
	return tables;
}

std::string keilo_database::get_name() const { return name; }

void keilo_database::set_name(const std::string& name) { this->name = name; }
