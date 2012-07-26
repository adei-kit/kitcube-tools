/***************************************************************************
    akinifile.cpp  -  description

    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Andreas Kopmann
    email                : kopmann@hpe.fzk.de
 ***************************************************************************/

#include <cstdlib>
#include <cstring>

#include "akinifile.h"


akInifile::akInifile(const char *filename, FILE *logfile,
     const char *secondDir, const char *thirdDir)
    :Inifile((char *) filename){
  int err;
  char inipath[255];

  this->logfile = logfile;

  // Chack whether loging is required
  if (logfile == 0) writeLog = false;
  else writeLog = true;

  if (Status()==kSUCCESS)  return;

  if (filename == 0) {
    if (writeLog) fprintf(logfile, "Inifile: Specify name of the inifile\n");
    return;
  }

  if (writeLog) fprintf(logfile,"Inifile: No ini file %s found \n",filename);

  // Try to find inifile in the second dir
  if (secondDir > 0){
    err = concatFilename(inipath, (char *) filename, (char *)secondDir);
  if (err==0){
      open(inipath);
      if (Status()==kSUCCESS)  return;
      if (writeLog) fprintf(logfile,"Inifile: No ini file %s found \n",inipath);

  }else
    if (writeLog) fprintf(logfile,"Inifile: Environment variable %s does not exist\n",secondDir);
  
  }


  // Try to find inifile in the third dir
  if (thirdDir > 0) {
    err = concatFilename(inipath, (char *) filename, (char *) thirdDir);
  if (err == 0){
      open(inipath);
      if (Status()==kSUCCESS)  return;

      if (writeLog) fprintf(logfile,"Inifile: No ini file %s found \n",inipath);
  } else {
    if (writeLog) fprintf(logfile,"Inifile: Environment variable %s does not exist\n",thirdDir);
  }
  }


}



int akInifile::concatFilename(char *filepath, 
                 char *filename, char *dir){
  char *rest, envVar[255], *envValue;


  if (dir > 0){
  // Get the directory
  // Replace environment variables (leading $)
  if (dir[0]=='$') {  
      strcpy(envVar,&dir[1]);

    // Is there a sub directory added?
      rest = strchr(envVar,'/');
    if (rest > 0) {
    *rest = 0;
    rest = rest +1;

    envValue = getenv(envVar);
    if (envValue == 0) return (1); // Not defined

    sprintf(filepath,"%s/%s",envValue,rest);
        //strcpy(filepath,envValue);
    //strcat(filepath,"/");
        //strcat(filepath,rest);

    } else {
    envValue = getenv(envVar);
    if (envValue == 0) return (1); // Not defined
        strcpy(filepath,envValue);
    }

  } else
      strcpy(filepath,dir);

  // Add slash, if required
  if (filepath[strlen(filepath)-1] != '/')
      strcat(filepath,"/");

  // Add filename
    strcat(filepath,filename);

  } else {
    // No directory ?!
    strcpy(filepath,filename);
  }


  return(0);
}


akInifile::~akInifile(){

  // Inifile is closed in the class inifile ???

}


int akInifile::open(char *filename){
  if (fStatus == kSUCCESS) {  // 2.8.00 ak
    fclose ( fInifile );
  }


  fInifile = fopen (filename, "r");

  if ( fInifile == NULL )
   {
    char buf[1024];
    sprintf (buf, "Inifile \"%s\" could not be opened", filename );
    //perror (buf); // ak 22.10.01
    fStatus = kFAIL;

    return(kFAIL);
  }

  this->fFilename = new char[strlen(filename) + 1];
  if (this->fFilename == NULL)
   {
    perror ("not enough memory for filename");
    return(kFAIL);
  }
  strcpy (this->fFilename, filename);
  fStatus   = kSUCCESS;

  return(kSUCCESS);
}


Inifile::result akInifile::SpecifyGroup(const char *string){
	Inifile::result error;
	
	error = Inifile::SpecifyGroup((char *) string);
	if (error == kFAIL) {
		if (writeLog) 
			fprintf(logfile,"Inifile: Group %s not found\n", string);
	}
	return( error );
}


int akInifile::GetFirstValue(const char *entry, int defValue, result *error) {
  int value=0;

  if (Status() == kFAIL)
    *error = kFAIL;
  else
    value = Inifile::GetFirstValue((char *) entry,error);

  if (*error == kFAIL) {
    if (writeLog) fprintf(logfile,"Inifile: No value for %s found, using default=%d\n",
                    entry,defValue);
    value = defValue;
  }

  return(value);
}


int akInifile::GetNextValue(int defValue, result *error) {
  int value=0;

  if (Status() == kFAIL)
    *error = kFAIL;
  else
    value = Inifile::GetNextValue(error);

  if (*error == kFAIL) {
    //if (writeLog) fprintf(logfile,"Inifile: No value further value found for this entry, using default=%d\n",
    //                              defValue);
    value = defValue;
  }

  return(value);
}


char * akInifile::GetFirstString(const char *entry, const char * defString, result *error) {
  char * ptr;

  if (Status() == kFAIL)
    ptr = NULL;
  else
    ptr = Inifile::GetFirstString((char *) entry);

  if (ptr == NULL) {
    if (writeLog) fprintf(logfile,"Inifile: No value for %s found, using default=%s\n",
                  entry,defString);
    *error= kFAIL;
    ptr = (char *) defString;
  } else
    *error = kSUCCESS;

  return(ptr);

}


char * akInifile::GetNextString(const char *defString, result *error) {
  char * ptr;

  if (Status() == kFAIL)
    ptr = NULL;
  else
    ptr = Inifile::GetNextString();

  if (ptr == NULL) {
    if (writeLog) fprintf(logfile,"Inifile: No further value found, using default=%s\n",
                  defString);
    *error= kFAIL;
    ptr = (char *) defString;
  } else
    *error = kSUCCESS;
    
  return(ptr);
}


double akInifile::GetFirstValue(const char *entry, double defValue, result *error) {
  double value=0;

  if (Status() == kFAIL)
    *error = kFAIL;
  else
    value = GetFirstDouble(entry,error);

  if (*error == kFAIL) {
    if (writeLog) fprintf(logfile,"Inifile: No value for %s found, using default=%g\n",
                    entry,defValue);
    value = defValue;
  }

  return(value);
}

double akInifile::GetNextValue(double defValue, result *error) {
  double value=0;

  if (Status() == kFAIL)
    *error = kFAIL;
  else
    value = GetNextDouble(error);

  if (*error == kFAIL) {
    if (writeLog) fprintf(logfile,"Inifile: No further value found, using default=%g\n",
                    defValue);
    value = defValue;
  }

  return(value);
}



double akInifile::GetFirstDouble (const char* _entry, result* error)
 {
  if (fStatus == kFAIL) {  // 2.8.00 ak
   *error = kFAIL;
   return 0;
  }

  if (error != NULL) *error = kFAIL;
  fPtr = Inifile::GetFirstString ((char *) _entry);
  if (fPtr == NULL) {
    if ( error ) *error = kFAIL;
    return 0;
  }

  double ret;
  ret = atof ( fPtr );
  if (error != NULL)  *error = kSUCCESS;

  return ret;
}


double akInifile::GetNextDouble (result* error)
 {
  if (fStatus == kFAIL) {  // 2.8.00 ak
   *error = kFAIL;
   return 0;
  }

  if (error != NULL) *error = kFAIL;
  fPtr = Inifile::GetNextString ();
  if (fPtr == NULL) {
    if ( error ) *error = kFAIL;
    return 0;
  }

  double ret;
  ret = atof ( fPtr );
  if (error != NULL)  *error = kSUCCESS;

  return ret;
}


/*
int akInifile::Addline(char *line){

  // Read the whole file and cound the number of lines
  // Allocate a buffer to store the wohle file
  // Open the file again and store in buffer
  // Find the position to insert the new line
  // Open destination file (can be same localtion, always home!)
  // Write first part
  // Write new line
  // Write second part
  // Close and open the new file again for reading

}
*/


