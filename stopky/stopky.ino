// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

bool leftRun, rightRun;

//RTC - Start

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

DateTime beforeTime, finishedLeft, finishedRight;
long beforeMillis, finishedLeftMillis, finishedRightMillis;

//RTC - End

//LCD - Start

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

//LCD - End

//Button - Start

int exLeftButton = 51;
int exRightButton = 48;
int exStartButton = 30;
int exResetButton = 44;

//Button - End

//external display - Start
//Pin connected to ST_CP - 12 of 74HC595
int latchPin = 37;

//Pin connected to SH_CP - 11 of 74HC595
int clockPin = 39;

//Pin connected to DS - 14 of 74HC595
int dataPin = 38;

// Number of digits attached
int const numberOfRegisters = 6;

// Numbers mapped on our seven segment display
int zero = 63;
int one = 6;
int two = 91;
int three = 79;
int four = 102;
int five = 109;
int six = 125;
int seven = 7;
int eight = 127;
int nine = 103;
int empty = 0b00000000;
int pi = 115;
int el = 56;
int numbers[] = {zero,one,two,three,four,five,six,seven,eight,nine};

// Internal Vars
int dataBuffer[numberOfRegisters];

//external display END


void setup() {    
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Loading");
    
    // Nastartuj seriovou linku na rychlosti 9600 bps
    Serial.begin(9600);

    delay(1000); // wait for console opening

    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, lets set the time!");
        // following line sets the RTC to the date & time this sketch was compiled
        //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
         rtc.adjust(DateTime(2017, 5, 3, 0, 0, 0));
    }
    
    resetTimer();
  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Levy");
    lcd.setCursor(0, 1);
    lcd.print("Pravy");

    pinMode(exLeftButton, INPUT_PULLUP);
    pinMode(exRightButton, INPUT_PULLUP);
    pinMode(exStartButton, INPUT_PULLUP);
    pinMode(exResetButton, INPUT_PULLUP);

    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);

}

void loop() {   
      
    SolveLCDButton(read_LCD_buttons());    
    DisplayOnLCD(finishedLeft, finishedLeftMillis, 0);
    DisplayOnLCD(finishedRight, finishedRightMillis, 1);

    printInfoToDisplay();

    if (digitalRead(exLeftButton) == LOW) stopLeft();
   
    if (digitalRead(exRightButton) == LOW) stopRight();

    if (digitalRead(exStartButton) == LOW && leftRun == false && rightRun == false) startTimer();

    if(digitalRead(exResetButton) == LOW) resetTimer();
    
    if(leftRun || rightRun)
    {
        DateTime now = rtc.now();    
        if((now - beforeTime).totalseconds() != 0)
        {      
          beforeTime = now;  
          beforeMillis = millis();  
        }
        
        long actualMillis = millis() - beforeMillis;

        if(leftRun)
        {
          finishedLeft = now;
          finishedLeftMillis = actualMillis;
        }
        if(rightRun)
        {
          finishedRight = now;
          finishedRightMillis = actualMillis;
        }
    }
}

void startTimer() {    
    resetTimer();
    leftRun = true;
    rightRun = true;
 }

void stopLeft(){
    if(leftRun)
    {
      finishedLeft = rtc.now();
      leftRun = false;
    }
 }

 void stopRight(){
    if(rightRun)
    {
      finishedRight = rtc.now();
      rightRun = false;
    }
      
 }

  void resetTimer(){
    leftRun = false;
    rightRun = false;
    rtc.adjust(DateTime(2017, 5, 3, 0, 0, 0));  
    finishedLeft = rtc.now();
    finishedRight = rtc.now();
    finishedLeftMillis = 0;
    finishedRightMillis = 0;     
 }

void DisplayOnLCD( DateTime t, unsigned long millisec, unsigned int lcdRow ){  
     
    lcd.setCursor(7, lcdRow);     
    if(t.minute() < 10)lcd.print("0");
    lcd.print(t.minute(), DEC); 
    lcd.print(":");
    if(t.second() < 10)lcd.print("0");
    lcd.print(t.second(), DEC); 
    lcd.print(":");
    if(millisec < 100)lcd.print("0");
    if(millisec < 10)lcd.print("0");
    lcd.print(millisec);

   //if(lcdRow == 0)
   // {         
   //     if(t.minute() < 10)Serial.print("0");
   //     Serial.print(t.minute(), DEC); 
   //     Serial.print(":");
   //     if(t.second() < 10)Serial.print("0");
   //     Serial.print(t.second(), DEC); 
   //     Serial.print(":");
   //     if(millisec < 100)Serial.print("0");
   //     if(millisec < 10)Serial.print("0");
   //     Serial.println(millisec);
   //}
 }

int read_LCD_buttons(){               // read the buttons
    int adc_key_in = analogRead(0);       // read the value from the sensor

     if (adc_key_in > 1000) return btnNONE;
     if (adc_key_in < 50)   return btnRIGHT;  
     if (adc_key_in < 195)  return btnUP; 
     if (adc_key_in < 380)  return btnDOWN; 
     if (adc_key_in < 555)  return btnLEFT; 
     if (adc_key_in < 790)  return btnSELECT;

     return btnNONE;                // when all others fail, return this.
     }

void SolveLCDButton(int btn){
  switch (btn){
       case btnRIGHT:{             
            stopRight();
            break;
       }
       case btnLEFT:{
            stopLeft(); 
             break;
       }    
       case btnSELECT:{
            startTimer();        
             break;
       }
       case btnNONE:{               
             break;
       }
   }
      
}

bool showLeft = true;
bool showDisplayTime;

void printInfoToDisplay()
{
    if(leftRun || rightRun)// if time is still running
      showLeft = leftRun && !rightRun; // if left is running and right is not. Show first left time
    
    if(leftRun)    
      addToBuffer(empty,finishedLeft.minute(),finishedLeft.second(),finishedLeftMillis);    
    else if(rightRun)
      addToBuffer(empty,finishedRight.minute(),finishedRight.second(),finishedRightMillis); 
    else if(finishedLeft.secondstime() == DateTime(2017, 5, 3, 0, 0, 0).secondstime()) // is probably zero time
      addToBuffer(empty,finishedLeft.minute(),finishedLeft.second(),finishedLeftMillis);    
    else
    {      
      if(showLeft)
        addToBuffer(el,finishedLeft.minute(),finishedLeft.second(),finishedLeftMillis);
      else
        addToBuffer(pi,finishedRight.minute(),finishedRight.second(),finishedRightMillis);

      if(millis() - showDisplayTime > 3000)
      {
        showDisplayTime = millis();
        showLeft = !showLeft;
      }
    }
}

void addToBuffer(int side, int minits, int sec, int milsec){
    
  // clear buffer
  memset(dataBuffer, 0, sizeof(dataBuffer));

  if(milsec != 0)
    milsec = milsec /10;

  int digit = (10000 * minits) + (100 * sec) + milsec;

  int c = 0;
  while( digit > 0 ){
    int b = numbers[digit % 10]; // modulus 10 of our input
    dataBuffer[c] = b;
    digit /= 10;
    c++;
  }
    
  dataBuffer[numberOfRegisters - 1] = side;
  
  writeBuffer();
}

void writeBuffer(){
  digitalWrite(latchPin, LOW); 
  for (int a = sizeof(dataBuffer) - 1; a >= 0  ; a--) {
    if(dataBuffer[a] == 0) dataBuffer[a] = numbers[0];      
    shiftOut(dataPin, clockPin, MSBFIRST, dataBuffer[a]);
  }  
  digitalWrite(latchPin, HIGH);
}

