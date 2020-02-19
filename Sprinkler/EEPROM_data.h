#ifndef EEPROM_data_h
#define EEPROM_data_h

#include <Arduino.h>
#include <EEPROM.h>

#define CH_NUM 2

typedef struct {
  uint16_t time_ON;
  uint8_t  duration;
  uint8_t  active;
} chanel_setup;


typedef struct {
  chanel_setup ch[CH_NUM];
} EEPROM_struct;

#define EEPROM_SIZE sizeof(EEPROM_struct)
extern EEPROM_struct data;

void ReadEEPROM(uint8_t * p, uint16_t size);
void WriteEEPROM(uint8_t * p, uint16_t size);

#endif //EEPROM_data_h
