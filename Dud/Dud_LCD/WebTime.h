/*
  http://api.geonames.org/timezoneJSON?lat=30.0&lng=35.0&username=d224
   {"sunrise":"2015-10-28 05:50","lng":35,"countryCode":"IL","gmtOffset":2,"rawOffset":2,"sunset":"2015-10-28 16:56","timezoneId":"Asia/Jerusalem","dstOffset":3,"countryName":"Israel","time":"2015-10-28 20:57","lat":30}
   {"sunrise":"2017-11-16 06:05","lng":35,"countryCode":"IL","gmtOffset":2,"rawOffset":2,"sunset":"2017-11-16 16:43","timezoneId":"Asia/Jerusalem","dstOffset":3,"countryName":"Israel","time":"2017-11-16 08:45","lat":30}
   {"sunrise":"2017-11-19 06:07","lng":35,"countryCode":"IL","gmtOffset":2,"rawOffset":2,"sunset":"2017-11-19 16:42","timezoneId":"Asia/Jerusalem","dstOffset":3,"countryName":"Israel","time":"2017-11-19 08:24","lat":30}
  /*

  AIzaSyCmSs4ybjoi-2h3kHM1UD0eOUHkPSoTv4c
  https://maps.googleapis.com/maps/api/timezone/json?location=30.0,35.0&key=AIzaSyCmSs4ybjoi-2h3kHM1UD0eOUHkPSoTv4c&timestamp=1510816607
  https://maps.googleapis.com/maps/api/timezone/json?location=30.0,35.0&key=AIzaSyCmSs4ybjoi-2h3kHM1UD0eOUHkPSoTv4c&timestamp=1505546206
  http://maps.googleapis.com/maps/api/
  {
   "dstOffset" : 3600,
   "rawOffset" : 7200,
   "status" : "OK",
   "timeZoneId" : "Asia/Jerusalem",
   "timeZoneName" : "Israel Daylight Time"
  }
*/

#ifndef WebTime_h
#define WebTime_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <ArduinoJson.h>
#include <JsonListener.h>
#include <Arduino.h>
#include <WiFiUdp.h>
#include <Ticker.h>

typedef std::function<void(void)> THandlerFunction;

class WebTimeClass
{
public:
	WebTimeClass();
	void Start(const char* ssid, const char* password);
	void Stop();
	bool UpdateTask();
	void onNewSec(THandlerFunction handler)  { m_NewSecHandlerFunction = handler; };
	void onNewMin(THandlerFunction handler)  { m_NewMinHandlerFunction = handler; };
	void onNewHour(THandlerFunction handler) { m_NewHourHandlerFunction = handler; };
	void onNewDay(THandlerFunction handler)  { m_NewDayHandlerFunction = handler; };
	void PrintTime();
	uint8_t _HH;
	uint8_t _MM;
	uint8_t _SS;
	char _strTime[6];
	unsigned long get_secSinceMidNight() { return m_secSinceMidNight; };
	unsigned int  get_minSinceMidNight() { return m_secSinceMidNight / 60; };
	uint8_t get_weekday() { return m_weekday; };
	bool isDay();
	unsigned int GetDayLen() {
		return (m_Sunset - m_Sunrise) / 60;
	}; // in Minute
	unsigned int GetNightLen() {
		return 1440 - GetDayLen();
	}; // in Minute
	int TimeFromSunrise() {
		return (m_secSinceMidNight - m_Sunrise) / 60;
	};  // in Minute
	int TimeBeforeSunset() {
		return (m_Sunset - m_secSinceMidNight) / 60;
	};  // in Minute
	unsigned int TimeFromSunset() {
		if (m_secSinceMidNight > m_Sunset)
			return (m_secSinceMidNight - m_Sunset) / 60;
		else
			return 1440 - (m_Sunset - m_secSinceMidNight) / 60;
	};
	uint32_t GetSecSince2000() { return m_secSince1900 - 3155673600L + m_Offset; };
	uint16_t GetDaysSince2000() { return GetSecSince2000() / 86400; };
private:
	bool m_bInitDone;
	void CalcTime();
	bool getNTP();
	bool GeonamesApi();
	bool GoogleMapsApi();
	const char* m_ssid;
	const char* m_password;
	char buff[512];
	int   m_Offset;
	int   m_Sunrise;
	int   m_Sunset;
	uint32_t m_secSince1900;
	uint32_t m_secSinceMidNight;
	uint8_t  m_weekday;
	WiFiUDP udp;
	unsigned long sendNTPpacket(IPAddress& address);
	unsigned long epoch;
	uint8_t m_nUpdateOffset;
	uint8_t m_nUpdateSun;
	uint8_t m_nUpdateNTP;
	static void OneSecTimerHandler();
	Ticker Timer1;
	bool isNewSec();
	bool isNewMin();
	bool isNewHour();
	THandlerFunction m_NewSecHandlerFunction;
	THandlerFunction m_NewMinHandlerFunction;
	THandlerFunction m_NewHourHandlerFunction;
	THandlerFunction m_NewDayHandlerFunction;
};

extern WebTimeClass WebTime;
#endif
