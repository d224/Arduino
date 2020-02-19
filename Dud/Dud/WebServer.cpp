#include <Arduino.h>
#include "WebServer.h"

extern float t0, t1, t2, t3, t4;
extern uint16_t lux ;

ESP8266WebServer server(80);
char temp[1024];

extern float t0, t1, t2, t3, t4;
extern uint16_t lux ;

//////////////////////////////////////////////////////////////////////////
void handleRoot() 
{
  const char compile_date[] = __DATE__ " " __TIME__;
  snprintf(temp, 1024,
           "<html>\
		  <head>\
			<meta http-equiv='refresh' content='5'/>\
			<meta http-equiv='pragma' content='no-cache' />\
			<title>DUD</title>\
			<style> \
        h1{color:darkblue;border-bottom:solid 1px;border-top:solid 1px;width:480px;padding:20px;margin-left:auto;margin-right:auto;text-align:center;}\
				body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
			</style>\
		  </head>\
		  <body>\
			<h1Dud</h1>\
			<h1>Temp: %02d&deg; </h1>\
      <h1>Board: %02d&deg; </h1>\
      <h1>Water in: %02d&deg; </h1>\
      <h1>Water out: %02d&deg; </h1>\
      <h1>Panel: %02d&deg; </h1>\
			<h1>Ligth: %0d Lux</h1>\
      %s\
		  </body>\
		</html>",
           (int)t0, (int)t1, (int)t2, (int)t3, (int)t4, lux, compile_date
          );
  server.send(200, "text/html", temp);
}
//////////////////////////////////////////////////////////////////////////
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);

}
//////////////////////////////////////////////////////////////////////////
void WebServerHandleClient()
{
  server.handleClient();
}
//////////////////////////////////////////////////////////////////////////
void WebServerBegin()
{
  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/T0", []() { server.send(200, "text/plain", String(round(t0)));  });
  server.on("/T1", []() { server.send(200, "text/plain", String(round(t1)));  });
  server.on("/T2", []() { server.send(200, "text/plain", String(round(t2)));  });
  server.on("/T3", []() { server.send(200, "text/plain", String(round(t3)));  });
  server.on("/T4", []() { server.send(200, "text/plain", String(round(t4)));  });
  server.on("/L0", []() { server.send(200, "text/plain", String(lux));  });
  
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}
//////////////////////////////////////////////////////////////////////////
