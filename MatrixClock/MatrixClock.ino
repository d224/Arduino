#include <Arduino.h>
#include <ArduinoOTA.h>
//#include <NeoPixelBus.h>
#include "WiFiHelper.h"
#include "WebTime.h"
#include "NeoPixel3x5Helper.h"
//#include <ArduinoLog.h>
#include <Wire.h>
#include "MAX44009.h"
Max44009 cMAX44009;

const char* host = "matrix-clock";

const char* ssid     = "D224_2.4G";
const char* password = "1qazxsw2";


WebTime cWebTime(ssid, password);
NeoMatrix cMatrix;
float Lux_value;

#undef min
inline int min(int a, int b) {
  return ((a) < (b) ? (a) : (b));
}

void setup()
{
  Serial.begin(115200);
  while (!Serial && !Serial.available()) {}

  //Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  cMatrix.NeoPixelSetup(true); //with test
  WiFiConnect(ssid, password);
  ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]()
  { cWebTime.Stop();
    cMatrix.SetPixel(0, RGB_white, true);
  });
  ArduinoOTA.onEnd([]()
  { cMatrix.SetPixel(0, RGB_black, true);
    ESP.restart();
  });
  ArduinoOTA.onError([](ota_error_t error)
  { ESP.restart();
  });
  ArduinoOTA.begin();
  cWebTime.Start();
  cMatrix.SetAll();

  Lux_value = cMAX44009.getLux();
  Serial.print("lux:\t ");
  Serial.println(Lux_value);
}

void loop()
{
  RgbColor mainColor = RGB_green;
  if ( cWebTime.isNewSec() && cMatrix.isReady() ) // once a sec
  {
    bool bNeedUpdate = cWebTime.UpdateTask();
    if ( cWebTime.isNewMin() ) //once a min
    {
      if (!bNeedUpdate && isWiFiConnected())
        WiFiDisconnect();
      else
        printf("WiFi.Status: %s\n", strWiFiGetStatus().c_str());

      if (bNeedUpdate) // still need update :(
        ESP.restart();

      if ( cWebTime.isNewHour() )//once a hour
        if(cWebTime._HH < 4) // from 00 to 03
          cWebTime.SetUpdate();
    }

    if ( cWebTime.isDay())
    {
      mainColor = RGB_green;
      RgbColor ColoaArr[5] = {RGB_green, RGB_green, RGB_green, RGB_green, RGB_green};

      unsigned int FromSunrise_BeforeSunset = min(cWebTime.TimeFromSunrise(), cWebTime.TimeBeforeSunset());
      switch (FromSunrise_BeforeSunset)
      {
        case 1: ColoaArr[1] = RgbColor(0, 0, 64);
        case 2: ColoaArr[2] = RgbColor(0, 0, 64);
        case 3: ColoaArr[3] = RgbColor(0, 0, 64);
        case 4: ColoaArr[4] = RgbColor(0, 0, 64);
      }

      cMatrix.Show3x5Digit(0, cWebTime._HH / 10, ColoaArr);
      cMatrix.Show3x5Digit(4, cWebTime._HH % 10, ColoaArr);
      cMatrix.Show3x5Digit(10, cWebTime._MM / 10, ColoaArr);
      cMatrix.Show3x5Digit(14, cWebTime._MM % 10, ColoaArr);
    }
    else
    {
      mainColor = RGB_blue;
      Lux_value = cMAX44009.getLux();
      if (cMAX44009.getError())
      {
        printf("MAX44009 Error\n");
      }
      else
      {
        printf("Lux:\t %f\n", Lux_value);
        if (Lux_value < 10)
          mainColor = RgbColor(0, 0, 64);
        if (Lux_value < 1)
          mainColor = RgbColor(0, 0, 32);
      }
      cMatrix.Show3x5Digit(0, cWebTime._HH / 10, mainColor);
      cMatrix.Show3x5Digit(4, cWebTime._HH % 10, mainColor);
      cMatrix.Show3x5Digit(10, cWebTime._MM / 10, mainColor);
      cMatrix.Show3x5Digit(14, cWebTime._MM % 10, mainColor);
    }

    if (cWebTime._SS % 2 == 0)
    {
      cMatrix.Show3x5Point(mainColor, mainColor);
      if (isWiFiConnected()) cMatrix.Set2DPixel(8, 2, RGB_yelow);
      else                   cMatrix.Set2DPixel(8, 2);
    }
    else
    {
      cMatrix.Show3x5Point();
      if (bNeedUpdate)       cMatrix.Set2DPixel(8, 2, RGB_red);
      else                   cMatrix.Set2DPixel(8, 2);
    }

    cMatrix.Show();
  }

  delay(10);
  ArduinoOTA.handle();
}
