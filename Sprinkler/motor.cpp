#include "motor.h"
#include "Sprinkler.h"
#include "EEPROM_data.h"
#include "WebTime.h"

#define M_OPEN  true
#define M_CLOSE false


uint32_t nNextMotor = NOT_SET;
uint32_t nCurrentMotorTimeLeftSec = 0;
int8_t   nCurrentMotorIndex = -1;

uint32_t mmPrev = 0;

Motor::Motor(uint8_t index, uint8_t PA, uint8_t PB)
{
  m_index = index;
  m_PA = PA;
  m_PB = PB;

  pinMode(PA, OUTPUT);
  pinMode(PB, OUTPUT);
}

void Motor::Set(bool open)
{
  Serial.printf("Motor %d", m_index);
  if (open)
  {
    digitalWrite(m_PA, HIGH);
    digitalWrite(m_PB, LOW);
    Serial.println("->OPEN");
    nCurrentMotorIndex = m_index;
  }
  else
  {
    digitalWrite(m_PA, LOW);
    digitalWrite(m_PB, HIGH);
    Serial.println("->CLOSE");
    nCurrentMotorIndex = -1;
  }

  delay(100);
  digitalWrite(m_PA, LOW);
  digitalWrite(m_PB, LOW);  
}

Motor Motors[] = {
  Motor(0, 19, 18),
  Motor(1,  5, 17),
  Motor(2,  4, 0 ),
  Motor(3,  2, 15),
};

void open_valwe(uint8_t m, const uint32_t open_time_sec)
{

  Motors[m].Set(M_OPEN);

  for (nCurrentMotorTimeLeftSec = open_time_sec; nCurrentMotorTimeLeftSec > 0; nCurrentMotorTimeLeftSec--)
    delay(1000);

  Motors[m].Set(M_CLOSE);
}

void motor_op()
{
  uint32_t mm = WebTime::m_secSinceMidNight / 60;
  if( mmPrev != mm )
  {
    mmPrev = mm ;
    for( uint8_t m=0; m<CH_NUM; m++)
    {
      Serial.printf("M%d, A:%d, ON:%d, d:%d \n", m, 
      data.ch[m].active,
      data.ch[m].time_ON,
      data.ch[m].duration );
      if( data.ch[m].active && data.ch[m].time_ON >= mm )
      {
        uint32_t delta = data.ch[m].time_ON - mm;
        if( nNextMotor > delta )
          nNextMotor = delta;
        if( delta == 0 ) // time to open ! :)))
        {
          open_valwe(m, data.ch[m].duration * 60 );
          nNextMotor = NOT_SET;
        }
      }
    }
  }
  delay(1000);
}

void test(uint8_t m)
{
  if( nCurrentMotorIndex == m )
  {
    Serial.printf("Test m%d CLOSE \n", m);
    Motors[m].Set(M_CLOSE);
    return;
  }

  if( nCurrentMotorIndex == -1 )
  {
    Serial.printf("Test m%d OPEN \n", m);
    Motors[m].Set(M_OPEN);
    return;
  }

  Serial.printf("ignored, first close m%d \n", nCurrentMotorIndex);
}

void motor_test()
{
    if( IS_PRESSED(BTN_M0) )
    {
      test(0);    
      delay(1000);     
    }
    if( IS_PRESSED(BTN_M1) )
    {
      test(1);    
      delay(1000);     
    }
    if( IS_PRESSED(BTN_M2) )
    {
      test(2);    
      delay(1000);     
    } 
    if( IS_PRESSED(BTN_M3) )
    {
      test(3);    
      delay(1000);     
    }   
    delay(100);        
}
void motors_close_all()
{
  for( uint8_t m=0; m<CH_NUM; m++)
  {
    Motors[m].Set(M_CLOSE);
    delay(100);
  }
}

void taskMotors( void * parameter)
{
  motors_close_all();
  delay(3000);

  while (! WebTime::isValid() )
   delay(1000);

  for(;;)
  {
    switch(Mode)
    {
      case MODE_INIT:
        delay(1000);
        break;
      case MODE_OPERATIONAL:
        motor_op();
        break;
      case MODE_TEST:
        motor_test();
        break;
    }
  }
}


/*
void set_motor(const uint8_t* m, bool bOnOff)
{
  Serial.print("Motor ");
  Serial.print(m[P_NAME]);

  if (bOnOff)
  {
    digitalWrite(m[PA], HIGH);
    digitalWrite(m[PB], LOW);
    Serial.println("->OPEN");
  }
  else
  {
    digitalWrite(m[PA], LOW);
    digitalWrite(m[PB], HIGH);
    Serial.println("->CLOSE");
  }

  delay(100);
  digitalWrite(m[PA], LOW);
  digitalWrite(m[PB], LOW);
}
*/