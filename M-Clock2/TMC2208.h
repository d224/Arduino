#pragma once
//
//    FILE: TMC2208.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.2.0
// PURPOSE: Arduino library for TMC2208 stepper motor driver
//    DATE: 2022-07-07
//     URL: https://github.com/RobTillaart/TMC2208


#include "Arduino.h"


#define TMC2208_LIB_VERSION              (F("0.2.0"))


//  setDirection
const uint8_t TMC2208_CLOCK_WISE        = 0;  //  
const uint8_t TMC2208_COUNTERCLOCK_WISE = 1;  //  

#define DIR_BACKWARD TMC2208_COUNTERCLOCK_WISE
#define DIR_FORWARD  TMC2208_CLOCK_WISE

#define TMC2208_STEP_1x2   2
#define TMC2208_STEP_1x4   4
#define TMC2208_STEP_1x8   8
#define TMC2208_STEP_1x16  16

class TMC2208
{
public:
  TMC2208();

  bool     begin(uint8_t DIR, uint8_t STEP, uint8_t EN = 255, uint8_t MS1 = 255, uint8_t MS2 = 255, uint8_t RST = 255, uint8_t SLP = 255);

  //       DIRECTION
  //       0 = TMC2208_CLOCK_WISE
  //       1 = TMC2208_COUNTERCLOCK_WISE
  //       returns false if parameter out of range.
  bool     setDirection(uint8_t direction = TMC2208_CLOCK_WISE);
  bool     setMode(uint16_t mode);
  uint8_t  getDirection();

  //       STEPS
  void     setStepsPerRotation(uint32_t stepsPerRotation);
  uint32_t getStepsPerRotation();
  void     step();
  uint32_t resetSteps(uint32_t s = 0 );
  uint32_t getSteps();

  //       POSITION
  //       only works if stepsPerRotation > 0
  //       returns false if position > stepsPerRotation.
  bool     setPosition(uint32_t position = 0);
  uint32_t getPosition();

  //       CONFIGURATION
  //       step pulse length is in microseconds
  //       datasheet default = 1.9 us
  void     setStepPulseLength(uint16_t stepPulseLength = 2);
  uint16_t getStepPulseLength();

  //       ENABLE pin should be set.
  bool     enable();
  bool     disable();
  bool     isEnabled();

  //       RESET pin should be set.
  bool     reset();

  //       SLEEP pin should be set.
  bool     sleep();
  bool     wakeup();
  bool     isSleeping();


protected:
  uint8_t  _directionPin     = 255;
  uint8_t  _stepPin          = 255;
  uint8_t  _enablePin        = 255;
  uint8_t  _resetPin         = 255;
  uint8_t  _sleepPin         = 255;
  uint8_t  _ms1Pin            = 255;
  uint8_t  _ms2Pin            = 255;

  uint8_t  _direction        = TMC2208_CLOCK_WISE;

  uint32_t _stepsPerRotation = 0;
  uint32_t _steps            = 0;
  uint32_t _position          = 0;
  uint16_t _stepPulseLength  = 2;
};


//  -- END OF FILE --


