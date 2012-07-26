INCLUDES = $(all_includes)
libakutil_a_CXXFLAGS = -fPIC
METASOURCES = AUTO
lib_LIBRARIES = libakutil.a
libakutil_a_SOURCES = akinifile.cpp akinifile.h aksingletoncleaner.cpp	aksingletoncleaner.h aksingleton.cpp aksingleton.h aksingletontest.cpp	aksingletontest.h aktimelib.cpp aktimelib.h aktimelib.ini akutil.h	analysematrix.cpp analysematrix.h analyseNoise.cpp analyseNoise.h	analysePulse.cpp analysePulse.h baseserver.cpp baseserver.h inifile.cpp	inifile.h keyboard.h procDuration.cpp simpleserver.h simplesocket.cpp	simplesocket.h socketserver.cpp socketserver.h statistics.cpp	statistics.h unsignedint64.cpp unsignedint64.h semaphore.cpp semaphore.h sharedMemory.cpp sharedMemory.h simpleserver.cpp wgcommand.cpp wgcommand.h


# Modify the name of the package install dir first
#pkgincludedir = $(prefix)/@PACKAGE@-@VERSION@/include/akutil
pkgincludedir = $(prefix)/include/akutil
pkginclude_HEADERS = *.h 

# Configuration files
#EXTRA_DIST = aktimelib.ini        
sysconf_DATA = aktimelib.ini
