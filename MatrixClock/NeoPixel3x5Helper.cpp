#include <Arduino.h>
#include "NeoPixel3x5Helper.h"

RgbColor RGB_red(colorSaturation, 0, 0);
RgbColor RGB_green(0, colorSaturation, 0);
RgbColor RGB_blue(0, 0, colorSaturation);
RgbColor RGB_white(colorSaturation);
RgbColor RGB_black(0);
RgbColor RGB_yelow(colorSaturation / 2, colorSaturation, 0);

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> NeoMatrix::strip(PixelCount);      //D4
////////////////////////////////////////////////////////////////////////////
NeoMatrix::NeoMatrix()
{

}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::NeoPixelSetup(bool withTest)
{
  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();

  if (withTest)
  {
    for (int y = 0; y < MATRIX_H; y++)
      for (int x = 0; x < MATRIX_W; x++)
      {
        Set2DPixel(x, y, RGB_red, true);
        delay(10);
      }
    for (int y = 0; y < MATRIX_H; y++)
      for (int x = 0; x < MATRIX_W; x++)
      {
        Set2DPixel(x, y, RGB_green, true);
        delay(10);
      }
    for (int y = 0; y < MATRIX_H; y++)
      for (int x = 0; x < MATRIX_W; x++)
      {
        Set2DPixel(x, y, RGB_blue, true);
        delay(10);
      }
  }
}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::SetPixel(uint16_t pos, RgbColor color, bool show)
{
  strip.SetPixelColor(pos, color);
  if (show)
    strip.Show();
}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::Show()
{
  strip.Show();
}
////////////////////////////////////////////////////////////////////////////
bool NeoMatrix::isReady()
{
  return strip.CanShow();
}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::StartAnimating()
{
  //strip.StartAnimating();
}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::UpdateAnimations()
{
  //if (strip.IsAnimating())
  //  strip.UpdateAnimations();
}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::SetAll(RgbColor color)
{
  for (int i = 0; i < PixelCount; i++)
    SetPixel(i, color);
  strip.Show();
}
////////////////////////////////////////////////////////////////////////////
uint8_t NeoMatrix::Matrix2Snake(uint8_t x, uint8_t y)
{
  if (y % 2 == 0)
    return y * MATRIX_W + x;
  else
    return  y * MATRIX_W + (MATRIX_W - 1) - x;
}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::Set2DPixel(uint8_t x, uint8_t y, RgbColor color, bool show)
{
  //Serial.println(Matrix2Snake(x, y));
  SetPixel(Matrix2Snake(x, y), color, show);
}
////////////////////////////////////////////////////////////////////////////
  byte _3x5[10][15] = {
    1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, //0
    1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, //1
    1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, //2
    1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, //3
    0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, //4
    1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, //5
    1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, //6
    0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, //7
    1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, //8
    1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1  //9
  };
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::Show3x5Digit(uint16_t col, uint16_t digit, RgbColor color)
{

  uint8_t i = 0;
  for (uint8_t y = 0; y < 5; y++)
    for (uint8_t x = col; x < col + 3; x++, i++)
      if (_3x5[digit][i])
        Set2DPixel(x, y, color);
      else
        Set2DPixel(x, y, RGB_black);
}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::Show3x5Digit(uint16_t col, uint16_t digit, RgbColor color[5])
{

  uint8_t i = 0;
  for (uint8_t y = 0; y < 5; y++)
    for (uint8_t x = col; x < col + 3; x++, i++)
      if (_3x5[digit][i])
        Set2DPixel(x, y, color[y]);
      else
        Set2DPixel(x, y, RGB_black);
}
////////////////////////////////////////////////////////////////////////////
void NeoMatrix::Show3x5Point(RgbColor colorU, RgbColor colorL)
{
  Set2DPixel(8, 1, colorU);
  Set2DPixel(8, 3, colorL);
}
/*
  ////////////////////////////////////////////////////////////////////////////
  void NeoMatrix::ChangePoints (bool p1, bool p2)
  {
  uint16_t time = 500;
  if (p1)
    strip.LinearFadePixelColor(time, 14, color);
  else
    strip.LinearFadePixelColor(time, 14, 0);

  if (p2)
    strip.LinearFadePixelColor(time, 15, color);
  else
    strip.LinearFadePixelColor(time, 15, 0);
  }

*/



