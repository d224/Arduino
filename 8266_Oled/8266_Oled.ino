
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

extern void drawClock(int16_t H, int16_t M, bool dotA, bool dotB, int16_t x, int16_t y);
//105x30


// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h

#define RST_OLED 16

void setup() {
  Serial.begin(115200);
  //udp.begin(localUdpPort);

  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, LOW); // turn D2 low to reset OLED
  delay(50);
  digitalWrite(RST_OLED, HIGH); // while OLED is running, must set D2 in high`

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);

  Serial.println();
  Serial.println("Oled Com Display");

  display.drawString(0, 0, "Ready ....");
  display.display();
}

uint8_t hh = 0;
uint8_t mm = 0;
const int BUFFER_SIZE = 128;
char buf[BUFFER_SIZE];
uint32_t sleep_cnt = 0;

uint8_t dX = 12;// 0..25
uint8_t dY = 1; // 0..2

void loop() 
{
  if ( Serial.available() >= 5 ) 
  {
    uint8_t len = Serial.readBytes( buf, BUFFER_SIZE );

    //Serial.printf("in: %d [%s]\n", len, buf);
    if( buf[2] == ':')
    {
      hh = (buf[0] - '0' ) * 10 + (buf[1] - '0' );
      mm = (buf[3] - '0' ) * 10 + (buf[4] - '0' );

      dY = (mm / 20); //0..3
      dX = hh; // 0..24

      display.clear();
      drawClock(hh, mm, 1, 1, dX, dY);
      display.display();
      //responce:
      //Serial.print(buf);
      Serial.print("OK");
      sleep_cnt = 0;
    }
  }
  else 
  {
    delay(1000);
    sleep_cnt++;
    if( sleep_cnt > 40 )
    {
      display.clear();
      display.display();
    }
    else if( sleep_cnt > 30 )
    {
      display.clear();
      display.drawString(0, 0, "Sleep ....");
      display.display();
    }
  }
}

/*
uint32_t timeWD = 0;
uint32_t _SS = 99;
void loop0()
{
  WebTime.UpdateTask();
  if( _SS != WebTime._SS )
  {
    _SS = WebTime._SS;
    display.clear();
    if( _SS % 2 == 0 )
      drawClock(WebTime._HH, WebTime._MM, 1, 1, 12, 1);
    else
      drawClock(WebTime._HH, WebTime._MM, 0, 0, 12, 1); 
    display.display();
  }
  delay(100);
}
*/