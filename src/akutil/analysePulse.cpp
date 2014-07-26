/***************************************************************************
    analysePulse.cpp  -  description

    begin                : Fri Aug 25 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#include "analysePulse.h"

analysePulse::analysePulse(){
}
analysePulse::~analysePulse(){
}

void analysePulse::of(unsigned short *data, int Ndata,statistics *noise){

        threshC = (float) (noise->mean + 4.0 * noise->variance);
        //threshC = (float) (noise->mean + 0 * noise->variance);
        pedestal = (unsigned short) (noise->mean + 0.5);

        calcCharge(data,Ndata,threshC,pedestal);

        threshW = (float) (noise->mean + 6.0 * noise->variance);
        //threshW = (float) (noise->mean + 0 * noise->variance);
        calcWidth(data,Ndata,threshW);

}


int analysePulse::calcWidth(unsigned short *data, int Ndata, float thresh) {
	int i;

  width=0;
	for(i=0; i<Ndata; i++)
    if (data[i] > thresh)  width++;

	return (width);
}


unsigned long analysePulse::calcCharge(unsigned short *data, int Ndata,
             float thresh, unsigned short ped) {
	int i;

  charge=0;
	for(i=0; i<Ndata; i++)
    if (data[i] > thresh)  charge += data[i] - ped;

	return (charge);
}

