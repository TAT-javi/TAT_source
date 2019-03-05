#!/bin/sh
#	   g++ -c test_alta.cpp -I/opt/apogee/include
#	   g++ -I/opt/apogee/include  test_alta.o -IFpgaRegs -o test_altau \
#                        ApnCamera.o ApnCamera_Linux.o ApnCamera_USB.o  \
#                        ApogeeUsbLinux.o ApnCamData*.o ApnCamTable.o  \
#                        -L/opt/apogee/lib -ltcl8.3 -lccd -lfitsTcl /opt/apogee/lib/libusb.so
make apogee_USB.so
g++ -g test_alta.cpp -IFpgaRegs -o test  ApnCamera.o ApnCamera_Linux.o\
	ApnCamera_USB.o ApogeeUsbLinux.o ApnCamData*.o ApnCamTable.o \
	-L/opt/apogee/lib -ltcl8.3  -lfitsTcl /opt/apogee/lib/libusb.so \
	-I/opt/apogee/include 


