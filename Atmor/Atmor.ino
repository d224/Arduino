#include <OneWire.h>
#include <DallasTemperature.h>
//#include "BluetoothSerial.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//const char* ssid = "D224_2.4G";
const char* ssid = "4OBEZYAN";
const char* password = "1qazxsw2";

//BluetoothSerial SerialBT;

volatile int PulseWaterCount;
volatile float  TempWaterIN = 0;
volatile float  TempWaterOUT = 0;
volatile float  TempWaterDUD = 0;
volatile float  TempWaterDest = 39.0;
volatile float  WaterFlow;
#define MAX_POSIBLE_DELTA 12.0 //Max heating possible at 5kw

volatile byte pow1 = 0; // 0 to 4
volatile byte pow2 = 0; // 0 to 4
volatile byte power = 0;  // 0 to 9, 0->off 9->5KW

volatile int preHeatCnt = 0;
volatile int postHeatCnt = 0;

#define LED_BUILTIN  2
#define interruptPin  34
#define ONE_WIRE_BUS 15
#define OUT_PORT_3 13
#define OUT_PORT_2 14
#define OUT_PORT_1 27
#define OUT_PORT_0 26
#define PRE_HEAT_SET 120
#define POST_HEAT_SET 120

#define P2KW OUT_PORT_0
#define P3KW OUT_PORT_1

volatile int qSec = 0; // 1/4 sec counter
bool bNewQSec = false;
bool isWaterFlow = false;
bool isWiFiInitDone = false;
bool isOTAinProgress = false;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiServer telnetServer(23); // --> default port for communication usign TELNET protocol | Server Instance
WiFiClient telnetClient; // --> Client Instanse

volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

enum STATE {
  _IDLE,
  _PRE_HEAT,
  _HEAT,
  _POST_HEAT
} eState = _IDLE;

uint secondI = 0; // seconds counter

//1->25% 2->50% 3->75% 4->100%
static const byte powTbl[][2] = {
  {0, 0}, // 0.0
  {1, 0}, // 0.5
  {2, 0}, // 1.0
  {3, 0}, // 1.5
  {4, 0}, // 2.0
  {2, 2}, // 2.5
  {0, 4}, // 3.0
  {1, 4}, // 3.5
  {2, 4}, // 4.0
  {3, 4}, // 4.5
  {4, 4}  // 5.0
};


///////////////////////////////////////////////////////////////////
void IRAM_ATTR onTimer()
{
  //portENTER_CRITICAL_ISR(&timerMux);
  bNewQSec = true;
  //portEXIT_CRITICAL_ISR(&timerMux);
}
///////////////////////////////////////////////////////////////////
void IRQcounter()
{
  PulseWaterCount++;
}
///////////////////////////////////////////////////////////////////
void setup_timer()
{
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 250000, true); //1000000 = 1 sec
  timerAlarmEnable(timer);
}
void setup_io()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(OUT_PORT_0, OUTPUT);
  pinMode(OUT_PORT_1, OUTPUT);
  pinMode(OUT_PORT_2, OUTPUT);
  pinMode(OUT_PORT_3, OUTPUT);
  digitalWrite(OUT_PORT_0, HIGH);
  digitalWrite(OUT_PORT_1, HIGH);
  digitalWrite(OUT_PORT_2, HIGH);
  digitalWrite(OUT_PORT_3, HIGH);
}
void setup_wifi()
{
  Serial.println("setup_wifi ...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(3000);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed!");
    WiFi.setAutoReconnect(false);
    WiFi.disconnect();
    return;
    //delay(5000);
    //Serial.println(" Rebooting...");
    //ESP.restart();
  }
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  WiFi.setAutoReconnect(true);
  ArduinoOTA.setHostname("Atmor");

  // No authentication by default
  // ArduinoOTA.setPassword("admin")
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
  .onStart([]() {
    isOTAinProgress = true;
    telnetServer.close();

    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    isOTAinProgress = false;
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    isOTAinProgress = false;
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setup_telnet();
  isWiFiInitDone = true;
}
void setup_telnet() {

  telnetServer.begin();
  telnetServer.setNoDelay(true); // --> Won't be storing data into buffer and wait for the ack. rather send the next data and in case nack is received, it will resend the whole data
  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 23' to connect");
}
///////////////////////////////////////////////////////////////////
size_t DEBUG(const char *format, ...)
{
  char loc_buf[64];
  char * temp = loc_buf;
  va_list arg;
  va_list copy;
  va_start(arg, format);
  va_copy(copy, arg);
  size_t len = vsnprintf(NULL, 0, format, arg);
  va_end(copy);
  if (len >= sizeof(loc_buf)) {
    temp = new char[len + 1];
    if (temp == NULL) {
      return 0;
    }
  }
  len = vsnprintf(temp, len + 1, format, arg);
  Serial.write((uint8_t*)temp, len);
  if (telnetClient && telnetClient.connected())
    telnetClient.write((uint8_t*)temp, len);

  va_end(arg);
  if (len > 64) {
    delete[] temp;
  }
  return len;
}
///////////////////////////////////////////////////////////////////
void telnet_handle()
{
  //check if there are any new clients
  if (telnetServer.hasClient())
  {
    if (telnetClient)
      telnetClient.stop();
    telnetClient = telnetServer.available();
    DEBUG("New Telnet Client connected\n");
    DEBUG(" WIn > WOut | Flow| Power\n");
  }

  //check client for data
  if (telnetClient && telnetClient.connected()) {
    if (telnetClient.available())
    {
      //get data from the telnet client and push it to the UART
      while (telnetClient.available())
      {
        char ch = telnetClient.read();
        if (ch < 127 && ch >= ' ')
          Serial.write(ch);
      }
    }
  }
}
///////////////////////////////////////////////////////////////////
void readAllTemp()
{
  //sensors.requestTemperatures(); // Send the command to get temperatures
  float t;
  t = sensors.getTempCByIndex(1);
  if (t > 0)
    TempWaterIN = t;
  t = sensors.getTempCByIndex(0);
  if (t > 0)
    TempWaterOUT = t;
  sensors.requestTemperatures(); // Send the command to get temperatures
}
/*
  void readTempWaterIN()
  {
  sensors.requestTemperaturesByIndex(1);
  TempWaterIN = sensors.getTempCByIndex(1);
  if(TempWaterIN < 0)
    TempWaterIN = 0;
  }
  void readTempWaterOUT()
  {
  sensors.requestTemperaturesByIndex(0);
  TempWaterOUT = sensors.getTempCByIndex(0);
  if(TempWaterOUT < 0)
    TempWaterOUT = 0;
  }
*/
///////////////////////////////////////////////////////////////////
byte CalcPow()
{
  int nNewPow = power;
  if (TempWaterDUD > TempWaterDest)
  {
    return 0;
  }

  if (TempWaterIN < (TempWaterDest - MAX_POSIBLE_DELTA))
    nNewPow = 10;

  float diff =  TempWaterDest - TempWaterOUT;

  if (diff > 1.0)
    nNewPow++;
  if (diff > 5.0)
    nNewPow++;

  if (diff < -1.0)
    nNewPow--;
  if (diff < -5.0)
    nNewPow--;

  if (nNewPow > 10)
    nNewPow = 10;

  if (nNewPow < 0)
    nNewPow = 0;

  return nNewPow;
}
///////////////////////////////////////////////////////////////////
void SetPow(byte nNewPow)
{
  if (nNewPow > 10)
    nNewPow = 10;

  power = nNewPow;
  pow1 = powTbl[power][0];
  pow2 = powTbl[power][1];
}
///////////////////////////////////////////////////////////////////
void SetPowerOff()
{
  power = 0;
  pow1 = 0;
  pow2 = 0;
}
///////////////////////////////////////////////////////////////////
void setKW(int nPin, boolean bOn)
{
  if (bOn)
    digitalWrite(nPin, LOW);
  else
    digitalWrite(nPin, HIGH);
}
///////////////////////////////////////////////////////////////////
void PWM_5KW(byte pwm) // 1/4 sec
{
  if (pwm == 4)
    pwm = 0;

  if (pow1 > pwm)
    setKW(P2KW, true);
  else
    setKW(P2KW, false);

  if (pow2 > pwm)
    setKW(P3KW, true);
  else
    setKW(P3KW, false);

}
///////////////////////////////////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  setup_io();
  Serial.begin (115200);
  //SerialBT.begin("Atmor"); //Bluetooth device name
  setup_wifi(); // + 	setup_telnet() inside
  sensors.begin();
  sensors.setWaitForConversion(false); // FALSE: function requestTemperature() etc returns immediately (USE WITH CARE!!)
  sensors.requestTemperatures();
  setup_timer();
  attachInterrupt(interruptPin, IRQcounter, RISING);
}
///////////////////////////////////////////////////////////////////
void readWaterCounter()
{
  WaterFlow = PulseWaterCount;
  if (PulseWaterCount > 0)
    isWaterFlow = true;
  else
    isWaterFlow = false;
  PulseWaterCount = 0;
}
///////////////////////////////////////////////////////////////////
void PrintStatus()
{
  DEBUG("%.1f > %.1f | %3d | %d \n",
        TempWaterIN, TempWaterOUT, (int)WaterFlow, power);
}
///////////////////////////////////////////////////////////////////
void every250mSec()
{
  //portENTER_CRITICAL(&timerMux);
  bNewQSec = false;
  //portEXIT_CRITICAL(&timerMux);
  PWM_5KW(qSec);

  if (qSec % 2 == 0)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);
}
///////////////////////////////////////////////////////////////////
void setState(STATE s)
{
  if(eState == s )
    return;
  eState = s;
  switch (eState)
  {
    case _IDLE:      DEBUG("State=>Idle\n");        break;
    case _PRE_HEAT:  DEBUG("State=>PreHeat\n");     break;
    case _POST_HEAT: DEBUG("State=>PostHead\t\n");  break;
    case _HEAT:      DEBUG("State=>Heat\n");        
                     secondI = 0;
               break;    
  }

}
///////////////////////////////////////////////////////////////////
void every1Sec() //New second section
{
  readAllTemp();
  readWaterCounter();

  if (!isWaterFlow)
  {
    SetPowerOff();
    switch (eState)
    {
      case _IDLE:
        break;

      case _POST_HEAT:
        postHeatCnt--;
        if (postHeatCnt == 0)
          setState(_IDLE);
        break;

      case _PRE_HEAT:
        preHeatCnt++;
        if (preHeatCnt >= PRE_HEAT_SET)
          setState(_IDLE);
        break;

      case _HEAT:
        postHeatCnt = POST_HEAT_SET;
        setState(_POST_HEAT);
        break;

    }

    return;
  }

  //WaterFlow ON
  PrintStatus();

  if (TempWaterDUD > (TempWaterDest + 3))
  {
    SetPowerOff();
    return;
  }

  switch (eState)
  {
    case _IDLE:
      preHeatCnt = PRE_HEAT_SET;
      setState(_PRE_HEAT);
      break;

    case _POST_HEAT:
      setState(_HEAT);
      break;

    case _PRE_HEAT:
      preHeatCnt--;
      if (preHeatCnt <= 0)
        setState(_HEAT);
      break;

    case _HEAT:
      if (secondI % 5 == 0) // every 5 sec
        SetPow(CalcPow());
      break;

  }
}
///////////////////////////////////////////////////////////////////
void every60Sec()
{
  if (!isWaterFlow)
  {
    if (!isWiFiInitDone)
    {
      Serial.println("setup_wifi try ...");
      setup_wifi();
    }
    else if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi.reconnect ...");
      WiFi.reconnect();
    }
  }
  PrintStatus();
}
///////////////////////////////////////////////////////////////////
void loop()
{
  if (isOTAinProgress)
  {
    ArduinoOTA.handle();
    return;
  }

  if (!bNewQSec)
  {
    if (!isWaterFlow) // nothing to do :)
    {
      delay(1);
      ArduinoOTA.handle();
      telnet_handle();
    }
    return;
  }

  every250mSec();

  qSec++;
  if (qSec == 4)
    qSec = 0;
  else
    return;

  secondI++;
  every1Sec();

  if ((secondI % 60 != 0))
    return;

  every60Sec();
}
