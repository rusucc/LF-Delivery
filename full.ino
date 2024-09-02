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
#define REMOTEXY_BLUETOOTH_NAME "ESP 3.2 TDI"


#include <RemoteXY.h>

// RemoteXY GUI configuration
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =  // 612 bytes
  { 255, 45, 0, 43, 0, 93, 2, 18, 0, 0, 0, 16, 2, 106, 200, 200, 84, 3, 1, 0,
    0, 8, 0, 5, 8, 20, 60, 60, 7, 6, 54, 54, 32, 6, 16, 31, 4, 8, 173, 94,
    12, 11, 62, 146, 14, 128, 6, 8, 4, 84, 47, 12, 117, 141, 2, 14, 61, 0, 6, 8,
    2, 13, 119, 38, 24, 64, 50, 72, 10, 0, 190, 26, 31, 31, 79, 78, 0, 79, 70, 70,
    0, 129, 16, 81, 71, 29, 65, 36, 27, 12, 64, 8, 67, 108, 97, 119, 0, 131, 59, 62,
    21, 33, 160, 1, 40, 14, 2, 0, 6, 31, 80, 97, 103, 101, 32, 77, 97, 105, 110, 0,
    41, 131, 62, 12, 21, 33, 160, 17, 40, 14, 2, 0, 191, 31, 80, 97, 103, 101, 32, 49,
    0, 38, 131, 86, 86, 21, 33, 160, 33, 40, 14, 2, 0, 191, 31, 80, 97, 103, 101, 32,
    50, 0, 26, 15, 0, 68, 18, 41, 21, 95, 11, 7, 72, 70, 50, 16, 6, 15, 112, 118,
    0, 115, 101, 116, 112, 111, 105, 110, 116, 0, 67, 47, 21, 21, 24, 83, 12, 66, 5, 68,
    16, 24, 21, 129, 246, 16, 138, 29, 84, 19, 38, 8, 64, 8, 70, 111, 108, 108, 111, 119,
    32, 108, 105, 110, 101, 0, 2, 48, 91, 23, 52, 85, 29, 56, 10, 0, 6, 28, 31, 31,
    79, 78, 0, 79, 70, 70, 0, 131, 85, 26, 21, 33, 160, 0, 40, 14, 2, 0, 6, 31,
    80, 97, 103, 101, 32, 77, 97, 105, 110, 0, 9, 131, 85, 67, 21, 33, 160, 17, 40, 14,
    2, 0, 6, 31, 80, 97, 103, 101, 32, 49, 0, 6, 1, 25, 119, 57, 57, 86, 43, 15,
    15, 0, 1, 31, 67, 97, 108, 105, 98, 0, 7, 82, 83, 24, 24, 159, 35, 40, 10, 108,
    64, 2, 26, 2, 2, 129, 59, 74, 71, 29, 148, 37, 9, 7, 64, 24, 75, 112, 0, 7,
    84, 88, 24, 24, 159, 47, 40, 10, 108, 64, 2, 26, 2, 2, 129, 71, 93, 21, 17, 148,
    49, 7, 7, 64, 24, 75, 105, 0, 7, 85, 93, 24, 24, 159, 59, 40, 10, 108, 64, 2,
    26, 2, 2, 129, 72, 97, 21, 17, 148, 61, 9, 7, 64, 24, 75, 100, 0, 1, 38, 136,
    57, 57, 186, 72, 11, 11, 0, 2, 31, 0, 129, 39, 166, 71, 29, 164, 71, 18, 12, 64,
    17, 83, 101, 116, 0, 11, 0, 131, 83, 45, 21, 33, 157, 5, 40, 14, 2, 17, 2, 31,
    80, 97, 103, 101, 95, 109, 97, 105, 110, 0, 41, 131, 83, 86, 21, 33, 157, 20, 40, 14,
    2, 17, 2, 31, 80, 97, 103, 101, 0, 26, 7, 17, 17, 24, 24, 49, 7, 40, 10, 100,
    64, 2, 26, 2, 11, 7, 18, 50, 24, 24, 49, 21, 40, 10, 100, 64, 2, 26, 2, 11,
    129, 231, 9, 71, 29, 3, 7, 40, 12, 64, 17, 67, 117, 114, 114, 101, 110, 116, 0, 129,
    221, 21, 96, 29, 6, 20, 34, 12, 64, 17, 84, 97, 114, 103, 101, 116, 0, 1, 238, 83,
    57, 57, 5, 38, 24, 24, 0, 2, 31, 83, 101, 116, 0, 1, 252, 91, 57, 57, 31, 38,
    24, 24, 0, 2, 31, 71, 111, 0, 65, 16, 95, 43, 43, 61, 40, 18, 18, 112, 2, 52,
    17, 23, 52, 98, 7, 44, 22, 0, 2, 26, 31, 31, 79, 78, 0, 79, 70, 70, 0, 67,
    51, 102, 21, 24, 94, 46, 96, 8, 68, 2, 26, 11 };

// this structure defines all the variables and events of your control interface
struct {

  // input variables
  int8_t joystick_01_x;  // from -100 to 100
  int8_t joystick_01_y;  // from -100 to 100
  int8_t slider_01;      // from 0 to 100
  int8_t slider_02;      // from 0 to 100
  uint8_t switch_01;     // =1 if switch ON and =0 if OFF
  uint8_t switch_02;     // =1 if switch ON and =0 if OFF
  uint8_t button_01;     // =1 if button pressed, else =0
  float edit_01;
  float edit_02;
  float edit_03;
  uint8_t button_02;        // =1 if button pressed, else =0
  char current_node[11];    // string UTF8 end zero
  char direction_node[11];  // string UTF8 end zero
  uint8_t button_set;       // =1 if button pressed, else =0
  uint8_t button_go;        // =1 if button pressed, else =0
  uint8_t switch_03;        // =1 if switch ON and =0 if OFF

  // output variables
  float pv;
  float setpoint;
  char text_01[21];    // string UTF8 end zero
  uint8_t led_02_r;    // =0..255 LED Red brightness
  uint8_t led_02_g;    // =0..255 LED Green brightness
  uint8_t led_02_b;    // =0..255 LED Green brightness
  char decisions[11];  // string UTF8 end zero

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
float Kp = 0.1, Ki = 0, Kd = 0;
int base_pwm = 200;

int M1_A = D2, M1_B = D3;
int M2_A = D4, M2_B = D5;

bool trav[20] = {};
int cost[20] = {};
short int A[20][20] = {
  { 255, 2, 2, 255, 255, 255, 255, 255, 255 },
  { 2, 255, 1, 255, 255, 255, 255, 255, 255 },
  { 2, 1, 255, 1, 255, 255, 255, 255, 255 },
  { 255, 255, 1, 255, 1, 2, 255, 255, 255 },
  { 255, 255, 255, 1, 255, 2, 255, 1, 255 },
  { 255, 255, 255, 2, 2, 255, 255, 255, 255 },
  { 255, 255, 255, 255, 255, 255, 255, 1, 2 },
  { 255, 255, 255, 255, 1, 255, 1, 255, 2 },
  { 255, 255, 255, 255, 255, 255, 2, 2, 255 }
};
int parent[20] = {};
int n = 9;
int path[20] = {};
char decisions[50] = {};
int path_length = 0;
int connectionListLength = 34;

struct node {
  uint8_t rfid[4];
  int id, conn_number = 0;
  char letter;
  struct connections {
    node* from = nullptr;
    node* to = nullptr;
    char dir = NULL;  //l-left, f-forward, r-right
  } conn[20];
} v[20];

// current, from, to, direction
char connectionList[40][4] = {
  { 'A', 'B', 'C', 'F' },
  { 'A', 'C', 'B', 'F' },
  { 'B', 'A', 'C', 'L' },
  { 'B', 'C', 'A', 'R' },
  { 'C', 'A', 'D', 'L' },
  { 'C', 'B', 'D', 'F' },
  { 'C', 'B', 'A', 'L' },
  { 'C', 'D', 'A', 'R' },
  { 'C', 'A', 'B', 'R' },
  { 'C', 'D', 'B', 'F' },
  { 'D', 'C', 'F', 'L' },
  { 'D', 'E', 'F', 'L' },
  { 'D', 'F', 'E', 'L' },
  { 'D', 'C', 'E', 'F' },
  { 'D', 'E', 'C', 'F' },
  { 'D', 'F', 'C', 'R' },
  { 'F', 'E', 'D', 'F' },
  { 'F', 'D', 'E', 'F' },
  { 'E', 'H', 'F', 'R' },
  { 'E', 'D', 'F', 'L' },
  { 'E', 'F', 'D', 'R' },
  { 'E', 'H', 'D', 'F' },
  { 'E', 'F', 'H', 'L' },
  { 'E', 'D', 'H', 'F' },
  { 'H', 'E', 'I', 'L' },
  { 'H', 'G', 'I', 'R' },
  { 'H', 'E', 'G', 'F' },
  { 'H', 'I', 'G', 'L' },
  { 'H', 'I', 'E', 'R' },
  { 'H', 'G', 'E', 'F' },
  { 'G', 'I', 'H', 'R' },
  { 'G', 'H', 'I', 'L' },
  { 'I', 'H', 'G', 'F' },
  { 'I', 'G', 'H', 'F' },
};
short p[100][2];
int integral = 0, e_old = 0;
int out = 0;
int curr_node_id = 0;

void setup();
void turn_left();
void turn_right();
void djikstra(char start);
void generate_path(char start, char stop);
void connectionArrayToStruct();
void generate_decisions();
int calculatePosition();
void pick_up_object();
void put_down_object();
void follow_line();

bool delivery = false;
bool joystick_mode = true;
void loop() {
  RemoteXY_Handler();
  if (RemoteXY.button_02) {  //set gains
    Kp = RemoteXY.edit_01;
    Ki = RemoteXY.edit_02;
    Kd = RemoteXY.edit_03;
  }

  if (RemoteXY.button_go == 1 and delivery == false) {
    delivery = true;
    joystick_mode = false;
    char source = RemoteXY.current_node[0];
    char dest = RemoteXY.direction_node[0];
    char buf[20] = "";
    buf[0] = toupper(source);
    buf[1] = toupper(dest);
    buf[2] = '\0';
    strcpy(RemoteXY.decisions, buf);
    djikstra(buf[1]);
    char mesaj[10] = "Ok path";
    generate_path(buf[1], buf[0]);
    generate_decisions();
    strcpy(RemoteXY.decisions, decisions);
    pick_up_object();
  }
  if (RemoteXY.button_01) {  //calibrare
    strcpy(RemoteXY.text_01, "Calibrare");
    for (uint16_t i = 0; i < 400; i++) {
      qtr.calibrate();
      char chnumber[10];
      itoa(i, chnumber, 10);
      strcpy(RemoteXY.text_01, chnumber);
      RemoteXY_delay(5);
    }
    strcpy(RemoteXY.text_01, "Gata");
  }
  //afisare citire senzori
  int pwm_m1_a = 0;
  int pwm_m1_b = 0;
  int pwm_m2_a = 0;
  int pwm_m2_b = 0;

  pwm_m1_a = 0;
  pwm_m1_b = 0;
  pwm_m2_a = 0;
  pwm_m2_b = 0;
  if(!RemoteXY.switch_02) joystick_mode = false;
  if (joystick_mode) {  //joystick mode
    int delta = map(RemoteXY.joystick_01_x, -100, 100, -255, 255);
    int speed = map(RemoteXY.joystick_01_y, -100, 100, -255, 255);
    int pos_servo1 = map(RemoteXY.slider_01, 0, 100, 10, 90);
    int pos_servo2 = map(RemoteXY.slider_02, 0, 100, 45, 140);
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
    
    servo1.write(pos_servo1);
    servo2.write(pos_servo2);
    ledcWrite(M1_A, pwm_m1_a);
    ledcWrite(M1_B, pwm_m1_b);
    ledcWrite(M2_A, pwm_m2_a);
    ledcWrite(M2_B, pwm_m2_b);
    if (RemoteXY.switch_01) claw.write(0);
    else claw.write(90);
  } else {                     //follow line mode
    if (RemoteXY.switch_03) {  // delivery
      RemoteXY.led_02_r = 255;
      RemoteXY.led_02_b = 0;
      while (delivery == true) {
        if (rfid.PICC_IsNewCardPresent()) {  // am ajuns in intersectie
          RemoteXY.led_02_b = 255;
          ledcWrite(M1_A, 0);
          ledcWrite(M1_B, 0);
          ledcWrite(M2_A, 0);
          ledcWrite(M2_B, 0);
          char ch_curr_node[2];
          itoa(curr_node_id,ch_curr_node,10);
          RemoteXY_delay(1000);
          strcpy(RemoteXY.decisions,ch_curr_node);
          if (rfid.PICC_ReadCardSerial()) {
            if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {
              Serial.println(F("A new card has been detected."));
              // Store NUID into nuidPICC array
              for (byte i = 0; i < 4; i++) {
                nuidPICC[i] = rfid.uid.uidByte[i];
              }
              printHex(rfid.uid.uidByte, rfid.uid.size);
              Serial.println();
            }
            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
          }
          switch (decisions[curr_node_id]) {
            case 'L':  //left
              RemoteXY.led_02_g = 255 * 0.33;
              qtr.read(sensorValues);
              while (sensorValues[0] < 400) {
                qtr.read(sensorValues);
                turn_left();
                RemoteXY_delay(20);
                //de scos
              }
              ledcWrite(M1_A, 0);
              ledcWrite(M1_B, 0);
              ledcWrite(M2_A, 0);
              ledcWrite(M2_B, 0);
              RemoteXY_delay(300);
              break;
            case 'F':  //forward
              RemoteXY.led_02_g = 255 * 1;
              pwm_m1_a = 0;
              pwm_m1_b = 200;
              pwm_m2_a = 200;
              pwm_m2_b = 0;
              RemoteXY_delay(300);
              pwm_m1_a = 0;
              pwm_m1_b = 0;
              pwm_m2_a = 0;
              pwm_m2_b = 0;
              ledcWrite(M1_A, pwm_m1_a);
              ledcWrite(M1_B, pwm_m1_b);
              ledcWrite(M2_A, pwm_m2_a);
              ledcWrite(M2_B, pwm_m2_b);
              break;
            case 'R':  //right
              RemoteXY.led_02_g = 255 * 0.66;
              qtr.read(sensorValues);
              while (sensorValues[7] < 500) {
                qtr.read(sensorValues);
                turn_right();
                RemoteXY_delay(20);
              }
              ledcWrite(M1_A, 0);
              ledcWrite(M1_B, 0);
              ledcWrite(M2_A, 0);
              ledcWrite(M2_B, 0);
              RemoteXY_delay(300);
              break;
            default:
              break;
          }
          RemoteXY_delay(500);
          if (curr_node_id == strlen(decisions)-1) {
            delivery = false;
            put_down_object();
            char buf_end[20] = "Gata";
            strcpy(RemoteXY.decisions, buf_end);
            while(true){
              RemoteXY.led_02_r = 0;
              RemoteXY.led_02_g = 0;
              RemoteXY.led_02_b = 0;
              ledcWrite(M1_A, 0);
              ledcWrite(M1_B, 0);
              ledcWrite(M2_A, 0);
              ledcWrite(M2_B, 0);
              RemoteXY_delay(100);
              RemoteXY.led_02_r = 255;
              RemoteXY_delay(100);
            }
          }
          curr_node_id++;
        } else follow_line();
      }
    } else {
      follow_line();
    }
  }
  //turn_right();
  RemoteXY_delay(10);
}
void setup() {
  Serial.begin(115200);
  RemoteXY_Init();
  analogReadResolution(10);  //0-1024 READ RESOLUTION
  ledcAttach(M1_A, 5000, 8);
  ledcAttach(M1_B, 5000, 8);
  ledcAttach(M2_A, 5000, 8);
  ledcAttach(M2_B, 5000, 8);
  servo1.attach(D9);
  servo2.attach(D7);
  claw.attach(claw_pin);

  connectionArrayToStruct();
  //djikstra('A');
  //Serial.println("A si iesit din djikstra?");
  //generate_path('A','G');
  //generate_decisions();
  //Serial.println(decisions);

  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //qtr
  qtr.setTypeAnalog();
  qtr.setSensorPins(pins_qtr, number_qtr);
}
void follow_line() {
  qtr.read(sensorValues);
  int pwm_m1_a = 0;
  int pwm_m1_b = 0;
  int pwm_m2_a = 0;
  int pwm_m2_b = 0;
  int position = calculatePosition();
  RemoteXY.pv = position;
  RemoteXY.setpoint = 3500;
  int e = position - 3500;
  int P = Kp * e;
  integral += Ki * e;
  if (e * e_old < 0) integral = 0;
  int D = Kd * (e - e_old);
  out = P + integral + D;
  e_old = e;
  pwm_m1_b = base_pwm - out;
  pwm_m2_a = base_pwm + out;

  pwm_m1_a = (pwm_m1_b < 0) ? abs(pwm_m1_b) : 0;
  pwm_m1_a = (pwm_m1_a > 255) ? 255 : pwm_m1_a;
  pwm_m1_a = (pwm_m1_a < 0) ? 0 : pwm_m1_a;

  pwm_m1_b = (pwm_m1_b > 255) ? 255 : pwm_m1_b;
  pwm_m1_b = (pwm_m1_b < 0) ? 0 : pwm_m1_b;

  pwm_m2_b = (pwm_m2_a < 0) ? abs(pwm_m2_a) : 0;
  pwm_m2_b = (pwm_m2_b > 255) ? 255 : pwm_m2_b;
  pwm_m2_b = (pwm_m2_b < 0) ? 0 : pwm_m2_b;

  pwm_m2_a = (pwm_m2_a > 255) ? 255 : pwm_m2_a;
  pwm_m2_a = (pwm_m2_a < 0) ? 0 : pwm_m2_a;

  ledcWrite(M1_A, pwm_m1_a);
  ledcWrite(M1_B, pwm_m1_b);
  ledcWrite(M2_A, pwm_m2_a);
  ledcWrite(M2_B, pwm_m2_b);

  String msj = "1B:" + String(pwm_m1_b) + " | 2A:" + String(pwm_m2_a) + " | e:" + String(e);
  msj.toCharArray(RemoteXY.text_01, 20);
}
int calculatePosition() {
  int sum = 0;
  int w_avg = 0;
  line = false;
  int position = 0;
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
void turn_right() {
  int pwm_m1_a = 200;  //pentru facut stanga
  int pwm_m1_b = 0;
  int pwm_m2_a = 200;
  int pwm_m2_b = 0;
  ledcWrite(M1_A, pwm_m1_a);
  ledcWrite(M1_B, pwm_m1_b);
  ledcWrite(M2_A, pwm_m2_a);
  ledcWrite(M2_B, pwm_m2_b);
}
void turn_left() {
  int pwm_m1_a = 0;  //pentru facut stanga
  int pwm_m1_b = 200;
  int pwm_m2_a = 0;
  int pwm_m2_b = 200;
  ledcWrite(M1_A, pwm_m1_a);
  ledcWrite(M1_B, pwm_m1_b);
  ledcWrite(M2_A, pwm_m2_a);
  ledcWrite(M2_B, pwm_m2_b);
}
void pick_up_object() {
  claw.write(90);  //open
  RemoteXY_delay(100);
  servo2.write(45);  //inaltime
  servo1.write(0);   //xoy
  claw.write(0);
  RemoteXY_delay(100);
  for (int posDegrees = 45; posDegrees <= 90; posDegrees++) {
    servo2.write(posDegrees);
    RemoteXY_delay(20);
  }  //up
  for (int posDegrees = 0; posDegrees <= 90; posDegrees++) {
    servo1.write(posDegrees);
    RemoteXY_delay(20);
  }  //pan
  for (int posDegrees = 90; posDegrees >= 45; posDegrees--) {
    servo2.write(posDegrees);
    RemoteXY_delay(20);
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
  claw.write(90);
}
void connectionArrayToStruct() {
  for (int i = 0; i < connectionListLength; i++) {
    int nd = int(connectionList[i][0] - 'A');
    int from = int(connectionList[i][1] - 'A');
    int to = int(connectionList[i][2] - 'A');
    int dir = connectionList[i][3];
    v[nd].id = nd;
    v[nd].conn[v[nd].conn_number].from = &v[from];
    v[nd].conn[v[nd].conn_number].to = &v[to];
    v[nd].conn[v[nd].conn_number].dir = dir;
    v[nd].conn_number++;
    v[nd].letter = connectionList[i][0];
  }
}
void djikstra(char start) {
  Serial.println("****************************************");
  int id_start = int(start - 'A');
  for (int i = 0; i < n; i++) {
    cost[i] = 255;
    trav[i] = false;
  }
  cost[id_start] = 0;
  trav[id_start] = 1;
  parent[id_start] = -1;
  int p_start_index = 0;
  int p_end_index = 1;

  p[0][0] = id_start;
  p[0][1] = 0;
  while (p_start_index <= p_end_index) {
    delay(1);
    for (int i = 0; i < n; i++) {
      int A_buff = A[(p[p_start_index][0])][i];
      if (A_buff != 255) {  //este vecin
        int cost_inst = p[p_start_index][1] + A_buff;
        if (cost[i] > cost_inst) {  //daca traseul actual are un costa asociat mai mic decat cel cunoscut, il analizeaza
          cost[i] = cost_inst;
          if (trav[i] == 0) {
            parent[i] = p[p_start_index][0];
            p[p_end_index][0] = i;
            p[p_end_index][1] = cost_inst;
            p_end_index++;
            Serial.printf("Added element <%d,%d> to queue\n", i, cost_inst);
          }
        }
      }
    }
    trav[p[p_start_index][0]] = 1;
    p_start_index++;
    if (p_start_index > p_end_index) {
      Serial.printf("No more elements\n");
    }
    Serial.printf("Start %d, stop: %d \n", p_start_index, p_end_index);
  }
}
void generate_path(char start, char stop) {
  int id_start = start - 'A';
  int id_stop = stop - 'A';
  int curr = id_stop;
  path[path_length++] = id_stop;
  while (parent[curr] != -1) {
    curr = parent[curr];
    path[path_length++] = curr;
  }
  path[path_length] = id_start;
}
void generate_decisions() {
  for (int i = 1; i < path_length - 1; i++) {  //luam fiecare nod din path
    //path e int array
    node* current = &v[int(path[i])];
    for (int j = 0; j < current->conn_number; j++) {
      int fr_id = (current->conn[j]).from->id;
      int to_id = (current->conn[j]).to->id;
      if (fr_id == path[i - 1] and to_id == path[i + 1]) {
        char buf[2];
        buf[0] = (current->conn[j]).dir;
        buf[1] = '\0';
        strcat(decisions, buf);
      }
    }
  }
}
void printHex(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
