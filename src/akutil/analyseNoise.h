/***************************************************************************
    analyseNoise.h  -  description

    begin                : Fri Aug 25 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#ifndef ANALYSENOISE_H
#define ANALYSENOISE_H

#include <akutil/statistics.h>

/** Analyse the noise level of a
  * given time series.
  *
  * @ingroup akutil_group
  */

class analyseNoise : public statistics {
public: 
	analyseNoise();
	~analyseNoise();

  /** Analyse the noise level of a given
    * time series. The alogrithm divides the
    * series in segments. The segments with
    * the highest variance are not consider
    * for the estimation of the noise level
    * parameters.
    * @param data Pointer to the time series
    * @param n Length of the time series
    * @param seg Length of the segements
    * @param skip Number of segments to leave out
    */
  void of(unsigned short *data, int n, int seg, int skip);


};

#endif
