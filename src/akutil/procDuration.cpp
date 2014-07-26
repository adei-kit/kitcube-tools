/***************************************************************************
    procDuration.cpp  -  description

    begin                : Thu Jun 29 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/



#include <cstring>

#include "aktimelib.h"
#include "procDuration.h"


const char text[]="Andreas";


void my_sleep(short seconds) {
	long start,finish;
	time(&start);
	finish = start;
	while(finish-start < seconds) {time(&finish);}
}



const char procDuration::dateformat[] = "%02d.%02d.%04d";

const char procDuration::timeformat[] = "%02d:%02d:%02d";

const char procDuration::zoneformat[] = "%s";

char procDuration::datestring[] = "dd.mm.jjjj";
char procDuration::timestring[] = "hh:mm:ss";
char procDuration::zonestring[] = "           ";


procDuration::procDuration(){
   startTick = 0;
   startTime.tv_sec = 0;
   startTime.tv_usec = 0;

}


void procDuration::setStart()
{
   startTick=clock();
   gettimeofday( &startTime, &startZone);

   return;
}

void procDuration::setStart(unsigned long time){

   startTick = 0;
   startTime.tv_sec = time;
   startTime.tv_usec = 0;

   return;
}


void procDuration::setEnd()
{
   endTick=clock();
   gettimeofday( &endTime, &endZone );


   durationCPU = ((double) (endTick-startTick))/CLOCKS_PER_SEC;
   durationRT =  (((double)(endTime.tv_sec-startTime.tv_sec)) * 1000000
                        +  ( (double)(endTime.tv_usec-startTime.tv_usec) )) / 1000000;


   return;
}


void procDuration::setEndFromStart(procDuration *dur){

   endTick = 0;
   endTime.tv_sec = dur->getStartSec();
   endTime.tv_usec = dur->getStartMicroSec();

   durationCPU = (double) (endTick-startTick)/CLOCKS_PER_SEC;
   durationRT =  (double) ((endTime.tv_sec-startTime.tv_sec) * 1000000
                        + (endTime.tv_usec-startTime.tv_usec) ) / 1000000;

}

double procDuration::getDurationCPU()
{
   return(durationCPU);
}


double procDuration::getDurationRT()
{
   return(durationRT);
}


void procDuration::displayDuration(FILE *fout)
{
   fprintf(fout,"\nElaspsed CPU time %.6fs\n",durationCPU);
   fprintf(fout,"Real-time: %.6fs\n",durationRT);

   return;
}


void procDuration::displayDuration()
{
  displayDuration(stdout);
}



void procDuration::displayTransferRate(FILE *fout, double blocksize) // kB
{
   fprintf(fout,"\nNetto transfer rate:    %.3fkB/s block %.3fkB\n",
              blocksize / durationCPU, blocksize);
   fprintf(fout,  "Efective transfer rate: %.3fkB/s\n",
              blocksize / durationRT);

   return;
}


void procDuration::displayTransferRate(double blocksize)
{
  displayTransferRate(stdout,blocksize);
}


void procDuration::displayStartTime(FILE *fout)
{
  struct tm *zeit;

  zeit = localtime((const time_t *) &startTime.tv_sec);
  //fprintf(fout,ctime(&startTime.tv_sec));
  fprintf(fout,"%02d:%02d:%02d %s",zeit->tm_hour,
            zeit->tm_min,zeit->tm_sec,zeit->tm_zone);

}

void procDuration::displayStartDate(FILE *fout)
{
  struct tm *zeit;

  zeit = localtime( (const time_t *) &startTime.tv_sec);
  //fprintf(fout,ctime(&startTime.tv_sec));
  fprintf(fout,"%d.%d.%d",zeit->tm_mday,zeit->tm_mon+1,
                           zeit->tm_year+1900);

}



unsigned long procDuration::getStartSec(){
  return(startTime.tv_sec);
}

unsigned long procDuration::getEndSec(){
  return(endTime.tv_sec);
}

int procDuration::getStartMiliSec(){
  return(startTime.tv_usec/1000);
}

int procDuration::getEndMiliSec(){
  return(endTime.tv_usec/1000);
}


int procDuration::getStartMicroSec(){
  return(startTime.tv_usec);
}

int procDuration::getEndMicroSec(){
  return(endTime.tv_usec);
}


int procDuration::getStartNanoSec(){
  return(startTime.tv_usec*1000);
}

int procDuration::getEndNanoSec(){
  return(endTime.tv_usec*1000);
}


void procDuration::meanTime(procDuration &dur){
  int res;
  unsigned long sec, usec;
  int sign;

  startTick = 0;
  // Differnce between start and end value
  dur.getDurationRT(&sign, &sec, &usec);

  // Half of the difference
  res = (sec * 1000000 + usec) / 2 ;
  usec = res % 1000000;
  sec = res / 1000000;

  // Add to the first counter
  res = 1000000 + dur.getStartMicroSec() + sign * usec; 
  startTime.tv_usec = res % 1000000;

  res = dur.getStartSec() + sign * sec + res / 1000000 - 1;
  startTime.tv_sec = res;

}


void procDuration::getDurationRT(int *sign, unsigned long *sec, unsigned long *usec){
  int result;

  result = 1000000 + endTime.tv_usec - startTime.tv_usec;
  diffTimeMicroSec = result % 1000000;

  result = endTime.tv_sec - startTime.tv_sec + result/1000000 - 1; 
  if (result < 0){
    diffTimeMicroSec = 1000000 - diffTimeMicroSec;
    diffTimeSec = (result + 1) * (-1);
    diffTimeSign = -1;
  } else {
    diffTimeSec = result;
    diffTimeSign = 1;
  }

  *sign = diffTimeSign;
  *sec = diffTimeSec;
  *usec = diffTimeMicroSec;
}


const char * procDuration::cStartTime(){
  struct tm *zeit;

  zeit = localtime( (const time_t *) &startTime.tv_sec);
  sprintf(timestring,timeformat,zeit->tm_hour,zeit->tm_min,zeit->tm_sec);
  return(timestring);
}


const char * procDuration::cEndTime(){
  struct tm *zeit;

  zeit = localtime( (const time_t *) &endTime.tv_sec);
  sprintf(timestring,timeformat,zeit->tm_hour,zeit->tm_min,zeit->tm_sec);
  return(timestring);
}


const char * procDuration::cStartDate(){
  struct tm *zeit;

  zeit = localtime( (const time_t *) &startTime.tv_sec);
  sprintf(datestring,dateformat,zeit->tm_mday,zeit->tm_mon+1,
                                 zeit->tm_year+1900);
  return(datestring);
}

const char * procDuration::cEndDate(){
  struct tm *zeit;

  zeit = localtime( (const time_t *) &endTime.tv_sec);
  sprintf(datestring,dateformat,zeit->tm_mday,zeit->tm_mon+1,
                                 zeit->tm_year+1900);
  return(datestring);
}

const char * procDuration::cStartZone(){
  struct tm *zeit;

  zeit = localtime((const time_t *) &startTime.tv_sec);
  sprintf(zonestring,zoneformat, zeit->tm_zone);
  return(zonestring);
}

const char * procDuration::cEndZone(){
  struct tm *zeit;

  zeit = localtime((const time_t *) &endTime.tv_sec);
  sprintf(zonestring,zoneformat, zeit->tm_zone);
  return(zonestring);
}


const char * procDuration::cTime(time_t *t){
  struct tm *zeit;

  zeit = localtime(t);
  sprintf(timestring,timeformat,zeit->tm_hour,zeit->tm_min,
              zeit->tm_sec);

  return(timestring);
}

const char *procDuration::cGpsTime(unsigned long t){
  struct tm *zeit;
  time_t t_utc;
  int isLeapSec;
  akTimeLib *tlib;
  tlib = akTimeLib::getReference();

  t_utc = tlib->convertGPSToUTC(t);
  isLeapSec = tlib->isLeapSecond(t);
  if (isLeapSec == 1){
    t_utc = t_utc -1;
    zeit = localtime(&t_utc);
    zeit->tm_sec = zeit->tm_sec + 1;
  } else {
    zeit = localtime(&t_utc);
  }
  sprintf(timestring,timeformat,zeit->tm_hour,zeit->tm_min,
              zeit->tm_sec);


  return(timestring);
}



const char * procDuration::cDate(time_t *t){
  struct tm *zeit;

  zeit = localtime(t);
  sprintf(datestring, dateformat, zeit->tm_mday,zeit->tm_mon+1,
              zeit->tm_year+1900);
  return(datestring);
}

const char * procDuration::cGpsDate(unsigned long t){
  struct tm *zeit;
  time_t t_utc;
  int isLeapSec;
  akTimeLib *tlib;
  tlib = akTimeLib::getReference();


  t_utc = tlib->convertGPSToUTC(t);
  isLeapSec = tlib->isLeapSecond(t);
  if (isLeapSec == 1){
    t_utc = t_utc -1;
    zeit = localtime(&t_utc);
  } else {
    zeit = localtime(&t_utc);
  }
  sprintf(datestring, dateformat, zeit->tm_mday,zeit->tm_mon+1,
              zeit->tm_year+1900);
  return(datestring);
}

const char * procDuration::cZone(time_t *t){
  struct tm *zeit;

  zeit = localtime(t);
  sprintf(zonestring, zoneformat, zeit->tm_zone);

  return(zonestring);
}

procDuration procDuration::operator=(procDuration &dur){

   startTick = dur.startTick;
   endTick = dur.endTick;

   startTime.tv_sec = dur.startTime.tv_sec;
   startTime.tv_usec = dur.startTime.tv_usec;
   startZone.tz_minuteswest = dur.startZone.tz_minuteswest;
   startZone.tz_dsttime = dur.startZone.tz_dsttime;
   endTime.tv_sec = dur.endTime.tv_sec;
   endTime.tv_usec = dur.endTime.tv_usec;
   endZone.tz_minuteswest = dur.endZone.tz_minuteswest;
   endZone.tz_dsttime = dur.endZone.tz_dsttime;

   durationCPU = durationCPU;
   durationRT = durationRT;

   diffTimeSign = dur.diffTimeSign;
   diffTimeSec = dur.diffTimeSec;
   diffTimeMicroSec = dur.diffTimeMicroSec;

   return( *this);
}


void procDuration::set(procDuration dur){

   startTime.tv_sec = dur.startTime.tv_sec;
   startTime.tv_usec = dur.startTime.tv_usec;

   endTime.tv_sec = dur.endTime.tv_sec;
   endTime.tv_usec = dur.endTime.tv_usec;

   durationRT =  (double) ((endTime.tv_sec-startTime.tv_sec) * 1000000
                        + (endTime.tv_usec-startTime.tv_usec) ) / 1000000;


}




