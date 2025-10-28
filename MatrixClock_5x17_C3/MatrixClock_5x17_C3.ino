//ESP32-C3 SuperMini
#include <Arduino.h>

#include "NeoPixel3x5Helper.h"
#include "Max44009.h"

#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"
#include <SolarCalculator.h>
#include <Wire.h>
//#include <ld2410.h>

//#include <SoftwareSerial.h>

//const char *ssid = "D224";
//const char *password = "1qazxsw2";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
//const long gmtOffset_sec = 3600;
//const int daylightOffset_sec = 3600;

int loopCnt = 0;

const char *time_zone = "IST-2IDT,M3.4.4/26,M10.5.0";
struct tm time_now;
struct tm time_sunrise;
struct tm time_sunset;

bool time_get_done  = false;
bool time_sync_done = false;

int GMTOffset;

float LightLevel = 0;

float getGMTOffset(void) 
{ 
    time_t t, t2; 
    struct tm *tm2; 
    
    time(&t); // get seconds since 1970 
    tm2 = gmtime(&t); // Convert to UTC time 
    tm2->tm_isdst = -1; 
    t2 = mktime(tm2); // Convert back to seconds 
    return((t-t2)/3600); 
}

time_t tmTimeDiff( struct tm * tm1, struct tm * tm2 )
{
  return mktime(tm2) - mktime(tm1); 
}

bool isDay()
{
  if( mktime( &time_now) < mktime(&time_sunrise) || mktime( &time_now) > mktime(&time_sunset) )
    return false;
  else
    return true;
}

void GetSunriseSunset()
{
  time_t now;
  double transit, sunrise, sunset;
  calcSunriseSunset(time(&now), 30.812426, 34.859474, transit, sunrise, sunset);
  sunrise += GMTOffset;
  sunset  += GMTOffset;
  int m;

  getLocalTime(&time_sunrise);
  m = int(round(sunrise * 60));
  time_sunrise.tm_hour = (m / 60) % 24;
  time_sunrise.tm_min = m % 60;
  time_sunrise.tm_sec = 0;
  Serial.print("Sunrise: ");
  Serial.println(&time_sunrise, "%H:%M:%S");

  getLocalTime(&time_sunset);
  m = int(round(sunset * 60));
  time_sunset.tm_hour = (m / 60) % 24;
  time_sunset.tm_min = m % 60;
  time_sunset.tm_sec = 0;
  Serial.print("Sunset : ");
  Serial.println(&time_sunset, "%H:%M:%S");
}


void printLocalTime() {
  if (!getLocalTime(&time_now)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&time_now, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) 
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
  time_sync_done = true;
}

NeoMatrix cMatrix;

//RgbColor ColoaArrDay[5] = {RGB_green, RGB_green, RGB_green, RGB_green, RGB_green};
//RgbColor ColoaArrNgt[5] = {RGB_blue, RGB_blue, RGB_blue, RGB_blue, RGB_blue};

RgbColor GetCurrentColor()
{

  uint8_t Saturation = 8;
  if( LightLevel > 50 )
    Saturation = 16;
    if( LightLevel > 75 )
    Saturation = 32;
  if( LightLevel > 100 )
    Saturation = 64;

    if( isDay() )
    {
      //return( RGB_green );
      return( RgbColor(0, Saturation, 0));
    }
    else
    {
      //return( RGB_blue );
      return( RgbColor(0, 0, Saturation));
    }  
}


void TaskLightMeter( void * parameter )
{
  Max44009 myLux(0x4A);
  Wire.begin();

  for(;;)
  {

    LightLevel = myLux.getLux();
    int error = myLux.getError();
    if (error != 0)
    {
      Serial.print("Error:\t");
      Serial.println(error);
    }
    /*
    else
    {
      Serial.print("lux:\t");
      Serial.println(LightLevel, 3);
    }
    */

    delay(1000);
  }

}

void TaskClockShow( void * parameter )
{
  Serial.print("TaskClockShow running on core ");
  Serial.println(xPortGetCoreID());
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 1000;
  xLastWakeTime = xTaskGetTickCount();

  for(;;)
  {
    if (!time_get_done)
    {
      Serial.printf("TaskClockShow - No time \n");
    }
    else
    {
      getLocalTime(&time_now);
      //Serial.println(&time_now, "%A, %B %d %Y %H:%M:%S");  
      Serial.print(&time_now, "%H:%M:%S [");
      Serial.print(LightLevel);
      if( isDay() )
      {
        Serial.println(" lux] - Day");
        //ColoaArr = ColoaArrDay;
      }
      else
      {
          Serial.println(" lux] - Nigth");  
          //ColoaArr = ColoaArrNgt;  
      }

      //Serial.printf( "delta from time_sunrise %d ",  tmTimeDiff( &time_sunrise, &time_now ));
      //Serial.printf( "delta from time_sunset  %d \n",tmTimeDiff( &time_sunset,  &time_now ));

      //cMatrix.SetAll( RGB_black );
      cMatrix.Buff3x5Digit(0, time_now.tm_hour / 10, GetCurrentColor()); //Show3x5Digit
      cMatrix.Buff3x5Digit(4, time_now.tm_hour % 10, GetCurrentColor());
      cMatrix.Buff3x5Digit(10, time_now.tm_min  / 10, GetCurrentColor());
      cMatrix.Buff3x5Digit(14, time_now.tm_min  % 10, GetCurrentColor());

      if( time_now.tm_sec % 4 == 0 )
          cMatrix.Buff2Point(GetCurrentColor(),GetCurrentColor());
      else 
          cMatrix.Buff2Point(); 

      cMatrix.myShow(); 

    }    
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  } 
}

void ShowIP()
{
  for(int i=0; i<4; i++)
  {
    uint8_t d = WiFi.localIP()[i];
    cMatrix.SetAll( RGB_black );
    cMatrix.Show3x5Digit(0, d / 100,     RGB_white);
    cMatrix.Show3x5Digit(4, d / 10 % 10, RGB_white);
    cMatrix.Show3x5Digit(8, d % 10,      RGB_white);
    cMatrix.Show();
    delay(1000);
  }

}

extern void TaskWiFi( void * parameter );
void setup()
{
  Serial.begin(115200);
  delay(1000);
  //while (!Serial && !Serial.available()) {}

  //Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  cMatrix.NeoPixelSetup(true); //with test


 // ESP32_WiFi_Manager_setup();

  xTaskCreate(
      TaskClockShow,    // Function that should be called
      "TaskClockShow",  // Name of the task (for debugging)
      1024 * 2,         // Stack size (bytes)
      NULL,            // Parameter to pass
      1,               // Task priority
      NULL             // Task handle
      );

  xTaskCreate(
      TaskWiFi,    // Function that should be called
      "TaskWiFi",  // Name of the task (for debugging)
      1024 * 3,         // Stack size (bytes)
      NULL,            // Parameter to pass
      1,               // Task priority
      NULL             // Task handle
      );

  xTaskCreate(
      TaskLightMeter,    // Function that should be called
      "TaskLightMeter",  // Name of the task (for debugging)
      1024 * 1,         // Stack size (bytes)
      NULL,            // Parameter to pass
      1,               // Task priority
      NULL             // Task handle
      );
    
}

void loop()
{
  //Serial.printf("WiFi.getMode %d \n ", WiFi.getMode());

  if(WiFi.getMode() == WIFI_AP)  // WIFI_AP
  {

      cMatrix.SetAll();
      cMatrix.Show3x5Digit(0, DIG_A , RGB_white ); //'A'
      cMatrix.Show3x5Digit(4, DIG_P , RGB_white ); //'P'
      cMatrix.Show();
      delay(1000);

      for(int i=0; i<4; i++)
      {
        uint8_t d = WiFi.softAPIP()[i];
        cMatrix.SetAll();
        cMatrix.Show3x5Digit(6, d / 100,     RGB_white);
        cMatrix.Show3x5Digit(10, d / 10 % 10, RGB_white);
        cMatrix.Show3x5Digit(14, d % 10,      RGB_white);
        cMatrix.Show();
        delay(1000);
      }     

    cMatrix.SetAll( RGB_black );
     
    if( loopCnt > 120 )
      ESP.restart();
  }
  else // WIFI_STA
  {
    if( WiFi.status() == WL_CONNECTED  )
    {
      if ( !time_get_done )
      {

        esp_sntp_servermode_dhcp(1);  // (optional)
        sntp_set_time_sync_notification_cb(timeavailable);
        configTzTime(time_zone, ntpServer1, ntpServer2);

        while (!getLocalTime(&time_now))
        {
          Serial.printf("No time available \n");
          cMatrix.Show2Point( RGB_blue, RGB_black);
          delay(1000);
          cMatrix.Show2Point( RGB_black, RGB_blue); 
          //delay(1000);
        }
      
        GMTOffset = getGMTOffset();
        Serial.printf("GMTOffset %d  \n",GMTOffset);

        GetSunriseSunset();       
        ShowIP();
        time_get_done = true;
      }

      if ( time_sync_done )
      {
        Serial.println("time_sync_done ->WiFi.disconnect");
        WiFi.disconnect();
      }
    }
    else // WiFi.status() != WL_CONNECTED
    {
      if( !time_sync_done )
      {
        if( loopCnt % 2 == 0 )
          cMatrix.Show2Point( RGB_black, RGB_blue);
        else
          cMatrix.Show2Point( RGB_blue, RGB_black);         
      }
      else // time_sync_done
      {
        if( time_now.tm_hour == 3 && time_now.tm_min == 0 && time_now.tm_sec < 10 )
        {
          //dayly reboot
          ESP.restart();
        }
      }
    }
  
  }

  delay(1000);
  loopCnt++;
}
