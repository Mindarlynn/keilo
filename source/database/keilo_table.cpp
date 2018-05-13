#include "keilo_table.hpp"

#include <mutex>

keilo_table::keilo_table(const std::string name) : name_(name), records_()
{
}

keilo_table::keilo_table(const std::string name, const std::list<keilo_record> rows) : name_(name), records_(rows)
{
}

keilo_table::keilo_table(const keilo_table& other) : name_(other.name_), records_(other.records_)
{
}

keilo_table keilo_table::join(keilo_table* other)
{
	auto joined_table{ get_records() };
	auto other_table{ other->get_records() };

	for (auto& i_record : joined_table)
	{
		keilo_instance i_instance;

		for (const auto& instance : i_record)
			if (instance.first == "index")
			{
				i_instance = instance;
				break;
			}

		auto found = false;

		for (auto& j_record : other_table)
		{
			for (const auto& j_instance : j_record)
				if (i_instance == j_instance)
					found = true;
				else
					i_record.emplace_back(j_instance.first, j_instance.second);
			if (found)
				break;
		}
	}
	return keilo_table(get_name() + "+" + other->get_name(), joined_table);
}

keilo_record* keilo_table::select_record(const keilo_instance where)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	for (auto record : records_)
		for (const auto instance : record)
			if (instance == where)
				return &record;
	return nullptr;
}

std::string keilo_table::insert_record(keilo_record& record)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	std::string index;
	auto pos = 0;

	for (auto& i_instance : record)
		if (i_instance.first == "index")
		{
			for (const auto& j_record : records_)
				for (const auto& j_instance : j_record)
					if (i_instance == j_instance)
						return R"(Record that has index ")" + i_instance.second + R"(" is already exist in table ")" + get_name() +
							R"(".)";

			index = i_instance.second;
			pos = atoi(index.c_str());
			break;
		}

	auto count = 0;
	auto it = records_.begin();

	while (it != records_.end() && ++count != pos)
		++it;

	records_.insert(it, record);

	return R"(Successfully inserted record that has index")" + index + R"(" to table ")" + get_name() + R"(".)";
}

std::string keilo_table::update_record(const keilo_instance from, const keilo_instance to)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	keilo_record* found_record = nullptr;

	for (auto& record : records_)
	{
		for (const auto& instance : record)
			if (instance == from)
			{
				found_record = &record;
				break;
			}
		if (found_record)
			break;
	}
	if (!found_record)
		return "Record that has " + from.first + R"( ")" + from.second + R"(" does not exist in table ")" + get_name() +
			R"(".)";

	auto changed = false;

	for (auto& instance : *found_record)
		if (instance.first == to.first)
		{
			instance.second = to.second;
			changed = true;
			break;
		}
	if (!changed)
		return R"(Identifier ")" + to.first + R"(" does not exist in table ")" + get_name() + R"(".)";
	return "Successfully updated record that has " + from.first + R"( ")" + from.second + R"(" in table ")" + get_name() +
		R"(".)";
}

std::string keilo_table::remove_record(const keilo_instance where)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	auto it = records_.end();

	for (auto record = records_.begin(); record != records_.end(); ++record)
	{
		for (auto instance = record->begin(); instance != record->end(); ++instance)
			if (*instance == where)
			{
				it = record;
				break;
			}
		if (it != records_.end())
			break;
	}
	if (it == records_.end())
		return "Record that has " + where.first + '\"' + where.second + R"(" does not exist in table ")" + get_name() +
			R"(".)";

	records_.erase(it);
	return "Successfully removed record that has " + where.first + '\"' + where.second + R"(" in table ")" + get_name() +
		'\"';
}

std::list<keilo_record> keilo_table::get_records()
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	return records_;
}

u_int keilo_table::count()
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	return records_.size();
}

std::string keilo_table::get_name() const
{
	return name_;
}

void keilo_table::set_name(const std::string name)
{
	name_ = name;
}
