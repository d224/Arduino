#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* WIFI_SSID = "MASCORP";
const char* WIFI_PWD = "";


unsigned int udpPort = 4210; // local port to listen for UDP packets
char UDP_client = 255;
char ackPacket[255];  // buffer for incoming packets

WiFiUDP udp;

OneWire  ds(D4);  // on pin D4 (a 4.7K resistor is necessary)
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
float celsius;

void setup(void)
{
  Serial.begin(9600);
  pinMode(D1, OUTPUT);

  if ( !ds.search(addr))
  {
    ds.reset_search();
    delay(250);
    Serial.println("no Device DS18x20 family device found.");;
  }
  else
  {
    if (OneWire::crc8(addr, 7) != addr[7])
    {
      Serial.println("CRC is not valid!");
      return;
    }
    Serial.println();

    // the first ROM byte indicates which chip
    switch (addr[0])
    {
      case 0x10:
        type_s = 1;
        Serial.println("Device 0x10.");
        break;
      case 0x28:
        type_s = 0;
        Serial.println("Device 0x28.");
        break;
      case 0x22:
        type_s = 0;
        Serial.println("Device 0x22.");
        break;
      default:
        Serial.println("Device is not a DS18x20 family device.");
        return;
    }
  }

  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }

  udp.begin(udpPort);

}

bool SendUDPBroadcast(String str)
{
  for (int i = 0; i < 256; i++)
  {

    IPAddress broadcastIp = WiFi.localIP();
    broadcastIp[3] = UDP_client;
    Serial.println(broadcastIp);
    udp.beginPacket(broadcastIp, udpPort);
    udp.print(celsius);
    udp.endPacket();

 /*   if (i == 0)
      delay(3000);
    else
      delay(100);
*/
  
    int packetSize = 0;
    int timeout = 0;
    do{
      delay(100);
      timeout += 100;
      packetSize = udp.parsePacket();
      if(i == 0)
        if(timeout >= 5000) 
          break;
        else
          Serial.printf(".");
      else
          break;
    }while (packetSize == 0);
    
    if (packetSize)
    {
      // receive incoming UDP packets
      Serial.printf("Received ACK %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
      int len = udp.read(ackPacket, 255);
      if (len > 0)
      {
        ackPacket[len] = 0;
      }
      Serial.printf("UDP packet contents: %s\n", ackPacket);
      String strAck(ackPacket);

      ///////////////////////////////////////////////////
      if(strAck == "ON")
        digitalWrite(D1, HIGH );
      if(strAck == "OFF")
        digitalWrite(D1, LOW );
        
      IPAddress ackIp = udp.remoteIP();
      UDP_client = ackIp[3];
      return true;
    }
    else
    {
      Serial.printf("No replay\n");
      UDP_client++;
    }
  }
  return false;
}

void loop(void)
{
  byte i;

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);       
  //delay(1000); // start conversion, with parasite power on at the end
  present = ds.reset();
  if (present)
  {
    ds.select(addr);
    ds.write(0xBE);         // Read Scratchpad

    for ( i = 0; i < 9; i++)
    {
      data[i] = ds.read();
    }

    // Convert the data to actual temperature
    int16_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms

    celsius = (float)raw / 16.0;
    Serial.print("T = ");
    Serial.print(celsius);
    Serial.println(" C");

    if (celsius > 70)
      digitalWrite(D1, LOW );
  }

  SendUDPBroadcast(String(celsius));
  delay(1000);
}



