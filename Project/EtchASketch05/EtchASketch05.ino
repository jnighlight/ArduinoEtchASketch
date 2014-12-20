/*
  The circuit:
 * 5V to Arduino 5V pin
 * GND to Arduino GND pin
 * CLK to Analog #5
 * DAT to Analog #4
// pin 6 - Serial clock out (SCLK)
// pin 5 - Serial data out (DIN)
// pin 4 - Data/Command select (D/C)
// pin 3 - LCD chip select (CS)
// pin 2 - LCD reset (RST)
*/

// include the library code:
#include "Wire.h"
#include "LiquidCrystal.h"
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "Point.h"

// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd(0);
//Create Digital Display
Adafruit_PCD8544 display = Adafruit_PCD8544(6, 5, 4, 3, 2);


int button1 = 8;
int button2 = 9;
int LR = A0;
int UD = A1;

int xRes = 0;
int yRes = 0;

int drawX = 10;
int drawY = 10;

int xTime = 0;
int yTime = 0;

boolean changed = false;

int curTime;
int oldTime;
int timePassed;

int curSize = 0;
Point dots[150];

void setup()
{
  Serial.begin(9600);
  
  lcd.setBacklight(HIGH);
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  // Print a message to the LCD. 
  
  display.begin();
  // init done
  
  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);
  display.display(); // show splashscreen
  delay(1000);
  display.clearDisplay();
  display.display();
  
  
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(UD, INPUT);
  pinMode(LR, INPUT);
  display.drawPixel(drawX, drawY, BLACK);
  display.display();
  
  lcd.setCursor(0,0);
  lcd.print("drawX:");
  lcd.setCursor(0,1);
  lcd.print("drawY:");
}

void loop()
{
  oldTime = curTime;
  curTime = millis();
  timePassed = curTime - oldTime;
  xRes = analogRead(LR) - 511; //For easier understanding of the way the joystick is pointing
  yRes = analogRead(UD) - 511; //instead of 0 being left, 1023 being right, -511 is left, 511 is right
  timeUpdate();
  checkIncrements();
  
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(7, 0);
  lcd.print(drawX < 10? "0" + (String)drawX : drawX);
  lcd.setCursor(7,1);
  lcd.print(drawY < 10? "0" + (String)drawY : drawY);
  lcd.setCursor(13,1);
  lcd.print(curSize > 99? curSize : curSize > 9 ? "0" + (String)curSize : "00" + (String)curSize);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  
  if(changed)
  {
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(7, 0);
    lcd.print(drawX < 10? "0" + (String)drawX : drawX);
    lcd.setCursor(7,1);
    lcd.print(drawY < 10? "0" + (String)drawY : drawY);
    lcd.setCursor(13,1);
    lcd.print(curSize > 99? curSize : curSize > 9 ? "0" + (String)curSize : "00" + (String)curSize);
    // print the number of seconds since reset:
    //lcd.print(millis()/1000);
    
    display.clearDisplay();
    display.drawPixel(drawX, drawY, BLACK);
    for(int i=0; i < curSize; i++)
    {
      display.drawPixel(dots[i].getXPoint(), dots[i].getYPoint(), BLACK);
    }
    display.display();
    changed = false;
  }
  
  if(digitalRead(9))
  {
    //Serial.println("BUTTON PUSHED");
    boolean there = false;
    for(int i=0; i < curSize; i++)
    {
      if((dots[i].getXPoint() == drawX && dots[i].getYPoint() == drawY))
      {
        there = true;
      }
    }
    if(!there && curSize < 150)
      {
        Serial.println("NEW POINT at: " + (String)drawX + ", " + (String)drawY);
        Point newPoint = Point::Point(drawX, drawY);
        dots[curSize] = newPoint;
        curSize++;
      }
  }
  if(digitalRead(8))
  {
    Serial.println("BUTTON 2 PUSHED");
    boolean there = false;
    for(int i=0; i < curSize; i++)
    {
      if((dots[i].getXPoint() == drawX && dots[i].getYPoint() == drawY))
      {
        there = true;
      }
      if(there)
      {
        dots[i] = dots[i+1];
      }
    }
    curSize -= there? 1 : 0;
  }
  delay(1);
}

void timeUpdate()
{
  if(xRes > 20)
  {
    xTime = xTime < 0 ? 0 : xTime; //Sets xTime to 0 if it's negative
    xTime += (int) (((double) (xRes/512.0)) * ((double) timePassed*2));
  }
  if(xRes < -20)
  {
    xTime = xTime > 0 ? 0 : xTime;
    xTime += (int) (((double) (xRes/512.0)) * ((double) timePassed*2));
  }
  if(yRes > 20)
  {
    yTime = yTime < 0 ? 0 : yTime; //Sets yTime to 0 if it's negative
    yTime += (int) (((double) (yRes/512.0)) * ((double) timePassed*2));
  }
  if(yRes < -20)
  {
    yTime = yTime > 0 ? 0 : yTime;
    yTime += (int) (((double) (yRes/512.0)) * ((double) timePassed*2));
  }
}

void checkIncrements()
{
  if(abs(xTime) >= 500)
  {
    if(xTime < 0)
    {
      dotIncrementX();
    }
    else
    {
      dotDecrementX();
    }
    changed = true;
    xTime = 0;
  }
  if(abs(yTime) >= 500)
  {
    if(yTime < 0)
    {
      dotIncrementY();
    }
    else
    {
      dotDecrementY();
    }
    changed = true;
    yTime = 0;
  }
}

void dotIncrementX()
{
  if(drawX == 83)
  {
    drawX = 0;
  }
  else
  {
    drawX = drawX + 1;
  }
}

void dotDecrementX()
{
  if(drawX == 0)
  {
    drawX = 83;
  }
  else
  {
    drawX = drawX - 1;
  }
}

void dotIncrementY()
{
  if(drawY == 47)
  {
    drawY = 0;
  }
  else
  {
    drawY = drawY + 1;
  }
}

void dotDecrementY()
{
  if(drawY == 0)
  {
    drawY = 47;
  }
  else
  {
    drawY = drawY - 1;
  }
}


