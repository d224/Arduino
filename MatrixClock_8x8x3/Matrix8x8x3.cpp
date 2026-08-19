#include <Arduino.h>
#include "Matrix8x8x3.h"

// NeoPixel stick DIN pin
#define DIN_PIN_A 1
#define DIN_PIN_B 2
#define DIN_PIN_C 15

#include "font_5x8.h"
#include "font_4x7.h"
#include "font_3x5.h"


// How many NeoPixels on the stick?
#define NUM_PIXELS (64)

// Third parameter:
//   NEO_RGB     Pixels are wired for RGB bitstreamc:\D224\Git\Arduino\MatrixClock_5x17_C3\NeoPixel3x5Helper.h
//   NEO_GRB     Pixels are wired for GRB bitstream (NeoPixel Stick)
//   NEO_KHZ400  400 KHz bitstream for FLORA pixels
//   NEO_KHZ800  800 KHz bitstream for High Density LED strip (NeoPixel Stick)
Adafruit_NeoPixel strip_A = Adafruit_NeoPixel(NUM_PIXELS, DIN_PIN_A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_B = Adafruit_NeoPixel(NUM_PIXELS, DIN_PIN_B, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_C = Adafruit_NeoPixel(NUM_PIXELS, DIN_PIN_C, NEO_GRB + NEO_KHZ800);


void Matrixsetup() {
  strip_A.begin();
  strip_A.show(); // Start with all pixels off
  strip_B.begin();
  strip_B.show(); // Start with all pixels off
  strip_C.begin();
  strip_C.show(); // Start with all pixels off
  delay(1000);
}

void Matrix(uint8_t x, uint8_t y, uint32_t c)
{
  if( x < 8 )
  {
    strip_A.setPixelColor( x*8 + y , c );
    return;
  }
  x-=8;
  if( x < 8 )
  {
    strip_B.setPixelColor( x*8 + y , c );
    return;
  }
  x-=8;
  strip_C.setPixelColor( x*8 + y , c );
}

void Show()
{
  strip_A.show();
  strip_B.show();
  strip_C.show();  
}

void Clear()
{
  strip_A.clear();
  strip_B.clear();
  strip_C.clear(); 
}

void randomPixels( uint32_t color,  uint8_t dencity )
{
  for( uint8_t x=0; x<24 * 3; x++ )
    for( uint8_t y=0; y<8; y++ )
  {
    if( random( 100 ) < dencity )
      Matrix( x, y, color );
    else
      Matrix( x, y, 0 );
  }
  Show();
}

void printChar5x8(uint8_t ch, uint8_t x, uint32_t color)
{
  if (ch < 0x20 || ch > 0x7E) 
    return; 

  int font_index = ch - 0x20;

  for(uint8_t i=0; i<5; i++)
  {
    uint8_t line = font_5x8[font_index][i];
    for(uint8_t row=0; row<8; row++)
    {
      if(line & (1 << row))
        Matrix(x+i, 7-row, color);
      else
        Matrix(x+i, 7-row, 0);
    }
  }
}
void printChar4x7(uint8_t ch, uint8_t x, uint32_t color)
{
  if (ch < 0x20 || ch > 90) 
    return; 

  int font_index = ch - 0x20;

  for(uint8_t i=0; i<4; i++)
  {
    uint8_t line = font_4x7[font_index][i];
    for(uint8_t row=0; row<7; row++)
    {
      if(line & (1 << row))
        Matrix(x+i, 6-row, color);
      else
        Matrix(x+i, 6-row, 0);
    }
  }
}

void printChar3x5(uint8_t ch, uint8_t x, uint32_t color)
{
  if (ch < 0x20 || ch > 90) 
    return; 

  int font_index = ch - 0x20;

  for(uint8_t i=0; i<3; i++)
  {
    uint8_t line = font_3x5[font_index][i];
    for(uint8_t row=0; row<5; row++)
    {
      if(line & (1 << row))
        Matrix(x+i, 5-row, color);
      else
        Matrix(x+i, 5-row, 0);
    }
  }
}

void drawClock(uint8_t H, uint8_t M, uint8_t x, bool bDots, uint32_t color)
{ 
  Clear();
  if( bDots )
    printChar3x5(':', 8 + x,  color);

  printChar3x5('0'+ (H / 10),  0 + x, color);
  printChar3x5('0'+ (H % 10),  4 + x, color);

  printChar3x5('0'+ (M / 10), 10 + x, color);
  printChar3x5('0'+ (M % 10), 14 + x, color);
  Show();
}

void drawString(char * str, uint8_t x, uint32_t color)
{
  Clear(); 
  uint8_t i = 0;
  while(str[i] != 0)
  {
    printChar4x7(str[i], x, color);
    i++;
    x+=5;
  }
  Show();
}

