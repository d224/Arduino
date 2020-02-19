#include <Arduino.h>
#include "WebTime.h"
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include "WiFiHelper.h"
#include <ESP8266HTTPClient.h>          // ESP8266 http library
HTTPClient http; 

// A UDP instance to let us send and receive packets over UDP
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte NTPpacketBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

////////////////////////////////////////////////////////////////////////////
uint32_t WebTime::m_secSince1900 = 0;
uint32_t WebTime::m_secSinceMidNight = 0;

////////////////////////////////////////////////////////////////////////////
WebTime::WebTime(const char* ssid, const char* password)
{
  m_ssid = ssid;
  m_password = password;
  m_Timezone = 3 * 3600;
  m_Sunrise = 0;
  m_Sunset = 0;
  m_secSince1900 = 0;
  m_secSinceMidNight = 0;

  _HH = 0;
  _MM = 0;
  _SS = 0;

  m_bNeedUpdateGMT = true;
  m_bNeedUpdateNTP = false;
}
////////////////////////////////////////////////////////////////////////////
void WebTime::Start()
{
  //WiFiConnect( m_ssid,  m_password);
  
  while (!openweathermap())
    delay(100000);

  while (!getNTP())
    delay(100000);

  Timer1.attach(1, OneSecTimerHandler);
}
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
void WebTime::PrintTime()
{
  printf("# %02d:%02d:%02d :+%d ", _HH, _MM, _SS, m_secSinceMidNight);
  if (isDay())
    Serial.print("Day\n");
  else
    Serial.print("Nigth\n");
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
bool WebTime::isDay()
{
  if ((m_secSinceMidNight > m_Sunrise) && (m_secSinceMidNight < (m_Sunset)) )
    return true;
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


////////////////////////////////////////////////////////////////////////////
/* 
bool WebTime::geonames()
{
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
            return false;
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
          return true;
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

  return false;
}
*/