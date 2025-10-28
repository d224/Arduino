#ifndef NeoPixel3x5Helper_h
#define NeoPixel3x5Helper_h

#include <Arduino.h>
#include <NeoPixelBus.h>

#define colorSaturation 128

extern RgbColor RGB_red;
extern RgbColor RGB_green;
extern RgbColor RGB_blue;
extern RgbColor RGB_white;
extern RgbColor RGB_black;
extern RgbColor RGB_yelow;

#define DIG_A 10
#define DIG_P 11

class NeoMatrix{
  public:
    NeoMatrix();
    void NeoPixelSetup(bool withTest = false);
    void SetPixel(uint16_t pos, RgbColor color = RGB_black, bool show = false);
    void Set2DPixel(uint8_t x, uint8_t y, RgbColor color = RGB_black, bool show = false);
    void Show();
    void SetAll(RgbColor color = RGB_black);
    void Show3x5Digit(uint16_t pos, uint16_t digit, RgbColor color);
    void Show3x5Digit(uint16_t col, uint16_t digit, RgbColor color[5]);
    void Show2Point(RgbColor colorU = RGB_black, RgbColor colorL = RGB_black);
    bool isReady();
    void ChangePixel (RgbColor _color1, RgbColor _color2);
    void SetColor (RgbColor & _color ) { color = _color;  };
    void SetColor (RgbColor _color )   { color = _color;  };
    void StartAnimating();
    void UpdateAnimations();

    void Buff3x5Digit(uint16_t col, uint8_t digit, RgbColor color);
    void Buff2Point(RgbColor colorU = RGB_black, RgbColor colorL = RGB_black);
    void myShow();
  private:
    RgbColor  color;
    static NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip;    
    uint8_t Matrix2Snake(uint8_t x, uint8_t y); 
};

#define MATRIX_H 5
#define MATRIX_W 17
#define PixelCount MATRIX_W * MATRIX_H
#define PixelPin 5

#endif


