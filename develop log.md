## 2019_03_15
프로그램의 방향을 완전히 틀었다.<br/>
기존에는 Standalone Database server를 만드는 방향으로 프로젝트를 진행햐였다. 조금의 고뇌를 해보니 그것 보다는 Erlang의 ETS처럼 local database로 개발하는 것이 낫다고 판단하였다.<br/>
코드를 전체적으로 수정하면서 비 효율적이거나 가독성이 좋지 않은 코드를 전부 수정하였다. 나름대로 최대한 빠르게 돌 수 있게 구현하였다.

## 2018_05_19
5월 10일에는 tmporary file을 만드는 아이디어를 가지고있었다. 그 전에 file을 만들지 않고도 보낼 수 있지 않을까 생각해봤다.<br/>
그래서 compression을 plain text에 적용해보면 어떨까 해서 해보았다.<br/><br/>
해본 방법은 두가지다.<br/>
### 1. des & base64 encrytion 후 compress
이 방법을 가지고 plain text를 process 해보았다.<br/>
plain text는 688글자의 json-formatted string이다. <br/>
des & base64 encryption을 진행하면 920글자가 나왔다. 이 상태에서는 오버헤드가 너무 심해진다.<br/>
이 encrypted text를 compress하면 579글자가 나왔다. plain text보다는 조금 줄어든 것을 확인할 수 있었다.<br/>
### 2. compress 후 des & base64 encryption
plain text를 compress를 했을 경우 140글자가 나왔다.<br/>
이 compressed text를 encrypt 했을 때 192자가 나왔다.<br/>
1번 방법을 진행했을 때보다 훨씬 적은 글자의 text를 얻을 수 있었다.

이 2방법을 사용하여 평문 통신 방식을 교체할 예정이다.

## 2018_05_10
### 문제해결
원래 평문을 암호화 해서 보낼 예정이었으나, 평문 자체에 json 텍스트가 모두 포함되어 전송되는 방식을 취하고 있기 때문에 오브젝트가 많아지면 암호화를 할 수 없는 경우가 발생함을 확인함.<br/>
조금 더 개발을 하여 temporary json file을 생성하고, 생성한 파일을 binary코드로 읽어들인 후 암호화 하여 파일 자체를 전송하는 방식을 구현 진행목록에 저장해두었다.<br/>
기존 코드에서는 winsock 자체의 코드를 사용하였지만, tcp_client라는 wrapped 된 socket class를 만들어 코드를 더 깔끔하게 정리하였다. 이 부분에서 run 함수 안에 첫번째 람다함수에서 non-pointer variable인 client variable의 값을 제대로 가져오지 못하는 것을 발견하였다. client variable을 single-pointer variable로 변경하였고, client_list 등 non-pointer tcp_client variable을 다루는 variable과 functions를 double-pointer variable을 parameter로 받게 수정하였다. 또 기존의 코드에서는 single-pointer variable인 database variable과 table variable을 single-pointer reference type parameter로 받았었는데, 코드의 통일성을 위해 double-pointer variable type의 parameter를 받는 것으로 변경하였다.

## 2017_10_16
### 문제해결
클라이언트와의 세션 유지 전 유저 인증기능 구현함. <br/>
server 안에 user_database_ 변수에 유저 데이터베이스 파일을 import 해서 유저 정보를 가져옴. <br/>

## 2017_10_11
### 문제해결
1. 변수명 변경<br/>
2. 함수 주석 추가<br/>
3. 스코프 위치 변경<br/>
등의 리팩토링 진행하였음.

## 2017_10_10
### 문제해결
기존에 사용되던 독자적 포맷을 프로그램의 범용성을 위해 json으로 변경함. 외부 라이브러리를 사용함.

## 2017_10_03_2
### 문제해결
기존의 database와 table 변수를 single-pointer로 관리해 값을 넘겨주었는데 함수에서 call by value로 처리되었다.<br/>
single-pointer variable 자체가 하나의 value로 인식되었기 때문이다.<br/>
그래서 함수 인자값들을 double-pointer로 변경, 이로인해 클라이언트 개개인마다 독립적인 작업을 할 수 있게 되었다.<br/>

## 2017_10_03_1
### 문제해결
boost소켓 연결 후 read하는 부분에서 알수없는 exception이 발생해 boost에서 다시 winsock으로 변경.<br/>
프로젝트에서 boost 라이브러리 사용을 중지함.<br/>
현재 1대N 통신가능. 커맨드 테스트 완료.<br/>
<br/><br/>
### 에러
클라이언트의 세션 독립성을 위해 클라이언트별로 database와 table 변수를 가지게 하였음.<br/>
함수에서 변수에 대입을 하는 부분까지 제대로 동작하지만 함수를 탈출하면 변수에 기존의 데이터가 아닌 nullptr이 들어가있음.

## 2017_09_26
### 문제해결
2017_09_25에 winsock에서 오류가 발생하였기에 boost asio socket으로 변경함<br/>
<br/><br/>
### 에러
boost asio socket에 대한 이해도 부족으로 인해 test client에서 connect 불가 exception이 throw됨.<br/>

## 2017_09_25
### 문제해결
2017_09_23에 발생한 std::mutex내에 null이 들어가는 것은 상위 클래스인 keilo_application을 동적할당하여 해결함.<br/>
<br/><br/>
### 에러
recv함수가 client의 send를 waiting하지 않음. 그래서 메시지 처리를 제대로 못하는 버그 있음.<br/>
이 경우 client가 disconnect되는데, 이 때 상위 thread에서 client variable을 찾는 if문에서 에러가 발생함.<br/>
client의 소켓이 disconnect되어 그런 것으로 추정. Disconnect의 이유도 발견필요.<br/>
