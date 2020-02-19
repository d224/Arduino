#ifndef WebServer_h
#define WebServer_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

void WebServerBegin(const char * host);
void WebServerHandleClient();
#endif
