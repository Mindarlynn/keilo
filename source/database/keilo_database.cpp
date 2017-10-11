#include "keilo_database.hpp"
#include "json.hpp"

#include <string>
#include <fstream>
#include <unordered_map>

using json = nlohmann::json;

keilo_database::keilo_database(const std::string name) : name_(name), tables_(std::list<keilo_table>())
{
}

keilo_database::keilo_database(std::ifstream& file) : tables_(std::list<keilo_table>())
{
	parse_file(file);
}

keilo_database::keilo_database(const keilo_database& other) : name_(other.name_), tables_(other.tables_)
{
}

std::string keilo_database::create_table(const std::string name)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);

	for (const auto& table : tables_)
		if (table.get_name() == name)
			return "Table that was named \"" + name + "\" already exist in database.";

	tables_.push_back(keilo_table(name));
	return "Successfully create table that was named \"" + name + "\".";
}

std::string keilo_database::add_table(keilo_table& other)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	for (const auto& table : tables_)
	{
		if (table.get_name() != other.get_name())
			continue;

		return "Table \"" + other.get_name() + "\" already exist in database \"" + get_name() + "\".";
		//throw std::exception(("Table \"" + _table.get_name() + "\" already exist in database \"" + get_name() + "\".").c_str());
	}
	tables_.push_back(other);
	return "Successfully added table that was named \"" + other.get_name() + "\".";
}

keilo_table* keilo_database::select_table(const std::string name)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	for (auto& table : tables_)
	{
		if (table.get_name() != name)
			continue;

		return &table;
	}
	return nullptr;
}

std::string keilo_database::drop_table(const std::string name)
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	auto it = tables_.end();

	for (auto table = tables_.begin(); table != tables_.end(); ++table)
	{
		if (table->get_name() != name)
			continue;

		it = table;
		break;
	}

	if (it == tables_.end())
		return "Table that was named \"" + name + "\" dose not exist in database \"" + get_name() + "\".";

	tables_.erase(it);
	return "Successfully droped table that was named \"" + name + "\".";
}

void keilo_database::parse_file(std::ifstream& file)
{
	json js;
	file >> js;

	for (auto db = js.cbegin(); db != js.cend(); ++db)
	{
		set_name(db.key());

		for (auto tb = db->cbegin(); tb != db->cend(); ++tb)
		{
			std::list<keilo_record> records;
			for (auto rc = (*tb)["value"].cbegin(); rc != (*tb)["value"].cend(); ++rc)
			{
				keilo_record record;
				auto pos = 0;
				for (auto it = rc->cbegin(); it != rc->cend(); ++it)
				{
					const keilo_instance inst{it.key(), it.value().dump()};
					record.push_back(inst);
					if (it.key() == "index")
						pos = it.value();
				}

				auto it = records.cbegin();
				auto count = 0;
				while (it != records.cend() && ++count != pos)
					++it;
				records.insert(it, record);
			}
			mutex_.lock();
			tables_.push_back(keilo_table{(*tb)["name"], records});
			mutex_.unlock();
		}
	}
}

std::list<keilo_table> keilo_database::get_tables()
{
	std::lock_guard<std::mutex> mutex_guard(mutex_);
	return tables_;
}

std::string keilo_database::get_name() const
{
	return name_;
}

void keilo_database::set_name(const std::string name)
{
	name_ = name;
}
