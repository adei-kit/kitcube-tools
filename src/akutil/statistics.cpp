/***************************************************************************
    statistics.cpp  -  description

    begin                : Fri Aug 25 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#include "statistics.h"

statistics::statistics(){
  mask = 0xffff;
}
statistics::~statistics(){
}


void statistics::of(unsigned short *data, int Ndata) {
	int i;
	//unsigned long sum=0, sumsq=0;
  double sum=0, sumsq=0;

  min = data[0] & mask;
  max = data[0] & mask;

  for(i=0; i<Ndata; i++) {
    if ((data[i] & mask) < min) min = data[i] & mask;
    if ((data[i] & mask) > max) max = data[i] & mask;

    sum = sum + (double) (data[i] & mask);
		sumsq = sumsq + (double) (data[i] & mask) * (double) (data[i] & mask);
  }


  //printf("n=%d sum=%f sumsq=%f\n",Ndata,sum,sumsq);

  mean = (float)( sum / Ndata);
//	variance = (float) ( ( (double) sumsq * (double) Ndata
//        - (double) sum* (double) sum ) / (Ndata*(Ndata-1))  );
	variance = (float) (  (double) sumsq / (double) Ndata
        - (double) sum / (double) Ndata * (double) sum  / (double) (Ndata)  );


	return;

}


void statistics::of(statistics *stat, int n) {
  int i;

  min = stat[0].min;
  max = stat[0].max;
  mean = 0;
  variance = 0;
  for (i=0;i<n;i++) {
    if (min > stat[i].min) min = stat[i].min;
    if (max < stat[i].max) max = stat[i].max;
    mean += stat[i].mean;
    variance += stat[i].variance;
  }
  mean = mean / n;
  variance = variance / n;

}


unsigned short statistics::calcMin(unsigned short *data, int Ndata) {
	int i;
  unsigned short min = data[0];

	for(i=1; i<Ndata; i++)
    if ((data[i] & mask) < min) min = (data[i] & mask);

	return (min);
}


unsigned short statistics::calcMax(unsigned short *data, int Ndata) {
	int i;
	unsigned short max = data[0];

	for(i=1; i<Ndata; i++)
    if ((data[i] & mask) > max) max = (data[i] & mask);

	return (max);
}


float statistics::calcMean(unsigned short *data, int Ndata) {
	int i;
	unsigned long sum=0;

	for(i=0; i<Ndata; i++)
    sum += (data[i] & mask);

	return ((float) sum/Ndata);
}



float statistics::calcVariance(unsigned short *data, int Ndata) {
	int i;
	unsigned long sumquad=0,sum=0;
	double temp;

	for(i=0; i<Ndata; i++) {
		sumquad += (data[i] & mask) * (data[i] & mask);
		sum += (data[i] & mask);
	}

	temp = -1.0*sum*sum + 1.0*sumquad*Ndata;
//	printf("Ndata*sumquad = %f\tsumquad=%f\tsum=%f\tsum*sum=%f\n",
//         Ndata*sumquad, sumquad,sum,sum*sum);

	return (float) (temp/(Ndata*(Ndata-1)));
}


void statistics::display(FILE *fout) {
  fprintf(fout,"Min  %10d  hex %10x\n", min, min);
  fprintf(fout,"Max  %10d  hex %10x\n", max, max);
  fprintf(fout,"Mean %10f  hex %10x\n", mean,     (unsigned short) mean);
  fprintf(fout,"Var  %10f  hex %10x\n", variance, (unsigned short) variance);
}



void statistics::add(statistics *stat) {
  min += stat->min;
  max += stat->max;
  mean += stat->mean;
  variance += stat->variance;
}


void statistics::minus(statistics *stat) {
  min = min - stat->max;
  max = max - stat->min;
  mean = mean - stat->mean;
  variance = variance + stat->variance;
}


