# keilo

keilo는 c++로 제작된 데이터베이스 서버입니다. <br/>
c++17버전을 기반으로 제작되었고, MSVC와 같은 상용 컴파일러로 컴파일 가능합니다. <br/>
(Tested platform : Windows10 ver 1709(build 16299.431), msvc 14.1) <br/>

## 사용된 라이브러리

JSON for Modern C++ (nlohmann) - https://github.com/nlohmann/json <br/>
Crypto++ - https://www.cryptopp.com/ <br/>
zlib - https://zlib.net/ <br/>

## 빌드방법
원하는 대로 빌드하시면 됩니다. <br/>
(정적 라이브러리 추천) <br/>

## 파일
keilo는 하나의 json파일을 하나의 데이터베이스로 사용하고 있습니다. <br/>

### 파일 - 예시

```json
{
    "database1": [
        {
            "name": "table1", 
            "value": [
                {
                    "age": 20, 
                    "name": "kwonyegu"
                }, 
                {
                    "age": 20, 
                    "name": "xklest"
                }, 
                {
                    "age": 10, 
                }, 
                {
                    "age": 40, 
                }
            ]
        }, 
        {
            "name": "table2", 
            "value": [
                {
                    "english": 100, 
                    "history": 100, 
                    "korean": 100, 
                    "math": 100
                }, 
                {
                    "english": 50, 
                    "history": 50, 
                    "korean": 50, 
                    "math": 50
                }
            ]
        }
    ]
}
```

## 네트워킹

### 암호화된 평문
만약 데이터베이스의 내용이 너무 많아질 경우, 한정된 버퍼가 데이터베이스를 손실없이 온전하게 받을 수 있다고 보장할 수 없습니다. 그래서 클라이언트와 통신할때 DES알고리즘을 통해 암호화, Base64를 통해 인코딩을 진행한 데이터를 압축해서 통신합니다. <br/>
(암호화, 인코딩: Crypto++, 데이터 압축: zlib) <br/>

### 유저 인증
타 데이터베이스와 동일하게 허가된 유저만 데이터베이스에 접속하게 할 수 있습니다. ID와 Password를 통해 클라이언트에서 서버와 통신하기 위한 인증을 받을 수 있습니다. <br/>
유저 인증 기능을 사용하기 위해서는 유저의 ID와 Password를 기록하고 있는 json형식의 파일이 필요합니다. <br/>

#### 유저 인증 - 예시
```json
{
    "user_database": [
        {
            "name": "user", 
            "value": [
                {
                    "ID": "root", 
                    "Password": "root"
                }
            ]
        }
    ]
}
```

### 커맨드
서버를 사용하지 않고 프로젝트에서 직접 데이터베이스를 관리 할 수도 있습니다(via API). 그러나 현재 제작된 keilo server를 database server로 사용하는 경우가 일반적이라고 생각했기 때문에 클라이언트에서 커맨드를 입력해 데이터를 받아볼 수 있게 구현되어 있습니다(like SQL). <br/>

------
#### import database from file
```
import [file name];
```

-------
#### export database with file
```
export [file name];
```

-----------
#### create
##### database
```
create database [name];
```

##### table
```
create table [name];
```
-----------
#### select
##### database
```
select database [name]
```

##### table
###### ※데이터베이스를 select한 상태에서만 사용할 수 있습니다.
```
select table [name];
```

##### record
###### ※테이블을 select한 상태에서만 사용할 수 있습니다.
```
select record [field]:[value]; or select record all;
```
------------------
###### ※이후의 명령어들은 테이블을 select한 상태에서만 사용할 수 있습니다.
-----------
##### join (table)
###### ※각 테이블에 동일한 이름의 field가 무조건 하나 이상 존재해야만 합니다.

###### ※join할 table이 현재 데이터베이스에 있는 경우:
```
join [name];
```

###### ※다른 데이터베이스에 있을 경우
```
join [[database name]_[table name]];
```
------------------
##### insert
```
insert [field]:[value], ..., [field]:[value];
```
------------------
##### update
###### ※첫 번째로 나오는 field와 value의 경우는 condition, 두 번째로 나오는 field와 value는 replace할 새로운 value
```
update [field]:[value] [field]:[value];
```
------------------
##### remove
###### ※field와 value는 condition임.
```
remove [field]:[value];
```
------------------
