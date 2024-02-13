/*
   A simple program to demonstrate the use of the R4SwRTC library.

   Copyright (C) 2024 Guglielmo Braguglia
*/

#include <R4SwRTC.h>

#define  TMR_FREQ_HZ  100.076  /* If swRTC goes forward, decrease the frequency, if it lags, increase the frequency */
#define  CLOCK_UPDT    900000  /* loop() dateTime display interval in millisec. */

r4SwRTC   myRTC;

bool              ledState = false;
time_t            T_Time;
uint32_t          ledMillis;
uint32_t          lastMillis;

/*
   ---------------------------------------- setup()
*/

void setup() {
   bool retVal;
   //
   delay ( 500 );
   pinMode ( LED_BUILTIN, OUTPUT );
   //
   Serial.begin ( 115200 );
   while ( !Serial ) delay ( 100 );
   //
   lastMillis = millis();
   retVal = myRTC.begin ( TMR_FREQ_HZ );
   if ( !retVal ) {
      Serial.println ( "Unable to start a free timer." );
      while ( true ) delay ( 100 );
   }
   //
   Serial.setTimeout ( 10000 );
   while ( true ) {
      Serial.print ( "Please enter the actual uinxTime: " );
      T_Time = Serial.parseInt();
      Serial.println ( T_Time );
      Serial.println();
      if ( 0 != T_Time ) break;
   }
   myRTC.setUnixTime ( T_Time );
   Serial.println ( asctime ( myRTC.getTmTime() ) );
   Serial.println ( );
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
   //
   if ( millis() - lastMillis > CLOCK_UPDT ) {
      T_Time = myRTC.getUnixTime();
      Serial.print ( "Time from SwRTC:" );
      Serial.println ( T_Time );
      Serial.println ( asctime ( myRTC.getTmTime() ) );
      Serial.println ( );
      lastMillis = millis();
   }
}
