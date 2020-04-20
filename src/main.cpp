
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

// This example shows a scrolling text.
// If U8G2_16BIT is not set (default), then the pixel width of the text must be lesser than 128
// If U8G2_16BIT is set, then the pixel width an be up to 32000 


const int timeouts[5] = {1, 5, 10, 20, 30};
const char *modes[2] = {"Wash", "Cure"};
const char *activeModes[2] = {"Washing", "Curing"};

char cstr[16];

int mode = 0;
int time = 0;
int endstop = 0;
bool onPause = true;
bool onAction = false;

char *secToTime(int seconds){
  char result[5];
  sprintf(result,"%i:%i",seconds/60, seconds%60);
  // char *min = itoa(seconds%60, cstr, 10);
  // char *sec = itoa(seconds%60, cstr, 10);
  // return min + ":" + sec; 
  return result;
}

void actionScreen(){
  u8g2.firstPage();
  do {
    int firstStringPos = 14;
    int secondStringPos = 50;
    
    u8g2.setFontMode(1);  /* activate transparent font mode */
    u8g2.setDrawColor(1); /* color 1 for the box */
    u8g2.drawBox(0, 29, 128, 34);
    u8g2.setFont(u8g2_font_logisoso16_tf);
    u8g2.drawStr((127-u8g2.getStrWidth(activeModes[mode]))/2, firstStringPos, activeModes[mode]);
    u8g2.setFont(u8g2_font_logisoso26_tf);
    u8g2.setDrawColor(2);
    u8g2.drawStr(27, secondStringPos, secToTime(200));
    
    if (onPause) {
      u8g2.setFont(u8g2_font_unifont_t_symbols);
       u8g2.drawGlyph(105, 14, 0x23f8); 
       }
    else {
      // u8g2.setFont(u8g2_font_unifont_t_symbols);
       u8g2.drawStr(105, 14, " "); 
    }
  } while ( u8g2.nextPage() );
}

void menuScreen(){
  u8g2.firstPage();
  do {
    int firstStringPos = 18;
    int secondStringPos = 52;
    u8g2.setFont(u8g2_font_logisoso20_tf);
    
    u8g2.setFontMode(1);  /* activate transparent font mode */
    u8g2.setDrawColor(1); /* color 1 for the box */
    u8g2.drawBox(0, 0, 128, 31);
    // u8g2.setFont(u8g2_font_inb16_mf);
    u8g2.drawStr((127-u8g2.getStrWidth("10m"))/2, secondStringPos, "10m");
    u8g2.setDrawColor(2);
    u8g2.drawStr((127-u8g2.getStrWidth(modes[mode]))/2, firstStringPos, modes[mode]);
    
  } while ( u8g2.nextPage() );
}


void demo(){
  menuScreen();
  delay(5000);
  actionScreen();
  delay(5000);

  
}

void setup(void) {

  /* U8g2 Project: SSD1306 Test Board */
  //pinMode(10, OUTPUT);
  //pinMode(9, OUTPUT);
  //digitalWrite(10, 0);
  //digitalWrite(9, 0);		

  /* U8g2 Project: T6963 Test Board */
  //pinMode(18, OUTPUT);
  //digitalWrite(18, 1);	

  /* U8g2 Project: KS0108 Test Board */
  //pinMode(16, OUTPUT);
  //digitalWrite(16, 0);	

  u8g2.begin();  
  
  u8g2.setFont(u8g2_font_logisoso20_tf);
  u8g2.setFontPosCenter();	// set the target font to calculate the pixel width
  // width = u8g2.getUTF8Width(text);		// calculate the pixel width of the text
  
  u8g2.setFontMode(0);		// enable transparent mode, which is faster
}




void loop(void) {
  demo();
}