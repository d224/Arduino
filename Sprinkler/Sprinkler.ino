/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-thermostat-web-server/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#define SOFTAP

#include <WiFi.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <RTClib.h>


RTC_DS3231 rtc;
DateTime rtc_now;
	 
#include "WebServer.h"
#include "EEPROM_data.h"

EEPROM_struct data;
#define LED_BUILTIN 2

#ifdef SOFTAP
const char* ssid = "SPRINKLER";
const char* password = "12345678";
#else
const char* ssid = "4OBEZYAN";
const char* password = "1QAZXSW2";
#endif


uint16_t nCurrentTime = 0;
#define M_OPEN  true
#define M_CLOSE false
#define NOT_SET 0xFFFFFFFF

#define GPIO_WAKE_UP GPIO_NUM_33

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t secCounter = NOT_SET;
volatile uint32_t secCounterPrev = NOT_SET;
volatile uint32_t secWIFI_ON = 60;


void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  secCounter++;
  if(secCounter == 86400)
	  secCounter = 0;
  if(secWIFI_ON)
	  secWIFI_ON --;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR ISR33_FALLING() {
    Serial.println("FALLING");
}

void setup_rtc()
{
	 if (! rtc.begin()) {
	   Serial.println("Couldn't find RTC");
	 }
	 else 
	 {
		if (rtc.lostPower()) 
		{
			Serial.println("RTC lost power, lets set the time!");
			// following line sets the RTC to the date & time this sketch was compiled
			rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
			   // This line sets the RTC with an explicit date & time, for example to set
			   // January 21, 2014 at 3am you would call:
			   // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
			 }	
			 rtc_now = rtc.now();		 
			 Serial.printf("RTC: %d:%d \n",rtc_now.hour(), rtc_now.minute());
			 start_timer(rtc_now.hour()*3600 +  rtc_now.minute()*60);
		}
}
void setup_timer()
{
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  //timerAlarmEnable(timer);
}
void set_time(int min) //in min
{
	Serial.printf("set_time to: %d min\n", min);
	rtc.adjust(DateTime(rtc_now.year(), rtc_now.month(), rtc_now.day(), min/60, min%60, 0));
	start_timer(min*60);
}

void start_timer(int sec)
{
  Serial.printf("start_timer\n");
  secCounter = sec;
  timerAlarmEnable(timer);
}

void printTime()
{
  if(secCounter != NOT_SET)
  {
    Serial.print("now: ");
    Serial.print(secCounter / 3600);
    Serial.print(":");
    Serial.print(secCounter % 3600 / 60);
    Serial.print(":");
    Serial.println(secCounter % 60);
  }
  else
    Serial.println("Time: N.A.");
}

uint32_t timeToSec(String t)
{
	uint32_t r = 0;
	if(t.length() < 8)
		return 0;
	if(t[2] != ':' || t[5] != ':')
		return 0;
	
	r += (t[0]-'0')*36000;
	r += (t[1]-'0')*3600;	
	r += (t[3]-'0')*600;	
	r += (t[4]-'0')*60;	
	r += (t[6]-'0')*10;	
	r += (t[7]-'0');
	return r;	
}

#define P_NAME 0
#define PA 1
#define PB 2
//const uint8_t M1[] = {1,19,18};
//const uint8_t M2[] = {2,17,16};
const uint8_t MOTORS[CH_NUM][3] = {{1,19,18},{2,17,16}};

void setup_motors()
{
  pinMode(MOTORS[0][PA], OUTPUT);
  pinMode(MOTORS[0][PB], OUTPUT);
  pinMode(MOTORS[1][PA], OUTPUT);
  pinMode(MOTORS[1][PB], OUTPUT);  
  set_motor(MOTORS[0], false);
  delay(100);
  set_motor(MOTORS[1], false);
  delay(100);
}

void set_motor(const uint8_t* m, bool bOnOff)
{
   Serial.print("Motor ");
   Serial.print(m[P_NAME]);

   if(bOnOff)
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
	if(open_time == 0)
		return;
	
	set_motor(m, true); 
	digitalWrite(LED_BUILTIN, HIGH);
	for(int i=open_time; i>0; i--)
	{
		Serial.println(i);
		delay(1000);
	}	 
	set_motor(m, false);
}


void setup() {
  Serial.begin(115200);
  
#ifdef SOFTAP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);
 
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  } 
  
  Serial.println("Set softAPConfig");
  IPAddress Ip(1, 1, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //.softAP(const char* ssid, const char* password, int channel, int ssid_hidden, int max_connection)
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());
#endif  
	
	ReadEEPROM((uint8_t *)&data, sizeof(data));
 
    pinMode(LED_BUILTIN, OUTPUT);
	pinMode(GPIO_WAKE_UP,INPUT_PULLUP); //INPUT_PULLUP
    attachInterrupt(GPIO_WAKE_UP, ISR33_FALLING, FALLING);
	
	digitalWrite(LED_BUILTIN, HIGH);  
	delay(3000);                       
    setup_motors();
	digitalWrite(LED_BUILTIN, LOW); 
	
    setup_timer();
	
	setup_rtc();	

	WebServerBegin();
}

uint16_t sec2WakeUp = 24*60*60;

uint16_t getSec2WakeUp(uint16_t time_ON)
{
	if(time_ON >= secCounter)
		return time_ON - secCounter;
	return	24*60*60 - (secCounter - time_ON);
}

void loop() 
{

	if(secCounter == NOT_SET)
	{
	  digitalWrite(LED_BUILTIN, HIGH);   delay(100);                       
	  digitalWrite(LED_BUILTIN, LOW);    delay(100); 
	  return;	  
	}

	if(secCounterPrev != secCounter) // new sec
	{
		rtc_now = rtc.now();
		printTime();
		secCounterPrev = secCounter;
		digitalWrite(LED_BUILTIN, (secCounter % 2) == 0);
		
		for(int i=0; i<CH_NUM; i++)
		{
			if(data.ch[i].active)
			{
				uint16_t time_ON = data.ch[i].time_ON * 60;
				if(secCounter == time_ON)
				{
					esp_wifi_stop(); 
					open_valwe(MOTORS[i], data.ch[i].duration * 60);			
					return;
				}
				if(sec2WakeUp > getSec2WakeUp(time_ON))
					sec2WakeUp = getSec2WakeUp(time_ON);
			}			
		}
		
			
		if(secWIFI_ON == 1)
		{
			Serial.println("esp_wifi_stop");
			esp_wifi_stop(); 
			if(sec2WakeUp > 600)
			{
				sec2WakeUp -= 300;
				uint16_t alarm_h = rtc_now.hour() + sec2WakeUp / 3600;
				uint16_t alarm_m = rtc_now.minute() + sec2WakeUp % 3600 / 60 ;
				Serial.printf("Time to WakeUp: %d sec\n", sec2WakeUp);
				Serial.printf("Next Alarm1: %d:%d \n", alarm_h, alarm_m);
				Serial.printf("Sleep ....\n");
				rtc.setAlarm1(0, alarm_h, alarm_m , 0, DS3231_MATCH_H_M_S);
				//  rtc.setAlarm1(0, 0, 0, 0, DS3231_EVERY_SECOND);
				rtc.enableAlarm1Interrupt(1);				
				esp_sleep_enable_ext0_wakeup(GPIO_WAKE_UP, 0);
				esp_sleep_enable_gpio_wakeup();
				esp_deep_sleep_start();				
			}

		}
  
			
	}

	delay(100); 

}
