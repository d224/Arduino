#include <WiFi.h>
#include <WiFiMulti.h>
#include "WebTime.h"
#include "sntp.h"
#include "7SegDisplay.h"
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

WiFiMulti wifiMulti;
WebTime webTime;

uint8_t  _7SegValueNew = 64;

void lightMeterTask( void * parameter)
{
  Wire.begin(33, 35);
  Wire.begin();
  lightMeter.begin();
  for(;;)
  {
    float lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");

    uint8_t _7SegValueTmp = 128;

    if(lux < 1000)
      _7SegValueTmp = 64;
    
    if(lux < 100)
      _7SegValueTmp = 36;

    if(lux < 5)
      _7SegValueTmp = 16;

    if(lux < 1)
      _7SegValueTmp = 4;  

    _7SegValueNew = _7SegValueTmp;
    delay(10000);       
  }
}


void setup()
{
  Serial.begin(115200);
  WiFi.disconnect(true);

  _7SegSetup();
  webTime.Start();

  xTaskCreatePinnedToCore( lightMeterTask,  "lightMeterTask", 10240, NULL, 0, NULL, 1);   

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN", "1qazxsw2");
  wifiMulti.addAP("D224", "1qazxsw2");
//  wifiMulti.run(1000);
/*
  while ( wifiMulti.run() != WL_CONNECTED ) 
  {
      _7SegHH(88); _7SegMM(88); delay(500);
      _7SegHH(0);  _7SegMM(0);  delay(500);
  }

    Serial.println("");
    Serial.print("WiFi connected: ");
    Serial.println(WiFi.localIP());
*/
  //updateLocalTime();     // it will take some time to sync time :)

}


void loop()
{
  if( WebTime::isWiFi_connectNeeded() )
  {
    Serial.println("Connecting Wifi...");
    if ( wifiMulti.run() != WL_CONNECTED )
    {
        Serial.print("!WL_CONNECTED\n");
        delay(1000);
    }    
  }
  else  // !isWiFi_connectNeeded
  {
    if ( WiFi.status() == WL_CONNECTED )
    {
      Serial.print("WiFi close\n");
      WiFi.disconnect(true);      
    }  
  }
  delay(1000);
}


