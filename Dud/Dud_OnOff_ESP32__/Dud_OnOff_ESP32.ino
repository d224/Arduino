#include <WiFi.h>
#include <WiFiMulti.h>
#include <Adafruit_NeoPixel.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PW_SW LED_BUILTIN
WiFiMulti wifiMulti;
Adafruit_NeoPixel rgb0 (1, 15, NEO_GRB + NEO_KHZ800 );
bool nState = 0;

// WiFi connect timeout per AP. Increase when connecting takes longer.
bool bConnected = false;
uint nCurrent_mA = 0;
#define bCurrent (nCurrent_mA > 10)
const char* host = "Dud_OnOff";
#define LED_VAL_R ( 0xFF0000 )
#define LED_VAL_G ( 0x00FF00 )
#define LED_VAL_B ( 0x0000FF )

#define RED		   (LED_VAL_R)
#define GREEN 	 (LED_VAL_G)
#define BLUE	   (LED_VAL_B)  
#define YELLOW	 (LED_VAL_R | LED_VAL_G) 
#define CYAN	   (LED_VAL_G | LED_VAL_B)
#define MAGENTA	 (LED_VAL_R | LED_VAL_B)
#define WHITE	   (LED_VAL_R | LED_VAL_G | LED_VAL_B)
#define LED_OFF  ( 0 )
void RGB_Show( uint32_t c )
{
  rgb0.setPixelColor( 0, c );
  rgb0.show();
}
void RGB_Begin()
{
  rgb0.begin(); 
  rgb0.clear(); 
  rgb0.setBrightness( 32 );

  RGB_Show( RED );   delay( 100 );
  RGB_Show( GREEN ); delay( 100 );
  RGB_Show( BLUE );  delay( 100 );
  RGB_Show( WHITE );
}
const uint32_t connectTimeoutMs = 10000;

void notificationTask( void * parameter) 
{
  for(;;)
  {
    if( nState ) 
    {
      // ON
      digitalWrite(PW_SW, HIGH);
      if( bCurrent )
         RGB_Show( RED );
      else
         RGB_Show( YELLOW );
    } 
    else 
    {
      // OFF
      digitalWrite(PW_SW, LOW);
      RGB_Show( LED_OFF );
    }
    delay(500);
    
    if( bConnected )
    {
      RGB_Show( BLUE );
    }
    else // disconnected 
    {
      RGB_Show( GREEN );
    }
    delay(500);
  }
}

void mainTask( void * parameter) 
{
  uint di = 0;
  for(;;)
  {
    
    if (wifiMulti.run() != WL_CONNECTED)
    {
      bConnected = false;
      Serial.print("WiFi disconnected ");
      Serial.println( di++ );
      delay(1000);
    } 
    else 
    {
      if( bConnected == false ) // new connection 
      {
        Serial.print("WiFi connected: ");
        Serial.println(WiFi.localIP());
      }    
      bConnected = true;
      delay(10);
    }    
    vTaskDelay(10);
  }

}

void setup()
{
  RGB_Begin();
  Serial.begin(115200);
  delay(10);
  WiFi.mode(WIFI_STA);

  while (!Serial && !Serial.available()) {}
  Serial.printf("****Start****");

  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN", "1qazxsw2");
  wifiMulti.addAP("D224", "1qazxsw2");

  xTaskCreatePinnedToCore( notificationTask,  "notificationTask", 1024, NULL, tskIDLE_PRIORITY + 1 , NULL, 1);   
  xTaskCreatePinnedToCore( mainTask,          "mainTask",         10240, NULL, tskIDLE_PRIORITY + 1 , NULL, 1);
  //vTaskStartScheduler();
}

uint di = 0;
void loop() 
{
  /*
    if (wifiMulti.run() != WL_CONNECTED)
    {
      bConnected = false;
      Serial.print("WiFi disconnected ");
      Serial.println( di++ );
      delay(1000);
    } 
    else 
    {
      if( bConnected == false ) // new connection 
      {
        Serial.print("WiFi connected: ");
        Serial.println(WiFi.localIP());
      }    
      bConnected = true;
      delay(10);
    }
    */
}
