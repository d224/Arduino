/*
  http://api.geonames.org/timezoneJSON?lat=30.0&lng=35.0&username=d224
   {"sunrise":"2015-10-28 05:50","lng":35,"countryCode":"IL","dstOffset":2,"rawOffset":2,"sunset":"2015-10-28 16:56","timezoneId":"Asia/Jerusalem","dstOffset":3,"countryName":"Israel","time":"2015-10-28 20:57","lat":30}
*/


#ifndef WebTime_h
#define WebTime_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFiUdp.h>
#include <Ticker.h>

class WebTime
{
  public:
    WebTime(const char* ssid, const char* password);
    void Start();
    void Stop();
    bool UpdateTask();
    void SetUpdate();
    void CalcTime();
    void PrintTime();
    bool getNTP();
    bool geonames();
    bool openweathermap();
    uint8_t _HH;
    uint8_t _MM;
    uint8_t _SS;
    unsigned long get_secSinceMidNight();
    bool isNewSec();
    bool isNewMin();
    bool isNewHour();
    bool isDay();
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
  private:
    const char* m_ssid;
    const char* m_password;
    char buff [512];
    unsigned int   m_Timezone; //in sec
    unsigned int   m_Sunrise;
    unsigned int   m_Sunset;
    static uint32_t m_secSince1900;
    static uint32_t m_secSinceMidNight;
    WiFiUDP udp;
    unsigned long sendNTPpacket(IPAddress& address);
    bool m_bNeedUpdateGMT;
    bool m_bNeedUpdateNTP;
    static void OneSecTimerHandler();
    Ticker Timer1;
};

#endif
