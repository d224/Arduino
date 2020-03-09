#include "WebServer.h"
#include "EEPROM_data.h"

AsyncWebServer server(80);

// Default Threshold Temperature Value
String strCH0_duration = "15";
String strCH1_duration = "20";
String strCH0_Checked = "checked";
String strCH1_Checked = "checked";
String strCH0_TimeOn = "19:00";
String strCH1_TimeOn = "19:30";

String strCurrentTime;
extern void set_time(int sec); //in min
extern uint16_t moistureSensorVal;

String MinTime2Sring(uint16_t t )
{
	char res[]="00:00\0";
	uint8_t h = t / 60;
	uint8_t m = t % 60;
	res[0] = h / 10 + '0';
	res[1] = h % 10 + '0';
	res[3] = m / 10 + '0';
	res[4] = m % 10 + '0';	
	Serial.printf("MinTime2Sring %d -> %s\n", t, res);
	return String(res);
}

uint16_t Sring2MinTime(String s )
{	
	uint16_t res = 0;
	res += (s.charAt(0)-'0') * 600;
	res += (s.charAt(1)-'0') * 60;
	res += (s.charAt(3)-'0') * 10;
	res += (s.charAt(4)-'0');
	Serial.printf("Sring2MinTime %s -> %d \n", s.c_str(), res);
	return res;
}

// HTML web page to handle 2 input fields (ch0_duration, ch0_enable)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Temperature Threshold Output Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  

  <h2>Sprinkler</h2>
  <h2>MoistureSensor: %MOISTURE_SENSOR%</h2>
  <form action="/get">
	Time: <input type="time" id="time_id" name="current_time" value="00:00"> <br>
    <h2>Chanel A:</h2>
    Time On <input type="time" name="ch0_timeON" value=%CH0_TIME_ON%> <br>
    Duration <input type="number" step="1" name="ch0_duration" value="%CH0_DURATION%" required><br> 
    Active <input type="checkbox" name="ch0_enable" value="true" %CH0_ENABLE%><br>
    <h2>Chanel B:</h2>
    Time On <input type="time" name="ch1_timeON" value=%CH1_TIME_ON%> <br>
    Duration <input type="number" step="1" name="ch1_duration" value="%CH1_DURATION%" required><br> 
    Active <input type="checkbox" name="ch1_enable" value="true" %CH1_ENABLE%><br>	
    <br> <input type="submit" value="Submit">
	
    <script>
	  var date = new Date();
	  var currentTime = ((date.getHours()<10?'0':'') + date.getHours()) + ':' + ((date.getMinutes()<10?'0':'') + date.getMinutes());
	  document.getElementById("time_id").value = currentTime;  
    </script>	
	
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


// Replaces placeholder with DS18B20 values
String processor(const String& var){
  //Serial.println(var);
	if(var == "CH0_DURATION")
	{
		return String(data.ch[0].duration); //strCH0_duration;
	}
	else if(var == "CH1_DURATION")
	{
		return String(data.ch[1].duration); //strCH1_duration;
	} 
	else if(var == "CH0_ENABLE")
	{
		if(data.ch[0].active)  return String("checked");
		else return String("");
	}
	else if(var == "CH1_ENABLE")
	{
		if(data.ch[1].active)  return String("checked");
		else return String("");
	} 
	else if(var == "CH0_TIME_ON")
	{
		return MinTime2Sring(data.ch[0].time_ON);
	}
	else if(var == "CH1_TIME_ON")
	{
		return MinTime2Sring(data.ch[1].time_ON);
	}  
	else if(var == "MOISTURE_SENSOR")
	{
		return String(moistureSensorVal);
	}	
	
	
  return String();
}

void WebServerBegin() 
{
  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Receive an HTTP GET request at <ESP_IP>/get?ch0_duration=<strCH0_duration>&ch0_enable=<inputMessage2>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
	//String inputMessage2 = "true";
    // GET ch0_duration value on <ESP_IP>/get?ch0_duration=<strCH0_duration>
    if (request->hasParam("current_time")) 
	{
	  uint16_t nCurrentTime;
      strCurrentTime = request->getParam("current_time")->value();
	  nCurrentTime = Sring2MinTime(strCurrentTime);
	  Serial.printf("CurrentTime=%d min \n", nCurrentTime);
	  set_time(nCurrentTime); //in min
    }	
    if (request->hasParam("ch0_duration")) 
	{
      strCH0_duration = request->getParam("ch0_duration")->value();
	  data.ch[0].duration = strCH0_duration.toInt();
    }	  
    if (request->hasParam("ch1_duration")) 
	{
      strCH1_duration = request->getParam("ch1_duration")->value();
	  data.ch[1].duration = strCH1_duration.toInt();
    }	
    if (request->hasParam("ch0_timeON")) 
	{
      strCH0_TimeOn = request->getParam("ch0_timeON")->value();
	  data.ch[0].time_ON = Sring2MinTime(strCH0_TimeOn);
    }	  
    if (request->hasParam("ch1_timeON")) 
	{
      strCH1_TimeOn = request->getParam("ch1_timeON")->value();
	  data.ch[1].time_ON = Sring2MinTime(strCH1_TimeOn);
    }		
    // GET ch0_enable value on <ESP_IP>/get?ch0_enable=<inputMessage2>
    if (request->hasParam("ch0_enable")) 
	{
      //inputMessage2 = request->getParam("ch0_enable")->value();
      data.ch[0].active = 1;
      strCH0_Checked = "checked";
    }
    else 
	{
      //inputMessage2 = "false";
      data.ch[0].active = 0;
      strCH0_Checked = "";
    }
    if (request->hasParam("ch1_enable")) 
	{
      data.ch[1].active = 1;
      strCH1_Checked = "checked";
    }
    else {
      data.ch[1].active = 0;
      strCH1_Checked = "";
    }	  

    //Serial.println(strCH0_TimeOn);
    //Serial.println(inputMessage2);
    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
	
	WriteEEPROM((uint8_t*)&data, sizeof(data));  
  });

  server.onNotFound(notFound);
  server.begin();
}
