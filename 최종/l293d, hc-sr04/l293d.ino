// L293D 아두이노 핀
#define MOTORLATCH 12
#define MOTORCLK 4
#define MOTORENABLE 7
#define MOTORDATA 8
// 아두이노 PWM 핀
#define MOTOR1_PWM 6    //M3
#define MOTOR2_PWM 5    //M4

// 8-bit bus after the 74HC595 shift register 
// (not Arduino pins)
// These are used to set the direction of the bridge driver.
#define MOTOR1_A 5      //M3
#define MOTOR1_B 7
#define MOTOR2_A 0      //M4
#define MOTOR2_B 6

// 모터 제어 코드
#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4
//////////////////////////////////////////////////////////////
//HC-SR04
#define trig 22
#define echo 24

void setup() {
  pinMode(trig, OUTPUT); // trig(13)핀을 출력모드로 설정한다
  pinMode(echo, INPUT);  // echo(12)핀을 입력모드로 설정한다
  Serial.begin(9600);   // 보드레이트 설정
  Serial.println("Run scv");
}

void loop() {
  digitalWrite(trig, HIGH); // trig(13)핀에 HIGH신호를 보낸다
  delayMicroseconds(10);    // 10마이크로초(1/100,000초) 동안
  digitalWrite(trig, LOW);  // trig(13)핀에 LOW신호를 보낸다
  int distance = pulseIn(echo, HIGH, 100000) * 17 / 1000;
  // 10마이크로초 동안 수신이 되지 않으면 0을 반환
  // (0.0001 = micro초, 초음파 속도 340m/s, 왕복1번(나누기2))
  // d = 0.0001 x 340/2
  // cm값으로 변환시키면 (1000/17)
  // 변수 distance = echo핀이 다시 HIGH신호를 받기까지의 시간 * 17/1000
  Serial.print(distance);  // 변수 distance를 출력한다
  Serial.println("cm");    // 문자열 cm을 출력하고 줄바꿈

  if(distance >= 0 && distance < 10) {      // 장애물 감지(10cm 이내)
    motor_2wheel(RELEASE);
    delay(1000);
    right();                                // 우회전
    /*
    int rnd = random(0, 2);  // 장애물 감지시 좌/우회전 랜덤처리
    if(rnd == 0) {
      right();
    }
    else {
      left();
    }
    */
  }
  else {
    motor_2wheel(FORWARD);
  }
}
void motor_2wheel(int command) {
  switch (command)
    {
    case FORWARD:
      motor(1, FORWARD, 255);
      motor(2, FORWARD, 140);
      break;
    case BACKWARD:
      motor(1, BACKWARD, 128);
      motor(2, BACKWARD, 128);
      break;
    case BRAKE:
      motor(1, BRAKE, 0);
      motor(2, BRAKE, 0);
      break;
    case RELEASE:
      motor(1, RELEASE, 0);
      motor(2, RELEASE, 0);
      break;
    default:
      break;
    }
}
void right() {
  motor(1, FORWARD, 255);
  motor(2, RELEASE, 0);
  delay(2000);
}
void left() {
  motor(1, RELEASE, 0);
  motor(2, FORWARD, 140);
  delay(2000);
}
void motor(int nMotor, int command, int speed) {   // 각각의 모터를 제어하는 함수
  int motorA, motorB;

  if (nMotor >= 1 && nMotor <= 2) {  
    switch (nMotor) {
    case 1:
      motorA   = MOTOR1_A;
      motorB   = MOTOR1_B;
      break;
    case 2:
      motorA   = MOTOR2_A;
      motorB   = MOTOR2_B;
      break;
    default:
      break;
    }

    switch (command) {
    case FORWARD:
      motor_output (motorA, HIGH, speed);
      motor_output (motorB, LOW, -1);     // -1: no PWM set
      break;
    case BACKWARD:
      motor_output (motorA, LOW, speed);
      motor_output (motorB, HIGH, -1);    // -1: no PWM set
      break;
    case BRAKE:
      motor_output (motorA, LOW, 255); // 255: fully on.
      motor_output (motorB, LOW, -1);  // -1: no PWM set
      break;
    case RELEASE:
      motor_output (motorA, LOW, 0);  // 0: output floating.
      motor_output (motorB, LOW, -1); // -1: no PWM set
      break;
    default:
      break;
    }
  }
}

void motor_output (int output, int high_low, int speed) {  //motor() 함수를 backup하는 함수
  int motorPWM;

  switch (output) {
  case MOTOR1_A:
  case MOTOR1_B:
    motorPWM = MOTOR1_PWM;
    break;
  case MOTOR2_A:
  case MOTOR2_B:
    motorPWM = MOTOR2_PWM;
    break;
    
  default:
    speed = -3333;    // 유효하지 않은 speed 값
    break;
  }

  if (speed != -3333) {
    shiftWrite(output, high_low);   // shift register의 방향 설정
    if (speed >= 0 && speed <= 255) {
      analogWrite(motorPWM, speed);
    }
  }
}


// ---------------------------------
// shiftWrite
//
// The parameters are just like digitalWrite().
//
// The output is the pin 0...7 (the pin behind 
// the shift register).
// The second parameter is HIGH or LOW.
//
// There is no initialization function.
// Initialization is automatically done at the first
// time it is used.
//
void shiftWrite(int output, int high_low)
{
  static int latch_copy;
  static int shift_register_initialized = false;

  // Do the initialization on the fly, 
  // at the first time it is used.
  if (!shift_register_initialized)
  {
    // Set pins for shift register to output
    pinMode(MOTORLATCH, OUTPUT);
    pinMode(MOTORENABLE, OUTPUT);
    pinMode(MOTORDATA, OUTPUT);
    pinMode(MOTORCLK, OUTPUT);

    // Set pins for shift register to default value (low);
    digitalWrite(MOTORDATA, LOW);
    digitalWrite(MOTORLATCH, LOW);
    digitalWrite(MOTORCLK, LOW);
    // Enable the shift register, set Enable pin Low.
    digitalWrite(MOTORENABLE, LOW);

    // start with all outputs (of the shift register) low
    latch_copy = 0;

    shift_register_initialized = true;
  }

  // The defines HIGH and LOW are 1 and 0.
  // So this is valid.
  bitWrite(latch_copy, output, high_low);

  // Use the default Arduino 'shiftOut()' function to
  // shift the bits with the MOTORCLK as clock pulse.
  // The 74HC595 shiftregister wants the MSB first.
  // After that, generate a latch pulse with MOTORLATCH.
  shiftOut(MOTORDATA, MOTORCLK, MSBFIRST, latch_copy);
  delayMicroseconds(5);    // For safety, not really needed.
  digitalWrite(MOTORLATCH, HIGH);
  delayMicroseconds(5);    // For safety, not really needed.
  digitalWrite(MOTORLATCH, LOW);
}
