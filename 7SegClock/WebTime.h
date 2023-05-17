/*
  http://api.geonames.org/timezoneJSON?lat=30.0&lng=35.0&username=d224
   {"sunrise":"2015-10-28 05:50","lng":35,"countryCode":"IL","dstOffset":2,"rawOffset":2,"sunset":"2015-10-28 16:56","timezoneId":"Asia/Jerusalem","dstOffset":3,"countryName":"Israel","time":"2015-10-28 20:57","lat":30}
*/


#ifndef WebTime_h
#define WebTime_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include "time.h"


class WebTime
{
  public:
    WebTime();
    static uint8_t _HH;
    static uint8_t _MM;
    static uint8_t _SS;
    static bool bTimeValid;
    //static uint32_t m_secSince1900;
    static uint32_t m_secSinceMidNight;
    static uint32_t m_Timezone; //in sec
    static uint32_t m_Sunrise;
    static uint32_t m_Sunset;
    static bool m_WiFi_connectNeeded;

    void Start();
    static bool isValid();
    static void PrintTime();
    static int32_t geonames();

    static bool isDay();

    static bool isWiFi_connectNeeded() { return m_WiFi_connectNeeded; };

    /*  
    void Stop();
    bool UpdateTask();
    void SetUpdate();
    void CalcTime();
 
    bool getNTP();

    bool openweathermap();

    unsigned long get_secSinceMidNight();
    bool isNewSec();
    bool isNewMin();
    bool isNewHour();
  
    unsigned int GetDayLen() {return (m_Sunset - m_Sunrise) / 60;}; // in Minute
    unsigned int GetNightLen() {return 1440 - GetDayLen();}; // in Minute
    unsigned int TimeFromSunrise(){ return (m_secSinceMidNight - m_Sunrise) / 60; };  // in Minute
    unsigned int TimeBeforeSunset(){ return (m_Sunset - m_secSinceMidNight) / 60; };  // in Minute
    unsigned int TimeFromSunset(){
      if (m_secSinceMidNight > m_Sunset ) 
        return (m_secSinceMidNight - m_Sunset) / 60; 
      else 
        return 1440 - (m_Sunset - m_secSinceMidNight) / 60; 
       }; 
      */

  private:
 //   const char* m_ssid;
 //   const char* m_password;
    char buff [512];

    //WiFiUDP udp;
  //  unsigned long sendNTPpacket(IPAddress& address);
  //  bool m_bNeedUpdateGMT;
  // bool m_bNeedUpdateNTP;
  //  static void OneSecTimerHandler();
    //Ticker Timer1;
    
};

#endif
