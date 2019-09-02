all: fpga fpga_control
.PHONY : all
OBJS1 = fpga.o
OBJS2 = fpga_control.o

DEPPKGS = gio-unix-2.0 glib-2.0
CC      ?=  $(CROSS_COMPILE)gcc
INCLUDES += $(shell pkg-config --cflags $(DEPPKGS))

PACKAGE_DEPS=libsystemd
LDLIBS+=$(shell pkg-config --libs $(PACKAGE_DEPS))
ALL_CFLAGS+=$(shell pkg-config --cflags $(PACKAGE_DEPS)) -fPIC -Werror $(CFLAGS)

fpga:$(OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) -lrt -o fpga $(OBJS1) $(LDLIBS)

fpga_control:$(OBJS2)
	$(CC) $(CFLAGS) $(LDFLAGS) -o fpga_control $(OBJS2) $(LDLIBS)

$(OBJS1):fpga.c
	$(CC) -c $(ALL_CFLAGS) -lrt -o $@ $<
$(OBJS2):fpga_control.c
	$(CC) -c $(ALL_CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -rf *.o fpga fpga_control
