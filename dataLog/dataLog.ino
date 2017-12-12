#include <Wire.h> //I2C Arduino Library
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

#define PIN_RESET  4 // dummy

#define BUTTON1    A3
#define BUTTON2    A2

#define LED_BLUE   11
#define LED_GREEN  10
#define LED_RED    9
#define LED_OFF    LOW
#define LED_ON     HIGH

#define SERIAL_SPEED  9600
#define MIDSIZE       40
#define XDOUBLE       1.0

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t GcX,GcY,GcZ,Tmp,AcX,AcY,AcZ;

// The temperature sensor is -40 to +85 degrees Celsius.
// It is a signed integer.
// According to the datasheet: 
//   340 per degrees Celsius, -512 at 35 degrees.
// At 0 degrees: -512 - (340 * 35) = -12412
double dT;

Adafruit_SSD1306 * oled = new Adafruit_SSD1306(PIN_RESET);

struct Midways {
  uint8_t _basic;
  uint8_t _val[MIDSIZE];
  int _nxt;

  Midways(uint8_t initval) {
    _nxt = 0;
    _basic = initval;
    for (int i=0; i<MIDSIZE; ++i) { 
      _val[i] = _basic;
    }
  }

  void add(float val) {
    _val[_nxt] = val;
    _nxt++;
    if (_nxt == MIDSIZE) _nxt = 0;
    
    /*  modify basic with 50% of difference from middle.
     *  This makes it easier to equalize pressure changes on a longer timespan and
     *  force detecting altitude changes in a short timespan.
     */
    _basic += 0.1 * (midget() - (float)_basic);
  }

  float midget() {
    float mid = 0;
    for (int i=0; i<MIDSIZE; ++i) mid += _val[i];
    
    return mid/(float)MIDSIZE;
  }

  void draw(byte x, byte y, float fak) {
    int id = _nxt-1;
    float mid = midget();
    if (id < 0) id += MIDSIZE;

    byte lastx,lasty;
    byte dx = x + XDOUBLE*(MIDSIZE);
    short dy = y - fak*((float)_val[id] - mid);
    
    for (int i=0; i<MIDSIZE; ++i) {
      lastx = dx;
      lasty = dy;
      dx = x + XDOUBLE*(MIDSIZE-i);
      dy = y - fak*((float)_val[id] - mid);
      if (dy < 0 || dy > 63) dy = 0;
      oled->drawLine(lastx, lasty, dx, dy, WHITE);
      id--;
      if (id < 0) id += MIDSIZE;
    }
  }
};

Midways * mee1;
Midways * mee2;
Midways * mee3;

Midways * mee4;
Midways * mee5;
Midways * mee6;

void setup() {
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE,  OUTPUT);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  //            .-------- sleep
  //            I .------ no temp
  //Wire.write(0b0101000);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(SERIAL_SPEED);
  Serial.println("AT+NAME=I2C Data Raw");
  delay(800);
  
  mee1 = new Midways(128);
  mee2 = new Midways(128);
  mee3 = new Midways(128);
  mee4 = new Midways(128);
  mee5 = new Midways(128);
  mee6 = new Midways(128);
  
  digitalWrite(LED_RED,   LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE,  LOW);
  
  oled->begin();
  oled->clearDisplay();
  oled->display();
  oled->setTextColor(WHITE);
  oled->setTextSize(1);
}

void loop() {
  //delay(7); // "refresh rate"
      
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);  // request a total of 14 registers

  // +/- 32768
  GcX=128+(Wire.read()<<8|Wire.read())/256;  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GcY=128+(Wire.read()<<8|Wire.read())/256;  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GcZ=128+(Wire.read()<<8|Wire.read())/256;  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  
  AcX=128+(Wire.read()<<8|Wire.read())/256;  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=128+(Wire.read()<<8|Wire.read())/256;  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=128+(Wire.read()<<8|Wire.read())/256;  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  dT = ( (double) Tmp + 12412.0) / 340.0;

  oled->clearDisplay();
  oled->setCursor( 0, 0);
  oled->print(GcX);
  oled->setCursor(43, 0);
  oled->print(GcY);
  oled->setCursor(87, 0);
  oled->print(GcZ);

  oled->setCursor( 0, 32);
  oled->print(AcX);
  oled->setCursor(43, 32);
  oled->print(AcY);
  oled->setCursor(87, 32);
  oled->print(AcZ);

  oled->setCursor(0, 54);
  oled->print((int)dT);

  mee1->add(GcX);
  mee2->add(GcY);
  mee3->add(GcZ);
  mee1->draw(0,  16, 0.3);
  mee2->draw(43, 16, 0.3);
  mee3->draw(87, 16, 0.3);
  
  mee4->add(AcX);
  mee5->add(AcY);
  mee6->add(AcZ);
  mee4->draw(0,  48, 0.1);
  mee5->draw(43, 48, 0.1);
  mee6->draw(87, 48, 0.1);
  
  oled->display();
}

