ACLOCAL_AMFLAGS = -I m4
AM_DEFAULT_SOURCE_EXT = .cpp
nobase_include_HEADERS = 

sbin_PROGRAMS = \
	fpga-util

fpga_util_SOURCES = \
	argument.cpp \
	fpga-util.cpp

fpga_util_LDFLAGS = \
	$(FPGA_LIBS) \
	$(LIBADD_DLOPEN) \
	-lpthread

fpga_util_CXXFLAGS = -flto

