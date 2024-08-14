#include "BluetoothSerial.h"
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <QTRSensors.h>


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
int claw_pin = SDA;

Servo servo1, servo2, claw;

#define SS_PIN 5
#define RST_PIN 0
MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;
byte nuidPICC[4];

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE

#include <BLEDevice.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "ESP 3.2 TFSI"


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 416 bytes
  { 255,20,0,29,0,153,1,17,0,0,0,16,2,106,200,200,84,2,1,0,
  7,0,5,8,20,60,60,7,6,54,54,32,6,16,31,4,8,173,94,12,
  11,62,146,14,128,6,8,4,84,47,12,117,142,2,14,61,0,6,8,2,
  13,119,38,24,64,50,72,10,0,190,26,31,31,79,78,0,79,70,70,0,
  129,16,81,71,29,65,36,27,12,8,67,108,97,119,0,131,59,62,21,33,
  160,0,40,14,2,0,6,31,80,97,103,101,32,77,97,105,110,0,9,131,
  62,12,21,33,160,17,40,14,2,0,191,31,80,97,103,101,32,49,0,6,
  16,0,68,18,41,21,95,11,7,72,70,50,16,6,15,112,118,0,115,101,
  116,112,111,105,110,116,0,67,47,21,21,24,83,12,66,5,4,16,24,21,
  129,246,16,138,29,84,19,38,8,8,70,111,108,108,111,119,32,108,105,110,
  101,0,2,48,91,23,52,85,29,56,10,0,6,28,31,31,79,78,0,79,
  70,70,0,131,85,26,21,33,160,0,40,14,2,0,6,31,80,97,103,101,
  32,77,97,105,110,0,9,131,85,67,21,33,160,17,40,14,2,0,6,31,
  80,97,103,101,32,49,0,6,1,25,119,57,57,125,41,15,15,0,6,31,
  0,129,36,131,71,29,87,45,32,8,8,67,97,108,105,98,114,97,114,101,
  0,7,82,83,24,24,159,35,40,10,44,16,26,6,2,129,59,74,71,29,
  148,37,9,7,24,75,112,0,7,84,88,24,24,159,47,40,10,44,16,26,
  6,2,129,71,93,21,17,148,49,7,7,24,75,105,0,7,85,93,24,24,
  159,59,40,10,44,16,26,6,2,129,72,97,21,17,148,61,9,7,24,75,
  100,0,1,40,138,57,57,189,71,10,10,0,4,31,0,129,53,169,71,29,
  149,72,38,8,24,83,101,116,32,86,97,108,117,101,115,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t joystick_01_x; // from -100 to 100
  int8_t joystick_01_y; // from -100 to 100
  int8_t slider_01; // from 0 to 100
  int8_t slider_02; // from 0 to 100
  uint8_t switch_01; // =1 if switch ON and =0 if OFF
  uint8_t switch_02; // =1 if switch ON and =0 if OFF
  uint8_t button_01; // =1 if button pressed, else =0
  float edit_01;
  float edit_02;
  float edit_03;
  uint8_t button_02; // =1 if button pressed, else =0

    // output variables
  float pv;
  float setpoint;
  char text_01[21]; // string UTF8 end zero

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////


const int number_qtr = 8;
const uint8_t pins_qtr[] = { 36, 39, 34, 35, 15, 14, 12, 4 };
QTRSensors qtr;
uint16_t sensorValues[number_qtr];
bool line = false;
float Kp=0.01,Ki=0,Kd=0;
int base_pwm = 200;

int M1_A = D2, M1_B = D3;
int M2_A = D4, M2_B = D5;

void setup();

int calculatePosition();
void pick_up_object();
void put_down_object();
void loop() {
  RemoteXY_Handler();
  if (rfid.PICC_IsNewCardPresent()) {
    if (rfid.PICC_ReadCardSerial()) {
      if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {
        Serial.println(F("A new card has been detected."));

        // Store NUID into nuidPICC array
        for (byte i = 0; i < 4; i++) {
          nuidPICC[i] = rfid.uid.uidByte[i];
        }

        //SerialBT.println(F("The NUID tag is:"));
        //SerialBT.print(F("In hex: "));
        printHex(rfid.uid.uidByte, rfid.uid.size);
        Serial.println();
      }
      // Halt PICC
      rfid.PICC_HaltA();

      // Stop encryption on PCD
      rfid.PCD_StopCrypto1();
    }
  }
  if(RemoteXY.button_02){
    Kp=RemoteXY.edit_01;
    Ki=RemoteXY.edit_02;
    Kd=RemoteXY.edit_03;
  }
  if(RemoteXY.button_01){
    strcpy(RemoteXY.text_01,"Calibrare");
    for (uint16_t i = 0; i < 400; i++) {
      qtr.calibrate();
      char chnumber[10];
      itoa(i,chnumber,10);
      strcpy(RemoteXY.text_01,chnumber);
      RemoteXY_delay(5);
    }
    strcpy(RemoteXY.text_01,"Gata");
  }
  qtr.read(sensorValues);
  int pwm_m1_a = 0;
  int pwm_m1_b = 0;
  int pwm_m2_a = 0;
  int pwm_m2_b = 0;

  int delta = map(RemoteXY.joystick_01_x, -100, 100, -255, 255);
  int speed = map(RemoteXY.joystick_01_y, -100, 100, -255, 255);
  int pos_servo1 = map(RemoteXY.slider_01, 0, 100, 10, 90);
  int pos_servo2 = map(RemoteXY.slider_02, 0, 100, 45, 140);
  servo1.write(pos_servo1);
  servo2.write(pos_servo2);
  if (RemoteXY.switch_01) claw.write(0);
  else claw.write(90);

  int integral = 0,e_old=0;
  int out = 0;
  if(!RemoteXY.switch_02){
    pwm_m1_a = (delta > 0) ? delta : 0;
    pwm_m1_b = (delta > 0) ? 0 : -delta;
    pwm_m2_a = (delta > 0) ? delta : 0;
    pwm_m2_b = (delta > 0) ? 0 : -delta;
    if (speed > 0) {
      pwm_m1_b += speed;
      pwm_m2_a += speed;
    } else {
      pwm_m1_a += abs(speed);
      pwm_m2_b += abs(speed);
    }
  }
  else{
    int position=calculatePosition();
    RemoteXY.pv = position;
    RemoteXY.setpoint = 3500;
    int e = position-3500;
    int P = Kp*e;
    integral += Ki*e;
    if(e*e_old<0) integral=0;
    int D = Kd*(e-e_old);
    out = P+integral+D;
    e_old = e;

    pwm_m1_b=base_pwm-out;
    pwm_m2_a=base_pwm+out;

    pwm_m1_a=(pwm_m1_b<0)? abs(pwm_m1_b):0;
    pwm_m1_a=(pwm_m1_a>255)? 255:pwm_m1_a;
    pwm_m1_a=(pwm_m1_a<0)? 0:pwm_m1_a;

    pwm_m1_b=(pwm_m1_b>255)? 255:pwm_m1_b;
    pwm_m1_b=(pwm_m1_b<0)? 0:pwm_m1_b;
    
    pwm_m2_b=(pwm_m2_a<0)? abs(pwm_m2_a):0;
    pwm_m2_b=(pwm_m2_b>255)? 255:pwm_m2_b;
    pwm_m2_b=(pwm_m2_b<0)? 0:pwm_m2_b;

    pwm_m2_a=(pwm_m2_a>255)? 255:pwm_m2_a;
    pwm_m2_a=(pwm_m2_a<0)? 0:pwm_m2_a;

    String msj = "1B:"+String(pwm_m1_b)+" | 2A:" +String(pwm_m2_a)+" | e:"+String(e);
    msj.toCharArray(RemoteXY.text_01,20);
  }
  
  ledcWrite(M1_A, pwm_m1_a);
  ledcWrite(M1_B, pwm_m1_b);
  ledcWrite(M2_A, pwm_m2_a);
  ledcWrite(M2_B, pwm_m2_b);
  RemoteXY_delay(10);
}
void setup() {
  RemoteXY_Init();

  ledcAttach(M1_A, 5000, 8);
  ledcAttach(M1_B, 5000, 8);
  ledcAttach(M2_A, 5000, 8);
  ledcAttach(M2_B, 5000, 8);

  servo1.attach(D9);
  servo2.attach(D7);
  claw.attach(claw_pin);

  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //qtr
  qtr.setTypeAnalog();
  qtr.setSensorPins(pins_qtr, number_qtr);
}
int calculatePosition() {
  int sum = 0;
  int w_avg = 0;
  line = false;
  int position=0;
  for (int i = 0; i < number_qtr; i++) {
    if (sensorValues[i] > 300)
      line = true;
    if (sensorValues[i] > 100) {
      sum += sensorValues[i];
      w_avg += (i * 1000) * sensorValues[i];
    }
  }
  if (!line) {
    if (position < (number_qtr - 1) * 1000 / 2)
      position = 0;
    else
      position = (number_qtr - 1) * 1000;
  } else
    position = w_avg / sum;
  return position;
}
void pick_up_object() {
  claw.write(90);  //open
  delay(100);
  //analogWrite(servo3_pin, 0);
  for (int posDegrees = 45; posDegrees <= 90; posDegrees++) {
    servo2.write(posDegrees);
    delay(20);
  }  //up
  //up daca nu era deja
  for (int posDegrees = 90; posDegrees >= 0; posDegrees--) {
    servo1.write(posDegrees);
    delay(20);
  }  //pan
  for (int posDegrees = 90; posDegrees >= 45; posDegrees--) {
    servo2.write(posDegrees);
    delay(20);
  }  //down
  //claw
  claw.write(0);
  delay(100);
  for (int posDegrees = 45; posDegrees <= 90; posDegrees++) {
    servo2.write(posDegrees);
    delay(20);
  }  //up
  for (int posDegrees = 0; posDegrees <= 90; posDegrees++) {
    servo1.write(posDegrees);
    delay(20);
  }  //pan
  for (int posDegrees = 90; posDegrees >= 45; posDegrees--) {
    servo2.write(posDegrees);
    delay(20);
  }  //down
}
void put_down_object() {
  for (int posDegrees = 45; posDegrees <= 90; posDegrees++) {
    servo2.write(posDegrees);
    delay(20);
  }  //up
  for (int posDegrees = 90; posDegrees >= 0; posDegrees--) {
    servo1.write(posDegrees);
    delay(20);
  }  //pan
  for (int posDegrees = 90; posDegrees >= 45; posDegrees--) {
    servo2.write(posDegrees);
    delay(20);
  }  //down


  //analogWrite(servo3_pin, 0);
  claw.write(90);


  for (int posDegrees = 45; posDegrees <= 90; posDegrees++) {
    servo2.write(posDegrees);
    delay(20);
  }  //up
  for (int posDegrees = 0; posDegrees <= 90; posDegrees++) {
    servo1.write(posDegrees);
    delay(20);
  }  //pan
  for (int posDegrees = 90; posDegrees >= 45; posDegrees--) {
    servo2.write(posDegrees);
    delay(20);
  }  //down
}
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
