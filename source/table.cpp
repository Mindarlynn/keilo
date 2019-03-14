#include "core.hpp"

namespace keilo {
	table::table(std::string table_name, std::string key) : name(std::move(table_name)), key(std::move(key)) { }

	table::table(std::string table_name, std::string key, std::list<record> instances) : name(std::move(table_name)),
	                                                                                     key(std::move(key)),
	                                                                                     records(std::move(instances)) {
	}

	table::table(const table& other) { *this = other; }

	table& table::operator=(const table& other) {
		this->name = other.name;
		this->key = other.key;
		for (const auto& record : other.records)
			this->records.emplace_back(record);

		return *this;
	}

	table table::join(table& other) {
		std::unique_lock<std::mutex> lock(this->mutex);
		auto joined_table{records};
		auto other_table{other.records};

		for (auto& origin_record : joined_table)
			for (auto& other_record : other_table)
				for (auto& other_instance : other_record.instances)
					if (key != other_instance.identifier &&
						other_record.key.value == origin_record.key.value)
						origin_record.instances.emplace_back(other_instance);

		return {get_name() + '+' + other.get_name(), key, joined_table};
	}

	std::list<record> table::select_record(const std::list<instance>& conditions) const {
		auto dump = records;
		auto selected = std::list<record>();

		for (;;) {
			auto it = std::find_if(dump.cbegin(), dump.cend(),
			                       [conditions](const record& record) {
				                       auto found = true;
				                       for (const auto& condition : conditions) {
					                       if (record.instances.cend() == std::find_if(
							                       record.instances.cbegin(), record.instances.cend(),
							                       [condition](const instance& instance) {
								                       return instance == condition;
							                       })
					                       ) {
						                       found = false;
						                       break;
					                       }
				                       }
				                       return found;
			                       }
			);
			if (it != dump.cend()) {
				selected.emplace_back(*it);
				dump.erase(it);
			}
			else
				break;
		}

		return selected;
	}

	result_t table::insert_record(const record& record) {
		std::unique_lock<std::mutex> lock(this->mutex);

		if (record.key.identifier != this->key)
			return result_t::key_not_exist;

		const auto val = record.key.value;

		const auto pos = std::find_if(records.cbegin(), records.cend(), [this, val](const keilo::record& record) {
			return record.instances.cend() != std::find_if(record.instances.cbegin(), record.instances.cend(),
			                                               [this, val](const instance& instance) {
				                                               return instance.identifier == key && instance.value ==
					                                               val;
			                                               });
		});

		if (pos != records.cend())
			return result_t::key_duplicated;

		records.insert(pos, record);

		return result_t::success;
	}

	result_t table::update_record(const std::list<instance>& conditions, const std::list<instance>& replacements) {
		std::unique_lock<std::mutex> lock(mutex);

		// find suitable records
		const auto to_update = select_record(conditions);

		if (to_update.empty())
			return result_t::cannot_find;

		// check duplication of instance of key if there is modifying of key
		const auto dup = std::find_if(replacements.cbegin(), replacements.cend(),
		                              [this](const instance& instance) { return instance.identifier == key; }
		);
		if (dup != replacements.cend())
			if (records.cend() != std::find_if(records.cbegin(), records.cend(),
			                                   [&dup](const record& record) { return record.key == *dup; })
			)
				return result_t::key_duplicated;

		for (const auto& replacement : replacements) {
			for (const auto& be_update : to_update) {
				auto rt = std::find_if(records.begin(), records.end(),
				                       [&be_update](const record& record) { return be_update.key == record.key; }
				);

				auto it = std::find_if(rt->instances.begin(), rt->instances.end(),
				                       [replacement](const instance& instance) {
					                       return replacement.identifier == instance.identifier;
				                       }
				);
				it->value = replacement.value;
			}
		}

		return result_t::success;
	}

	result_t table::remove_record(const std::list<instance>& conditions) {
		std::unique_lock<std::mutex> lock(mutex);

		const auto selected_records = this->select_record(conditions);

		// when program cannot find suitable records
		if (selected_records.empty())
			return result_t::cannot_find;

		for (const auto& selected_record : selected_records) {
			auto it = std::find_if(this->records.cbegin(), this->records.cend(),
			                       [&selected_record](const record& origin_record) {
				                       return selected_record.key.value == origin_record.key.value;
			                       }
			);
			if (it != this->records.cend()) {
				auto tmp = it;
				++tmp;
				records.erase(it);
				it = tmp;
			}
		}

		return result_t::success;
	}

	std::list<record> table::get_records() {
		std::unique_lock<std::mutex> lock(mutex);
		return records;
	}

	size_t table::count() {
		std::unique_lock<std::mutex> lock(mutex);
		return records.size();
	}

	std::string table::get_key() const { return key; }

	void table::sort(const bool& order) {
		records.sort([order](const record& a, const record& b) {
			return order ? a.key.value < b.key.value : a.key.value > b.key.value;
		});
	}

	void table::sort(const char* sort_key, const bool& order) {
		records.sort([sort_key, order](const record& a, const record& b) {
			const auto a_key = std::find_if(a.instances.begin(), a.instances.end(),
			                                [sort_key](const instance& instance) {
				                                return sort_key == instance.identifier;
			                                });
			const auto b_key = std::find_if(b.instances.begin(), b.instances.end(),
			                                [sort_key](const instance& instance) {
				                                return sort_key == instance.identifier;
			                                });
			return order ? a_key->value < b_key->value : a_key->value > b_key->value;
		});
	}


	std::string table::get_name() const { return name; }

	void table::set_name(std::string name) { this->name = std::move(name); }

}
