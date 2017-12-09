#include <MsTimer2.h>
#include "myNumbers.h"

#include <Wire.h> //I2C Arduino Library
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

// HARDWARE I2C: A4 -> SDA, A5 -> SCL
// --------- HARDWARE SPI OLED: 11 -> MOSI/DIN, 13 -> SCK

#define PIN_RESET  4 // dummy

#define BUTTON1    A3
#define BUTTON2    A2

#define LED_BLUE   11
#define LED_GREEN  10
#define LED_RED    9
#define LED_OFF    LOW
#define LED_ON     HIGH

#define ON_TRESHOLD    300
#define ON_SEC         4
#define MEMOSTR_LIMIT  25

#define SERIAL_SPEED  9600

byte hours   = 18;
byte minutes = 59;
byte seconds = 50;
byte onsec   = 0;
byte tick    = 0;
byte loopi   = 0;

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ;
unsigned long stoss1;
unsigned long stoss2;

char memoStr[MEMOSTR_LIMIT] = {'\0'};
byte memoStrPos = 0;

int  vcc = 3100;
long steps = 0;
long goal  = 10000;
long threshold = 90;

static const uint8_t PROGMEM stepicon[] = {
B01100000,B00000000,
B11110000,B00000000,
B11110000,B00000000,
B11110011,B00000000,
B11100111,B10000000,
B01100111,B10000000,
B00000111,B10000000,
B11100011,B10000000,
B11110011,B00000000,
B01100000,B00000000,
B00000011,B10000000,
B00000111,B10000000,
B00000011,B00000000
};

Adafruit_SSD1306 * oled = new Adafruit_SSD1306(PIN_RESET);

void readVcc() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
  vcc = ADCL; 
  vcc |= ADCH<<8; 
  vcc = 1126400L / vcc;
}

inline byte tob(char c) { return c - '0';}

void hideA(byte x, short y, int h) {
  oled->fillRect(x, y+h, 16, 32-h, BLACK);
}

void hideB(byte x, short y, int h) {
  oled->fillRect(x, y, 16, 16+h, BLACK);
}

void myFont(byte x, short y, byte b, bool change) {
  int i = 0;
  int j = 18;
  int h = 0;

  if (change == true && tick > 19) {
    if(tick < 33) {
      h = tick - 20;
      
      if (b == 0) {
        oled->drawBitmap(x, y, zz1, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz0, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz0b, 16, 16, WHITE);
      } else if (b == 1) {
        oled->drawBitmap(x, y, zz2, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz1, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz1b, 16, 16, WHITE);
      } else if (b == 2) {
        oled->drawBitmap(x, y, zz3, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz2, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz2b, 16, 16, WHITE);
      } else if (b == 3) {
        oled->drawBitmap(x, y, zz4, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz3, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz3b, 16, 16, WHITE);
      } else if (b == 4) {
        oled->drawBitmap(x, y, zz5, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz4, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz4b, 16, 16, WHITE);
      } else if (b == 5) {
        oled->drawBitmap(x, y, zz6, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz5, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz5b, 16, 16, WHITE);
      } else if (b == 6) {
        oled->drawBitmap(x, y, zz7, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz6, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz6b, 16, 16, WHITE);
      } else if (b == 7) {
        oled->drawBitmap(x, y, zz8, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz7, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz7b, 16, 16, WHITE);
      } else if (b == 8) {
        oled->drawBitmap(x, y, zz9, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz8, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz8b, 16, 16, WHITE);
      } else if (b == 9) {
        oled->drawBitmap(x, y, zz0, 16, 16, WHITE);
        hideA(x,y,h);
//        oled->drawBitmap(x, y+h, zz9, 16, 16, WHITE);
//        hideA(x,y,16);
        oled->drawBitmap(x, y+j, zz9b, 16, 16, WHITE);
      }
      
      oled->drawLine(x-1,    y+h-1, x+17, y+h-1,  WHITE);
      oled->drawLine(x-1,    y+h-1, x+2,  y+16, WHITE);
      oled->drawLine(x+17,   y+h-1, x+14, y+16, WHITE);

    } else {
      
      h = tick - 33;
      
      if (b == 0) {
        oled->drawBitmap(x, y+j, zz0b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz1b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz1, 16, 16, WHITE);
      } else if (b == 1) {
        oled->drawBitmap(x, y+j, zz1b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz2b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz2, 16, 16, WHITE);
      } else if (b == 2) {
        oled->drawBitmap(x, y+j, zz2b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz3b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz3, 16, 16, WHITE);
      } else if (b == 3) {
        oled->drawBitmap(x, y+j, zz3b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz4b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz4, 16, 16, WHITE);
      } else if (b == 4) {
        oled->drawBitmap(x, y+j, zz4b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz5b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz5, 16, 16, WHITE);
      } else if (b == 5) {
        oled->drawBitmap(x, y+j, zz5b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz6b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz6, 16, 16, WHITE);
      } else if (b == 6) {
        oled->drawBitmap(x, y+j, zz6b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz7b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz7, 16, 16, WHITE);
      } else if (b == 7) {
        oled->drawBitmap(x, y+j, zz7b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz8b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz8, 16, 16, WHITE);
      } else if (b == 8) {
        oled->drawBitmap(x, y+j, zz8b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz9b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz9, 16, 16, WHITE);
      } else if (b == 9) {
        oled->drawBitmap(x, y+j, zz9b, 16, 16, WHITE);
        hideB(x,y,h);
        oled->drawBitmap(x, y+h, zz0b, 16, 16, WHITE);
        oled->fillRect(x, y, 16, 16, BLACK);
        oled->drawBitmap(x, y, zz0, 16, 16, WHITE);
      }      

      oled->drawLine(x-1,    y+17+h, x+17, y+17+h,  WHITE);
      oled->drawLine(x+2,    y+17,   x-1,  y+17+h,  WHITE);
      oled->drawLine(x+14,   y+17,   x+16, y+17+h,  WHITE);      
    }

    
  } else {
  
    if (b == 0) {
      oled->drawBitmap(x, y+i, zz0,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz0b, 16, 16, WHITE);
    } else if (b == 1) {
      oled->drawBitmap(x, y+i, zz1,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz1b, 16, 16, WHITE);
    } else if (b == 2) {
      oled->drawBitmap(x, y+i, zz2,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz2b, 16, 16, WHITE);
    } else if (b == 3) {
      oled->drawBitmap(x, y+i, zz3,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz3b, 16, 16, WHITE);
    } else if (b == 4) {
      oled->drawBitmap(x, y+i, zz4,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz4b, 16, 16, WHITE);
    } else if (b == 5) {
      oled->drawBitmap(x, y+i, zz5,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz5b, 16, 16, WHITE);
    } else if (b == 6) {
      oled->drawBitmap(x, y+i, zz6,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz6b, 16, 16, WHITE);
    } else if (b == 7) {
      oled->drawBitmap(x, y+i, zz7,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz7b, 16, 16, WHITE);
    } else if (b == 8) {
      oled->drawBitmap(x, y+i, zz8,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz8b, 16, 16, WHITE);
    } else if (b == 9) {
      oled->drawBitmap(x, y+i, zz9,  16, 16, WHITE);
      oled->drawBitmap(x, y+j, zz9b, 16, 16, WHITE);
    }
  }

}

inline void flipClock() {
  bool flap = false;
  int t = hours/10;
  if ((hours+1)%10 == 0 && (minutes+1)%60 == 0 && (seconds+1)%60 == 0) flap=true;
  myFont(4, 3, t, flap);

  flap = false;
  t = hours - t*10;
  if ((minutes+1)%60 == 0 && (seconds+1)%60 == 0) flap=true;
  myFont(23, 3, t, flap);
  
  flap = false;
  t = minutes/10;
  if ((minutes+1)%10 == 0 && (seconds+1)%60 == 0) flap=true;
  myFont(48, 3, t, flap);
  
  flap = false;
  t = minutes - t*10;
  if ((seconds+1)%60 == 0) flap=true;
  myFont(66, 3, t, flap);

  flap = false;
  t = seconds/10;
  if ((seconds+1)%10 == 0) flap=true;
  myFont(92, 3, t, flap);
  
  t = seconds - t*10;
  myFont(110, 3, t, true);
}

inline void ticking() {
  tick = (tick+1) % 40;
  
  if (tick == 0) {
    seconds++;
    if (onsec > 0) onsec++;
  }
  
  if (seconds > 59) {
    minutes += seconds / 60;
    seconds  = seconds % 60;
  }
  if (minutes > 59) {
    hours  += minutes / 60;
    minutes = minutes % 60;
  }
  if (hours > 23) {
    hours = hours % 24;
  }
}

void serialEvent() {
  while (Serial.available()) {
    if (memoStrPos >= MEMOSTR_LIMIT) memoStrPos = MEMOSTR_LIMIT;
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      memoStr[MEMOSTR_LIMIT-1] = '\n';
      continue;
    }
    if (inChar == 'A') {
      digitalWrite(LED_RED, HIGH);
    }
    if (inChar == 'B') {
      digitalWrite(LED_GREEN, HIGH);
    }
    if (inChar == 'C') {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, HIGH);
    }
    digitalWrite(LED_BLUE, HIGH);
    memoStr[memoStrPos] = inChar;
    memoStrPos++;    
    if (memoStrPos >= MEMOSTR_LIMIT) {
      // ignore the other chars
      memoStrPos = MEMOSTR_LIMIT-1;
      memoStr[memoStrPos] = '\0';
    }
  }
}

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
  Serial.println("AT+NAME=I2C Step Watch");
  delay(800);
  
  oled->begin();
  oled->clearDisplay();
  oled->display();
  oled->setTextColor(WHITE);
  oled->setTextSize(2);
  
  MsTimer2::set(25, ticking); // 50ms period -> 1sec = 40 parts/ticks
  MsTimer2::start();
}

void loop() {
  loopi = (loopi+1)%20;
  delay(7); // "refresh rate"

  if (tick==0 && (seconds%10==0)) {
    readVcc();
  }
  
  if (digitalRead(BUTTON1) == LOW) {
    delay(800);
    onsec=1;
    if (digitalRead(BUTTON1) == LOW) {
      hours++;
      if (hours==24) steps=0;
      seconds=0;
      tick=0;
    }
  }
  if (digitalRead(BUTTON2) == LOW) {
    delay(800);
    onsec=1;
    if (digitalRead(BUTTON2) == LOW) {
      minutes++;
      seconds=0;
      tick=0;
    }
  }

  oled->clearDisplay();
  oled->drawBitmap(2, 50, stepicon,  16, 13, WHITE);
  flipClock();
  oled->setCursor(20, 50);
  oled->print(steps);

  // goal bar
  oled->drawLine(4, 44, 123, 44, WHITE);
  if (steps >= goal) {
    oled->fillRect(4, 43, 120,  3, WHITE);    
  } else {
    oled->fillRect(4, 43, (int) (120.0*((double)steps/(double)goal)),  3, WHITE);    
  }

  // battery
  oled->drawRect(106, 54, 19, 10, WHITE);
  oled->fillRect(125, 57,  2,  4, WHITE);
  if (vcc > 3130) {
    oled->fillRect(108, 56,  3,  6, WHITE);
  }
  if (vcc > 3180) {
    oled->fillRect(112, 56,  3,  6, WHITE);
  }
  if (vcc > 3230) {
    oled->fillRect(116, 56,  3,  6, WHITE);
  }
  if (vcc > 3280) {
    oled->fillRect(120, 56,  3,  6, WHITE);
  }

  oled->display();

  if (loopi==0) {
    digitalWrite(LED_RED,   LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE,  LOW);
    
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU,14,true);  // request a total of 14 registers

    //unused
    AcX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    AcX=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    AcX=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    AcX=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    
    AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
    AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    stoss1 = (((unsigned long)AcX)*((unsigned long)AcX) + ((unsigned long)AcY)*((unsigned long)AcY) + ((unsigned long)AcZ)*((unsigned long)AcZ))/1000000l; 

    if (stoss1 > stoss2) {
      if ((stoss1-stoss2) > threshold) steps++;
      if ((stoss1-stoss2) > ON_TRESHOLD) onsec=1;      
    } else {
      if ((stoss2-stoss1) > threshold) steps++;
      if ((stoss2-stoss1) > ON_TRESHOLD) onsec=1;      
    }
    
    stoss2 = stoss1;
  }

  if (onsec==1) {
    oled->ssd1306_command(SSD1306_DISPLAYON);
  }
  if (onsec > ON_SEC) {
    onsec = 0;
    oled->ssd1306_command(SSD1306_DISPLAYOFF);
  }

  if ((memoStr[0]=='0' || memoStr[0]=='1' || memoStr[0]=='2') && memoStr[MEMOSTR_LIMIT-1] == '\n') {
    onsec = 1;
    hours = tob(memoStr[0])*10 + tob(memoStr[1]);
    minutes = tob(memoStr[2])*10 + tob(memoStr[3]);
    seconds = tob(memoStr[4])*10 + tob(memoStr[5]);

    goal  = tob(memoStr[12]);
    goal += 100000l*tob(memoStr[7]);
    goal += 10000l*tob(memoStr[8]);
    goal += 1000l*tob(memoStr[9]);
    goal += 100l*tob(memoStr[10]);
    goal += 10l*tob(memoStr[11]);

    threshold  = tob(memoStr[16]);
    threshold += 100*tob(memoStr[14]);
    threshold += 10*tob(memoStr[15]);

    memoStr[MEMOSTR_LIMIT-1] = '\0';
    memoStrPos=0;
  }
}

