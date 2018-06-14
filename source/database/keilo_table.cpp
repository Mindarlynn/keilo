#include "keilo_table.hpp"

#include <mutex>

keilo_table::keilo_table(const std::string& name) : name(name), records() {}

keilo_table::keilo_table(const std::string& name,
                         const std::list<keilo_record>& rows)
	: name(name), records(rows) {}

keilo_table::keilo_table(const keilo_table& other)
	: name(other.name), records(other.records) {}

keilo_table keilo_table::join(keilo_table* const other) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);
	const auto other_table{other->get_records()};
	auto joined_table{get_records()};

	for (auto& i_record : joined_table) {
		keilo_field i_field;

		for (const auto& field : i_record)
			if (field.first == "index") {
				i_field = field;
				break;
			}

		auto found = false;

		for (auto& j_record : other_table) {
			for (const auto& j_field : j_record)
				if (i_field == j_field)
					found = true;
				else
					i_record.emplace_back(j_field.first, j_field.second);
			if (found) break;
		}
	}
	return keilo_table(get_name() + "+" + other->get_name(), joined_table);
}

keilo_record* keilo_table::select_record(const keilo_field& where) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);

	for (auto& records : records)
		for (const auto field : records)
			if (field == where) return &records;
	return nullptr;
}

std::string keilo_table::insert_record(const keilo_record& record) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);
	std::string index;
	auto pos = 0;

	for (auto& i_field : record)
		if (i_field.first == "index") {
			for (const auto& j_record : records)
				for (const auto& j_field : j_record)
					if (i_field == j_field)
						return "Record that has index " + i_field.second +
						" is already exist in table " + get_name();

			index = i_field.second;
			pos = atoi(index.c_str());
			break;
		}

	auto count = 0;
	auto it = records.begin();

	while (it != records.end() && ++count != pos) ++it;

	records.insert(it, record);

	return "Successfully inserted record that has index" + index +
		" to table " + get_name();
}

std::string keilo_table::update_record(const keilo_field& from,
                                       const keilo_field& to) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);

	keilo_record* found_record = nullptr;

	for (auto& records : records) {
		for (const auto& field : records)
			if (field == from) {
				found_record = &records;
				break;
			}
		if (found_record) break;
	}
	if (!found_record)
		return "Record that has " + from.first + ' ' + from.second +
		" does not exist in table " + get_name();

	auto changed = false;

	for (auto& field : *found_record)
		if (field.first == to.first) {
			field.second = to.second;
			changed = true;
			break;
		}
	if (!changed)
		return "(Identifier " + to.first + " does not exist in table " +
		get_name();
	return "Successfully updated record that has " + from.first + ' ' +
		from.second + " in table " + get_name();
}

std::string keilo_table::remove_record(const keilo_field& where) {
	const std::lock_guard<std::mutex> mutex_guard(mutex);

	auto it = records.end();

	for (auto record = records.begin(); record != records.end(); ++record) {
		for (auto field = record->begin(); field != record->end(); ++field)
			if (*field == where) {
				it = record;
				break;
			}
		if (it != records.end()) break;
	}
	if (it == records.end())
		return "Record that has " + where.first + ' ' + where.second +
		" does not exist in table " + get_name();

	records.erase(it);
	return "Successfully removed record that has " + where.first + ' ' +
		where.second + " in table " + get_name();
}

std::list<keilo_record> keilo_table::get_records() {
	const std::lock_guard<std::mutex> mutex_guard(mutex);
	return records;
}

uint32_t keilo_table::count() {
	const std::lock_guard<std::mutex> mutex_guard(mutex);
	return records.size();
}

std::string keilo_table::get_name() const { return name; }

void keilo_table::set_name(const std::string& name) { this->name = name; }
