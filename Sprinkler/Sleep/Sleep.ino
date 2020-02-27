/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-thermostat-web-server/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/


#include <Wire.h>
#include "RTClib.h"
#define LED_BUILTIN 2

RTC_DS3231 rtc;
DateTime rtc_now;


uint16_t nCurrentTime = 0;

void IRAM_ATTR ISR33_RISING() {
    Serial.println("RISING");
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
			 Serial.printf("setup_rtc: %d:%d \n",rtc_now.hour(), rtc_now.minute());

		}
		
  // Set Alarm1 - Every 20s in each minute
  // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)	
		rtc.setAlarm1(0, 0, 0, 20, DS3231_MATCH_S);
		//  rtc.setAlarm1(0, 0, 0, 0, DS3231_EVERY_SECOND);
		rtc.enableAlarm1Interrupt(1);
}

void printTime()
{
	rtc_now = rtc.now();		 
	Serial.printf("RTC: %d:%d:%d \n",rtc_now.hour(), rtc_now.minute(), rtc_now.second());

}



void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(33,INPUT_PULLUP); //INPUT_PULLUP
  attachInterrupt(33, ISR33_FALLING, FALLING);
  //attachInterrupt(33, ISR33_RISING, RISING);
  setup_rtc();	
}


void loop() {

	  digitalWrite(LED_BUILTIN, HIGH);  
	  delay(500);                       
	  digitalWrite(LED_BUILTIN, LOW);   
	  delay(500);    
	  
	  
	//rtc_gpio_pullup_en();
	//esp_sleep_enable_ext0_wakeup();
	//esp_deep_sleep_start();
   printTime();	 
  if (rtc.isAlarm1())
  {
	 // printTime();
    Serial.println("ALARM 1 TRIGGERED!");
  }
  
  	 esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);
     esp_sleep_enable_gpio_wakeup();
	 esp_deep_sleep_start();

}
