/***************************************************************************
    fdhwlib.h  -  description (Automatically created from fdhwlib.h.in)

    begin                : Mon Jan 13 2003
    copyright            : (C) 2003 by A Kopmann
    email                : kopmann@ipe.fzk.de
    status               :
    test                 :
    history              :
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H



#ifndef ModuleVersionCode
#define ModuleVersionCode(v,p,s) (((v)<<16)+((p)<<8)+(s))
#endif // ModuleVersionCode 


#ifndef FDHWLIB_VER
#define FDHWLIB_VER ModuleVersionCode(2, 0, 22)
//#define FDHWLIB_VER "2.0.22"

#define FDHWLIB_RPM_VERSION "2.0.22"
#define FDHWLIB_RPM_RELEASE "80"
#endif // FDHWLIB_VER



#define FE_INIPATH1 "$HOME"
#define FE_INIPATH2 "/opt/auger/etc"
#define FE_INIPATH3 ""

// Defines the number of telescopes
#define FD_TELESCOPES 27

// Defines the number of eye stations
#define FD_STATIONS 5

// TODO: Number of stations per eye?


/** Convert between network and host byte order.
  * If this option is selected the data is always
  * transfered in network byte order (big endian). If the
  * option is disabled the local host byte order is used.
  * In this case the user is responsible to ensure that
  * client and server always use the same byte order
  * (e.b Windows + Linux = little endian, Mac = big endian).
  *
  * Warning: Using this switch will cause inkompatibilities if
  * client and server do not use the same settings.
  */
//#define USE_BYTEORDER

/** Dynamically find out the byte order of the connected
  * client and send always the suitable format
  */
#define USE_DYN_BYTEORDER



/** Use MYSQL to realize persistant data storage 
  * The feature is detected by the configure script. Use
  * the compiler flag -D USE_MYSQL to override the default
  * setting of the package.
  */ 
#define USE_MYSQL



/** @mainpage Hardware Access Library
  *
  * Contents:
  * - @ref fdhwlib_intro
  * - @ref fdhwlib_usage
  * - @ref fdhwlib_dirs
  * - @ref fdhwlib_build
  * - @ref fdhwlib_version
  *
  *
  * @section fdhwlib_intro Introduction
  * This document contains a description of the hardware access
  * library for the Pierre Auger Fluoresence Detector Electronic (%FE)
  * other experiments.
  * The library provides (nearly) the same functionality for Linux and Windows
  * and Apple operating system.
  *
  * The library is divided in three layers:
  * - @ref api This layer provides the interface to the
  *   for the application programms. The function of this layer are rather independent
  *   from the realization of the hardware.
  * - The register model layer implements the address model of the
  *   hardware. Changes
  *   in the hardware structure will also cause changes in this layer.
  *   This means also that for the different experiments own
  *   register models exist.
  *   Implemented are:
  *     - @ref basehw
  *     - @ref hw
  *     - @ref katrinhw
  *
  * - @ref pbus This layer provides the interface to the hardware via the
  *   Pbus protocol (simple VME bus type).
  *
  * Included in the package are some more general utility
  * functions:
  *  - @ref akutil_group
  *     - @ref timesync
  *  - @ref simpleshell
  *  - @ref calib_group
  *  - @ref ts_group
  *     - @ref bgformat
  *     - @ref fddaswin
  *  - @ref gps
  *     - @ref gps_remote
  *     - @ref gps_conf
  *
  *
  * Dependancies of the libraries in the package
  * 
@verbatim
   akutil
  
   akshell -- akutil
  
   
   calib -- Hw -- Pbus\<AccessType\> -- akutil
  
   am_testutil -- Hw -- Pbus\<AccessType\> -- akutil
  
   FEdata --Hw --Pbus\<AccessType\> -- akutil
    
  
   FE -- Hw -- Pbus\<AccessType\> -- akutil
  
   Hw -- Pbus\<AccessType\> -- akutil
   
   Pbus\<AccessType\> -- akutil 
    
@endverbatim
  * 
  * @section fdhwlib_usage Usage
  * Projects using the hardware access library need to add the
  * directory fdhwlib/fdhwlib to their include path.
  *
  * Example (Auger API):
@verbatim
#include <FE/FE.h>

main(){

  FE fe;
  FE fe.config.init();
  ...

}
@endverbatim
  * Linking the project it is necessary to add the hardware access
  * library and Pbus library.
  *
  * Example (Katrin Register Model):
@verbatim
#include <katrinhw/kasubrack.h>

main(){

   KaSubrack *s;
   s = new KaSubrack(katrin.ini);
   ...
}  
@endverbatim
  *
  *
  * @section fdhwlib_dirs Directory Structure
  * The directory fdhwlib contains project file for Kdevelop/Linux and
  * MS Visual C++/MS Windows to create the approritate version of the
  * library. The sources for the different layers can be found in the
  * directories fdhwlib/%FE, fdhwlib/Hw and
  * fdhwlib/%Pbus. Some more general utilities will be found
  * in fdhwlib/akutil. The definition of the KATRIN hardware
  * model is based on the same generic classes used in the Auger
  * register model. All KATRIN specific classes can be found in fdhwlib/katrinhw
  *
  * The documentation generated can be found in the directory
  * fdhwlib-api. It is available in html and latex format.
  *
  *
  * @section fdhwlib_build Compilation of the libraries
  *
  * @subsection buildlinux Linux
  * To compile and install the library use the commands
  * @verbatim
./configure
./makeAll
./makeAll daq
./make doc
./makeInstall
./makeInstall daq
@endverbatim
  * Without the argument "daq" no fd-das or ROOT support is required.
  * For most applications it may be sufficient to omit creating
  * the fd-daq dependant part of the package.
  *
  * The package requires some external packages. These are
  * Root, fd-das and the microEnable drivers. Use the
  * environment variable CPLUS_INCLUDE_PATH to specify
  * the path to the header files. E.g. use
  * @verbatim
CPLUS_INCLUDE_PATH=/usr/software/fdhwlib/include:
       /home/kopmann/FD-das/include:/home/kopmann/FD-das/mirror/include:
	   /cern/root/include:/usr/src/menable/include:/usr/lib/qwt/include:
@endverbatim
  *
  * To install the package it is necessary to have write
  * access to the install directory /usr/software.
  *
  *
  * @subsection buildwindows Windows
  * For all libraries a MSVC project exists. Use these to
  * compile a separate library.
  *
  * To generate all
  * libraries use the script fdhwlib/makeAll.bat. The script
  * accepts two additional arguments to generate the menable and
  * daq dependant parts of the library. For these libraies
  * it is required to specify the location of the required
  * packages (menable + daq + root) in the INCLUDE environment
  * variable. The path of the root libraries has to be added to
  * to the LIB environment variable.
  *
  * @verbatim
makeAll                  Generate the standard libraries
makeAll menable          Generate menable depandant part
makeAll daq              Generate daq dependant part
@endverbatim
  *
  * To use the libraries add the directory fdhwlib/fdhwlib to the include
  * path and all required fdhwlib/fdhwlib/\<subdir\>/debug directories
  * (e.g. fdhwlib/fdhwlib/Hw/debug) to the library path.
  *
  *
  * @subsection builddoc Documentation
  * The documentation can be automatically generated from
  * the header files using doxygen (or other programs handling
  * javadoc type comments).
  *
  * Using "make doc" will generate documentation for all packages.
  *
  *
  * @subsection buildinternals Build system internals
  * The packages uses the standard build system of the
  * kdevelp IDE and MSVC where this is possible and sufficient.
  * However in some case it turned out to be adequate to
  * provide additional scripts. The scripts are named on both
  * platforms as "makeAll" and "makeInstall". The scripts will
  * accept additional arguments ("daq", "meanble")
  * to create microenable or
  * DAQ dependant parts of the package.
  *
  * Central with kdevelop are the files Makefile.am in every
  * directory of the projects. The configure step will generate
  * from this file the required Makefiles. Some manual work has
  * to be done in the scripts to activate all make options
  * and to install some parts that are not installed
  * automatically.
  *
  * In MSVC the information of the build process is hidden in
  * the DSP project files. It is possble to generate makefiles
  * (extension MAK) for Microsofts nmake program. The redundand
  * MAK files allow to build the whole project automatically from
  * the command line and to control the make process by script files.
  *
  *
  *
  * @section fdhwlib_version Remark on version numbers
  *
  * There are several version numbers used in the package
  * fdhwlib. First comes the overall version of the package
  * It can be found in the files fdhwlib.h config.h (only in the
  * development version, not included in the tar-ball!) somewhere
  * in the kdevelop project file and in the doxygen configuration.
  * The project file will automatically update config.h but not
  * the other places. This has to be done manually. All files
  * should show the same version number.
  *
  * There are other version numbers that are hardware related.
  * The purpose of these numbers is to keep the library compatible
  * with older versions of the hardware design. The version numbers
  * are used in this case for conditional comiplation.
  * (PBUS_VER, FLT_VER and FLT_VER).
  *
  *
  */


/** @page changes Changes
  *
  * @verbinclude ChangeLog
  */


