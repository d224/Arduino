#include "WebServer.h"
#include "EEPROM_data.h"

AsyncWebServer server(80);

// Default Threshold Temperature Value
String strCH0_duration = "15";
String strCH1_duration = "20";
String strCH2_duration = "20";
String strCH3_duration = "20";

String strCH0_TimeOn = "19:00";
String strCH1_TimeOn = "19:30";
String strCH2_TimeOn = "20:00";
String strCH3_TimeOn = "20:30";

String strCH0_Checked = "checked";
String strCH1_Checked = "checked";
String strCH2_Checked = "checked";
String strCH3_Checked = "checked";

String strCurrentTime;
extern void set_time(int sec); //in min
uint16_t moistureSensorVal = 0;

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
  <style>
html {font-family: Times New Roman; display: inline-block; }
h2 {font-size: 2.3rem;}
p {font-size: 1.9rem;}
body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
.slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #38c0ff; outline: none; -webkit-transition: .2s; transition: opacity .2s;}
.slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background:#01070a; cursor: pointer;}
.slider::-moz-range-thumb { width: 35px; height: 35px; background: #01070a; cursor: pointer; } 
.toggle {position: relative; display: inline-block; width: 120px; height: 68px} 
.toggle input {display: none;}
.shade {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
.shade:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}   
input:checked+.shade {background-color: #2196F3}
input:checked+.shade:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)} 
  </style>
  </head><body>
  

  <h2>Sprinkler</h2>
  <h2>MoistureSensor: %MOISTURE_SENSOR%</h2>
  <form action="/get">
	
    <h2>Chanel A:</h2>
    Time On <input type="time" name="ch0_timeON" value=%CH0_TIME_ON%> <br>
    Duration <input type="number" maxlength="4" step="1" name="ch0_duration" value="%CH0_DURATION%" required><br> 
    Active <input type="checkbox" name="ch0_enable" value="true" %CH0_ENABLE%><br>

    <h2>Chanel B:</h2>
    Time On <input type="time" name="ch1_timeON" value=%CH1_TIME_ON%> <br>
    Duration <input type="number" step="1" name="ch1_duration" value="%CH1_DURATION%" required><br> 
    Active <input type="checkbox" name="ch1_enable" value="true" %CH1_ENABLE%><br>	

    <h2>Chanel C:</h2>
    Time On <input type="time" name="ch2_timeON" value=%CH2_TIME_ON%> 
    <div class="slidecontainer">
    <input type="range" min="1" max="60" class="slider" name="ch2_duration" id="ch2_duration" value="%CH2_DURATION%" >
    <p>Duration: <span id="ch2_duration_val"></span></p>
    </div>
    <script>
      var slider2 = document.getElementById("ch2_duration");
      var output2 = document.getElementById("ch2_duration_val");
      output2.innerHTML = slider2.value;
      slider2.oninput = function() { output2.innerHTML = this.value;}
    </script>
    Active <input type="checkbox" name="ch2_enable" value="true" %CH2_ENABLE% ><br>	
    
    <h2>Chanel D:</h2>
    Time On <input type="time" name="ch3_timeON" value=%CH3_TIME_ON%> 
    <div class="slidecontainer">
    <input type="range" min="1" max="60" class="slider" name="ch3_duration" id="ch3_duration" value="%CH3_DURATION%" >
    <p>Duration: <span id="ch3_duration_val"></span></p>
    </div>
    <script>
      var slider3 = document.getElementById("ch3_duration");
      var output3 = document.getElementById("ch3_duration_val");
      output3.innerHTML = slider3.value;
      slider3.oninput = function() { output3.innerHTML = this.value;}
    </script>
    <label class="toggle"><input type="checkbox" onchange="toggleCheckbox" name="ch3_enable" value="true"  %CH3_ENABLE% ><span class="shade"></span>    </label>		

    <br>	
    <br> <input type="submit" value="Submit">    
	
	
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
	else if(var == "CH2_DURATION")
	{
		return String(data.ch[2].duration); //strCH2_duration;
	}   
	else if(var == "CH3_DURATION")
	{
		return String(data.ch[3].duration); //strCH3_duration;
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
	else if(var == "CH2_ENABLE")
	{
		if(data.ch[2].active)  return String("checked");
		else return String("");
	} 
	else if(var == "CH3_ENABLE")
	{
		if(data.ch[3].active)  return String("checked");
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
 	else if(var == "CH2_TIME_ON")
	{
		return MinTime2Sring(data.ch[2].time_ON);
	}
 	else if(var == "CH3_TIME_ON")
	{
		return MinTime2Sring(data.ch[3].time_ON);
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
 /*   
    if (request->hasParam("current_time")) 
	{
	  uint16_t nCurrentTime;
      strCurrentTime = request->getParam("current_time")->value();
	  nCurrentTime = Sring2MinTime(strCurrentTime);
	  Serial.printf("CurrentTime=%d min \n", nCurrentTime);
	  set_time(nCurrentTime); //in min
    }	
 */   
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
  if (request->hasParam("ch2_duration")) 
	{
    strCH1_duration = request->getParam("ch2_duration")->value();
	  data.ch[2].duration = strCH2_duration.toInt();
  }	
  if (request->hasParam("ch3_duration")) 
	{
    strCH3_duration = request->getParam("ch3_duration")->value();
	  data.ch[3].duration = strCH1_duration.toInt();
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
  if (request->hasParam("ch2_timeON")) 
	{
      strCH2_TimeOn = request->getParam("ch2_timeON")->value();
	  data.ch[2].time_ON = Sring2MinTime(strCH2_TimeOn);
  }	
  if (request->hasParam("ch3_timeON")) 
	{
      strCH3_TimeOn = request->getParam("ch3_timeON")->value();
	  data.ch[3].time_ON = Sring2MinTime(strCH3_TimeOn);
  }	    		
    // GET ch0_enable value on <ESP_IP>/get?ch0_enable=<inputMessage2>
  if (request->hasParam("ch0_enable")) 
	{
    data.ch[0].active = 1;
    strCH0_Checked = "checked";
  }
  else 
	{
    data.ch[0].active = 0;
    strCH0_Checked = "";
  }
  if (request->hasParam("ch1_enable")) 
	{
    data.ch[1].active = 1;
    strCH1_Checked = "checked";
  }
  else 
  {
    data.ch[1].active = 0;
    strCH1_Checked = "";
  }
  if (request->hasParam("ch2_enable")) 
	{
    data.ch[2].active = 1;
    strCH2_Checked = "checked";
  }
  else 
  {
    data.ch[2].active = 0;
    strCH2_Checked = "";
  }	   	  
  if (request->hasParam("ch3_enable")) 
	{
    data.ch[3].active = 1;
    strCH3_Checked = "checked";
  }
  else 
  {
    data.ch[3].active = 0;
    strCH3_Checked = "";
  }	 
    //Serial.println(strCH0_TimeOn);
    //Serial.println(inputMessage2);
  request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
	
	WriteEEPROM((uint8_t*)&data, sizeof(data)); 
  printEEPROM_Data(); 
  });

  server.onNotFound(notFound);
  server.begin();
}
