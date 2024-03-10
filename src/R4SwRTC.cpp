/*
   R4swRTC: a library to implement a software RTC on Arduino UNO R4 where the
   native RTC is overly inaccurate.
   The library will always be less accurate than a real RTC like the DS3231
   (because of the lack of quartz even on the main clock), but it can still
   be calibrated to have an error of about ± 1 second over 24 hours.
   (In my UNO R4 WiFi, I had to use a value of 100.076 Hz for the timer, to
   get that precision)

   Copyright (C) 2024 Guglielmo Braguglia

   The original code of how to use a timer is taken from the following article
   https://www.pschatzmann.ch/home/2023/07/01/under-the-hood-arduino-uno-r4-timers/
   written by Phil Schatzmann.

   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   This library is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "R4SwRTC.h"

static volatile uint64_t timerCount;
static volatile time_t   unixTime;
static FspTimer          the_timer;

#ifdef OUT_CLOCK
static volatile bool     pinState = false;
#endif

/*
      --- Private methods ---
*/

void r4SwRTC::timer_callback ( timer_callback_args_t __attribute ( ( unused ) ) *p_args ) {
   timerCount++;
   if ( timerCount >= CNT_CMPR ) {
      unixTime++;
      timerCount = 0;
   }

#ifdef OUT_CLOCK
   if ( pinState ) {

#if defined ( ARDUINO_UNOR4_MINIMA )
      *PFS_P107PFS_BY = 0x04;         // digitalWrite(D7, LOW);
#elif defined ( ARDUINO_UNOR4_WIFI )
      *PFS_P112PFS_BY = 0x04;         // digitalWrite(D7, LOW);
#else
#error "This program can run only on UNO R4"
#endif

   } else {

#if defined ( ARDUINO_UNOR4_MINIMA )
      *PFS_P107PFS_BY = 0x05;         // digitalWrite(D7, HIGH);
#elif defined ( ARDUINO_UNOR4_WIFI )
      *PFS_P112PFS_BY = 0x05;         // digitalWrite(D7, HIGH);
#else
#error "This program can run only on UNO R4"
#endif

   }
   pinState = !pinState;
#endif /* OUT_CLOCK */

}

bool r4SwRTC::beginTimer ( float rate ) {
   uint8_t timer_type = GPT_TIMER;
   int8_t tindex = FspTimer::get_available_timer ( timer_type );
   if ( tindex < 0 ) {
      tindex = FspTimer::get_available_timer ( timer_type, true );
   }
   if ( tindex < 0 ) {
      return false;
   }

   FspTimer::force_use_of_pwm_reserved_timer();

   if ( !the_timer.begin ( TIMER_MODE_PERIODIC, timer_type, tindex, rate, 0.0f, timer_callback ) ) {
      return false;
   }

   if ( !the_timer.setup_overflow_irq() ) {
      return false;
   }

   if ( !the_timer.open() ) {
      return false;
   }

   if ( !the_timer.start() ) {
      return false;
   }
   return true;
}

/*
      --- Public methods ---
*/
r4SwRTC::r4SwRTC ( void ) {
   timerCount = 0;
   unixTime   = 0;
}

bool r4SwRTC::begin ( float timerFreq ) {
   if ( ( timerFreq < ( TMR_FREQ - 2.0 ) ) || ( timerFreq > ( TMR_FREQ + 2.0 ) ) ) {
      return false;
   }
   return beginTimer ( timerFreq );
}

void r4SwRTC::setUnixTime ( time_t settingTime ) {
#if defined (INC_ARDUINO_FREERTOS_H) || defined (INC_FREERTOS_H)
	if ( xTaskGetSchedulerState( ) == taskSCHEDULER_RUNNING ) {
		taskENTER_CRITICAL();
		unixTime = settingTime;
		taskEXIT_CRITICAL();
	} else {
      ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) {
         unixTime = settingTime;
      }
	}
#else
   ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) {
      unixTime = settingTime;
   }
#endif
}

time_t  r4SwRTC::getUnixTime ( void ) {
   time_t TimerTime;
   

#if defined (INC_ARDUINO_FREERTOS_H) || defined (INC_FREERTOS_H)
	if ( xTaskGetSchedulerState( ) == taskSCHEDULER_RUNNING ) {
		taskENTER_CRITICAL();
		TimerTime = unixTime;
		taskEXIT_CRITICAL();
	} else {
      ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) {
         TimerTime = unixTime;
      }
	}
#else
   ATOMIC_BLOCK ( ATOMIC_RESTORESTATE ) {
      TimerTime = unixTime;
   }
#endif
   return TimerTime;
}

/*
   Standard TM structure from <time.h>

    Member      Type    Meaning                     Range
    --------    ---     ------------------------    -----
    tm_sec      int     seconds after the minute    0-59
    tm_min      int     minutes after the hour      0-59
    tm_hour     int     hours since midnight        0-23
    tm_mday     int     day of the month            1-31
    tm_mon      int     months since January        0-11
    tm_year     int     years since 1900
    tm_wday     int     days since Sunday           0-6
    tm_yday     int     days since January 1        0-365
    tm_isdst    int     Daylight Saving Time        flag

    Regarding tm_isdst ... if you do not set the value of
    tm_isdst in struct tm it is assumed to be 0, and thus
    mktime() ALWAYS assumes NO DST (just as if you set it
    to 1, it ALWAYS assumes WITH DST). If you want it to
    use TM settings you MUST set it to -1.

    For CET use TZ='CET-1CEST,M3.5.0/2,M10.5.0/3'
    The three fields (separate by commas) means:
    CET-1CEST tzone name (CET) offset (-1) summer time name
    (CEST) M3.5.0/2 time changes to summer-time on March (M3)
    last week (5), on Sunday at 2:00 (0/2) M10.5.0/3 time
    changes to regular time on October (M10) last week (5) on
    Sunday at 3:00 (0/3).
*/

time_t r4SwRTC::setTmTime ( struct tm * stTM ) {
   time_t the_time;

   the_time = mktime ( stTM );
   setUnixTime ( the_time );
   return the_time;
}

struct tm * r4SwRTC::getTmTime ( void ) {
   time_t      the_time;

   the_time = getUnixTime ( );
   return gmtime ( &the_time );
}
