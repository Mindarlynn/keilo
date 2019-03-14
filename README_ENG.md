# keilo

[![Build Status](https://travis-ci.org/Mindarlynn/keilo.svg?branch=master)](https://travis-ci.org/Mindarlynn/keilo)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/b652ca6925cf4292a779c37a227b777e)](https://www.codacy.com/app/Mindarlynn/keilo?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Mindarlynn/keilo&amp;utm_campaign=Badge_Grade)<br/>
<i> for korean user</i>
[README.md](./README.md)

keilo is the local database that was developed by C++.<br/>
It was developed with C++17, and you can compile this project into a compiler such as MSVC.<br/>
(Tested platform : Windows10 ver 1803(build 17134.649), Intel C++ Compiler 19.0) <br/>

## Used library

JSON for Modern C++ (nlohmann) - https://github.com/nlohmann/json <br/>

## File
keilo uses one json file as one database.<br/>

### File - example

```json
{
    "db1":[
       {
          "key":"name",
          "name":"table1",
          "value":[
             {
                "hobby":"soccer",
                "name":"a"
             },
             {
                "hobby":"baseball",
                "name":"c"
             },
             {
                "hobby":"basketball",
                "name":"b"
             },
             {
                "hobby":"game",
                "name":"e"
             },
             {
                "hobby":"movie",
                "name":"d"
             }
          ]
       }
    ]
 }
 
```

## Specification
You can get result of function via result_t.
```C++
enum class result_t{
    success,
    fail,
    has_no_value,
    cannot_find,
    already_exist,
    key_overlapped,
    key_not_exists,
    file_not_support
};
```
<br/>

### <b>application.hpp</b>
```C++
// result_t::already_exist : A database with the name entered by the parameter already exists.
// result_t::success
result_t create_database(const std::string& database_name); 

// result_t::already_exist : A database with the name entered by the parameter already exists.
// result_t::success
result_t add_database(const keilo::database& database_name); 

// result_t::cannot_find : No database with name entered by parameter.
// result_t::success
result_t drop_database(const std::string& database_name)

// Success : Address of the database that meets the condition.
// Fail : nullptr
keilo::database* select_database(const std::string& database_name);

// result_t::cannot_find : The file cannot be found from the file name entered as a parameter.
// result_t::success
result_t import_file(const std::string& file_name); 

// result_t::fail : An unexpected error occurred while using the json and ofstream.
// result_t::success
result_t export_database(const std::string& database_name, const std::string& file_name); 

// Return all the databases Application has
std::list<keilo::database> get_databases();
```

### <b>database.hpp</b>
```C++
// result::already_exist : A table with the name entered by the parameter already exists.
// result_t::success
result_t create_table(const std::string& table_name, const std::string& key);

// result::already_exist : A table with the key entered by the parameter already exists.
// result_t::success
result_t add_table(const keilo::record& record);

// result_t::cannot_find : No table with name entered by parameter.
// result_t::success
result_t drop_table(const std::string& table_name)

// Success : Address of the table that meets the condition.
// Fail : nullptr
table* select_table(const std::string& table_name);

// Return all the tables Database has.
std::list<table> get_tables();

// Return Database's name.
std::string get_name() const;
```

### <b>table.hpp</b>
```C++
// result_t::key_not_exist : The record entered by the parameter does not have a key value for the table.
// result_t::key_overlapped : A record with the same key value as the record entered by the parameter is in the table.
// result_t::success
result_t insert_record(const record& record);
// result_t::cannot_find : No records are available that meet the conditions entered by the parameter.
// result_t::key_overlapped : When any of the values to be exchanged have a key value, that key value overlaps the record present in the table.
// result_t::success
result_t update_record(const std::list<keilo::instance>& conditions, const std::list<keilo::instance>& replacements);
// result_t::cannot_find : No records are available that meet the conditions entered by the parameter.
// result_t::success
result_t remove_record(const std::list<keilo::instance>& conditions);

// Success : Returns all records that meet the condition
// Fail : std::runtime_error
std::list<record> select_record(const std::list<keilo::instance>& conditions) const;

// Returns one table of innner join with table entered by parameter
keilo::table join(keilo::table& table);

// Sort records by key
// true = ascending
// false = descending
void sort(const bool& order = true);
		
// Sort records by custom key
// WARNING : All records you want to sort must have a key value.
void sort(const char* sort_key, const bool& order = true);
```

### <b>record.hpp</b>
```C++
// Return all the instances Record has
std::list<instance> operator()();
```

### <b>instance.hpp</b>
```C++
// Return Instance's identifier.
std::string get_identifier() const;

// Return Instance's value.
std::string get_value() const;
```

## Example code
```C++
#include <iostream>
#include <string>

#include <application.hpp>

int main() {
	try {
		keilo::application app{};

		// (application.hpp)
		app.create_database("db1");
		auto db1 = app.select_database("db1");

		const keilo::database db2{ "db2" };
		app.add_database(db2);

		// adding database
		std::cout << "<<adding database>>" <<"\n\n";
		for (const auto& db : app.get_databases())
			std::cout << db.get_name() << std::endl;
		std::cout << std::endl;

		app.drop_database("db2");

		// dropping database
		std::cout << "<<dropping database>>" <<"\n\n";
		for (const auto& db : app.get_databases())
			std::cout << db.get_name() << std::endl;
		std::cout << std::endl;

		// (database.hpp)
		db1->create_table("tb1", "name");
		auto tb1 = db1->select_table("tb1");

		const keilo::table tb2{ "tb2", "name" };
		db1->add_table(tb2);

		// adding table
		std::cout<<"<<adding table>>" <<"\n\n";
		for(const auto& tb : db1->get_tables()) 
			std::cout << tb.get_name() << std::endl;
		std::cout << std::endl;

		db1->drop_table("tb2");

		// dropping table
		std::cout << "<<dropping table>>" <<"\n\n";
		for (const auto& tb : db1->get_tables())
			std::cout << tb.get_name() << std::endl;
		std::cout << std::endl;

		// (table.hpp)

		// inserting
		tb1->insert_record({ {{"name", "a"}, {"hobby", "soccer"}}, tb1->get_key() });
		tb1->insert_record({ {{"name", "c"}, {"hobby", "baseball"}}, tb1->get_key() });
		tb1->insert_record({ {{"name", "b"}, {"hobby", "basketball"}}, tb1->get_key() });
		tb1->insert_record({ {{"name", "e"}, {"hobby", "game"}}, tb1->get_key() });
		tb1->insert_record({ {{"name", "d"}, {"hobby", "movie"}}, tb1->get_key() });

		std::cout << "<<inserting>>" <<"\n\n";
		for (auto& record : tb1->get_records())
			for (const auto& instance : record())
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		// updating
		tb1->update_record({ {"name", "a"} }, { {"hobby", "jogging"} });

		std::cout << "<<updating>>" <<"\n\n";
		for (auto& record : tb1->get_records())
			for (const auto& instance : record())
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		// removing
		tb1->remove_record({ {"hobby", "jogging"} });

		std::cout << "<<removing>>" <<"\n\n";
		for (auto& record : tb1->get_records())
			for (const auto& instance : record())
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		// joining
		keilo::table tb3{ "tb3", "name" };
		tb3.insert_record({ {{"name", "a"}, {"age", "10"}} , tb3.get_key() });
		tb3.insert_record({ {{"name", "b"}, {"age", "12"}} , tb3.get_key() });
		tb3.insert_record({ {{"name", "c"}, {"age", "13"}} , tb3.get_key() });
		tb3.insert_record({ {{"name", "d"}, {"age", "14"}} , tb3.get_key() });

		auto tb1_3 = tb1->join(tb3);

		std::cout << "<<joining>>" <<"\n\n";
		for(auto& record : tb1_3.get_records())
			for(const auto& instance : record()) 
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		// sorting
		tb1_3.sort(false);

		std::cout << "<<sorting>> (false = descending)" <<"\n\n";
		for (auto& record : tb1_3.get_records())
			for (const auto& instance : record())
				std::cout << instance <<"\n\n";
		std::cout << std::endl;

		app.export_database("db1", "db1.json");
	}
	catch(std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}
```
