/***************************************************************************
    aktimelib.h  -  description

    begin                : Thu Feb 21 2002
    copyright            : (C) 2002 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifndef AKTIMELIB_H
#define AKTIMELIB_H

#include <akutil/procDuration.h>
#include <akutil/aksingleton.h>
#include <akutil/aksingletoncleaner.h>


  /** @page timesync Relation between PC clock and GPS time
    *
    * It is asumed that the PC clock follows
    * the unix-like time modell. The function "time"
    * will return the UTC time in seconds starting from
    * the 1.1.1970. Every 4 years a leap year
    * occures but leap second are ignored (see "man 2 time").
    *
    * To set the GPS time for the crate two informations are
    * required:
    * @li The difference between the time base of
    *     UTC and GPS time (The GPS time started at the
    *     6.1.1980)
    * @li The number of leap seconds since the begin
    *     of the GPS time.
    *
    * The first information does not change, while the
    * number of leap second will increase from time to time.
    * The information when another leap second is added
    * is published some month in advance.
    *
    * List of leap seconds (form TimeStmp.cc by T.Paul)
@verbatim
// Table of GPS seconds when leap second occured.
// THIS HAS TO BE EDITED BY HAND WHENEVER A NEW LEAP SECOND
// OCCURS.  Maybe there is a better way... D.V.: What about putting it
// in an XML file and read it in at singleton runtime initialization so
// that the code does not need recompilation?
//
// Table contains pairs of (GPS second when leap occurred, correction to apply)
//
const unsigned int kNumLeaps=13;
const int kSecPerDay = 24*3600;
const unsigned int kLeapSeconds[kNumLeaps][2] = {
  //
  // (GPS epoch + years + leap days + Jan-Jul)*kSecPerDay + leapSeconds
  //                                           |
  { (361 +       0 +   0 + 181)*kSecPerDay +  0, +1  },    // 1 JUL 1981
  { (361 +     365 +   0 + 181)*kSecPerDay +  1, +2  },    // 1 JUL 1982
  { (361 +   2*365 +   0 + 181)*kSecPerDay +  2, +3  },    // 1 JUL 1983
  { (361 +   4*365 +   1 + 181)*kSecPerDay +  3, +4  },    // 1 JUL 1985
  { (361 +   7*365 +   1      )*kSecPerDay +  4, +5  },    // 1 JAN 1988
  { (361 +   9*365 +   2      )*kSecPerDay +  5, +6  },    // 1 JAN 1990
  { (361 +  10*365 +   2      )*kSecPerDay +  6, +7  },    // 1 JAN 1991
  { (361 +  11*365 +   3 + 181)*kSecPerDay +  7, +8  },    // 1 JUL 1992
  { (361 +  12*365 +   3 + 181)*kSecPerDay +  8, +9  },    // 1 JUL 1993
  { (361 +  13*365 +   3 + 181)*kSecPerDay +  9, +10 },    // 1 JUL 1994
  { (361 +  15*365 +   3      )*kSecPerDay + 10, +11 },    // 1 JAN 1996
  { (361 +  16*365 +   4 + 181)*kSecPerDay + 11, +12 },    // 1 JUL 1997
  { (361 +  18*365 +   4      )*kSecPerDay + 12, +13 },    // 1 JAN 1999
  { (361 +  25*365 +   5      )*kSecPerDay + 13, +14 }     // 1 JAN 2006 // 820022413 , ak
};

@endverbatim    

    What happens with the different clocks when a leap
    second occures?

@verbatim
    Civil UTC time       PC wo NTP sync?   PC w NTP sync    Leap     GPS  Leap
    31.12.2005 23:59:58        200             200           32      700   13
    31.12.2005 23:59:59        201             201           32      701   13
     1. 1.2006 23:59:60        202             202           32      702   13
     1. 1.2006 00:00:00        203             202           33      703   14
     1. 1.2006 00:00:01        204             203           33      704   14
@endverbatim    

    - localtime(<second counter>) will always return 59:59 or 00:00.
      The unix system does not know anything from the doubled second with the
      number 202.
    - A PC without NTP synchronisation will return a wrong time by one second.
    - The equation NTP = GPS + Offset + LeapSec is always valid.
      The counting of the GPS is unique and linear in time. 
    - The feshel function chktime will work as long as the 
      ntp daemon of the PC will increment the PC clock after the leap
      second has occured. 

    Dataformat of the Inifile for the leap seconds
@verbatim
[GPSTime]
; Give a list of leap seconds. The relation between
; NTP time and GPS time is only for the periods
; correct where the leap seconds are given.
;
; Format: <month> <year> <num>
; Where <month> <year> give the date where the number
; of leap seconds changes
; <num> gives the number of leap seconds in the period
; starting with the given date. The leap seconds are
; counted as published in IERS Bulletin C (TAI - UTC/NTP)
;
leap0 = jan 1980 19
leap1 = jul 1997 31
leap2 = jan 1999 32
leap3 = jan 2006 33
@endverbatim    
            
    */


/** Library to handle the different time
  * formats. The class needs to reads it's basic information
  * (about the leap seconds) from some configuration file.
  * Before using the library the first time load these
  * value with the function init.
  *
  *
  * The first implementation is based on a default number of
  * leap second at the presend time and
  * the time when the next leap second will occure.
  * After this given occurence the ini file parameters
  * have to be adjusted.
  *
  * There are two
  * values taken from the ini file (stanard FE.ini):
  * @li The actual number of leap seconds (default is 13)
  *     and
  * @li the time (in UTC format) when the next leap
  *     second is introcuded. If this value is zero
  *     no change in the leap second is considered.
  *
  * A sample entry is FE.ini looks like:
  * <pre>
  * [GPSTime]
  * ; number of leap seconds (before nextLeapSecond - time)
  * leapSeconds = 13
  * ; Time when the next leap second occures (UTC seconds)
  * nextLeapSecond = 1016534603
  * </pre>
  *
  *
  * @ingroup akutil_group
  *
  */

class akTimeLib :  public akSingleton
{
public:
  // akSingletonCleaner needs to delete this call at program end! 
  friend class akSingletonCleaner;

  
protected:
  akTimeLib();

public: // Windows does not allow protected destructors??
  ~akTimeLib(){
   cleaner.setObject( 0 );
     //instance = (akTimeLib) 0;  // ???
  }
  

  
public:

  /** Get the reference to the singleton */
  static akTimeLib *getReference();


  /** Read the leap seconds table from an ini file */
  void init(const char *inifile="aktimelib.ini", FILE *fout=0);

  /** Give information about class internals */
  void dump(FILE *fout=stdout){
    
     fprintf(fout, "akTimeLib: \n");

     // TODO: Give information about the next
     // change in the number of leap seconds
     // --> Table of second counters...
     
  }

  /** Test the functions of the class */
  void test();


  /** Convert the Hardware second counter the the
    * format used by the PC (UTC time started at 1.1.1970)
    *
    * @param t_gps Second counter in GPS format
    * @return Time in UTC format format
    *
    */
  time_t convertGPSToUTC(unsigned long t_gps);


  /** Convert UTC to GPS time
    *
    * @param t_utc Second counter in UTC format
    * @return Time in GPS format format
    */
  unsigned long  convertUTCToGPS(time_t t_utc);

  /** Check if a gps second counter points
    * to an leap second
    * 
    * @returns -1 error, 0 no lepa second, 1 leap second
    */
  int isLeapSecond(unsigned long t_gps);

  /** Determine the number of leap seconds
    */
  int nLeapSeconds(unsigned long t_gps);

  /** Add new leap second
    *
    *
    */
  int addLeapSecond();
  
private:

  /** The only instance of this singleton class */
  static akTimeLib instance;

  /** Can the cleaner be defined in the singleton class?
    * I fear, it will only be possible to have one singleton in a program ?!*/
  static akSingletonCleaner cleaner;



  /** Delay between GPS and UTC second count
    * gpsDelay = t(UTP) - t(GPS)
    */
  unsigned long gpsDelay;

  /** Number of actual leap seconds
    *
    */
  int leapSeconds;

  /** Time when the next leap second occures.
    * This time has to be given in UTC seconds.
    */
  unsigned long nextLeapSecond;

  /** List of gps seconds where a leap second is added */
  unsigned long *leapSecGps;

  /** List of utc/ntp second where a leap second is added */
  unsigned long *leapSecUtc;
  
  /** Number of leap second beginning from leapSecGps */
  int *leapSecN;

  /** Number of entries in the leap list */
  int nLeapSecs;

  /** Flag to show if leap second table is missing */
  static int noLeapSecTable;
    
};



#endif
