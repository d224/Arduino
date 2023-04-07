/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-thermostat-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/


#include <WiFi.h>
#include <esp_wifi.h>
#include <Wire.h>
#include "I2CSoilMoistureSensor.h"
#include "wiring_shift_mod.h"
#include <NTPClient.h>
WiFiUDP udp;
NTPClient timeClient(udp);

#include "WebServer.h"
#include "EEPROM_data.h"

EEPROM_struct data;
#define LED_BUILTIN 2

//Are we currently connected?
boolean connected = false;

const char* ssid = "4OBEZYAN";
const char* password = "1QAZXSW2";


//uint16_t nCurrentTime = 0;
#define M_OPEN  true
#define M_CLOSE false
#define NOT_SET 0xFFFFFFFF


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t secCounter ;
volatile uint32_t secCounterPrev;

void set_time(int min) //in min
{
  Serial.printf("set_time to: %d min\n", min);
  start_timer(min * 60);
}

void start_timer(int sec)
{
  Serial.printf("start_timer\n");
  secCounter = sec;
  timerAlarmEnable(timer);
}

void printEEPROM_Data()
{
  Serial.printf("EEPROM_Data:\n");
  Serial.printf("ver 0x%X\n", data.ver);
  for (int i = 0; i < CH_NUM; i++)
    Serial.printf("Ch[%d] active[%d] time[%d] duration[%d]:\n", i,	data.ch[i].active, data.ch[i].time_ON, data.ch[i].duration);
}

void printTime()
{
  if (secCounter != NOT_SET)
  {
    uint8_t h = secCounter / 3600;
    uint8_t m = secCounter % 3600 / 60;
    uint8_t s = secCounter % 60;
    Serial.printf("now %d:%d:%d\n" ,h ,m, s);

    bool dot = s % 2;
    TM1638Digit(0, h / 10);
    TM1638Digit(1, h % 10, dot);
    TM1638Digit(2, m / 10);
    TM1638Digit(3, m % 10);
  }
  else
    Serial.println("Time: N.A.");
}

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

#define P_NAME 0
#define PA 1
#define PB 2
//const uint8_t M1[] = {1,19,18};
//const uint8_t M2[] = {2,17,16};
const uint8_t MOTORS[CH_NUM][3] = {{1, 19, 18}, {2, 17, 16}};

void setup_motors()
{
  pinMode(MOTORS[0][PA], OUTPUT);
  pinMode(MOTORS[0][PB], OUTPUT);
  pinMode(MOTORS[1][PA], OUTPUT);
  pinMode(MOTORS[1][PB], OUTPUT);
}

void set_motor(const uint8_t* m, bool bOnOff)
{
  Serial.print("Motor ");
  Serial.print(m[P_NAME]);

  if (bOnOff)
  {
    digitalWrite(m[PA], HIGH);
    digitalWrite(m[PB], LOW);
    Serial.println("->OPEN");
  }
  else
  {
    digitalWrite(m[PA], LOW);
    digitalWrite(m[PB], HIGH);
    Serial.println("->CLOSE");
  }

  delay(100);
  digitalWrite(m[PA], LOW);
  digitalWrite(m[PB], LOW);
}

void open_valwe(const uint8_t* m, const uint32_t open_time)
{
  if (open_time == 0)
    return;

  TM1638Digit(0, m[P_NAME]);
  TM1638Char(1, ' ');
  TM1638Char(2, 'O');
  TM1638Char(3, 'n');
    
  set_motor(m, true);
  digitalWrite(LED_BUILTIN, HIGH);
  for (int i = open_time; i > 0; i--)
  {
    TM1638Digit(4, i / 1000);
    TM1638Digit(5, i / 100 % 10);
    TM1638Digit(6, i / 10 % 10);
    TM1638Digit(7, i % 10);
    Serial.println(i);
    delay(1000);
  }
  set_motor(m, false);
}

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

void setup() {

  Serial.begin(115200);

  secCounter = NOT_SET;
  secCounterPrev = NOT_SET;

  if (!EEPROM.begin(EEPROM_SIZE))
    Serial.println("failed to initialise EEPROM");
  else
  {
    ReadEEPROM((uint8_t *)&data, sizeof(EEPROM_struct));
    if (data.ver == 0xFFFFFFFF)
    {
      Serial.println("EEPROM empty :(");
      memset((uint8_t *)&data, 0, sizeof(EEPROM_struct));
      data.ch[0].time_ON = 19 * 60 + 11;
      data.ch[0].duration = 1;
      data.ch[0].active = true;
    }

    printEEPROM_Data();
    data.ver = 1;
  }
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  setup_motors();

  delay(3000);
  set_motor(MOTORS[0], M_CLOSE);
  delay(100);
  set_motor(MOTORS[1], M_CLOSE);
  delay(100);

  connectToWiFi("D224_2.4G", "1qazxsw2");
  WebServerBegin();

  digitalWrite(LED_BUILTIN, LOW);

  timeClient.begin();
  timeClient.setTimeOffset(+3600 * 3);
  
  TM1638reset();
}

void secCounterUpdate()
{
  secCounter = timeClient.getHours() * 3600 + timeClient.getMinutes() * 60 + timeClient.getSeconds();
}


void loop()
{

  if (connected && ((secCounter == NOT_SET) || (secCounter > 3600 * 24)) )
  {
    if (!timeClient.update())
    {
      timeClient.forceUpdate();
    }
    else
    {
      timeClient.getFormattedTime();
      Serial.println("start_timer from NTPClient");
      secCounterUpdate();
      //start_timer(secCounter);
    }
  }

  if (secCounter == NOT_SET)
  {
    digitalWrite(LED_BUILTIN, HIGH);   delay(100);
    digitalWrite(LED_BUILTIN, LOW);    delay(100);
    return;
  }

  secCounterUpdate();
  if (secCounterPrev != secCounter) // new sec
  {
    printTime();
    secCounterPrev = secCounter;
    digitalWrite(LED_BUILTIN, (secCounter % 2) == 0);
    for (int i = 0; i < CH_NUM; i++)
    {
      if (data.ch[i].active)
      {
        uint32_t time_ON = data.ch[i].time_ON * 60;
        if (secCounter == time_ON)
        {
          open_valwe(MOTORS[i], data.ch[i].duration * 60);
          return;
        }
      }
    }
  }

  delay(100);

}
