/*----------------------------------------------------------------------*
 * I2CSoilMoistureSensor.h - Arduino library for the Sensor version of  *
 * I2C Soil Moisture Sensor version from Chrirp                         *
 * (https://github.com/Miceuz/i2c-moisture-sensor).                     *
 *                                                                      *
 * Ingo Fischer 11Nov2015                                               *
 * https://github.com/Apollon77/I2CSoilMoistureSensor                   *
 *                                                                      *
 * MIT license                                                          *
 *----------------------------------------------------------------------*/ 

#ifndef I2CSOILMOISTURESENSOR_H
#define I2CSOILMOISTURESENSOR_H

#include <Arduino.h> 

//Default I2C Address of the sensor
#define SOILMOISTURESENSOR_DEFAULT_ADDR 0x20




class I2CSoilMoistureSensor {
    public:
        I2CSoilMoistureSensor(uint8_t addr = SOILMOISTURESENSOR_DEFAULT_ADDR);

		uint8_t begin();
        unsigned int getCapacitance();
        void resetSensor();
        uint8_t getVersion(){ return m_version;}
		
    private:
		uint8_t readVersion();
		int sensorAddress;
		uint8_t m_version;
        void writeI2CRegister8bit(int addr, int value);
        void writeI2CRegister8bit(int addr, int reg, int value);
        uint16_t readI2CRegister16bitUnsigned(int addr, byte reg);
        int16_t readI2CRegister16bitSigned(int addr, byte reg);
        uint8_t readI2CRegister8bit(int addr, int reg);
};

#endif