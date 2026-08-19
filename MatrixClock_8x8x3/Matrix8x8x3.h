#ifndef Matrix8x8x3_h
#define Matrix8x8x3_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define COLOR  Adafruit_NeoPixel::Color

void Matrixsetup();

void Matrix(uint8_t x, uint8_t y, uint32_t c);
void Show();
void Clear();

void printChar5x8( uint8_t ch, uint8_t x, uint32_t color );
void printChar4x7( uint8_t ch, uint8_t x, uint32_t color );
void printChar3x5( uint8_t ch, uint8_t x, uint32_t color );

void drawClock( uint8_t H, uint8_t M, uint8_t x, bool bDots, uint32_t color );
void drawString( char * str, uint8_t x, uint32_t color );
void randomPixels( uint32_t color,  uint8_t dencity );

#endif //Matrix8x8x3_h