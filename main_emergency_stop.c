#include <stdio.h>
#include <stdlib.h>
#include "symblc_const.h"
#include "common_func.h"
#include "dsp_func.h"
#include "ppc_func.h"
#include "ccd_func.h"
#include "open_close_enc.h"
#include "Apogee.h"
#include "myAstro.h"

st_tat_info *p_tat_info;

#define CCD_OFF 0
#define CCD_ON 1
#define MOVE_TO_NEXT_OBS 2

int main(int argc,char* argv[])
{
	FILE *cmdFile;
	int i,input_option;
	
	if(argc != 2)
	{
		printf("##################DO NOT FORGET #########################\n");
		printf("Usage: %s [0 or 1 or 2]\n",argv[0]);
		printf("0: Stop automatic observation and turn off CCD\n");
		printf("1: Stop automatic observations but do not turn off CCD\n");
		printf("2: Do not stop, but force automatic observation to observe the next scheduled target\n");
		printf("#########################################################\n");
		return 1;
	}
	
	input_option = atoi(argv[1]);

	if(input_option <0 || input_option > 2)
	{
		printf("ERROR: Option must be 0, 1 or 2\n");
		return 1;
	}
	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
	
	
	
	cmdFile=fopen(AUTO_CMD_FILENAME,"w");
	
	if(input_option == MOVE_TO_NEXT_OBS)
		fprintf(cmdFile,"%s",DO_NEXT_OBSERVATION);
	else
		fprintf(cmdFile,"%s",BREAK_ALL_OBSERVATION);
	
	fclose(cmdFile);
	
	if(input_option == MOVE_TO_NEXT_OBS) 
	{
		step("### Moving to the next observations");
		return 0; //Just send a message
	}
	
	step("WARNING: Emergency stop observation now !!");
	if(input_option == CCD_OFF)
		printf("## CCD will not be turned off ##\n");
	else if(input_option == CCD_OFF)
		printf("## CCD will be turned off ##\n");
	
	sleep(2); /* wait for auto-observe*/
	p_tat_info->obs_info.ccd_status = CCD_IDLE;
	p_tat_info->obs_info.auto_observing = 0;
	
	
	//Turn web cams ON
	pwr_rx_poweron();
    /* Often CCD is exposing when sending this command.
       So, The command may be missed. */
	if(!input_option)
	{
		steplog("CCD cooler warm up",AUTO_OBSERVE_LOG_TYPE);
		ccd_cooler_shutdown();
	}
	//Move 6 degree in case the telescope is in origin
    steplog("Forward Telescope ~3 Degree", AUTO_OBSERVE_LOG_TYPE);
    ForwardTelescope3Degree();
    /* Telescope will stop after ForwardTelescope3Degree() */

    p_tat_info->obs_info.status = Returning;
    steplog("Move Telescope to Reference Position", AUTO_OBSERVE_LOG_TYPE);
    RapidResetTelescope();
    /* timing in ParkTelescope() */
    if ( p_tat_info->dsp_info.ra.origin && p_tat_info->dsp_info.dec.origin )
   	      ClosePart();
    else
    {
		steplog("WARNING: Cannot move Telescope to safe position", AUTO_OBSERVE_LOG_TYPE);
		printf("Please move Telescope to safe position by hand.\n");
		printf("And then close enclosure by youself.");
		exit(1);
    }

    steplog("Move Telescope to Safe Position", AUTO_OBSERVE_LOG_TYPE);
    ForwardTelescope3Degree();
	
	steplog("Turn part power off", AUTO_OBSERVE_LOG_TYPE);
	TurnPartPowerOff();
	
    /* try */
	if(!input_option)
	{
		p_tat_info->obs_info.status = STOP;
		steplog("CCD cooler warm up", AUTO_OBSERVE_LOG_TYPE);
		for( i=0;i<40;i++)
		{
			ccd_cooler_shutdown();
			sleep(60);
#ifdef AP6
			if( p_tat_info->ccd_info.cooler_status == 4 || //"AtAmbient
				p_tat_info->ccd_info.cooler_status==6 || //AtMin
				p_tat_info->ccd_info.cooler_status ==8) //cooler on first
				break;
			else
				printf("After %d seconds the temperature of CCD = %-7.5f\n",i*60,p_tat_info->ccd_info.curr_point);
#elif defined U6
			if(p_tat_info->ccd_info.cooler_status == Apn_CoolerStatus_AtSetPoint ) //cooler shutdown
					break;
			else if(p_tat_info->ccd_info.curr_point >= p_tat_info->ccd_info.set_point) 
				break;
			else
				printf("After %d seconds the temperature of CCD = %-7.5f\n",i*60,p_tat_info->ccd_info.curr_point);
#elif defined F6
			if(p_tat_info->ccd_info.cooler_status == Apn_CoolerStatus_AtSetPoint ) //cooler shutdown
					break;
			else if(p_tat_info->ccd_info.curr_point >= p_tat_info->ccd_info.set_point) 
				break;
			else
				printf("After %d seconds the temperature of CCD = %-7.5f\n",i*60,p_tat_info->ccd_info.curr_point);
#endif
		}
		steplog("Turn cooler OFF", AUTO_OBSERVE_LOG_TYPE);
		ccd_cooler_off();

		/* wait for ccd cooler off work */
		sleep(5);
		steplog("### Turn CCD OFF and END", AUTO_OBSERVE_LOG_TYPE);
		TurnCcdMainPowerOff();
	}
    return 0;
}
