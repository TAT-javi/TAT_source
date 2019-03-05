#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ppcdaemon.h"
#include "dsp_func.h"
#include "symblc_const.h"
#include "pwrdaemon.h"

st_tat_info    *p_tat_info;

void draw_obs_status( st_obs_info* p_object);
void draw_tel_status( st_dsp_info *p_object);
void draw_pwr_status( st_ppc_info* p_object);
void draw_ppc_status( st_ppc_info* p_object);
void draw_enc_status( st_dsp_info* p_object);
void draw_ccd_status( st_ccd_info* p_object);
void draw_fli_status( st_fli_info* p_object);


int main(int argc, char *argv[])
{
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);

	if(argc!=2)
	{
		printf("### USAGE ###\n");
		printf(" %s [Option]\n",argv[0]);
		printf("###\nOptions:\n a = print all the shm\n");
		printf(" o = output 1 if auto_observation is ON\n###\n");
	}
	
	
	if(!strcmp(argv[1],"a"))
	{
		printf("### Shared Memory of TAT ###\n");
		draw_obs_status(&(p_tat_info->obs_info));
		draw_tel_status(&(p_tat_info->dsp_info));
		draw_ccd_status(&(p_tat_info->ccd_info));
		draw_fli_status(&(p_tat_info->fli_info));
		draw_enc_status(&(p_tat_info->dsp_info));
		draw_ppc_status(&(p_tat_info->ppc_info));
		draw_pwr_status(&(p_tat_info->pwr_info));
		printf("### END OF PROGRAM ###\n");
	}
	else if(!strcmp(argv[1],"o"))
	{
		printf("%d",p_tat_info->obs_info.auto_observing);
	}
	
	return 0;
}


void draw_obs_status( st_obs_info* p_object)
{
	int i,j;
	struct tm tm_time;
	char time_string[17];
	char temp_string[200],filter_string[100];
// 	time_t	now;
	char filters[FILTER_TOTAL_NUMBER];
	
	get_filter_array_char(filters);
	
	
		
	printf( "# Auto Observation Info\nAuto Observing - %3s\n",p_object->auto_observing?"YES":"NO");
	switch( p_object->status )
	{
		case Pursuing:
			sprintf( temp_string, "Pursuing");
		break;
		case Tracking:
			sprintf( temp_string, "Tracking");
		break;
		case Returning:
			sprintf( temp_string, "Returning");
		break;
		case STOP:
			sprintf( temp_string, "STOP");
		break;
		case InOrigin:
			sprintf( temp_string, "InOrigin");
		break;
		case Moving:
			sprintf( temp_string, "Moving");
		break;
		case Flating:
			sprintf( temp_string, "Flating   ");
		break;
		case Darking:
			sprintf( temp_string, "Darking   ");
		break;

		default:
			sprintf( temp_string, "N.A.");

		break;
	}
	printf( "Status - %s\n", temp_string);

// 	time(&now);
// 	gmtime_r(&now, &tm_time);
	
	gmtime_r(&(p_object->begin_time), &tm_time);
	strftime( time_string, 17, "20%y/%m/%d %H:%M", &tm_time);
	printf( "Begin Time - %15s", time_string);

	gmtime_r(&(p_object->end_time), &tm_time);
	strftime( time_string, 17, "20%y/%m/%d %H:%M", &tm_time);
	printf( "End Time - %15s", time_string);

	printf( "Curr. Expose Time - %.0f",p_object->expose_time);
	printf( "Curr. Filter - %c", p_object->filter_type);
	
	sprintf(filter_string,"");
	for(i=0;i<p_object->N_filters;i++)
	{
		j = p_object->filter_seq[i];
		sprintf(filter_string,"%s %c(%d)",
				filter_string,filters[j],p_object->filter_exp_time[i]);
	}
	printf( "Filter Sequence - %15s\n",filter_string);
	
	printf("Target name - %s\n",p_object->target_name);
	printf("RA = %.04f\tDEC = %.4f",p_object->RA,p_object->DEC);
}


void draw_tel_status( st_dsp_info *p_object)
{
	static char *msg[]=	{
				 "NA",
				 /* 1     2 */
				 "POS","NEG",
				 /*    3          4      5        6            7 */
				 "RETURN","InOrigin","STOP","MOVING","NearOrigin",
				 /*    8          9      10 */
				 "DISABLE", "ENABLE", "DONE",
				 /*11   12  */
			    	 "NO","YES",0
			    	 };
	static char option;


	printf( "# DSP Info\nDSP Time Count - %6d\n",p_object->time_count);

        /* RA */
    printf( "RA Reg - %6d | ",p_object->ra.reg);
    printf( "Pulse - %8d | ", p_object->ra.pulse);

	switch( p_object->ra.dir )
	{
		case 1:
			option=1;
			break;
		case 0:
			option=2;
			break;
		default:
			option=0;
	}
	printf( "Dir - %3s | ",msg[option]);

	switch( p_object->ra.status )
	{
		case 10:
			option=3;
			break;
		case 11:
			option=4;
			break;

		case 12:
			option=5;
			break;

		case 13:
			option=6;
			break;
	case 14:
		option=7;
		break;
	default:
		option=0;
	}
	printf( "Status - %10s\n",msg[option]);

	switch( p_object->ra.pulsectl )
	{
		case 0:
			option=8;
			break;
	case 1:
		option=9;
		break;
	case 2:
		option=10;
		break;
	default:
		option=0;
	}
        printf( "RA PulseCtl - %7s | ",msg[option]);
        printf( "MovePulse - %8d | ",p_object->ra.refpulse);
        printf( "Remainder - %8d | ",p_object->ra.purpulse);

	switch( p_object->ra.origin )
	{
		case 0:
			option=11;
			break;
		case 1:
			option=12;
			break;
		default:
			option=0;
	}
	printf( "In Origin - %3s\n",msg[option]);

	/* DEC */
	printf( "DEC Reg - %6d |",p_object->dec.reg);
	printf( "Pulse - %8d | ", p_object->dec.pulse);

	switch( p_object->dec.dir )
	{
		case 1:
			option=1;
			break;
		case 0:
			option=2;
			break;
		default:
			option=0;
	}
	printf( "Dir - %-3s | ",msg[option]);

	switch( p_object->dec.status )
	{
		case 10:
			option=3;
			break;
		case 11:
			option=4;
			break;

		case 12:
			option=5;
			break;

		case 13:
			option=6;
			break;
		case 14:
			option=7;
			break;
		default:
			option=0;
	}
	printf( "Status - %10s\n",msg[option]);

	switch( p_object->dec.pulsectl )
	{
		case 0:
			option=8;
			break;
		case 1:
			option=9;
			break;
		case 2:
			option=10;
			break;
		default:
			option=0;
	}
	printf( "DEC PulsCtl - %7s | ",msg[option]);
	printf( "MovePulse - %8d | ",p_object->dec.refpulse);
	printf( "Remainder - %8d | ",p_object->dec.purpulse);

	switch( p_object->dec.origin )
	{
		case 0:
			option=11;
			break;
		case 1:
			option=12;
			break;
		default:
			option=0;
	}
	printf( "In Origin - %3s\n",msg[option]);
}


void draw_ccd_status( st_ccd_info* p_object)
{
	char temp_string[BUFSIZ];
	printf("# CCD Info");
	printf( "Current Temp - %7.5f\t", p_object->curr_point);
	printf( "Set Temp - %7.5f", p_object->set_point);

// 	#define	Apn_Status_DataError -2
// 	#define	Apn_Status_PatternError	 -1
// 	#define	Apn_Status_Idle	 0
// 	#define	Apn_Status_Exposing  1
// 	#define	Apn_Status_ImagingActive  2
// 	#define	Apn_Status_ImageReady  3
// 	#define	Apn_Status_Flushing  4
// 	#define	Apn_Status_WaitingOnTrigger 5
// 	#define	Apn_Status_ConnectionError 6
	
	
	switch( p_object->camera_status)
	{
		case 0:
			strcpy( temp_string,"Idle");
		break;
		case 1:
			strcpy( temp_string,"Exposing");
		break;
		case 2:
			strcpy( temp_string,"ImagingActive");
		break;
		case 3:
			strcpy( temp_string,"ImageReady");
		break;
		case 4:
			strcpy( temp_string,"Flushing");
		break;
		case 5:
			strcpy( temp_string,"WaitingOnTrigger");
		break;
		case 6:
			strcpy( temp_string,"ConnectionError");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	printf( "CCD Status - %20s\n", temp_string);


// 	#define	Apn_CoolerStatus_Off 0
// 	#define	Apn_CoolerStatus_RampingToSetPoint 1
// 	#define	Apn_CoolerStatus_AtSetPoint 2
// 	#define	Apn_CoolerStatus_Revision 3
	switch( p_object->cooler_status)
	{
		case 0:
			strcpy( temp_string,"Off");
		break;
		case 1:
			strcpy( temp_string,"RampingToSetPoint");
		break;
		case 2:
			strcpy( temp_string,"AtSetPoint");
		break;
		case 3:
			strcpy( temp_string,"Revision");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	printf( "Cooler Status - %20s\t", temp_string);

// 	#define	Apn_FanMode_Off	0
// 	#define	Apn_FanMode_Low 1
// 	#define	Apn_FanMode_Medium 2
// 	#define	Apn_FanMode_High 3
	
	switch( p_object->fan_status)
	{
		case 0:
			strcpy( temp_string, "Off");
		break;
		case 1:
			strcpy( temp_string, "Low");
		break;
		case 2:
			strcpy( temp_string, "Medium");
		break;
		case 3:
			strcpy( temp_string, "High");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	printf( "Mode - %20s\n", temp_string);
	printf( "Recent File: %s\n",
			*(p_tat_info->obs_info.recent_image)?p_tat_info->obs_info.recent_image:"None");
}

void draw_enc_status( st_dsp_info* p_object)
{
	static char temp_string[BUFSIZ];
	printf("# Enclosure Latch Info\n");
	
	if( p_object->enc.open )
		sprintf( temp_string, "Opening");
	else if( p_object->enc.close )
		 sprintf( temp_string, "Closing");
	else
		sprintf( temp_string, "No Action");
	
	printf("Enclosure - %9s\tTime - %3d\n", temp_string,p_object->enc.time);
	printf("Enclosure Closed LS = %d\n",p_object->enc.closed_ls);
	printf("Enclosure opened LS = %d\n",p_object->enc.opened_ls);
	
	if( p_object->latch.lock )
		sprintf( temp_string, "Locking");
	else if( p_object->latch.unlock )
		sprintf( temp_string, "Unlocking");
	else
		sprintf( temp_string, "No Action");

	printf("Latch - %9s\tTime - %3d\n",temp_string,p_object->latch.time);
}

void draw_ppc_status( st_ppc_info* p_object)
{
	printf( "# PPC Info\nMain Power - %3s\n",(p_object->control)&MAINPWRREG?"OFF":"ON");

	printf( "RA Power - %3s\t",(p_object->data)&RAPWRREG?"OFF":"ON");
	printf( "DEC Power - %3s\n",(p_object->data)&DECPWRREG?"OFF":"ON");


	printf( "DSP Power - %3s\t",(p_object->data)&DSPPWRREG?"OFF":"ON");
	printf( "CCD Power - %3s\n",(p_object->data)&CCDPWRREG?"OFF":"ON");

	printf( "Wheel Power - %3s\t",(p_object->data)&WHEELPWRREG?"OFF":"ON");
	printf( "VDC Power - %3s\n",(p_object->control)&VDCPWRREG?"OFF":"ON");

	printf( "DSP Jumper - %3s\n",(p_object->data)&DSPJPREG?"OFF":"ON");

	printf( "Limit Switch Status - %d%d%d%d %d\n",
		p_object->status&RAEASTREG?1:0,
		p_object->status&RAWESTREG?0:1,
		p_object->status&DECSOUTHREG?1:0,
		p_object->status&DECNORTHREG?1:0,
		p_object->status&RADECLIMREG?1:0);
}


void draw_pwr_status( st_ppc_info* p_object)
{
	
	printf(  "# PWR Info\nNo UPS Power - %3s ", (p_object->control)&PIN01REG?"OFF":"ON");
	printf(  "%3s ", (p_object->control)&PIN14REG?"OFF":"ON");
	printf(  "%3s ", (p_object->data)&PIN02REG?"ON":"OFF");
	printf(  "%3s ", (p_object->data)&PIN03REG?"ON":"OFF");
	printf(  "%3s ", (p_object->control)&PIN16REG?"ON":"OFF");
	printf(  "%3s\n", (p_object->data)&PIN04REG?"ON":"OFF");
	
	printf(  "UPS Power - %3s ", (p_object->data)&PIN09REG?"ON":"OFF");
	printf(  "%3s ", (p_object->data)&PIN08REG?"ON":"OFF");
	printf(  "%3s ", (p_object->data)&PIN07REG?"ON":"OFF");
	printf(  "%3s ", (p_object->data)&PIN06REG?"ON":"OFF");
	printf(  "%3s ", (p_object->data)&PIN05REG?"ON":"OFF");
	printf(  "%3s\n", (p_object->control)&PIN17REG?"OFF":"ON");
}

void draw_fli_status( st_fli_info* p_object)
{
	if(p_object->focuser_moving) printf("Focuser is moving\n");
	else printf("Focuser is not moving\n");
	
	printf("Focuser Max step = %ld\n",p_object->focuser_extent);
	printf("Focuser current position %ld\n",p_object->focuser_curr_position);
	
	if(p_object->wheel_moving) printf("Filter Wheel is moving\n");
	else printf("Filter Wheel is not moving\n");
	
	printf("Focuser current position %ld\n",p_object->wheel_curr_position);
}


