/*
  DOIT ESP32 DEVKIT V1
   echo  "REQ:DUD GET:T0" | socat - UDP-DATAGRAM:255.255.255.255:8266,broadcast
          RES:DUD T0:12.5 T1:23.8 L0:123
	 curl http://192.168.0.8/L0
   https://github.com/raphaelcohn/bish-bosh
   //C:\Users\teifd\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.3.0\libraries\ArduinoOTA
   https://github.com/dzavalishin/mqtt_udp/tree/master/lang/arduino/MQTT_UDP
*/
#define DEBUG_ESP_OTA
#define DEBUG_ESP_PORT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "mqtt_udp.h"
#include "WebServer.h"
#include <Adafruit_NeoPixel.h>
#include <ezButton.h>
//#include <ACS712.h>

#define PW_SW LED_BUILTIN

//TaskHandle_t NotificationTask;
//TaskHandle_t ButtonTask;

WiFiMulti wifiMulti;
ezButton button( 13 );
WiFiUDP udp;
Adafruit_NeoPixel rgb0 (1, 15, NEO_GRB + NEO_KHZ800 );
//  ACS712 5A  uses 185 mV per A
//  ACS712 20A uses 100 mV per A
//  ACS712 30A uses  66 mV per A
//ACS712  ACS(25, 3.3, 4095, 100);   // 20A

bool nState = 0;
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

int GetSSR()
{
 //return digitalRead( PW_SW ); 
  return nState; 
}

void SetSSR(uint8_t val)
{
 //digitalWrite( PW_SW, val ); 
  nState = val; 
}

String GetPowerStr()
{
 if( bCurrent )  
    return "Porwer: " + String( float( nCurrent_mA / 100.0 ), 1 ) + " A";
  else
    return "";  
}

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

void acsTask( void * parameter) 
{
  for(;;)
  {
    //nCurrent_mA = ACS.mA_AC();
    Serial.print("mA: ");
    Serial.println( nCurrent_mA );
//  Serial.print(". Form factor: ");
//  Serial.println(ACS.getFormFactor());
    delay (1000 );
  }
}

void buttonTask( void * parameter) 
{
  button.setDebounceTime( 50 );
  for(;;)
  {
    button.loop();

    if(button.isReleased())
    {
      nState = !nState;
      Serial.print( "button: PW " );
      Serial.println( nState );
    }
    
    delay(100);
  }
}

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


void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  bConnected = true;
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  bConnected = false;
  //Serial.println("Trying to Reconnect");
  //WiFi.begin(ssid, password);
}


void setup(void) 
{
  RGB_Begin();

  Serial.begin(115200);
  while (!Serial && !Serial.available()) {}
  Serial.printf("****Start %s****\n", host);

  WiFi.disconnect(true);
  delay(1000);

  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);


  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN", "1qazxsw2");
  wifiMulti.addAP("D224", "1qazxsw2");


  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
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
  Serial.println(udp.begin(MQTT_PORT) ? "MQTT_PORT success" : "MQTT_PORT failed");
  WebServerSetup();

  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PW_SW, OUTPUT);

  xTaskCreatePinnedToCore( notificationTask,  "notificationTask", 10240, NULL, 0, NULL, 1);        
  xTaskCreatePinnedToCore( buttonTask, "buttonTask", 10240,  NULL, 0, NULL, 1);         
  xTaskCreatePinnedToCore( acsTask, "acsTask", 10240, NULL, 0, NULL, 1);        
}

String srtPacket;
//////////////////////////////////////////////////////////////////////////
int mqtt_udp_send_pkt(int fd, char* data, size_t len) {
  int success;
  success = udp.beginPacket(IPAddress(255, 255, 255, 255), MQTT_PORT);
  success = udp.write((uint8_t*)data, len);
  success = udp.endPacket();
  udp.stop();
  //Serial.println("sent!");
  udp.begin(MQTT_PORT);
  return 0;
}
void UDPMQTTHandler() {
  int pktSize = udp.parsePacket();
  if (pktSize) {
    byte pktBuf[pktSize];
    udp.read(pktBuf, pktSize);
    udp.stop();
    udp.begin(MQTT_PORT);
    int topic_len = pktBuf[3];
    int val_len = pktSize - topic_len - 4;
    String sBuf = String((char*)pktBuf + 4);
    String topic = sBuf.substring(0, topic_len);
    String val = sBuf.substring(topic_len, topic_len + val_len);
    Serial.printf("%s=%s\n", topic.c_str(), val.c_str());

    if (topic == String("Dud/SetPower")) 
    {
      bool nStatePrev = nState;
      nState = val.toInt();
      if (nStatePrev != nState) 
      {
        Serial.print( "UDPMQTTHandler: PW " );
        Serial.println( nState );
      }
    }
  }
}
///////////////////////////////////////////////////MQTT_PORT success///////////////////////
unsigned long previousMillis = millis();
int di = 0;

void loop(void) {
  //ESP.wdtFeed();
  yield();

  if ( wifiMulti.run() != WL_CONNECTED )
  {
    bConnected = false;
    Serial.print("WiFi disconnected ");
    Serial.println( di++ );
    delay(1000);
    if ( di > 30 )
      ESP.restart();    
  } 

  if( bConnected )
  {   
    di = 0;
    ArduinoOTA.handle();
    UDPMQTTHandler();

    unsigned long currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
    if (currentMillis - previousMillis >= 1000)  //test whether the period has elapsed
    {
      previousMillis = currentMillis;
      // onse a 1 sec
      String tmp = String( nState );
      mqtt_udp_send(0, "Dud/Power", (char*)tmp.c_str());
      //Serial.print("mqtt_udp_send Dud/Power:");
      //Serial.println(tmp);
    }
  }
}