#include <MsTimer2.h>
#include <Wire.h> //I2C Arduino Library
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

#define LOG_OUT 1 // use the log output function
#define FHT_N 64 // set to 256 point fht

#include <FHT.h> // include the library

#define BUTTON1    A3
#define BUTTON2    A2

#define LED_BLUE   11
#define LED_GREEN  10
#define LED_RED    9
#define LED_UV     3

#define SERIAL_SPEED  9600
#define MIDSIZE       64

// HARDWARE I2C: A4 -> SDA, A5 -> SCL
// --------- HARDWARE SPI OLED: 11 -> MOSI/DIN, 13 -> SCK

#define PIN_RESET  4 // dummy
Adafruit_SSD1306 * oled = new Adafruit_SSD1306(PIN_RESET);

// wtv audio chip
/*
int resetPin = A1;  // The pin number of the reset RST
int busyPin  = A0;  // The pin number of the  busy P06
int clockPin = 12;  // The pin number of the clock P04
int dataPin  = 13;  // The pin number of the  data P05
*/

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t val;

int ticks = 0;

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
    
    _basic += 0.1 * (midget() - (float)_basic);
  }

  float midget() {
    float mid = 0;
    for (int i=0; i<MIDSIZE; ++i) mid += _val[i];
    
    return mid/(float)MIDSIZE;
  }

  void draw(byte x, byte y, float fak) {
    int q;
    
    int id = _nxt-1;
    float mid = midget();
    if (id < 0) id += MIDSIZE;

    byte lastx,lasty;
    byte dx = x + 1.0*(MIDSIZE);
    short dy = y - fak*((float)_val[id] - mid);
    
    for (int i=0; i<MIDSIZE; i++) {
      lastx = dx;
      lasty = dy;
      dx = x + 1.0*(MIDSIZE-i);
      dy = y - fak*((float)_val[id] - mid);
      if (dy < 0 || dy > 63) dy = 0;
      oled->drawLine(lastx, lasty, dx, dy, WHITE);
      id--;
      if (id < 0) id += MIDSIZE;
      fht_input[id] = _val[id];
    }
  }

  void drawf(byte x, byte y) {
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht
    for (int q=2; q<FHT_N/2; ++q) {
      oled->drawLine(x+2*(q-2), y, x+2*(q-2), y-fht_log_out[q], WHITE);
    }
  }
};

Midways * mee1;

inline void ticking() {
  ticks++;  
}

void setup() {
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE,  OUTPUT);
  pinMode(LED_UV,    OUTPUT);

  digitalWrite(LED_RED,   LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE,  LOW);
  digitalWrite(LED_UV,    LOW);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  //              .-------- sleep
  //              I .------ no temp
  //Wire.write(0b0101000);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(SERIAL_SPEED);
  //Serial.println("AT+NAME=I2C Step Watch");
  delay(800);
  
  mee1 = new Midways(128);
  oled->begin();
  oled->clearDisplay();
  oled->display();
  oled->setTextColor(WHITE);
  oled->setTextSize(1);

  MsTimer2::set(100, ticking);
  MsTimer2::start();
}

void loop() {

  if (ticks>5) {
    ticks=0;
    oled->clearDisplay();
    oled->setCursor(0, 0);
    oled->print(val);  
    mee1->draw (0,  32, 0.25);
    mee1->drawf(64, 63);
    oled->display();
  }
    
  Wire.beginTransmission(MPU);
  Wire.write(0x3D);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,1,true);

  val=128 + Wire.read();  // 0x3D (Accel_YOUT_H)
  mee1->add(val);
  delay(30);
}

