#include <Arduino.h>
#include "WebServer.h"

//#import "index.h"
const char HTTP_HEAD[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\"content=\"width=device-width,initial-scale=1,user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM = "<style> .c{text-align:center;}div,input{padding:5px;font-size:1em;}input{width:95%;}body{text-align:center;font-family:verdana;}button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float:right;width:64px;text-align:right;} .l{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\")no-repeat left center;background-size:1em;} </style>";
const char HTTP_SCRIPT[] PROGMEM = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();} </script>";
const char HTTP_HEAD_END[] PROGMEM = "</head><body><div style=\"text-align:left;display:inline-block;min-width:260px;\">";
const char HTTP_PORTAL_OPTIONS[] PROGMEM = "<form action=\"/wifi\"method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\"method=\"get\"><button>Configure WiFi(No Scan)</button></form><br/><form action=\"/i\"method=\"get\"><button>Info</button></form><br/><form action=\"/r\"method=\"post\"><button>Reset</button></form>";
const char HTTP_ITEM[] PROGMEM = "<div><a href='#p'onclick='c(this)'>{v}</a>&nbsp;<span class='q{i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM = "<form method='get'action='wifisave'><input id='s'name='s'length=32 placeholder='SSID'><br/><input id='p'name='p'length=64 type='password'placeholder='password'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM = "<br/><input id='{i}'name='{n}'length={l}placeholder='{p}'value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM = "<br/><button type='submit'>save</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_END[] PROGMEM = "</div></body></html>";


const char HTML_page_ON_OFF[] PROGMEM = R"=====(
		<html>
		  <head>
			<meta http-equiv='refresh' content='5; url=./'/>
			<title>My Bad</title>
			<style>
			  body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
			</style>
		  </head>
		  <body>
			<h1>Heating {v}</h1>
		  </body>
		</html>
)=====";

const char HTML_page_MAIN[] PROGMEM = R"=====(
		<html>
		  <head>
			<meta http-equiv='refresh' content='5'/>
			<meta http-equiv='pragma' content='no-cache' />
			<title>My Bad</title>
			<style> 
				body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
				.button {background-color: #bbb; display: block; margin: 10px 0; padding: 10px; width: 100% ;} 
			</style>
		  </head>
		  <body>
			<h1>My Bad</h1>
			<h2>Temp: {t}&deg; <br>Humidity: {h}&#37;</h2>
			<a href='./on'  > <button class='button'> ON  </button></a> 
			<a href='./off' > <button class='button'> OFF </button></a>
		  </body>
		</html>
)=====";

ESP8266WebServer server(80);


extern float temperature;
extern int humidity;

extern uint8_t ssr1_power;
extern uint8_t ssr2_power;

void handleRoot() {
/*

	String page = FPSTR(HTTP_HEAD);
	page.replace("{v}", "My Bad");
	page += FPSTR(HTTP_SCRIPT);
	page += FPSTR(HTTP_STYLE);
	page += FPSTR(HTTP_HEAD_END);
	page += F("<dl>");
	page += F("<dt>Temperature</dt><dd>");
	page += (int)temperature;
	page += F("</dd>");
	page += F("<dt>Humidity</dt><dd>");
	page += (int)humidity;
	page += F("</dd>");
	page += F("</dl>");
	page += FPSTR(HTTP_SCAN_LINK);
	page += FPSTR(HTTP_END);
	*/

	String page = HTML_page_MAIN;
	page.replace("{t}", (String)(int)temperature);
	page.replace("{h}", (String)      humidity);
	server.sendHeader("Content-Length", String(page.length()));
	server.send(200, "text/html", page);
}

void handleOn() {
	ssr1_power = 100;
	ssr2_power = 100;

	String page = HTML_page_ON_OFF;
	page.replace("{v}", "ON");
	server.sendHeader("Content-Length", String(page.length()));
	server.send(200, "text/html", page);
}

void handleOff() {
	ssr1_power = 0;
	ssr2_power = 0;

	String page = HTML_page_ON_OFF;
	page.replace("{v}", "OFF");
	server.sendHeader("Content-Length", String(page.length()));
	server.send(200, "text/html", page);

}

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

void WebServerHandleClient()
{
	server.handleClient();
}

void WebServerBegin(const char * host)
{
	if (MDNS.begin(host)) {
		Serial.println("MDNS responder started");
	}

	server.on("/", handleRoot);
	server.on("/on", handleOn);
	server.on("/off", handleOff);
	server.on("/inline", []() {
		server.send(200, "text/plain", "this works as well");
	});
	server.onNotFound(handleNotFound);
	server.begin();
	Serial.println("HTTP server started");
}
