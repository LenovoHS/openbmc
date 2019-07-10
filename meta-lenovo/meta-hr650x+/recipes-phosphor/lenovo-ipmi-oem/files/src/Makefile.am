ACLOCAL_AMFLAGS = -I m4
AM_DEFAULT_SOURCE_EXT = .cpp
nobase_include_HEADERS = 
	

liboemcmdsdir = ${libdir}/ipmid-providers
liboemcmds_LTLIBRARIES = liboemcmds.la
liboemcmds_la_SOURCES = oemcommands.cpp
    
liboemcmds_la_LDFLAGS = \
	$(LIBIPMID_LIBS)
	-version-info 0:0:0 -shared

liboemcmds_la_CXXFLAGS = -flto \
	$(LIBIPMID_CFLAGS)