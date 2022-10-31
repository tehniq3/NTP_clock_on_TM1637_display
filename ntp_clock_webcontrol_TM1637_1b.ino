// based on NTP clock with ESP8266 on commom chatode 7 segment led display with webpage control -  https://github.com/tehniq3/NTPclock_7seg_cc
// used base schematic from http://www.valvewizard.co.uk/iv18clock.html 
// NTP clock, used info from // https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
// NTP info: https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
// changed for common cathode multiplexed led display by Nicu FLORICA (niq_ro)
// added real temperature and humidity measurements with DHT22 
// added 12-hour format - https://en.wikipedia.org/wiki/12-hour_clock
// added web control for Daylight Saving Time (or summer time) - https://github.com/tehniq3/NTP_DST_ESP8266
// added web control for TimeZome: https://nicuflorica.blogspot.com/2021/02/ceas-gps-cu-reglaj-ora-locala-4.html
// added WiFi Manager, see http://nicuflorica.blogspot.com/2019/10/configurare-usoara-conectare-la-retea.html
// display IP on led display
// changed also by Nicu FLORICA (niq_ro) to TM1637 display in 15.10.2022, Craiova 
// display state AP or IP in TM1637 display for easy know the IP of local control webpage (eg: 192.168.3.1)
// v.1a2a4 - small updates for issue at date chamge after change TimeZone or DST
// v.1a3 - solved issue with special TimeZone (-9., 2.75,etc)
// v.1a4 - added brightness control from webpage
// v.1b - added automatic brightness due to sunrise/sunset using https://github.com/jpb10/SolarCalculator library
// v.1b1 - corrected changes from 23:59 to 0:00 (sometimes remains 20:00)and reading data every hour


#include <SolarCalculator.h> //  https://github.com/jpb10/SolarCalculator
#include <EEPROM.h>
#include <DS3231.h>       // For the DateTime object, originally was planning to use this RTC module: https://www.arduino.cc/reference/en/libraries/ds3231/
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager                                                                                                              
#include <TM1637Display.h>     // library from https://github.com/avishorp/TM1637

#define CLK 12 // GPIO12 = D6                       // Define the connections pins:
#define DIO 14 // GPIO14 = D5
// Definitions using https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/ 

TM1637Display display = TM1637Display(CLK, DIO);              // Create display object of type TM1637Display:

float TIMEZONE = 0; // Define your timezone to have an accurate clock (hours with respect to GMT +2)
// "PST": -7 
// "MST": -6 
// "CST": -5 
// "EST": -4 
// "GMT": 0 

int timezone0 = 16; 
int timezone1 = 0;
float diferenta[38] = {-12., -11.,-10.,-9.5,-9.,-8.,-7.,-6.,-5.,-4.,-3.5,-3.,-2.,-1.,0,
                      1.,2.,3.,3.5,4.,4.5,5.,5.5,5.75,6.,6.5,7.,8.,8.75,9.,9.5,10.,10.5,
                      11.,12.,12.75,13.,14};   // added manualy by niq_ro
#define adresa  100  // adress for store the
byte zero = 0;  // variable for control the initial read/write the eeprom memory

const long utcOffsetInSeconds =  2*3600;  // +3 hour

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


// Definitions using https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/ 

// DST State
byte DST = 0;
byte DST0 = 0;
String oravara = ""; //

int ora, minut, secunda, rest;
int ora0, minut0;
byte ziua, luna, an;
unsigned long ceas = 0;
unsigned long ceas0 = 0;
unsigned long tpcitire = 0;

byte citire = 0;
byte citire2 = 0;
byte refres = 0;

unsigned long epochTime;
byte tensHour, unitsHour, tensMin, unitsMin, tensSec, unitsSec;

String ip = "";
byte lungip = 0;
int aipi[20];

int citire3 = 0; // variable to read data

int A,B,H;
byte pm =0;
byte h1224;  // 12/24-hour format
byte h12240 = 0;
String formatora = "";
byte stergere = 0;
byte intensitate11 = 2; // brightness for day
byte intensitate10 = 2;
byte intensitate21 = 0; // brightness for night
byte intensitate20 = 0;

// Web Host
String header; // Variable to store the HTTP request
unsigned long currentTime = millis(); // Current time
unsigned long previousTime = 0; // Previous time
const long timeoutTime = 2000; // Define timeout time in milliseconds (example: 2000ms = 2s)

// Hour State
// * DateTime objects cannot be modified after creation, so I get the current DateTime and use it to create the default 7AM alarm *
DateTime today = DateTime(timeClient.getEpochTime());

WiFiServer server(80); // Set web server port number to 80

int aipi2[4][4];
int aipi0[4][4]= {1,9,2,0,
                  1,6,8,0,
                  0,0,4,0,
                  0,0,1,0};
int j, k;

const uint8_t AP[] = {
  SEG_D ,                                           // _
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,    // A
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,            // P
  SEG_D                                             // _
  };

  // Location - Craiova: 44.317452,23.811336
  double latitude = 44.31;
  double longitude = 23.81;
  int utc_offset = 3;

double transit, sunrise, sunset;
char str[6];
int ora1, ora2, oraactuala;
int m1, hr1, mn1, m2, hr2, mn2; 
unsigned long tpcitire4;


void setup(){
  EEPROM.begin(512);  //Initialize EEPROM  - http://www.esp8266learning.com/read-and-write-to-the-eeprom-on-the-esp8266.php
  Serial.begin(115200);   // Setup Arduino serial connection
  Serial.println(" ");
  Serial.println("-------------------");

  display.clear(); // Clear the display:
  
  Serial.println("-----clock------");

zero = EEPROM.read(adresa - 1); // variable for write initial values in EEPROM
if (zero != 16)
{
EEPROM.write(adresa - 1, 16);  // zero
EEPROM.write(adresa, timezone0); // time zone (0...37 -> -12...+12) // https://en.wikipedia.org/wiki/Coordinated_Universal_Time
EEPROM.write(adresa + 1, 1);  // 0 => winter time, 1 => summer time
EEPROM.write(adresa + 2, 0);  // 0 => auto 12/24-hour format, 1 => 12-hour format, 2 - 24-hour format
EEPROM.write(adresa + 3, 2);  // day: 2, BRIGHT_DARKEST = 0,BRIGHTEST = 7;
EEPROM.write(adresa + 4, 0);  // day: 0, BRIGHT_DARKEST = 0,BRIGHTEST = 7;
EEPROM.commit();    //Store data to EEPROM
} 

// read EEPROM memory;
timezone0 = EEPROM.read(adresa);  // timezone +12
timezone1 = timezone0;
TIMEZONE = (float)diferenta[timezone0];  // convert in hours
DST = EEPROM.read(adresa+1);
DST0 = DST;
h1224 = EEPROM.read(adresa+2);
intensitate11 = EEPROM.read(adresa+3);
intensitate10 = intensitate11;
intensitate21 = EEPROM.read(adresa+4);
intensitate20 = intensitate21;

if (h1224 > 3) h1224 = 0;  // if hour format is invalit set to "auto"
if (h1224 == 0) formatora = " auto  ";
if (h1224 == 1) formatora = "12-hour";
if (h1224 == 2) formatora = "24-hour";

if (intensitate11 > 7) intensitate11 = 2; // typical value
if (intensitate21 > 7) intensitate21 = 0; // typical value
            
if (DST == 0) oravara = "off"; 
else oravara = "on"; 
Serial.println("==============");
Serial.print("TimeZone: ");
Serial.print(TIMEZONE);
Serial.print(" / ");
Serial.println(diferenta[timezone0]);
Serial.print("DST: ");
Serial.println(DST);
Serial.println("==============");

//WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

Serial.println("Wi-Fi: AutoConnectAP");
Serial.println("IP: 192.168.4.1     ");

display.setBrightness(intensitate21);   // Set the brightness:
//BRIGHT_TYPICAL = 2, BRIGHT_DARKEST = 0,BRIGHTEST = 7;

display.clear(); 

display.setSegments(AP);
delay(100);

Serial.println("------- ");

  //reset settings - for testing
  //wifiManager.resetSettings();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(180);
  
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if(!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  } 

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

    // Print local IP address
  Serial.println("\nWiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

ip = WiFi.localIP().toString();
lungip = ip.length();

 Serial.print(lungip);
 Serial.println("chars");

aipi[lungip+1];

j = 3;
k = 3;
 
for (byte i=lungip; i>0; i--)  
{ 
  if (ip[i] == '.')
  {
    aipi[i] = 18;
    j--;
    k = 3;
    Serial.print(".");
  }
  aipi2[j][k] = ip[i] - 48;
  Serial.print(ip[i] - 48);
  Serial.print(" j = "); 
  Serial.print(j); 
  Serial.print(" k  = "); 
  Serial.print(k);  
 Serial.println(" "); 
  k--;
 delay(1);
}

aipi2[0][0] = ip[0]-48;  //ASCII table
Serial.print(ip[0]);
Serial.print(" j = "); 
Serial.print(0); 
Serial.print(" k  = "); 
Serial.print(0);  
Serial.println(" "); 

Serial.println("-------//--------"); 

//delay(2000);

for (byte j=0; j<4; j++)  
{ 
 for (byte k=0; k<3; k++)
{  
Serial.print(aipi2[j][k]);
delay(4);
}
Serial.print("-");
}
Serial.println("recovered IP");
//delay(3000);

  display.clear(); 
  
k = 0;
for (byte j=0; j<4; j++)  
{ 
A = 100*aipi2[j][k] + 10*aipi2[j][k+1]+aipi2[j][k+2];
Serial.println(A);
display.showNumberDecEx(A, 0b00010000 , false, 4, 0); 
delay(3000);

}

  timeClient.begin();
  if (DST == 1) 
  timeClient.setTimeOffset((TIMEZONE+1)*3600); // Offset time from the GMT standard
  else
  timeClient.setTimeOffset(TIMEZONE*3600); // Offset time from the GMT standard
  timeClient.update(); // Update the latest time from the NTP server
  server.begin(); // Start web server!

iaOra();
delay(1000);
iaData();
display.clear(); 

Soare();
luminita();
}

void loop(){

  timeClient.update(); // Update the latest time from the NTP server
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client is connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) { // If the current line is blank, you got two newline characters in a row. That's the end of the client HTTP request, so send a response:
            client.println("HTTP/1.1 200 OK"); // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            client.println("Content-type:text/html"); // and a content-type so the client knows what's coming, then a blank line:
            client.println("Connection: close");
            client.println();
            
            if (header.indexOf("GET /vara/on") >= 0) { // If the user clicked the alarm's on button
              Serial.println("Daylight saving time (DST) was activated !");
              oravara = "on";
              display.clear(); 
              timeClient.setTimeOffset((TIMEZONE+1)*3600); // Offset time from the GMT standard
              client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
              iaOra();
            } 
            else if (header.indexOf("GET /vara/off") >= 0) { // If the user clicked the alarm's off button
              Serial.println("Daylight saving time (DST) was deactivated !");
              oravara = "off";
              display.clear(); 
              timeClient.setTimeOffset((TIMEZONE+0)*3600); // Offset time from the GMT standard
              client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
              iaOra();
            }
              if (header.indexOf("GET /TZplus") >= 0) { // If the user clicked the TimeZone change button
              Serial.println("TimeZone was changed !");
              timezone0 = timezone0 + 1;
              display.clear(); 
              if (timezone0 > 37) timezone0 = 0;
              TIMEZONE = (float)diferenta[timezone0];  // convert in hours
              timeClient.setTimeOffset((TIMEZONE+DST)*3600); // Offset time from the GMT standard
            client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            } 

            if (header.indexOf("GET /TZminus") >= 0) { // If the user clicked the TimeZone change button
              Serial.println("TimeZone was changed !");
              timezone0 = timezone0 - 1;
              display.clear(); 
              if (timezone0 < 0) timezone0 = 37;
              TIMEZONE = (float)diferenta[timezone0];  // convert in hours
              timeClient.setTimeOffset((TIMEZONE+DST)*3600); // Offset time from the GMT standard
              client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            }

            if (header.indexOf("GET /HourFormat") >= 0) { // If the user clicked the HourFormat change button
              Serial.println("HourFormat was changed !");
              h1224 = h1224 + 1;
              display.clear(); 
              if (h1224 > 2) h1224 = 0;
              if (h1224 == 0) formatora = " auto  ";
              if (h1224 == 1) formatora = "12-hour";
              if (h1224 == 2) formatora = "24-hour";
              client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            } 

              if (header.indexOf("GET /Refreshinfo") >= 0) { // If the user clicked the "Refresh Info" button
              Serial.println("Clock was updated !");
            //  client.println("<meta http-equiv='Refresh' content=0; url=//" + WiFi.localIP().toString()+ ":80/>");
            }
            
            else if (header.indexOf("GET /time") >= 0) { // If the user submitted the time input form
              // Strip the data from the GET request
              int index = header.indexOf("GET /time");
              String timeData = header.substring(index + 15, index + 22);
              display.clear(); 
      
              Serial.println(timeData);
              // Update our alarm DateTime with the user selected time, using the current date.
              // Since we just compare the hours and minutes on each loop, I do not think the day or month matters.
              DateTime temp = DateTime(timeClient.getEpochTime()); 
         //     client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            }

            if (header.indexOf("GET /IZminus") >= 0) { // If the user clicked the Brightness change button
              Serial.println("Day Brightness was changed !");
              intensitate11 = intensitate11 - 1;
              if (intensitate11 > 7) intensitate11 = 0;
              display.clear();
              display.setBrightness(intensitate11);   // Set the brightness
              tpcitire4 = millis();
              client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            }

            if (header.indexOf("GET /IZplus") >= 0) { // If the user clicked the Brightness change button
              Serial.println("Day Brightness was changed !");
              intensitate11 = intensitate11 + 1;
              if (intensitate11 > 7) intensitate11 = 7;
              display.clear();
              display.setBrightness(intensitate11);   // Set the brightness
              tpcitire4 = millis();
              client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            }

            if (header.indexOf("GET /INminus") >= 0) { // If the user clicked the Brightness change button
              Serial.println("Night Brightness was changed !");
              intensitate21 = intensitate21 - 1;
              if (intensitate21 > 7) intensitate21 = 0;
              display.clear();
              display.setBrightness(intensitate21);   // Set the brightness
              tpcitire4 = millis();
              client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            }

            if (header.indexOf("GET /INplus") >= 0) { // If the user clicked the Brightness change button
              Serial.println("Night Brightness was changed !");
              intensitate21 = intensitate21 + 1;
              if (intensitate21 > 7) intensitate21 = 7;
              display.clear();
              display.setBrightness(intensitate21);   // Set the brightness
              tpcitire4 = millis();
              client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            }

          
            // Display the HTML web page
            // Head
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=0\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<link rel=\"stylesheet\" href=\"//stackpath.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css\">"); // Bootstrap
          //  client.println("<meta http-equiv='Refresh' content=0;url=//" + WiFi.localIP().toString()+ ":80/>");
            client.println("</head>");
            
            // Body
            client.println("<body>");
            client.println("<h1 class=\"text-center mt-3\"NTP / DSP Clock</h1>"); // Title

            // Current Time
            client.print("<h1 class=\"text-center\">"); 
            client.print(timeClient.getFormattedTime());
            client.println("</h1>");
            client.print("<h4 class=\"text-center\">"); 
            client.print("(last update for clock)"); 
            client.println("<a href=\"/Refreshinfo\"><button class=\"btn btn-sm btn-warning\">Refresh info (clock)</button></a></p>");
            client.println("</h4>");
            
            // Display sunrise / sunset
               client.print("<h3 class=\"text-center\">Sunrise: " + String(hr1));
               client.println(":" + String(mn1));
               client.println(" / Sunset: " + String(hr2));
               client.println(":" + String(mn2) + "</h3>");

              client.println("</br>");
              client.println("</br>");
              
             // Display current state, and ON/OFF buttons for DST 
            client.println("<h2 class=\"text-center\">Daylight Saving Time - " + oravara + "</h2>");
            if (oravara=="off") {
              client.println("<p class=\"text-center\"><a href=\"/vara/on\"><button class=\"btn btn-sm btn-danger\">ON</button></a></p>");
              DST = 0;
            }
            else {
              client.println("<p class=\"text-center\"><a href=\"/vara/off\"><button class=\"btn btn-success btn-sm\">OFF</button></a></p>");
              DST = 1;
            }

            // Display TimeZone state, and button for increase TimeZone 
            client.println("<h2 class=\"text-center\">TimeZone = " + String(TIMEZONE) + "</h2>");
            client.println("<p class=\"text-center\"><a href=\"/TZminus\"><button class=\"btn btn-sm btn-info\">decrease TimeZone (-)</button></a>");
            client.println("<a href=\"/TZplus\"><button class=\"btn btn-sm btn-warning\">increase TimeZone (+)</button></a></p>");

            // Display 12/24-hour format state, and button for increase 12/24-hout format 
            client.println("<h2 class=\"text-center\">HourFormat = " + formatora + "</h2>");
           
            client.println("<p class=\"text-center\"><a href=\"/HourFormat\"><button class=\"btn btn-sm btn-info\">change 12/24-hour format</button></a></p>");

            // Display brightness state, and button for increase/decrease brightness 
            client.println("<h2 class=\"text-center\">Day Brightness = " + String(intensitate11) + "</h2>");
            client.println("<p class=\"text-center\"><a href=\"/IZminus\"><button class=\"btn btn-sm btn-info\">decrease brightness (-)</button></a>");
            client.println("<a href=\"/IZplus\"><button class=\"btn btn-sm btn-warning\">increase brightness (+)</button></a></p>");
            client.println("<h2 class=\"text-center\">Night Brightness = " + String(intensitate21) + "</h2>");
            client.println("<p class=\"text-center\"><a href=\"/INminus\"><button class=\"btn btn-sm btn-info\">decrease brightness (-)</button></a>");
            client.println("<a href=\"/INplus\"><button class=\"btn btn-sm btn-warning\">increase brightness (+)</button></a></p>");

            client.println("</body></html>");
            client.println(); // The HTTP response ends with another blank line
            break; // Break out of the while loop
            
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    
    header = ""; // Clear the header variable
    client.stop(); // Close the connection
    Serial.println("Client disconnected.");
    Serial.println("");
  }

if(DST == 1)
timeClient.setTimeOffset((TIMEZONE+1)*3600); // Offset time from the GMT standard
else
timeClient.setTimeOffset(TIMEZONE*3600); // Offset time from the GMT standard

if (millis() - tpcitire > 1000)
{
iaOra();
tpcitire = millis();
}

if (millis() - tpcitire4 > 2000)
{
luminita();
tpcitire4 = millis();
}

if ((minut == 0) and (citire3 == 0))  // read day, mounth and year every hour
{
iaData();
Soare();
citire3 = 1;
display.clear();
}
if ((minut > 1) and (citire3 == 1))  // reseting variable for reading the data every hours
{
  citire3 = 0;
}
 if (secunda <= 50)
 {
  if (h1224 == 0)
  {
  if (minut % 2 == 0) displayTime();
     else displayTime12();
  }
  else
  if (h1224 == 1) displayTime12();
  else  displayTime();
 }
else
 if ((secunda > 50) and (secunda <= 55))
 {
 displayDate();
 }
else
 {
  displayYear();
  stergere = 0;
 }
if ((stergere == 0) and (secunda == 0))
{
    display.clear(); 
    stergere = 1;
}

if (DST0 != DST)
{
timeClient.setTimeOffset((TIMEZONE+DST)*3600);
EEPROM.write(adresa + 1, DST);  // 1 => summer format, 0 => winter format 
EEPROM.commit();    //Store data to EEPROM 
Serial.print("Write in EEPROM: DST = ");
Serial.println(DST);
iaData();
Serial.println("==============");
Serial.print("TimeZone: ");
Serial.print(TIMEZONE);
Serial.print(" / ");
Serial.println(diferenta[timezone0]);
Serial.print("DST: ");
Serial.println(DST);
Serial.println("==============");
Soare();
}
DST0 = DST;

if (h12240 != h1224)
{
EEPROM.write(adresa + 2, h1224);  // 0 => auto 12/24-hour format, 1 => 12-hour format, 2 - 24-hour format
EEPROM.commit();    //Store data to EEPROM 
Serial.print("Write in EEPROM: h1224 = "); 
Serial.println(h1224);
}
h12240 = h1224;

if (timezone1 != timezone0)
{
EEPROM.write(adresa, timezone0);  // new time zone
EEPROM.commit();    //Store data to EEPROM 
Serial.print("Write in EEPROM: timezone = ");
Serial.println(timezone0);
iaData();
Serial.println("==============");
Serial.print("TimeZone: ");
Serial.print(TIMEZONE);
Serial.print(" / ");
Serial.println(diferenta[timezone0]);
Serial.print("DST: ");
Serial.println(DST);
Serial.println("==============");
Soare();  
}
timezone1 = timezone0;

if (intensitate10 != intensitate11)
{
EEPROM.write(adresa + 3, intensitate11);  // brightness
EEPROM.commit();    //Store data to EEPROM 
Serial.print("Write in EEPROM: day brightness = "); 
Serial.println(intensitate11);
Soare();
}
intensitate10 = intensitate11;

if (intensitate20 != intensitate21)
{
EEPROM.write(adresa + 4, intensitate21);  // brightness
EEPROM.commit();    //Store data to EEPROM 
Serial.print("Write in EEPROM: night brightness = "); 
Serial.println(intensitate21);
Soare();
}
intensitate20 = intensitate21;

}   //End of main program loop


void displayTime(){
    byte tensHour = ora / 10; //Extract the individual digits
    byte unitsHour = ora % 10;
    byte tensMin = minut / 10;
    byte unitsMin = minut % 10;
    byte tensSec = secunda / 10;
    byte unitsSec = secunda % 10;

  A = ora * 100 + minut;
  B = secunda;

  if (A >= 100)
  {
  if((B % 2) == 0)
  {
    display.showNumberDecEx(A, 0b01000000 , false, 4, 0); 
  }
  else
  {
    display.showNumberDecEx(A, 0b00000000 , false, 4, 0); 
  }
  }
  else
   {
  if((B % 2) == 0)
  {
    display.showNumberDecEx(A, 0b10000000 , true, 3, 1); 
  }
  else
  {
    display.showNumberDecEx(A, 0b00000000 , true, 3, 1); 
  }
  } 
}

void displayTime12(){
   H = timeClient.getHours();
  if (H > 12)
  {
    H = H%12;
    pm = 1;
  }
  else
  {
    pm = 0;
  }
  if (H == 0) H = 12;
  
//  A = timeClient.getHours() * 100 + timeClient.getMinutes();
  A = H * 100 + timeClient.getMinutes();
  B = timeClient.getSeconds();

  if (pm == 0)
  {
  if((B % 2) == 0)
  {
    display.showNumberDecEx(A, 0b01000000 , false, 4, 0); 
  }
  else
  {
    display.showNumberDecEx(A, 0b00000000 , false, 4, 0); 
  }
  }
  else
  {
  if((B % 2) == 0)
  {
    display.showNumberDecEx(A, 0b01010000 , false, 4, 0); 
  }
  else
  {
    display.showNumberDecEx(A, 0b00010000 , false, 4, 0); 
  }
  }
}

void displayDate(){
    byte tensDate = ziua / 10; //Extract the individual digits
    byte unitsDate = ziua % 10;
    byte tensMon = luna / 10;
    byte unitsMon = luna % 10;
    byte tensYear = an / 10;
    byte unitsYear = an % 10;    

  A = ziua * 100 + luna;
  display.showNumberDecEx(A, 0b01010000 , false, 4, 0);

}


void displayYear(){
    byte tensDate = ziua / 10; //Extract the individual digits
    byte unitsDate = ziua % 10;
    byte tensMon = luna / 10;
    byte unitsMon = luna % 10;
    byte tensYear = an / 10;
    byte unitsYear = an % 10;

  display.showNumberDecEx(an+2000, 0b00010000 , false, 4, 0);
}


void iaData()
{
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  ziua = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(ziua);

  luna = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(luna);


  an = ptm->tm_year+1900-2000;
  Serial.print("Year: ");
  Serial.println(an);
}

void iaOra()
{
  timeClient.update();

epochTime = timeClient.getEpochTime();
//  Serial.print("Epoch Time: ");
//  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
//  Serial.print("Formatted Time: ");
//  Serial.println(formattedTime);  
  
    ora = timeClient.getHours();
  //  Serial.print("ora = ");
  //  Serial.println(ora);
    minut = timeClient.getMinutes();
    secunda = timeClient.getSeconds();
}

// Rounded HH:mm format
char * hoursToString(double h, char *str)
{
  int m = int(round(h * 60));
  int hr = (m / 60) % 24;
  int mn = m % 60;

  str[0] = (hr / 10) % 10 + '0';
  str[1] = (hr % 10) + '0';
  str[2] = ':';
  str[3] = (mn / 10) % 10 + '0';
  str[4] = (mn % 10) + '0';
  str[5] = '\0';
  return str;
}

void Soare()
{
   // Calculate the times of sunrise, transit, and sunset, in hours (UTC)
  calcSunriseSunset(2000+an, luna, ziua, latitude, longitude, transit, sunrise, sunset);

m1 = int(round((sunrise + TIMEZONE+DST) * 60));
hr1 = (m1 / 60) % 24;
mn1 = m1 % 60;

m2 = int(round((sunset + TIMEZONE+DST) * 60));
hr2 = (m2 / 60) % 24;
mn2 = m2 % 60;

  Serial.print("Sunrise = ");
  Serial.print(sunrise+TIMEZONE+DST);
  Serial.print(" = ");
  Serial.print(hr1);
  Serial.print(":");
  Serial.print(mn1);
  Serial.print(" -> ");
  Serial.println(hoursToString(sunrise + TIMEZONE+DST, str));
  Serial.print("Sunset = ");
  Serial.print(sunset+TIMEZONE+DST);
  Serial.print(" = ");
  Serial.print(hr2);
  Serial.print(":");
  Serial.print(mn2);
  Serial.print(" -> ");
  Serial.println(hoursToString(sunset + TIMEZONE+DST, str));
}

void luminita()
{
  ora1 = 100*hr1 + mn1;  // rasarit 
  ora2 = 100*hr2 + mn2;  // apus
  oraactuala = 100*ora + minut;  // ora actuala
  if ((oraactuala > ora1) and (oraactuala < ora2))  // zi
  {
     display.setBrightness(intensitate11);   // Set the brightness
  }
  else  // noapte
  {
     display.setBrightness(intensitate21);   // Set the brightness
  }
}
