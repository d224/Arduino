
/*
  
   echo  "REQ:DUD GET:T0" | socat - UDP-DATAGRAM:255.255.255.255:8266,broadcast
          RES:DUD T0:12.5 T1:23.8 L0:123
		  
	 curl http://192.168.0.8/L0

   https://github.com/raphaelcohn/bish-bosh

   //C:\Users\teifd\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.3.0\libraries\ArduinoOTA
   
   https://github.com/dzavalishin/mqtt_udp/tree/master/lang/arduino/MQTT_UDP
   
*/
#define DEBUG_ESP_OTA
#define DEBUG_ESP_PORT Serial

#include <ArduinoOTA.h>
#include <DallasTemperature.h>
#include <mqtt_udp.h>
#include "WebServer.h"
#include "BH1750.h"

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Ticker.h>  //Ticker Library

ESP8266WiFiMulti wifiMulti;

const char* host     = "dud";
//const char* ssid     = "D224_2.4G";
//const char* password = "1qazxsw2";


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(13);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress owT0 = {0x28, 0xEB, 0xF3, 0xEB, 0x04, 0x00, 0x00, 0x64}; //DUD
DeviceAddress owT1 = {0x28, 0x28, 0x13, 0x4B, 0x04, 0x00, 0x00, 0xD1}; //Board
DeviceAddress owT2 = {0x28, 0x8D, 0x41, 0x26, 0x00, 0x00, 0x80, 0x6F}; //Water in
DeviceAddress owT3 = {0x28, 0x4D, 0x35, 0x26, 0x00, 0x00, 0x80, 0x82}; //Water out
DeviceAddress owT4 = {0x28, 0x5D, 0x93, 0x04, 0x00, 0x00, 0x80, 0x01}; //Panel


//OneWire  ds(13);  // on pin 13 (a 4.7K resistor is necessary)
BH1750 lightMeter;

//char packetBuffer[255]; //buffer to hold incoming packet
WiFiUDP udp;
 
Ticker ticker;
#define WD_MAX 60
int WD_cnt = WD_MAX;

void _1sec()
{
  WD_cnt --;
  if(WD_cnt <=0 )
    ESP.restart();
}

void setup(void)
{
  Serial.begin(115200);
  while (!Serial && !Serial.available()) {}
  Serial.printf("****Start %s****\n", host);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN",  "1qazxsw2");

  Serial.println("Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED) 
  {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
  }
    
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

  lightMeter.begin(4, 5, BH1750_CONTINUOUS_HIGH_RES_MODE_2);

  // locate devices on the bus
  sensors.begin();
  Serial.printf("Found %d  OneWire devices.\n", sensors.getDeviceCount());

  int success = udp.begin( MQTT_PORT );
  Serial.println( success ? "success" : "failed");

  ReadAll();
  WebServerBegin();
  ESP.wdtDisable();
  ticker.attach(0.5, _1sec);
}

float t0, t1, t2, t3, t4;
uint16_t lux ;
String srtPacket;

//////////////////////////////////////////////////////////////////////////
bool find_text(String haystack, String needle) {
  for (int i = 0; i <= haystack.length() - needle.length(); i++) {
    if (haystack.substring(i, needle.length() + i) == needle) {
      return true;
    }
  }
  return false;
}
//////////////////////////////////////////////////////////////////////////
float ReadTemp(DeviceAddress addr)
{
  float res =  sensors.getTempC(addr);
  if (res > -127)
    printf("T[%02X]=%d.%02d C\n", addr[7], (int)res, (int)(res * 100) % 100);
  else
    printf("Sensor [%02X] error\n", addr[7]);
  return res;
}
//////////////////////////////////////////////////////////////////////////
void ReadAll()
{
  sensors.requestTemperatures();
  t0 = ReadTemp(owT0);
  t1 = ReadTemp(owT1);
  t2 = ReadTemp(owT2);
  t3 = ReadTemp(owT3);
  t4 = ReadTemp(owT4);
  lux = lightMeter.readLightLevel();
}
unsigned int _10sec_i = 0;
//////////////////////////////////////////////////////////////////////////
void ReadNext()
{
  //unsigned long start = millis();


  switch (_10sec_i % 6)
  {
    case 0:
      sensors.setWaitForConversion(false);  // makes it async
      sensors.requestTemperatures();
      sensors.setWaitForConversion(true);
      printf("requestTemperatures\n");
      lux = lightMeter.readLightLevel(); break;
      break;

    case 1: t0 = ReadTemp(owT0); break;
    case 2: t1 = ReadTemp(owT1); break;
    case 3: t2 = ReadTemp(owT2); break;
    case 4: t3 = ReadTemp(owT3); break;
    case 5: t4 = ReadTemp(owT4); break;
  }


  //Serial.print("Time used: ");
  //Serial.println(millis() - start);

}
//////////////////////////////////////////////////////////////////////////
char userResponceBuffer[128]; //buffer to hold Responce packet
char* UDP_resp(char* userPacketBuffer)
{
  srtPacket = String(userPacketBuffer);
  if (srtPacket.startsWith("REQ:DUD "))
  {
    srtPacket.remove(0, 8);
    Serial.printf("My Request: %s", srtPacket.c_str());
    if (srtPacket.startsWith("GET:"))
    {
      srtPacket.remove(0, 4);
      Serial.printf("Requested : %s\n", srtPacket.c_str());
      String strResp = String("RES:DUD");

      if (find_text(srtPacket, "T0")) strResp += " T0:" + String(t0);
      if (find_text(srtPacket, "T1")) strResp += " T1:" + String(t1);
      if (find_text(srtPacket, "T2")) strResp += " T2:" + String(t2);
      if (find_text(srtPacket, "T3")) strResp += " T3:" + String(t3);
      if (find_text(srtPacket, "T4")) strResp += " T4:" + String(t4);
      if (find_text(srtPacket, "L0")) strResp += " L0:" + String(lux);
      strResp += '\n';
      Serial.printf("Responce: \n %s", strResp.c_str());
      strcpy(userResponceBuffer, strResp.c_str());
      return userResponceBuffer;

      // send a reply, to the IP address and port that sent us the packet we received
      //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      //Udp.write(strResp.c_str());
      //Udp.endPacket();

    }
  }
  return NULL;
}
int mqtt_udp_send_pkt( int fd, char *data, size_t len )
{
  int success;
  success = udp.beginPacket(IPAddress(255, 255, 255, 255), MQTT_PORT);
  success = udp.write( data, len );
  success = udp.endPacket();
  udp.stop();
  //Serial.println("sent!");
  return 0;
}

//////////////////////////////////////////////////////////////////////////
unsigned long previousMillis = 0;
void loop(void)
{
  ESP.wdtFeed();
  yield();
  
  if(wifiMulti.run() != WL_CONNECTED)
  {
        Serial.println("WiFi not connected!");
        delay(100);
        return;
  }
  
    ArduinoOTA.handle();
    WebServerHandleClient();
    WD_cnt = WD_MAX;
    if (millis() - previousMillis >= 10 * 1000) // onse a 10 sec
    {
      previousMillis = millis();
      ReadNext();
      String tmp;
      if(_10sec_i % 2 == 0)
      {
        tmp = String(t0);
        mqtt_udp_send( 0, "Dud/Temp", (char*)tmp.c_str() );        
      }
      else
      {
        tmp = String(lux);
        mqtt_udp_send( 0, "Dud/Ligth", (char*)tmp.c_str() );          
      }
      _10sec_i++;
    }
}
