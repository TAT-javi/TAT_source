
/****************************************************************
           star_track-v3d.c (2006/01/23)
****************************************************************
Modifications:
  1. Major modifications:
     Using higher speed for third-stage movement to make sure telescope
     arrives the preset poistion earlier, and wait there for preset time
     arrives.  The preset time and position are computed as before.
    a. Line 130: add the actual speed for third stage
    b. Line 517: both RA and DEC puluses are restricted.
    c. LIne 569: third-stage speed is changed to higher value
  2. Minor modifications:
    a. Change the delay time from 1 sec to 5 sec to make sure computation
       is finished before sending signals to DSP.
    b. Move getting sychronizing time (time(&curtime))
       to line 412, immediately after getting system time.
    c. Line 600: change actual observing time (RA motor stops at closeTime)
    d. Some minor changes, such as adding explanations, print more
       statements to log file....
Compile:   cc star_track-v3d.c posixtm.c east.info.c -lm -o star_track-v3d
****************************************************************
           star_track-v3d.c (2005/07/15)
****************************************************************
Modifications:
  1. Line 361:  stop the program if time is wrong
  2. Line 220:  change the log filename to distinguish from that of manual
  3. function mv(), line 681:  change criterion from -6hr to -5.3hr
****************************************************************
           star_track-v3d.c (2005/06/22)
****************************************************************
Modifications:
  1.  The position of reference is read from a file: ref_parameter.dat
  2.  It needs "posixtm.c" while compiling:
      cc star_track-v3d.c posixtm.c -lm -o star_track-v3d
****************************************************************
           star_track-v3d.c (2005/06/10)
****************************************************************
Modifications:
  1.  Change the foramt of date_string and other output format
  2.  Line 264: Change "if" statement to solve the problem of
       open_day different from close_day.
  3.  It needs "posixtm.c" while compiling:
      cc star_track-v3d.c posixtm.c -lm -o star_track-v3d
****************************************************************
           star_track-v3d.c (2005/05/15)
****************************************************************
Modifications:
  1.  Line 280: The original one does not include the second of DEC.
      The new one includes up to the second.
****************************************************************
           star_track-v3d.c (2005/04/28)
****************************************************************
Modifications:
  1. Line 125: The size of character cmd was not long enough,
     now changes to 100.
  2. Line 241: The original constraint (open_flag == "Y") is removed.
****************************************************************
           star_track-v3d.c (2004/10/17)
****************************************************************
Modifications:
  1. Input star's coordinates from a file only.
  2. New formate of input file.
****************************************************************
           star_track-v2d.c (2004/10/17)
Modifications:
  1. Use three different speeds for RA to move to the position of star.
     It remains two speeds for DEC.
  2. Only DEC pulse number is restricted.  RA pulse number is not
     because count of RA pulsse is not accurate due to different timer.
****************************************************************
           star_track-v1d.c (2004/10/10)
Modifications:
  1. set range of hour angle to be (-12hr, 12hr)
  2. make sure the pulse number not to exceed
  3. system time is set to UTC, so LOCAL_UT = 0
*****************************************************************/
/*           star_track-v1c.c (2003/2/26)
How to compile:
cc star_track-v1c.c -lm

A star tracking program based on observe-v1.5e

What's new:
1. Changes the tracking speed to 2-phase, 500Hz for the first 10s, and jump to 2KHz for the rest.

Changes log:
(star_track-v1b.c)
1. Found timing error which causes the moving time of RA ended 1 second earlier as expected.
   This will reduces the tracking error from 0.25deg to 0.15deg

(star_track-v1a.c)
1. The serial port routine has been separated, and it is now a standalone daemon.
   it's now if ones needs to send data to DSP through serial port, it can be accomplished through
   writing a command file to serialdaemon.			(15.Feb 2003)
2. timing function is changed from Date structure
   to int(long) data type(time_t) of seconds since 1.Jan.1970.	(18.Jan 2003)
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <curses.h>
#include <pthread.h>

#include "symblc_const.h"
#include "myAstro.h"		/*Header file for Astronomical routines and structures*/
#include "main_cmd_menu.h"

//#define EXPOSURE_I		0   //exposure time (integer part) in sec
//#define EXPOSURE_D		80  //exposure time (decimal part), e.g. 10=0.1 sec

// parameters for tracking system, determined by DSP tests
//#define RA_DEG_BEGIN		84.2044+0.15	//RA : degree between zenith and origin position
//#define DEC_DEG_BEGIN		74.8042+0.4	//DEC: degree between zenith and origin position
//#define RA_DEG_BEGIN		105.76
//#define DEC_DEG_BEGIN		55.26
//#define RA_DEG_BEGIN		86.7161   // TF (2004.7)
//#define DEC_DEG_BEGIN		59.6818-0.2   // TF (2004.7)
//#define RA_DEG_BEGIN		86.7161 - 0.05   // TF (2004.10)
//#define DEC_DEG_BEGIN		59.6818 + 0.16  // TF (2004.10)

#define	DTOP_RA			10000	//no. of pulses per degree for RA
#define	DTOP_DEC	 	 6000	//no. of pulses per degree for DEC

#define	FAST_SPEED_REGISTER1	833	//500Hz
#define	FAST_SPEED_REGISTER2	208	//2KHz
#define	FAST_SPEED_REGISTER3	833*2   //250Hz
#define	FAST_SPEED_REGISTER3F	417     //1KHz  //atual speed used to make sure telescope reaches preset position to wait for time arrive

#define	FAST_SPEED_TIME1	10
#define BUFFERTIME		10	/* to make sure transition between                                       different speed is smooth  */
#define BUFFERTIME_1		0       /* to make sure RA pulse number is enough*/
#define BUFFERTIME_2		2       /* to make sure DEC pulse number is enough*/
#define DELAY_COMPUTE	        5 // delay to make sure computation done before send commands to DSP

#define INPUT_FILENAME	        "/home/observer/star/data/star_input.dat" /* file of star */
#define REF_FILENAME	        "/home/observer/ref_parameter.dat" /* file of star */

//Speed of motor to move from origin to position

/************************************************************/
/*******  parameters for local observing site  **************/
//THESE PAREMETERS ARE IN symblc.const.h
/*
//#define  LOCAL_LAT		 28.3003  //TF local latitude (in deg)
//#define  LOCAL_LONG		-16.5122  //TF local longitude(in deg)
#define  LOCAL_LAT		 38.4053  //UZ local latitude (in deg)
#define  LOCAL_LONG		 66.5376  //UZ local longitude(in deg)
//#define  LOCAL_LAT		 24.79	  //NTHU local latitude (in deg)
//#define  LOCAL_LONG		121.0	  //NTHU local longitude(in deg)
#define  LOCAL_UT		  0.0   //difference between system time and UT time
#define	 GmtOffSet	LOCAL_LONG*24.0/360.0  // in hour
*/

#define SIDEREALSPEED		41.78079012//In Hz, for stepper motor
#define MENUSTARLIST		1
#define MENUUSERSTAR		2
#define MENUFILE		3
#define MENUQUIT		4

#define IN_THE_SKY		0
#define OUT_OF_RANGE		1
#define NOT_RISE		2
#define ALREADY_SET		4


struct TrackingMotor	//structure for Ra and Dec motors
{
	int	cur_step;
	int	actual_time_mv, time_mv, start_time_mv, time_mv2, time_mv3;
	int	star_speed_register;

	float	time_mv_f, time_mv2_f, time_mv3_f, actual_time_mv_f;
	float	deg_begin;
	float	deg0_mv, deg1_mv;
	float	star_speed_deg, star_speed_pulse;
	float	motor_speed1, motor_speed2, motor_speed3, motor_speed3f;
	float	dtop;

	//Variable for DSP string
	char cmd[100];
}Ra, Dec;

struct StarList
{
	//right ascension and declination for star
	float ra;
	float dec;
	char name[64];
};

typedef struct StarList STARLIST;

DATE convert2UT(DATE d);
DATE getSystemTime();

void star_timing(long);		// waits until for certain time
char mv(DATE, STARLIST, float *ra_deg_mv, float *dec_deg_mv);
// computes degrees and steps telescope should move
time_t getStarSetTime();

EQUATOR Star;	//Star position in Equatorial coordinate

void *star_track(void*)	;
char IsObservable;
STARLIST StarPos;


extern WINDOW *star_win;
/*
main()
{

	pthread_t	star_thread;
	int		res;

	initscr();


	star_win = newwin(7, COLS, 16, 0);
        scrollok(star_win, 1);
	printw("Creates Star Track thread.\n");
        res=pthread_create(&star_thread,NULL,star_track,NULL);
        if(res!=0)
        {
                perror("Serial_read_thread Creation Failed.\n");
                exit(EXIT_FAILURE);
        }
        else {printw("star_thread Created.\n");}

	while(1)
	{
	mvprintw(0,0, "this is main win");
	//wrefresh( star_win );
	refresh();
	sleep (1);

	}

	delwin(star_win);

	endwin();


}
*/
extern	st_tat_info     *p_tat_info;
void *star_track(void* arg)
{
	float	tmp1, tmp2;
	double	RaActualPulse, DecActualPulse;
	double	open_time, close_time;
	int	if_time_flag;
	int	i, shutter_flag, cycle, exposure_i, exposure_d;
	int	ra_h, ra_m, ra_s, dec_d, dec_m, dec_s, expo_s, expo_d;
	int	temp, cycle_time, cycle_number, obs_time, actual_obs_time;
	int	cycle_time_min, delay, actual_cycle_number;
	char	open_string[13], close_string[13];
	char	open_flag[1], date_string[200], date_string1[200], command[200];
	char	image_filename[200], log_filename[200], camera_command[200];
	char	log_file[100], star_filename[100], str[200], str1[200];
	FILE	*fn1, *camera_fn, *fout, *fin1, *fin2, *fn_test;


	//Variables for Time: Local Time and Universal Time
	DATE lt, ut, starlist, open, close;
	time_t curtime, waittime, openTime, closeTime, obscurtime;

	unsigned short nstar=8;
	unsigned char MenuFlagOk, ListStarOk, UserStarRaOk, UserStarDecOk;
	short MenuFlag, ListStarChoice;
	float UserStarRa, UserStarDec;



	if( p_tat_info->dsp_info.ra.status != InOrigin ||
	    p_tat_info->dsp_info.dec.status != InOrigin )
        {
          wprintw( star_win, "Move Telescope to reference position first.\n");
          pthread_exit( (void*) 0);

        }

//	Ra.deg_begin	= RA_DEG_BEGIN;
	Ra.dtop		= DTOP_RA;

//	Dec.deg_begin	= DEC_DEG_BEGIN;
	Dec.dtop	= DTOP_DEC;

//	exposure_i	= EXPOSURE_I;
//	exposure_d	= EXPOSURE_D;



/********* beginning of main ***********************/

// open log file
	lt= getSystemTime();
        sprintf(date_string, "%4d%02d%02d", lt.yr, lt.mon, lt.day);
        sprintf(date_string1, "%4d%02d%02d", lt.yr, lt.mon, lt.day+1);
        sprintf(log_filename, "/home/observer/star/log/%st.manual.log", date_string);
	if( (fn1=fopen(log_filename,"a+") )==NULL)
  	  sprintf("Can not open %s", log_filename);

	wprintw(star_win, "*******************************************************************************\n");
	wprintw(star_win, "***************************  OBSERVATION BEGINS  ******************************\n");
	wprintw(star_win, "*******************************************************************************\n");
	fprintf(fn1,"*******************************************************************************\n"); fflush(fn1);
	fprintf(fn1,"***************************  OBSERVATION BEGINS  ******************************\n");  fflush(fn1);
	fprintf(fn1,"*******************************************************************************\n"); fflush(fn1);

	wprintw(star_win, "Log file: %s\n", log_filename);
	fprintf(fn1, "Log file: %s\n", log_filename); fflush(fn1);


// read position of reference
	if((fin2=fopen(REF_FILENAME,"r"))==NULL)
	  {wprintw(star_win, "Input file %s does not exist.\n",REF_FILENAME);
	   fprintf(fn1, "Input file %s does not exist.\n",REF_FILENAME);
 	   fflush(fn1); return 0;}
	fscanf(fin2,"%f", &Ra.deg_begin);
	fscanf(fin2,"%f", &Dec.deg_begin);
	fclose(fin2);

	wprintw(star_win,"reference position: RA(deg)=%10.5f,  DEC(deg)=%10.5f\n", Ra.deg_begin,
Dec.deg_begin);
	fprintf(fn1,"reference position: RA(deg)=%10.5f,  DEC(deg)=%10.5f\n", Ra.deg_begin,Dec.deg_begin); fflush(fn1);


//////////////////////////////////////////////////////////////////
//  read input file
//////////////////////////////////////////////////////////////////
//  star coordinates
	      StarPos.ra = p_tat_info->obs_info.curr_ra;
	      StarPos.dec= p_tat_info->obs_info.curr_dec;
	      wprintw(star_win, "Star's Coordinates:  RA = %9.6f;  DEC = %10.6f\n", StarPos.ra, StarPos.dec);
	      fprintf(fn1,"Star's Coordinates:  RA = %9.6f;  DEC = %10.6f\n", StarPos.ra, StarPos.dec);
	      fflush(fn1);
//  CCD exposure time

// total observing time

// check star's coordinates
	      if(StarPos.dec < -30.0 || StarPos.dec > 55.0)
	        {wprintw(star_win,"DEC out of range (-30, 55)! Observation stops!\n");
	         fprintf(fn1, "DEC out of range (-30, 55)! Observation stops!\n");  fflush(fn1);
	         return 0; }

	      wprintw(star_win, "*******************************************************************************\n");
	      fprintf(fn1,"*******************************************************************************\n");
	      fflush(fn1);

////////////////////////////////////////////////////////////////////////
	//	Starting to move telescope from origin to star
////////////////////////////////////////////////////////////////////////

//Calculate degrees telescope should move to point to object
// get system (local) time
	lt= getSystemTime();

//	printf("0 Current System Time (YYYY/MM/DD hr:min:sec)=%4d/%2d/%2d %2d:%2d:%2d\n",lt.yr, lt.mon, lt.day, lt.hr, lt.min, lt.sec);
//	fprintf(fn1,"0 Current System Time (YYYY/MM/DD hr:min:sec)=%4d/%2d/%2d %2d:%2d:%2d\n",
//	        lt.yr, lt.mon, lt.day, lt.hr, lt.min, lt.sec);
//	fflush(fn1);


/* Compute the position at a delay of DELAY_COMPUTE seconds;
DSP command will also be sent out at this time to allow computer
to have time process following procedure */

	lt.sec += DELAY_COMPUTE;
//	printf("0 Compute position for next second(YYYY/MM/DD hr:min:sec)=%4d/%2d/%2d %2d:%2d:%2d\n",  lt.yr, lt.mon, lt.day, lt.hr, lt.min, lt.sec);

	//current time in seconds, will be used to synchronize the deay of DELAY_COMPUTE seconds in timing
	time(&curtime);

	IsObservable = mv(lt, StarPos, &Ra.deg0_mv,&Dec.deg0_mv);
//	If IsObservable=0, star not observanle; if 1, observable.


	if(IsObservable!=IN_THE_SKY)//NOT OBSERVABLE
	  {wprintw(star_win,"Object is NOT OBSERVABLE! Program Terminated.\n");
	   fprintf(fn1, "Object is NOT OBSERVABLE! Program Terminated.\n");
           fflush(fn1);  return 0;  }

	if(Ra.deg0_mv <= 0. || Dec.deg0_mv <= 0.)
	  {wprintw(star_win,"Object out of range! Stop. Wait and run again.  RA_mv=%8.2f, DEC_mv=%8.2f\n", Ra.deg0_mv, Dec.deg0_mv);
	   fprintf(fn1, "Object out of range! Stop. Wait and run again. RA_mv=%8.2f, DEC_mv=%8.2f\n", Ra.deg0_mv, Dec.deg0_mv);
	   fflush(fn1);  return 0;  }


	// computes the speed for stepper motor (pulse per second)
        // fast move ~500Hz for 10s
	// Ra and Dec DSP timer is prescale to 10
	Ra.motor_speed1 = 1./(2*10*120.0e-9*FAST_SPEED_REGISTER1);
	Dec.motor_speed1= 1./(2*10*120.0e-9*FAST_SPEED_REGISTER1);

	Ra.motor_speed2 = 1./(2*10*120.0e-9*FAST_SPEED_REGISTER2);
	Dec.motor_speed2= 1./(2*10*120.0e-9*FAST_SPEED_REGISTER2);

	Ra.motor_speed3 = 1./(2*10*120.0e-9*FAST_SPEED_REGISTER3);
	Dec.motor_speed3= 1./(2*10*120.0e-9*FAST_SPEED_REGISTER3);

	Ra.motor_speed3f = 1./(2*10*120.0e-9*FAST_SPEED_REGISTER3F);

	wprintw(star_win,"Motor Speed 1 (Hz): RA= %8.2f; DEC= %8.2f\n", Ra.motor_speed1, Dec.motor_speed1);
	wprintw(star_win,"Motor Speed 2 (Hz): RA= %8.2f; DEC= %8.2f\n", Ra.motor_speed2, Dec.motor_speed2);
	wprintw(star_win,"Motor Speed 3 (Hz): RA= %8.2f\n", Ra.motor_speed3f);

	fprintf(fn1, "Motor Speed 1 (Hz): RA= %8.2f; DEC= %8.2f\n", Ra.motor_speed1, Dec.motor_speed1); fflush(fn1);
	fprintf(fn1, "Motor Speed 2 (Hz): RA= %8.2f; DEC= %8.2f\n", Ra.motor_speed2, Dec.motor_speed2); fflush(fn1);
	fprintf(fn1, "Motor Speed 3 (Hz): RA= %8.2f\n", Ra.motor_speed3f); fflush(fn1);

	wprintw(star_win,"Distance from Reference (deg): RA= %7.2f; DEC= %7.2f\n", Ra.deg0_mv, Dec.deg0_mv);
	fprintf(fn1, "Distance from Reference (deg): RA= %7.2f; DEC= %7.2f\n", Ra.deg0_mv, Dec.deg0_mv);  fflush(fn1);

//Compute time required to move from origin to object's current position
	Ra.time_mv2_f = (float)( Ra.deg0_mv* Ra.dtop - FAST_SPEED_TIME1* Ra.motor_speed1)/ Ra.motor_speed2;
	Dec.time_mv2_f= (float)(Dec.deg0_mv*Dec.dtop - FAST_SPEED_TIME1*Dec.motor_speed1)/Dec.motor_speed2;

        if(Ra.time_mv2_f < 0. || Dec.time_mv2_f < 0. )
          {wprintw(star_win,"2nd-stage time is negative! Stop! Wait and run again.  RA=%10.2f, DEC=%10.2f\n", Ra.time_mv2_f, Dec.time_mv2_f);
           fprintf(fn1, "2nd-stage time is negative! Stop! Wait and run again.   RA=%12.2f, DEC=%12.2f\n", Ra.time_mv2_f, Dec.time_mv2_f);
           fflush(fn1); return 0;
	  }

	Ra.time_mv2   = (int)(0.5 + Ra.time_mv2_f);
	Dec.time_mv2  = (int)(0.5 +Dec.time_mv2_f);
	Ra.time_mv_f  = (float)FAST_SPEED_TIME1 + Ra.time_mv2_f;
	Dec.time_mv_f = (float)FAST_SPEED_TIME1 +Dec.time_mv2_f;
	Ra.time_mv    = FAST_SPEED_TIME1 + Ra.time_mv2;
	Dec.time_mv   = FAST_SPEED_TIME1 +Dec.time_mv2;

// Compute additional time required to move to object due to movement of star
// A third speed will be used for this additional time
// See the note for the formular to compute Ra.time_mv3_f.
        Ra.time_mv3_f = (SIDEREALSPEED*Ra.time_mv + Ra.motor_speed2*(Ra.time_mv_f-Ra.time_mv))/(Ra.motor_speed3-SIDEREALSPEED);
	Ra.time_mv3   = (int)(0.5 + Ra.time_mv3_f);

// Compute total time required to move to object due to movement of star
	Ra.actual_time_mv_f = Ra.time_mv_f + Ra.time_mv3_f;
	Ra.actual_time_mv   = (int)(0.5 + Ra.actual_time_mv_f);
// Star does not move in DEC
	Dec.actual_time_mv  = Dec.time_mv;

// print information
	wprintw(star_win," RA: 1st stage= 10 s; 2nd stage=%4d s; 3rd stage=%4d s; total=%5d s\n", Ra.time_mv2, Ra.time_mv3, Ra.actual_time_mv);
	fprintf(fn1," RA: 1st stage= 10 s; 2nd stage=%4d s; 3rd stage=%4d s; total=%5d s\n", Ra.time_mv2, Ra.time_mv3, Ra.actual_time_mv);
        fflush(fn1);

	wprintw(star_win,"Dec: 1st stage= 10 s; 2nd stage=%4d s; total=%5d s\n", Dec.time_mv2, Dec.actual_time_mv);
	fprintf(fn1,"Dec: 1st stage= 10 s; 2nd stage=%4d s; total=%5d s\n", Dec.time_mv2, Dec.actual_time_mv);
        fflush(fn1);

/**************************************************************************/
//	compute actual number of pulses
/*
        tmp1 = Ra.motor_speed1*FAST_SPEED_TIME1 + Ra.motor_speed2* Ra.time_mv2_f + Ra.motor_speed3*Ra.time_mv3_f;
        tmp2 =Dec.motor_speed1*FAST_SPEED_TIME1 +Dec.motor_speed2*Dec.time_mv2_f;

	printf("RA pulse from speed = %12.1f, Dec pulse from speed = %12.1f\n", tmp1, tmp2);
	fprintf(fn1,"RA pulse from speed = %12.1f, Dec pulse from speed = %12.1f\n", tmp1, tmp2);
        fflush(fn1);
*/

	RaActualPulse = Ra.deg0_mv*DTOP_RA + SIDEREALSPEED*Ra.actual_time_mv;
	DecActualPulse = Dec.deg0_mv*DTOP_DEC;

	wprintw(star_win,"RA actual pulse:      %12.1f, DEC actual pulse:      %12.1f\n",
		RaActualPulse,	DecActualPulse);
	fprintf(fn1,"RA actual pulse:      %12.1f, DEC actual pulse:      %12.1f\n",
		RaActualPulse,	DecActualPulse);
        fflush(fn1);

//  send out DSP command to restrict the number of pulses
	sprintf(Ra.cmd ,"RA POS LIMIT 0 0 0 0 0 0 0\nRA POS LIMIT 1 0 %d 0 0 0 0\n", (int)RaActualPulse);
	sprintf(Dec.cmd ,"DEC POS LIMIT 0 0 0 0 0 0 0\nDEC POS LIMIT 1 0 %d 0 0 0 0\n", (int)DecActualPulse);
	fout=fopen("dsp_command.cmd", "w");
// both RA and DEC puluse numbers are restricted.
	fprintf(fout, "%s\n%s\n", Ra.cmd, Dec.cmd);
// only DEC pulse number is restricted.
//	fprintf(fout, "%s\n", Dec.cmd);
	fclose(fout);

	wprintw(star_win,"%s\n%s\n", Ra.cmd, Dec.cmd);
	fprintf(fn1, "%s\n%s\n", Ra.cmd, Dec.cmd); fflush(fn1);
/**************************************************************************/

	Ra.start_time_mv= Dec.start_time_mv= 1;


	p_tat_info->obs_info.status = Pursuing;
	// First Stage
	//Move RA/DEC motor with 500Hz for 10sec
	//BUFFERTIME is added to avoid gap between two different speeds
	sprintf(Ra.cmd, "RA POS FREQUENCY %d %d %d 0 0 0 0", Ra.start_time_mv, FAST_SPEED_TIME1+BUFFERTIME, FAST_SPEED_REGISTER1);
	sprintf(Dec.cmd, "DEC POS FREQUENCY %d %d %d 0 0 0 0", Dec.start_time_mv, FAST_SPEED_TIME1+BUFFERTIME, FAST_SPEED_REGISTER1);

	wprintw(star_win,"Waiting for %d sec, until %4d/%02d/%02d %02d:%02d:%02d, to send commands to DSP\n", DELAY_COMPUTE, lt.yr, lt.mon, lt.day, lt.hr, lt.min, lt.sec);
	fprintf(fn1, "Waiting for %d sec, until %4d/%02d/%02d %02d:%02d:%02d, to send commands to DSP\n", DELAY_COMPUTE, lt.yr, lt.mon, lt.day, lt.hr, lt.min, lt.sec);
	fflush(fn1);

	// synchronize with the time at which variables are computed
	star_timing(curtime+ DELAY_COMPUTE);

	//Send commands to DSP
	fout=fopen("dsp_command.cmd", "w");
	fprintf(fout, "%s\n%s\n", Ra.cmd, Dec.cmd);
	fclose(fout);
/*
	sleep(1);
	fout=fopen("dsp_command.cmd", "w");
	sprintf(Ra.cmd, "RA POS REQUEST %d %d %d 0 0 0 0", Ra.start_time_mv, FAST_SPEED_TIME1+BUFFERTIME, FAST_SPEED_REGISTER1);
	fprintf(fout, "%s\n", Ra.cmd);
	fclose(fout);
*/
	wprintw(star_win,"Ra.cmd=%s, Dec.cmd=%s\n", Ra.cmd, Dec.cmd);
	fprintf(fn1, "0 Ra.cmd=%s, Dec.cmd=%s\n", Ra.cmd, Dec.cmd);
        fflush(fn1);

        p_tat_info->obs_info.status = Pursuing;
	// Second Stage
	//Move RA/DEC motor with 2kHz for Ra.time_mv2 and Dec.time_mv2
	sprintf(Ra.cmd ,"RA POS FREQUENCY %d %d %d 0 0 0 0", Ra.start_time_mv, Ra.time_mv2+BUFFERTIME, FAST_SPEED_REGISTER2);
	//BUFFERTIME is NOT added for DEC since it stops at the end of 2nd stage, instead BUFFERTIME_2 is added to make sure DEC has enough pulse number
	sprintf(Dec.cmd ,"DEC POS FREQUENCY %d %d %d 0 0 0 0", Dec.start_time_mv, Dec.time_mv2+BUFFERTIME_2, FAST_SPEED_REGISTER2);

	// synchronize with the time at which variables are computed
	star_timing(curtime+ DELAY_COMPUTE + FAST_SPEED_TIME1);

	//Send commands to DSP
	fout=fopen("dsp_command.cmd", "w");
	fprintf(fout, "%s\n%s\n", Ra.cmd, Dec.cmd);
	fclose(fout);
	wprintw(star_win,"Ra.cmd=%s, Dec.cmd=%s\n", Ra.cmd, Dec.cmd);
	fprintf(fn1, "0 Ra.cmd=%s, Dec.cmd=%s\n", Ra.cmd, Dec.cmd);
        fflush(fn1);
/*
	sleep(1);
	fout=fopen("dsp_command.cmd", "w");
	sprintf(Ra.cmd, "RA POS REQUEST %d %d %d 0 0 0 0", Ra.start_time_mv, FAST_SPEED_TIME1+BUFFERTIME, FAST_SPEED_REGISTER1);
	fprintf(fout, "%s\n", Ra.cmd);
	fclose(fout);
*/

        p_tat_info->obs_info.status = Pursuing;
	// Third Stage
	//Move RA motor with 1KHz for Ra.time_mv3
	sprintf(Ra.cmd ,"RA POS FREQUENCY %d %d %d 0 0 0 0", Ra.start_time_mv, Ra.time_mv3+BUFFERTIME, FAST_SPEED_REGISTER3F);

	// synchronize with the time at which variables are computed
	star_timing(curtime+ DELAY_COMPUTE + FAST_SPEED_TIME1 + Ra.time_mv2);

	//Send commands to DSP
	fout=fopen("dsp_command.cmd", "w");
	fprintf(fout, "%s\n", Ra.cmd);
	fclose(fout);
	wprintw(star_win,"Ra.cmd=%s\n", Ra.cmd);
	fprintf(fn1, "0 Ra.cmd=%s\n", Ra.cmd); fflush(fn1);

/*************************************************************************/
//  end of moving telescope from origin to star
/*************************************************************************/

	//waits till RA moving time is finish.
        // synchronize with the time at which variables are computed
        // BUFFERTIME_1 is added to make sure the pulse number is enough
	star_timing(curtime + DELAY_COMPUTE + Ra.actual_time_mv + BUFFERTIME_1);

	wprintw(star_win,"Telescope has pointed to the star.\n");
	fprintf(fn1, "Telescope has pointed to the star.\n"); fflush(fn1);

/*************************************************************************/
/*  DEC will stop after fastmove.  RA will move with a speed  */
/*************************************************************************/
//  compute actual observing time
	/*time(&curtime);*/
	time(&obscurtime);
	actual_obs_time=closeTime-obscurtime;
	actual_cycle_number=(time_t)actual_obs_time/60;
//	wprintw(star_win,"actual observing time = %6ld sec;  actual number of cycle = %4d\n", actual_obs_time, actual_cycle_number);
//	fprintf(fn1, "actual observing time = %6d sec;  actual number of cycle = %4d\n", actual_obs_time, actual_cycle_number);
//	fflush(fn1);

	Ra.star_speed_register = (int)( 0.5 + ( 1./ (2*10*120.0e-9*SIDEREALSPEED) ) );

	sprintf(Ra.cmd ,"RA POS FREQUENCY 1 %d %d 0 0 0 0", getStarSetTime(), Ra.star_speed_register);
	fout=fopen("dsp_command.cmd", "w");
	fprintf(fout, "%s\n ", Ra.cmd);
	fclose(fout);
	wprintw(star_win,"Ra.cmd=%s\n", Ra.cmd);
	fprintf(fn1, "0 Ra.cmd=%s\n", Ra.cmd);
        fflush(fn1);

/*************************************************************************/
//   beginning of taking the first image
/*************************************************************************/
// Wait for telescope motion to stabilize
// If DEC takes longer time than RA, it needs to wait until DEC finishes.
	if(Dec.actual_time_mv > Ra.actual_time_mv)
	  {wprintw(star_win,"DEC is longer than RA by %5d,  waiting....\n",
	            Dec.actual_time_mv - Ra.actual_time_mv);
	   sleep(10 + (Dec.actual_time_mv - Ra.actual_time_mv) );
          }

          wprintw(star_win,"\nTelescope has pointed to the star.\n");
        p_tat_info->obs_info.status = Tracking;

	return 0;
} // main

/*****************   end of main    **********************************/


/********************************************************************/
/* Subroutine for computing degrees and steps telescope should move **/
/********************************************************************/
/* subroutine:
	char mv(DATE d, STARLIST star_pos, ra_deg_mv, dec_deg_mv)
	Version 1.1  (2003/1/1)

	Function:
		Input	: date
		Output	: degrees telescope should move:
			  	ra_deg_mv, dec_deg_mv

		return 0  if the object is below horizon,
			  telescope will NOT move.
		return 1  if the object is above horizon,
			  telescope will move to object position
								      */
/**********************************************************************/

char mv(DATE d, STARLIST star_pos, float *ra_deg_mv, float *dec_deg_mv)
{
	extern	struct TrackingMotor	Ra, Dec;

	DATE ut, lt;//Universal and Local Time
	double	LST; //Local Sidereal Time, in hours

	float	hourAngle_obj, dec_deg_obj;
	float	ra_deg1_mv, dec_deg1_mv;

	char objectStatus=0;	//Object is NOT Observable

//	printf("TEST  %f %f \n", Ra.deg_begin, Dec.deg_begin);

//	printf("mv:System Time (YYYY/MM/DD hr:min:sec)=%4d/%2d/%2d %2d:%2d:%2d\n",d.yr, d.mon, d.day, d.hr, d.min, d.sec);

// convert given local time to UT
	ut= convert2UT(d);
//	printf("mv:UT (YYYY/MM/DD hr:min:sec)=%4d/%2d/%2d %2d:%2d:%2d\n",
//	        ut.yr, ut.mon, ut.day, ut.hr, ut.min, ut.sec);

// compute local sidereal time
	LST= getLocalSidereal(ut);
#ifdef VERBOSE
	wprintw(star_win,"mv: Local Sidereal Time = %10.6f (hr)\n", LST);
#endif

/*
 If Local Sidereal Time(LST) equals the Right Ascension(RA) of the star,
 the star is in zenith.
 Hour angle of star is computed as:
        HA(star) = LST(observer) - RA(star)
 If LST< RA (HA<0), the sta
 If HA<-6, the star has not risen yet.
 If HA> 6, the star has already set.
 Usually the range of HA is (0, 24hr). Here we set the range of RA
 to be (-12hr, 12hr).
*/

// compute the hour angle of the object:
	hourAngle_obj	= LST - star_pos.ra;
	dec_deg_obj	= star_pos.dec;
// set the range of HA to (-12, 12)
	while (hourAngle_obj < -12.0) hourAngle_obj +=24.0;
	while (hourAngle_obj >  12.0) hourAngle_obj -=24.0;
	if(hourAngle_obj < -12.0 || hourAngle_obj > 12.0)
	{
		wprintw(star_win,"mv: Hour angle is out of range; HA=%16.6f\n", hourAngle_obj);
		objectStatus=OUT_OF_RANGE;
		return objectStatus;
	}
#ifdef VERBOSE
	wprintw(star_win,"mv: HourAngle= %12.6f, Dec=%12.6f\n", hourAngle_obj, dec_deg_obj);
#endif
	if(hourAngle_obj < -5.3)
	{
#ifdef VERBOSE
		wprintw(star_win,"mv: HA = %f; Star has not risen yet! (<-5.3hr)\n", hourAngle_obj);
#endif
		objectStatus=NOT_RISE;	//Object is NOT OBSERVABLE
	}
	else if(hourAngle_obj > 5.3)
	{
#ifdef VERBOSE
		wprintw(star_win,"mv: HA = %f; Star has already set! (>5.3hr)\n", hourAngle_obj);
#endif
		objectStatus=ALREADY_SET;	//Object is NOT OBSERVABLE
	}
	else
	{
		objectStatus=IN_THE_SKY;	//Object is OBSERVABLE
          // computes total degree which telescope should move
	  // ra_deg_begin is the degree needed to move the Ra of telescope pointing to zenith(transit)
	  // dec_deg_begin is the degree needed to move the Dec of telescope pointing up from observer latitude
//	  ra_deg1_mv	= hourAngle_obj*15. + RA_DEG_BEGIN;
//	  dec_deg1_mv	= (dec_deg_obj - LOCAL_LAT) + DEC_DEG_BEGIN;
	  ra_deg1_mv	= hourAngle_obj*15. + Ra.deg_begin;
	  dec_deg1_mv	= (dec_deg_obj - LOCAL_LAT) + Dec.deg_begin;

  	  *ra_deg_mv	= fmod(ra_deg1_mv , 360);
	  *dec_deg_mv	= fmod(dec_deg1_mv, 360);
	}
	return objectStatus;
}

time_t getStarSetTime()
{
	DATE d;
	char 	buf[256];
	int 	yr, mon, day, hr, min, sec;
	struct tm *tm_ptr;
	time_t tm_now,i;

	time(&tm_now);


	for( i=tm_now; i<tm_now+43200;i+=3600 )
	{
		tm_ptr=localtime(&i);
		// read system time
		strftime(buf,256,"%m %d %Y %H %M %S",tm_ptr);
		sscanf(buf,"%d %d %d %d %d %d", &mon, &day, &yr, &hr, &min, &sec);
		d.mon=mon; d.day=day; d.yr=yr; d.hr=hr; d.min=min; d.sec=sec;
		if( mv(d, StarPos, &Ra.deg0_mv,&Dec.deg0_mv ) == ALREADY_SET )
		{
			return i-tm_now;
		}
	}
	return 1;

}



/**********************************************************/
/*Subroutine star_timing(long waitTillTime)		          */
/**********************************************************/
/*    Function: wait until certain time
      Input:	time in second to wait to 		  */
/**********************************************************/
extern st_manual_cmd manual;
void star_timing(long wait2Time)
{
	time_t curTime;
	int diffTime;


	while(1)
	{
		time(&curTime);
		if( manual.ra_stop_moving == 1 || manual.dec_stop_moving == 1)
		{
		        p_tat_info->obs_info.status = STOP;
		        wclear( star_win );
		        pthread_exit(NULL);
                }
		//sleep if idle time is too long to save CPU cycle
		if( ( diffTime =(wait2Time- curTime) ) > 3)
		//	usleep(10);
		{
//			if(diffTime%10==0) wprintw(star_win, "\nCountdown:");
			wprintw(star_win,"\rCountdown %4d, ",  diffTime);fflush(NULL);
			sleep(1);
		}
		else
		{
			if(curTime==wait2Time)  break;
			usleep(10);
		}
	}
}


