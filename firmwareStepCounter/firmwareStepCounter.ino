#include <MsTimer2.h>
#include <Wire.h>
#include <EEPROM.h>

int eeAddress = 0;

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
long stoss;
long barrier = 0;

#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

/* HARDWARE I2C: A4 -> SDA, A5 -> SCL */

#define BUTTON1        2
#define STEPLED       13
#define OLED_RESET     4 // (unused)

#define DELAYTRESHOLD  3
#define DELAYOFF      20

#define MAX_POWER  4100.0
#define MIN_POWER  3500.0

Adafruit_SSD1306 display(OLED_RESET);


// battery
byte maxLength = 28;
int  vcc;

int seconds  = 0;
int minutes  = 0;
int hours    = 0;
long steps   = 0;

int onsec = 0;
int tsec = 0;
long treshold = 1;
bool showTreshold = false;

// The temperature sensor is -40 to +85 degrees Celsius.
// It is a signed integer.
// According to the datasheet: 
//   340 per degrees Celsius, -512 at 35 degrees.
// At 0 degrees: -512 - (340 * 35) = -12412
double dT;

// Sleep - Logo
static const uint8_t PROGMEM bitmap[] = {
B00000000,B00000000,B01111110,B00000000,B00000000,
B00000000,B00000111,B11110001,B11100000,B00000000,
B00000000,B00011000,B10001101,B00011000,B00000000,
B00000000,B01100001,B00000111,B00001110,B00000000,
B00000000,B10000001,B11111111,B11111011,B00000000,
B00000011,B00000001,B00011010,B11000100,B11000000,
B00000110,B00000001,B00000010,B00000100,B01100000,
B00000100,B00000001,B00000010,B00000100,B00110000,
B00001000,B00000001,B00000111,B00001000,B00010000,
B00010000,B00000000,B10001100,B10011000,B00001000,
B00010000,B00000000,B11111110,B11100000,B00111000,
B00100000,B00000000,B00001111,B11000000,B00000100,
B00100000,B00000000,B00001111,B10000000,B11100100,
B01000000,B01111111,B10000111,B00000111,B10000100,
B01000000,B00000000,B00000010,B00000000,B00000100,
B01000000,B00000000,B00000010,B00000000,B00000010,
B01000100,B00001111,B11000010,B00000111,B11100010,
B01001000,B11111000,B00000010,B00000000,B00110010,
B01001000,B00000000,B00000010,B00000000,B00000010,
B01001000,B00000000,B01000010,B00000100,B00000010,
B01001000,B00000111,B11000010,B00000111,B11000010,
B01001000,B00111000,B00001111,B10000000,B01110010,
B01001000,B01000000,B00111000,B01110000,B00000010,
B00101000,B00000001,B11000000,B00001100,B00000100,
B00101100,B00000110,B00000000,B00000011,B00000100,
B00011100,B00011000,B00000000,B00000000,B10000100,
B00010100,B00100000,B00000000,B00000000,B01001000,
B00001110,B01000000,B00000000,B00000000,B00010000,
B00000110,B00000000,B00000000,B00000000,B00110000,
B00000011,B00000000,B00000000,B00000000,B00100000,
B00000001,B10000000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000};


void balken(int x, int y, int maxVal, int minVal, int val) {
  if (val <= minVal) {
    val = 0;
  } else if(val >= maxVal) {
    val = maxVal;
  } else {
    val = maxLength * (float)(val-minVal)/((float)(maxVal-minVal));
  }
  display.drawRect(x,   y,                 8, maxLength+2, WHITE);
  display.fillRect(x+2, y+1+maxLength-val, 4, val, WHITE);    
}

int readVcc() {
  int mv;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
  mv = ADCL; 
  mv |= ADCH<<8; 
  mv = 1126400L / mv;
  return mv;
}

void tick() {
  seconds++;
  tsec++;
  onsec++;
  if (seconds == 60) {
    minutes++; seconds = 0;
  }
  if (minutes == 60) {
    EEPROM.put(eeAddress, steps);
    hours++; minutes = 0;
  }
}

void setup() {
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(STEPLED, OUTPUT);
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  //            .-------- sleep
  //            I .------ no temp
  //Wire.write(0b0101000);
  Wire.write(0);
  Wire.endTransmission(true);
  
  Serial.begin(9600);
  // initialize with the I2C addr 0x3C (for the 128x32)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.startscrollleft(0x00, 0x0F);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("John D.");
  display.setTextSize(1);
  display.println("90210 B-Hills");
  display.print("Fashionstreet 42");
  display.display();
  EEPROM.get(eeAddress, steps);
  delay(3000);
  display.stopscroll();
  MsTimer2::set(1000, tick);
  MsTimer2::start();
}

void loop() {
  delay(150);

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);  // request a total of 14 registers
  
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  dT = ( (double) Tmp + 12412.0) / 340.0;  stoss = ((long)AcX)*((long)AcX) + ((long)AcY)*((long)AcY) + ((long)AcZ)*((long)AcZ);

  if (treshold<=15) {
    barrier = treshold*40000000l + 260000000l;
    if (stoss > barrier) {
      steps++;
      if (steps%1000 == 0) {
        EEPROM.put(eeAddress, steps);
      }
      digitalWrite(STEPLED, HIGH);
      delay(50);
      digitalWrite(STEPLED, LOW);
    }
  }

  if (digitalRead(BUTTON1) == LOW) {
    delay(300);
    onsec=0;
    display.ssd1306_command(SSD1306_DISPLAYON);
    if (digitalRead(BUTTON1) == LOW) {
      delay(800);  
      if (digitalRead(BUTTON1) == LOW) {
        treshold++;
        if (treshold>=17) treshold=1;
        showTreshold = true;
        tsec = 0;
      }
    }
  }

  if (onsec == DELAYOFF || onsec == DELAYOFF+1) {
    showTreshold = false;
    display.clearDisplay();
    display.drawBitmap(25, 0, bitmap, 40, 32, WHITE);
    display.setCursor(80,5);
    display.setTextSize(1);
    display.print("tzzz..");
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    if (treshold == 16) {
      treshold=1;
      steps=0;
    }
    EEPROM.put(eeAddress, steps);
  }
  
  if (onsec < DELAYOFF) {  
    vcc = readVcc();
    display.setCursor(0,0);
    display.clearDisplay();
    if (hours<10) display.print('0');
    display.print(hours);
    display.print(':');
    if (minutes<10) display.print('0');
    display.print(minutes);
    display.print(':');
    if (seconds<10) display.print('0');
    display.println(seconds);
    display.setTextSize(1);
    
    if (showTreshold) {
      if (treshold == 16) {
        display.print("Please wait ");
        display.print(DELAYOFF - onsec);
        display.println("s to");
        display.print  ("set Steps to ZERO !");
        tsec = 0;
      } else {
        display.setCursor(0,20);
        display.print("Level ");
        display.setTextSize(2);
        display.setCursor(42,17);
        display.print(treshold);
      }
    } else {
      display.println("Temp.");
      display.print(dT);
      display.setCursor(42,17);
      display.setTextSize(2);
      display.print(steps);  
    }
  
    if (tsec > DELAYTRESHOLD) {
      showTreshold = false;
    }
    if (
      (vcc > MIN_POWER) ||
      (vcc <= MIN_POWER && tsec%2 == 0)
    ) {
      display.drawLine(121,0,124,0, WHITE);
      balken(119,1,MAX_POWER,MIN_POWER,vcc);
    }
    display.display();
  }
}

