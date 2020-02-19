#include <Arduino.h>
#include "WebTime.h"
#include <ArduinoJson.h>
#include <JsonListener.h>
#include <WiFiUdp.h>
#include "WiFiHelper.h"

#include "GoogleMapsApi.h"
#include <WiFiClientSecure.h>
#define API_KEY "AIzaSyCmSs4ybjoi-2h3kHM1UD0eOUHkPSoTv4c"  // your google apps API Token

extern WiFiClient WiFi_client;
WiFiClientSecure WiFi_client_secure;
GoogleMapsApi GoogleApi(API_KEY, WiFi_client_secure);

// A UDP instance to let us send and receive packets over UDP
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte NTPpacketBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

////////////////////////////////////////////////////////////////////////////
#define seventyYears 2208988800UL

StaticJsonDocument<512> jsonBuffer;
#define UPDATE_RETRY 3

////////////////////////////////////////////////////////////////////////////
WebTimeClass::WebTimeClass()
{
	m_Offset = 2 * 60 * 60;
	//m_Offset = 0;
	m_Sunrise = 0;
	m_Sunset = 0;
	m_secSince1900 = 0;
	m_secSinceMidNight = 0;

	_HH = 0;
	_MM = 0;
	_SS = 0;

	m_bInitDone = false;
	m_nUpdateOffset = 0;
	m_nUpdateSun = 0;
	m_nUpdateNTP = 0;
	m_weekday = 0;
	m_NewSecHandlerFunction = NULL;
	m_NewMinHandlerFunction = NULL;
	m_NewHourHandlerFunction = NULL;
	m_NewDayHandlerFunction = NULL;
}
////////////////////////////////////////////////////////////////////////////
void WebTimeClass::Start(const char* ssid, const char* password)
{
	m_ssid = ssid;
	m_password = password;

	Timer1.attach(1, OneSecTimerHandler);

	m_nUpdateOffset = UPDATE_RETRY * 3;
	m_nUpdateSun = UPDATE_RETRY * 3;
	m_nUpdateNTP = UPDATE_RETRY * 3;
	while (m_nUpdateOffset + m_nUpdateSun + m_nUpdateNTP != 0)
	{
		UpdateTask();
		delay(1000);
	}

	CalcTime();
}
////////////////////////////////////////////////////////////////////////////
void WebTimeClass::Stop()
{
	Timer1.detach();
}
////////////////////////////////////////////////////////////////////////////
void WebTimeClass::OneSecTimerHandler()
{
	WebTime.m_secSince1900++;
	WebTime.m_secSinceMidNight++;
	WebTime.m_secSinceMidNight = WebTime.m_secSinceMidNight % 86400L;
}
////////////////////////////////////////////////////////////////////////////
bool WebTimeClass::GoogleMapsApi() {
  /*
	//https://maps.googleapis.com/maps/api/timezone/json?location=30.0,35.0&key=AIzaSyCmSs4ybjoi-2h3kHM1UD0eOUHkPSoTv4c&timestamp=1510816607
	Serial.println("********** GoogleMapsApi **********");
	String command = "https://maps.googleapis.com/maps/api/timezone/json?location=30.0,35.0&timestamp=";
	command += m_secSince1900 - seventyYears;
	Serial.println(command);
	String strRes = GoogleApi.sendGetToGoogleMaps(command);

	jsonBuffer.clear();
	JsonObject& root = jsonBuffer.parseObject(strRes.c_str());
	if (root.success())
	{
		const char*  strStatus = root["status"];
		int DstOffset = root["dstOffset"];
		int RawOffset = root["rawOffset"];

		if (strcmp(strStatus, "OK") == 0)
		{
			Serial.print("DstOffset :");
			Serial.println(DstOffset);
			Serial.print("RawOffset :");
			Serial.println(RawOffset);

			m_Offset = RawOffset + DstOffset;
			Serial.print("Offset :");
			Serial.println(m_Offset);
			return true;
		}
		else
		{
			Serial.print("Status :");
			Serial.println(strStatus);
		}
	}
 */
	return false;
}
////////////////////////////////////////////////////////////////////////////
bool WebTimeClass::GeonamesApi() {
/*
	const char* host = "api.geonames.org";
	Serial.println(host);
	const char* strTime = NULL;
	const char* strSunrise = NULL;
	const char* strSunset = NULL;
	//g_dstOffset = 0;
	int responce_cnt = 0;

	if (!WiFi_client.connect(host, 80)) {
		Serial.println("connection failed");
		return false;
	}
	String command = "GET /timezoneJSON?lat=30.0&lng=35.0&username=d224 HTTP/1.0\r\nHost: api.geonames.org\r\nConnection: close\r\n\r\n";
	WiFi_client.print(command);
	//client.print(String("GET //timezoneJSON?lat=30.0&lng=35.0&username=d224 HTTP//1.0\r\nHost: ") + host + "\r\n" + "Connection: close\r\n\r\n");

	unsigned long timeout = millis();
	while (WiFi_client.available() == 0)
	{
		if (millis() - timeout > 5000)
		{
			Serial.println("Client Timeout !");
			WiFi_client.stop();
			return false;
		}
	}

	while (WiFi_client.available())
	{
		//read line
		memset(buff, 0, 512);
		size_t len = WiFi_client.readBytesUntil('\r', buff, 512);
		if (buff[1] == '{')
		{
			//Serial.println(buff);
			jsonBuffer.clear();
			JsonObject& root = jsonBuffer.parseObject(buff);
      deserializeJson(root, buff);
			if (root.success())
			{
				strTime = root["time"];
				strSunrise = root["sunrise"];
				strSunset = root["sunset"];
				//m_dstOffset = root["dstOffset"];

				Serial.print("time :");
				Serial.println(strTime);
				//Serial.print("dstOffset :");
				//Serial.println(m_dstOffset);

				if ((strlen(strSunrise) == 16) && (strSunrise[13] == ':'))
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

				if ((strlen(strSunset) == 16) && (strSunset[13] == ':'))
				{
					m_Sunset = (strSunset[15] - '0') * 60 +
						(strSunset[14] - '0') * 600 +
						(strSunset[12] - '0') * 3600 +
						(strSunset[11] - '0') * 36000;

					Serial.print("Sunset  :");
					Serial.print(strSunset);
					Serial.print(" :+ ");
					Serial.println(m_Sunset);
				}
				WiFi_client.stop();
				return true;
			}
			else
			{
				Serial.println("Parse error");
				Serial.println(buff);
				WiFi_client.stop();
				return false;
			}
		}
	}
	Serial.println("Responce error");
	Serial.println(buff);
	WiFi_client.stop();
 */
	return false;
}
////////////////////////////////////////////////////////////////////////////
// send an NTP request to the time server at the given address
unsigned long WebTimeClass::sendNTPpacket(IPAddress & address)
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
	NTPpacketBuffer[12] = 49;
	NTPpacketBuffer[13] = 0x4E;
	NTPpacketBuffer[14] = 49;
	NTPpacketBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(NTPpacketBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}
////////////////////////////////////////////////////////////////////////////
bool WebTimeClass::getNTP()
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
	Serial.print("Seconds since Jan 1 1900 = ");
	Serial.println(m_secSince1900);
	return true;
}
////////////////////////////////////////////////////////////////////////////
void WebTimeClass::PrintTime()
{
	printf("\n# %02d:%02d:%02d :+%d ", _HH, _MM, _SS, m_secSinceMidNight);
	if (isDay())
		Serial.println("Day");
	else
		Serial.println("Nigth");
}
////////////////////////////////////////////////////////////////////////////
void WebTimeClass::CalcTime()
{
	epoch = m_secSince1900 - seventyYears + m_Offset;
	_HH = (epoch % 86400L) / 3600; // print the hour (86400 equals secs per day)
	_MM = (epoch % 3600) / 60; // print the minute (3600 equals secs per minute)
	_SS = epoch % 60;
	m_secSinceMidNight = _HH * 60 * 60 + _MM * 60 + _SS;
	m_weekday = (epoch / 86400L + 4) % 7;
	_strTime[0] = _HH / 10 + '0';
	_strTime[1] = _HH % 10 + '0';
	_strTime[2] = ':';
	_strTime[3] = _MM / 10 + '0';
	_strTime[4] = _MM % 10 + '0';
	_strTime[5] = 0;
}
////////////////////////////////////////////////////////////////////////////
bool WebTimeClass::isNewSec()
{
	static uint32_t prev_sec = m_secSinceMidNight;
	if (prev_sec != m_secSinceMidNight)
	{
		prev_sec = m_secSinceMidNight;
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////
bool WebTimeClass::isNewMin()
{
	static uint32_t prev_min = m_secSinceMidNight / 60;
	if (prev_min != m_secSinceMidNight / 60)
	{
		prev_min = m_secSinceMidNight / 60;
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////
bool WebTimeClass::isNewHour()
{
	static uint32_t prev_hour = m_secSinceMidNight / 3600;
	if (prev_hour != m_secSinceMidNight / 3600)
	{
		prev_hour = m_secSinceMidNight / 3600;
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////
bool WebTimeClass::isDay()
{
	if ((m_secSinceMidNight > m_Sunrise) && (m_secSinceMidNight < m_Sunset))
		return true;
	return false;
}
////////////////////////////////////////////////////////////////////////////
// true - keep connection - not finished yet
bool WebTimeClass::UpdateTask()
{
	if (!isNewSec())
		if (m_nUpdateNTP + m_nUpdateSun + m_nUpdateOffset == 0)
		{
			return false;
			m_bInitDone = true;
		}
		else
		{
			return true;
			m_bInitDone = false;
		}

	CalcTime();

	//once a second
	if (m_NewSecHandlerFunction && m_bInitDone)
		m_NewSecHandlerFunction();

	//once a Minute	
	if (isNewMin() && m_bInitDone)
	{
		if (m_NewMinHandlerFunction)
			m_NewMinHandlerFunction();
	}

	//once a Hour	
	if (isNewHour() && m_bInitDone)
	{
		if (m_NewHourHandlerFunction)
			m_NewHourHandlerFunction();

		// new day
		if(_HH == 0) 
			if (m_NewDayHandlerFunction && m_bInitDone)
				m_NewDayHandlerFunction();

		m_nUpdateNTP = UPDATE_RETRY;
		if (m_secSinceMidNight < 60 * 60 * 6) // from 00:00 to 05:00
		{
			m_nUpdateSun = UPDATE_RETRY;
			m_nUpdateOffset = UPDATE_RETRY;
			CalcTime();
		}
	}

	if (m_nUpdateNTP + m_nUpdateSun + m_nUpdateOffset == 0)
	{
		m_bInitDone = true;
		return false;
	}

	m_bInitDone = false;
	if (WiFiConnectTry(m_ssid, m_password))
	{
		//connecting .....
		return true;
	}

	printf("NTP:%d UpdateSun:%d UpdateOffset:%d\n", m_nUpdateNTP, m_nUpdateSun, m_nUpdateOffset);

	if (m_nUpdateNTP > 0)
	{
		if (getNTP())
		{
			m_nUpdateNTP = 0;
			CalcTime();
			return true;
		}
		else
			m_nUpdateNTP--;
	}

	if (m_nUpdateSun > 0)
	{
		if (GeonamesApi())
		{
			m_nUpdateSun = 0;
			CalcTime();
			return true;
		}
		else
			m_nUpdateSun--;
	}

	if (m_nUpdateOffset > 0)
	{
		if (GoogleMapsApi())
		{
			m_nUpdateOffset = 0;
			CalcTime();
			return true;
		}
		else
			m_nUpdateOffset--;
	}

	CalcTime();
	return true;
}

WebTimeClass WebTime;
