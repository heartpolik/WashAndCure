
#include <Arduino.h>
#include <U8g2lib.h>
#include "timer-api.h"

#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE); // All Boards without Reset of the Display

// This example shows a scrolling text.
// If U8G2_16BIT is not set (default), then the pixel width of the text must be lesser than 128
// If U8G2_16BIT is set, then the pixel width an be up to 32000

const int modesCount = 2;
const int timesCount = 5;

const int timeouts[timesCount] = {1, 5, 10, 20, 30};
const char *modes[modesCount] = {"Wash", "Cure"};
const char *activeModes[modesCount] = {"Washing", "Curing"};
const int motorSpeed[modesCount] = {25, 400};

const int coil1Pin = 3;
const int coil2Pin = 5;
const int coil3Pin = 6;
const int lampPin = 9;
const int tonePin = 11;

const int modeBtn = 2;
const int timePauseBtn = 4;
const int startStopBtn = 7;

const int firstStringPos = 19;
const int secondStringPos = 51;

char cstr[16];

int activeCoil;
int mode = 0;
int time = 0;
int endstop = 60;
char secToStr[6];
char minToStr[4] = {"1m"};
bool onPause = false;
bool onAction = false;

void changeMode()
{
  mode = (mode + 1) % modesCount;
}

void secToTime()
{
  if (endstop % 60 > 9)
  {
    sprintf(secToStr, "%i:%i", endstop / 60, endstop % 60);
  }
  else
  {
    sprintf(secToStr, "%i:0%i", endstop / 60, endstop % 60);
  }
}

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
    // u8g2.setFont(u8g2_font_logisoso16_tf);
    u8g2.drawStr((127 - u8g2.getStrWidth(activeModes[mode])) / 2, firstStringPos, activeModes[mode]);
    // u8g2.setFont(u8g2_font_logisoso26_tf);
    u8g2.setDrawColor(2);
    u8g2.drawStr((127 - u8g2.getStrWidth(secToStr)) / 2, secondStringPos, secToStr);

  } while (u8g2.nextPage());
}

void menuScreen()
{
  u8g2.firstPage();
  do
  {
    // u8g2.setFont(u8g2_font_logisoso20_tf);

    u8g2.setFontMode(1);  /* activate transparent font mode */
    u8g2.setDrawColor(1); /* color 1 for the box */
    u8g2.drawBox(0, 0, 128, 31);
    // u8g2.setFont(u8g2_font_inb16_mf);
    u8g2.drawStr((127 - u8g2.getStrWidth(minToStr)) / 2, secondStringPos, minToStr);
    u8g2.setDrawColor(2);
    u8g2.drawStr((127 - u8g2.getStrWidth(modes[mode])) / 2, firstStringPos, modes[mode]);

  } while (u8g2.nextPage());
}

void rotateMotor()
{
  switch (activeCoil)
  {
  case 1:
    digitalWrite(coil1Pin, HIGH);
    digitalWrite(coil2Pin, LOW);
    digitalWrite(coil3Pin, LOW);
    break;

  case 2:
    digitalWrite(coil1Pin, LOW);
    digitalWrite(coil2Pin, HIGH);
    digitalWrite(coil3Pin, LOW);
    break;

  default:
    digitalWrite(coil1Pin, LOW);
    digitalWrite(coil2Pin, LOW);
    digitalWrite(coil3Pin, HIGH);

    break;
  }

  activeCoil = (activeCoil + 1) % 3;
}

void stopAll()
{
  digitalWrite(coil1Pin, LOW);
  digitalWrite(coil2Pin, LOW);
  digitalWrite(coil3Pin, LOW);
  digitalWrite(lampPin, LOW);
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
void startTone()
{
  tone(tonePin, 500, 40);
  tone(tonePin, 800, 80);
  tone(tonePin, 1300, 150);
}
void finishTone()
{
  tone(tonePin, 1300, 40);
  tone(tonePin, 800, 80);
  tone(tonePin, 1300, 150);
}

void setup(void)
{

  pinMode(modeBtn, INPUT);
  pinMode(startStopBtn, INPUT);
  pinMode(timePauseBtn, INPUT);
  pinMode(coil1Pin, OUTPUT);
  pinMode(coil2Pin, OUTPUT);
  pinMode(coil3Pin, OUTPUT);
  pinMode(lampPin, OUTPUT);
  pinMode(tonePin, OUTPUT);

  u8g2.begin();

  u8g2.setFont(u8g2_font_logisoso18_tf);
  u8g2.setFontPosCenter(); // set the target font to calculate the pixel width
  // width = u8g2.getUTF8Width(text);		// calculate the pixel width of the text

  u8g2.setFontMode(0); // enable transparent mode, which is faster
  timer_init_ISR_1KHz(TIMER_DEFAULT);
  // updateScreen();
  secToTime();
}

void loop(void)
{
  updateScreen();
  if (digitalRead(modeBtn) == HIGH)
  {
    if (onAction)
    {
      badTone();
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
      startTone();
    }
    updateScreen();
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
  static unsigned long prev_time = 0;

  // дополнильный множитель периода
  static int motorCount = motorSpeed[mode];
  static int timeoutCount = 1000;

  // Печатаем сообщение на каждый 12й вызов прерывания:
  // если базовая частота 10Гц и базовый период 100мс,
  // то сообщение будет печататься каждые 100мс*12=1200мс
  // (5 раз за 6 секунд)

  if (onAction && !onPause && endstop > 0)
  {
    if (motorCount == 0)
    {
      unsigned long _time = micros();
      unsigned long _period = _time - prev_time;
      prev_time = _time;

      if (onAction && !onPause)
      {
        rotateMotor();
      }

      // взводим счетчик
      motorCount = motorSpeed[mode];
    }
    else
    {
      motorCount--;
    }
    if (timeoutCount == 0)
    {
      endstop--;
      timeoutCount = 1000;
    }
    else
    {
      timeoutCount--;
    }
  }
}