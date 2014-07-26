/***************************************************************************
    analyseMatrix.h  -  description

    begin                : Tue Apr 18 2006
    copyright            : (C) 2006 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#ifndef ANALYSEMATRIX_H
#define ANALYSEMATRIX_H

#include <cstdio>


/** Analyse the variance matrix of a telescope
  * All functions are on matrices of 20x22 elements.
  *
  * @ingroup akutil_group
  */

class analyseMatrix {
public: 
  analyseMatrix();
  ~analyseMatrix();

  /** Filter a matrix using a free 2x2 matrix.
    * With mask pixel can be excluded from the calculation
    * matrix, res and mask are 20x22 pixel 
    * Parameters (col/row) determine the center pixel of the 
    * filter. Possible values are col, row = {0|1}.
    */
  void filterBy2x2(int filter[2][2], int col, int row, 
           double matrix[20][22], double res[20][22], int mask[20][22]);

  /** Display a matrix of 20x22 elements */
  void displayMatrix(FILE *fout, double matrix[20][22], const char *title);

  /** Display a matrix of 20x22 integer elements.
    * With the argument norm > 0, relative values in percent
    * a the normation given will be displayed.
    * A special value can be replaced by a tag.
    */
  void displayMatrix(FILE *fout, int matrix[20][22], const char *title, int norm = 0, 
                      int value = 0, const char *tag = 0);

  /** Display list of coordinates of all pixel in the mask 
    * Returns the number of pixel in the mask */
  int displayPixelList(FILE *fout, int matrix[20][22]);

  /** Classify by using a constant threshold
    * matrix, res and mask are 20x22 pixel.
    * The up parameter controls whether the values higher
    * or lower of the threshold should be taken.
    * 
    */
  void classifyByThreshold(double thresh, double matrix[20][22], int res[20][22], int up = 1);

  /** Set matrix to constant value */
  void setMatrix(double value, double matrix[20][22]);
  void setMatrix(int value, int matrix[20][22]);

  /** Multiply matrix by constant */
  void multMatrix(double value, double matrix[20][22]);  

  /** Multiply mask by constant */
  void multMask(int value, int mask[20][22]);
  
  /** Devide by matrix */
  void divMatrix(double res[20][22], double matrix[20][22]);
  
  /** Devide by matrix */
  void divMatrix(double res[20][22], int matrix[20][22]);
  
  /** Calculate the mean value of a matrix. Mask can be used
    * to exclude some pixel from mean value */
  void meanMatrix(double matrix[20][22], 
              double *mean, double *var, int *n, int mask[20][22]);

  void meanMatrix(double matrix[20][22], 
              double *mean, double *var, int *n, int mask[20][22], int err[20][22]);

  /** Add two masks */
  void addMask(int res[20][22], int mask[20][22]);

  /** Add two matrixes */
  void addMatrix(double res[20][22], double matrix[20][22]);
  
  /** Copy matrix to res */
  void copyMatrix(double res[20][22], double matrix[20][22]);
  
    
  /** Merge two mask using a logical or */
  void orMask(int res[20][22], int mask[20][22]);

  /** Invert the values of the mask */
  void invertMask(int mask[20][22]); 

  /** Count the pixel in the mask */
  int nMask(int mask[20][22]);

  /** Copy the mask to res */
  void copyMask(int res[20][22], int mask[20][22]);

  /** Maximum of constant and mask values */
  void maxMask(int max, int mask[20][22]);

  /** Minimum of constant and mask values */
  void minMask(int min, int mask[20][22]);

  
  /** Automatically find pixel with high noise in the 
    * variance matrix 
    *
    * Algorithm:
    *    - Calculate statistics of pixel distribution
    *    - Take threshold proportional to N times the distribution
    *      variance. Consider a minumum threshold.
    *    - Repeat the algorithm taking only non noisy pixel into
    *      account.
    * 
    * Parameter of the alorithm:
    *    - Factor N of the variances [-]
    *    - Minimum threshold [ADC Counts]  
    */
  void findHighNoise1(FILE *fout, double matrix[20][22], int res[20][22], int err[20][22]);

  /** Automatically find pixel with high noise in the 
    * variance matrix 
    *
    * Algorithm:
    *    - Find all negative values and exclude from further analsysis
    *    - Run a 2x2 edge filters over the pixel matrix. 
    *    - Use a threshold on the difference matrix. 
    *    - Repeat this rotating the 2x2 filter to consider all orientations.
    *      and overlap the resulting pixel mask.
    *    - Calculate the distribution mean for low and high noise pixel
    *    - Use a threshold on the orignal distribution.
    *      The thresold is choosen to mean high noise value minus
    *      half of the difference image threshold.
    *      Add the results.
    * 
    * Parameter of the algorithm:
    *    - Threshold in the difference matrixes
    *
    */
  void findHighNoise2(FILE *fout, double matrix[20][22], int res[20][22], int err[20][22]);
  
  // ======= Function working with the local arrays ============
        
  /** Clear noisy pixel matrix */
  void clearNoisyPixel(); 
  
  /** Clear faulty pixel matrix */
  void clearFaultyPixel();

  /** Add a new noisy pixel mask */
  void  addNoisyPixel(int mask[20][22]);
  
  /** Analyse matrix and store result in the local arrays */
  void findHighNoise(FILE *fout, double matrix[20][22]);

  /** Mean of normal pixel. 
    * Faulty pixel are not considered */
  void meanOfNormalPixel(double matrix[20][22], 
              double *mean, double *var, int *n);
      
  /** Mean of noisy pixel 
    * Faulty pixel are not considered */
  void meanOfNoisyPixel(double matrix[20][22], 
              double *mean, double *var, int *n);

  /** Number of noisy pixel */
  int nNoisyPixel();      
  
  /** Display the list of noisy pixel in the last analysis */
  void displayNoisyPixelList(FILE *fout);       

                  
  // ========== Local arrays =======
      
  /** Matrix of noisy pixel. Every time the statistical evaluation 
    * finds pixel with high noise they were marked in the noisy pixel matrix
    * and excluded from  further calculations */
  int noisyPixel[20][22];

  /** Last noisy pixel mask calculated by findNoisyPixel() */
  int noisyPixelLast[20][22];
  
  /** Matrix of faulty pixel. Every time the statistical evaluation 
    * finds pixel with neg variances they were marked in the faulty pixel matrix
    * and excluded from  further calculations */
  int faultyPixel[20][22];

  /** Count the number of analysis runs per telescope */
  int noisyRuns;


  /** 
    * Tolerated variance changes proportional to 
    * variance distribution over the telescope   */
  double noisyNVariances;

  /** Tolerated varinance deviation. 
    */
  int noisyMinVariance;

  /** Difference between neiboring  pixel relativ to the distribution mean */
  double noisyNeighborDiff;
 
  /** Absolut threshold used to find noisy pixel relativ to the distribution mean */
  double noisyAbsThresh; 

  /** Find extreme low noise pixel?! These pixel will be added to the
    * error pixel */
  double noisyLowThresh;  

  
  /** Debug flag */
  int debug;
  
};

#endif
