#ifndef motor_h
#define motor_h

#include <Arduino.h>
void setup_motors();

void taskMotors( void * parameter);

#define NOT_SET (60*24)
extern uint32_t nNextMotor; // time delta to next on in min
extern uint32_t nCurrentMotorTimeLeftSec;
extern int8_t   nCurrentMotorIndex;

void motors_close_all();

class Motor
{
  public:
    Motor(uint8_t index, uint8_t PA, uint8_t PB);
    void Set(bool open);
  private:
    uint8_t m_index;
    uint8_t m_PA;
    uint8_t m_PB;
};

#endif //motor_h