# Define the site 
# Do not change this line manually
SITE = TF

# Define the CCD daemon
# for Apogee AP6 => ccddaemon
#for Apogee U6 => ccddaemonu6
#for Apogee F6 => ccddaemonf6
# Do not change this line manually
CCD = ccddaemonf6

DSP = dspdaemon
PPC = ppcdaemon
PWR = pwrdaemon
CTL = ctldaemon
FLI = flidaemon
LST = lstdaemon

DAEMON = $(DSP) $(CCD) $(PPC) $(PWR) $(CTL) $(FLI) $(LST)

all: $(DAEMON) autocontrol emergencystop \
     cmdmenu star flat dark read_shm web update_dsp\
     checkinput use_astrometry check_weather send_msg_hq
     
#      parameter_shm

daemon: $(DAEMON)

# which compiler
CC = gcc
CPP = g++

# where to install
DAEMON_INSTDIR = /home/tat/daemon
APP_INSTDIR = /home/observer
CGI_INSTDIR= /var/www/cgi-bin
UDSP_INSTDIR= /home/tat/update_dsp

# where are include files kept
INCLUDE = .

# Options for development
#CFLAGS = -g -Wall -O3

# Options for release
MYSQL_FLAG = $(shell mysql_config --cflags)
C_FLAG = -O

# path var
FUNCDIR = func
BINDIR = bin
UTILDIR = util_bin
FLIDIR = libfli

# libraries
LIBMATH = -lm
LIBCOMMON = $(LIBMATH) -lncurses
LIBMYSQL = $(shell mysql_config --libs)
LIBFLI = ./$(FLIDIR)/libfli.a
LIBFITS = -lcfitsio
LIBTHREAD = -lpthread

# OBJECTS
COMOBJS = common_func.o tat_info.o

ADJUST = adjust_fov.o 

CCDSRC =  ccddaemonf6.o $(COMOBJS)
CCDBIN = $(BINDIR)/ccddaemonf6

DSPOBJS =  dspdaemon.o $(COMOBJS)
DSPBIN = $(BINDIR)/dspdaemon

PPCOBJS = ppcdaemon.o $(COMOBJS) par_func.o
PPCBIN = $(BINDIR)/ppcdaemon

PWROBJS = pwrdaemon.o $(COMOBJS)
PWRBIN = $(BINDIR)/pwrdaemon

CTLOBJS = ctldaemon.o ccd_func.o $(COMOBJS)
CTLBIN = $(BINDIR)/ctldaemon

FLIOBJS = flidaemon.o $(COMOBJS)
FLIBIN = $(BINDIR)/flidaemon

LSTOBJS = lstdaemon.o $(COMOBJS) weather_func.o
LSTBIN = $(BINDIR)/lstdaemon

AUTOOBJS = main_auto_observe.o $(COMOBJS) dsp_func.o ccd_func.o ppc_func.o\
	   auto_observe_func.o open_close_enc.o guard_function.o\
	   weather_func.o pwr_func.o get_date.o ctl_func.o fli_func.o\
	    check_input_file.o send_remote_func.o
AUTOBIN = $(BINDIR)/auto_observing

EMGOBJS =  main_emergency_stop.o $(COMOBJS) open_close_enc.o ppc_func.o\
	   dsp_func.o ccd_func.o pwr_func.o get_date.o
EMGBIN = $(BINDIR)/manu_stop_observation

MENUOBJS = main_cmd_menu.o $(COMOBJS) dsp_func.o ppc_func.o ccd_func.o\
	   pwr_func.o ctl_func.o posixtm.o star_track_menu.o \
	   get_date.o $(ADJUST) fli_func.o 
MENUBIN = $(BINDIR)/cmdMenu

STAROBJS = star_track.o $(COMOBJS) $(ADJUST) dsp_func.o get_date.o fli_func.o ppc_func.o
STARBIN = $(BINDIR)/star_track

FLATOBJS = auto_flat.o dsp_func.o ccd_func.o pwr_func.o ppc_func.o \
		 	get_date.o fli_func.o weather_func.o $(COMOBJS)
FLATBIN = $(BINDIR)/auto_flat

DARKOBJS = auto_dark.o ccd_func.o ppc_func.o $(COMOBJS) get_date.o
DARKBIN = $(BINDIR)/auto_dark

WEBOBJS = monitor_on_web.o $(COMOBJS) get_date.o
WEBBIN = $(UTILDIR)/webmonitor.cgi
WEBFILES = $(WEBBIN) local_shift.php total_shift.php

SHMOBJS= main_read_shm.o $(COMOBJS) get_date.o
SHMBIN= $(UTILDIR)/read_shm

UDSPOBJ= update_dsp.o
UDSPBIN=$(UTILDIR)/update_dsp
UDSPFILES= $(UDSPBIN) *.hex

CHECKOBJS = main_check_input_file.o check_input_file.o get_date.o $(COMOBJS)
CHECKBIN = $(BINDIR)/check_input_file

INSERTOBSOBJS = main_insert_obs.o check_input_file.o get_date.o $(COMOBJS)
INSERTOBSBIN = $(BINDIR)/insert_obs_line

WEATHEROBJS = main_weather_check.o weather_func.o $(COMOBJS)
WEATHERBIN = $(UTILDIR)/check_tat_weather

MSGHQOBJS = main_send_msg_HQ.o send_remote_func.o  $(COMOBJS)
MSGHQBIN = $(BINDIR)/send_msg_hq

ASTROMEOBJS = main_use_astrometry.o dsp_func.o ppc_func.o adjust_fov.o $(COMOBJS)
ASTROMEBIN = $(UTILDIR)/get_RA_DEC_astrometry

DAEMONBIN = $(DSPBIN) $(BINDIR)/$(CCD)  $(PPCBIN) $(CTLBIN) $(PWRBIN) $(FLIBIN) $(LSTBIN)

APPBIN = $(AUTOBIN) $(EMGBIN) $(MENUBIN) $(INSERTOBSBIN)\
	$(STARBIN) $(STARBINF) $(FLATBIN)  $(FLATBINF)\
	$(DARKBIN) $(CHECKBIN) $(MSGHQBIN) 

ALLBIN = $(DAEMONBIN) $(APPBIN)
 
$(AUTOOBJS):
$(PPCOBJS):
$(CTLOBJS):
$(DSPOBJS):
$(FLATOBJS):
$(DARKOBJS):
$(STAROBJS):
$(ADJUST):
$(WEBOBJS):
$(UDSPOBJ):
$(CHECKOBJS):
$(WEATHEROBJS):
$(SHMOBJS):
$(ASTROMEOBJS):

autocontrol: $(AUTOOBJS)
	$(CC) -o $(AUTOBIN) $(AUTOOBJS) $(LIBCOMMON) $(LIBMYSQL) $(LIBFITS)

emergencystop: $(EMGOBJS)
	$(CC) $(EMGOBJS) -o $(EMGBIN) $(LIBCOMMON) $(LIBFITS)

dspdaemon: $(DSPOBJS)
	$(CC) -o $(DSPBIN) $(DSPOBJS) $(LIBTHREAD)

ccddaemonf6: $(CCDSRC)
	$(CC) -o $(CCDBIN) $(CCDSRC) $(LIBCOMMON) $(LIBFITS)

ccddaemonu6:
	cd ccdU6;make ccddaemonu6

ppcdaemon: $(PPCOBJS)
	$(CC) -o $(PPCBIN) $(PPCOBJS) -O2 $(LIBCOMMON)

pwrdaemon: $(PWROBJS)
	$(CC) -o $(PWRBIN) $(PWROBJS) -O2

ctldaemon: $(CTLOBJS)
	$(CC) -o $(CTLBIN) $(CTLOBJS) $(LIBCOMMON)

flidaemon: $(FLIOBJS)
	$(CC) -o $(FLIBIN) $(FLIOBJS) $(LIBFLI) $(LIBMATH)

lstdaemon: $(LSTOBJS)
	$(CC) -o $(LSTBIN) $(LSTOBJS) $(LIBMYSQL) 
	
star: $(STAROBJS)
	$(CC) -o $(STARBIN) $(STAROBJS) $(LIBCOMMON) $(LIBFITS) $(LIBMYSQL) 

cmdmenu: $(MENUOBJS)
	$(CC) -g -o $(MENUBIN) $(MENUOBJS) $(LIBCOMMON) $(LIBFITS) $(LIBMYSQL)

flat: $(FLATOBJS)
	$(CC) -g -o $(FLATBIN) $(FLATOBJS) $(LIBCOMMON) $(LIBFITS) $(LIBMYSQL)
	
dark: $(DARKOBJS)
	$(CC) -g -o $(DARKBIN) $(DARKOBJS) $(LIBCOMMON)

web:$(WEBOBJS)
	$(CC) -g -o $(WEBBIN) $(WEBOBJS) $(LIBCOMMON) $(LIBFITS) $(LIBMYSQL)

read_shm:$(SHMOBJS)
	$(CC) -g -o $(SHMBIN) $(SHMOBJS) $(LIBCOMMON)

update_dsp:$(UDSPOBJ)
	$(CC) -g -o $(UDSPBIN) $(UDSPOBJ) $(LIBTHREAD)

checkinput:$(CHECKOBJS) $(INSERTOBSOBJS)
	$(CC) -g -o $(CHECKBIN) $(CHECKOBJS) $(LIBCOMMON)
	$(CC) -g -o $(INSERTOBSBIN) $(INSERTOBSOBJS) $(LIBCOMMON)

check_weather:$(WEATHEROBJS)
	$(CC) -g -o $(WEATHERBIN) $(WEATHEROBJS) $(LIBCOMMON) $(LIBMYSQL)
	
send_msg_hq:$(MSGHQOBJS)
	$(CC) -g -o $(MSGHQBIN) $(MSGHQOBJS) $(LIBCOMMON)

use_astrometry:$(ASTROMEOBJS)
	$(CC) -g -o $(ASTROMEBIN) $(ASTROMEOBJS) $(LIBFITS) $(LIBCOMMON) $(LIBMYSQL)
	
.PHONY : clean daemon_install app_install web_install up_install install
install:
	make daemon_install;make app_install; make up_install; make web_install;

app_install :
	install -m 755 -o observer -g users $(APPBIN) $(APP_INSTDIR)
	cp -n  ref_parameter.dat tat_parameter.dat $(APP_INSTDIR)

up_install:
	@mkdir -p -m 777 $(UDSP_INSTDIR)
	install -m 777 -o root -g root $(UDSPFILES) $(UDSP_INSTDIR)

web_install:
	install -m 777 -o root -g root $(WEBFILES) $(CGI_INSTDIR)
#	setfacl -R -m u:apache:rx /var/www/
#	setfacl -m u:observer:rw /var/www/htdocs/

daemon_install :
	@mkdir -p -m 777 $(DAEMON_INSTDIR) 
	install -m 744 $(DAEMONBIN) $(DAEMON_INSTDIR)
	ln -sf $(DAEMON_INSTDIR)/dspdaemon $(DAEMON_INSTDIR)/dspdaemon-curr
	ln -sf $(DAEMON_INSTDIR)/$(CCD) $(DAEMON_INSTDIR)/ccddaemon-curr
	ln -sf $(DAEMON_INSTDIR)/ppcdaemon $(DAEMON_INSTDIR)/ppcdaemon-curr
	ln -sf $(DAEMON_INSTDIR)/pwrdaemon $(DAEMON_INSTDIR)/pwrdaemon-curr
	ln -sf $(DAEMON_INSTDIR)/ctldaemon $(DAEMON_INSTDIR)/ctldaemon-curr
	ln -sf $(DAEMON_INSTDIR)/flidaemon $(DAEMON_INSTDIR)/flidaemon-curr
	ln -sf $(DAEMON_INSTDIR)/lstdaemon $(DAEMON_INSTDIR)/lstdaemon-curr

clean:
	@rm *.o */*.o */*~ *~ $(UTILDIR)/* $(BINDIR)/* -f; echo done
