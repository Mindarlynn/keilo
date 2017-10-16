## 2017_09_25

### 문제해결

2017_09_23에 발생한 std::mutex내에 null이 들어가는 것은 상위 클래스인 keilo_application을 동적할당하여 해결함.<br/>
<br/><br/>
### 에러

recv함수가 client의 send를 waiting하지 않음. 그래서 메시지 처리를 제대로 못하는 버그 있음.<br/>
이 경우 client가 disconnect되는데, 이 때 상위 thread에서 client variable을 찾는 if문에서 에러가 발생함.<br/>
client의 소켓이 disconnect되어 그런 것으로 추정. Disconnect의 이유도 발견필요.<br/>

## 2017_09_26

### 문제해결
2017_09_25에 winsock에서 오류가 발생하였기에 boost asio socket으로 변경함<br/>
<br/><br/>

### 에러
boost asio socket에 대한 이해도 부족으로 인해 test client에서 connect 불가 exception이 throw됨.<br/>

## 2017_10_03_1

### 문제해결
boost소켓 연결 후 read하는 부분에서 알수없는 exception이 발생해 boost에서 다시 winsock으로 변경.<br/>
프로젝트에서 boost 라이브러리 사용을 중지함.<br/>
현재 1대N 통신가능. 커맨드 테스트 완료.<br/>
<br/><br/>

### 에러
클라이언트의 세션 독립성을 위해 클라이언트별로 database와 table 변수를 가지게 하였음.<br/>
함수에서 변수에 대입을 하는 부분까지 제대로 동작하지만 함수를 탈출하면 변수에 기존의 데이터가 아닌 nullptr이 들어가있음.

## 2017_10_03_2

### 문제해결
기존의 database와 table 변수를 single-pointer로 관리해 값을 넘겨주었는데 함수에서 call by value로 처리되었다.<br/>
single-pointer variable 자체가 하나의 value로 인식되었기 때문이다.<br/>
그래서 함수 인자값들을 double-pointer로 변경, 이로인해 클라이언트 개개인마다 독립적인 작업을 할 수 있게 되었다.<br/>

## 2017_10_10

### 문제해결
기존에 사용되던 독자적 포맷을 프로그램의 범용성을 위해 json으로 변경함. 외부 라이브러리를 사용함.
 
## 2017_10_11
### 문제해결
1. 변수명 변경<br/>
2. 함수 주석 추가<br/>
3. 스코프 위치 변경<br/>
등의 리팩토링 진행하였음.

## 2017_10_16
### 문제해결
클라이언트와의 세션 유지 전 유저 인증기능 구현함. <br/>
server 안에 user_database_ 변수에 유저 데이터베이스 파일을 import 해서 유저 정보를 가져옴. <br/>
