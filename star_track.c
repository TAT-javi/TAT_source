#undef DEBUG
//#define DEBUG 1

/************************************************
		star_track.c
		
		It will read the parameters of the observation from the input file,
		point to the star, track the star and perform observation with 
		filters.
		
		User can input 2 parameters (current RA and DEC), if not they will 
		obtained from the ref_parameter.dat
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

#include "myAstro.h"		/*Header file for Astronomical routines and structures*/
#include "symblc_const.h"
#include "adjust_fov.h"
#include "fli_func.h"

#define DTOP_RA			10000	/*no. of pulses per degree for RA*/
#define DTOP_DEC	 	 6000	/*no. of pulses per degree for DEC*/

#define FAST_SPEED_REGISTER1	833	//500Hz
#define FAST_SPEED_REGISTER2	208	//2KHz
#define FAST_SPEED_REGISTER3	833*2   //250Hz
#define FAST_SPEED_REGISTER3F	417     //1KHz

#define FAST_SPEED_TIME1	10
#define BUFFERTIME		10	/* to make sure transition between different speed is smooth  */
#define BUFFERTIME_1		0       /* to make sure RA pulse number is enough*/
#define BUFFERTIME_2		2       /* to make sure DEC pulse number is enough*/
#define DELAY_COMPUTE	        5  /*delay to make sure computation done before send commands to DSP  */

#define SIDEREALSPEED		41.78079012 /*In Hz, for stepper motor*/
#define MENUSTARLIST		1
#define MENUUSERSTAR		2
#define MENUFILE		3
#define MENUQUIT		4

#define BUFFER_SIZE 512

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
	float	mv_sign;
	float	dtop;

	//Variable for DSP string
	char cmd[100],dir[4];
}Ra, Dec;

typedef struct StarList
{
	//right ascension and declination for star
	float ra;
	float dec;
	char name[64];
}STARLIST;

void short_timing(long);// waits until for certain time
int mv(DATE, STARLIST, STARLIST, float *, float *);
// computes degrees and steps telescope should move


EQUATOR Star;//Star position in Equatorial coordinate

st_tat_info  *p_tat_info;

int main(int argc,char* argv[])
{
	double RaActualPulse, DecActualPulse;
	int	if_time_flag;
	int	i, shutter_flag, cycle,c,image_counter,IsObservable;
	int	ra_h, ra_m, ra_s, dec_d, dec_m, dec_s;
	int	temp, total_obs_time;
	int cycle_time, cycle_number;
	int	delay;
	char open_string[13], close_string[13];
	char ra_string[15], dec_string[15];
	char date_string[200], command[300];
	char image_filename[BUFSIZ], camera_command[BUFSIZ*2];
	char str[200], str1[200],target_name[50];
	char temp_string[BUFFER_SIZE],buf[BUFFER_SIZE];
	float set_point, exposure_time;
	int diff_ra,diff_dec; 
	int N_filters, filter_seq[FILTER_TOTAL_NUMBER],filter_exp_time[FILTER_TOTAL_NUMBER];
	int filter_obs_time[FILTER_TOTAL_NUMBER];
	int current_seq_step;

	char filter_type,filter_string[100];
	FILE *camera_fn, *fout, *fin1, *fin2;
	fpos_t *fposition;
	pid_t pid;
	//Variables for Time: Local Time and Universal Time
	DATE lt, ut, starlist, open, close,now;
	time_t curtime, waittime;
	time_t adjust_start,adjust_stop;
	int diff_begin,diff_end;
	int Adjust_FOV=0;
	int Min_dec=-30,Max_dec=55;
	
	STARLIST Star[12], StarPos,Current_pos;
	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);

	//Prepare everything for check the Field of view
	p_tat_info->obs_info.FOV = FOV_TBC;
	sprintf(p_tat_info->obs_info.target_name,"Unknown");
	
	Adjust_FOV = DoGetValue("ADJUST_FOV");
	Min_dec = DoGetValue("MIN_DEC");
	Max_dec = DoGetValue("MAX_DEC");

	//Check if we use current position of the telescope
	//initialize the variables
	Current_pos.ra = -1.0;
	Current_pos.dec = -100.0;
	
	if (argc == 3)
	{
		Current_pos.ra = atof(argv[1]);
		Current_pos.dec = atof(argv[2]);
		
		if(Current_pos.dec >=Max_dec || Current_pos.dec<=Min_dec)
		{
			sprintf(temp_string,"ERROR: Input current DEC position out of bounds (%f)",Current_pos.dec);
			step(temp_string);
			return 1;
		}
	}
	else if(argc!=1)
	{
		printf("#### USAGE ###\n");
		printf(" %s [Optional: Current Ra (hours) and Dec (deg)]\nExample:\n",argv[0]);
		printf(" %s\n or, \n %s 10.342 4.23123\n",argv[0],argv[0]);
		printf("#########\n");
		return 1;
	}
	
/********* beginning of main ***********************/
// open log file
	log_this("Star_track starts",STAR_TRACK_LOG_TYPE,1);

	steplog("***  OBSERVATION BEGINS  ***",STAR_TRACK_LOG_TYPE);
// read position of reference
	if(Current_pos.ra<0) //Current position is home switch
	{
		if((fin2=fopen(REF_PAR_FILENAME,"r"))==NULL)
		{
			sprintf(temp_string, "ERROR: Input file %s does not exist.",REF_PAR_FILENAME);
			steplog(temp_string,STAR_TRACK_LOG_TYPE);
			return 1;
		}
		fscanf(fin2,"%f", &Ra.deg_begin);
		fscanf(fin2,"%f", &Dec.deg_begin);
		fclose(fin2);
		sprintf(temp_string,"Reference position: RA(deg)=%10.5f,  DEC(deg)=%10.5f", 
				Ra.deg_begin,Dec.deg_begin);
	}
	else
	{
		sprintf(temp_string,"Current position: RA(hour)=%10.5f,  DEC(deg)=%10.5f", 
				Current_pos.ra,Current_pos.dec);
	}
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

// make directory for images
	// Get current time
	now= getSystemTime();
	sprintf(date_string, "%4d%02d%02d", now.yr, now.mon, now.day);
	
	sprintf(command,"mkdir -p /home/observer/star/image/%s", date_string);
	system(command);

	sprintf(temp_string,"Images are saved in: /home/observer/star/image/%s",date_string);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

//////////////////////////////////////////////////////////////////
//  read input file
//////////////////////////////////////////////////////////////////
	if((fin1=fopen(TIME_TABLE_FILENAME,"r"))==NULL)
	{
		sprintf(temp_string, "ERROR: Could not find input file (%s).",TIME_TABLE_FILENAME);
		steplog(temp_string,STAR_TRACK_LOG_TYPE);
		return 1;
	}

// skip header
	while(!feof(fin1))
	{
		fgets(buf, BUFFER_SIZE, fin1);
		if( !strncmp(buf,"DATA",4) )
					break;
	}
	//After DATA read 3 more lines
	fgets(buf, BUFFER_SIZE, fin1);
	fgets(buf, BUFFER_SIZE, fin1);
	fgets(buf, BUFFER_SIZE, fin1);
// begin to read data

	if_time_flag=0;
// 	fposition = (fpos_t *) malloc (sizeof(fpos_t));
	while(fgets(buf, BUFFER_SIZE, fin1))
	{
// 		fgetpos( fin1, fposition);
		
		i=sscanf(buf,"%*s %s %s %s %s %*s %s %*s %s",
				open_string, close_string,
				ra_string,dec_string,
				filter_string, target_name);
		
		if(i<6)continue; //not all the input are read
		
//  convert long string into yr, mon, day, hr, min
		open = string2date(open_string);
		close = string2date(close_string);
		 
		diff_begin= now.timestamp - open.timestamp;
		diff_end= close.timestamp - now.timestamp;

		if(diff_begin>-5 && diff_end > 3600)
		{
			sprintf(temp_string,"Open Time (UT) = %4d/%02d/%02d %02d:%02d\n", 
						open.yr,open.mon,open.day,open.hr,open.min);
			

			sprintf(temp_string,"%sClose Time (UT) = %4d/%02d/%02d %02d:%02d", 
					temp_string,close.yr, close.mon, close.day, close.hr, close.min);
			steplog(temp_string,STAR_TRACK_LOG_TYPE);

			sscanf(ra_string,"%d:%d:%d",&ra_h,&ra_m,&ra_s);
			sscanf(dec_string,"%d:%d:%d",&dec_d,&dec_m,&dec_s);
			if(dec_d <0){dec_m *= -1;dec_s *= -1;}
			
	//  star coordinates
			StarPos.ra = (float)ra_h + (float)ra_m/60. + (float)ra_s/3600.;
			StarPos.dec= (float)dec_d + (float)dec_m/60. + (float)dec_s/3600.;
			strcpy(p_tat_info->obs_info.target_name,target_name);
			
			sprintf(temp_string,"Target's name: %s\nCoordinates:  RA = %2dh %2dm %2ds  DEC = %3dd %3dm %3ds", 
						target_name,ra_h, ra_m, ra_s, dec_d, dec_m, dec_s);
			steplog(temp_string,STAR_TRACK_LOG_TYPE);

// Filter sequence and exposure time
			//get the information of the filters
			N_filters = read_filter_string(filter_string,filter_seq,filter_exp_time,filter_obs_time);
			if(!N_filters)
			{
				sprintf(temp_string,"ERROR: Filter sequence has problems %s",filter_string);
				steplog(temp_string,STAR_TRACK_LOG_TYPE);
				return 1;
			}

			sprintf(temp_string,"Number of filters = %d",N_filters);
			
			p_tat_info->obs_info.N_filters = N_filters;
			cycle_time =0;
			for(i=0; i< p_tat_info->obs_info.N_filters;i++)
			{
				p_tat_info->obs_info.filter_seq[i]= filter_seq[i];
				p_tat_info->obs_info.filter_exp_time[i]= filter_exp_time[i];
				p_tat_info->obs_info.filter_obs_time[i]= filter_obs_time[i];
				cycle_time += filter_obs_time[i];

				sprintf(temp_string,
						"%s\n%d: Filter type %d, exposure time %d, cycle time %d",
						temp_string,i+1,p_tat_info->obs_info.filter_seq[i],
						p_tat_info->obs_info.filter_exp_time[i],
						p_tat_info->obs_info.filter_obs_time[i]);
			}
			steplog(temp_string,STAR_TRACK_LOG_TYPE);
// total observing time
			total_obs_time=diff_end;
			cycle_number = total_obs_time/ cycle_time;
			
			sprintf(temp_string,
				"Observing Time=%6d s; Cycle Time = %3d s;  No. of Cycles = %3d;",
				total_obs_time,cycle_time, cycle_number);
			steplog(temp_string,STAR_TRACK_LOG_TYPE);

			p_tat_info->obs_info.cycle_time=cycle_time;
			p_tat_info->obs_info.cycle_number=cycle_number;
		
		  
// check star's coordinates
			if(StarPos.dec < Min_dec|| StarPos.dec > Max_dec)
			{
				sprintf(temp_string, "ERROR: DEC out of range (%d, %d)! Observation stops!",
							Min_dec,Max_dec); 
				steplog(temp_string,STAR_TRACK_LOG_TYPE);
				return 1; 
			}

			steplog("****************************",STAR_TRACK_LOG_TYPE);
			
			if_time_flag=1;
			//Set D
//			fsetpos(fin1, fposition);
//			fseek(fin1, 0, SEEK_CUR);
//			fputc('D', fin1);
			break;
		}
	}
// 	free( fposition);
	fclose(fin1);

	if(if_time_flag == 0)
	{
		sprintf(temp_string,"ERROR: It is not observing time!\nCheck \"%s\"",
					TIME_TABLE_FILENAME);
		steplog(temp_string,STAR_TRACK_LOG_TYPE);
		return 1;
	}
////////////////////////////////////////////////////////////////////////
	//	Starting to move telescope from origin to star
////////////////////////////////////////////////////////////////////////

//Calculate degrees telescope should move to point to object
// get system (local) time
	lt= getSystemTime();

/* Compute the position at a delay of DELAY_COMPUTE seconds;
DSP command will also be sent out at this time to allow computer
to have time process following procedure */

	lt.sec += DELAY_COMPUTE;
//	printf("0 Compute position for next second(YYYY/MM/DD hr:min:sec)=%4d/%2d/%2d %2d:%2d:%2d\n",  lt.yr, lt.mon, lt.day, lt.hr, lt.min, lt.sec);

	//current time in seconds, will be used to synchronize the deay of DELAY_COMPUTE seconds in timing
	time(&curtime);

	IsObservable = mv(lt, StarPos,Current_pos, &Ra.deg0_mv,&Dec.deg0_mv);
//	If IsObservable=0, star not observanle; if 1, observable.

	if(!IsObservable)//NOT OBSERVABLE
	{
		steplog("ERROR: Object is NOT OBSERVABLE!",STAR_TRACK_LOG_TYPE);
		return 1;  
	}

//	if(Ra.deg0_mv <= 0. || Dec.deg0_mv <= 0.)
//	{
//		sprintf(temp_string,
//				"ERROR: Object out of range! RA_mv=%8.2f, DEC_mv=%8.2f", 
//				Ra.deg0_mv, Dec.deg0_mv);
//		steplog(temp_string,STAR_TRACK_LOG_TYPE);
//		return 1;  
//	}

	//Report
	sprintf(temp_string,"Star_track: RA %8.2f DEC %8.2f Obs = %s",
			Ra.deg0_mv, Dec.deg0_mv, filter_string);
	log_this(temp_string,AUTO_REPORT_LOG_TYPE,0);
	
	if(Ra.deg0_mv<0) Ra.mv_sign = -1.0;
	else Ra.mv_sign =1.0;
	
	if(Dec.deg0_mv<0) Dec.mv_sign = -1.0;
	else Dec.mv_sign =1.0;
	
	Ra.deg0_mv *= Ra.mv_sign;
	Dec.deg0_mv *= Dec.mv_sign;
	
	sprintf(Ra.dir,"%s",Ra.mv_sign>0?"POS":"NEG");
	sprintf(Dec.dir,"%s",Dec.mv_sign>0?"POS":"NEG");
	
	shutter_flag=1; //1: open shutter; 0: shutter is closed
	
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

	sprintf(temp_string,"Motor Speed 1 (Hz): RA= %8.2f; DEC= %8.2f\n", Ra.motor_speed1, Dec.motor_speed1);
	sprintf(temp_string,"%sMotor Speed 2 (Hz): RA= %8.2f; DEC= %8.2f\n",temp_string, Ra.motor_speed2, Dec.motor_speed2);
	sprintf(temp_string,"%sMotor Speed 3 (Hz): RA= %8.2f",temp_string,Ra.motor_speed3f);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

	sprintf(temp_string, 
			"Distance from Reference (deg): RA= %7.2f; DEC= %7.2f", 
			Ra.deg0_mv, Dec.deg0_mv); 
	steplog(temp_string,STAR_TRACK_LOG_TYPE);
	
//Compute time required to move from origin to object's current position
	Ra.time_mv2_f = (float)( Ra.deg0_mv*DTOP_RA - FAST_SPEED_TIME1* Ra.motor_speed1)/ Ra.motor_speed2;
	Dec.time_mv2_f= (float)(Dec.deg0_mv*DTOP_DEC- FAST_SPEED_TIME1*Dec.motor_speed1)/Dec.motor_speed2;

	if(Ra.time_mv2_f < 0. || Dec.time_mv2_f < 0. )
	{
		steplog("ERROR: 2nd-stage time is negative!",STAR_TRACK_LOG_TYPE);
		return 1;
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
	sprintf(temp_string,
			" RA: 1st stage= 10 s; 2nd stage=%4d s; 3rd stage=%4d s; total=%5d s", 
			Ra.time_mv2, Ra.time_mv3, Ra.actual_time_mv);
	sprintf(temp_string,
			"%s\nDec: 1st stage= 10 s; 2nd stage=%4d s; total=%5d s",temp_string, 
			Dec.time_mv2, Dec.actual_time_mv);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

/**************************************************************************/
//	compute actual number of pulses
	RaActualPulse = Ra.deg0_mv*DTOP_RA + SIDEREALSPEED*Ra.actual_time_mv;
	DecActualPulse = Dec.deg0_mv*DTOP_DEC;

	sprintf(temp_string,
			"RA actual pulse:      %12.1f, DEC actual pulse:      %12.1f",
			RaActualPulse,	DecActualPulse);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

//  send out DSP command to restrict the number of pulses
	sprintf(Ra.cmd ,"RA POS LIMIT 0 0 0 0 0 0 0\nRA POS LIMIT 1 0 %d 0 0 0 0\n", (int)RaActualPulse);
	sprintf(Dec.cmd ,"DEC POS LIMIT 0 0 0 0 0 0 0\nDEC POS LIMIT 1 0 %d 0 0 0 0\n", (int)DecActualPulse);
#ifndef DEBUG
	fout=fopen(DSP_CMD_FILENAME, "w");
// both RA and DEC pulse numbers are restricted.
	fprintf(fout, "%s\n%s\n",Ra.cmd,Dec.cmd);
	fclose(fout);
#endif
	sprintf(temp_string, "Ra.cmd=%s, Dec.cmd=%s", Ra.cmd, Dec.cmd);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);
/**************************************************************************/

	Ra.start_time_mv= Dec.start_time_mv= 1;
	// First Stage
	//Move RA/DEC motor with 500Hz for 10sec
	//BUFFERTIME is added to avoid gap between two different speeds
	sprintf(Ra.cmd, "RA %s FREQUENCY %d %d %d 0 0 0 0",Ra.dir,Ra.start_time_mv, FAST_SPEED_TIME1+BUFFERTIME, FAST_SPEED_REGISTER1);
	sprintf(Dec.cmd, "DEC %s FREQUENCY %d %d %d 0 0 0 0",Dec.dir,Dec.start_time_mv, FAST_SPEED_TIME1+BUFFERTIME, FAST_SPEED_REGISTER1);

	sprintf(temp_string, 
		"Waiting for %d sec, until %4d/%02d/%02d %02d:%02d:%02d, to send commands to DSP", 
		DELAY_COMPUTE,lt.yr, lt.mon, lt.day, lt.hr, lt.min, lt.sec);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);
#ifndef DEBUG
	// synchronize with the time at which variables are computed
	short_timing(curtime+ DELAY_COMPUTE);
#endif
	p_tat_info->obs_info.status = Pursuing;
	//Send commands to DSP
#ifndef DEBUG
	fout=fopen(DSP_CMD_FILENAME, "w");
	fprintf(fout, "%s\n%s\n", Ra.cmd, Dec.cmd);
	fclose(fout);
#endif
	sprintf(temp_string, "Ra.cmd=%s, Dec.cmd=%s", Ra.cmd, Dec.cmd);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);


	// Second Stage
	//Move RA/DEC motor with 2kHz for Ra.time_mv2 and Dec.time_mv2
	sprintf(Ra.cmd ,"RA %s FREQUENCY %d %d %d 0 0 0 0",Ra.dir,Ra.start_time_mv, Ra.time_mv2+BUFFERTIME, FAST_SPEED_REGISTER2);
	//BUFFERTIME is NOT added for DEC since it stops at the end of 2nd stage, instead BUFFERTIME_2 is added to make sure DEC has enough pulse number
	sprintf(Dec.cmd ,"DEC %s FREQUENCY %d %d %d 0 0 0 0",Dec.dir, Dec.start_time_mv, Dec.time_mv2+BUFFERTIME_2, FAST_SPEED_REGISTER2);
#ifndef DEBUG
	// synchronize with the time at which variables are computed
	short_timing(curtime+ DELAY_COMPUTE + FAST_SPEED_TIME1);

	//Send commands to DSP
	fout=fopen(DSP_CMD_FILENAME, "w");
	fprintf(fout, "%s\n%s\n", Ra.cmd, Dec.cmd);
	fclose(fout);
#endif
	sprintf(temp_string, "Ra.cmd=%s, Dec.cmd=%s", Ra.cmd, Dec.cmd);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

	// Third Stage
	//Move RA motor with 1KHz for Ra.time_mv3
	sprintf(Ra.cmd ,"RA POS FREQUENCY %d %d %d 0 0 0 0", Ra.start_time_mv, Ra.time_mv3+BUFFERTIME, FAST_SPEED_REGISTER3F);
#ifndef DEBUG
	// synchronize with the time at which variables are computed
	short_timing(curtime+ DELAY_COMPUTE + FAST_SPEED_TIME1 + Ra.time_mv2);
	//Send commands to DSP
	fout=fopen(DSP_CMD_FILENAME, "w");
	fprintf(fout, "%s\n", Ra.cmd);
	fclose(fout);
#endif
	sprintf(temp_string, "Ra.cmd=%s", Ra.cmd); 
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

/*************************************************************************/
//  end of moving telescope from origin to star
/*************************************************************************/

	//waits till RA moving time is finish.
        // synchronize with the time at which variables are computed
        // BUFFERTIME_1 is added to make sure the pulse number is enough
#ifndef DEBUG
	short_timing(curtime + DELAY_COMPUTE + Ra.actual_time_mv + BUFFERTIME_1);
#endif
	steplog( "Telescope has pointed to the star",STAR_TRACK_LOG_TYPE);
	
	p_tat_info->obs_info.status = Tracking;

/*************************************************************************/
/*  DEC will stop after fastmove.  RA will move with a speed  */
/*************************************************************************/

	Ra.star_speed_register = (int)( 0.5 + ( 1./ (2*10*120.0e-9*SIDEREALSPEED) ) );

	sprintf(Ra.cmd ,"RA POS FREQUENCY 1 36030 %d 0 0 0 0",  Ra.star_speed_register);
#ifndef DEBUG
	fout=fopen(DSP_CMD_FILENAME, "w");
	fprintf(fout, "%s\n ", Ra.cmd);
	fclose(fout);
#endif
	
	sprintf(temp_string, "Ra.cmd=%s", Ra.cmd);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

	printf("Wait 10 sec for telescope motion to stabilize");
// Wait for telescope motion to stabilize
// If DEC takes longer time than RA, it needs to wait until DEC finishes.
#ifndef DEBUG
	if(Dec.actual_time_mv > Ra.actual_time_mv)
	{
		printf("\nDEC is longer than RA by %5d,  waiting....\n",
		Dec.actual_time_mv - Ra.actual_time_mv);
		sleep(10 + (Dec.actual_time_mv - Ra.actual_time_mv) );
	}
	else
	  sleep(10);
#endif
	  
	//  send out DSP command stop restricting the number of pulses
	sprintf(Ra.cmd ,"RA POS LIMIT 2 0 0 0 0 0 0\n");
	sprintf(Dec.cmd ,"DEC POS LIMIT 2 0 0 0 0 0 0\n");
#ifndef DEBUG
	fout=fopen(DSP_CMD_FILENAME, "w");
	fprintf(fout, "%s\n%s\n", Ra.cmd, Dec.cmd);
	fclose(fout);
#endif
	sprintf(temp_string, "Ra.cmd=%s\nDec.cmd=%s", Ra.cmd, Dec.cmd);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);
	
/*************************************************************************/
//   beginning of taking the first image
/*************************************************************************/
//send again sidereal speed
	sprintf(Ra.cmd ,"RA POS FREQUENCY 1 36030 %d 0 0 0 0",  Ra.star_speed_register);
#ifndef DEBUG
	fout=fopen(DSP_CMD_FILENAME, "w");
	fprintf(fout, "%s\n ", Ra.cmd);
	fclose(fout);
#endif
	sprintf(temp_string, "Ra.cmd=%s", Ra.cmd);
	steplog(temp_string,STAR_TRACK_LOG_TYPE);

	p_tat_info->obs_info.ccd_status = CCD_IMAGE;
	current_seq_step =0;
	// Now, current position is suppose to be Star position
	Current_pos.ra = StarPos.ra;
	Current_pos.dec = StarPos.dec;
	p_tat_info->obs_info.FOV = FOV_TBC;
	
	////////////////////////////////////////////////////////
	////////////EXPOSING //////////////////////
	///////////////////////////////////////////////////////////
	for(cycle=1,image_counter=1;cycle<=cycle_number; image_counter++)
	{
		//First move to filter
		if(p_tat_info->obs_info.filter_seq[current_seq_step] != p_tat_info->obs_info.current_filter)
		{
			move_to_filter(p_tat_info->obs_info.filter_seq[current_seq_step]);
			sprintf(temp_string, "Moving to filter %d", p_tat_info->obs_info.filter_seq[current_seq_step]);
			steplog(temp_string,STAR_TRACK_LOG_TYPE);
			sleep(2);
			while(p_tat_info->fli_info.wheel_moving)sleep(1);
		}
		
		//Compute exposure time
		p_tat_info->obs_info.expose_time  = (float) p_tat_info->obs_info.filter_exp_time[current_seq_step];
		p_tat_info->obs_info.cycle_time = p_tat_info->obs_info.filter_obs_time[current_seq_step];
		filter_type = p_tat_info->obs_info.filter_type;

		exposure_time = p_tat_info->obs_info.expose_time;
		
		sprintf(temp_string,"/////////////\n");
		sprintf(temp_string,"%sCycle: %d, Image: %5d, Filter: %c", temp_string,cycle,image_counter,filter_type);
		steplog(temp_string,STAR_TRACK_LOG_TYPE);
		
// give the image a filename
		lt= getSystemTime();
		sprintf(image_filename,"%s/%s/%cStar%s%4d%02d%02d_%02d%02d%02d.fit",IMAGE_PATH,
					date_string, filter_type, SITE,lt.yr, lt.mon, lt.day, lt.hr, lt.min, lt.sec);

		sprintf(temp_string,"Image file: %s", image_filename);
		steplog(temp_string,STAR_TRACK_LOG_TYPE);
		sprintf(temp_string,"Star_track image: %s", image_filename);
		log_this(temp_string,AUTO_REPORT_LOG_TYPE,0);
		
	  // creates file for ccd camera daemon to read
		shutter_flag=1; //1: open shutter; 0: shutter is closed
		sprintf(camera_command, "camera takeimage %s %.0f %d 1",
					image_filename, exposure_time, shutter_flag);
		 
		steplog(camera_command,STAR_TRACK_LOG_TYPE);
		
#ifndef DEBUG
		if((camera_fn=fopen(CCD_CMD_FILENAME,"w"))==NULL)
		{
			sprintf(temp_string, "ERROR: Can not open %s",CCD_CMD_FILENAME);
			steplog(temp_string,STAR_TRACK_LOG_TYPE);
		}
		else
		{
			fprintf(camera_fn,"%s\n", camera_command);
			fclose(camera_fn);
		}
#endif
		//Now waiting
		time(&curtime);
		delay = exposure_time;
		sprintf(temp_string,"Waiting for %3d sec", delay);
		step(temp_string);
#ifndef DEBUG
		short_timing(curtime+delay);
#endif
		delay = CCD_READOUT_TIME;
		time(&curtime);
#ifndef DEBUG
		short_timing(curtime+delay);
#endif
		if(p_tat_info->obs_info.filter_seq[current_seq_step]==0)
		{//only adjust for filter N  (filter_pos == 0)
			////*****////////ADJUST FOV//////******///////
// 			if( i>=2 )
			//Conditions to initiate the astrometry program
			strcpy(str1,p_tat_info->obs_info.recent_image);
			//
			printf("Sending command for astrometry with RA = %f and DEC = %f\n",Current_pos.ra,Current_pos.dec);
			
			temp=get_RA_DEC_astrometry(str1,&Current_pos.ra,&Current_pos.dec);
			if(!temp)
			{
				printf("Coordinates of %s are:\n",str1);
// 				printf("RA = %f\tDEC = %f\n",Current_pos.ra,Current_pos.dec);
				
				diff_ra = (int) ((Current_pos.ra - StarPos.ra)*15*DTOP_RA); //difference in pulses
				diff_dec = (int) ((StarPos.dec - Current_pos.dec)*DTOP_DEC);
				
				sprintf(temp_string,"Image ra = %f, dec = %f\t diff_ra = %d, diff_dec = %d\n",
					Current_pos.ra,Current_pos.dec,diff_ra,diff_dec);
				
				if(p_tat_info->obs_info.FOV == FOV_TBC)
				{
					if( abs(diff_ra) > 100 || abs(diff_dec) > 50)
					{//If difference too big repeat
						sprintf(temp_string,"%s => FOV is TBC",temp_string);
						p_tat_info->obs_info.FOV = FOV_TBC; //
					}
					else
					{
						sprintf(temp_string,"%s => FOV is CORRECT",temp_string);
						p_tat_info->obs_info.FOV = FOV_CORRECT;
					}
				}
				else if(p_tat_info->obs_info.FOV == FOV_CORRECT)
				{
					if( abs(diff_ra) > 100 || abs(diff_dec) > 50)
					{//If difference too big don't move (most likely astronetry is wrong
						sprintf(temp_string,"%s Difference too big. Don't move and set FOV=TBC",temp_string);
						p_tat_info->obs_info.FOV = FOV_TBC; 
						diff_ra =diff_dec =0;
					}
				}
				steplog(temp_string,STAR_TRACK_LOG_TYPE);
				
				
				// move the telescope if more than 5 pixels difference, reset other whie
			
				if( abs(diff_ra) > 0 || abs(diff_dec) > 0) 
				{
					move_pulse(diff_ra,diff_dec);
				}
			}
			
		}
		
		// Change filter only if FOV is correct
		if(p_tat_info->obs_info.N_filters>=2 && p_tat_info->obs_info.FOV == FOV_CORRECT)
		{
			current_seq_step++;
			if(current_seq_step >=p_tat_info->obs_info.N_filters)
			{
				cycle++;
				current_seq_step=0;
			}
		}
		else cycle++;
		//In case observer change sequence and reduce N_filter
		if(p_tat_info->obs_info.N_filters <2) current_seq_step=0;
	}
	p_tat_info->obs_info.ccd_status=CCD_IDLE;
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

int mv(DATE d, STARLIST star_pos, STARLIST Current_pos, float *ra_deg_mv, float *dec_deg_mv)
{
	extern	struct TrackingMotor	Ra, Dec;
	char temp_string[300];
	DATE ut, lt;//Universal and Local Time
	double LST; //Local Sidereal Time, in hours
	float hourAngle_obj, dec_deg_obj;
	float ra_deg1_mv, dec_deg1_mv;
	int objectStatus=0;	//Object is NOT Observable
	
// convert given local time to UT
	ut= convert2UT(d);
//	printf("mv:UT (YYYY/MM/DD hr:min:sec)=%4d/%2d/%2d %2d:%2d:%2d\n",
//	        ut.yr, ut.mon, ut.day, ut.hr, ut.min, ut.sec);

// compute local sidereal time
	LST= getLocalSidereal(ut);
	sprintf(temp_string,"Local Sidereal Time = %10.6f (hr)", LST);
	step(temp_string);

/*
 If Local Sidereal Time(LST) equals the Right Ascension(RA) of the star,
 the star is in zenith.
 Hour angle of star is computed as:
        HA(star) = LST(observer) - RA(star)
 If LST< RA (HA<0), the star has not past observer's zenith.
 If HA<-6, the star has not risen yet.
 If HA> 6, the star has already set.
 Usually the range of HA is (0, 24hr). Here we set the range of RA
 to be (-12hr, 12hr).
*/
	objectStatus=0;	//Object is NOT OBSERVABLE
// compute the hour angle of the object:
	hourAngle_obj	= LST - star_pos.ra;
	dec_deg_obj	= star_pos.dec;
// set the range of HA to (-12, 12)
	while (hourAngle_obj < -12.0) hourAngle_obj +=24.0;
	while (hourAngle_obj >  12.0) hourAngle_obj -=24.0;
	if(hourAngle_obj < -12.0 || hourAngle_obj > 12.0)
	{
		sprintf(temp_string,"ERROR: Hour angle is out of range; HA=%16.6f", hourAngle_obj);
		step(temp_string);
		return 0;
	}
	sprintf(temp_string,"HourAngle= %12.6f, Dec=%12.6f", hourAngle_obj, dec_deg_obj);
	step(temp_string);
	
	if(hourAngle_obj < MIN_ALTITUDE) 
	{
		sprintf(temp_string,"ERROR: HA = %f; Star has not risen yet! (<%.2fhr)", hourAngle_obj,MIN_ALTITUDE);
		step(temp_string);
	}
	else if(hourAngle_obj >  5.0) // elevation 22 deg 
	{
		sprintf(temp_string,"ERROR: HA = %f; Star has already set! (>5.3hr)", hourAngle_obj);
		step(temp_string);
	}
	else
	{
		objectStatus=1;	//Object is OBSERVABLE
          // computes total degree which telescope should move
	  // ra_deg_begin is the degree needed to move the Ra of telescope pointing to zenith(transit)
	  // dec_deg_begin is the degree needed to move the Dec of telescope pointing up from observer latitude
//	  ra_deg1_mv	= hourAngle_obj*15. + RA_DEG_BEGIN;
//	  dec_deg1_mv	= (dec_deg_obj - LOCAL_LAT) + DEC_DEG_BEGIN;
		if(Current_pos.ra <0) //Current position is home switch
		{
			ra_deg1_mv= hourAngle_obj*15. + Ra.deg_begin;
			dec_deg1_mv= (dec_deg_obj - LOCAL_LAT) + Dec.deg_begin;
			
			if(ra_deg1_mv <0 || dec_deg1_mv <0)
			{
				step("We got negative movement from origin. Check input coordinates");
				return 0;
			}
		}
		else
		{
			ra_deg1_mv = Current_pos.ra -  star_pos.ra; //in hours
			dec_deg1_mv = star_pos.dec - Current_pos.dec; //in deg
			
			if(ra_deg1_mv > 9.0) ra_deg1_mv -= 24.0;
			else if(ra_deg1_mv < -9.0) ra_deg1_mv += 24.0;
			
			ra_deg1_mv *= 15.0; //hours to deg
		}
		// make sure it is between 0- 360
		*ra_deg_mv= fmod(ra_deg1_mv , 360);
		*dec_deg_mv= fmod(dec_deg1_mv, 360);
	}
	return objectStatus;
}

/**********************************************************/
/*Subroutine short_timing(long waitTillTime)		          */
/**********************************************************/
/*    Function: wait until certain time
      Input:	time in second to wait to 		  */
/**********************************************************/
void short_timing(long wait2Time)
{
	time_t curTime;
	int diffTime;
	while(1)
	{
		time(&curTime);
		//sleep if idle time is too long to save CPU cycle
		if( ( diffTime =(wait2Time- curTime) ) > 3)
		{
			printf("\rCountdown %4d, ",  diffTime);fflush(NULL);
			sleep(1);
		}
		else
		{
			if(curTime>=wait2Time) {printf("\n");break;}
			usleep(10);
		}
	}
}


