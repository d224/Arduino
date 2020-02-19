/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  See the LICENSE file for details.
 ***************************************************************************/

#include <Wire.h>
//#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define RELAY 12
#define LED1 13

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

TwoWire theI2C;

unsigned long delayTime = 10000;
float fTemperature = 0;
float fHumidity = 0;
#define Humidity_MAX 80.0
#define LED_ON LOW
#define LED_OFF HIGH

void setup() {
  Serial.begin(9600);
  while (!Serial);   // time to get serial running
  Serial.println(F("BME280 test"));

  unsigned status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  theI2C.begin(4, 14); //(int sda, int scl)
  status = bme.begin(0x76, &theI2C);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
    while (1);
  }

  pinMode(RELAY, OUTPUT);
  pinMode(LED1, OUTPUT);
  Serial.println();
}

void loop()
{
  readValues();
  printValues();
  if (fHumidity > Humidity_MAX)
  {
    Serial.println("ON");
    digitalWrite(RELAY, HIGH);
    digitalWrite(LED1, LED_ON);
    delay(1000);
  }
  else
  {
    digitalWrite(RELAY, LOW);

    for(int i=0; i<10; i++)
    {
      if(fHumidity / 10 > i)
        digitalWrite(LED1, LED_ON);
      delay(50);
      digitalWrite(LED1, LED_OFF);
      delay(50);
    }    
  }
    
  //analogWrite(LED1, 255 - fHumidity*2.55);
}

void readValues() {

  fTemperature = bme.readTemperature();
  fHumidity = bme.readHumidity();

  /*
      Serial.print(bme.readPressure() / 100.0F);
      Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  */



}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(fTemperature);
  Serial.println(" *C");

  Serial.print("Humidity = ");
  Serial.print(fHumidity);
  Serial.println(" %");
  /*
      Serial.print("Pressure = ");
      Serial.print(bme.readPressure() / 100.0F);
      Serial.println(" hPa");

      Serial.print("Approx. Altitude = ");
      Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
      Serial.println(" m");
  */


  Serial.println();
}
