//
//    FILE: TMC2208.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.2.0
// PURPOSE: Arduino library for TMC2208 stepper motor driver
//    DATE: 2022-07-07
//     URL: https://github.com/RobTillaart/TMC2208
//
//17H20HM-0404A
//6.Motor type: 2-Phase 4-Wire (Red:A+;Yellow:A-; Orange:B+; Black:B-)

#include "TMC2208.h"


TMC2208::TMC2208()
{
}


bool TMC2208::begin(uint8_t DIR, uint8_t STEP, uint8_t EN,
                    uint8_t MS1, uint8_t MS2, uint8_t RST, uint8_t SLP)
{
  _directionPin = DIR;
  _stepPin      = STEP;

  pinMode(_directionPin, OUTPUT);
  pinMode(_stepPin, OUTPUT);
  digitalWrite(_directionPin, LOW);
  digitalWrite(_stepPin, LOW);

  if (MS1 != 255)
  {
    _ms1Pin = MS1;
    pinMode(_ms1Pin, OUTPUT);
    digitalWrite(_ms1Pin, LOW);
  }

  if (MS2 != 255)
  {
    _ms2Pin = MS2;
    pinMode(_ms2Pin, OUTPUT);
    digitalWrite(_ms2Pin, LOW);
  }  

  //  handle conditional parameters
  if (EN != 255)
  {
    _enablePin = EN;
    pinMode(_enablePin, OUTPUT);
    digitalWrite(_enablePin, LOW);   //  page 3
  }
  if (RST != 255)
  {
    _resetPin = RST;
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, HIGH);   //  page 3
  }
  if (SLP != 255)
  {
    _sleepPin = SLP;
    pinMode(_sleepPin, OUTPUT);
    digitalWrite(_sleepPin, HIGH);   //  page 3
  }
  return true;
}


void TMC2208::setStepsPerRotation(uint32_t stepsPerRotation)
{
  _stepsPerRotation = stepsPerRotation;
}


uint32_t TMC2208::getStepsPerRotation()
{
  return _stepsPerRotation;
}


bool TMC2208::setDirection(uint8_t direction)
{
  if (direction > 1) return false;
  _direction = direction;
  //  timing from datasheet 650 ns figure 1
  delayMicroseconds(1);
  digitalWrite(_directionPin, _direction);
  delayMicroseconds(1);
  return true;
}


uint8_t TMC2208::getDirection()
{
  return digitalRead(_directionPin);
}


void TMC2208::step()
{
  digitalWrite(_stepPin, HIGH);
  if (_stepPulseLength > 0) delayMicroseconds(_stepPulseLength);
  digitalWrite(_stepPin, LOW);
  if (_stepPulseLength > 0) delayMicroseconds(_stepPulseLength);

  _steps++;
  if (_stepsPerRotation > 0)
  {
    if (_direction == DIR_FORWARD)
    {
      _position++;
      if (_position >= _stepsPerRotation)   _position = 0;
    }
    else
    {
      if (_position == 0)  _position = _stepsPerRotation;
      _position--;
    }

    //Serial.printf("*** pos %d \n" , _position);
  }
}


uint32_t TMC2208::resetSteps(uint32_t s)
{
  uint32_t t = _steps;
  _steps = s;
  return t;
}


uint32_t TMC2208::getSteps()
{
  return _steps;
}


void TMC2208::setStepPulseLength(uint16_t stepPulseLength)
{
  _stepPulseLength = stepPulseLength;
}


uint16_t TMC2208::getStepPulseLength()
{
  return _stepPulseLength;
}


bool TMC2208::setPosition(uint32_t position)
{
  if (position >= _stepsPerRotation) return false;
  _position = position;
  return true;
}


uint32_t TMC2208::getPosition()
{
  return _position;
}

//  Table page 3
bool TMC2208::enable()
{
  if (_enablePin == 255) return false;
  digitalWrite(_enablePin, LOW);
  return true;
}

bool TMC2208::disable()
{
  if (_enablePin == 255) return false;
  digitalWrite(_enablePin, HIGH);
  return true;
}

bool TMC2208::isEnabled()
{
  if (_enablePin != 255)
  {
    return (digitalRead(_enablePin) == LOW);
  }
  return true;
}


bool TMC2208::reset()
{
  if (_resetPin == 255) return false;
  digitalWrite(_resetPin, LOW);
  delay(1);
  digitalWrite(_resetPin, HIGH);
  return true;
}


bool TMC2208::sleep()
{
  if (_sleepPin == 255) return false;
  digitalWrite(_sleepPin, LOW);
  return true;
}

bool TMC2208::wakeup()
{
  if (_sleepPin == 255) return false;
  digitalWrite(_sleepPin, HIGH);
  return true;
}

bool TMC2208::isSleeping()
{
  if (_sleepPin != 255)
  {
    return (digitalRead(_sleepPin) == LOW);
  }
  return false;
}

bool TMC2208::setMode(uint16_t mode)
{
  switch (mode) 
  {
    case TMC2208_STEP_1x8:
        digitalWrite(_ms1Pin, LOW);
        digitalWrite(_ms2Pin, LOW);
        break;
    case TMC2208_STEP_1x2:
        digitalWrite(_ms1Pin, HIGH);
        digitalWrite(_ms2Pin, LOW);
        break;
    case TMC2208_STEP_1x4:
        digitalWrite(_ms1Pin, LOW);
        digitalWrite(_ms2Pin, HIGH);
        break;
    case TMC2208_STEP_1x16:
        digitalWrite(_ms1Pin, HIGH);
        digitalWrite(_ms2Pin, HIGH);
        break;

     
    default:
      return false;
  }
  return true;
}

//  -- END OF FILE --

