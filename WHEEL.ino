
// Настройки
#define DEBUG 0       // режим отладки
#define ENC_TYPE 0   // тип энкодера, 0 или 1
#define INV_WHEEL 0   // инверсия руля

// пины
#define ENC_A 5       // пин энкодера
#define ENC_B 6       // пин энкодера
#define POT_THR A0    // педаль газа
#define POT_BR A1     // педаль тормоза
#define D0 0          // кнопка 
#define D1 1          // кнопка 
#define D2 2          // кнопка 
#define D3 3          // кнопка 
#define D4 4          // кнопка 
#define D7 7          // кнопка
#define D8 8          // кнопка
#define LPWM 9          // кнопка
#define RPWM 10        // кнопка
#define R 11        // кнопка
#define D 12        // кнопка
#define D13 13        // кнопка
#define D16 A4       // кнопка
#define D17 A3       // кнопка
#define BCALL A5

volatile int encCounter = 0;
volatile boolean state0, lastState, turnFlag;
int throttleMin, throttleMax, brakeMin, brakeMax, wheelMax;
uint32_t timer;
#include <EEPROM.h>
#include "HID-Project.h"
void encTick();
void calibration();
void debug();
void setupTmr();
void gamepadTick();
void writeMotor();
void setup() {
  pinMode(D0, INPUT_PULLUP);
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, INPUT_PULLUP);
  pinMode(BCALL, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  pinMode(D8, INPUT_PULLUP);
  pinMode(LPWM, OUTPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(D, INPUT_PULLUP);
  pinMode(R, INPUT_PULLUP);
  pinMode(D13, INPUT_PULLUP);
  pinMode(D16, INPUT_PULLUP);
  pinMode(D17, INPUT_PULLUP);
  pinMode(ENC_A, INPUT_PULLUP);
//  calibration();
  setupTmr();
   debug();
  
  EEPROM.get(0, throttleMin);
  EEPROM.get(2, brakeMin);
  EEPROM.get(4, throttleMax);
  EEPROM.get(6, brakeMax);
  EEPROM.get(8, wheelMax);
  Gamepad.begin();
  //Serial.begin(9600);
}

void loop() {
  gamepadTick();
   // debug();
  writeMotor();
  // при нажатии кнопки калибровки скидываем позицию руля в 0
  if (!digitalRead(BCALL)) {
    encCounter = 0;
  }
}
void encTick() {
  state0 = digitalRead(ENC_A);
  if (state0 != lastState) {
#if (ENC_TYPE == 1)
    turnFlag = !turnFlag;
    if (turnFlag)
      encCounter += (digitalRead(ENC_B) != lastState) ? -1 : 1;
#else
    encCounter += (digitalRead(ENC_B) != lastState) ? -1 : 1;
#endif
    lastState = state0;
  }
}

// блокировка мотором
void writeMotor(){
  if(encCounter < -wheelMax && encCounter > -wheelMax-200){
    digitalWrite(LPWM, 250);
    digitalWrite(RPWM, 0);
  } else {
    digitalWrite(LPWM, 0);
    digitalWrite(RPWM, 0);
  }
  if(encCounter > wheelMax && encCounter < wheelMax+200){
    digitalWrite(LPWM, 0);
    digitalWrite(RPWM, 250);
  } else {
    digitalWrite(LPWM, 0);
    digitalWrite(RPWM, 0);
  }
}

// калибровка
void calibration() {
    int wm=1500;
    Serial.begin(9600);
    delay(100);
    Serial.print(F("Calibration start"));
    encCounter = 0;
    int zeroTHR = analogRead(POT_THR);
    int zeroBR = analogRead(POT_BR);
    int maxTHR, maxBR, maxWHEEL;

    EEPROM.put(0, zeroTHR);
    EEPROM.put(2, zeroBR);
    delay(100);                     // дебаунс
    for(int i=0;i<10;i++) {                  // крутимся
      
      maxTHR = analogRead(POT_THR);
      maxBR = analogRead(POT_BR);
      Serial.println(maxTHR);
      Serial.print("\t");
      Serial.print(maxBR);
      Serial.print("\t");
      delay(1000);
    }
    EEPROM.put(4, maxTHR);
    EEPROM.put(6, maxBR);
    EEPROM.put(8, wm);

    Serial.println(F("Calibration end"));
    Serial.print(F("Wheel: "));
    Serial.println(wm);
    Serial.print(F("Throat: "));
    Serial.print(zeroTHR);
    Serial.print(" - ");
    Serial.println(maxTHR);
    Serial.print(F("Brake: "));
    Serial.print(zeroBR);
    Serial.print(" - ");
    Serial.println(maxBR);
    Serial.println();
    Serial.end();
    delay(5000);
}

// дебаг
void debug() {

  Serial.begin(9600);
  uint32_t timer;
  
    encTick();
    if (millis() - timer > 100) {
      timer = millis();
      Serial.print(encCounter);
      Serial.print("\t");
      Serial.print(analogRead(POT_THR));
      Serial.print("\t");
      Serial.println(analogRead(POT_BR));
      Serial.print("\t");
    }
  
  Serial.end();
}

ISR(TIMER3_COMPA_vect) {
  encTick();
}

void setupTmr() {
  TCCR3B = 0b00001001;
  TIMSK3 = 0b00000010;
  OCR3AH = highByte(15999 / 2);
  OCR3AL = lowByte(15999 / 2);
}

void akpp(){
    bool ture = true;
    if(!digitalRead(D))ture=false;
    if(!digitalRead(R)) ture=false;
      if(ture) Gamepad.press(13);
       else Gamepad.release(13);
    if (!digitalRead(R)) Gamepad.press(8);
      else Gamepad.release(8);
    if (!digitalRead(D)) Gamepad.press(9);  
     else Gamepad.release(9);
}

void gamepadTick() {
  if (millis() - timer > 10) {
    timer = millis();
    int wheel;
    if (INV_WHEEL) wheel = constrain(-encCounter, -wheelMax, wheelMax);
    else wheel = constrain(encCounter, -wheelMax, wheelMax);
    wheel = map(wheel, -wheelMax, wheelMax, -32768, 32767);
    Gamepad.xAxis(wheel);

    int thr, br;
    thr = map(analogRead(POT_THR), throttleMin, throttleMax, -128, 127);
    thr = constrain(thr, -128, 127);
    Gamepad.zAxis(thr);

    br = map(analogRead(POT_BR), brakeMin, brakeMax, -128, 127);
    br = constrain(br, -128, 127);
    Gamepad.rzAxis(br);
    // Если кнопка нажата, нажать ее на геймпаде
    if (!digitalRead(D0)) Gamepad.press(1);
    else Gamepad.release(1);
    if (!digitalRead(D1)) Gamepad.press(2);
    else Gamepad.release(2);
    if (!digitalRead(D2)) Gamepad.press(3);
    else Gamepad.release(3);
    if (!digitalRead(D3)) Gamepad.press(4);
    else Gamepad.release(4);
    if (!digitalRead(D4)) Gamepad.press(5);
    else Gamepad.release(5);
    if (!digitalRead(D7)) Gamepad.press(6);
    else Gamepad.release(6);
    if (!digitalRead(D8)) Gamepad.press(7);
    else Gamepad.release(7);
    akpp();
    if (!digitalRead(D13)) Gamepad.press(10);
    else Gamepad.release(10);
    if (!digitalRead(D16)) Gamepad.press(11);
    else Gamepad.release(11);
    if (!digitalRead(D17)) Gamepad.press(12);
    else Gamepad.release(12);
  
    Gamepad.write();
  }
}
