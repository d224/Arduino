/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-thermostat-web-server/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#define SOFTAP
#define CH_NUM 2

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "EEPROM.h"


typedef struct {
  uint16_t time_ON;
  uint8_t  len;
  uint8_t  active;
} chanel_setup;


typedef struct {
  chanel_setup ch[CH_NUM];
} EEPROM_struct;

#define EEPROM_SIZE sizeof(EEPROM_struct)
EEPROM_struct data;

void ReadEEPROM(uint8_t * p, uint16_t size)
{
	for(uint16_t i=0; i<size; i++)
		p[i] = EEPROM.readByte(i);
}
void WriteEEPROM(uint8_t * p, uint16_t size)
{
	for(uint16_t i=0; i<size; i++)
		EEPROM.writeByte(i, p[i]);
	EEPROM.commit();
}

String MinTime2Sring(uint16_t t )
{
	char res[]="00:00\0";
	uint8_t h = t / 60;
	uint8_t m = t % 60;
	res[0] = h / 10 + '0';
	res[1] = h % 10 + '0';
	res[2] = m / 10 + '0';
	res[4] = m % 10 + '0';	
	return String(res);
}

#ifdef SOFTAP
const char* ssid = "SPRINKLER";
const char* password = "12345678";
#else
const char* ssid = "4OBEZYAN";
const char* password = "1QAZXSW2";
#endif

// Default Threshold Temperature Value
String inputMessage = "25.0";
String lastTemperature;
String enableArmChecked = "checked";
String inputMessage2 = "true";

// HTML web page to handle 2 input fields (threshold_input, enable_arm_input)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Temperature Threshold Output Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>DS18B20 Temperature</h2> 
  <h3>%TEMPERATURE% &deg;C</h3>
  <h2>ESP Arm Trigger</h2>
  <form action="/get">
    Temperature Threshold <input type="number" step="0.1" name="threshold_input" value="%THRESHOLD%" required><br>
    Arm Trigger <input type="checkbox" name="enable_arm_input" value="true" %ENABLE_ARM_INPUT%><br>
    Time <input type="time" name="timeON" value="20:00"> <br>
    <br> <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);

// Replaces placeholder with DS18B20 values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return lastTemperature;
  }
  else if(var == "THRESHOLD"){
    return inputMessage;
  }
  else if(var == "ENABLE_ARM_INPUT"){
    return enableArmChecked;
  }
  return String();
}

// Flag variable to keep track if triggers was activated or not
bool triggerActive = false;

const char* PARAM_INPUT_1 = "threshold_input";
const char* PARAM_INPUT_2 = "enable_arm_input";

// Interval between sensor readings. Learn more about ESP32 timers: https://RandomNerdTutorials.com/esp32-pir-motion-sensor-interrupts-timers/
unsigned long previousMillis = 0;     
const long interval = 5000;    

// GPIO where the output is connected to
const int output = 2;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  
#ifdef SOFTAP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);
 
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  } 
  
  Serial.println("Set softAPConfig");
  IPAddress Ip(1, 1, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //.softAP(const char* ssid, const char* password, int channel, int ssid_hidden, int max_connection)
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());
#endif  

  ReadEEPROM((uint8_t *)&data, sizeof(data));
  inputMessage = String(data.ch[0].len);

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);
  
  // Start the DS18B20 sensor
  sensors.begin();
  
  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Receive an HTTP GET request at <ESP_IP>/get?threshold_input=<inputMessage>&enable_arm_input=<inputMessage2>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
	  
	  data.ch[0].len = inputMessage.toInt();
	  WriteEEPROM((uint8_t*)&data, sizeof(data));
	  
      // GET enable_arm_input value on <ESP_IP>/get?enable_arm_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        enableArmChecked = "checked";
      }
      else {
        inputMessage2 = "false";
        enableArmChecked = "";
      }
    }
    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sensors.requestTemperatures();
    // Temperature in Celsius degrees 
    float temperature = sensors.getTempCByIndex(0);
    Serial.print(temperature);
    Serial.println(" *C");
    
    // Temperature in Fahrenheit degrees
    /*float temperature = sensors.getTempFByIndex(0);
    Serial.print(temperature);
    Serial.println(" *F");*/
    
    lastTemperature = String(temperature);
    
    // Check if temperature is above threshold and if it needs to trigger output
    if(temperature > inputMessage.toFloat() && inputMessage2 == "true" && !triggerActive){
      String message = String("Temperature above threshold. Current temperature: ") + 
                            String(temperature) + String("C");
      Serial.println(message);
      triggerActive = true;
      digitalWrite(output, HIGH);
    }
    // Check if temperature is below threshold and if it needs to trigger output
    else if((temperature < inputMessage.toFloat()) && inputMessage2 == "true" && triggerActive) {
      String message = String("Temperature below threshold. Current temperature: ") + 
                            String(temperature) + String(" C");
      Serial.println(message);
      triggerActive = false;
      digitalWrite(output, LOW);
    }
  }
}
