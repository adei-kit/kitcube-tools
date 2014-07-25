/***************************************************************************
    akinifile.h  -  description

    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/


#ifndef AKINIFILE_H
#define AKINIFILE_H


#include <akutil/inifile.h>

/**  Extension of the inifile reader
  *
  *  New features:
  * @li Logging
  * @li Default values
  *
  * Changes:
  *  - 8.3.01 ak: Internal preprocessor define pragmas SUCCESS and FAIL
  *       changed to kSUCCESS and kFAIL
  *
  * @ingroup akutil_group
  */
class akInifile : public Inifile  {
public: 
  akInifile(const char * filename, FILE * logfile = 0,
     const char *secondDir = 0, const char *thirdDir = 0);

  ~akInifile();

  // --- Extended interface ------

  result SpecifyGroup(const char *string);


  //##ModelId=399121F9011C
  int GetFirstValue(const char *entry, int defValue, result *error);

  int GetNextValue(int defValue, result *error);

  //##ModelId=399121F90118
  char * GetFirstString(const char *entry, const char *defString, result *error);

  char * GetNextString(const char *defString, result *error);
    
  /** Read the full string in a line. */
  char * GetFullString(const char *entry, const char *defString, result *error);

  char * GetFullString(const char *entry);
  
  double GetFirstValue(const char *entry, double defValue, result *error);

  double GetNextValue(double defValue, result *error);


  // --- Basic Interface ----
  double GetFirstDouble(const char *entry, result *error);

  double GetNextDouble(result *error);

    /** Set the debug level - 0 means no debug output */
    void setDebugLevel(int value);

    
protected:
  /** Try to open another file */
  int open(char *filename);

  /** Concat filename and directory to 
    * the filepath. The directory entry can also be 
    * a environment variable (e.g. $HOME)
    */
  int concatFilename(char *filepath, char *filename, char *dir);

private:
  //##ModelId=399121F90117
  FILE * logfile;

  /** Flag set if logging output is required */
  bool writeLog;
    
    /** Debug level */
    int debug;

};

#endif
