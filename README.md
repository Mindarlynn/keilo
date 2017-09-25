# keilo

"keilo" is c++ database server that created by yegu kwon(l0g3x). This code is based on c++ 14 and can be compiled with commonly available compilers such as MSVC. <br/>
Tested platform : Windows10 ver 1703(build 15063.608), msvc 14.1

## build
### 1. import codes directly to project
### 2. compile codes to static library file.

## data file
keilo database use <b><i>*.klo</i></b> as an extension.

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
