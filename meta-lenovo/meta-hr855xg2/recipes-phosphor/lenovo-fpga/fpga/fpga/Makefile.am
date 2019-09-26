ACLOCAL_AMFLAGS = -I m4
AM_DEFAULT_SOURCE_EXT = .c
nobase_include_HEADERS = ../include/fpga.h

libfpgadir = ${libdir}/
libfpga_LTLIBRARIES = libfpga.la
libfpga_la_SOURCES = \
	fpga_control.c

pkgconfiglibdir = ${libdir}/pkgconfig
pkgconfiglib_DATA = fpga.pc

libfpga_la_LDFLAGS = \
	$(SDBUSPLUS_LIBS) \
	-lstdc++fs \
	-shared -fPIC -O2
	
libfpga_la_CXXFLAGS = \
	$(SDBUSPLUS_CFLAGS) \
	-flto

sbin_PROGRAMS	 = fpga_control
fpga_control_SOURCES = fpga_control.c
fpga_control_CFLAGS = \
	-flto \
	-Wno-psabi \
	$(SYSTEMD_CFLAGS)
fpga_control_LDFLAGS = \
	$(SYSTEMD_LIBS)
