# keilo

[![Build Status](https://travis-ci.org/Mindarlynn/keilo.svg?branch=master)](https://travis-ci.org/Mindarlynn/keilo)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/b652ca6925cf4292a779c37a227b777e)](https://www.codacy.com/app/Mindarlynn/keilo?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Mindarlynn/keilo&amp;utm_campaign=Badge_Grade)<br/>
<i> for english user</i>
[README_ENG.md](./README_ENG.md)

keilo는 c++로 제작된 로컬 데이터베이스 입니다.. <br/>
c++17버전을 기반으로 제작되었고, MSVC와 같은 상용 컴파일러로 컴파일 가능합니다. <br/>
(Tested platform : Windows10 ver 1803(build 17134.649), Intel C++ Compiler 19.0) <br/>

## 사용된 라이브러리

JSON for Modern C++ (nlohmann) - https://github.com/nlohmann/json <br/>

## 파일
keilo는 하나의 json파일을 하나의 데이터베이스로 사용하고 있습니다. <br/>

### 파일 - 예시

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

## 사양
result_t 를 통해 함수의 결과를 알 수 있습니다.
```C++
enum class result_t{
    success,
    fail,
    has_no_value,
    cannot_find,
    already_exist,
    key_duplicated,
    key_not_exists,
    file_not_support
};
```
<br/>

### <b>application.hpp</b>
```C++
// result_t::already_exist : 인자로 들어온 이름의 DB가 이미 있는 경우
// result_t::success
result_t create_database(const std::string& database_name); 

// result_t::already_exist : 인자로 들어온 DB와 같은 이름의 DB가 이미 있는 경우
// result_t::success
result_t add_database(const keilo::database& database_name); 

// result_t::cannot_find : 인자로 들어온 이름의 DB가 없는 경우
// result_t::success
result_t drop_database(const std::string& database_name)

// 성공 : Suitable한 DB의 주소
// 실패 : nullptr
keilo::database* select_database(const std::string& database_name);

// result_t::cannot_find : 파일을 찾을 수 없는 경우
// result_t::success
result_t import_file(const std::string& file_name); 

// result_t::fail : json, ofstream 사용 중 예상치 못한 에러가 발생할 경우
// result_t::success
result_t export_database(const std::string& database_name, const std::string& file_name); 

// Application이 가지고 있는 모든 DB를 반환
std::list<keilo::database> get_databases();
```

### <b>database.hpp</b>
```C++
// result::already_exist : 인자로 들어온 이름의 테이블이 이미 있는 경우
// result_t::success
result_t create_table(const std::string& table_name, const std::string& key);

// result::already_exist : 인자로 들어온 레코드와 같은 키값을 가진 레코드가 이미 있는 경우
// result_t::success
result_t add_table(const keilo::record& record);

// result_t::cannot_find : 인자로 들어온 이름의 테이블이 없는 경우
// result_t::success
result_t drop_table(const std::string& table_name)

// 성공 : Suitable한 테이블의 주소
// 실패 : nullptr
table* select_table(const std::string& table_name);

// DB가 가지고 있는 모든 테이블을 반환
std::list<table> get_tables();

// DB의 이름을 반환
std::string get_name() const;
```

### <b>table.hpp</b>
```C++
// result_t::key_not_exist : 인자로 들어온 레코드에 테이블의 키값이 없는 경우
// result_t::key_duplicated : 인자로 들어온 레코드와 동일한 키값을 가진 레코드가 테이블에 있는 경우
// result_t::success
result_t insert_record(const record& record);
// result_t::cannot_find : 인자로 들어온 조건에 만족하는 레코드가 없을 경우
// result_t::key_duplicated : 치환될 데이터 중 키값이 있을 때, 이 키값이 테이블에 존재하는 레코드의 키값과 중복될 경우
// result_t::success
result_t update_record(const std::list<keilo::instance>& conditions, const std::list<keilo::instance>& replacements);
// result_t::cannot_find : 인자로 들어온 조건에 만족하는 레코드가 없을 경우
// result_t::success
result_t remove_record(const std::list<keilo::instance>& conditions);

// 성공 : Suitable한 모든 레코드를 반환
// 실패 : std::runtime_error
std::list<record> select_record(const std::list<keilo::instance>& conditions) const;

// 인자로 들어온 테이블과의 inner join한 테이블을 반환
keilo::table join(keilo::table& table);

// 키 순으로 레코드를 정렬
void sort(const bool& order = true);
		
// Custom 키를 사용해 레코드를 정렬
// 주의 : 정렬하려는 모든 레코드에 key값이 존재해야 함!
void sort(const char* sort_key, const bool& order = true);
```

### <b>record.hpp</b>
```C++
// 레코드가 가진 모든 인스턴스를 반환
std::list<instance> operator()();
```

### <b>instance.hpp</b>
```C++
// 인스턴스의 identifier(필드명)을 반환
std::string get_identifier() const;

// 인스턴스의 value(값)을 반환
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
