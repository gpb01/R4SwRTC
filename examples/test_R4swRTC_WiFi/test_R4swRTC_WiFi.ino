/*
   A simple program to demonstrate the use of the R4SwRTC library comparing
   the time given by the swRTC with the time received from the NTP server.

   Copyright (C) 2024 Guglielmo Braguglia
*/

#define  CNT_FREQ      100.076   /* If swRTC goes forward, decrease the frequency, if it lags, increase the frequency */
#define MAX_WAITING_TIME 30000

#include <WiFiS3.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <R4SwRTC.h>
#include "arduino_secrets.h"

/*
   -------------------------------- static vars
*/

/* You can specify the time server pool and the offset, (in seconds)
   additionally you can specify the update interval (in milliseconds).
   NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
*/
WiFiUDP   ntpUdp;
NTPClient timeClient ( ntpUdp );
r4SwRTC   myRTC;

char      ssid[] = SECRET_SSID;
char      pass[] = SECRET_PASS;

uint16_t  WiFi_Status;

bool              ledState = false;
time_t            NTPtime;
time_t            TimerTime;
uint32_t          ledMillis;
uint32_t          lastMillis;
uint16_t          nHours;
uint16_t          nHalfHours;

/*
   ------- connectWiFi ( char* ssid, char* passwd )
*/

uint16_t connectWiFi ( char* ssid, char* passwd ) {
   uint16_t  WiFiStatus  = WL_IDLE_STATUS;
   uint32_t  WiFiTimeout = 0;

   if ( WiFi.status() == WL_NO_MODULE ) {
      return WL_NO_MODULE;
   }
   
   if ( WiFi.status() == WL_CONNECTED ) {
      return WL_CONNECTED;
   }
   
   WiFiTimeout = millis();
   while ( WiFiStatus != WL_CONNECTED ) {
      WiFiStatus = WiFi.begin ( ssid, passwd );
      delay ( 1000 );
      if ( millis() - WiFiTimeout >= MAX_WAITING_TIME ) {
         return WL_CONNECT_FAILED;
      }      
      timeClient.begin();
      delay ( 100 );
   }
   return WL_CONNECTED;
}

/*
   ---------------------------- getNTPtime ( void )
*/

time_t getNTPtime ( void ) {
   if ( !timeClient.update() ) {
      return 0;
   }
   return timeClient.getEpochTime();
}

/*
   ---------------------------------------- setup()
*/

void setup() {
   delay ( 500 );
   pinMode ( LED_BUILTIN, OUTPUT );
   
   Serial.begin ( 115200 );
   while ( !Serial ) delay ( 100 );
   
   Serial.println ( "Program started, try to connect to Wifi ..." );
   WiFi_Status = connectWiFi ( ssid, pass );
   if ( WiFi_Status != WL_CONNECTED ) {
      Serial.println ( "Unable to connect to network WiFi." );
      while ( true ) delay ( 100 );
   }
   Serial.print ( "Connected to WiFi, local IP address is: " );
   Serial.println ( WiFi.localIP() );
   
   myRTC.begin ( CNT_FREQ );
   
   NTPtime = getNTPtime();
   if ( 0 == NTPtime ) {
      Serial.println ( "Unable to retrive NTP time!" );
      while ( true ) delay ( 100 );
   }
   Serial.print ( "Initial time from NTP:" );
   Serial.println ( NTPtime );
   Serial.println( );
   
   myRTC.setUnixTime ( NTPtime );
   nHours     = 0;
   nHalfHours = 0;
   lastMillis = millis();
}

/*
   ---------------------------------------- loop()
*/

void loop() {
   if ( millis() - ledMillis > 1000 ) {
      if ( ledState == true ) {
         digitalWrite ( LED_BUILTIN, HIGH );
      } else {
         digitalWrite ( LED_BUILTIN, LOW );
      }
      ledState = !ledState;
      ledMillis += 1000;
   }
   
   if ( millis() - lastMillis > 1800000 ) {
      nHalfHours ++;
      if ( 2 == nHalfHours ) {
         nHalfHours = 0;
         nHours++;
      }
      NTPtime = getNTPtime();
      if ( 0 == NTPtime ) {
         Serial.println ( "Unable to retrive NTP time!" );
         Serial.println ( );
      } else {
         TimerTime = myRTC.getUnixTime();
         Serial.print ( "Time from NTP:" );
         Serial.print ( NTPtime );
         Serial.print ( " - Time from Timer:" );
         Serial.println ( TimerTime );
         Serial.print ( "Delta time after " );
         Serial.print ( nHours );
         Serial.print ( " hour(s) " );
         Serial.print ( "and " );
         Serial.print ( nHalfHours * 30 );
         Serial.print ( " minutes(s) " );
         Serial.print ( ( int32_t ) ( TimerTime - NTPtime ) );
         Serial.println ( " sec." );
         Serial.print ( asctime ( myRTC.getTmTime () ) );
         Serial.println ( " (UTC)" );
         Serial.println ( );
         lastMillis = millis();
      }
   }
}
