/***************************************************************************
    analysePulse.h  -  description

    begin                : Fri Aug 25 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#ifndef ANALYSEPULSE_H
#define ANALYSEPULSE_H

#include <akutil/statistics.h>

/** Analyse the properties of a pulse in a
  * given time series.
  *
  * @ingroup akutil_group
  */

class analysePulse {
public: 
	analysePulse();
	~analysePulse();

  void of(unsigned short *data, int n, statistics *noise);

  int calcWidth(unsigned short *data, int n, float tresh);
  unsigned long calcCharge(unsigned short *data, int n, float tresh,
       unsigned short pedestal);

  int width;

  unsigned long charge;

  float threshW;

  float threshC;

  unsigned short pedestal;

};

#endif
