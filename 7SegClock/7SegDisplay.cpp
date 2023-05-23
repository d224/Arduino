#include "7SegDisplay.h"
#include <Adafruit_NeoPixel.h>
#include "WebTime.h"
#include <WiFi.h>

#define NeoPixelPIN_MM        18
#define NeoPixelPIN_HH        9
#define NeoPixelPIN_DOT       39

Adafruit_NeoPixel pixelsMM(28, NeoPixelPIN_MM, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsHH(28, NeoPixelPIN_HH, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsDOT(5, NeoPixelPIN_DOT, NEO_GRB + NEO_KHZ800);

void _7SegMM(uint8_t mm);
void _7SegHH(uint8_t mm);
void _7SegHHMM(uint8_t hh, uint8_t mm);

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

void _7SegShowDigit(  uint8_t startIndex, uint8_t arr7seg[7], Adafruit_NeoPixel * p )
{
  for(int i=0; i<7; i++) 
  {
    uint32_t c = 0;
    if( arr7seg[i]  )
      c = _7SegColor;
    p->setPixelColor( i * 2 + startIndex, c );  
    p->setPixelColor( i * 2 + startIndex + 1, c );
  }
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
    //_7SegHH(88);
    //_7SegMM(88);
    _7SegHHMM(88,88);
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

  uint32_t _WiFiStatus = Adafruit_NeoPixel::Color(0,0,0);
  if( WebTime::isWiFi_connectNeeded() )
    _WiFiStatus = Adafruit_NeoPixel::Color(255,255,0);
  if( WiFi.status() == WL_CONNECTED )
    _WiFiStatus = Adafruit_NeoPixel::Color(0,0,255);
  pixelsDOT.setPixelColor(4, _WiFiStatus );

  uint32_t _geonames_updateStatus = Adafruit_NeoPixel::Color(0,0,0);
  if( (!WebTime::isWiFi_connectNeeded()) && (!WebTime::m_geonames_updated) )
    _geonames_updateStatus = Adafruit_NeoPixel::Color(255,0,0);
  pixelsDOT.setPixelColor(3, _geonames_updateStatus );

  pixelsDOT.show(); 
}

void _7SegTask( void * parameter)
{
 uint8_t _HH = 127;
 uint8_t _MM = 127;
 TickType_t xLastWakeTime = xTaskGetTickCount();
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

      if( _HH != WebTime::_HH || _MM != WebTime::_MM )
      {
        _HH = WebTime::_HH;
        _MM = WebTime::_MM;
        _7SegHHMM(_HH, _MM);
      }

      _7SegDOT_2( WebTime::_SS % 2 == 0 );   
    }
    else 
    {
      _7SegDOT_2( WebTime::_SS % 2 == 0 );
    }

   //delay(1000);
   vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_PERIOD_MS ) );
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

typedef union myColor
{
	  uint32_t val;
	  uint8_t  arr [ 4 ];    
};

void ColorChangeSteps( myColor* sArr, myColor* dArr, myColor* tArr,  uint16_t step, uint16_t steps )
{
  //Serial.printf("sArr: ");  printArr(sArr);
  //Serial.printf("dArr: ");  printArr(dArr);
  for(int i=0; i<28; i++)
  {
    //myColor s = sArr[i];
    //myColor d = sArr[i]; 
    myColor t;
    for( int j=0; j<4; j++)
    {
      uint8_t sn = sArr[i].arr[j];
      uint8_t dn = dArr[i].arr[j];
      float delta = dn - sn;
      float step_delta = delta  * (step + 1) / steps;
      t.arr[j] = sn + step_delta;

      //Serial.printf("j[%d]: s:%02X, d:%02X t:%02X \n", j, sn, dn, t.arr[j]);  

    }  
    tArr[i] = t;
  }
  //Serial.printf("tArr: ");  printArr(tArr);
}

myColor  sArrMM[28];
myColor  dArrMM[28];
myColor  tArrMM[28];

myColor  sArrHH[28];
myColor  dArrHH[28];
myColor  tArrHH[28];

void _7SegHHMM(uint8_t hh, uint8_t mm)
{
  uint8_t h0 = hh % 10;
  uint8_t h1 = hh / 10;

  uint8_t m0 = mm / 10;
  uint8_t m1 = mm % 10;

  for(int i=0; i<28; i++)
  {
    sArrHH[i].val = pixelsHH.getPixelColor(i);
    sArrMM[i].val = pixelsMM.getPixelColor(i);   
  }

  pixelsHH.setBrightness(_7SegValue);
  pixelsMM.setBrightness(_7SegValue);

  _7SegShowDigit( 0 , arr7segHH[h0], &pixelsHH );
  _7SegShowDigit( 14, arr7segHH[h1], &pixelsHH );

  _7SegShowDigit( 0 , arr7segMM[m0], &pixelsMM );
  _7SegShowDigit( 14, arr7segMM[m1], &pixelsMM ); 

  for(int i=0; i<28; i++)
  {
    dArrHH[i].val = pixelsHH.getPixelColor(i);
    dArrMM[i].val = pixelsMM.getPixelColor(i);
  }

  for(int s=0; s<10; s++)
  {
    ColorChangeSteps( sArrMM, dArrMM, tArrMM,  s, 10 );
    ColorChangeSteps( sArrHH, dArrHH, tArrHH,  s, 10 );
    for(int i=0; i<28; i++)
    {
      pixelsHH.setPixelColor(i, tArrHH[i].val );
      pixelsMM.setPixelColor(i, tArrMM[i].val );
    }
        
    pixelsHH.show();
    pixelsMM.show();
    delay(50);
  }

}

/*
void _7SegMM_new(uint8_t mm)
{
  uint8_t m0 = mm / 10;
  uint8_t m1 = mm % 10;
  //pixelsMM.clear();

  for(int i=0; i<28; i++)
    sArrMM[i].val = pixelsMM.getPixelColor(i);

  pixelsMM.setBrightness(_7SegValue);
  _7SegShowDigit( 0 , arr7segMM[m0], &pixelsMM );
  _7SegShowDigit( 14, arr7segMM[m1], &pixelsMM );

  for(int i=0; i<28; i++)
    dArrMM[i].val = pixelsMM.getPixelColor(i);

  for(int s=0; s<10; s++)
  {
    ColorChangeSteps( sArrMM, dArrMM, tArrMM,  s, 10 );
    for(int i=0; i<28; i++)
    {
      uint32_t color = tArrMM[i].val;
      pixelsMM.setPixelColor(i, color );
    }
      
    pixelsMM.show();
    delay(50);
  }
}

void printArr(myColor arr[28])
{
   for(int i=0; i<28; i++)
    Serial.printf("%08X ", arr[i].val); 
  Serial.printf("\n");
}

void _7SegMM_old(uint8_t mm)
{
  uint8_t m0 = mm / 10;
  uint8_t m1 = mm % 10;
  pixelsMM.clear();
  pixelsMM.setBrightness(_7SegValue);
  _7SegShowDigit( 0 , arr7segMM[m0], &pixelsMM );
  _7SegShowDigit( 14, arr7segMM[m1], &pixelsMM );
  pixelsMM.show(); 
}

void _7SegHH_old(uint8_t hh)
{
  uint8_t h0 = hh % 10;
  uint8_t h1 = hh / 10;
  pixelsHH.clear();
  pixelsHH.setBrightness(_7SegValue);
  _7SegShowDigit( 0 , arr7segHH[h0], &pixelsHH );
  _7SegShowDigit( 14, arr7segHH[h1], &pixelsHH );
  pixelsHH.show(); 
}
*/


