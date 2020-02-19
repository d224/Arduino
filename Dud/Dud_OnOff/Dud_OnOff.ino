
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
#include <mqtt_udp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;
bool nState = 0;
bool nStatePrev = 0;

const char* host     = "Dud_OnOff";
#define PW_SW D1

WiFiUDP udp;
void setup(void)
{
  Serial.begin(115200);
  while (!Serial && !Serial.available()) {}
  Serial.printf("****Start %s****\n", host);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN",  "1qazxsw2");

  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED)
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

  int success = udp.begin( MQTT_PORT );
  Serial.println( success ? "success" : "failed");

  //WebServerBegin();
  //ESP.wdtDisable();
  //ticker.attach(0.5, _1sec);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PW_SW, OUTPUT);
}

String srtPacket;
//////////////////////////////////////////////////////////////////////////
int mqtt_udp_send_pkt( int fd, char *data, size_t len )
{
  int success;
  success = udp.beginPacket(IPAddress(255, 255, 255, 255), MQTT_PORT);
  success = udp.write( data, len );
  success = udp.endPacket();
  udp.stop();
  //Serial.println("sent!");
  udp.begin( MQTT_PORT );
  return 0;
}
void UDPMQTTHandler()
{

  int pktSize = udp.parsePacket();
  if (pktSize)
  {
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
      nState = val.toInt();
      if (nStatePrev != nState)
      {
        nStatePrev = nState;
        Serial.print("nState changed to [");
        Serial.print(nState);
        Serial.println("]");
      }
    }
  }

}
//////////////////////////////////////////////////////////////////////////
unsigned long previousMillis = millis();
void loop(void)
{
  //ESP.wdtFeed();
  yield();

  if (nState)
  {
	// ON
    digitalWrite(LED_BUILTIN, LOW);
	digitalWrite(PW_SW, HIGH);
  }
  else
  {
	// OFF
    digitalWrite(LED_BUILTIN, HIGH);
	digitalWrite(PW_SW, LOW);
  }

  if (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected!");
    delay(1000);
    //return;
  }
  else
  {
    ArduinoOTA.handle();
    UDPMQTTHandler();
    if (millis() - previousMillis >= 1 * 1000) // onse a 1 sec
    {
      previousMillis = millis();
      String tmp;
      tmp = String(nState);
      mqtt_udp_send( 0, "Dud/Power", (char*)tmp.c_str() );
      //Serial.print("mqtt_udp_send Dud/Power:");
      //Serial.println(tmp);
    }
  }
}
