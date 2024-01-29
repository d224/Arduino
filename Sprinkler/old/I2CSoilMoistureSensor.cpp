/*----------------------------------------------------------------------*
 * I2CSoilMoistureSensor.cpp - Arduino library for the Sensor version of*
 * I2C Soil Moisture Sensor version from Chrirp                         *
 * (https://github.com/Miceuz/i2c-moisture-sensor).                     *
 *                                                                      *
 * Ingo Fischer 11Nov2015                                               *
 * https://github.com/Apollon77/I2CSoilMoistureSensor                   *
 *                                                                      *
 * MIT license                                                          *
 *----------------------------------------------------------------------*/ 

#include "I2CSoilMoistureSensor.h"

/* I2C slave Address Scanner
for 5V bus
 * Connect a 4.7k resistor between SDA and Vcc
 * Connect a 4.7k resistor between SCL and Vcc
for 3.3V bus
 * Connect a 2.4k resistor between SDA and Vcc
 * Connect a 2.4k resistor between SCL and Vcc
Kutscher07: Modified for TTGO TQ board with builtin OLED
 */

#define SDA_2 27
#define SCL_2 26

//Wire1.setClockStretchLimit(4000);

#include <Wire.h>
#define i2cBegin Wire1.begin(SDA_2, SCL_2, 100000); 
#define i2cBeginTransmission Wire1.beginTransmission
#define i2cEndTransmission Wire1.endTransmission
#define i2cRequestFrom Wire1.requestFrom
#define i2cRead Wire1.read
#define i2cWrite Wire1.write

//Soil Moisture Sensor Register Addresses
#define SOILMOISTURESENSOR_GET_CAPACITANCE 	0x00 // (r) 	2 bytes
#define SOILMOISTURESENSOR_SET_ADDRESS 		0x01 //	(w) 	1 byte
#define SOILMOISTURESENSOR_GET_ADDRESS 		0x02 // (r) 	1 byte
#define SOILMOISTURESENSOR_MEASURE_LIGHT 	0x03 //	(w) 	n/a
#define SOILMOISTURESENSOR_GET_LIGHT 		0x04 //	(r) 	2 bytes
#define SOILMOISTURESENSOR_GET_TEMPERATURE	0x05 //	(r) 	2 bytes
#define SOILMOISTURESENSOR_RESET 			0x06 //	(w) 	n/a
#define SOILMOISTURESENSOR_GET_VERSION 		0x07 //	(r) 	1 bytes
#define SOILMOISTURESENSOR_SLEEP	        0x08 // (w)     n/a
#define SOILMOISTURESENSOR_GET_BUSY	        0x09 // (r)	    1 bytes
/*----------------------------------------------------------------------*
 * Constructor.                                                         *
 * Optionally set sensor I2C address if different from default          *
 *----------------------------------------------------------------------*/
I2CSoilMoistureSensor::I2CSoilMoistureSensor(uint8_t addr) : sensorAddress(addr) {
  // nothing to do ... Wire.begin needs to be put outside of class
}
/*----------------------------------------------------------------------*
 * Initializes anything ... it does a reset.                            *
 * When used without parameter or parameter value is false then a       *
 * waiting time of at least 1 second is expected to give the sensor     *
 * some time to boot up.                                                *
 * Alternatively use true as parameter and the method waits for a       *
 * second and returns after that.                                       *
 *----------------------------------------------------------------------*/
uint8_t I2CSoilMoistureSensor::begin() {
  uint8_t ver;
  i2cBegin;
  readI2CRegister16bitUnsigned(sensorAddress, SOILMOISTURESENSOR_GET_CAPACITANCE);
  for(int i=0; i<10; i++)
  {
		delay(100);
		m_version = readVersion();
		if (m_version == 1)
			  return m_version;
  }
  return 0;
}
/*----------------------------------------------------------------------*
 * Return measured Soil Moisture Capacitance                            *
 * Moisture is somewhat linear. More moisture will give you higher      *
 * reading. Normally all sensors give about 290 - 310 as value in free  * 
 * air at 5V supply.                                                    *
 *----------------------------------------------------------------------*/
unsigned int I2CSoilMoistureSensor::getCapacitance() {
  if(m_version)
	return readI2CRegister16bitUnsigned(sensorAddress, SOILMOISTURESENSOR_GET_CAPACITANCE);
  return 0;
}
/*----------------------------------------------------------------------*
 * Resets sensor. Give the sensor 0.5-1 second time to boot up after    *
 * reset.                                                               *
 *----------------------------------------------------------------------*/
void I2CSoilMoistureSensor::resetSensor() {
  writeI2CRegister8bit(sensorAddress, SOILMOISTURESENSOR_RESET);
}
/*----------------------------------------------------------------------*
 * Get Firmware Version. 0x22 means 2.2                                 *
 *----------------------------------------------------------------------*/
uint8_t I2CSoilMoistureSensor::readVersion() {
  return readI2CRegister8bit(sensorAddress, SOILMOISTURESENSOR_GET_VERSION);
}
/*----------------------------------------------------------------------*
 * Helper method to write an 8 bit value to the sensor via I2C          *
 *----------------------------------------------------------------------*/
void I2CSoilMoistureSensor::writeI2CRegister8bit(int addr, int value) {
  i2cBeginTransmission(addr);
  i2cWrite(value);
  i2cEndTransmission();
}
/*----------------------------------------------------------------------*
 * Helper method to write an 8 bit value to the sensor via I2C to the   *
 * given register                                                       *
 *----------------------------------------------------------------------*/
void I2CSoilMoistureSensor::writeI2CRegister8bit(int addr, int reg, int value) {
  i2cBeginTransmission(addr);
  i2cWrite(reg);
  i2cWrite(value);
  i2cEndTransmission();
}
/*----------------------------------------------------------------------*
 * Helper method to read a 16 bit unsigned value from the given register*
 *----------------------------------------------------------------------*/
uint16_t I2CSoilMoistureSensor::readI2CRegister16bitUnsigned(int addr, byte reg)
{
  uint16_t value;

  i2cBeginTransmission((uint8_t)addr);
  i2cWrite((uint8_t)reg);
  i2cEndTransmission();
  delay(20);
  i2cRequestFrom((uint8_t)addr, (byte)2);
  value = (i2cRead() << 8) | i2cRead();

  return value;
}
/*----------------------------------------------------------------------*
 * Helper method to read a 8 bit value from the given register          *
 *----------------------------------------------------------------------*/
uint8_t I2CSoilMoistureSensor::readI2CRegister8bit(int addr, int reg) {
  i2cBeginTransmission(addr);
  i2cWrite(reg);
  i2cEndTransmission();
  delay(20);
  i2cRequestFrom(addr, 1);
  return i2cRead();
}