
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

  Serial.printf("WriteEEPROM %d bytes\n", size);
}

void printEEPROM_Data()
{
  Serial.printf("EEPROM_Data:\n");
  Serial.printf("ver 0x%X\n", data.ver);
  for (int i = 0; i < CH_NUM; i++)
    Serial.printf("Ch[%d] active[%d] time[%d] duration[%d]:\n", i,	data.ch[i].active, data.ch[i].time_ON, data.ch[i].duration);
}

void setup_EEPROM()
{
  if (!EEPROM.begin(EEPROM_SIZE))
    Serial.println("failed to initialise EEPROM");
  else
  {
    ReadEEPROM((uint8_t *)&data, sizeof(EEPROM_struct));
    if (data.ver == 0xFFFFFFFF)
    {
      Serial.println("EEPROM empty :(");
      memset((uint8_t *)&data, 0, sizeof(EEPROM_struct));
      data.ch[0].time_ON = 19 * 60 + 11;
      data.ch[0].duration = 1;
      data.ch[0].active = true;

      data.ch[1].time_ON = 11 * 60 + 20;
      data.ch[1].duration = 1;
      data.ch[1].active = false;

      data.ch[2].time_ON = 10 * 60 + 15;
      data.ch[2].duration = 1;
      data.ch[2].active = false;

      data.ch[3].time_ON = 15 * 60 + 49;
      data.ch[3].duration = 2;
      data.ch[3].active = false;            
    }

    printEEPROM_Data();
    data.ver = 1;
  }
}