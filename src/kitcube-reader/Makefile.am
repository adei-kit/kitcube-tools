# Makfile.am 
#
#

EXTRA_DIST = kitcube.ini


if USE_MYSQL
MYSQL_TARGETS = kitcube-reader-mysql
endif


bin_PROGRAMS = kitcube-reader $(MYSQL_TARGETS) 
kitcube_reader_SOURCES = kitcube-reader.cpp \
	reader.h reader.cpp 

# set the include path found by configure
INCLUDES= $(all_includes)


# the library search path.
kitcube_reader_LDFLAGS = $(all_libraries)  -L../kitcube-devices -L../akutil  -lkitcube-devices  -lakutil $(ICONV_LIBS)


if USE_MYSQL
kitcube_reader_mysql_SOURCES = $(kitcube_reader_SOURCES)

kitcube_reader_mysql_CXXFLAGS = -DUSE_MYSQL

kitcube_reader_mysql_LDFLAGS = $(all_libraries) $(MYSQL_LIBS) -L../kitcube-devices -L../akutil  -lkitcube-devices-mysql  -lakutil $(ICONV_LIBS)
endif
 

