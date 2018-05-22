#include "keilo_table.hpp"

#include <mutex>

keilo_table::keilo_table(const std::string& name) : name_(name), records_()
{
}

keilo_table::keilo_table(const std::string& name, const std::list<keilo_record>& rows) : name_(name), records_(rows)
{
}

keilo_table::keilo_table(const keilo_table& other) : name_(other.name_), records_(other.records_)
{
}

keilo_table keilo_table::join(keilo_table* const other)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);
	const auto other_table{ other->get_records() };
	auto joined_table{ get_records() };

	for (auto& i_record : joined_table)
	{
		keilo_field i_field;

		for (const auto& field : i_record)
			if (field.first == "index")
			{
				i_field = field;
				break;
			}

		auto found = false;

		for (auto& j_record : other_table)
		{
			for (const auto& j_field : j_record)
				if (i_field == j_field)
					found = true;
				else
					i_record.emplace_back(j_field.first, j_field.second);
			if (found)
				break;
		}
	}
	return keilo_table(get_name() + "+" + other->get_name(), joined_table);
}

keilo_record* keilo_table::select_record(const keilo_field& where)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);

	for (auto record : records_)
		for (const auto field : record)
			if (field == where)
				return &record;
	return nullptr;
}

std::string keilo_table::insert_record(const keilo_record& record)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);
	std::string index;
	auto pos = 0;

	for (auto& i_field : record)
		if (i_field.first == "index")
		{
			for (const auto& j_record : records_)
				for (const auto& j_field : j_record)
					if (i_field == j_field)
						return R"(Record that has index ")" + i_field.second + R"(" is already exist in table ")" + get_name() +
							R"(".)";

			index = i_field.second;
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

std::string keilo_table::update_record(const keilo_field& from, const keilo_field& to)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);

	keilo_record* found_record = nullptr;

	for (auto& record : records_)
	{
		for (const auto& field : record)
			if (field == from)
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

	for (auto& field : *found_record)
		if (field.first == to.first)
		{
			field.second = to.second;
			changed = true;
			break;
		}
	if (!changed)
		return R"(Identifier ")" + to.first + R"(" does not exist in table ")" + get_name() + R"(".)";
	return "Successfully updated record that has " + from.first + R"( ")" + from.second + R"(" in table ")" + get_name() +
		R"(".)";
}

std::string keilo_table::remove_record(const keilo_field& where)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);

	auto it = records_.end();

	for (auto record = records_.begin(); record != records_.end(); ++record)
	{
		for (auto field = record->begin(); field != record->end(); ++field)
			if (*field == where)
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
	const std::lock_guard<std::mutex> mutex_guard(mutex_);
	return records_;
}

u_int keilo_table::count()
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);
	return records_.size();
}

std::string keilo_table::get_name() const
{
	return name_;
}

void keilo_table::set_name(const std::string& name)
{
	name_ = name;
}
