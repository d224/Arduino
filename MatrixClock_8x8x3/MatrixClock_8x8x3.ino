//_C6 - 

#include <Arduino.h>
#include "Matrix8x8x3.h"

uint8_t hh = 0;
uint8_t mm = 0;
const int BUFFER_SIZE = 128;
char buf[BUFFER_SIZE];
uint32_t sleep_cnt = 0;

uint8_t dX = 12;// 0..25
uint8_t dY = 1; // 0..2

uint32_t _1sec = 0;
uint32_t color = COLOR(8, 0, 0);

#define WDT_TIMEOUT_MS 2000
#include <esp_task_wdt.h>

void myUART(void * pvParameters) 
{
  for(;;) 
  {
    delay( 100 );
    if ( Serial.available() >= 5 ) 
    {
      uint8_t len = Serial.readBytes( buf, BUFFER_SIZE );

      //Serial.printf("in: %d [%s]\n", len, buf);
      if( buf[2] == ':')
      {
        hh = (buf[0] - '0' ) * 10 + (buf[1] - '0' );
        mm = (buf[3] - '0' ) * 10 + (buf[4] - '0' );
        
        if( strlen(buf) > 5 && buf[5] == '-' )
        if( buf[6] == 'D' )
          color = COLOR(8, 0, 0);
        if( buf[6] == 'N' )
          color = COLOR(0, 0, 8);

        //dY = (mm / 20); //0..3
        //dX = 4; //mm % 8; // 0..7
        //drawClock(hh, mm, dX, (_1sec % 2), color);

        //responce://Serial.print(buf);
        
        Serial.print("OK");
        sleep_cnt = 0;
      }
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  delay(100);

  Matrixsetup();

  Serial.println();
  Serial.println("Matrix Com Display");

  Clear();
  printChar4x7('O', 8,   COLOR(8, 0, 0));
  printChar4x7('N', 12,  COLOR(8, 0, 0));
  Show();

  xTaskCreate(
    myUART,    // Task function
    "myUART",    // Task name
    16384,           // Stack size in bytes
    NULL,            // Parameters
    1,               // Priority
    NULL             // Task handle
  );


  // 1. Configure the Hardware Watchdog structure
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT_MS,
    .idle_core_mask = (1 << 0),   // ESP32-C6 is a single-core chip (Core 0)
    .trigger_panic = true         // True = Trigger system reset on timeout
  };

  // 2. Initialize the Watchdog with our custom config
  esp_err_t err = esp_task_wdt_init(&wdt_config);
  if (err == ESP_OK) {
    Serial.println("Hardware Watchdog successfully initialized.");
  } else {
    Serial.printf("Watchdog initialization failed: %s\n", esp_err_to_name(err));
  }

  // 3. Subscribe the current Arduino loop thread to the Watchdog
  esp_task_wdt_add(NULL); 
}

uint8_t dencity = 50;
void loop() 
{
  _1sec ++;
  sleep_cnt++;

  if( sleep_cnt < 30 )
  {
    //dX = 4; //mm % 8; // 0..7
    //drawClock(hh, mm, 4, (_1sec % 2), color);
    drawClock(hh, mm, 4, true, color);
    dencity = 50;
  }
  else 
  {
    if( sleep_cnt > 40 )
    {
      Clear();
      Show();
      dencity = 50;
    }
    else 
    {
      //drawString("SLEEP", 0, COLOR(0, 0, 8));
      randomPixels( color , dencity );
      if( dencity >=5 )
        dencity-=5;
    }
  }

  esp_task_wdt_reset();
  delay( 1000 );
}


  /*
void loop() 
{

  Clear();
  printChar4x7('1', 1,  COLOR(8, 0, 0));
  printChar4x7('2', 5,  COLOR(8, 0, 0));
  printChar4x7('3', 15, COLOR(8, 0, 0));
  printChar4x7('4', 20, COLOR(8, 0, 0));
  Show();
  delay(5000);

  Clear();
  printChar3x5(':', 8,  COLOR(8, 0, 0));

  printChar3x5('1', 0,  COLOR(8, 0, 0));
  printChar3x5('2', 4,  COLOR(8, 0, 0));
  printChar3x5('3', 10, COLOR(8, 0, 0));
  printChar3x5('4', 14, COLOR(8, 0, 0));
  Show();
  delay(5000);
}
*/