#include <Arduino.h>
#include <Wire.h>
#include "BH1750.h"
//#include <ArduinoLog.h>

BH1750::BH1750() {
}

void BH1750::begin(uint8_t SDA, uint8_t SCL, uint8_t mode) {
  Wire.begin(SDA, SCL);
  configure(mode);
}

void BH1750::configure(uint8_t mode) {

  switch (mode) {
    case BH1750_CONTINUOUS_HIGH_RES_MODE:
    case BH1750_CONTINUOUS_HIGH_RES_MODE_2:
    case BH1750_CONTINUOUS_LOW_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE_2:
    case BH1750_ONE_TIME_LOW_RES_MODE:
      // apply a valid mode change
      write8(mode);
      //_delay_ms(10);
      delay(10);
      break;
    default:
      // Invalid measurement mode
      Serial.println("Invalid measurement mode");
      break;
  }
}

uint16_t BH1750::readLightLevel(void) {
  uint16_t level;
  Wire.beginTransmission(BH1750_I2CADDR);
  Wire.requestFrom(BH1750_I2CADDR, 2);
  level = Wire.read();
  level <<= 8;
  level |= Wire.read();
  Wire.endTransmission();
  level = level / 1.2; // convert to lux

  printf("Light=%d lux\n", level);
  //Serial.print("Light=");
  //Serial.print(level);
  //Serial.println(" lux");
  
  return level;
}
/*********************************************************************/
void BH1750::write8(uint8_t d) {
  Wire.beginTransmission(BH1750_I2CADDR);
  Wire.write(d);
  Wire.endTransmission();
}
