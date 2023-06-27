#include "esp_wifi_types.h"
#include "UI.h"
#include "WebTime.h"
#include "motor.h"

#include <Wire.h>
#include <WiFi.h>
#include "wiring_shift_mod.h"
#include "Sprinkler.h"
#include "EEPROM_data.h"

#define LED_ACTIVE    0
#define LED_CONNECTED 1
#define LED_AP        2

#define LED_M0        4
#define LED_M1        5
#define LED_M2        6
#define LED_M3        7



void ui_init()
{
    TM1638Str("STA");
    if(WiFi.status() == WL_CONNECTED)
      TM1638Led(LED_CONNECTED, 1 );

    TM1638Led(LED_ACTIVE, 1);
    delay(500);      
    TM1638Led(LED_ACTIVE, 0);
    delay(500);   
    TM1638Led(LED_CONNECTED, 0 ); 
}

void ui_op()
{
    if(WiFi.getMode() & WIFI_MODE_AP) 
    {
      TM1638Led(LED_AP, 1 );
    } 

    if( WebTime::isValid() )
    {
      TM1638Digit(0, WebTime::_HH / 10 );
      TM1638Digit(1, WebTime::_HH % 10,  ( WebTime::_SS % 2 ) == 0);
      TM1638Digit(2, WebTime::_MM / 10 );
      TM1638Digit(3, WebTime::_MM % 10 );

      if( nCurrentMotorIndex >=0 ) // valve open - show sec to close
      {
        TM1638Digit_non0(4, nCurrentMotorTimeLeftSec / 1000);
        TM1638Digit     (5, nCurrentMotorTimeLeftSec / 100 % 10);
        TM1638Digit     (6, nCurrentMotorTimeLeftSec / 10 % 10);
        TM1638Digit     (7, nCurrentMotorTimeLeftSec % 10); 
      }
      else if( nNextMotor != NOT_SET ) // all valve close - show time to next open in min
      {
        TM1638Digit_non0(4, nNextMotor / 1000);
        TM1638Digit     (5, nNextMotor / 100 % 10);
        TM1638Digit     (6, nNextMotor / 10 % 10);
        TM1638Digit     (7, nNextMotor % 10);     
      }
    }

    for( uint8_t m=0; m<CH_NUM; m++)
    {
      TM1638Led(LED_M0 + m, data.ch[m].active );
    }

    TM1638Led(LED_ACTIVE, 1);
    delay(500);

    if( nCurrentMotorIndex >=0 ) // valve open - blink led
      TM1638Led(LED_M0 + nCurrentMotorIndex, 0);

    if( WiFi.softAPgetStationNum() >0 ) // AP client connected - blink led
      TM1638Led(LED_AP, 0 );

    TM1638Led(LED_ACTIVE, 0);
    delay(500);
}

void ui_test()
{
    TM1638Str("test");
    TM1638Led(LED_ACTIVE, 1);
    TM1638Led(LED_M0 , 0);
    TM1638Led(LED_M1 , 0);
    TM1638Led(LED_M2 , 0);
    TM1638Led(LED_M3 , 0);
    delay(500);      

    TM1638Led(LED_ACTIVE, 0);
    if( nCurrentMotorIndex >=0 ) // valve open - blink led
      TM1638Led(LED_M0 + nCurrentMotorIndex, 1);
    delay(500); 
}

void taskUI( void * parameter)
{
  TM1638reset();
  TM1638Str("Bootup");
/*
  for(;;)
  {
    uint8_t Buttons = TM1638Buttons();
    for(uint8_t b = 0; b < 8; b++)
    {
      bool  bPressed = ((0x1 << b) & Buttons) != 0 ;
      TM1638Led(b, bPressed );
    }
    delay(100);
  }
*/

  for(;;)
  {
    switch(Mode)
    {
      case MODE_INIT:
        ui_init();
        break;
      case MODE_OPERATIONAL:
        ui_op();
        break;
      case MODE_TEST:
        ui_test();
        break;
    }
  }
}
