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

#if !defined (ARDUINO_ARCH_RENESAS_UNO)
#error "This library is developed only for Arduino UNO R4."
#endif

#ifndef R4_SW_RTC
#define R4_SW_RTC

/*
   For debugging purposes only
*/

/* #define OUT_CLOCK */                   /* uncommento to have the clock on pin D7 */

#ifdef OUT_CLOCK
#define PORTBASE 0x40040000 /* Port Base */
#define PFS_P107PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 7 * 4))) // D7 UNO R4 MINIMA
#define PFS_P112PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (12 * 4))) // D7 UNO R4 WIFI
#endif

/*
   --- Module defines ---
*/

#define R4SWRTC_VER    "1.0.1"      /* r4SwRTC library internal string   version */
#define R4SWRTC_VER_NUM 10001       /* r4SwRTC library internal numeric  version */
#define R4SWRTC_VER_MAJ     1       /* r4SwRTC library internal major    version */
#define R4SWRTC_VER_MIN     0       /* r4SwRTC library internal minor    version */
#define R4SWRTC_VER_REV     1       /* r4SwRTC library internal revision version */

#define TMR_FREQ        100.0       /* default timer frequency in Hz */
#define CNT_CMPR          100       /* counter value for 1 second */

/*
   --- Includes ---
*/

#include <FspTimer.h>
#include <time.h>
#include "atomic.h"

/*
   --- class methods ---
*/

class r4SwRTC {

   public:

      r4SwRTC ( void );
      bool   begin ( float timerFreq = TMR_FREQ );
      void   setUnixTime ( time_t settingTime );
      time_t getUnixTime ( void );
      time_t setTmTime ( struct tm * );
      struct tm * getTmTime ( void );

   private:

      static void timer_callback ( timer_callback_args_t __attribute ( ( unused ) ) *p_args );
      bool beginTimer ( float rate );
};

#endif
