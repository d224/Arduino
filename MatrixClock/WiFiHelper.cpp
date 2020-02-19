#include <Arduino.h>
#include "WiFiHelper.h"

////////////////////////////////////////////////////////////////////////////
void WiFiPrintStatus()
{
  Serial.print("WiFi.status ");
  Serial.println(strWiFiGetStatus());
}
////////////////////////////////////////////////////////////////////////////
String strWiFiGetStatus()
{
  switch (WiFi.status())
  {
    case WL_NO_SHIELD:
      return "NO_SHIELD";
    case WL_IDLE_STATUS:
      return "IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "SCAN_COMPLETED";
    case WL_CONNECTED:
      return "CONNECTED";
    case WL_CONNECT_FAILED:
      return "CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "DISCONNECTED";
  }
  return "Error";
}
////////////////////////////////////////////////////////////////////////////
void WiFiPrintMode()
{
  Serial.print("WiFiConnect WiFi.mode   ");
  Serial.println(WiFi.getMode());
}
////////////////////////////////////////////////////////////////////////////
void WiFiPrintIP()
{
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
////////////////////////////////////////////////////////////////////////////
bool isWiFiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}
////////////////////////////////////////////////////////////////////////////
int WiFiConnectTry(const char* ssid, const char* password)
{
  static int attempt = 1;
  WiFiPrintStatus();

  if ( isWiFiConnected() )
  {
    attempt = 1;
    return 0;
  }

  if ( attempt == 1 )
  {
    Serial.println("Need reonnect");
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(100);
    Serial.print("Connecting to: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    attempt = 20;
  }
  else
  {
    Serial.print("Attempt : ");
    Serial.println(attempt);
    attempt --;
  }
  return attempt;
}
////////////////////////////////////////////////////////////////////////////
void WiFiConnect(const char* ssid, const char* password)
{
  while (!isWiFiConnected())
  {
    WiFiConnectTry(ssid, password);
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to: ");
  Serial.println(ssid);
  WiFiPrintIP();
}
////////////////////////////////////////////////////////////////////////////
void WiFiDisconnect()
{
  WiFiPrintStatus();
  if ( isWiFiConnected() )
  {
    Serial.print("Disconnect res:");
    Serial.println(WiFi.disconnect());
    WiFi.mode(WIFI_OFF);
    delay(10); // for WiFi.mode(WIFI_OFF)
  }
  WiFiPrintStatus();
}
////////////////////////////////////////////////////////////////////////////
void onDisconnected(const WiFiEventStationModeDisconnected& event)
{
  Serial.printf("Disconnected from SSID: %s\n", event.ssid.c_str());
  Serial.printf("Reason: %d\n", event.reason);
  delay(1000);
  ESP.restart();
}



