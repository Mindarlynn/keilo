# keilo

"keilo" is c++ database server that created by yegu kwon(l0g3x). This code is based on c++ 14 and can be compiled with commonly available compilers such as MSVC. <br/>
Tested platform : Windows10 ver 1703(build 15063.608), msvc 14.1

## used libraries
### json
https://github.com/nlohmann/json

## build
### 1. import codes directly to project
### 2. compile codes to static library file.

## commands
### create
database : create database [database name];<br/>
table : create table [table name];<br/>
### select
database : select database [database name];<br/>
table : select table [table name];<br/>
record : select record [identifier]:[value] or all;<br/>
### join (table)
join [(other database name_)other table name]
### insert (record)
insert [identifier]:[value], ..., [identifier]:[value];
### update (record)
update (where)[identifier]:[value] (new)[identifier]:[value];
### remove (record)
remove [identifier]:[value];
### export file
export [file name];
### import file
import [file name];
### clear console
clear;

## data file
keilo database use <b><i>*.json</i></b> as an extension.
