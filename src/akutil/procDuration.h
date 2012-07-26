/***************************************************************************
    procDuration.h  -  description

    begin                : Thu Jun 29 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/



#ifndef _INC_PROCDURATION_1234_INCLUDED
#define _INC_PROCDURATION_1234_INCLUDED


#include <cstdio>

// elapsed cpu time
#include <ctime>

// access to miliseconds of the real-time clock is system-dependent
#include <sys/time.h>
#include <unistd.h>


// Define a system independand sleep function?!
#ifdef __GNUC__
#define SLEEP(_delay) usleep(_delay * 1000)  // us ?
#endif

#ifdef _WIN32 // Windows
#include <windows.h>
#define SLEEP(_delay) Sleep(_delay)    // miliseconds
#endif



/** The class provide a timer like function
  * to measure the real-time and the CPU time elapsed
  * by a certain block of code.
  * The time is used by setting start and stop point
  * in the code.
  * Afterwards the elapsed time can be displayed.
  *
  * @ingroup akutil_group
  */
class procDuration
{

public:
   procDuration();

   void setStart();

   void setStart(unsigned long time);

   void setEnd();

   /** Set end time from another start time */
   void setEndFromStart(procDuration *dur);

   /** Used CPU time between start end end time in
     * second, precision given by CLOCKS_PER_SEC */
   double getDurationCPU();

   /** Duration between start end end time in
     * second, precision miliseconds. */
   double getDurationRT();

   void displayDuration();
   void displayDuration(FILE *fout);

   void displayTransferRate(double blocksize);
   void displayTransferRate(FILE *fout, double blocksize);

   void displayStartTime(FILE *fout);
   void displayStartDate(FILE *fout);


   /** Display the start time */
   const char * cStartTime();
   /** Display the start date */
   const char * cStartDate();
   /** Display the start timezone */
   const char * cStartZone();

   /** Display the end time */
   const char * cEndTime();
   /** Display the end date */
   const char * cEndDate();
   /** Display the end timezone */
   const char * cEndZone();

   /** Display the time of a given UTC time */
   const char * cTime(time_t *t);
   /** Display the time of a given GPS time */
   const char * cGpsTime(unsigned long t);
   /** Display the date of a given UTC time */
   const char * cDate(time_t *t);
   /** Display the date of a given GPS time */
   const char * cGpsDate(unsigned long t);
   /** Display the timezone of a give UTC time */
   const char * cZone(time_t *t);
   
   /** Returns the number of second
     * of the start time */
   unsigned long getStartSec();

   /** Returns the number of the second of the end time */
   unsigned long getEndSec();

   /** Returns the mili seconds of the start time */
   int getStartMiliSec();

   /** Returns the mili seconds of the end time */
   int getEndMiliSec();

   /** Returns the micro seconds of the start time */
   int getStartMicroSec();

   /** Returns the micro seconds of the end time */
   int getEndMicroSec();

   /** Returns the nano second of the start time */
   int getStartNanoSec();

   /** Returns the nano second of the end time */
   int getEndNanoSec();

   /** Get the mean time the given interval and set it as
     * new start time */
   void meanTime(procDuration &dur);

   /** Get the difference time */
   void getDurationRT(int *sign, unsigned long *sec, unsigned long *usec);

   /** Copy */
   procDuration operator=(procDuration &dur);

   /** Copy */
   void set(procDuration dur);

protected:

   clock_t startTick;
   clock_t endTick;

   struct timeval startTime;
   struct timezone startZone;
   struct timeval endTime;
   struct timezone endZone;

   double durationCPU;
   double durationRT;


   static const char dateformat[];
   static const char timeformat[];
   static const char zoneformat[];

   static char datestring[];
   static char timestring[];
   static char zonestring[];

//   static char datestring[ 4+1+2+1+2+1 ];
//   static char timestring[ 2+1+2+1+2+1 ];

   int diffTimeSign;
   unsigned long diffTimeSec;
   unsigned long diffTimeMicroSec;

};

#endif
