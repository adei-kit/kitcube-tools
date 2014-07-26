/***************************************************************************
    analyseMatrix.cpp  -  description

    begin                : Tue Apr 18 2006
    copyright            : (C) 2006 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#include <cmath>

#include "analysematrix.h"


analyseMatrix::analyseMatrix(){

  // Load last noisy pixel map?!
  // Read noisy pixel detection parameters

  noisyNVariances = 5.0;  // Tolerated varianve changes proportional to 
                        // variance distribution over the telescope
  noisyMinVariance = 5; // Tolerated varinance deviations

  noisyNeighborDiff = 1.0; // Difference between neighboring pixel relative to distribution mean

  noisyAbsThresh = 2.0; // Absoltut threshold relative to distribution mean

  noisyLowThresh = 0.3;  // If less than one third of the other cound as error pixel


  // Matrix of noisy pixel for a telescope
  noisyRuns = 0;
  setMatrix(0, noisyPixel);
  setMatrix(0, faultyPixel);
  
  // debug flag - default is no debug output
  debug = 0;
  
}


analyseMatrix::~analyseMatrix(){
}



// Analysing functions
void analyseMatrix::findHighNoise1(FILE *fout, double pVar[20][22], int res[20][22], int err[20][22]){
  int i, j;
  double maxMeanDiff;
  //double maxMeanDiffVirt;
  int nDetected;
  //int tel;
  double mean, var;
  int n;


  //tel = Pbus::getTelescope()-1;

    nDetected = 0;

    meanMatrix(pVar, &mean, &var, &n, err);

    maxMeanDiff = this->noisyNVariances * var;  
    //maxMeanDiffVirt = this->noisyNVariances * varVirt;
    if (maxMeanDiff < 5) maxMeanDiff = this->noisyMinVariance;
    //if (maxMeanDiffVirt < 5) maxMeanDiffVirt = this->noisyMinVariance;


    for (i=0;i<20;i++){
      for (j=0;j<22;j++){
        //if (j<22){ // normal channel
          if (fabs (pVar[i][j] - mean)    >  maxMeanDiff) {
            //if (!noisyPixel[tel][i][j]) {
            if (res[i][j] == 0) {
              if (nDetected %7 == 0) fprintf(fout, "\n%22s", "");
              fprintf(fout, " (%02d/%02d)", i+1, j+1);
              //fprintf(fout, "Pixel (%02d/%02d) might be noisy pixel: %6.2f  diff=%6.2f\n",
              //       i+1, j+1, pVar[i][j], fabs (pVar[i][j] - mean));

              //noisyPixel[tel][i][j] = 1; 
              res[i][j] = 1;
              nDetected += 1;
            }
          } else {
            //if (noisyPixel[tel][i][j]) {
            if (res[i][j]) {
              //fprintf(fout, "Pixel (%02d/%02d) not noisy anymore?    %6.2f  diff=%6.2f\n",
              //       i+1, j+1, pVar[i][j], fabs (pVar[i][j] - mean));
            }
          }
/*
        } else { // virtual channel
          if (fabs (pVar[i][j] - meanVirt) > maxMeanDiffVirt) {

            if (!noisyPixel[tel][i][j]) {
              if (nDetected %7 == 0) fprintf(fout, "\n%22s", "");
              fprintf(fout, " (%02d/%02d)", i+1, j+1);
              //fprintf(fout, "Pixel (%02d/%02d) might be noisy pixel: %6.2f  diff=%6.2f\n",
              //       i+1, j+1, pVar[i][j], fabs (pVar[i][j] - meanVirt));
              noisyPixel[tel][i][j] = 1; 
              nDetected += 1;
            }
          } else {
            if (noisyPixel[tel][i][j]) {
              //fprintf(fout, "Pixel (%02d/%02d) not noisy anymore?    %6.2f  diff=%6.2f\n",
              //       i+1, j+1, pVar[i][j], fabs (pVar[i][j] - meanVirt));
            }

          }

        }
*/

      }
    }
    if (nDetected == 0) fprintf(fout, " none\n");
    else fprintf(fout, "\n");
    //fprintf(fout, "   (--> \"getnoisy\")\n");
    //fprintf(fout, "A complete list of the detected noisy pixel can be displayed with \"getnoisy\"\n");


}



void analyseMatrix::findHighNoise2(FILE *, double matrix[20][22], int res[20][22], int err[20][22]){
    double pmtMean, pmtVar;
    int pmtN;
    int maskAll[20][22];
    int filter00[2][2] = { {  3, -1}, {-1, -1}};
    int filter11[2][2] = { { -1, -1}, {-1,  3}};
    int filter01[2][2] = { { -1,  3}, {-1, -1}};
    int filter10[2][2] = { { -1, -1}, { 3, -1}};
    double pmtFilter[20][22];
    int maskTmp[20][22];
    int maskNoisy[20][22];
    int maskNegs[20][22];
    int maskHighVars[20][22];
    double thresh;
    int up;


    // Do not consider error pixel
    copyMask(maskAll, err);

    setMatrix(0, maskNegs);
    setMatrix(0, maskNoisy);


    // 1. Remove negative variances
    classifyByThreshold(0, matrix, maskTmp);
    invertMask(maskTmp);
    orMask(maskNegs, maskTmp);
    orMask(maskAll, maskTmp);  // All contains all pixel but the negative ones

    // 2. Run edge filter
    //displayMatrix(fout, pmtVar, "PMT Variances"); 
    meanMatrix(matrix, &pmtMean, &pmtVar, &pmtN, maskAll);
    if (debug > 1) printf("Distribution mean  %.2f+-%.2f (n=%d)\n", pmtMean, pmtVar, pmtN);

    if (debug > 1) displayMatrix(stdout, maskAll, "Faulty pixel mask");

    if (this->noisyNeighborDiff < 0){
      //thresh = this->noisyNeighborDiff;
      thresh = this->noisyNeighborDiff * pmtMean;
      up = 0;
    } else { 
      thresh = this->noisyNeighborDiff * pmtMean;
      up = 1;
    }
    if (debug > 1) printf("Threshold = %f\n", thresh);


    filterBy2x2(filter00, 0, 0, matrix, pmtFilter, maskAll);
    if (debug > 1) displayMatrix(stdout, pmtFilter, "Filter result");
    classifyByThreshold(thresh, pmtFilter, maskTmp, up);
    orMask(maskNoisy, maskTmp);

    filterBy2x2(filter11, 1, 1, matrix, pmtFilter, maskAll);
    classifyByThreshold(thresh, pmtFilter, maskTmp, up);
    orMask(maskNoisy, maskTmp);

    filterBy2x2(filter01, 0, 1, matrix, pmtFilter, maskAll);
    classifyByThreshold(thresh, pmtFilter, maskTmp, up);
    orMask(maskNoisy, maskTmp);

    filterBy2x2(filter10, 1, 0, matrix, pmtFilter, maskAll);
    classifyByThreshold(thresh, pmtFilter, maskTmp, up);
    orMask(maskNoisy, maskTmp);

    if (debug > 1) displayMatrix(stdout, maskNoisy, "Edge filter result mask");


    // 3. Absolut thresh
    // Add maskAll to the noisies
    copyMask(maskTmp, maskNoisy);
    orMask(maskTmp, maskAll);
    meanMatrix(matrix, &pmtMean, &pmtVar, &pmtN, maskTmp);
    if (debug > 1) printf("Distribution mean  %.2f+-%.2f (n=%d)\n", pmtMean, pmtVar, pmtN);

    if (this->noisyAbsThresh < 0){
      thresh = fabs(this->noisyAbsThresh) * pmtMean;
      up = 0;
    } else {
      thresh = this->noisyAbsThresh * pmtMean;
      up = 1;
    }

    // Mean of the noisy pixel
    copyMask(maskHighVars, maskNoisy);
    invertMask(maskHighVars);
    orMask(maskHighVars, maskAll);
    meanMatrix(matrix, &pmtMean, &pmtVar, &pmtN, maskHighVars);
    //fprintf(fout, "Distribution mean  %.2f+-%.2f (n=%d)\n", pmtMean, pmtVar, pmtN);


    //thresh = pmtMean - this->noisyNeighborDiff/2;

    // Try to use the higher level as threshold for the low var area
    // Are there pixel that were not found in the 
    //if (nMask(maskNoisy) > 0){
      classifyByThreshold(thresh, matrix, maskTmp, up);
      //fprintf(fout, "Found n=%d more pixel with threshold = %.2f\n", 
      //             nMask(maskTmp), pmtMean - this->noisyNeighborDiff/2);
      orMask(maskNoisy, maskTmp);
      //displayMatrix(fout, maskNoisy, "PMT Noisy pixel?");

      // Add maskAll to the noisies
      copyMask(maskTmp, maskNoisy);
      orMask(maskTmp, maskAll);
      meanMatrix(matrix, &pmtMean, &pmtVar, &pmtN, maskTmp);
      //fprintf(fout, "Distribution mean  %.2f+-%.2f (n=%d)\n", pmtMean, pmtVar, pmtN);

/*
      // Mean of the noisy pixel
      copyMask(maskHighVars, maskNoisy);
      invertMask(maskHighVars);
      orMask(maskHighVars, maskAll);
      meanMatrix(matrix, &pmtMean, &pmtVar, &pmtN, maskHighVars);
      //fprintf(fout, "Distribution mean  %.2f+-%.2f (n=%d)\n", pmtMean, pmtVar, pmtN);
      //fprintf(fout, "Number of errors n = %d\n", nMask(maskAll));

*/
    //}


    // 4. Skip Low noise pixel - add them to the error mask
    up = 0;
    thresh = this->noisyLowThresh * pmtMean;
    classifyByThreshold(thresh, matrix, maskTmp, up);

    copyMask(maskAll, maskTmp);


    // Copy the results
    copyMask(res, maskNoisy);
    copyMask(err, maskAll);

}




void analyseMatrix::displayMatrix(FILE *fout, double matrix[20][22], const char *title){
  int i, j;

  fprintf(fout,"%-50s                            column\n", title);
  fprintf(fout,"row|");
  for (i=0;i<20;i++) fprintf(fout,"%4d",i+1);
  fprintf(fout,"\n");
  fprintf(fout,"---+--------------------------------------------------------------------------------");

  for (j=0;j<22;j++){
    fprintf(fout,"\n%3d|",j+1);
    for (i=0;i<20;i++){
      fprintf(fout, "%4.0f", matrix[i][j]);
    }
  }
  fprintf(fout, "\n");

}


void analyseMatrix::displayMatrix(FILE *fout, int matrix[20][22], const char *title, int norm,
                             int value, const char *tag){
  int i, j;

  if (norm > 0)
    fprintf(fout,"%-50s             norm = %-6d  column\n", title, norm);
  else 
    fprintf(fout,"%-50s             %13s  column\n", title, "");

  fprintf(fout,"row|");
  for (i=0;i<20;i++) fprintf(fout,"%4d",i+1);
  fprintf(fout,"\n");
  fprintf(fout,"---+--------------------------------------------------------------------------------");

  for (j=0;j<22;j++){
    fprintf(fout,"\n%3d|",j+1);
    for (i=0;i<20;i++){
      if (matrix[i][j] == 0) fprintf(fout, "   .");
      else { 
        if (norm > 0)
          fprintf(fout, "%4.0f", (double) matrix[i][j] / norm * 100);
        else 
          if ((tag > 0) && (matrix[i][j] == value))
            fprintf(fout, "%4s", tag);
          else
            fprintf(fout, "%4d", matrix[i][j]);
      }
    }
  }
  fprintf(fout, "\n");
}


void analyseMatrix::setMatrix(double value, double matrix[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      matrix[i][j] = value;

}


void analyseMatrix::multMatrix(double value, double matrix[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      matrix[i][j] = value * matrix[i][j];

}


void analyseMatrix::multMask(int value, int mask[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      mask[i][j] = value * mask[i][j];

}


void analyseMatrix::copyMatrix(double res[20][22], double matrix[20][22]){
  int i, j;

  for (i=0;i<20;i++){
    for (j=0;j<22;j++){
      res[i][j] =  matrix[i][j];
    }
  }

}


void analyseMatrix::divMatrix(double res[20][22], double matrix[20][22]){
  int i, j;

  for (i=0;i<20;i++){
    for (j=0;j<22;j++){
      try{        
        res[i][j] = res[i][j] / matrix[i][j];
      } catch (...){
        res[i][j] = 0;
      }     
    }
  }
  
}


void analyseMatrix::divMatrix(double res[20][22], int matrix[20][22]){
  int i, j;

  for (i=0;i<20;i++){
    for (j=0;j<22;j++){
      try{
        res[i][j] = res[i][j] / matrix[i][j];
      } catch (...){
        res[i][j] = 0;
      }
    }
  }

}

int analyseMatrix::displayPixelList(FILE *fout, int matrix[20][22]){
  int i, j, n;

  n = 0;
  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      if (matrix[i][j] > 0){
         if (n%10 == 0) fprintf(fout, "\n    ");
         fprintf(fout, " (%02d/%02d)", i+1, j+1);
         n = n + 1;
      }

  fprintf(fout, "\n");

  return(n);
}

void analyseMatrix::setMatrix(int value, int matrix[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      matrix[i][j] = value;

}

void analyseMatrix::orMask(int res[20][22], int mask[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      if ((res[i][j]==1) || (mask[i][j]==1))
        res[i][j] = 1;

} 


void analyseMatrix::addMask(int res[20][22], int mask[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      res[i][j]= res[i][j] + mask[i][j];

} 


void analyseMatrix::addMatrix(double res[20][22], double matrix[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      res[i][j]= res[i][j] + matrix[i][j];

} 


void analyseMatrix::invertMask(int mask[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
       if (mask[i][j] == 0) mask[i][j] = 1;
       else mask[i][j] = 0;

}


int analyseMatrix::nMask(int mask[20][22]){
  int i, j, n;

  n = 0;
  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
       if (mask[i][j] > 0) n = n +1;

  return(n);
}


void analyseMatrix::copyMask(int res[20][22], int mask[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      res[i][j] = mask[i][j];

} 


void analyseMatrix::maxMask(int max, int mask[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      if (max > mask[i][j]) mask[i][j] = max;

}


void analyseMatrix::minMask(int min, int mask[20][22]){
  int i, j;

  for (i=0;i<20;i++)
    for (j=0;j<22;j++)
      if (mask[i][j] > min) mask[i][j] = min;

}


void analyseMatrix::meanMatrix(double matrix[20][22], double *mean, double *var, int *n, int mask[20][22]){
  int i, j;
  double sum;
  double sumSq;
  int nSum;
  sum = 0;
  sumSq = 0;
  nSum = 0;
  for (i=0;i<20;i++){
    for (j=0;j<22;j++){
      if (mask[i][j] == 0){
         sum += matrix[i][j];
         sumSq += matrix[i][j] * matrix[i][j];
         nSum += 1;
      }
    }
  }
 
  if (nSum > 0){
    *n = nSum;
    *mean = sum  / nSum;
    *var =  sumSq / nSum  - sum / nSum * sum / nSum;
    if (*var > 0) *var = sqrt(*var);

  } else {
    *n = 0;
    *mean = 0;
    *var = 0;
  }

}


void analyseMatrix::meanMatrix(double matrix[20][22], 
              double *mean, double *var, int *n, int mask[20][22], int err[20][22]){

  int tmpMask[20][22];

  copyMask(tmpMask, mask);
  orMask(tmpMask, err);

  meanMatrix(matrix, mean, var, n, tmpMask);

}


void analyseMatrix::classifyByThreshold(double thresh, double matrix[20][22], int res[20][22], int up){
  int i, j;


  //printf("Thresh %f\n", thresh);
 

  // Set result to zero
  setMatrix(0, res);

  // All value hight than the trahold will be marked as one
  for(i=0;i<20;i++)
   for(j=0;j<22;j++){
     if (up > 0){
       if (matrix[i][j] > thresh) res[i][j]=1;
     } else {
       if (matrix[i][j] < thresh) res[i][j]=1;
     }
     //printf(" %.0f %d", matrix[i][j], res[i][j]);
   }
}




void analyseMatrix::filterBy2x2(int filter[2][2], int col, int row, 
          double matrix[20][22], double res[20][22], int mask[20][22]){
  int i,j, k, l;
  int weight;    // Sum of all pixel wo center pixel 

  // Check that center and sourounding has the same weight
  // Not important, will be normlized anyhow...

  // Clear result matrix
  setMatrix(0, res);
  

  for (i=0;i<19;i++)
    for(j=0;j<21;j++){

      // Calculate the filter weight + res
      weight = 0;
      for (k=0;k<2;k++)
        for(l=0;l<2;l++)
          // Add the surrounding pixel but not the center
          // Add only if not disabled in mask
          if (!((k==col) && (l==row)) && (mask[i+k][j+l] == 0)){
            //printf(".");
            weight += filter[k][l];
            res[i+col][j+row] += filter[k][l] * matrix[i+k][j+l];
          }

      //printf("%.0f ", res[i+col][j+row]);

      // Add center
      if (mask[i+col][j+row] == 0){
        weight = weight * (-1);
        res[i+col][j+row] += weight * matrix[i+col][j+row];

        //printf("w=%d %.0f ", weight, res[i+col][j+row]);

        // Normalize
        if (weight > 0)
          res[i+col][j+row] = res[i+col][j+row]/weight;
      }
    }

}

void analyseMatrix::clearNoisyPixel(){
  setMatrix(0, noisyPixel);
  noisyRuns = 0;
}


void analyseMatrix::clearFaultyPixel(){
  setMatrix(0, faultyPixel);
}


void analyseMatrix::addNoisyPixel(int mask[20][22]){
    addMask(noisyPixel, mask);
    noisyRuns += 1;
}


void analyseMatrix::findHighNoise(FILE *fout, double matrix[20][22]){

  findHighNoise2(fout, matrix, noisyPixelLast, faultyPixel);
  addNoisyPixel(noisyPixelLast);
  
}

void analyseMatrix::meanOfNormalPixel(double matrix[20][22], 
              double *mean, double *var, int *n){

  meanMatrix(matrix, mean, var, n, noisyPixelLast, faultyPixel);           
              
}


void analyseMatrix::meanOfNoisyPixel(double matrix[20][22], 
              double *mean, double *var, int *n){
  int highNoiseMask[20][22];
              
  copyMask(highNoiseMask, noisyPixelLast);
  invertMask(highNoiseMask);
  meanMatrix(matrix, mean, var, n, highNoiseMask, faultyPixel);           
              
}


int analyseMatrix::nNoisyPixel(){

  return(nMask(noisyPixelLast));

}


void analyseMatrix::displayNoisyPixelList(FILE *fout){

  displayPixelList(fout, noisyPixelLast);
}


