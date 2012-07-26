/***************************************************************************
    aktimelib.cpp  -  description

    begin                : Thu Feb 21 2002
    copyright            : (C) 2002 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#include <iostream>
#include <string>
#include <cstring>
#include <ctime>

#include <fdhwlib.h>

#include <akutil/akinifile.h>
#include <akutil/procDuration.h>

#include "aktimelib.h"

using namespace std;



#define SLT_GPS_DELAY 315964800 // GPS - UTC Base (6.1.1980 - 1.1.1970)
#define SLT_NTP_DELAY 2208988800LL // UTC Base - NTP Base (1.1.1970 - 1.1.1900)
#define SLT_LEAP_SECS 13 // leap seconds, default



// Attention: The variable "cleaner" has to be defined before "instance" !!!
akSingletonCleaner akTimeLib::cleaner;

akTimeLib akTimeLib::instance;

int akTimeLib::noLeapSecTable = 1;

akTimeLib *akTimeLib::getReference(){

  if (noLeapSecTable){
    fprintf(stderr, "\n");
    fprintf(stderr, "Error: Inifile with table of leap seconds not found\n");
    fprintf(stderr, "\n");
  }

  return ( &instance );

}


akTimeLib::akTimeLib(){
  // Clean up at the program end to avoid memory holes ?!
  cleaner.setObject( this );


  // Set defaults for the class variables
  gpsDelay = SLT_GPS_DELAY;

  leapSeconds = SLT_LEAP_SECS;

  nextLeapSecond = 0;

  leapSecGps = 0;
  leapSecUtc = 0;
  leapSecN = 0;
  nLeapSecs = 0;

  noLeapSecTable = 1;
  
  // Load actual leap second table
  init();

}

void akTimeLib::init(const char *inifile, FILE *fout){

  // Set the apropropriate leap second
  // Read from ini file
  Inifile::result error;
  akInifile *ini; // Exception handling is missing!



  //fprintf(controlReg->fdebug,"\nSlt: Reading GPSTime parameter from %s\n",inifile);
  //ini = new akInifile(inifile,controlReg->fdebug,FE_INIPATH1,FE_INIPATH2);

  //fout = stdout;
  noLeapSecTable = 0;
  
  if (fout == 0) {
    ini = new akInifile(inifile, 0, FE_INIPATH1, FE_INIPATH2);
  } else {
    fprintf(fout,"akTimeLib: Reading GPSTime parameter from %s\n",inifile);
    ini = new akInifile(inifile, fout, FE_INIPATH1, FE_INIPATH2);
  }
  


  if (ini->Status()!=Inifile::kSUCCESS){
    noLeapSecTable = 1;
    //fprintf(stderr, "\n");
    //fprintf(stderr, "Error: Inifile \"%s\" with table of leap seconds not found\n", inifile);
    //fprintf(stderr, "\n");
  }

    
  ini->SpecifyGroup("GPSTime");
  leapSeconds = ini->GetFirstValue("leapSeconds", SLT_LEAP_SECS, &error);
  nextLeapSecond = ini->GetFirstValue("nextLeapSecond", 0, &error);



  char tag[20];
  char *month;
  int year;
  int i;
  struct tm t;


  // Read table of all announced leap seconds
  if (nLeapSecs > 0){
    delete [] leapSecGps;
    delete [] leapSecUtc;
    delete [] leapSecN;
    nLeapSecs = 0;
  }

  // Determine the number of entries in the in inifile
  error = Inifile::kSUCCESS;
  while  (error == Inifile::kSUCCESS){
    nLeapSecs = nLeapSecs + 1;
    sprintf(tag,"leap%d", nLeapSecs);

    // First read the format of the inifile entries
    ini->GetFirstString(tag, "", &error);
  }
  nLeapSecs = nLeapSecs - 1;
  //printf("akTimeLib: Found %d leap second entries in %s\n", nLeapSecs, inifile);

  // Allocate memory for the lists and read the data
  if (nLeapSecs > 0){
    leapSecGps = new unsigned long[nLeapSecs];
    leapSecUtc = new unsigned long[nLeapSecs];
    leapSecN = new int[nLeapSecs];

    for (i=0;i<nLeapSecs;i++){
      sprintf(tag,"leap%d", i+1);

      // First read the format of the inifile entries
      month = ini->GetFirstString(tag, "", &error);
      year = ini->GetNextValue(0, &error);
      leapSecN[i] = ini->GetNextValue(0, &error) - 19;

      // Calculate the gps and utc/ntp second counters
      // Warning: The date all refer to UTC, so the timezone
      // has always to be subsracted again!
      if (strstr(month, "jul")) t.tm_mon = 6;
      else t.tm_mon = 0;
      t.tm_year = year - 1900;
      t.tm_mday = 1;
      t.tm_hour = 0;
      t.tm_min = 0;
      t.tm_sec = 0;
      t.tm_isdst = 0;


      leapSecUtc[i] = timegm(&t);
      leapSecGps[i] = timegm(&t) - SLT_GPS_DELAY + leapSecN[i];


      //printf("%d.%d.%d %02d:%02d:%02d  ---  ",
      //  t.tm_mday, t.tm_mon+1, t.tm_year+1900, t.tm_hour, t.tm_min, t.tm_sec);
      //printf("Leap second %3d at NTP %12Ld UTC %12d GPS %12d\n",
      //      leapSecN[i], leapSecUtc[i] + SLT_NTP_DELAY, leapSecUtc[i], leapSecGps[i]);
      if (fout > 0){
        fprintf(fout, "Leap second %3d at NTP %12Ld UTC %12ld GPS %12ld\n",
            leapSecN[i], leapSecUtc[i] + SLT_NTP_DELAY, leapSecUtc[i], leapSecGps[i]);
      }

      // Compare with the known leap seconds - are the
      // entries sensible?

    }

    //
  }


  delete ini;

  return;

}


void akTimeLib::test(){
  int i;
  procDuration dur;

  struct tm t, *tPtr;
  time_t count;

  // Calculate difference between
  // Unix time, UTC/NTP and GPS Clock
  // Base of
  //    * Unix time      1.1.1970 0:00
  //    * UTC            1.1.1972 0:00
  //    * NTP            1.1.1900 0:00
  //    * GPS            6.1.1980 0:00
  //

  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_mday = 1;
  t.tm_mon = 0;
  t.tm_year = 70;

  //printf("Second counter Unix time origin = %d\n", mktime(&t)-timezone);
  count = timegm(&t);
  printf("Second counter Unix time origin (1.1.1970) = %ld", count);
  if (count == 0) printf("\tOk\n");
  else {
    printf("\tError\n");
    printf("--> timegm(\"1.1.1970\") != 0\n");
  }

  count = 0;
  //tPtr = localtime(&count);
  tPtr = gmtime(&count);
  printf("Unix time origin (second counter = 0): %d.%d.%d %02d:%02d:%02d",
        tPtr->tm_mday, tPtr->tm_mon+1, tPtr->tm_year+1900,
	tPtr->tm_hour, tPtr->tm_min, tPtr->tm_sec);
  if ((tPtr->tm_mday == 1) && (tPtr->tm_mon+1 == 1) && (tPtr->tm_year+1900 == 1970)
        && (tPtr->tm_hour == 0) && (tPtr->tm_min == 0) && (tPtr->tm_sec == 0) ) printf("\tOk\n");
  else {
    printf("\tError\n");
    printf("--> localtime(0) != \"1.1.1970\"\n");
  }


  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_mday = 1;
  t.tm_mon = 0;
  t.tm_year = 72;
  count = timegm(&t);
  //printf("UTC time origin (1.1.1972) Second difference to unix = %d\n", mktime(&t)-timezone);
  printf("UTC time origin (1.1.1972) Second difference to unix = %ld\n", count);



  t.tm_year = 0;
  printf("NTP time origin 1.1.1900. Second counter    = %ld\n", timegm(&t));


  t.tm_year = 80;
  t.tm_mday = 6;
  count = timegm(&t);
  printf("GPS origin 6.1.1980. Second counter diff to unix   = %ld", count);
  if (count == SLT_GPS_DELAY) printf("\tOk\n");
  else {
    printf("\tError\n");
    printf(" --> diff should be %d\n", SLT_GPS_DELAY);
  }

  // timezone
  printf("Timezone (Seconds difference to GMT) = %ld\n", timezone);

  // Start of unix time 1.1.1970
  printf("Unix start time 1.1.1970 in NTP seconds (wo system functions)= %lld\n",
    ((long long) 70 * 365 + 17) * 24 * 60 * 60);

  // First leap second date 1.jan 1980
  printf("1.1.1980 in NTP seconds (wo system functions) = %lld\n",
    ((long long) 80 * 365 + 19) * 24 * 60 * 60);


  // Display the leap seconds
  printf("akTimeLib considers %d leap seconds\n", nLeapSecs);
  for (i=0;i<nLeapSecs;i++){
    printf("Leap second %3d at NTP %12lld UTC %12ld GPS %12ld\n",
            leapSecN[i], leapSecUtc[i] + SLT_NTP_DELAY, leapSecUtc[i], leapSecGps[i]);
  }


  // First leap sec in gps epoche (1.7.1981)
  printf("Compare manually with this dates. Use gettime to check the date\n");
  printf("Be aware that gettime always returns localtime. The leap second will not\n");
  printf("necessarily occure at mid night localtime!\n");
  count = ((long) 11 * 365 + 181 + 3) * 24 * 60 * 60;
  printf("Leap second at 1.7.1981 in Unix time %ld\n", count);

  // 1.1.1999
  count = ((long) 29 * 365 + 7) * 24 * 60 * 60;
  printf("Leap second at 1.1.1999 in Unix time %ld\n", count);

  // Last leap second (1.1.2006)
  count = ((long) 36 * 365 + 9) * 24 * 60 * 60;
  printf("Leap second at 1.1.2006 in Unix time %ld\n", count);

  count = ((long) 26 * 365 + 7 - 5) * 24 * 60 * 60 + 14;
  printf("Leap second at 1.1.2006 in GPS time %ld\n", count);

  for (i=-2;i<3;i++){
    printf("%s %s -- GPS %ld -- leap sec %d  -- isLeap %d\n",
       dur.cGpsDate(count+i), dur.cGpsTime(count+i),
       count+i, nLeapSeconds(count+i), isLeapSecond(count+i));
  }


}


time_t akTimeLib::convertGPSToUTC(unsigned long time){
  int i;
  time_t t_utc;
  int actLeapSecs;

  // Find the number of leap seconds
  if (nLeapSecs > 0){
    actLeapSecs = leapSecN[0]-1; // Before the first leapsecond
    for (i=0;i<nLeapSecs;i++){
      if (time >= leapSecGps[i])
        actLeapSecs = leapSecN[i];
    }

  } else {
    actLeapSecs = leapSeconds; // Use the default leap second
  }


  //printf("Found %d leap seconds\n", actLeapSecs);
  t_utc = time + SLT_GPS_DELAY - actLeapSecs;

  return(t_utc);

}


unsigned long akTimeLib::convertUTCToGPS(time_t t_utc){
  int i;
  unsigned long t_gps;
  int actLeapSecs;


  // Find the number of leap seconds
  if (nLeapSecs > 0){
    actLeapSecs = leapSecN[0]-1; // Before the first leapsecond
    for (i=0;i<nLeapSecs;i++){
      if ((unsigned long) t_utc >= leapSecUtc[i])
        actLeapSecs = leapSecN[i];
    }

  } else {
    actLeapSecs = leapSeconds; // Use the default leap second
  }

  //printf("Found %d leap seconds\n", actLeapSecs);
  t_gps = t_utc - (SLT_GPS_DELAY - actLeapSecs);

  return(t_gps);

}


int akTimeLib::isLeapSecond(unsigned long t_gps){
  int i;
  int index;

  // Find the number of leap seconds
  if (nLeapSecs > 0){
    index = 0;
    for (i=0;i<nLeapSecs;i++){
      if (t_gps >= leapSecGps[i])
        index = i;
    }

  } else {
    // Error: Can determine
    return(-1);
  }

  if (t_gps == leapSecGps[index+1]-1)
    return(1);

  return(0);
}


int akTimeLib::nLeapSeconds(unsigned long t_gps){
  int i;
  int actLeapSecs;

  // Find the number of leap seconds
  if (nLeapSecs > 0){
    actLeapSecs = leapSecN[0]-1; // Before the first leapsecond
    for (i=0;i<nLeapSecs;i++){
      if (t_gps >= leapSecGps[i])
        actLeapSecs = leapSecN[i];
    }

  } else {
    actLeapSecs = leapSeconds; // Use the default leap second
  }

  return(actLeapSecs);

}
