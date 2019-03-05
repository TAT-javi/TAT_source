
DIR	= $(shell pwd)
CC	= gcc
INC	= $(DIR) $(DIR)/unix
CFLAGS	= -Wall -O2 -g 

AR	= ar
ARFLAGS	= -rus

SYS	= libfli-sys.o
DEBUG	= libfli-debug.o
MEM	= libfli-mem.o
USB_IO	= libfli-usb.o libfli-usb-sys.o
PPORT_IO	= libfli-parport.o
SERIAL_IO	= libfli-serial.o
IO	= $(USB_IO) $(PPORT_IO) $(SERIAL_IO)
CAM	= libfli-camera.o libfli-camera-parport.o libfli-camera-usb.o
FILT	= libfli-filter-focuser.o

ALLOBJ	= $(SYS) $(DEBUG) $(MEM) $(IO) $(CAM) $(FILT)

libfli.a: libfli.o $(ALLOBJ)
	$(AR) $(ARFLAGS) $@ $^


.PHONY: clean install
clean:
	@rm -f $(ALLOBJ) libfli.o libfli.a; echo done
install:
	@cp libfli.a ..; echo done