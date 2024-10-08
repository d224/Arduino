#ifndef EEPROM_data_h
#define EEPROM_data_h

#include <Arduino.h>
#include <EEPROM.h>
#include "Sprinkler.h"

typedef struct {
  uint32_t time_ON;
  uint8_t  duration;
  uint8_t  active;
} chanel_setup;


typedef struct {
  char id[8];
  int32_t ver;
  chanel_setup ch[CH_NUM];
} EEPROM_struct;

#define EEPROM_SIZE sizeof(EEPROM_struct)
extern EEPROM_struct data;

void ReadEEPROM(uint8_t * p, uint16_t size);
void WriteEEPROM(uint8_t * p, uint16_t size);
void printEEPROM_Data();

void setup_EEPROM();

#endif //EEPROM_data_h
