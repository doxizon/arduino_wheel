#define INV_WHEEL 0 
#define citycar 1 // это чисто моя настройка для city car driving 
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

volatile int ec = 0;
volatile boolean s0, lasts;
int tMi, tMa, bMi, bMa, whMa;
uint32_t tim;
#include <EEPROM.h>
#include "HID-Project.h"

void encTick();
void calibration();
void debug();
void setupTmr();
void gT();
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
  
  EEPROM.get(0, tMi);
  EEPROM.get(2, bMi);
  EEPROM.get(4, tMa);
  EEPROM.get(6, bMa);
  EEPROM.get(8, whMa);
  Gamepad.begin();
}

void loop() {
  gT();
   debug();
  // writeMotor();
  // при нажатии кнопки калибровки скидываем позицию руля в 0
  if (!digitalRead(BCALL)) {
    ec = 0;
  }
}
void encTick() {
  s0 = digitalRead(ENC_A);
  if (s0 != lasts) {
    ec += (digitalRead(ENC_B) != lasts) ? -1 : 1;
    lasts = s0;
  }
}

// блокировка мотором
void writeMotor(){
  if(ec < -whMa){
    digitalWrite(LPWM, 250);
    digitalWrite(RPWM, 0);
  } else {
    digitalWrite(LPWM, 0);
    digitalWrite(RPWM, 0);
  }
  if(ec > whMa){
    digitalWrite(LPWM, 0);
    digitalWrite(RPWM, 250);
  } else {
    digitalWrite(LPWM, 0);
    digitalWrite(RPWM, 0);
  }
}

// калибровка
void calibration() {
    int wm=1800;
    Serial.begin(9600);
    delay(100);
    Serial.println("calibration");
    ec = 0;
    int zt = analogRead(POT_THR);
    int zb = analogRead(POT_BR);
    int maxTHR, maxBR, maxWHEEL;

    EEPROM.put(0, zt);
    EEPROM.put(2, zb);
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

    Serial.println(F("end"));
    Serial.print(F("Throat: "));
    Serial.print(zt);
    Serial.print(" - ");
    Serial.println(maxTHR);
    Serial.print(F("Brake: "));
    Serial.print(zb);
    Serial.print(" - ");
    Serial.println(maxBR);
    Serial.println();
    Serial.end();
    delay(10000);  
}

// дебаг
void debug() {
  Serial.begin(9600);
  uint32_t tim;
  encTick();
  if (millis() - tim > 100) {
    tim = millis();
    Serial.print(ec);
    Serial.print("\t");
    Serial.print(analogRead(POT_THR));
    Serial.print("\t");
    Serial.println(analogRead(POT_BR));
    Serial.print("\t");
  }
  Serial.end();
}

ISR(tim3_COMPA_vect) {
  encTick();
}
void setupTmr() {
  TCCR3B = 0b00001001;
  TIMSK3 = 0b00000010;
  OCR3AH = highByte(15999 / 2);
  OCR3AL = lowByte(15999 / 2);
}
// Настройка коробки передач для city car driving
void akpp(){
    bool ture = true;
    if(!digitalRead(D)) ture=false;
    if(!digitalRead(R)) ture=false;
      if(ture) Gamepad.press(13);
       else Gamepad.release(13);
    if (!digitalRead(R)) Gamepad.press(8);
      else Gamepad.release(8);
    if (!digitalRead(D)) Gamepad.press(9);  
     else Gamepad.release(9);
}

void gT() {
  if (millis() - tim > 10) {
    tim = millis();
    int wheel;
    if (INV_WHEEL) wheel = constrain(-ec, -whMa, whMa);
    else wheel = constrain(ec, -whMa, whMa);
    wheel = map(wheel, -whMa, whMa, -32768, 32767);
    Gamepad.xAxis(wheel);
    int thr, br;
    thr = map(analogRead(POT_THR), tMi, tMa, -128, 127);
    thr = constrain(thr, -128, 127);
    Gamepad.zAxis(thr);
    br = map(analogRead(POT_BR), bMi, bMa, -128, 127);
    br = constrain(br, -128, 127);
    Gamepad.rzAxis(br);
    // кнопки
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
    #ifndef citycar
      akpp(); 
    #else
      if (!digitalRead(D13)) Gamepad.press(8);
      else Gamepad.release(8);
      if (!digitalRead(D16)) Gamepad.press(9);
      else Gamepad.release(9);
    #endif
    if (!digitalRead(D13)) Gamepad.press(10);
    else Gamepad.release(10);
    if (!digitalRead(D16)) Gamepad.press(11);
    else Gamepad.release(11);
    if (!digitalRead(D17)) Gamepad.press(12);
    else Gamepad.release(12);
  
    Gamepad.write();
  }
}
