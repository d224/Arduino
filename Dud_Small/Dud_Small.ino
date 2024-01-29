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

#include "motor.h"
#include "FlowSensor.h"
#include <ACS712.h>
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1 = { 0x28, 0xFF, 0x64, 0x1D, 0xF9, 0x86, 0x02, 0xD9 };




//  ACS712 5A  uses 185 mV per A
//  ACS712 20A uses 100 mV per A
//  ACS712 30A uses  66 mV per A
//ACS712  ACS( 1, 2.5, 51000 , 100 );   // 20A //ADC2 is shared with the WIFI module

Motor valve( 33, 18, 35, 37 );

//int nCurrent_mA = 0;

void secTask( void * parameter) 
{
  for(;;)
  {
    digitalWrite(15, HIGH);
    delay (500 );
    digitalWrite(15, LOW);
    delay (500 );
  }
}


void setup(void) 
{

  pinMode(15,OUTPUT);
  digitalWrite(15, HIGH);

  Serial.begin(115200);
  delay (3000 );
  Serial.printf("****Start ****\n");

  //xTaskCreatePinnedToCore( secTask, "secTask", 10240, NULL, 0, NULL, 1);      

  //valve.Init();
  FlowSensorStart();
}

void loop(void) 
{
  
  sensors.requestTemperatures();
  Serial.printf("Sensor 1(*C): %f \n", sensors.getTempC(sensor1) );
  Serial.printf("Flow %f \n", FlowSensorGetRate());

  delay (1000);
  
  /*

  if (digitalRead(pulseGPIO) == HIGH)
  {
    // turn LED on:
    digitalWrite(15, HIGH);
  } else {
    // turn LED off:
    digitalWrite(15, LOW);
    Serial.printf("ppm %d \n", pulseCnt);
  }
  */
}


/*
  byte i;
  byte addr[8];
  
  if (!ds.search(addr)) {
    Serial.println(" No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  Serial.print(" ROM =");
  for (i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }
  */
