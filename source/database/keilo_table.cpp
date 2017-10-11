#include "keilo_table.hpp"

#include <mutex>

keilo_table::keilo_table(const std::string name) : name_(name), records_(std::list<keilo_record>())
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
	auto joined_table{get_records()};
	auto other_table = other->get_records();

	for (auto& i_record : joined_table)
	{
		keilo_instance i_instance;

		for (const auto& instance : i_record)
		{
			if (instance.first != "index")
				continue;

			i_instance = instance;
			break;
		}

		auto found = false;

		for (auto& j_record : other_table)
		{
			for (const auto& j_instance : j_record)
			{
				if (j_instance.first == "index")
				{
					if (i_instance.second != j_instance.second)
						break;

					found = true;
				}
				else
					i_record.push_back(keilo_instance{j_instance.first, j_instance.second});
			}
			if (found)
				break;
		}
	}
	return keilo_table(get_name() + "+" + other->get_name(), joined_table);
}

keilo_record keilo_table::select_record(const keilo_instance where)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	for (auto record : records_)
	{
		for (const auto instance : record)
		{
			if (instance != where)
				continue;

			return record;
		}
	}
	return keilo_record();
}

std::string keilo_table::insert_record(keilo_record& record)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	auto pos = 0;
	std::string index;

	for (auto& i_instance : record)
	{
		if (i_instance.first != "index")
			continue;

		for (const auto& j_record : records_)
			for (const auto& j_instance : j_record)
			{
				if (i_instance != j_instance)
					continue;
				return "Record that has index \"" + i_instance.second + "\" is already exist in table \"" + get_name() + "\".";
			}

		index = i_instance.second;
		pos = atoi(index.c_str());
		break;
	}

	auto count = 0;
	auto it = records_.begin();

	while (it != records_.end() && ++count != pos)
		++it;

	records_.insert(it, record);
	return "Successfully inserted record that has index\"" + index + "\" to table \"" + get_name() + "\".";
}

std::string keilo_table::update_record(const keilo_instance from, const keilo_instance to)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	keilo_record* found_record = nullptr;
	auto found = false;

	for (auto& record : records_)
	{
		for (const auto& instance : record)
		{
			if (instance != from)
				continue;

			found_record = &record;
			found = true;
			break;
		}
		if (found)
			break;
	}
	if (!found)
		return "Record that has " + from.first + " \"" + from.second + "\" does not exist in table \"" + get_name() +
			"\".";

	auto changed = false;

	for (auto& instance : *found_record)
	{
		if (instance.first != to.first)
			continue;

		instance.second = to.second;
		changed = true;
		break;
	}
	if (!changed)
		return "Identifier \"" + to.first + "\" does not exist in table \"" + get_name() + "\".";
	return "Successfully updated record that has " + from.first + " \"" + from.second + "\" in table \"" + get_name() +
		"\".";
}

std::string keilo_table::remove_record(const keilo_instance where)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	std::list<keilo_record>::iterator it;
	auto found = false;

	for (auto record = records_.begin(); record != records_.end(); ++record)
	{
		for (auto instance = record->begin(); instance != record->end(); ++instance)
		{
			if (*instance != where)
				continue;

			it = record;
			found = true;
			break;
		}
		if (found)
			break;
	}
	if (!found)
		return "Record that has" + where.first + " \"" + where.second + "\" does not exist in table \"" + get_name() +
			"\".";

	records_.erase(it);
	return "Successfully removed record that has " + where.first + " \"" + where.second + "\" in table \"" +
		get_name() + "\"";
}

std::list<keilo_record> keilo_table::get_records()
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	return records_;
}

int keilo_table::count()
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
