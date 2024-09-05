#define DEBUG_ESP_OTA
#define DEBUG_ESP_PORT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "WebTime.h"
#include "sntp.h"
#include "Clock.h"



bool bConnected = false;
const char* host = "M_Clock";

WiFiMulti wifiMulti;
WebTime webTime;

void setup(void) 
{
  Serial.begin(115200);
  delay(3000);
  Serial.printf("****Start ****\n");
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.disconnect(true);
  delay(1000);

  webTime.Start();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN", "1qazxsw2");
  wifiMulti.addAP("D224", "1qazxsw2");

  xTaskCreatePinnedToCore( ClockTask,  "ClockTask", 10240, NULL, 0, NULL, 1); 

  while ( wifiMulti.run() != WL_CONNECTED ) 
  {
      Serial.print("."); 
      delay(500);
  }

  Serial.println("");
  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    ESP.restart();
  });

  ArduinoOTA.begin();
  
}

uint32_t timeWD = 0;
void loop(void) 
{
  
  if( WebTime::isWiFi_connectNeeded() )
  {
    if( WiFi.status() != WL_CONNECTED )
    {
      Serial.println("Connecting Wifi...");
      if ( wifiMulti.run() != WL_CONNECTED )
      {
          Serial.print("!WL_CONNECTED\n");
          delay(8000);
      }       
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

  if( WebTime::isValid())
  {
    timeWD = 0;
  }
  else 
  {
    timeWD ++;
    if( timeWD > 600)
    {
        Serial.print("Restart - mo time availible\n");
        ESP.restart();
    }
  }
  
  delay(1000);
  /*
  DoSteps( STEPS_PerMM );
  delay(10000);

    DoSteps( STEPS_PerRotation * 12 );
    stepper.setDirection( DIR_BACKWARD );
    delay(5000);
    DoSteps( STEPS_PerRotation * 12);
    stepper.setDirection( DIR_FORWARD  );
    delay(5000);
    */
}