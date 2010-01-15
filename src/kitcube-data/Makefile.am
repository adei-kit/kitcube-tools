# Makefile.am
#

EXTRA_DIST = M12_01.DAR.template M12_01.DAX.template kitcube.ini


bin_PROGRAMS = kitcube-data
kitcube_data_SOURCES = kitcube-data.cpp \
	dataserver.h dataserver.cpp 
	

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
kitcube_data_LDFLAGS = $(all_libraries) $(MYSQL_LIBS) -L../akutil -L../kitcube-devices -lkitcube-devices -lakutil $(ICONV_LIBS)

