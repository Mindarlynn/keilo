# keilo

"keilo" is c++ database server that created by yegu kwon(l0g3x). This code is based on c++ 14 and can be compiled with commonly available compilers such as MSVC. <br/>
Tested platform : Windows10 ver 1703(build 15063.608)

## build
### 1. import codes directly to project
### 2. compile codes to static library file.

## example code
```c++
#include <iostream>
#include <exception>
#include <keilo_server.hpp>

#pragma comment(lib, "keilo_server.lib")

int main(){
  keilo_server server;
  
  try{
    // import data file.
    // throw exception when file does not exist.
    server.import_file("example.klo");

    // select database.
    // throw exception when database does not exist in server.
    keilo_database database = server.select_database("database");
    
    // select table.
    // throw exception when table does not exist in database.
    keilo_table table1 = database->select_table("table1");
    keilo_table table2 = database->select_table("table2");
    
    // join with value of index.
    keilo_table joined_table = table1->join(table2);
    
    // keilo_record : std::list<keilo_field>
    keilo_record record;
    
    // keilo_instance : std::pair<std::string, std::string>
    keilo_instance index{ "index", "10" };
    keilo_instance name{ "name", "yegu kwon" };
    
    // parameters(attribute, field)
    record.push_back(index);
    record.push_back(name);
    
    // insert row into table that sort element with value of index.
    // throw exception when table has element that has same index with parameter's index.
    joined_table.insert(record);
    
    // throw exception when attributes or fields does not exist.
    // parameters(key, value)
    joined_table.update(index, name);
    
    // throw exception when attribute and field does not exist.
    // parameters(instance)
    joined_table.remove(name);
    
    // add table to database.
    // throw exception when there is table that has same name as parameters'
    database->add_table(joined_table);
    
    // throw exception when attribute or field does not exist.
    // parameters(attribute, field)
    auto selected_record = table1->select_record(index);
    
    for(const auto& instance : selected_record){
      std::cout << instance.first << " :" << instance.second << std::endl;
    }
    
    // select all rows
    auto records = joined_table.get_records();
    for(const auto& record : records){
      for(const auto& instance : record){
         std::cout << instance.first << " :" << instance.second << std::endl;
      }
    }
    
    // export database to file
    server.export_database("database", "example2.klo");
  }
  catch(std::exception& e){
    std::cerr << e.what() << std::endl;
  }
}
```

## data file
keilo database use "*.klo" as an extension.

### element
- name[] : database
- name{} : table
- () : record
- a:b; : instance

### format
```
database
[
  table1
  {
    (
      index:1;
      name:yegu kwon;
      age:19;
    )
    (
      index:2;
      name:l0g3x;
      age:19;
    )
  }
  table2
  {
    (
      index:1;
      math:70;
      english:80;
    )
    (
      index:2;
      math:50;
      english:90;
    )
  }
]
```
