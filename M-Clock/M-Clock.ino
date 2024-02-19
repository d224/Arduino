#define DEBUG_ESP_OTA
#define DEBUG_ESP_PORT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "WebTime.h"
#include "sntp.h"
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
WebTime webTime;

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
#define STEPS_PerMM ( STEPS_PerHH / 60 ) // 512

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

hw_timer_t *Timer1_Cfg = NULL;
 
uint do_steps = 0; //STEPS_PerHour;
void IRAM_ATTR Timer1_ISR()
{
  if( do_steps )
  {
        stepper.step();
        do_steps--;
  }
}

#define IR_TH 500
int ir_adc;
//#define IR_DETECTED ( ir_adc < IR_TH )

int IR_READ( int sample=10 )
{
  int adc=0;
  for( int i=0; i<sample; i++ )
    adc+=analogRead( IR_ADC );
  ir_adc = adc/sample;
  return  ir_adc;
}
int IR_DETECTED( int sample=10 )
{
  return ( IR_READ(sample) < IR_TH );
}



void DoStep( uint32_t delayMs = 50 )
{
  if( stepper.getDirection() == DIR_FORWARD )
    do_steps++;
  else 
    do_steps--;
  stepper.step();
  //delay(1);
  delayMicroseconds( delayMs );
}

void DoSteps( int n, uint32_t delayMs = 50 )
{
  if( n > 0 && stepper.getDirection() == DIR_BACKWARD )
    stepper.setDirection( DIR_FORWARD );

  if( n < 0 && stepper.getDirection() == DIR_FORWARD )
    stepper.setDirection( DIR_BACKWARD );
  
  for( int i=0; i<abs(n); i++ )
    DoStep( delayMs );
}

int step_end;
int step_start;
void found_hh_mm()
{

  //1. found area
  ir_adc = analogRead(IR_ADC);
  while(  !(ir_adc < IR_TH) )
  {
    DoStep();
    ir_adc = analogRead(IR_ADC);
  }
  timerAlarmDisable(Timer1_Cfg);
  Serial.printf("1.[%d] - %d\n", do_steps, ir_adc ) ;

  //2. found start
  stepper.setDirection( DIR_BACKWARD );
  while( IR_DETECTED() )
  {
    DoStep();
  }
  step_start = do_steps;
  Serial.printf("2.[%d] - %d\n", do_steps, ir_adc ) ;

  //3. found end
  stepper.setDirection( DIR_FORWARD );
  while( !IR_DETECTED() )
  {
    DoStep();
  }

  int _min = 0x1FFE;
  int _max = 0;
  int d;
  do
  {
    _min = min( _min, ir_adc);
    _max = max( _max, ir_adc);
    d = _max - _min;
    DoSteps( 10 );
    //Serial.printf("3.[%d] - %d\n", do_steps, ir_adc ) ;
  }while( IR_DETECTED() || ( d < 50 ) );

  step_end = do_steps;


  //Serial.printf("found in range %d - %d\n", step_start, step_end ) ;
  int width = step_end - step_start;
  Serial.printf("found in  %d with %d d=%d \n", step_start + width / 2 , width, d ) ;

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
  //delay(5000);

  do_steps = ( step_end - step_start ) / 2; // "0"
  DoSteps( STEPS_PerMM * 5 );

//  return;

  // found next
  for (int i=0; i<15; i++)
  {
    found_hh_mm();
    int pos = step_start + ( step_end - step_start ) / 2;
    int width = step_end - step_start;
    if( abs( STEPS_PerHH - pos ) < 100 && width < 1000 )
    {
      Serial.printf("MM pos %d (%d)\n", pos, abs( STEPS_PerHH - pos) ) ;     
    }
    else 
    {
      int delta = (STEPS_PerHH / 2) - pos;
      if( abs( delta ) < 2500  && width > 1000 )
      {
        Serial.printf("HH pos %d (%d)\n", pos,  delta ) ; 
        int d = (STEPS_PerHH / 2) - step_end;
        Serial.printf("Go %d", d);
        //delay(1000);
        DoSteps( d );
        return;
      }
      else
      {
        Serial.printf("Wrong %d (%d)\n", pos, delta ) ; 
        do_steps = ( step_end - step_start ) / 2; // "0"
      }
    }
    //delay(1000);    
    do_steps = ( step_end - step_start ) / 2;
    DoSteps(STEPS_PerMM * 25);
  }
  // found second
  //do_steps = STEPS_PerHH;
}

void webTimeloop()
{
  uint32_t timeWD = 0;
  for(int i = 0; i<1000; i++)
  {
    if( WebTime::isWiFi_connectNeeded() )
    {
      if( WiFi.status() != WL_CONNECTED )
      {
        Serial.println("Connecting Wifi...");
        if ( wifiMulti.run() != WL_CONNECTED )
        {
            Serial.print("!WL_CONNECTED\n");
            delay(8000);
        }       
      }
    }
    else  // !isWiFi_connectNeeded
    {
      if ( WiFi.status() == WL_CONNECTED )
      {
        Serial.print("WiFi close\n");
        WiFi.disconnect(true);      
        return;
      }  
    }

    if( WebTime::isValid())
    {
      timeWD = 0;
    }
    else 
    {
      timeWD ++;
      if( timeWD > 600)
      {
          Serial.print("Restart - mo time availible\n");
          ESP.restart();
      }
    }
    delay(1000);    
  }
}
uint8_t last_updated_mm; 
void setHHMM()
{
  uint8_t h = webTime._HH;
  uint8_t m = webTime._MM;
  int mm_cor;
  if( h >= 12 ) 
    h-=12;

  Serial.printf("Set %d:%d\n", h, m);

  if( h >= 6)
    mm_cor = m + (h - 6) * 60;
  else
    mm_cor = (-1) * ((60 - m) +  ((5-h) * 60));

  Serial.printf("MM correction  %d\n", mm_cor);

  last_updated_mm = m;
  DoSteps( STEPS_PerMM * mm_cor );

  if( webTime._MM != m )
  {
    last_updated_mm = m;
    if( webTime._MM > m )
      mm_cor = webTime._MM - m;
    else
      mm_cor = webTime._MM + 60 - m;

    Serial.printf("MM additional correction  %d\n", mm_cor);
    DoSteps( STEPS_PerMM * mm_cor );
  }


}

void setup(void) 
{
  Serial.begin(115200);

  delay(3000);
  Serial.printf("****Start %s****\n", host);
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.disconnect(true);
  
  stepper.begin(DIRECTION_PIN, STEP_PIN, MEN, M0, M1, M2); //, MEN

  //pinMode(FORWARD_PIN, INPUT_PULLUP);
  //pinMode(BACKWARD_PIN, INPUT_PULLUP);

  stepper.setMode( STEP_X );
  //stepper.enable();

  stepper.setDirection( DIR_FORWARD );
  stepper.setStepsPerRotation( STEPS_PerRotation ); 

  Timer1_Cfg = timerBegin(1, 80, true);
  timerAttachInterrupt(Timer1_Cfg, &Timer1_ISR, true);
  timerAlarmWrite(Timer1_Cfg, 500, true);  // 1000 = 1ms

  pinMode(IR_INT, INPUT);
  calib();

  webTime.Start();
  
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("D224_2.4G", "1qazxsw2");
  wifiMulti.addAP("4OBEZYAN", "1qazxsw2");
  wifiMulti.addAP("D224", "1qazxsw2");

  webTimeloop();

  setHHMM();
}



void loop(void) 
{
  if( last_updated_mm != webTime._MM )
  {
    stepper.enable();
    delay(1);
    DoSteps( STEPS_PerMM , 1000 );
    stepper.disable();
    last_updated_mm = webTime._MM;
  }
  delay(100);
}

void loop0(void) 
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