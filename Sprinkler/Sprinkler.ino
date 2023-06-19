/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-thermostat-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/


#include <WiFi.h>
#include <WiFiMulti.h>
#include "WebTime.h"
#include "WebServer.h"
#include "sntp.h"
#include "Sprinkler.h"

#include "UI.h"
#include "motor.h"

#include "EEPROM_data.h"

EEPROM_struct data;

WiFiMulti wifiMulti;
WebTime webTime;

uint32_t timeToSec(String t)
{
  uint32_t r = 0;
  if (t.length() < 8)
    return 0;
  if (t[2] != ':' || t[5] != ':')
    return 0;

  r += (t[0] - '0') * 36000;
  r += (t[1] - '0') * 3600;
  r += (t[3] - '0') * 600;
  r += (t[4] - '0') * 60;
  r += (t[6] - '0') * 10;
  r += (t[7] - '0');
  return r;
}


/*
void connectToWiFi(const char * ssid, const char * pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);

  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      //When connected set
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      //initializes the UDP state
      //This initializes the transfer buffer
      //udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      connected = false;
      break;
    case SYSTEM_EVENT_STA_STOP:
      Serial.println("WiFi stop connection");
      connected = false;
      break;
    default:
      Serial.print("WiFi event: ");
      Serial.println(event);
  }
}
*/



void setup() {

  Serial.begin(115200);

  Serial.println("\n\n Start ... ");

  setup_EEPROM();

  xTaskCreatePinnedToCore( taskMotors,  "taskMotors", 10240, NULL, 0, NULL, 1); 
  xTaskCreatePinnedToCore( taskUI,  "taskUI", 10240, NULL, 0, NULL, 1); 
}

uint32_t timeWD = 0;
void modeSTA()
{
  Serial.print("### modeSTA ### ");
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN", "1qazxsw2");
  wifiMulti.addAP("D224", "1qazxsw2");

  if( wifiMulti.run() == WL_CONNECTED ) 
  {
    Serial.println("");
    Serial.print("WiFi connected: ");
    Serial.println(WiFi.localIP());
  }
  else 
  {
    Serial.print("WiFi !WL_CONNECTED :(");
  }

  webTime.Start();

  for(;;)
  {
    if( WebTime::isWiFi_connectNeeded() )
      {
        Serial.println("WiFi_connect Needed");
        if( WiFi.status() != WL_CONNECTED )
        {
          Serial.println("Connecting Wifi...");
          if ( wifiMulti.run() != WL_CONNECTED )
          {
              Serial.println("!WL_CONNECTED");
              delay(8000);
          }       
        }
      }
      else  // !isWiFi_connectNeeded
      {
        //Serial.println("WiFi_connect not-Needed");
        if ( WiFi.status() == WL_CONNECTED )
        {
          Serial.print("WiFi close\n");
          WiFi.disconnect(true);    
          return;  
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
  }
}

//WiFiServer server(80);
///////////////////////////////////////////////////////////////////////////
void modeAP()
{
  Serial.print("### modeAP ### ");
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_AP);


   // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP("Sprinkler", "1qazxsw2");

  IPAddress IP = WiFi.softAPIP();
  delay(100);

  Serial.println("Set softAPConfig");
  IPAddress Ip(192, 168, 0, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
   
  WebServerBegin();
  for(;;)
  {
      delay(1000);
  }  
}
///////////////////////////////////////////////////////////////////////////
void loop()
{
  modeSTA();
  modeAP();
}

