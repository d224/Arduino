
#include "7SegDisplay.h"
#include <Adafruit_NeoPixel.h>
#include "WebTime.h"

#define NeoPixelPIN_MM        18
#define NeoPixelPIN_HH        9
#define NeoPixelPIN_DOT       39

Adafruit_NeoPixel pixelsMM(28, NeoPixelPIN_MM, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsHH(28, NeoPixelPIN_HH, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsDOT(5, NeoPixelPIN_DOT, NEO_GRB + NEO_KHZ800);

uint8_t arr7segMM[10][7] =
{ {0, 1, 1, 1, 1, 1, 1}, 
  {0, 1, 0, 0, 0, 0, 1}, 
  {1, 0, 1, 1, 0, 1, 1}, 
  {1, 1, 1, 0, 0, 1, 1}, 
  {1, 1, 0, 0, 1, 0, 1}, 
  {1, 1, 1, 0, 1, 1, 0}, 
  {1, 1, 1, 1, 1, 1, 0}, 
  {0, 1, 0, 0, 0, 1, 1}, 
  {1, 1, 1, 1, 1, 1, 1}, 
  {1, 1, 1, 0, 1, 1, 1} };

uint8_t arr7segHH[10][7] =
{ {0, 1, 1, 1, 1, 1, 1}, 
  {0, 0, 0, 1, 1, 0, 0}, 
  {1, 1, 1, 0, 1, 1, 0}, 
  {1, 0, 1, 1, 1, 1, 0}, //3
  {1, 0, 0, 1, 1, 0, 1}, //4
  {1, 0, 1, 1, 0, 1, 1}, //5
  {1, 1, 1, 1, 0, 1, 1}, //6
  {0, 0, 0, 1, 1, 1, 0}, //7
  {1, 1, 1, 1, 1, 1, 1}, 
  {1, 0, 1, 1, 1, 1, 1} };

uint32_t _7SegColor = Adafruit_NeoPixel::Color(0,255,0);
uint8_t  _7SegValue = 64;
extern uint8_t _7SegValueNew;

void _7SegShowDigit( uint8_t d, uint8_t startIndex, uint8_t arr7seg[][7], Adafruit_NeoPixel * p )
{
  for(int i=0; i<7; i++) 
  {
    uint32_t c = 0;
    if( arr7seg[d][i]  )
      c = _7SegColor;

    p->setPixelColor( i * 2 + startIndex, c );  
    p->setPixelColor( i * 2 + startIndex + 1, c );
  }
}

void _7SegMM(uint8_t mm)
{
  uint8_t m0 = mm / 10;
  uint8_t m1 = mm % 10;
  pixelsMM.clear();
  pixelsMM.setBrightness(_7SegValue);
  _7SegShowDigit( m0, 0 , arr7segMM, &pixelsMM );
  _7SegShowDigit( m1, 14, arr7segMM, &pixelsMM );
 /* 
  for(int i=0; i<7; i++) 
  {
    uint32_t c = 0;
    if( arr7segMM[m0][i]  )
      c = _7SegColor;

    pixelsMM.setPixelColor(i*2+0, c);  
    pixelsMM.setPixelColor(i*2+1, c);
  }
  for(int i=0; i<7; i++) 
  {
    uint32_t c = 0;
    if( arr7segMM[m1][i]  )
      c = _7SegColor;

    pixelsMM.setPixelColor(i*2+14, c);  
    pixelsMM.setPixelColor(i*2+15, c);
  }
  */
  pixelsMM.show(); 
}

void _7SegHH(uint8_t hh)
{
  uint8_t h0 = hh % 10;
  uint8_t h1 = hh / 10;
  pixelsHH.clear();
  pixelsHH.setBrightness(_7SegValue);
  _7SegShowDigit( h0, 0 , arr7segHH, &pixelsHH );
  _7SegShowDigit( h1, 14, arr7segHH, &pixelsHH );
 /*   
  for(int i=0; i<7; i++) 
  {
    uint32_t c = 0;
    if( arr7segHH[h0][i]  )
      c = _7SegColor;

    pixelsHH.setPixelColor(i*2+0, c);  
    pixelsHH.setPixelColor(i*2+1, c);
  }
  for(int i=0; i<7; i++) 
  {
    uint32_t c = 0;
    if( arr7segHH[h1][i]  )
      c = _7SegColor;

    pixelsHH.setPixelColor(i*2+14, c);  
    pixelsHH.setPixelColor(i*2+15, c);
  }
  */
  pixelsHH.show(); 
}

void _7SegDOT_All(uint8_t val)
{
  pixelsDOT.clear();
  pixelsDOT.setBrightness(_7SegValue);
  
  for(int i=0; i<5; i++) 
  {
    uint32_t c = 0;
    if( (val >> i & 1)  )
      c = _7SegColor;
    pixelsDOT.setPixelColor(i, c);  
  }

  pixelsDOT.show(); 
}

void _7SegAll(bool show)
{
  if( show )
  {
    _7SegHH(88);
    _7SegMM(88);
    _7SegDOT_All(0x1F);
  }
  else
  {
    _7SegDOT_All( 0 );
    pixelsHH.clear(); pixelsHH.show();
    pixelsMM.clear(); pixelsMM.show();
  }
}

void _7SegDOT_2(bool show)
{
  //pixelsDOT.clear();
  pixelsDOT.setBrightness(_7SegValue);
  
  uint32_t c = 0;
  if( show  )
    c = _7SegColor;

  pixelsDOT.setPixelColor(1, c);  
  pixelsDOT.setPixelColor(2, c); 

  pixelsDOT.show(); 
}

void _7SegTask( void * parameter)
{
 for(;;)
  {
    _7SegValue = _7SegValueNew;
    if ( WebTime::isValid() )
    {
      if ( WebTime::m_secSinceMidNight == 4*3600 ) // 04:00
        {
          Serial.print("dayly resart :)");
          ESP.restart();
        }

      if(WebTime::isDay())
         _7SegColor = Adafruit_NeoPixel::Color(0,255,0);
      else
        _7SegColor = Adafruit_NeoPixel::Color(0,255,255);     

      _7SegHH( WebTime::_HH );
      _7SegMM( WebTime::_MM );
      _7SegDOT_2( WebTime::_SS % 2 == 0 );   
    }
    else 
    {
      _7SegDOT_2( WebTime::_SS % 2 == 0 );
    }

  delay(1000);
  }
}

void _7SegSetup()
{
    _7SegColor = Adafruit_NeoPixel::Color(255,0,0);
  _7SegAll(true); delay(50);  
  _7SegColor = Adafruit_NeoPixel::Color(0,0,255);
  _7SegAll(true); delay(50);  
  _7SegColor = Adafruit_NeoPixel::Color(0,255,0);
  _7SegAll(true); delay(50);  
  _7SegAll(false);

  xTaskCreatePinnedToCore( _7SegTask,  "_7SegTask", 10240, NULL, 0, NULL, 1); 
}

