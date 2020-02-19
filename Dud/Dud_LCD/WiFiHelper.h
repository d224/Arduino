#ifndef WiFiHelper_h
#define WiFiHelper_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

String strWiFiGetStatus();
void WiFiPrintStatus();
void WiFiPrintIP();
void WiFiPrintMode();
bool isWiFiConnected();
void WiFiConnect(const char* ssid, const char* password);
int WiFiConnectTry(const char* ssid, const char* password);
void WiFiDisconnect();
void onDisconnected(const WiFiEventStationModeDisconnected& event);
size_t DEBUG(const char *format, ...);
void telnet_handle();
void setup_telnet();
#endif
