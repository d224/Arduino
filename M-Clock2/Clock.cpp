#include <stdint.h>

#include <Arduino.h>
#include "WebTime.h"
#include "TMC2208.h" //https://github.com/RobTillaart/TMC2208
                      //17H20HM-0404A - 1.8 Degree - 200 ppr
TMC2208 stepper;


const int STEP_PIN      = 39;
const int DIRECTION_PIN = 37;
const int MS1 = 18;
const int MS2 = 33;
const int MEN = 16;
#define STEP_X TMC2208_STEP_1x16
#define STEPS_PerRotation ( 200 * STEP_X  ) // 16*200 = 3200
#define STEPS_PerHH ( STEPS_PerRotation )  
#define STEPS_PerMM ( STEPS_PerRotation / 60 ) // 53.333

const int SENSOR = 12;

int speed = 100;

void DoStep()
{
//  if( stepper.getDirection() == DIR_FORWARD )
 //   do_steps++;
 // else 
 //   do_steps--;
  if(!stepper.isEnabled())
    stepper.enable();
  stepper.step();
  delayMicroseconds(speed);
}

void DoSteps( int n )
{
  Serial.printf("DoSteps : %d from pos %d to pos %d \n", n, stepper.getPosition(),  stepper.getPosition() + n );
  if( n > 0)
    stepper.setDirection( DIR_FORWARD );
  else
    stepper.setDirection( DIR_BACKWARD ); 

  stepper.enable();
  for(int i=0; i<abs( n ); i++)
    DoStep();
  stepper.disable();

  Serial.printf("DoSteps done in pos: %d  \n", stepper.getPosition());
}


#define DETECTED 0
bool foundMagnet()
{
  uint16_t range = 0;
  stepper.setDirection( DIR_FORWARD );

  if( digitalRead(SENSOR) == DETECTED ) // go to outside of magnet
  {
    Serial.println("go to outside of magnet");
    while( digitalRead(SENSOR) == DETECTED ) 
      DoStep();   
    DoSteps( STEPS_PerMM );  // some more
  }


  while( digitalRead(SENSOR) != DETECTED )
   DoStep();

  Serial.println("in  - detected ");
  speed = 500;

  while( digitalRead(SENSOR) == DETECTED )
  {
    range ++;
    DoStep();
  }
 
  Serial.printf("out - detected at %d \n", range );

  if( (range > 5000) && (range < 6000) )
  {
    Serial.println("foundMagnet in range");
    DoSteps( range / (-2) );
    return true;
  }

  Serial.println("foundMagnet out of range");
  return false;

}

void FineTuning()
{
  if( digitalRead(SENSOR) != DETECTED ) 
  {
   Serial.println("FineTuning ERROR :(");   
   return;
  }
  
  Serial.println("FineTuning");
  speed = 500;
  uint16_t FT_Start = 0;
  uint16_t FT_Stop = 0;

  stepper.enable();
  stepper.setDirection( DIR_BACKWARD );
  while( digitalRead(SENSOR) == DETECTED )
  {
    DoStep();
    FT_Start++;
  }
  Serial.printf("FineTuning: A -%d \n", FT_Start );  

  stepper.setDirection( DIR_FORWARD );
  DoSteps(FT_Start); // go to "0"

  while( digitalRead(SENSOR) == DETECTED )
  {
    DoStep();
    FT_Stop++;
  }
  Serial.printf("FineTuning: B +%d \n", FT_Stop );  

  DoSteps((FT_Start + FT_Stop)/(-2));
  stepper.disable();

}

void DoInit()
{
  
  while (!foundMagnet());
  FineTuning();

  stepper.setPosition( 0 );
  stepper.setDirection( DIR_FORWARD );
  Serial.printf("DoInit Done \n");
  stepper.disable();
}

void GoTo( int n )
{
  n = n % STEPS_PerRotation;
  int delta = n - stepper.getPosition();
  DoSteps(delta);
}

void GoToMM(uint8_t MM)
{
  uint32_t pos = stepper.getPosition();
  uint32_t dest;
  if( MM == 0 && pos != 0 )
    dest = STEPS_PerRotation;
  else
    dest = MM * STEPS_PerMM;
  
  uint32_t delta = dest - pos;
  Serial.printf(" GoToMM [%d] from %d to %d: %d \n", MM, pos, dest, delta);
  DoSteps(delta);

}

void ClockTask( void * parameter)
{

  stepper.begin(DIRECTION_PIN, STEP_PIN, MEN, MS1, MS2); //, MEN
  stepper.setMode( STEP_X );
  stepper.setDirection( DIR_FORWARD );
  Serial.printf("STEPS_PerRotation %d \n" , STEPS_PerRotation);
  stepper.setStepsPerRotation( STEPS_PerRotation ); 

  pinMode(SENSOR, INPUT_PULLUP);
 
  DoInit();

  speed = 500;
  DoSteps( STEPS_PerMM * 13 );
  stepper.setPosition( 0 );
  Serial.println("***  00:00 ***\n");
  delay(10000);
 
  while( !WebTime::isValid() )
    delay(1000);

  speed = 200;
  int8_t HH = WebTime::_HH % 12;
  int8_t MM = WebTime::_MM;
  if( HH <= 6 )
    DoSteps( STEPS_PerHH * HH  ) ;
  else
    DoSteps( STEPS_PerHH * 12-HH * (-1) ) ;;

  Serial.printf("***  %02d:00 ***\n", HH);
  delay(1000);

  DoSteps( STEPS_PerMM * MM) ;
  speed = 10000;

  for(;;)
  {

      if(WebTime::isNewMin())
      {
        //Serial.print(" * move 1 min *\n");
        //GoTo( STEPS_PerMM * WebTime::_MM  ); 
        GoToMM(WebTime::_MM);       
      }

    delay(1000);
  }

}