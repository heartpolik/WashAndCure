
#include <Arduino.h>
#include <U8g2lib.h>
#include "timer-api.h"
#include <Wire.h>



U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE); // All Boards without Reset of the Display

const int modesCount = 2;
const int timesCount = 6;

const int timeouts[timesCount] = {1, 5, 10, 20, 30, 60};
const char *modes[modesCount] = {"Wash", "Cure"};
const char *activeModes[modesCount] = {"Washing", "Curing"};
const int pulseLength = 15;
int mode = 0;
const int minSpeed[modesCount] = {1000, 1000};
const int motorSpeed[modesCount] = {26, 1000};

int currSpeed = minSpeed[mode];
const int stepPin = 11;
const int enPin = 12;

const int ms1 = 6;
const int ms2 = 5;
const int ms3 = 3;
const int lampPin = 8;
const int tonePin = 9;

const int modeBtn = 2;
const int timePauseBtn = 4;
const int startStopBtn = 7;

const int firstStringPos = 19;
const int secondStringPos = 51;

char cstr[16];


int time = 0;
int endstop = 60;
char secToStr[6];
char minToStr[4] = {"1m"};
bool onPause = false;
bool onAction = false;

void changeMode()
{
  mode = (mode + 1) % modesCount;
  currSpeed = minSpeed[mode];
  if (mode == 1){
    digitalWrite(ms1, HIGH);
    digitalWrite(ms2, HIGH);
    digitalWrite(ms3, HIGH);
  } else {
    digitalWrite(ms1, LOW);
    digitalWrite(ms2, LOW);
    digitalWrite(ms3, LOW);
  }
}

void secToTime()
{
  if (currSpeed > motorSpeed[mode]){
           sprintf(secToStr, "%s", "SpdUp");    
        }
        else {
  if (endstop % 60 > 9)
  {
    sprintf(secToStr, "%i:%i", endstop / 60, endstop % 60);
  }
  else
  {
    sprintf(secToStr, "%i:0%i", endstop / 60, endstop % 60);
  }
}}

void changeTime()
{
  time = (time + 1) % timesCount;
  sprintf(minToStr, "%im", timeouts[time]);
  endstop = timeouts[time] * 60;
  secToTime();
}

void actionScreen()
{
  u8g2.firstPage();
  do
  {
    u8g2.setFontMode(1);  /* activate transparent font mode */
    u8g2.setDrawColor(1); /* color 1 for the box */
    u8g2.drawBox(0, 32, 128, 64);
    u8g2.drawStr((127 - u8g2.getStrWidth(activeModes[mode])) / 2, firstStringPos, activeModes[mode]);
    u8g2.setDrawColor(2);
    u8g2.drawStr((127 - u8g2.getStrWidth(secToStr)) / 2, secondStringPos, secToStr);

  } while (u8g2.nextPage());
}

void menuScreen()
{
  u8g2.firstPage();
  do
  {
    u8g2.setFontMode(1);  /* activate transparent font mode */
    u8g2.setDrawColor(1); /* color 1 for the box */
    u8g2.drawBox(0, 0, 128, 31);
    u8g2.drawStr((127 - u8g2.getStrWidth(minToStr)) / 2, secondStringPos, minToStr);
    u8g2.setDrawColor(2);
    u8g2.drawStr((127 - u8g2.getStrWidth(modes[mode])) / 2, firstStringPos, modes[mode]);

  } while (u8g2.nextPage());
}

void stopAll()
{
  currSpeed = minSpeed[mode];
  digitalWrite(stepPin, LOW);
  digitalWrite(lampPin, LOW);
  digitalWrite(enPin, HIGH);
}

void updateScreen()
{
  if (onAction)
  {
    actionScreen();
  }
  else
  {
    menuScreen();
  }
}

void goodTone()
{
  tone(tonePin, 1500, 150);
}
void badTone()
{
  tone(tonePin, 400, 250);
}
void finishTone()
{
  tone(tonePin, 1300, 150);
}

void setup(void)
{

  pinMode(modeBtn, INPUT);
  pinMode(startStopBtn, INPUT);
  pinMode(timePauseBtn, INPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(enPin, OUTPUT);
  pinMode(lampPin, OUTPUT);
  digitalWrite(lampPin, LOW);
  pinMode(tonePin, OUTPUT);
  pinMode(ms1, OUTPUT);
  pinMode(ms2, OUTPUT);
  pinMode(ms3, OUTPUT);
  digitalWrite(enPin, HIGH);

  u8g2.begin();

  u8g2.setFont(u8g2_font_crox3hb_tr);
  u8g2.setFontPosCenter(); // set the target font to calculate the pixel width
  u8g2.setFontMode(0); // enable transparent mode, which is faster
  timer_init_ISR_10KHz(TIMER_DEFAULT);
  secToTime();
}

void loop(void)
{
  updateScreen();
  digitalWrite(enPin, (onAction && !onPause) ? LOW : HIGH);
  if (digitalRead(modeBtn) == HIGH)
  {
    if (onAction)
    {
      goodTone();
      endstop += 60;
    }
    else
    {
      goodTone();
      changeMode();
      updateScreen();
      delay(50);
    }
  }
  if (digitalRead(startStopBtn) == HIGH)

  {
    onAction = !onAction;
    if (!onAction)
    {
      goodTone();
      stopAll();
      onPause = false;
      endstop = timeouts[time] * 60;
    }
    else
    {
      finishTone();
    }
    updateScreen();
    digitalWrite(enPin, (onAction && !onPause) ? LOW : HIGH);
    delay(50);
  }
  if (digitalRead(timePauseBtn) == HIGH)
  {
    goodTone();
    if (onAction)
    {
      if (endstop > 0)
      {
        onPause = !onPause;
        if (onPause)
        {
          stopAll();
          sprintf(secToStr, "%s", "Pause");
        }
        else
        {
          secToTime();
        }
      }
      else
      {
        badTone();
      }
    }
    else
    {
      changeTime();
    }
    updateScreen();
    digitalWrite(enPin, (onAction && !onPause) ? LOW : HIGH);
    delay(50);
  }

  if (onAction && !onPause)
  {
    if (endstop > 0)
    {
      
      secToTime();
      if (mode == 1)
      {
        digitalWrite(lampPin, HIGH);
      }
    }
    else
    {
      finishTone();
      stopAll();
      sprintf(secToStr, "%s", "End");
    }
  }
}
void timer_handle_interrupts(int timer)
{
  // ???????????????????????? ?????????????????? ??????????????
  static int motorCount = currSpeed;
  static int timeoutCount = 10000; //1sec

  // ???????????????? ?????????????????? ???? ???????????? 12?? ?????????? ????????????????????:
  // ???????? ?????????????? ?????????????? 10???? ?? ?????????????? ???????????? 100????,
  // ???? ?????????????????? ?????????? ???????????????????? ???????????? 100????*12=1200????
  // (5 ?????? ???? 6 ????????????)

  if (onAction && !onPause && endstop > 0)
  {

    //  if (motorCount == pulseLength + 5 && mode == 1){
    //       digitalWrite(enPin, LOW);
    //   }
      if (motorCount == pulseLength){
        digitalWrite(stepPin, HIGH);
      }
      if (motorCount == 0)
      {
        // if (mode == 1){
        //   digitalWrite(enPin, HIGH);
        // }
        digitalWrite(stepPin, LOW);
      // ?????????????? ??????????????
      if (currSpeed > motorSpeed[mode])
       { currSpeed = currSpeed - currSpeed/20 ;}
        motorCount = currSpeed;
      }
      else
      {
        motorCount--;
      }

      // seconds timer
      if (timeoutCount == 0)
      {
        if (currSpeed <= motorSpeed[mode]){
           endstop--;    
        }
        
        timeoutCount = 10000;
      }
      else
      {
        timeoutCount--;
      }
  }
}
