#ifndef motor_h
#define motor_h

#include <Arduino.h>
void setup_motors();

typedef enum 
{
  MOTOR_POS_NA,
  MOTOR_POS_A,
  MOTOR_POS_B,
} motor_pos_e;

typedef enum 
{
  MOTOR_STOP,
  MOTOR_GOTO_A,
  MOTOR_GOTO_B,
} motor_op_e;

#define M_OPEN  true
#define M_CLOSE false

class Motor
{
  public:
    Motor( uint8_t PA, uint8_t PB, uint8_t LimitSW_A,  uint8_t LimitSW_B);
    void Set(motor_op_e pos);
    motor_pos_e ReadPos();
    String Pos2Str(motor_pos_e pos);
    void PrintStatus();
    void Init();
    
  private:
    uint8_t m_PA;
    uint8_t m_PB;
    uint8_t m_LimitSW_A;
    uint8_t m_LimitSW_B;
    motor_pos_e m_pos;
    motor_op_e  m_op;
};

#endif //motor_h