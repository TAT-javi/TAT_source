/*Guard function
This function will protect the telescope interrupting the observation when necessary
*/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "symblc_const.h"
#include "weather_func.h"
#include "posixtm.h"
#include "common_func.h"
#include "dsp_func.h"
#include "ppc_func.h"


#define DSP_COUNT_CRITERION 10 /*Time for checking if DSP hangs*/

extern st_tat_info *p_tat_info;

//int type 0= before observation, 1 = observation, 2 = after observation, 
//			3 = dark, 4= observation no tracking
int guard(int type) 
{
	time_t curTime;
	int diffTime;
	char *pcCurTime,*pcWait2Time;
	char temp_string[200],systemCmd[80],log_text[200];
	char running_program[BUFSIZ];
	FILE *fp;
	weather_info weather;
	cloud_info cloud;
	int condition;
	int wind_criterion_time, wind_successive_time;
	int humidity_criterion_time, humidity_successive_time;
	int cloud_criterion_time,cloud_successive_time;
	int rain_criterion_time,rain_successive_time;
	int cloud_data_break_criterion_time, cloud_data_break_successive_time;
	int weather_data_break_criterion_time, weather_data_break_successive_time;
	int weather_unlink_criterion_time, weather_unlink_time;
	int result=0,counter_webcams=1;
	int flat_freeze_wait_time,flat_freeze_succesive_time;
	int prev_dsp_count;
	
	/* Initialize variables */
	wind_criterion_time = DoGetValue("WIND_CRITERION_TIME");
	wind_successive_time=0;
	humidity_criterion_time = DoGetValue("HUMIDITY_CRITERION_TIME");
	humidity_successive_time=0;
	weather_data_break_criterion_time=DoGetValue("DATA_BREAK_CRITERION_TIME");
	weather_data_break_successive_time=0;
	cloud_data_break_criterion_time= DoGetValue("DATA_BREAK_CRITERION_TIME");
	cloud_data_break_successive_time=0;
	weather_unlink_criterion_time= DoGetValue("WEATHER_UNLINK_CRITERION_TIME");
	weather_unlink_time=0;
	flat_freeze_wait_time= DoGetValue("FLAT_FREEZE_WAIT_TIME");
	flat_freeze_succesive_time=0;
	cloud_criterion_time= DoGetValue("CLOUD_CRITERION_TIME");
	cloud_successive_time=0;
	rain_criterion_time= DoGetValue("RAIN_CRITERION_TIME");
	rain_successive_time=0;

	
    //Check type
	if(type == 1) DoGetValueString("OBSERVE_PROGRAM", running_program);
	else if (type ==0 || type ==2) 
		DoGetValueString("FLAT_PROGRAM", running_program);
	else if (type == 3)
		DoGetValueString("DARK_PROGRAM", running_program);
	
	int minute = 0;	/* count to 60 */

	remove( AUTO_CMD_FILENAME);
	/* get current time  */
	time(&curTime);
	pcCurTime=ctime(&curTime);
	printf("curtime=  %s",pcCurTime);
	fflush(NULL);

	/* create mysql database session */
	result=0;
	prev_dsp_count = -1;
	
	while(result <= 0)//there isn't a final ouput yet
	{
		//First check if there is an emergency command
		if( (fp=fopen(AUTO_CMD_FILENAME,"r")) != NULL)
		{
			fscanf(fp,"%s",temp_string);
			fclose(fp);
			remove( AUTO_CMD_FILENAME);
			if(!strcmp(BREAK_ALL_OBSERVATION ,temp_string))
			{
				result = EMERGENCY_STATUS;
				condition =0;//do not do more checking
				continue; //finish the loop
			}
			else if(!strcmp(DO_NEXT_OBSERVATION ,temp_string))
			{
				result = NEXT_OBSERVATION;
				condition =0;//do not do more checking
				continue; //finish the loop
			}
		}
		if(result != EMERGENCY_STATUS)
		{
			if(p_tat_info->obs_info.ccd_status && counter_webcams)
			{
				pwr_rx_poweroff();
				counter_webcams=0;
			}
			time(&curTime);
			diffTime = p_tat_info->obs_info.end_time - curTime;
			
			//check if it is time to end the program
			switch(type)
			{
				case 0://before observation
				case 2://after observation
					condition=1;//taking flat
				//the autoflat changes the ccd status to doing_flat every cycle (less than one minute)
					if(p_tat_info->obs_info.ccd_status== CCD_FLAT)
					{
						flat_freeze_succesive_time++;
						if(flat_freeze_succesive_time >= flat_freeze_wait_time)
						{
							printf ("The auto_flat was frozen for %d.\n",flat_freeze_succesive_time);
							steplog("ERROR: Auto_flat does not respond", AUTO_OBSERVE_LOG_TYPE);
							condition =0;
						}
					}
					else if(p_tat_info->obs_info.ccd_status == CCD_DOING_FLAT)//CHECK IF AUTO FLAT FREEZE
					{
						flat_freeze_succesive_time=0; //reset the counter
						//change the status to see if the auto_flat changes it
						p_tat_info->obs_info.ccd_status = CCD_FLAT;
					}
					else condition =0;
					break;
				case 1://taking exposures
					if(diffTime<2) condition = 0;
					else condition = 1;//Time difference > 2
				break;
				case 3://taking dark
					if(p_tat_info->obs_info.ccd_status== CCD_DARK)
					{
						condition=1;//taking flat
						minute = 0;//to avoid the guard program to check.
					}
					else condition =0;
				break;
				default:
					condition =0;
				break;
			}
		}
		if( condition ) 
		{
			if ( minute == 60 )
			{
				minute = 0;
				/* get weather condition */
				result = get_weather_condition (&weather);
				result += get_cloud_condition (&cloud);

				if( result == 0 )
				{
					weather_unlink_time =0;
					if ( time(0)-weather.timekey <= 70 )
					{
						weather_data_break_successive_time=0;
						/* check humidity */
						if( weather.humid_out > (float) DoGetValue("HUMIDITY_CRITERION") )
						{
							/* high humidity */
							sprintf(log_text,"WARNING: Current outside humidity %.2f\n", weather.humid_out);
							sprintf(log_text,"%sHumidity successive time %d",log_text,humidity_successive_time+=60);
							step(log_text);
							if( humidity_successive_time >= humidity_criterion_time )
								result = HUMIDITY_STATUS;
						}
						else     /* safe. */
							humidity_successive_time=0;

						/* check wind speed */
						if( weather.wind_avg > (float) DoGetValue("WIND_CRITERION") )
						{
								/* strong wind */
								sprintf(log_text,"WARNING: Current wind speed %.2f \n",weather.wind_avg);
								sprintf(log_text,"%sStrong wind successive time %d",log_text,wind_successive_time+=60);
								step(log_text);
								if( wind_successive_time >= wind_criterion_time)
										result = WIND_STATUS;
						}
						else   /* safe	*/
								wind_successive_time=0;
					}
					else
					{
						sprintf(log_text,"WARNING: No data from console successive time %d",weather_data_break_successive_time+=60);
						step(log_text);
						if(weather_data_break_successive_time >= weather_data_break_criterion_time)
									result = WEATHER_DATA_BREAK;
					}
					//////CHECK CLOUD DATA
					if(time(0)-cloud.timekey <= 120)
					{
						cloud_data_break_successive_time=0;
						/* check cloud */
						if( cloud.clarity < (float) DoGetValue("CLOUD_CRITERION") )
						{
							/* Cloud present */
							sprintf(log_text,"WARNING: Current clarity %.2f \n",cloud.clarity);
							sprintf(log_text,"%sCloud successive time %d",log_text,cloud_successive_time+=60);
							step(log_text);
							if( cloud_successive_time >= cloud_criterion_time)
								result = CLOUD_STATUS;
						}
						else   /* safe  */
						{
// 			printf("CLARITY=%.2f\n",cloud.clarity);
							cloud_successive_time=0;
						}
						/* check rain */
						if( cloud.rain > (float) DoGetValue("RAIN_CRITERION") )
						{
							/* Raining */
							sprintf(log_text,"WARNING: Current rain %.2f \n", cloud.rain);
							sprintf(log_text,"%sRain successive time %d",log_text,rain_successive_time+=60);
							step(log_text);
							if( rain_successive_time >= rain_criterion_time)
								result = RAIN_STATUS;
						}
						else   /* safe  */
						{
//   			printf("RAIN=%.2f\n",cloud.rain);
							rain_successive_time=0;
						}
					}
					else
					{
						sprintf(log_text,"WARNING: No data from cloud sensor successive time %d",cloud_data_break_successive_time+=60);
						step(log_text);
						if(cloud_data_break_successive_time >= cloud_data_break_criterion_time)
							result = CLOUD_DATA_BREAK;
					}
				}
				else
				{
					sprintf(log_text,"WARNING: Database error successive time %d",weather_unlink_time+=60);
					step(log_text);
					if(weather_unlink_time >= weather_unlink_criterion_time)
						result = GET_WEATHER_FAILED;
				}
			}
			if( (fp=fopen(AUTO_CMD_FILENAME,"r")) != NULL) //check again
			{
				fscanf(fp,"%s",temp_string);
				fclose(fp);
				remove( AUTO_CMD_FILENAME);
				if(!strcmp(BREAK_ALL_OBSERVATION ,temp_string))
						result = EMERGENCY_STATUS;
				else if(!strcmp(DO_NEXT_OBSERVATION ,temp_string))
						result = NEXT_OBSERVATION;
			}

			
			if (type == 1) //star track is running
			{
				if(prev_dsp_count>0 && //only if prev_dsp is positive
					(p_tat_info->dsp_info.time_count - prev_dsp_count) > DSP_COUNT_CRITERION)
				{ 
					//DSP hang reboot
					ResetDsp();
					sprintf( temp_string, "RA POS FREQUENCY 1 30030 9973 0 0 0 0");
					send_cmd2dsp_chk(temp_string);
				}
				
					
				prev_dsp_count = p_tat_info->dsp_info.time_count;
			}
				
				
			printf("			Countdown %d \r", diffTime);fflush(NULL);
			sleep(1);
			minute++;
		}
		else
		{
			if (type == 1)
			{
				if(curTime==p_tat_info->obs_info.end_time)
					result = NORMAL_STATUS;
				else
				{
					if(  diffTime  < 0)
					{
						printf("timing diffTime < 0 ");
						result = NORMAL_STATUS;
					}
					else
						usleep(10000);
				}
			}
			else// if(type == 0 || type ==2 || type == 3)
			{
				p_tat_info->obs_info.ccd_status = CCD_IDLE;//ccd idle
				result = NORMAL_STATUS;
			}
		}
	} /* while */
	///// Done Finish programs /////
	sprintf(systemCmd,"pkill %s", running_program);
	system(systemCmd);
	steplog( systemCmd, AUTO_OBSERVE_LOG_TYPE);
	
	
    //Turn on the web cams
	pwr_rx_poweron();

	switch( result )
	{
		case WIND_STATUS:
            sprintf(log_text,"WARNING: Strong wind now !!!");
		break;

		case HUMIDITY_STATUS:
            sprintf(log_text,"WARNING: High humidity detected !!!");
		break;
		
		case RAIN_STATUS:
            sprintf(log_text,"WARNING: It is raining now !!!");
		break;
		
		case CLOUD_STATUS:
            sprintf(log_text,"WARNING: It is too cloudy now !!!");
		break;
		case ERROR_STATUS:
            sprintf(log_text,"ERROR: guard( ) encounter a problem!!!");
		break;

		case WEATHER_DATA_BREAK:
            sprintf(log_text,"ERROR: No weather data !!!");
		break;
		
		case CLOUD_DATA_BREAK:
            sprintf(log_text,"ERROR: No cloud sensor data !!!");
		break;

		case GET_WEATHER_FAILED:
            sprintf(log_text,"ERROR: Weather or cloud DB failed !!!");
		break;
		case NORMAL_STATUS:
			if(type ==0 || type ==2)
                sprintf(log_text,"### Normal finish taking flat fielding");
			else if(type == 1 || type ==4)
                sprintf(log_text,"### Normal finish observation");
			else if (type ==3)
                sprintf(log_text,"### Normal finish taking dark current");
			else
                sprintf(log_text,"### Normal finish");

		break;
		case NEXT_OBSERVATION:
			if(type == 1 || type ==4)
                sprintf(log_text,"WARNING: Observer requested to do next observation");
			else
                sprintf(log_text,"WARNING: Observer requested to move to next step");

		break;
		case EMERGENCY_STATUS:
            sprintf(log_text,"WARNING: Observer has stopped the auto observation");
		break;
		default:
			result = ERROR_STATUS;
			sprintf(log_text,"ERROR: result unknown!!!");
		break;
		
	}
	steplog(log_text, AUTO_OBSERVE_LOG_TYPE);
	
	return result;
}
