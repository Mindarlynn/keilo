# keilo

"keilo" is c++ database server that created by yegu kwon(l0g3x). This code is based on c++ 17 and can be compiled with commonly available compilers such as MSVC. <br/>
Tested platform : Windows10 ver 1709(build 16299.15), msvc 14.1

## used libraries
### json
https://github.com/nlohmann/json
### cryptoPP
https://www.cryptopp.com/

## build
### compile codes to static library file.

## networking
### crypted texts
Keilo uses one json file as database. At that time, if database is so big, limited buffer cannot receive whole completed database.<br/>
So, while communicate with client, we support encrypting plain text with des algorithme and compressing encrypted text.</br>

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

### example
```json
{
    "database1": 
    [
        {
            "name": "table1",
            "value":
            [
                {
                    "age": 19, 
                    "index": 1,
                    "name": "kwonyegu"
                }, 
                {
                    "age": 19,
                    "index": 2,
                    "name": "xklest"
                }, 
                {
                    "age": 10,
                    "index": 3
                }, 
                {
                    "age": 40,
                    "index": 4
                }
            ]
        }, 
        {
            "name": "table2",
            "value": 
            [
                {
                    "english": 100,
                    "history": 100,
                    "index": 1,
                    "korean": 100,
                    "math": 100
                }, 
                {
                    "english": 50,
                    "history": 50,
                    "index": 2,
                    "korean": 50,
                    "math": 50
                }
            ]
        }
    ]
}
```
## user authentication
### server
server have to import user file to use authentication. 
#### example format 
```json
{
    "user_database" : [
        {
            "name" : "user",
            "value" : [
                {
                    "ID" : "root",
                    "Password" : "root"
                }
            ]
        }
    ]
}
```
### client
client have to communicate about login before execute commands and receive result of login.
