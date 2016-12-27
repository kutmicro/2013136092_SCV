#include <SoftwareSerial.h>
#define TX 15
#define RX 14

SoftwareSerial BTSerial(TX, RX);

void setup() {
  //모니터 프로그램 셋팅 및 시작
  Serial.begin(9600);
  //BT 시리얼 셋팅 및 시작
  BTSerial.begin(9600);
  Serial.print("hello");
  //Serial.println("Start The BT App");
}

void loop() {
  //데이터가 있으면 모니터프로그램으로 다시 전송
  if(BTSerial.available())
  Serial.write(BTSerial.read());
  
  //모니터 프로그램에서 입력 받은 것을 BT로 전송
  if(Serial.available())
  BTSerial.write(Serial.read());
}
