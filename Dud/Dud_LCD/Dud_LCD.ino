#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <NeoPixelBus.h>
#include "mqtt_udp.h"       //https://github.com/dzavalishin/mqtt_udp
#include <PubSubClient.h>   //https://pubsubclient.knolleary.net/
#include <ArduinoOTA.h>

#include "WiFiHelper.h"
#include "WebTime.h"

#include <SH1106Wire.h>
#include "OLEDDisplayUi.h"

#include <JsonListener.h> //https://github.com/squix78/json-streaming-parser
//#include "WundergroundConditions.h"
//#include "WundergroundClient.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

#include "icons.h"

#include "My_openweathermap.h"
#include "Adafruit_MPR121.h"

#define OTA_host "DUD_LCD"

#define mqtt_server "m20.cloudmqtt.com"
#define mqtt_user "teozkocw"
#define mqtt_password "xcUiLSoAc024"
#define mqtt_port 11487

#define topic_SetPower  "Dud/SetPower"
#define topic_Power     "Dud/Power"
#define topic_Temp      "Dud/Temp"
#define topic_TempDest  "Dud/TempDest"
#define topic_Ligth     "Dud/Ligth"

/************ Global State (you don't need to change this!) ******************/
WiFiClient WiFi_client;
PubSubClient mqtt_client(WiFi_client);

////////////////////////////////////////////////////////////////////////////////////
// HW
NeoPixelBus<NeoRgbwFeature, NeoEsp8266BitBang800KbpsMethod> led(1, D8);
Adafruit_MPR121 kbd = Adafruit_MPR121();
SH1106Wire display(0x3c, D2, D1);  //SDC_PIN = D1; SDA_PIN = D2; I2C_DISPLAY_ADDRESS = 0x3c;
////////////////////////////////////////////////////////////////////////////////////

extern void drawTemp(int16_t T, int16_t x, int16_t y);

#define LED_YELOW RgbColor(128, 128, 0)
#define LED_RED RgbColor(0, 128, 0)
#define LED_BLUE RgbColor(0, 0, 128)
#define LED_GREEN RgbColor(128, 0, 0)
#define LED_OFF RgbColor(0, 0, 0)

#define MIN_TEMP 30
#define DEFAULT_DEST_TEMP 50
#define ABSOLUTE_MAX_TEMP 70
#define TIMEOUT_SEC 60

bool bMainON = false;
bool bMainON_req = false;
bool bUpdateUI = false;
bool bUpdate_mqtt = false;
bool bDisplayON = true;

String strDudTemp;
float fDudTemp = 0;
float fPrevDudTemp = 100;
unsigned int nDudTemp = 0;
unsigned int nDudDestTemp = DEFAULT_DEST_TEMP;
unsigned int nDudLigth = 0;
unsigned int nConnectionTempTimeout = 0;
unsigned int nConnectionPowTimeout = 0;
unsigned int nKeyboadTouchedSec = 0;

const char* ssid = "4OBEZYAN";
const char* password = "1qazxsw2";

WiFiUDP udp;
//unsigned int localUdpPort = 4210;  // local port to listen on
unsigned int localUdpPort = MQTT_PORT;
char incomingPacket[255];  // buffer for incoming packets

// Wunderground Settings
const String WUNDERGRROUND_API_KEY = "a2d39a9018f5c170";
const String WUNDERGRROUND_LANGUAGE = "EN";
const String WUNDERGROUND_COUNTRY = "IL";
const String WUNDERGROUND_CITY = "Nesher";
//WundergroundClient wunderground(true);// Set to false, if you prefere imperial/inches, Fahrenheit
//WundergroundConditions wunderground(true);

//http://api.openweathermap.org/data/2.5/weather?q=Nesher,IL&APPID=cec919e38cfe8397b302c610c7d85232&units=metric
//&mode=xml

/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  led.Begin();
  led.ClearTo(LED_OFF);
  led.Show();
  led.ClearTo(LED_OFF);
  led.Show();
  led.ClearTo(LED_RED);
  led.Show();
  led.ClearTo(LED_RED);
  led.Show();
  delay(1000);
  led.ClearTo(LED_GREEN);
  led.Show();
  led.ClearTo(LED_GREEN);
  led.Show();
  delay(1000);
  led.ClearTo(LED_BLUE);
  led.Show();
  led.ClearTo(LED_BLUE);
  led.Show();
  delay(1000);


  // initialize dispaly
  display.init();
  display.clear();
  display.display();

  display.flipScreenVertically();
  display.setContrast(255);

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Connecting to " + String(ssid));
  display.display();
  WiFiConnect(ssid, password);
  display.clear();
  display.drawString(0, 0, "Connected to " + String(ssid));
  display.drawString(0, 12, WiFi.localIP().toString());
  display.display();

  if (!kbd.begin(0x5A))
    Serial.println("MPR121 not found, check wiring?");
  else
    Serial.println("MPR121 found!");

  //#define MPR121_TOUCH_THRESHOLD_DEFAULT 12  ///< default touch threshold value
  //#define MPR121_RELEASE_THRESHOLD_DEFAULT 6 ///< default relese threshold value
  //kbd.setThresholds(12, 12);

  udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  display.drawString(0, 24, "Udp port " + String(localUdpPort));
  display.display();

  WebTime.Start(ssid, password);
  display.drawString(0, 36, "WebTime - OK");
  display.display();

  UpdateWeather();
  display.drawString(0, 60, "Weather - OK");
  display.display();

  ArduinoOTA.setHostname(OTA_host);
  ArduinoOTA.onStart([]() {
    DEBUG("OTAStart\n");
  });
  ArduinoOTA.onEnd([]() {
    DEBUG("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DEBUG("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DEBUG("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DEBUG("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DEBUG("Receive Failed");
    else if (error == OTA_END_ERROR) DEBUG("End Failed");
    ESP.restart();
  });
  ArduinoOTA.begin();

  WebTime.onNewSec(NewSecHandler);
  WebTime.onNewMin(NewMinHandler);
  WebTime.onNewHour(NewHourHandler);
  WebTime.onNewDay(NewDayHandler);

  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(mqtt_callback);
  mqtt_reconnect();

  led.ClearTo(LED_OFF);
  led.Show();
  led.ClearTo(LED_OFF);
  led.Show();
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void loop() {
  //UDPHandler();
  UDPMQTTHandler();
  KeyboadHandler();
  WebTime.UpdateTask();
  mqtt_client.loop();
  ArduinoOTA.handle();
  telnet_handle();
  drawMain();
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void mqtt_reconnect() {
  DEBUG("mqtt_client before rc=%d\n", mqtt_client.state());

  if (!mqtt_client.connected()) {
    DEBUG("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      DEBUG("connected\n");
      mqtt_client.subscribe(topic_SetPower);
      mqtt_client.subscribe(topic_TempDest);
    } else {
      DEBUG("failed, rc=%d\n", mqtt_client.state());
    }
  }

  DEBUG("mqtt_client after rc=%d\n", mqtt_client.state());
}
/////////////////////////////////////////////////////////////////////
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  DEBUG("Message arrived [%s] ", topic);
  for (int i = 0; i < length; i++) {
    DEBUG("%c", (char)payload[i]);
  }
  DEBUG("\n");

  if (String(topic) == String(topic_SetPower)) {
    bMainON_req = payload[0] - '0';
    DEBUG("SetPower [%d]\n", bMainON_req);
    bUpdateUI = true;
    bDisplayON = true;
    SetLed();
  }
  if (String(topic) == String(topic_TempDest)) {
    nDudDestTemp = atoi((char*)payload);
    if (nDudDestTemp > ABSOLUTE_MAX_TEMP)
      nDudDestTemp = ABSOLUTE_MAX_TEMP;
    if (nDudDestTemp < MIN_TEMP)
      nDudDestTemp = MIN_TEMP;
  }
}
/////////////////////////////////////////////////////////////////////
int mqtt_udp_send_pkt(int fd, char* data, size_t len) {
  int success;
  success = udp.beginPacket(IPAddress(255, 255, 255, 255), MQTT_PORT);
  success = udp.write(data, len);
  success = udp.endPacket();
  udp.stop();
  //Serial.println("sent!");
  udp.begin(MQTT_PORT);
  return 0;
}
/////////////////////////////////////////////////////////////////////
void NewHourHandler() {
  DEBUG("NewHourHandler [%d]\n", WebTime._HH);
  if (WebTime._HH == 17) {
    DEBUG("NewHourHandler check for smart power\n");
    if (nConnectionTempTimeout > 0) {
      if (nDudTemp < MIN_TEMP) {
        DEBUG("Low Temp [%d]=> ON\n", nDudTemp);
        bMainON_req = true;
      }
    }
  }

  if (WebTime._HH < 12)
    bDisplayON = false;

  UpdateWeather();
}
/////////////////////////////////////////////////////////////////////
void NewMinHandler()  // once a Minute
{
  DEBUG("NewMinHandler %d:%d\n", WebTime._HH, WebTime._MM);

  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }

  if (WebTime._MM % 10 == 0)  // once a 10 min
  {
    fPrevDudTemp = fDudTemp;
  }

  if (nDudTemp > nDudDestTemp) {
    DEBUG("Higth Temp => OFF\n");
    bMainON_req = false;
  }
}
/////////////////////////////////////////////////////////////////////
void NewSecHandler() {

  String tmp;
  if (nConnectionTempTimeout > 0)
    nConnectionTempTimeout--;

  if (nConnectionPowTimeout > 1) {
    nConnectionPowTimeout--;
  } else  // nConnectionPowTimeout <= 1
  {
    if (nConnectionPowTimeout == 1) {
      nConnectionPowTimeout = 0;
      SetLed();
    }
  }

  nKeyboadTouchedSec++;
  if (nKeyboadTouchedSec > 600 && !bMainON)
    bDisplayON = false;

  if (bMainON_req != bMainON) {
    tmp = String(bMainON_req);
    mqtt_udp_send(0, (char*)"Dud/SetPower", (char*)tmp.c_str());
  }

  if (bUpdate_mqtt) {
    if (mqtt_client.connected()) {
      tmp = String(nDudTemp);
      mqtt_client.publish(topic_Temp, (char*)tmp.c_str(), true);
      tmp = String(bMainON);
      mqtt_client.publish(topic_Power, (char*)tmp.c_str(), true);
      bUpdate_mqtt = false;
      DEBUG("Update_mqtt OK\n");
    } else {
      DEBUG("Update_mqtt FAIL\n");
    }
  }
  bUpdateUI = true;
}
/////////////////////////////////////////////////////////////////////
void NewDayHandler() {
  DEBUG("NewDayHandler\n");
}
/////////////////////////////////////////////////////////////////////
void drawMain() {
  static bool bDisplayON_prev = true;
  if (!bUpdateUI)
    return;

  if (bDisplayON_prev != bDisplayON) {
    bDisplayON_prev = bDisplayON;
    if (bDisplayON)
      display.displayOn();
    else
      display.displayOff();
  }

  bUpdateUI = false;
  //Serial.printf("drawMain\n");
  display.clear();
  static int frame = 0;

  //display.drawRect(26, 0, 32, 48);
  if (bMainON && nConnectionPowTimeout > 0) {
    if (frame % 3 == 0)
      display.drawXbm(10, 0, 32, 48, ico_dud0);
    if ((frame - 1) % 3 == 0)
      display.drawXbm(10, 0, 32, 48, ico_dud1);
    if ((frame - 2) % 3 == 0)
      display.drawXbm(10, 0, 32, 48, ico_dud2);
    frame++;

    display.drawString(45, 2, String(nDudDestTemp));
  } else {
    //String weather = ""; //wunderground.getTodayIconText();
    String weather = weatherData.weather_text;

    bool bSun_rise_set = false;
    if ((WebTime.TimeFromSunrise() < 60) || (WebTime.TimeBeforeSunset() < 60))
      bSun_rise_set = true;

    //Serial.printf("weather - %s\n", weather.c_str());
    String weatherIcon = getMeteoconIcon(weatherData.icon);
    int weatherIconWidth = display.getStringWidth(weatherIcon);
    bool isSunny = (weather == "sunny") || (weather == "clear");

    //DEBUG("drawMain %s %s\n", weather.c_str(), weatherIcon.c_str());
    if (bSun_rise_set && isSunny) {
      display.setFont(Meteocons_Plain_42);
      display.drawString(0, 0, "A");
    } else {
      if (isSunny && ((fDudTemp - fPrevDudTemp) > 0.5)) {
        display.drawXbm(24, 0, 32, 48, ico_panel0);
        display.setFont(Meteocons_Plain_21);
        display.drawString(0, 0, weatherIcon);
      } else {
        display.setFont(Meteocons_Plain_42);
        display.drawString(0, 0, weatherIcon);
      }
    }
  }
  if (nConnectionTempTimeout > 0)
    drawTemp(nDudTemp, 69, 5);
  else
    drawTemp(0, 69, 5);


  display.drawHorizontalLine(0, 52, 128);
  drawHeaderOverlay();
  display.display();
}
/////////////////////////////////////////////////////////////////////
void drawHeaderOverlay() {

  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 54, String(WebTime._strTime));

  String DudLigth = String(nDudLigth) + " Lux   " + weatherData.weather_text;
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 54, DudLigth);
}
/////////////////////////////////////////////////////////////////////
void UpdateWeather() {
  update_weather();
}
/////////////////////////////////////////////////////////////////////
void KeyboadHandler() {
  static uint16_t lasttouched = 0;
  static uint16_t currtouched = 0;
  static unsigned long touched_time;
  currtouched = kbd.touched();

  // it if *is* touched and *wasnt* touched before, alert!
  if ((currtouched & _BV(0)) && !(lasttouched & _BV(0))) {
    touched_time = millis();
    //led.ClearTo(LED_OFF); led.Show();
    DEBUG("touched\n");
  }
  // if it *was* touched and now *isnt*, alert!
  if (!(currtouched & _BV(0)) && (lasttouched & _BV(0))) {
    unsigned long delta = millis() - touched_time;
    if (delta < 100) {
      DEBUG("released debounce %d\n", delta);
    } else {
      DEBUG("released\n");
      nKeyboadTouchedSec = 0;
      if (bDisplayON) {
        bMainON_req = !bMainON_req;
        bUpdateUI = true;
        SetLed();
      } else {
        bDisplayON = true;
      }
    }
  }
  // reset our state
  lasttouched = currtouched;
}
/////////////////////////////////////////////////////////////////////
void SetLed() {
  if (nConnectionPowTimeout > 0) {
    if (bMainON != bMainON_req) {
      led.ClearTo(LED_GREEN);
      led.Show();
      led.ClearTo(LED_GREEN);
      DEBUG("LED_GREEN\n");
    } else {
      if (bMainON == true) {
        led.ClearTo(LED_RED);
        led.Show();
        led.ClearTo(LED_RED);
        DEBUG("LED_RED\n");
      } else {
        led.ClearTo(LED_BLUE);
        led.Show();
        led.ClearTo(LED_BLUE);
        DEBUG("LED_BLUE\n");
      }
    }
  } else {
    led.ClearTo(LED_OFF);
    led.Show();
    led.ClearTo(LED_OFF);
    DEBUG("LED_OFF\n");
  }
  led.Show();
}
/////////////////////////////////////////////////////////////////////
void UDPMQTTHandler() {
  static int nPrevDudTemp = 0;

  int pktSize = udp.parsePacket();
  if (pktSize) {
    byte pktBuf[pktSize];
    //Serial.print(udp.remoteIP(),udp.remotePort());
    udp.read(pktBuf, pktSize);
    udp.stop();
    udp.begin(MQTT_PORT);
    int topic_len = pktBuf[3];
    int val_len = pktSize - topic_len - 4;
    String sBuf = String((char*)pktBuf + 4);
    String topic = sBuf.substring(0, topic_len);
    String val = sBuf.substring(topic_len, topic_len + val_len);
    //DEBUG("%s=%s\n", topic.c_str(), val.c_str());

    if (topic == String(topic_Temp)) {
      fDudTemp = val.toFloat();
      if (fPrevDudTemp == 100)  // TODO change
        fPrevDudTemp = fDudTemp;

      nDudTemp = round(fDudTemp);
      if (nPrevDudTemp != nDudTemp) {
        nPrevDudTemp = nDudTemp;
        bUpdateUI = true;
        bUpdate_mqtt = true;
        DEBUG("%s=%s\n", topic.c_str(), val.c_str());
      }
      nConnectionTempTimeout = TIMEOUT_SEC;
    }

    if (topic == String(topic_Power)) {
      bool bNewVal = val.toInt();
      if (bMainON != bNewVal) {
        bMainON = bNewVal;
        bUpdateUI = true;
        bUpdate_mqtt = true;
        DEBUG("%s=%s\n", topic.c_str(), val.c_str());
        SetLed();
      }
      if (nConnectionPowTimeout == 0) {
        nConnectionPowTimeout = TIMEOUT_SEC;
        SetLed();
      }
      nConnectionPowTimeout = TIMEOUT_SEC;
    }

    if (topic == String(topic_SetPower)) {
      bool bNewVal = val.toInt();
      if (bMainON_req != bNewVal) {
        bMainON_req = bNewVal;
        bUpdateUI = true;
        bUpdate_mqtt = true;
        DEBUG("%s=%s\n", topic.c_str(), val.c_str());
        SetLed();
      }
    }

    if (topic == String(topic_Ligth)) {
      nDudLigth = val.toInt();
    }
  }
}
/////////////////////////////////////////////////////////////////////
