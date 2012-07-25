# Makefile.am
#

EXTRA_DIST = kitcube.ini


bin_PROGRAMS = kitcube-data
kitcube_data_SOURCES = kitcube-data.cpp \
	dataserver.h dataserver.cpp


# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
kitcube_data_LDFLAGS = $(all_libraries) $(MYSQL_LIBS) $(PYTHON_LIBS) -L../akutil -L../kitcube-devices \
			-lkitcube-devices -lakutil $(ICONV_LIBS) -lnetcdf -lnetcdf_c++


# Make copies of the reader for startup scriptes
LINKS = kcd-mast 
all: $(LINKS)

$(LINKS): kitcube-data
	rm -f $@ 
	ln $< $@



