#ifndef wiring_shift_mod_h
#define wiring_shift_mod_h

#define CLOCK_NORMAL 0
#define CLOCK_INVERT 1

#define CLOCK_TYPE CLOCK_INVERT
#define CLOCK_DELAY_US 1

#define strobe_pin  23//26 //4
#define clock_pin   22//27 //16
#define data_pin    21//14 //17

void sendCommand(uint8_t value);
void TM1638reset();

void shiftOutMod(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t clock_type, uint16_t clock_delay_us, uint8_t val);
uint8_t shiftInMod(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t clock_type, uint16_t clock_delay_us);

void TM1638Out(uint8_t adr, uint8_t val);
void TM16387Seg(uint8_t adr, uint8_t val);

void TM1638Digit(uint8_t adr, uint8_t val, bool dot = false);
void TM1638Digit_non0(uint8_t adr, uint8_t val);

void TM1638Char(uint8_t adr, const char ch);
void TM1638Str(const char* str);
void TM1638Led(uint8_t adr, uint8_t val);
uint8_t TM1638Buttons(void);
#endif
