#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Arduino.h>
#include "BasicStepperDriver.h"
RTC_DS1307 rtc;

File dataFile;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int SDcardpin = 10;
const byte interruptPin = 2;
int CurrentSensor = A0;
int VoltageSensor = A1;

float R1 = 14930;
float R2 = 982;
int AmountOfCogs = 40;

#define MOTOR_STEPS 200
#define RPM 120
#define MICROSTEPS 1
#define DIR 8
#define STEP 9
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);


double CountedCogs = 0;
int RotationSpeed = 0;
double Current = 0;
float Vout = 0.0;
float Vin = 0.0;
double Seconds;

DateTime now;
String FileName;

void setup() {
   now = rtc.now();
   FileName = String(now.month())+ String(now.day())+String(now.hour())+".csv";
   
   if (! rtc.begin()) {
    while (1);
  }
  
   if (!rtc.isrunning()) {
    // rtc.adjust(DateTime(2019, 10, 22, 19, 15, 9));
  }
  
  if (!SD.begin(SDcardpin)) 
  {
    while (1);
  }
  
   pinMode(CurrentSensor, INPUT);   
   pinMode(interruptPin, INPUT_PULLUP);
   
   attachInterrupt(digitalPinToInterrupt(interruptPin),CountTheCogs, FALLING);
   
   lcd.init();
   lcd.backlight();
     
   PrintHeadersToSD();
   
   stepper.begin(RPM, MICROSTEPS);
}

void loop() {
 now = rtc.now();
 Current = .0264 * analogRead(CurrentSensor) -13.52;

 if( Current < 0 )
   {
    Current *= -1;
   }
   
  Vout = (analogRead(VoltageSensor) * 5) / 1024.0;
  Vin = Vout / (R2/(R1+R2));

  RotationSpeed = CountedCogs / AmountOfCogs * 60;

  PrintDataToLCD();
  PrintDataToSD();
  Lift_Brake();
  
  if(now.second()!= Seconds){
    CountedCogs = 0;
    Seconds = now.second();
  }
  
  delay(500);
}

void CountTheCogs()
  {
    CountedCogs++;  
  }

void PrintHeadersToSD()
  {   
    if(!SD.exists(FileName))
    {
      dataFile = SD.open(FileName, FILE_WRITE);
      if(dataFile)
       {
         dataFile.print("Timestamp (YYYYMMDDHHSS)");
         dataFile.print(",");
         dataFile.print("Toerental [RPM]");
         dataFile.print(",");
         dataFile.print("Vin [V]");
         dataFile.print(",");
         dataFile.println("Stroom [A]");
         dataFile.close();
       }
    }
  }
  
void PrintDataToLCD()
  {
    String dataString = "";
    lcd.clear();
    lcd.print("I:");
    lcd.print(Current);
    lcd.setCursor(9,0);
    lcd.print("V:");  
    lcd.print(Vin);
    lcd.setCursor(0,1);
    lcd.print("R:");  
    lcd.print(RPM);
    lcd.setCursor(6,1);
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.day(), DEC); 
    lcd.print(" ");
    lcd.print(now.hour());
    lcd.print(now.minute());
  }
  
void PrintDataToSD()
  {
    dataFile = SD.open(FileName, FILE_WRITE);
    if (dataFile) 
    {
      dataFile.print(now.year(), DEC);
      dataFile.print(now.month(), DEC);
      dataFile.print(now.day(), DEC);
      dataFile.print(now.hour(), DEC);
      dataFile.print(now.minute(), DEC);
      dataFile.print(now.second(), DEC);
      dataFile.print(",");
      dataFile.print(RPM);
      dataFile.print(",");
      dataFile.print(Vin);
      dataFile.print(",");
      dataFile.println(Current);
      dataFile.close();
    }
  }

void Lift_Brake()
  {
   stepper.rotate(70);
   delay(5000);
   stepper.rotate(-70);

  }
