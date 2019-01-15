#include <LiquidCrystal.h>

//----------------------------------------------------------------------------Pin set
byte ThermistorPin = 0;
byte HotterPin = 8;
byte MotorPin = 9;
byte VRPin = A1;
byte SpindlePin = A2;

//----------------------------------------------------------------------------Motor control
const unsigned short minSpindleThreshold = 300;
const unsigned short maxSpindleThreshold = 1023;
short vrRead = 0;
byte speedPercentage = 0;

//----------------------------------------------------------------------------Counter for hot end
const unsigned long interval = 1000;
unsigned long previousMillis = 0;
bool State = LOW;

//----------------------------------------------------------------------------Temperature calculation
int Vo;
float R1 = 10000;
float logR2, R2, Tf, Tc;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
unsigned const short readyTemp = 180; //190 Default

//----------------------------------------------------------------------------LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
byte tempChar[] = {
  B00100,
  B01010,
  B01010,
  B01010,
  B01010,
  B10001,
  B10001,
  B01110
};
byte celciusChar[] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000
};
byte speedChar[] = {
  B00100,
  B01010,
  B10001,
  B01110,
  B01110,
  B01110,
  B10101,
  B10101
};
//----------------------------------------------------------------------------Counter for LCD
unsigned long currentMillisLCD = 0;

//----------------------------------------------------------------------------Wait function
long wait(long millisec) {
  return millisec;
}

void setup() {
  Serial.begin(9600);

  //----------------------------------------------------------------------------LCD Setup
  lcd.begin(16, 2);
  lcd.clear();
  lcd.createChar(0, tempChar);
  lcd.createChar(1, celciusChar);
  lcd.createChar(2, speedChar);

  //----------------------------------------------------------------------------I/O Setup
  pinMode(SpindlePin, INPUT);
  pinMode(VRPin, INPUT);
  pinMode(HotterPin, OUTPUT);
  pinMode(MotorPin, OUTPUT);
}

void loop() {

  //----------------------------------------------------------------------------Temperature calculation
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  Tf = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tf = Tf - 273.15;
  Tf = (Tf * 9.0) / 5.0 + 32.0;
  Tc = (Tf - 32.0) / 1.8;

  if (Tc >= readyTemp) {
    digitalWrite(HotterPin, LOW);
    lcd.setCursor(10, 0);
    lcd.print("READY!");
  } else {

    //----------------------------------------------------------------------------Counter for hot end
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if (State == LOW) {
        State = HIGH;
      } else {
        State = LOW;
      }
      digitalWrite(HotterPin, State);
    }
  }

  //----------------------------------------------------------------------------Motor control
  vrRead = analogRead(VRPin);
  speedPercentage = map(vrRead, 0, 1023, 0, 100);
  vrRead = map(vrRead, 0, 1023, 0, 255);

  lcd.setCursor(0, 1);
  lcd.print(char(2));
  lcd.setCursor(2, 1);
  lcd.print(speedPercentage);
  lcd.print("%");
  if (analogRead(SpindlePin) >= minSpindleThreshold && analogRead(SpindlePin) < maxSpindleThreshold) {
    analogWrite(MotorPin, vrRead);
  }
  else if (analogRead(SpindlePin) < 10) {
    analogWrite(MotorPin, 0);
  }
  else {
    lcd.setCursor(10, 1);
    lcd.print("ERROR!");
  }

  //----------------------------------------------------------------------------LCD Print
  lcd.setCursor(0, 0);
  lcd.print(char(0));
  lcd.setCursor(2, 0);
  lcd.print(Tc);
  lcd.print(char(1));

  currentMillisLCD = millis();
  while (millis() < currentMillisLCD + wait(1000)) {
  }
  lcd.clear();
}
