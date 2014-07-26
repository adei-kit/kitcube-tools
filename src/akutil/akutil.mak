# Microsoft Developer Studio Generated NMAKE File, Based on akutil.dsp
!IF "$(CFG)" == ""
CFG=akutil - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. akutil - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "akutil - Win32 Release" && "$(CFG)" != "akutil - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "akutil.mak" CFG="akutil - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "akutil - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "akutil - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "akutil - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\akutil.lib"


CLEAN :
	-@erase "$(INTDIR)\akinifile.obj"
	-@erase "$(INTDIR)\aksingleton.obj"
	-@erase "$(INTDIR)\aksingletoncleaner.obj"
	-@erase "$(INTDIR)\aktimelib.obj"
	-@erase "$(INTDIR)\analyseNoise.obj"
	-@erase "$(INTDIR)\analysePulse.obj"
	-@erase "$(INTDIR)\inifile.obj"
	-@erase "$(INTDIR)\procDuration.obj"
	-@erase "$(INTDIR)\semaphore.obj"
	-@erase "$(INTDIR)\simpleserver.obj"
	-@erase "$(INTDIR)\simplesocket.obj"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\akutil.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\akutil.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\akutil.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\akutil.lib" 
LIB32_OBJS= \
	"$(INTDIR)\akinifile.obj" \
	"$(INTDIR)\aksingleton.obj" \
	"$(INTDIR)\aksingletoncleaner.obj" \
	"$(INTDIR)\aktimelib.obj" \
	"$(INTDIR)\analyseNoise.obj" \
	"$(INTDIR)\analysePulse.obj" \
	"$(INTDIR)\inifile.obj" \
	"$(INTDIR)\procDuration.obj" \
	"$(INTDIR)\semaphore.obj" \
	"$(INTDIR)\simpleserver.obj" \
	"$(INTDIR)\simplesocket.obj" \
	"$(INTDIR)\statistics.obj"

"$(OUTDIR)\akutil.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\akutil.lib" "$(OUTDIR)\akutil.bsc"


CLEAN :
	-@erase "$(INTDIR)\akinifile.obj"
	-@erase "$(INTDIR)\akinifile.sbr"
	-@erase "$(INTDIR)\aksingleton.obj"
	-@erase "$(INTDIR)\aksingleton.sbr"
	-@erase "$(INTDIR)\aksingletoncleaner.obj"
	-@erase "$(INTDIR)\aksingletoncleaner.sbr"
	-@erase "$(INTDIR)\aktimelib.obj"
	-@erase "$(INTDIR)\aktimelib.sbr"
	-@erase "$(INTDIR)\analyseNoise.obj"
	-@erase "$(INTDIR)\analyseNoise.sbr"
	-@erase "$(INTDIR)\analysePulse.obj"
	-@erase "$(INTDIR)\analysePulse.sbr"
	-@erase "$(INTDIR)\inifile.obj"
	-@erase "$(INTDIR)\inifile.sbr"
	-@erase "$(INTDIR)\procDuration.obj"
	-@erase "$(INTDIR)\procDuration.sbr"
	-@erase "$(INTDIR)\semaphore.obj"
	-@erase "$(INTDIR)\semaphore.sbr"
	-@erase "$(INTDIR)\simpleserver.obj"
	-@erase "$(INTDIR)\simpleserver.sbr"
	-@erase "$(INTDIR)\simplesocket.obj"
	-@erase "$(INTDIR)\simplesocket.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\akutil.bsc"
	-@erase "$(OUTDIR)\akutil.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\akutil.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\akutil.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\akinifile.sbr" \
	"$(INTDIR)\aksingleton.sbr" \
	"$(INTDIR)\aksingletoncleaner.sbr" \
	"$(INTDIR)\aktimelib.sbr" \
	"$(INTDIR)\analyseNoise.sbr" \
	"$(INTDIR)\analysePulse.sbr" \
	"$(INTDIR)\inifile.sbr" \
	"$(INTDIR)\procDuration.sbr" \
	"$(INTDIR)\semaphore.sbr" \
	"$(INTDIR)\simpleserver.sbr" \
	"$(INTDIR)\simplesocket.sbr" \
	"$(INTDIR)\statistics.sbr"

"$(OUTDIR)\akutil.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=ws2_32.lib /nologo /out:"$(OUTDIR)\akutil.lib" 
LIB32_OBJS= \
	"$(INTDIR)\akinifile.obj" \
	"$(INTDIR)\aksingleton.obj" \
	"$(INTDIR)\aksingletoncleaner.obj" \
	"$(INTDIR)\aktimelib.obj" \
	"$(INTDIR)\analyseNoise.obj" \
	"$(INTDIR)\analysePulse.obj" \
	"$(INTDIR)\inifile.obj" \
	"$(INTDIR)\procDuration.obj" \
	"$(INTDIR)\semaphore.obj" \
	"$(INTDIR)\simpleserver.obj" \
	"$(INTDIR)\simplesocket.obj" \
	"$(INTDIR)\statistics.obj"

"$(OUTDIR)\akutil.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("akutil.dep")
!INCLUDE "akutil.dep"
!ELSE 
!MESSAGE Warning: cannot find "akutil.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "akutil - Win32 Release" || "$(CFG)" == "akutil - Win32 Debug"
SOURCE=.\akinifile.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\akinifile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\akinifile.obj"	"$(INTDIR)\akinifile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\aksingleton.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\aksingleton.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\aksingleton.obj"	"$(INTDIR)\aksingleton.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\aksingletoncleaner.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\aksingletoncleaner.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\aksingletoncleaner.obj"	"$(INTDIR)\aksingletoncleaner.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\aktimelib.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\aktimelib.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\aktimelib.obj"	"$(INTDIR)\aktimelib.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\analyseNoise.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\analyseNoise.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\analyseNoise.obj"	"$(INTDIR)\analyseNoise.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\analysePulse.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\analysePulse.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\analysePulse.obj"	"$(INTDIR)\analysePulse.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\inifile.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\inifile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\inifile.obj"	"$(INTDIR)\inifile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\procDuration.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\procDuration.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\procDuration.obj"	"$(INTDIR)\procDuration.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\semaphore.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\semaphore.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\semaphore.obj"	"$(INTDIR)\semaphore.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\simpleserver.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\simpleserver.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\simpleserver.obj"	"$(INTDIR)\simpleserver.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\simplesocket.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\simplesocket.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\simplesocket.obj"	"$(INTDIR)\simplesocket.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\statistics.cpp

!IF  "$(CFG)" == "akutil - Win32 Release"


"$(INTDIR)\statistics.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "akutil - Win32 Debug"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

