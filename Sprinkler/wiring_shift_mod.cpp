/*
https://www.hackster.io/jlorentz/scoreboard-with-esp32-and-tm1638-with-ble-android-app-e38cc7
*/

#include "wiring_private.h"
#include "wiring_shift_mod.h"

void sendCommand(uint8_t value)

{
  digitalWrite(strobe_pin, LOW);
  shiftOutMod(data_pin, clock_pin, LSBFIRST, CLOCK_TYPE, CLOCK_DELAY_US, value);
  digitalWrite(strobe_pin, HIGH);
}

void TM1638reset()

{
  pinMode(strobe_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  
  sendCommand(0x8f);  // activate
    
  sendCommand(0x40); // set auto increment mode
  digitalWrite(strobe_pin, LOW);
  shiftOutMod(data_pin, clock_pin, LSBFIRST, CLOCK_TYPE, CLOCK_DELAY_US, 0xc0);   // set starting address to 0
  for(uint8_t i = 0; i < 16; i++)
  {
    shiftOutMod(data_pin, clock_pin, LSBFIRST, CLOCK_TYPE, CLOCK_DELAY_US, 0x00);
  }
  digitalWrite(strobe_pin, HIGH);

  sendCommand(0x44);  // set single address
}

uint8_t shiftInMod(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t clock_type, uint16_t clock_delay_us) {
	uint8_t value = 0;
	uint8_t i;

  
	for (i = 0; i < 8; ++i) {
		digitalWrite(clockPin, (clock_type ? LOW : HIGH));
    delayMicroseconds(clock_delay_us);
		if (bitOrder == LSBFIRST)
			value |= digitalRead(dataPin) << i;
		else
			value |= digitalRead(dataPin) << (7 - i);
		digitalWrite(clockPin, (clock_type ? HIGH : LOW));
	}
	return value;
}

void shiftOutMod(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t clock_type, uint16_t clock_delay_us, uint8_t val)
{
	uint8_t i;

	for (i = 0; i < 8; i++)  {
		if (bitOrder == LSBFIRST)
			digitalWrite(dataPin, !!(val & (1 << i)));
		else	
			digitalWrite(dataPin, !!(val & (1 << (7 - i))));
			
		digitalWrite(clockPin, (clock_type ? LOW : HIGH));
    delayMicroseconds(clock_delay_us);
		digitalWrite(clockPin, (clock_type ? HIGH : LOW));		
	}
}

void TM1638Out(uint8_t adr, uint8_t val)
{

  digitalWrite(strobe_pin, LOW);
  shiftOutMod(data_pin, clock_pin, LSBFIRST, CLOCK_TYPE, CLOCK_DELAY_US, adr); 
  shiftOutMod(data_pin, clock_pin, LSBFIRST, CLOCK_TYPE, CLOCK_DELAY_US, val); 
  digitalWrite(strobe_pin, HIGH);
}

static const uint8_t digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x00 };
void TM1638Digit(uint8_t adr, uint8_t val, bool dot)
{
  if(val <= 10)
    if(dot)
      TM16387Seg(adr, digits[val] | 0x80);
    else
      TM16387Seg(adr, digits[val] );  
}

void TM1638Digit_non0(uint8_t adr, uint8_t val)
{
  if( val == 0 )
    val = 10; // empty
  TM1638Digit(adr,val);
}

void TM16387Seg(uint8_t adr, uint8_t val)
{
  //0xc0 first digit , 0xce last digit
  if(adr < 8)
    TM1638Out(0xc0 + adr*2, val);
}

void TM1638Led(uint8_t adr, uint8_t val)
{
  //0xc0 first led , 0xce last digit
  if(adr < 8)
    TM1638Out(0xc1 + adr*2, val);
}


uint8_t TM1638Buttons(void)
{
  uint8_t buttons = 0;
  digitalWrite(strobe_pin, LOW);
  shiftOutMod(data_pin, clock_pin, LSBFIRST, CLOCK_TYPE, CLOCK_DELAY_US, 0x42);

  pinMode(data_pin, INPUT);

  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t v = shiftInMod(data_pin, clock_pin, LSBFIRST, CLOCK_TYPE, CLOCK_DELAY_US) << i; 
    buttons |= v;
  }

  pinMode(data_pin, OUTPUT);
  digitalWrite(strobe_pin, HIGH);
  return buttons;
}

//https://en.wikichip.org/wiki/seven-segment_display/representing_letters
//https://gist.github.com/rwaldron/0dd696800d2a09786ec2
const unsigned char seven_seg_digits_decode_gfedcba[75]= {
/*  0     1     2     3     4     5     6     7     8     9     :     ;     */
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x00, 
/*  <     =     >     ?     @     A     B     C     D     E     F     G     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 
/*  H     I     J     K     L     M     N     O     P     Q     R     S     */
    0x76, 0x30, 0x1E, 0x75, 0x38, 0x55, 0x54, 0x3F, 0x73, 0x67, 0x50, 0x6D, 
/*  T     U     V     W     X     Y     Z     [     \     ]     ^     _     */
    0x78, 0x3E, 0x1C, 0x1D, 0x64, 0x6E, 0x5B, 0x00, 0x00, 0x00, 0x00, 0x00, 
/*  `     a     b     c     d     e     f     g     h     i     j     k     */
    0x00, 0x5F, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x30, 0x1E, 0x75, 
/*  l     m     n     o     p     q     r     s     t     u     v     w     */
    0x38, 0x55, 0x54, 0x5C, 0x73, 0x67, 0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x1D, 
/*  x     y     z     */
    0x64, 0x6E, 0x5B
};

/* Invalid letters are mapped to all segments off (0x00). */
unsigned char decode_7seg(unsigned char chr)
{ /* Implementation uses ASCII */
    if (chr > (unsigned char)'z' | chr <(unsigned char)'0')
        return 0x00;
    return seven_seg_digits_decode_gfedcba[chr - '0'];
    /* or  
  return seven_seg_digits_decode_gfedcba[chr - '0']; */
}

// adr 0--7
void TM1638Char(uint8_t adr, const char ch)
{
   TM16387Seg(adr, decode_7seg(ch));
}

void TM1638Str(const char* str)
{
  uint i = 0;
  while(str[i] !=0 && i<8 )
  {
    TM16387Seg(i, decode_7seg(str[i]));
    i++;  
  }
  for(;i<8; i++)
    TM16387Seg(i, 0);
}
