// video: https://youtu.be/ZWRT4mDb8O0
// small changes by niq_ro

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TM1637Display.h>     // library from https://github.com/avishorp/TM1637

#define CLK D6                       // Define the connections pins:
#define DIO D5

TM1637Display display = TM1637Display(CLK, DIO);              // Create display object of type TM1637Display:

const char *ssid     = "bbk2";
const char *password = "internet2";

const long utcOffsetInSeconds = 3*3600; //19802;  

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup(){
  Serial.begin(115200);
   // Clear the display:
  display.clear();
  
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {
  int A,B;
  
  timeClient.update();
  display.setBrightness(2);                   // Set the brightness:
  //BRIGHT_TYPICAL = 2, BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  
  A = timeClient.getHours() * 100 + timeClient.getMinutes();
  B = timeClient.getSeconds();
  
  if((B % 2) == 0)
  {
    display.showNumberDecEx(A, 0b01000000 , false, 4, 0); 
  }
  else
  {
    display.showNumberDecEx(A, 0b00000000 , false, 4, 0); 
  }
  
}
