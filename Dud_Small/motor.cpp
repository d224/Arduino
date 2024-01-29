#include "motor.h"

motor_pos_e Motor::ReadPos()
{
  uint8_t LimitSW_A = digitalRead(m_LimitSW_A);
  uint8_t LimitSW_B = digitalRead(m_LimitSW_B);

  m_pos = MOTOR_POS_NA;

  if( LimitSW_A == 0 && LimitSW_B == 1 )
    m_pos = MOTOR_POS_A;

  if( LimitSW_A == 1 && LimitSW_B == 0 )
    m_pos = MOTOR_POS_B;

  return m_pos;
}

Motor::Motor(uint8_t PA, uint8_t PB, uint8_t LimitSW_A,  uint8_t LimitSW_B)
{
  m_PA = PA;
  m_PB = PB;
  m_LimitSW_A = LimitSW_A;
  m_LimitSW_B = LimitSW_B;
  m_pos = MOTOR_POS_NA;

  pinMode(PA, OUTPUT);
  pinMode(PB, OUTPUT);
  pinMode(LimitSW_A, INPUT_PULLUP);
  pinMode(LimitSW_B, INPUT_PULLUP);

  ReadPos();
}

void Motor::Init()
{
  ReadPos();
  PrintStatus();

  switch(m_pos)
  {
    case MOTOR_POS_NA :
      Set(MOTOR_GOTO_B);
      Set(MOTOR_GOTO_A); 
      break;
    case MOTOR_POS_A :
      Set(MOTOR_GOTO_B);
      break;
    case MOTOR_POS_B :
      Set(MOTOR_GOTO_A); 
      break;
  }
  PrintStatus();  
}

String Motor::Pos2Str(motor_pos_e pos)
{
  switch(pos)
  {
    case MOTOR_POS_NA : return "MOTOR_POS_NA";
    case MOTOR_POS_A : return "MOTOR_POS_A";
    case MOTOR_POS_B : return "MOTOR_POS_B";
  }
  return "";
}

String Op2Str(motor_op_e op)
{
  switch(op)
  {
    case MOTOR_STOP : return "MOTOR_STOP";
    case MOTOR_GOTO_A : return "MOTOR_GOTO_A";
    case MOTOR_GOTO_B : return "MOTOR_GOTO_B";
  }
  return "";
}

void Motor::PrintStatus()
{
  Serial.printf("Motor: %s %s \n", Op2Str(m_op), Pos2Str(m_pos));
}

void Motor::Set(motor_op_e op)
{
  Serial.printf("CMD: %s", Op2Str(op));
  switch(op)
  {
    case MOTOR_STOP:
    break;

    case MOTOR_GOTO_A:
      if( m_pos == MOTOR_POS_A )
        return;
      m_op = MOTOR_GOTO_A;
      digitalWrite(m_PA, HIGH);
      digitalWrite(m_PB, LOW);  
    break;

    case MOTOR_GOTO_B:
      if( m_pos == MOTOR_POS_B )
        return;
      m_op = MOTOR_GOTO_B;
      digitalWrite(m_PA, LOW);
      digitalWrite(m_PB, HIGH);
    break;
  }


  for(uint8_t i=0; i< 350; i++)
  {
    ReadPos();
    PrintStatus();

    if( m_op == MOTOR_GOTO_A && m_pos == MOTOR_POS_A )
    {
      Serial.println("MOTOR_POS_A done");
      break;
    }

    if( m_op == MOTOR_GOTO_B && m_pos == MOTOR_POS_B )
    {
      Serial.println("MOTOR_POS_B done");
      break;
    }

    delay(100);
  }

  digitalWrite(m_PA, LOW);
  digitalWrite(m_PB, LOW); 
  m_op = MOTOR_STOP; 
  PrintStatus();

}

