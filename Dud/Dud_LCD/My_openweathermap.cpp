#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESP8266WiFi.h>
#include "My_openweathermap.h"
#include "WiFiHelper.h"

//https://github.com/bblanchon/ArduinoJson/issues/837
//https://create.arduino.cc/projecthub/officine/getting-weather-data-655720 

WiFiClient client; 

const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 1024;       // max size of the HTTP response

openweathermap_Data weatherData;

// Open connection to the HTTP server
bool connect(const char* hostName) {
  DEBUG("Connect to %s ", hostName);
  bool ok = client.connect(hostName, 80);
  DEBUG(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, const char* resource) {
  Serial.print("GET ");
  Serial.println(resource);

  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(host);
  client.println("Connection: close");
  client.println();

  return true;
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    DEBUG("No response or invalid response!\n");
  }
  return ok;
}

bool readReponseContent(struct openweathermap_Data* clientData) {
 
  DynamicJsonDocument  doc(MAX_CONTENT_SIZE);
  //JsonObject& root = jsonBuffer.parseObject(client);
  unsigned long n_sunrise, n_sunset, n_timezone;

  auto error = deserializeJson(doc, client);
  if (error) {
		DEBUG("deserializeJson() failed with code %s\n", error.c_str());
		return false;
  }
 
  // Here were copy the strings we're interested in using to your struct data
  clientData->temp = doc["main"]["temp"]; // "16.38"
  clientData->weather_text = String(( const char*)(doc["weather"][0]["main"])); //"Haze" , "rain"
  clientData->icon = String((const char*)(doc["weather"][0]["icon"])); //"13n" "50d"
  n_timezone = doc["timezone"]; //7200
  n_sunrise = doc["sys"]["sunrise"];
  n_sunset = doc["sys"]["sunset"];
  clientData->sunrise_sec = (n_sunrise + n_timezone) % 86400;
  clientData->sunset_sec  = (n_sunset + n_timezone) % 86400;

  return true;
}

// Print the data extracted from the JSON
void printclientData( struct openweathermap_Data* clientData) {
  DEBUG("Temp = %f\n", clientData->temp);
  DEBUG("weather: %s\n", clientData->weather_text.c_str());
  DEBUG("icon: %s\n", clientData->icon.c_str());
  DEBUG("sunrise: %d:%d\n",
										clientData->sunrise_sec / 3600,
										clientData->sunrise_sec % 3600 / 60);
  DEBUG("sunset: %d:%d\n",
										clientData->sunset_sec / 3600,
										clientData->sunset_sec % 3600 / 60);										
}

String getMeteoconIcon(String icon) {
 	// clear sky
  // 01d
  if (icon == "01d") 	{
    return "B";
  }
  // 01n
  if (icon == "01n") 	{
    return "C";
  }
  // few clouds
  // 02d
  if (icon == "02d") 	{
    return "H";
  }
  // 02n
  if (icon == "02n") 	{
    return "4";
  }
  // scattered clouds
  // 03d
  if (icon == "03d") 	{
    return "N";
  }
  // 03n
  if (icon == "03n") 	{
    return "5";
  }
  // broken clouds
  // 04d
  if (icon == "04d") 	{
    return "Y";
  }
  // 04n
  if (icon == "04n") 	{
    return "%";
  }
  // shower rain
  // 09d
  if (icon == "09d") 	{
    return "R";
  }
  // 09n
  if (icon == "09n") 	{
    return "8";
  }
  // rain
  // 10d
  if (icon == "10d") 	{
    return "Q";
  }
  // 10n
  if (icon == "10n") 	{
    return "7";
  }
  // thunderstorm
  // 11d
  if (icon == "11d") 	{
    return "P";
  }
  // 11n
  if (icon == "11n") 	{
    return "6";
  }
  // snow
  // 13d
  if (icon == "13d") 	{
    return "W";
  }
  // 13n
  if (icon == "13n") 	{
    return "#";
  }
  // mist
  // 50d
  if (icon == "50d") 	{
    return "M";
  }
  // 50n
  if (icon == "50n") 	{
    return "M";
  }
  // Nothing matched: N/A
  return ")";
}

void update_weather() 
{
  char weatherserver[] = "api.openweathermap.org";     
  const char* resource = "/data/2.5/weather?q=Nesher,IL&APPID=cec919e38cfe8397b302c610c7d85232&units=metric";
	
  if(connect(weatherserver)) 
  {
    if(sendRequest(weatherserver, resource) && skipResponseHeaders())
	{
      if(readReponseContent(&weatherData)) 
	  {
        printclientData(&weatherData);
      }
    }
  }

}