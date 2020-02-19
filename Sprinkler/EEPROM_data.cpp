
#include "EEPROM_data.h"

void ReadEEPROM(uint8_t * p, uint16_t size)
{
	for(uint16_t i=0; i<size; i++)
		p[i] = EEPROM.readByte(i);
}
void WriteEEPROM(uint8_t * p, uint16_t size)
{
	for(uint16_t i=0; i<size; i++)
		EEPROM.writeByte(i, p[i]);
	EEPROM.commit();
}