
/*
https://www.tinytronics.nl/shop/en/sensors/liquid/yf-b6-water-flow-sensor-brass-g3-4
he output of the sensor gives 6.6 pulses per second with a duty cycle of approximately 50% for every liter of liquid that passes through it per minute: Q [L/min] = fpulse[Hz ]/6.6.

Specifications:
Supply voltage: 5-18V DC
Signal voltage: 4.7V (with 5V DC power supply)
Connector: JST-SM 3p female
Pulse frequency per L/min: 6.6Hz*
Duty cycle pulse: 50% ±10%
Measuring range: 1-30L/min
Accuracy: ±3%*
Water connection: G3/4"
Maximum water pressure: 17.5bar
Working temperature: 1-100°C
Suitable liquids: Water**


Pinout:
Red: Vcc/plus (supply voltage)
Black: GND/min
Yellow: Signal
*/

#include "FlowSensor.h"

#define pulseGPIO 4
volatile uint32_t pulseCnt = 0;
volatile uint32_t pulseCntOld = 0;

float calibrationFactor = 6.6; // 4.5;

void IRAM_ATTR ISR_pcnt() 
{
    pulseCnt++;
}

void FlowSensorTask( void * parameter) 
{
  for(;;)
  {
    delay (1000 );
    pulseCntOld = pulseCnt;
    pulseCnt = 0;
  }
}

void FlowSensorStart()
{
  pinMode(pulseGPIO, INPUT);
  attachInterrupt(pulseGPIO, ISR_pcnt, FALLING);
  xTaskCreatePinnedToCore( FlowSensorTask, "FlowSensorTask", 10240, NULL, 0, NULL, 1);  
}

float FlowSensorGetRate() // litres/minute
{
  return  pulseCntOld / calibrationFactor;
}
