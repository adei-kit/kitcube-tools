# Makfile.am
#
#

EXTRA_DIST = kitcube.ini


if USE_MYSQL
MYSQL_TARGETS = kitcube-reader-mysql kitcube-monitor
endif


bin_PROGRAMS = kitcube-reader $(MYSQL_TARGETS) 
kitcube_reader_SOURCES = kitcube-reader.cpp \
			reader.h reader.cpp

# set the include path found by configure
INCLUDES= $(all_includes)


# the library search path.
kitcube_reader_LDFLAGS = $(all_libraries)  -L../kitcube-devices -L../akutil \
			-lkitcube-devices  -lakutil $(ICONV_LIBS) $(PYTHON_LIBS) -lnetcdf -lnetcdf_c++

			

if USE_MYSQL
kitcube_reader_mysql_SOURCES = $(kitcube_reader_SOURCES)

kitcube_reader_mysql_CXXFLAGS = -DUSE_MYSQL

kitcube_reader_mysql_LDFLAGS = $(all_libraries) $(MYSQL_LIBS) $(PYTHON_LIBS) -L../kitcube-devices -L../akutil \
				-lkitcube-devices-mysql  -lakutil $(ICONV_LIBS) -lnetcdf -lnetcdf_c++

kitcube_monitor_SOURCES = kitcube-monitor.cpp

kitcube_monitor_CXXFLAGS = -DUSE_MYSQL

kitcube_monitor_LDFLAGS = $(all_libraries) $(MYSQL_LIBS) $(PYTHON_LIBS) -L../kitcube-devices -L../akutil \
				-lkitcube-devices-mysql -lakutil

endif


# Make copies of the reader for startup scriptes
LINKS = kcr-mast kcr-3M kcr-cm kcr-cc kcr-gps kcr-rs kcr-hatpro kcr-sim kcr-wora
all: $(LINKS)

$(LINKS): kitcube-reader-mysql
	rm -f $@ 
	ln $< $@


 

