# Makefile.am
#

EXTRA_DIST = README

if USE_MYSQL
MYSQL_TARGETS=libkitcube-devices-mysql.a
endif


lib_LIBRARIES = libkitcube-devices.a  $(MYSQL_TARGETS)

libkitcube_devices_a_SOURCES = daqdevice.h daqdevice.cpp createdevice.h createdevice.cpp \
	daqbinarydevice.h daqbinarydevice.cpp daqasciidevice.h daqasciidevice.cpp \
	simrandom.h simrandom.cpp mast.h mast.cpp ceilometer.h ceilometer.cpp \
	norbert.h norbert.cpp jwd.h jwd.cpp parsivel.h parsivel.cpp \
	mrr.h mrr.cpp regenwippe.h regenwippe.cpp wolkenkamera.h wolkenkamera.cpp \
	scintillometer.cpp scintillometer.h 3m.cpp 3m.h \
	windtracer.cpp windtracer.h \
	gps.cpp gps.h sisomop.cpp sisomop.h \
	sodar.cpp sodar.h \
	radiosonde.cpp radiosonde.h \
	windcube.cpp windcube.h \
	hatpro.cpp hatpro.h \
	syslog.h syslog.cpp lara.h lara.cpp \
	orcaprocess.h orcaprocess.cpp \
	campbelltoa5.h campbelltoa5.cpp campbelltob1.h campbelltob1.cpp \
	hatpro.h hatpro.cpp \
	endian-osx.h 


libkitcube_devices_a_CXXFLAGS =

#libkitcube_devices_a_LDFLAGS = -lakutil -liconv

if USE_MYSQL
libkitcube_devices_mysql_a_SOURCES = $(libkitcube_devices_a_SOURCES)

libkitcube_devices_mysql_a_CXXFLAGS = -DUSE_MYSQL

#libkitcube_devices_a_LDFLAGS = -lakutil -liconv
endif


bin_PROGRAMS = test-device
test_device_SOURCES = test-device.cpp


# set the include path found by configure
INCLUDES= $(all_includes)


# the library search path.
test_device_LDFLAGS = $(all_libraries) -L./ -L../akutil  -lkitcube-devices -lakutil $(ICONV_LIBS) $(PYTHON_LIBS) -lnetcdf -lnetcdf_c++
