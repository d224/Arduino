#include <Arduino.h>
#include "WiFiHelper.h"

WiFiServer telnetServer(23); // --> default port for communication usign TELNET protocol | Server Instance
WiFiClient telnetClient; // --> Client Instanse

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
  setup_telnet();
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
///////////////////////////////////////////////////////////////////
void setup_telnet() {

  telnetServer.begin();
  telnetServer.setNoDelay(true); // --> Won't be storing data into buffer and wait for the ack. rather send the next data and in case nack is received, it will resend the whole data
  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 23' to connect");
}
///////////////////////////////////////////////////////////////////
size_t DEBUG(const char *format, ...)
{
  char loc_buf[64];
  char * temp = loc_buf;
  va_list arg;
  va_list copy;
  va_start(arg, format);
  va_copy(copy, arg);
  size_t len = vsnprintf(NULL, 0, format, arg);
  va_end(copy);
  if (len >= sizeof(loc_buf)) {
    temp = new char[len + 1];
    if (temp == NULL) {
      return 0;
    }
  }
  len = vsnprintf(temp, len + 1, format, arg);
  Serial.write((uint8_t*)temp, len);
  if (telnetClient && telnetClient.connected())
    telnetClient.write((uint8_t*)temp, len);

  va_end(arg);
  if (len > 64) {
    delete[] temp;
  }
  return len;
}
///////////////////////////////////////////////////////////////////
void telnet_handle()
{
  //check if there are any new clients
  if (telnetServer.hasClient())
  {
    if (telnetClient)
      telnetClient.stop();
    telnetClient = telnetServer.available();
    DEBUG("New Telnet Client connected\n");
    DEBUG(" WIn > WOut | Flow| Power\n");
  }

  //check client for data
  if (telnetClient && telnetClient.connected()) {
    if (telnetClient.available())
    {
      //get data from the telnet client and push it to the UART
      while (telnetClient.available())
      {
        char ch = telnetClient.read();
        if (ch < 127 && ch >= ' ')
          Serial.write(ch);
      }
    }
  }
}