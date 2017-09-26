## 2017_09_25

문제해결:<br/>

2017_09_23에 발생한 std::mutex내에 null이 들어가는 것은 상위 클래스인 keilo_application을 동적할당하여 해결함.<br/>
<br/><br/>
에러:<br/>

recv함수가 client의 send를 waiting하지 않음. 그래서 메시지 처리를 제대로 못하는 버그 있음.<br/>
이 경우 client가 disconnect되는데, 이 때 상위 thread에서 client variable을 찾는 if문에서 에러가 발생함.<br/>
client의 소켓이 disconnect되어 그런 것으로 추정. Disconnect의 이유도 발견필요.<br/>

## 2017_09_26

문제해결:<br/>
2017_09_25에 winsock에서 오류가 발생하였기에 boost asio socket으로 변경함<br/>
<br/><br/>
에러:<br/>
boost asio socket에 대한 이해도 부족으로 인해 test client에서 connect 불가 exception이 throw됨.<br/>
