/***************************************************************************
    statistics.h  -  description

    begin                : Fri Aug 25 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#ifndef STATISTICS_H
#define STATISTICS_H

#include <cstdio>

/** Provide statistics for a time series.
  * The statistic is described by the minimum
  * and maximum value, the mean value and the
  * variance of the series.
  *
  * @ingroup akutil_group
  */

class statistics {
public: 
  statistics();
  ~statistics();

  statistics operator=(statistics zuw) { // ???

    this->min = zuw.min;
    this->max = zuw.max;
    this->mean = zuw.mean;
    this->variance = zuw.variance;

  return(*this); // MS VC++ wants it ?!
  }

  /** Calculate the statistics a time series
    * defined by n values of type unsigned short. */
  void of(unsigned short *data, int n);

  /** Calculate the statistics of a time series
    * that has been analysed in n segments of
    * equal length */
  void of(statistics *stat, int n);

  float calcMean(unsigned short *data, int n);
  unsigned short calcMin(unsigned short *data, int n);
  unsigned short calcMax(unsigned short *data, int n);
  float calcVariance(unsigned short *data, int n);

  void display(FILE *fout);

  /** Set mask to select the data and remove possible
    * flags */
  void setMask(unsigned short mask) {
    this->mask = mask;
  }

  /** Add another statistical set */
  void add(statistics *stat);

  /** Substract another statistical set */
  void minus(statistics *stat);
  
    
  
// private:
  
  unsigned short min;
  unsigned short max;
  float mean;
  float variance;

private:

  /** Mask for the data */
  unsigned short mask;
  
};

#endif
