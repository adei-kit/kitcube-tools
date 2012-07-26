/***************************************************************************
    analyseNoise.cpp  -  description

    begin                : Fri Aug 25 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#include <akutil/analyseNoise.h>

analyseNoise::analyseNoise():statistics(){
}
analyseNoise::~analyseNoise(){
}


void analyseNoise::of(unsigned short *data, int n, int seg, int skip)
{
  int i, j, nseg, ndata, imax;
  statistics *segstat;
  float varmax;

  // Calc the number of segments!
  // the rest of the data is not considered !
  nseg = n / seg;
  ndata = seg * nseg;

  // allocate new stat set
  segstat = new statistics[nseg];

  // calc stat.
  for (i=0;i<nseg;i++)
    segstat[i].of(data + (i*seg), seg);

  // skip segments with high variance
  for (j=0;j<skip;j++) {
    imax = 0;
    varmax = segstat[0].variance;
    for (i=1;i<nseg-j;i++)
       if (varmax < segstat[i].variance) imax = i;

    segstat[imax] = segstat[nseg-j];
  }

  // calc overall stat. parameter
  statistics::of(segstat,nseg-skip);

  // free stat sets
  delete[] segstat;

}

