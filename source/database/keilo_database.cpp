#include "keilo_database.hpp"
#include "json.hpp"

#include <string>
#include <fstream>

using json = nlohmann::json;

keilo_database::keilo_database(const std::string& name) : name_(name), tables_(std::list<keilo_table>())
{
}

keilo_database::keilo_database(std::ifstream* const file) : tables_(std::list<keilo_table>())
{
	parse_file(file);
}

keilo_database::keilo_database(const keilo_database& other) : name_(other.name_), tables_(other.tables_)
{
}

std::string keilo_database::create_table(const std::string& name)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);

	for (const auto& table : tables_)
		if (table.get_name() == name)
			return R"(Table that was named ")" + name + R"(" already exist in database.)";

	tables_.emplace_back(name);
	return R"(Successfully create table that was named ")" + name + R"(".)";
}

std::string keilo_database::add_table(const keilo_table& other)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);
	for (const auto& table : tables_)
		if (table.get_name() == other.get_name())
			return R"(Table ")" + other.get_name() + R"(" already exist in database ")" + get_name() + R"(".)";

	tables_.push_back(other);
	return R"(Successfully added table that was named ")" + other.get_name() + R"(".)";
}

keilo_table* keilo_database::select_table(const std::string& name)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);
	for (auto& table : tables_)
		if (table.get_name() == name)
			return &table;

	return nullptr;
}

std::string keilo_database::drop_table(const std::string& name)
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);
	auto it = tables_.end();

	for (auto table = tables_.begin(); table != tables_.end(); ++table)
		if (table->get_name() == name)
		{
			it = table;
			break;
		}

	if (it == tables_.end())
		return R"(Table that was named ")" + name + R"(" dose not exist in database ")" + get_name() + R"(".)";

	tables_.erase(it);
	return R"(Successfully droped table that was named ")" + name + R"(".)";
}

void keilo_database::parse_file(std::ifstream* const file)
{
	json js;
	*file >> js;

	for (auto db = js.cbegin(); db != js.cend(); ++db)
	{
		set_name(db.key());

		for (const auto& tb : *db)
		{
			std::list<keilo_record> records;
			for (const auto& rc : tb["value"])
			{
				keilo_record record;
				auto pos = 0;
				for (auto it = rc.cbegin(); it != rc.cend(); ++it)
				{
					auto val = it.value().dump();

					if(it.value().is_string())
					{
						size_t strpos = 0;
						const std::string from = R"(")";

						while ((strpos = val.find(from, strpos)) != std::string::npos)
						{
							val.replace(strpos, from.length(), "");
							strpos += from.length();
						}
					}

					record.emplace_back(it.key(), val);
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
			tables_.emplace_back(tb["name"], records);
			mutex_.unlock();
		}
	}
}

std::list<keilo_table> keilo_database::get_tables()
{
	const std::lock_guard<std::mutex> mutex_guard(mutex_);
	return tables_;
}

std::string keilo_database::get_name() const
{
	return name_;
}

void keilo_database::set_name(const std::string& name)
{
	name_ = name;
}
