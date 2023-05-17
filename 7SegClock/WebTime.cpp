#include <Arduino.h>
#include "WebTime.h"
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "sntp.h"


/* A more convenient approach to handle TimeZones with daylightOffset 
* would be to specify a environmnet variable with TimeZone definition including daylight adjustmnet rules.
* A list of rules for your zone could be obtained from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h */

#define CONFIGTZTIMESTR "EET-2EEST,M3.4.4/50,M10.4.4/50" // #define TZ_Asia_Hebron

const char* ntpServer1 = "timeserver.iix.net.il";
const char* ntpServer2 = "pool.ntp.org";
//const char* ntpServer3 = "time.nist.gov";

const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


HTTPClient http; 

uint8_t  WebTime::_HH = 0;
uint8_t  WebTime::_MM = 0;
uint8_t  WebTime::_SS = 0;
//uint32_t WebTime::m_secSince1900 = 0;
uint32_t WebTime::m_secSinceMidNight = 0;
bool     WebTime::bTimeValid = false;
uint32_t WebTime::m_Timezone = 0; //in sec
uint32_t WebTime::m_Sunrise = 6 * 3600; // 06:00
uint32_t WebTime::m_Sunset = 19 * 3600; // 19:00
bool     WebTime::m_WiFi_connectNeeded = true;

//https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
void IRAM_ATTR onTimer0_1sec() 
{
  WebTime::_SS ++;
  WebTime::m_secSinceMidNight ++;
  if(WebTime::_SS >= 60 )
  {
    WebTime::_SS = 0;
    WebTime::_MM++;    
  }

  if(WebTime::_MM >= 60 )
  {
    WebTime::_MM = 0;
    WebTime::_HH++;
  }

  if(WebTime::_HH >= 24 )
  {
    WebTime::_HH=0;
    WebTime::m_secSinceMidNight = 0;
  }
}

// Callback function (get's called when time adjusts via NTP)
void NTP_timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  WebTime::_HH = timeinfo.tm_hour;
  WebTime::_MM = timeinfo.tm_min;
  WebTime::_SS = timeinfo.tm_sec;
  WebTime::m_secSinceMidNight = WebTime::_HH * 3600 + WebTime::_MM * 60 + WebTime::_SS;
  WebTime::bTimeValid = true;
}

// A UDP instance to let us send and receive packets over UDP
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte NTPpacketBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

////////////////////////////////////////////////////////////////////////////

void WebTimeTask( void * parameter)
{
  Serial.print("WebTimeTask started");
  short geonames_retry = 3;
  for(;;)
  {
    if ( WebTime::isValid() )
    {
      WebTime::PrintTime();
      if( geonames_retry > 0 )   
      {
        int32_t geonames_Time = WebTime::geonames();
        if( geonames_Time >=0 ) 
        {
          geonames_retry = 0;
          int32_t timeDeltaSS =  geonames_Time - WebTime::m_secSinceMidNight;
          Serial.printf("geonames time delta %d\n", timeDeltaSS);
          if( abs( timeDeltaSS ) > 50 * 60 ) // correction needed
          {
            int32_t timeDeltaHH;
            if( timeDeltaSS > 0)
              timeDeltaHH = (timeDeltaSS + 600) / 3600;
            else
              timeDeltaHH = (timeDeltaSS - 600) / 3600;

            WebTime::_HH += timeDeltaHH;
            Serial.printf("correction needed %d\n", timeDeltaHH );
          }
          else
          {
            Serial.printf("No correction needed :)\n" );
          }
          WebTime::m_WiFi_connectNeeded = false;
        }
        else // fail to get data
        {
          Serial.printf("geonames fail to get data [%d] retry in 60sec...\n", geonames_retry );
          delay( 60 * 1000 );
        }
        geonames_retry --;
      }
    }
    else 
    {
      Serial.print("#Time invalid\n");
    }

    delay(1000);
  }
}

////////////////////////////////////////////////////////////////////////////
WebTime::WebTime()
{
  m_Timezone = 3 * 3600;
}

bool WebTime::isValid()
{
  return WebTime::bTimeValid;
}
////////////////////////////////////////////////////////////////////////////
void WebTime::Start()
{
  // set notification call-back function
  sntp_set_time_sync_notification_cb( NTP_timeavailable );

  /**
   * NTP server address could be aquired via DHCP,
   *
   * NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
   * otherwise SNTP option 42 would be rejected by default.
   * NOTE: configTime() function call if made AFTER DHCP-client run
   * will OVERRIDE aquired NTP server address
   */
  sntp_servermode_dhcp(1);    // (optional)

  /**
   * This will set configured ntp servers and constant TimeZone/daylightOffset
   * should be OK if your time zone does not need to adjust daylightOffset twice a year,
   * in such a case time adjustment won't be handled automagicaly.
   */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  configTzTime(CONFIGTZTIMESTR, ntpServer1, ntpServer2);  

  hw_timer_t * timer0_1sec = NULL;
  timer0_1sec = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0_1sec, &onTimer0_1sec, true);
  timerAlarmWrite(timer0_1sec, 1000000, true);
  timerAlarmEnable(timer0_1sec);

  xTaskCreatePinnedToCore( WebTimeTask,  "WebTimeTask", 10240, NULL, 0, NULL, 1); 

}
////////////////////////////////////////////////////////////////////////////
void WebTime::PrintTime()
{
   Serial.printf("# %02d:%02d:%02d :+%d ", WebTime::_HH, WebTime::_MM, WebTime::_SS, WebTime::m_secSinceMidNight);
  if (isDay())
    Serial.print("Day\n");
  else
    Serial.print("Nigth\n");
}
////////////////////////////////////////////////////////////////////////////
bool WebTime::isDay()
{
  if ((m_secSinceMidNight > m_Sunrise) && (m_secSinceMidNight < (m_Sunset)) )
    return true;
  return false;
}
/*
////////////////////////////////////////////////////////////////////////////
void WebTime::Stop()
{
  Timer1.detach();
}
////////////////////////////////////////////////////////////////////////////
void WebTime::OneSecTimerHandler()
{
  m_secSince1900++;
  m_secSinceMidNight++;
  m_secSinceMidNight = m_secSinceMidNight % 86400L;
}

////////////////////////////////////////////////////////////////////////////
unsigned long WebTime::get_secSinceMidNight()
{
  return m_secSinceMidNight;
}

////////////////////////////////////////////////////////////////////////////
bool WebTime::openweathermap()
{
  bool ret = false;

  Serial.println("api.openweathermap.org");

    if(http.begin("http://api.openweathermap.org/data/2.5/weather?q=Nesher,IL&APPID=cec919e38cfe8397b302c610c7d85232&units=metric")) 
    {  // HTTP
      int httpCode = http.GET();
      if (httpCode > 0) 
      {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
        {
          String buff = http.getString();
          http.end();   //Close connection
    
          //Serial.println(buff);
          StaticJsonDocument<1024> jsonDoc;
          auto error = deserializeJson(jsonDoc, buff);
          if (error) 
          {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
            Serial.println(buff);
            return false;
          }
        
          m_Timezone = jsonDoc["timezone"] ;
          m_Sunrise  = jsonDoc["sys"]["sunrise"];
		  m_Sunrise  = (m_Sunrise + m_Timezone) % 86400L;
          m_Sunset   = jsonDoc["sys"]["sunset"];
		  m_Sunset   = (m_Sunset  + m_Timezone) % 86400L;
        
          Serial.print("timezone :");
          Serial.println(m_Timezone);
          Serial.print("Sunrise :");
          Serial.println(m_Sunrise);
          Serial.print("Sunset :");
          Serial.println(m_Sunset);        
          ret = true;
        }
      } 
      else
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } 
    else
    {
      Serial.printf("[HTTP} Unable to connect\n");
    }

  return ret;
}

////////////////////////////////////////////////////////////////////////////
// send an NTP request to the time server at the given address
unsigned long WebTime::sendNTPpacket(IPAddress& address)
{

  // A UDP instance to let us send and receive packets over UDP
  unsigned int localPort = 2390;      // local port to listen for UDP packets

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(NTPpacketBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  NTPpacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  NTPpacketBuffer[1] = 0;     // Stratum, or type of clock
  NTPpacketBuffer[2] = 6;     // Polling Interval
  NTPpacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  NTPpacketBuffer[12]  = 49;
  NTPpacketBuffer[13]  = 0x4E;
  NTPpacketBuffer[14]  = 49;
  NTPpacketBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(NTPpacketBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
////////////////////////////////////////////////////////////////////////////
bool WebTime::getNTP()
{

  //IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
  IPAddress timeServer(192, 114, 62, 250); // http://www.isoc.org.il/iix/2x_ntp.html - ntp.iix.net.il
  //IPAddress timeServer(82,80,130,220); // il.pool.Ntp.org
  //IPAddress timeServer(192,168,0,1); //

  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
    return false;
  }

  Serial.print("packet received, length=");
  Serial.println(cb);
  // We've received a packet, read the data from it
  udp.read(NTPpacketBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  //the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, esxtract the two words:

  unsigned long highWord = word(NTPpacketBuffer[40], NTPpacketBuffer[41]);
  unsigned long lowWord = word(NTPpacketBuffer[42], NTPpacketBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  m_secSince1900 = highWord << 16 | lowWord;
  Serial.print("Seconds since Jan 1 1900 = " );
  Serial.println(m_secSince1900);
  return true;
}

////////////////////////////////////////////////////////////////////////////
#define seventyYears 2208988800UL
void WebTime::CalcTime()
{
  unsigned long epoch = m_secSince1900 - seventyYears + (m_Timezone);
  _HH = (epoch  % 86400L) / 3600; // print the hour (86400 equals secs per day)
  _MM = (epoch  % 3600) / 60; // print the minute (3600 equals secs per minute)
  _SS = epoch % 60;
  m_secSinceMidNight = _HH * 60 * 60 + _MM * 60 + _SS;
  PrintTime();
}
////////////////////////////////////////////////////////////////////////////
bool WebTime::isNewSec()
{
  static uint32_t prev_sec = m_secSinceMidNight;
  if (prev_sec != m_secSinceMidNight)
  {
    CalcTime();
    prev_sec = m_secSinceMidNight;
    return true;
  }
  return false;
}
////////////////////////////////////////////////////////////////////////////
bool WebTime::isNewMin()
{
  static uint32_t next_min = m_secSince1900 + 60;
  if (m_secSince1900 >= next_min)
  {
    next_min += 60;
    Serial.print("New Minute\n");
    return true;
  }
  return false;
}
////////////////////////////////////////////////////////////////////////////
bool WebTime::isNewHour()
{
  static uint32_t next_hour = m_secSince1900 + 3600;
  if (m_secSince1900 >= next_hour)
  {
    next_hour += 3600;
    Serial.print("New Hour\n");
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////
// true - keep connection
bool WebTime::UpdateTask()
{
  if (!m_bNeedUpdateGMT && !m_bNeedUpdateNTP)
    return false;

  if (WiFiConnectTry(m_ssid, m_password))
  {
    //connecting .....
    return true;
  }

  if (m_bNeedUpdateGMT)
  {
    if (openweathermap())
    {
      m_bNeedUpdateGMT = false;
      CalcTime();
    }
    return true;
  }

  if (m_bNeedUpdateNTP )
  {
    if (getNTP())
    {
      m_bNeedUpdateNTP = false;
      //m_bNeedUpdateGMT = true;
      CalcTime();
    }
    return true;
  }

  return true;
}
////////////////////////////////////////////////////////////////////////////
void WebTime::SetUpdate()
{
  m_bNeedUpdateNTP = true;
}
*/

////////////////////////////////////////////////////////////////////////////
int32_t WebTime::geonames()
{
  int32_t geonames_Time = -1;
  const char* strTime  = NULL;
  const char* strSunrise = NULL;
  const char* strSunset = NULL;

  Serial.println("api.geonames.org");

    if(http.begin("http://api.geonames.org/timezoneJSON?lat=30.0&lng=35.0&username=d224")) 
    {  // HTTP
      int httpCode = http.GET();
      if (httpCode > 0) 
      {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
        {
          String buff = http.getString();
          http.end();   //Close connection
          
          //Serial.println(buff);
          StaticJsonDocument<8048> jsonDoc;
          auto error = deserializeJson(jsonDoc, buff);
          if (error) 
          {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
            Serial.println(buff);
            return -1;
          }
        
          strTime = jsonDoc["time"];
          strSunrise = jsonDoc["sunrise"];
          strSunset = jsonDoc["sunset"];
          //m_dstOffset = jsonDoc["dstOffset"];
          m_Timezone = jsonDoc["gmtOffset"];
		      m_Timezone *= 3600;
        
          Serial.print("time :");
          Serial.println(strTime);
          Serial.print("dstOffset :");
          Serial.println(m_Timezone);

          if ((strlen(strTime) == 16) && (strTime[13] == ':') )
          {
            geonames_Time = (strTime[15] - '0') * 60 +
                            (strTime[14] - '0') * 600 +
                            (strTime[12] - '0') * 3600 +
                            (strTime[11] - '0') * 36000;
        
            Serial.print("Time :");
            Serial.print(strTime);
            Serial.print(" :+ ");
            Serial.println(geonames_Time);
          }
        
          if ((strlen(strSunrise) == 16) && (strSunrise[13] == ':') )
          {
            m_Sunrise = (strSunrise[15] - '0') * 60 +
                        (strSunrise[14] - '0') * 600 +
                        (strSunrise[12] - '0') * 3600 +
                        (strSunrise[11] - '0') * 36000;
        
            Serial.print("Sunrise :");
            Serial.print(strSunrise);
            Serial.print(" :+ ");
            Serial.println(m_Sunrise);
          }
        
          if ((strlen(strSunset) == 16) && (strSunset[13] == ':') )
          {
            m_Sunset  = (strSunset[15] - '0') * 60 +
                        (strSunset[14] - '0') * 600 +
                        (strSunset[12] - '0') * 3600 +
                        (strSunset[11] - '0') * 36000;
        
            Serial.print("Sunset  :");
            Serial.print(strSunset);
            Serial.print(" :+ ");
            Serial.println(m_Sunset);
          }
          return geonames_Time;
        }
      } 
      else
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } 
    else
    {
      Serial.printf("[HTTP} Unable to connect\n");
    }

  return -1;
}
