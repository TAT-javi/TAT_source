/*******************************************************************
AUTOMATIC OBSERVATION MAIN PROGRAM
Steps:
00 - Read and check input file and weather conditions
10 - Preparing for observations
20 - Flat before observations
30 - Observations
40 - Flat after observations
50 - Ending observations
********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include "Apogee.h"
#include "posixtm.h"
#include "myAstro.h" 
#include "symblc_const.h"
#include "common_func.h"
#include "ppc_func.h"
#include "main_auto_observe.h"

#include "ccd_func.h"
#include "dsp_func.h"
#include "open_close_enc.h"
#include "pwr_func.h"
#include "ctl_func.h"
#include "weather_func.h"
#include "send_remote_func.h"

st_tat_info *p_tat_info;

int main(int argc ,char *argv[])
{
	time_t time_now, *p_begin_time, *p_end_time;
	char time_begin[20], time_end[20];
	char temp_string[BUFSIZ];
	char system_cmd[BUFSIZ],observe_program[BUFSIZ],dark_program[BUFSIZ];
	char backup_program[BUFSIZ];
	int i, countdown,try, wait_result =0, wait_temp = 0, star_rise,wait_flat;
	int enclosure_open_time,use_report;
	int N_obs,obs_count,N_input_error,update_coord_result,guard_option;
	float set_point,previous_RA,previous_DEC;
	int start_observation;
	char flat_filter_options[FILTER_TOTAL_NUMBER],dark_exp_option[200];

	DATE lt,now;
	cloud_info cloud;

	//for weather report
	float avg,min,max;
	const char *weather_var[]={"temper_out","humid_out","wind_avg",
					"clarity","rain"};//5 variable

	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
	
	p_begin_time = &(p_tat_info->obs_info.begin_time);
	p_end_time = &(p_tat_info->obs_info.end_time);

	long wait_interval =  DoGetValue("GET_OBSERVE_TIME_INTERVAL");

	while(1) //Do not stop
	{
/*
		00 - Read and check input file and weather conditions
*/
		step("Checking observing time");
		do
		{
			start_observation= DoGetObserveTime( &set_point, p_begin_time, p_end_time);
			//Check input errors
			N_input_error = check_input_file();
			if(N_input_error)
			{
				sprintf(temp_string,"ERROR : There are %d input errors\nCheck %s !!!",
					   N_input_error,TIME_TABLE_FILENAME);
				step(temp_string);
				start_observation=0;
			}
			if(!start_observation)
			{
				p_tat_info->obs_info.auto_observing = 0;
				step("Waiting for the next check");
				timing( time(NULL)+ wait_interval);
			}
		}while( !start_observation);
/*##########################################################
  		10 - Preparing for observations 
############################################################*/
		step ("Initialize variables");
		p_tat_info->obs_info.auto_observing = 1;
		p_tat_info->obs_info.ccd_status = CCD_IDLE;
		p_tat_info->obs_info.expose_time = set_point;
		
		for(i=0;i<FILTER_TOTAL_NUMBER;i++)
			p_tat_info->obs_info.exp_time_changed[i]=0; //Reset the change exp time flag

		
		enclosure_open_time=DoGetValue("ENCLOSURE_OPEN_TIME");
		DoGetValueString("OBSERVE_PROGRAM", observe_program);
		DoGetValueString("DARK_PROGRAM", dark_program);
		DoGetValueString("BACKUP_PROGRAM", backup_program);
		
		//CREATE REPORT AND LOG FILES
		log_this("Start automatic observation",AUTO_OBSERVE_LOG_TYPE,1);//create file if not present
		log_this("Start observation",AUTO_REPORT_LOG_TYPE,1);//create file if not present
		
		sprintf(temp_string,"Observing begin time: %s\nObserving end time: %s",ctime(p_begin_time),ctime(p_end_time));
		steplog(temp_string, AUTO_OBSERVE_LOG_TYPE);
		
		/* remove previous emergency stop command */
		remove(AUTO_CMD_FILENAME);

		/* All Power on */
		TurnAllPowerOn();

		/* IR Camera power on */
		pwr_rx_poweron();

		//send_cmd2ctl("ccddaemon restart");

		step("Waiting for system stabilizing (20 sec)");
		timing( time(NULL) + 20);

		/* CCD cooler on */
		steplog("CCD cooler ON", AUTO_OBSERVE_LOG_TYPE);
		ccd_cooler_on(set_point);

		//refresh focuser position and set filter in origin 
		refresh_FLI_values();
		
		steplog("Waiting for CCD cool down", AUTO_OBSERVE_LOG_TYPE);
		timing(time(NULL)+DoGetValue("CCD_COOLDOWN_TIME"));

		steplog("Taking first CCD image", AUTO_OBSERVE_LOG_TYPE);
		sprintf(temp_string,"/home/observer/test_image/Trash.fit");
		remove(temp_string);
		ccd_takeimage(temp_string, 2, 1, 1);

		//Reset the telescope for observation
		SafeResetTelescope();
		/*
			2. Unlock enclosure
			3. Open enclosure and ? Reset RF
		*/
		try =0;
		do
		{
			OpenPart();
			try++;
			if(try >=3)
			{
				steplog("ERROR: Could not confirm that the enclosure is open!!", AUTO_OBSERVE_LOG_TYPE);
				finish_observation(dark_program,dark_exp_option,EMERGENCY_STATUS);
				return 1;
			}
		}while(p_tat_info->dsp_info.enc.closed_ls);
		
		////Check for MultiObservations
		N_obs = check_multiple_observation(flat_filter_options,dark_exp_option);
		sprintf(temp_string,"Obs = %d %s %s\n",N_obs,flat_filter_options,dark_exp_option);
		step(temp_string);
		if(!N_obs)
		{
			steplog("ERROR: Number of observations is zero", AUTO_OBSERVE_LOG_TYPE);
			finish_observation(dark_program,dark_exp_option,EMERGENCY_STATUS);
			return 1;
		}
		sprintf(temp_string,
				"Number of scheduled observations = %d\tUsed filters: %s\tUsed exp: %s",
						N_obs,flat_filter_options,dark_exp_option);
		steplog(temp_string, AUTO_OBSERVE_LOG_TYPE);
/*##########################################################
  		20 - Flat before observations
############################################################*/
		////////////////Flat before observation showtime/////////////
		if(!strcmp(p_tat_info->obs_info.flat_start , "b") ||
				!strcmp(p_tat_info->obs_info.flat_start , "t"))
		{
			//Check current light and current light
			cloud.timekey=0;
			cloud.light = 50;
			wait_temp = get_cloud_condition(&cloud);
			now=getSystemTime();
			sprintf(temp_string,"Light = %.0f",cloud.light);
			step(temp_string);
			if(wait_temp>=0 && cloud.light < 10 && abs(now.timestamp -cloud.timekey)<180)
			{
				steplog("WARNING: Light too low for flat field", AUTO_OBSERVE_LOG_TYPE);
			} 
			else 
			{
				steplog("Take flat field before observation", AUTO_OBSERVE_LOG_TYPE);
				wait_flat=take_flat_process(0,flat_filter_options);
				if(wait_flat) 
				{
					return 1;
				}
				///////////Reset telescope and CCDdaemon////
	// 			send_cmd2ctl("ccddaemon restart");
				steplog("Reset Telescope", AUTO_OBSERVE_LOG_TYPE);
				p_tat_info->obs_info.status = Returning;
				SafeResetTelescope();
	// 			ccd_cooler_on(set_point);
			} // if (flat before observation)
		}
/*##########################################################
  		30 - Observations
############################################################*/
		// MULTI OBSERVATION SUPPORTED
		previous_DEC = previous_RA = -1;
		obs_count=0;
		do
		{
			update_coord_result = set_current_observation(flat_filter_options,dark_exp_option);
			if(!update_coord_result) 
			{
				step("### No more observations tonight");
				continue;
			}
			else if(update_coord_result<0)
				previous_RA = previous_DEC=-1; //for dark, return to home position 
				
			obs_count++;
			
			sprintf(temp_string,"Observation %d",obs_count);
			steplog(temp_string, AUTO_OBSERVE_LOG_TYPE);
			
			// For the first observation
			//doublecheck the telescope is in home position 
			
			if(previous_RA <0 &&(!p_tat_info->dsp_info.ra.origin || !p_tat_info->dsp_info.dec.origin ))
			{
				p_tat_info->obs_info.status = Returning;
				SafeResetTelescope();
			}
			// Updates the ending time and coordinates
			//in the shared memory
			
			if(update_coord_result>0) //star observation wait for star
			{
				sprintf(temp_string,"### Observing %s",p_tat_info->obs_info.target_name);
				steplog(temp_string, AUTO_OBSERVE_LOG_TYPE);
				////wait if the star is not on the sky
				steplog("Wait for Star Rise", AUTO_OBSERVE_LOG_TYPE);
				p_tat_info->obs_info.status = STOP;
				star_rise = wait4star_rise();
				
				if(star_rise==EMERGENCY_STATUS)
				{
					steplog("WARNING: Observation canceled by observer", AUTO_OBSERVE_LOG_TYPE);
					return 1;
				}
				else if(star_rise==NEXT_OBSERVATION)
				{
					steplog("WARNING: Observer requested to move to next observation", AUTO_OBSERVE_LOG_TYPE);
					continue;
				}
				else if(star_rise!=NORMAL_STATUS) //Bad weather
				{
					steplog("WARNING: No Observation today", AUTO_OBSERVE_LOG_TYPE);
					wait_result = star_rise;
					finish_observation(dark_program,dark_exp_option,wait_result);
					return 1;
				}
				
				p_tat_info->obs_info.FOV = FOV_TBC;
				steplog("Call observing program", AUTO_OBSERVE_LOG_TYPE);
				// Check if it is the first night's observation
				if(previous_RA<0)
					sprintf(system_cmd,"%s%s &",APP_PATH,observe_program);
				else
					sprintf(system_cmd,"%s%s %lf %lf &",APP_PATH,
							observe_program,previous_RA,previous_DEC);
					
				system(system_cmd);
				steplog(system_cmd, AUTO_OBSERVE_LOG_TYPE);
				guard_option=1;
				//for killing the program
				sprintf(system_cmd,"killall -9 %s",observe_program);
				previous_RA = p_tat_info->obs_info.RA;
				previous_DEC = p_tat_info->obs_info.DEC;
			}
			else //Dark observation == -1 for using shared memmory, -2 for use string
			{
				steplog("### Taking Darks", AUTO_OBSERVE_LOG_TYPE);
				p_tat_info->obs_info.FOV = FOV_CORRECT; //
				ForwardTelescope3Degree(); //move telescope away from HS
				steplog("Call dark program", AUTO_OBSERVE_LOG_TYPE);
				if(update_coord_result == -1)
					sprintf(system_cmd,"%s%s 0 4000 &",APP_PATH,dark_program);
				else
					sprintf(system_cmd,"%s%s %s 4000 &",APP_PATH,dark_program,dark_exp_option);
					
				system(system_cmd);
				steplog(system_cmd, AUTO_OBSERVE_LOG_TYPE);
				//for killing the program
				guard_option=3;
				sprintf(system_cmd,"killall -9 %s",dark_program);
				previous_RA = previous_DEC=-1;
				p_tat_info->obs_info.ccd_status = CCD_DARK;//3=taking dark
			}
	/////////////		Wait for end time
			steplog("Waiting for observing end time", AUTO_OBSERVE_LOG_TYPE);
			wait_result = guard(guard_option);
		// ++++++ Break observation program ++++++++
			step("End of observation.Break observing program");
			//Kill the program
			system(system_cmd);
			//abort any possible taking exposure from CCD
			ccd_abort_takeimage();
			
			steplog( system_cmd, AUTO_OBSERVE_LOG_TYPE);
			p_tat_info->obs_info.ccd_status = CCD_IDLE;

			if(wait_result == EMERGENCY_STATUS)
			{
				p_tat_info->obs_info.auto_observing = 0;
				return 1;//The emergency program will finish observation
			}
			else if(wait_result == NEXT_OBSERVATION)
			{
				sprintf(temp_string,"Obs %d stopped. Moving to next observation.",obs_count);
				steplog(temp_string, AUTO_OBSERVE_LOG_TYPE);
			}
			else if(wait_result != NORMAL_STATUS)//Weather wrong or something
			{
				////*****Finishing observation*********////
				finish_observation(dark_program,dark_exp_option,wait_result);
				return 1;//FINISH OBSERVATION ++++TOMORROW WILL BE ANOTHER DAY
			}
			else if(wait_result == NORMAL_STATUS)
			{
				sprintf(temp_string,"Observation %d had Normal Finish",obs_count);
				steplog(temp_string, AUTO_OBSERVE_LOG_TYPE);
			}//if(wait_result)
		}while(update_coord_result);
//////////////////////Normal uninstall procedure///////////////////////
/*##########################################################
  		40 - Flat after observations
############################################################*/
		if (!strcmp(p_tat_info->obs_info.flat_start , "a") || 
			!strcmp(p_tat_info->obs_info.flat_start , "t"))
		{
			//Check current light and current light
			cloud.timekey=0;
			cloud.light = -50;
			wait_temp = get_cloud_condition(&cloud);
			now=getSystemTime();
			sprintf(temp_string,"Light = %.0f",cloud.light);
			step(temp_string);
			if(wait_temp>=0 && cloud.light > 30 && abs(now.timestamp -cloud.timekey)<180)
			{
				steplog("WARNING: It is too bright for flat field", AUTO_OBSERVE_LOG_TYPE);
			}
			else 
			{
	// 			send_cmd2ctl("ccddaemon restart");
				steplog("Take flat field after observation", AUTO_OBSERVE_LOG_TYPE);
				steplog("Moving Telescope to Home Position", AUTO_OBSERVE_LOG_TYPE);
				p_tat_info->obs_info.status = Returning;
				SafeResetTelescope();
	// 			ccd_cooler_on(set_point);
				
				//////Call The Flat function //////////
				wait_flat= take_flat_process(2,flat_filter_options);
				if(wait_flat)
				{
					return 1; //EMERGENCY STATUS
				}
			}
		}
/*##########################################################
  		50 - Ending observations
############################################################*/
		finish_observation(dark_program,dark_exp_option,wait_result);
		
/*##########################################################
  		60 - Back up the data to remote server
############################################################*/

		steplog("Perform remote backup", AUTO_OBSERVE_LOG_TYPE);
		remote_backup();

		
		log_this("End of observation",AUTO_REPORT_LOG_TYPE,0);
		
		//Report of weather
		printf(temp_string,"### Weather Report ###");
		for(i=0;i<5;i++)
		{
			if(!get_avg_min_max_db(weather_var[i],&avg,&min,&max, *p_begin_time))
			{
				sprintf(temp_string,"%s\n%s avg=%.2f,min=%.2f,max=%.2f",
						temp_string,weather_var[i],avg,min,max);
			}
		}
		log_this(temp_string,AUTO_REPORT_LOG_TYPE,0);
		
	}//while(1)
	return 0;
}	/*main()*/

//////////////////////////////////////////////////////////////////////
////////END OF MAIN/////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
////////FINISH THE OBSERVATION SUBROUTINE/////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void finish_observation(char *dark_program,char * dark_exp_option,int wait_result)
{
	char system_cmd[BUFSIZ],temp_string[300];
	int i,dark_number=3,flat_time=10,mode_index;

	//shutdown the CCD if weather problem
	if(wait_result != NORMAL_STATUS && wait_result != EMERGENCY_STATUS)ccd_cooler_shutdown();

	p_tat_info->obs_info.status = Returning;
    
	//Move forward if the telescope is in origin
	if( p_tat_info->dsp_info.dec.origin || p_tat_info->dsp_info.ra.origin)
	{
		steplog("Forward Telescope 3 degree", AUTO_OBSERVE_LOG_TYPE);
		ForwardTelescope3Degree();
	}
    //CLOSE ENCLOSURE and reset telescope
	steplog("Moving telescope to Home position and closing enclosure", AUTO_OBSERVE_LOG_TYPE);
	SafeResetTelescope(); //Send telescope to home position first
	ClosePart();

    //shutdown the CCD if weather problem
	if(wait_result != NORMAL_STATUS && wait_result != EMERGENCY_STATUS)
		ccd_cooler_shutdown();
	
	/*timing in ParkTelescope()*/
	steplog("Park Telescope ", AUTO_OBSERVE_LOG_TYPE);
	ForwardTelescope3Degree();

	steplog("Turn all power off except CCD power and Main power", AUTO_OBSERVE_LOG_TYPE);
	TurnPartPowerOff();
	if(wait_result == NORMAL_STATUS)//NORMAL STATUS==> Take dark
	{
		//////////////dark current SHOWTIME//////////////////////////////////////
		//Turn webcams OFF first
		pwr_rx_poweroff();
		
		// GET FLAT EXPOSURE TIME and ADD to the dark_exp_option
// 		flat_time = DoGetValue("FLAT_EXPOSURE_TIME");
// 		sprintf(dark_exp_option,"%d,%s",flat_time,dark_exp_option);

// 		TAKE DARK CURRENT
		dark_number = DoGetValue("DARK_NUMBER");
		steplog("Call take dark current program", AUTO_OBSERVE_LOG_TYPE);
		sprintf(system_cmd,"%s%s %s %d &",APP_PATH,dark_program,
				dark_exp_option,dark_number);
		system(system_cmd);
		steplog(system_cmd, AUTO_OBSERVE_LOG_TYPE);
		log_this("Take dark current after observation",AUTO_REPORT_LOG_TYPE,0);
		p_tat_info->obs_info.ccd_status = CCD_DARK;//dark current
		p_tat_info->obs_info.status = Darking;

	/////////////////wait till dark current ends
		while(p_tat_info->obs_info.ccd_status==CCD_DARK) sleep(1);

		////kill the auto_dark program just in case////
		sprintf(system_cmd,"killall -9 %s",dark_program);
		system(system_cmd);
		//abort any possible taking exposure from CCD
		ccd_abort_takeimage();
		p_tat_info->obs_info.ccd_status = CCD_IDLE;//ccd idle
		p_tat_info->obs_info.status = STOP;
    }
	//Turn web cams ON to be sure
	pwr_rx_poweron();
   
	p_tat_info->obs_info.status = STOP;
	steplog("CCD cooler warm up", AUTO_OBSERVE_LOG_TYPE);
	for( i=0;i<10;i++)
	{
		ccd_cooler_shutdown();
		sleep(60);
		mode_index = p_tat_info->ccd_info.cooler_status;
		
		if( !strcmp(Cooler_status_string[mode_index],COOL_ATAMBIENT_STR) ||
			!strcmp(Cooler_status_string[mode_index],COOL_ATMIN_STR) ||
			!strcmp(Cooler_status_string[mode_index],COOL_ATSETPOINT_STR))
		{
			break;
		}
		else if(p_tat_info->ccd_info.curr_point >= p_tat_info->ccd_info.set_point) 
		{
			break;
		}
		else
		{
			printf("After %d seconds the temperature of CCD = %-7.5f\n",
				   i*60,p_tat_info->ccd_info.curr_point);
		}
	}
	steplog("Turn cooler OFF", AUTO_OBSERVE_LOG_TYPE);
	ccd_cooler_off();

	/* wait for ccd cooler off work */
	sleep(5);
	TurnCcdMainPowerOff();
	steplog("Turn CCD and Main Power OFF", AUTO_OBSERVE_LOG_TYPE);
	p_tat_info->obs_info.auto_observing = 0;
}

//////////////////////////////////////////////////////////////////////
////////FLAT PROCESSS Subroutine/////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int take_flat_process(int flat_type,char *flat_filter_options)
{
	int wait_result;
	char flat_program[BUFSIZ],system_cmd[BUFSIZ];

	DoGetValueString("FLAT_PROGRAM", flat_program);
	
	//////move telescope to zenith//////////
	steplog("Moving telescope to zenith", AUTO_OBSERVE_LOG_TYPE);
	
	p_tat_info->obs_info.status = Moving;
	Origin2zenith();

	/////////////Call flat program/////////////////////////
	steplog("Call take flat field program", AUTO_OBSERVE_LOG_TYPE);
	if( flat_type == 0 )
	{
		sprintf(system_cmd,"%s%s b %s &",APP_PATH,flat_program,flat_filter_options);
	}
	else if(flat_type == 2)
	{
		sprintf(system_cmd,"%s%s a %s &",APP_PATH,flat_program,flat_filter_options);
	}
	else return 0;

	system(system_cmd);
	steplog(system_cmd, AUTO_OBSERVE_LOG_TYPE);

	p_tat_info->obs_info.status = Flating;
	p_tat_info->obs_info.ccd_status = CCD_FLAT;//taking flat
	/////////////Wait for flat program to finish/////////////////////////
	steplog("Waiting for end of take flat field program", AUTO_OBSERVE_LOG_TYPE);
	wait_result = guard(flat_type); 

	/* Break take flat field program */
	step("Break take flat field program");
 	sprintf(system_cmd,"killall -9 %s",flat_program);
	system(system_cmd);
	//abort any possible taking exposure from CCD
	ccd_abort_takeimage();

	steplog( system_cmd, AUTO_OBSERVE_LOG_TYPE);
	p_tat_info->obs_info.ccd_status = CCD_IDLE;//ccd idle

	if(wait_result == EMERGENCY_STATUS)//Finish observation
	{
		p_tat_info->obs_info.auto_observing = 0;
		return 1;//FINISH OBSERVATION ++++TOMORROW WILL BE ANOTHER DAY
	}
	else if(wait_result != NORMAL_STATUS)//Finish observation
	{
	////*****Finishing observation*********////
		finish_observation(flat_program," ",wait_result);
		return 1;//FINISH OBSERVATION ++++TOMORROW WILL BE ANOTHER DAY
	}//if(wait_result != NORMAL_STATUS

	return 0; //NORMAL EXIT
}
