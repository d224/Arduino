#define DEBUG_ESP_OTA
#define DEBUG_ESP_PORT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "DRV8825.h" //https://github.com/RobTillaart/DRV8825
                      //small
                      //https://www.pololu.com/product/2133
                      // 3.75 degree

                      //big
                      //17H20HM-0404A - 0.9 Degree - 400 ppr

bool bConnected = false;
const char* host = "M_Clock";
WiFiMulti wifiMulti;

DRV8825 stepper;

#define DIR_FORWARD DRV8825_COUNTERCLOCK_WISE
#define DIR_BACKWARD DRV8825_CLOCK_WISE

const int STEP_PIN      = 11;
const int DIRECTION_PIN = 12;
const int M0 = 3;
const int M1 = 5;
const int M2 = 7;
const int MEN = 1;
#define STEP_X DRV8825_STEP_1x32
#define STEPS_PerRotation ( 400 * STEP_X  )
#define STEPS_PerHH ( STEPS_PerRotation * 72 / 30 )  // 30720
#define STEPS_PerMM ( STEPS_PerHH / 60 ) 

const int IR_ADC = 2;
const int IR_INT = 4;

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());
  bConnected = true;
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  bConnected = false;
  //Serial.println("Trying to Reconnect");
  //WiFi.begin(ssid, password);
}

hw_timer_t *Timer0_Cfg = NULL;
 
uint do_steps = 0; //STEPS_PerHour;
void IRAM_ATTR Timer0_ISR()
{
  if( do_steps )
  {
        stepper.step();
        do_steps--;
  }
}

#define IR_TH 500
# define IR_DETECTED ( ir_adc < IR_TH )

void DoStep()
{
  if( stepper.getDirection() == DIR_FORWARD )
    do_steps++;
  else 
    do_steps--;
  stepper.step();
  //delay(1);
  delayMicroseconds(50);
}

void DoSteps( int n )
{
    for(int i=0; i<n; i++)
      DoStep();
}

int step_end;
int step_start;
void found_hh_mm()
{

  //1. found area
  int ir_adc = analogRead(IR_ADC);
  while(  !IR_DETECTED )
  {
    DoStep();
    ir_adc = analogRead(IR_ADC);
  }
  timerAlarmDisable(Timer0_Cfg);
  Serial.printf("1.[%d] - %d\n", do_steps, ir_adc ) ;
  //delay(1000);

  //2. found start
  stepper.setDirection( DIR_BACKWARD );
  while( IR_DETECTED )
  {
    DoStep();
    ir_adc = analogRead(IR_ADC);
  }
  step_start = do_steps;
  Serial.printf("2.[%d] - %d\n", do_steps, ir_adc ) ;

  //3. found end
  stepper.setDirection( DIR_FORWARD );
  while( !IR_DETECTED )
  {
    DoStep();
    ir_adc = analogRead(IR_ADC);
  }

  while( IR_DETECTED || (do_steps - step_start) < 10 )
  {
    DoStep();
    Serial.printf("3.[%d] - %d\n", do_steps, ir_adc ) ;
    ir_adc = analogRead(IR_ADC);
  }
  step_end = do_steps;


  Serial.printf("found in range %d - %d\n", step_start, step_end ) ;

/*
  for(int i=0; i<60; i++)
  {
    do_steps = STEPS_PerMM ;
    delay(1000);
  }
  */
}

void calib()
{

  // found first
  do_steps = 0;
  found_hh_mm();
  delay(1000);

  do_steps = ( step_end - step_start ) / 2;
  DoSteps( STEPS_PerMM * 5 );

  // found next
  for (int i=0; i<15; i++)
  {
    found_hh_mm();
    int pos = step_start + ( step_end - step_start ) / 2;
    if( abs( STEPS_PerHH - pos) < 100 )
    {
      Serial.printf("MM pos %d (%d)\n", pos, abs( STEPS_PerHH - pos) ) ;     
    }
    else 
    {
      if( abs( (STEPS_PerHH / 2) - pos) < 2000 )
      {
        Serial.printf("HH pos %d (%d)\n", pos,  (STEPS_PerHH / 2) - pos ) ; 
        int d = (STEPS_PerHH / 2) - step_end;
        delay(1000);
        if( d > 0 )
          DoSteps( d );

        return;
      }
      else
      {
        Serial.printf("Wrong %d \n", pos ) ; 
      }
    }
    delay(1000);    
    do_steps = ( step_end - step_start ) / 2;
    for(int i=0; i < STEPS_PerMM * 5 ; i++)
      DoStep();
  }
  // found second


  //do_steps = STEPS_PerHH;
}

void setup(void) 
{
  Serial.begin(115200);
  //while (!Serial && !Serial.available()) {}
  //delay(100);
  delay(3000);
  Serial.printf("****Start %s****\n", host);
  pinMode(LED_BUILTIN, OUTPUT);
/*
  WiFi.disconnect(true);
  delay(1000);

  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);


  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN", "1qazxsw2");
  wifiMulti.addAP("D224", "1qazxsw2");


  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected: ");
    Serial.println(WiFi.localIP());
  }

  ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    ESP.restart();
  });

  ArduinoOTA.begin();
*/
  
  stepper.begin(DIRECTION_PIN, STEP_PIN, M0, M1, M2); //, MEN

  //pinMode(FORWARD_PIN, INPUT_PULLUP);
  //pinMode(BACKWARD_PIN, INPUT_PULLUP);

  stepper.setMode( STEP_X );
  //stepper.enable();

  stepper.setDirection( DIR_FORWARD );
  stepper.setStepsPerRotation( STEPS_PerRotation ); 

  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 500, true);  // 1000 = 1ms

  pinMode(IR_INT, INPUT);
  calib();

}


void loop(void) 
{
  /*
    //  read both buttons
  bool fw = digitalRead(FORWARD_PIN) == LOW;
  bool bw = digitalRead(BACKWARD_PIN) == LOW;

  // process the button state
  if (fw && bw)  //  both buttons pressed
  {
     Serial.println("not allowed, please release both buttons.");
     delay(1000);  //  block  
  }
  else if (fw)
  {
    stepper.setDirection(DRV8825_CLOCK_WISE);
    delay(50);
    stepper.step();
    delay(50);
  }
  else if (bw)
  {
    stepper.setDirection(DRV8825_COUNTERCLOCK_WISE);
    delay(50);
    stepper.step();
    delay(50);
  }
  */
  //1*96*7.2 = 691.2 step/3600 sec => 1 step 5.795235028976175 sec
  //32*96*7.2 = 22118.4/3600 sec => 0.0162760416666667
  //4*STEPS_PerRotation*72/30 =  921.6/3600 sec => 0.256 


  //stepper.step();
  //delay(162);
  //delay(1);

/*
  // 72x10
  float ppm = ( 7.2 * 6 / 3.75 * STEP_X );     //3.75 degree
  int  pT = 0;
  int  dp;
  for(int i=1; i<=60; i++)
  {
    dp = ( ppm * i ) - pT;
    pT += dp;
    Serial.printf("[%d]-%d-%d\n", i, dp, pT);
    for(; dp; dp--)
    {
      stepper.step();
      delay(5);
    }
    delay(1000);
  }
  */

   // Serial.println("*");
   // stepper.step();

    //do_steps = STEPS_PerMM ;
    delay(1000);


    //int ir_adc = analogRead(IR_ADC);
    //int ir_int = digitalRead(IR_INT)
    //Serial.printf("[%d] - %d-%d\n", do_steps, ir_int, ir_adc ) ;
    //delay(100);
    //digitalWrite(LED_BUILTIN, HIGH);
    //delay(1000);
    //digitalWrite(LED_BUILTIN, LOW);

}