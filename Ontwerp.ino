#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
RTC_DS1307 rtc;

File dataFile;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int SDcardpin = 10;
const byte interruptPin = 2;
const int CurrentSensor = A0;
const int VoltageSensor = A1;

float R1 = 14930;
float R2 = 982;
const int AmountOfCogs = 40;

const int driverPUL = 3;
const int driverDIR = 5;
bool SetDIR = LOW;

const int BrakeButton = 6;
const int MaxSteps = 550;
const int Pulse = 500;
int CurrentStep = MaxSteps;

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
   pinMode(BrakeButton, INPUT);  
   pinMode (driverPUL, OUTPUT);
   pinMode (driverDIR, OUTPUT);
   
   attachInterrupt(digitalPinToInterrupt(interruptPin),CountTheCogs, FALLING);
   
   lcd.init();
   lcd.backlight();
     
   PrintHeadersToSD(); 
   
   if(BrakeButton == HIGH){
    SetDIR = HIGH;
   }
  }

void loop() {
 now = rtc.now();   //start the realtime clock

 BrakeLogic();      //enable the braking logic. When to brake when not to.
 
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
    lcd.print(RotationSpeed);
    lcd.setCursor(6,1);
    lcd.print(now.day(), DEC);
    lcd.print('/'); 
    lcd.print(now.month(), DEC);
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
      dataFile.print(RotationSpeed);
      dataFile.print(",");
      dataFile.print(Vin);
      dataFile.print(",");
      dataFile.println(Current);
      dataFile.close();
    }
  }

void BrakeLogic()  //when to brake
  {
    if((digitalRead(BrakeButton) == HIGH) || (RotationSpeed >= 2000)){                           //if BrakeButton is pressed or speed is exessive: start braking
      Brake_ON();
    }
    
    if((digitalRead(BrakeButton) == LOW) && (RotationSpeed < 2000)){                            //if Brakebutton is not pressed, and  speed is not exessive: stop braking
      Brake_OFF();
    }
  }

void Brake_ON(){
  while(CurrentStep < MaxSteps){    //Turn on the brake
    digitalWrite(driverDIR,SetDIR);
    digitalWrite(driverPUL,HIGH);
    delayMicroseconds(Pulse);
    digitalWrite(driverPUL,LOW);
    delayMicroseconds(Pulse);
    CurrentStep++;
    }
}

void Brake_OFF(){                   //Turn off the brake
  while(CurrentStep > 0){
    digitalWrite(driverDIR,!SetDIR);
    digitalWrite(driverPUL,HIGH);
    delayMicroseconds(Pulse);
    digitalWrite(driverPUL,LOW);
    delayMicroseconds(Pulse);
    CurrentStep--;
  }
}
