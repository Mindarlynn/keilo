# keilo

"keilo" is c++ database server that created by yegu kwon(l0g3x). This code is based on c++ 14 and can be compiled with commonly available compilers such as MSVC. 
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
    keilo_table table1 = database.select_table("table1");
    keilo_table table2 = database.select_table("table2");
    
    // join with value of index.
    keilo_table joined_table = table1.join(table2);
    
    // keilo_row : std::list<keilo_field>
    keilo_row tmp_row;
    
    // keilo_field : std::pair<std::string, std::string>
    keilo_field index = std::make_pair("index", "10");
    keilo_field name = std::make_pair("name", "yegu kwon");
    
    // parameters(attribute, field)
    row.push_back(index);
    row.push_back(name);
    
    // insert row into table that sort element with value of index.
    // throw exception when table has element that has same index with parameter's index.
    joined_table.insert(add_row);
    
    // throw exception when attributes or fields does not exist.
    // parameters(dst_attribute, dst_field, src_attribute, src_field)
    joined_table.update("index", "10", "name", "yegu kwon");
    
    // throw exception when attribute and field does not exist.
    // parameters(attribute, field)
    joined_table.remove("name", "yegu kwon");
    
    // throw exception when attribute or field does not exist.
    // parameters(attribute, field)
    auto selected_row = table1.select_row("index", "10");
    
    for(const auto& field : selected_row){
      std::cout << field.first << " :" << field.second << std::endl;
    }
    
    // select all rows
    auto rows = joined_table.get_rows();
    for(const auto& row : rows){
      for(const auto& field : row){
         std::cout << field.first << " :" << field.second << std::endl;
      }
    }
  }
  catch(std::exception& e){
    std::cerr << e.what() << std::endl;
  }
}
```

## data file
keilo database use "*.klo" as an extension.

name[] : database
name{} : table
() : row
a:b; : record

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
