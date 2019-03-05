#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "symblc_const.h"
#include "weather_func.h"
#include "common_func.h"
#include "myAstro.h"
#include "fli_func.h"
#include "auto_observe_func.h"

#define TENMINUTES 600
#define ONEHOUR 3600
#define HALFHOUR 1800
#define THREEHOURS 10800
#define FOURHOURS 14400
#define BUFFER_SIZE 512

extern st_tat_info *p_tat_info;

int DoGetObserveTime(float *setpoint, time_t *p_begin_time, time_t *p_end_time)
{
	FILE *fp;
	char begin_string[15],end_string[15],ra_string[15],dec_string[15];
	char open_flag[2], flat_start[2];
	struct tm ptr_time;
	char *line = NULL;
	size_t len = 0;
	int i,ra_h,ra_m,ra_s,dec_d,dec_m,dec_s;
	char buf[BUFFER_SIZE],temp_string[200];
	int weather_flag;
	int clarity_criterion;
	int before_end_time,end_time_ok;
	char filter_string[100];
	DATE begin_date,end_date,now;
	int diff_begin, diff_end;

	/* initialize vars */
	before_end_time = DoGetValue("WAIT_BEFORE_END_TIME");

	now= getSystemTime();
	

	if( ( fp=fopen(TIME_TABLE_FILENAME, "r") ) == NULL)
	{
		sprintf(temp_string,"ERROR: Open %s failed", TIME_TABLE_FILENAME);
		step(temp_string);
		return 0;
	}

	while(!feof(fp) )
	{
		fgets(buf, BUFFER_SIZE, fp);
		if( !strncmp(buf,"DATA",4) )
					break;
	}
	//After DATA read 3 more lines
	fgets(buf, BUFFER_SIZE, fp);
	fgets(buf, BUFFER_SIZE, fp);
	fgets(buf, BUFFER_SIZE, fp);
	
	while(fgets(buf, BUFFER_SIZE, fp))
	{
		i=sscanf( buf,"%s %s %s %s %s %f %s %s %*s",
				open_flag, begin_string,end_string,
				ra_string,dec_string,
				setpoint,filter_string,flat_start);

		if(i<8) continue; //Some input is missing
//		printf("%s,%s,%s,%d\n",open_flag,begin_string,end_string,*setpoint);

		open_flag[1]='\0';
		flat_start[1]='\0';
		
		begin_date = string2date(begin_string);
		end_date = string2date(end_string);
		 
		diff_begin= now.timestamp - begin_date.timestamp;
		diff_end= end_date.timestamp - now.timestamp;
		 
		if(diff_begin > 0 && diff_end > before_end_time)
		{
		    if( ( strcmp(open_flag,"Y") && strcmp(open_flag,"y") )== 0)
		    {
// 				printf("%s,%s,%s,%d\n",open_flag,begin_string,end_string,*setpoint);
				//check weather conditions
				weather_flag =  check_previous_weather_conditions_print(1);//be conservative
				if(weather_flag != NORMAL_STATUS) return 0;

			    ////////////////////////analising flat time
				strcpy(p_tat_info->obs_info.flat_start , flat_start);
				if (!strcmp(p_tat_info->obs_info.flat_start,"a"))
					step("Lets do flat after observation");
				else if (!strcmp(p_tat_info->obs_info.flat_start,"b"))
					step("Lets do flat before observation");
				else if (!strcmp(p_tat_info->obs_info.flat_start,"t"))
					step("Lets do flat after and before observation");
				else if (!strcmp(p_tat_info->obs_info.flat_start,"n"))
					step("No Flat Fielding in this observation");
				else
				{
					sprintf(temp_string,"ERROR: Can not understand this flat \"%s\".",
						   p_tat_info->obs_info.flat_start);
					
					step(temp_string);
					break;
				}
				
				sscanf(ra_string,"%d:%d:%d",&ra_h,&ra_m,&ra_s);
				sscanf(dec_string,"%d:%d:%d",&dec_d,&dec_m,&dec_s);
				if(dec_d <0){dec_m *=-1;dec_s *=-1;}
				
				//for the shared memory
				p_tat_info->obs_info.RA  = (float)ra_h + (float)ra_m/60. + (float)ra_s/3600.;
				p_tat_info->obs_info.DEC = (float)dec_d + (float)dec_m/60. + (float)dec_s/3600.;
				
				//get the information of the filters
				p_tat_info->obs_info.N_filters = read_filter_string(filter_string, 
								p_tat_info->obs_info.filter_seq, 
								p_tat_info->obs_info.filter_exp_time,
								p_tat_info->obs_info.filter_obs_time);

				fclose( fp);
				//send the info back
				ptr_time.tm_year= begin_date.yr - 1900;
				ptr_time.tm_mon= begin_date.mon - 1;
				ptr_time.tm_mday= begin_date.day;
				ptr_time.tm_hour= begin_date.hr;
				ptr_time.tm_min= begin_date.min;
				ptr_time.tm_sec= begin_date.sec;
				ptr_time.tm_isdst= 0;
				*p_begin_time=mktime(&ptr_time);
				
				ptr_time.tm_year= end_date.yr - 1900;
				ptr_time.tm_mon= end_date.mon - 1;
				ptr_time.tm_mday= end_date.day;
				ptr_time.tm_hour= end_date.hr;
				ptr_time.tm_min= end_date.min;
				ptr_time.tm_sec= end_date.sec;
				ptr_time.tm_isdst= 0;
				*p_end_time=mktime(&ptr_time);
				
				return 1;
		    }
		    else
		    {
		    	sprintf(temp_string,"%s,%s,%s",open_flag,begin_string,end_string);
				step(temp_string);
				sprintf(temp_string,"WARNING: It is observing time, but Flag is \"%s\".",open_flag);
		    	step(temp_string);
		    }
		}
	}
	fclose( fp);
	return 0;
}


int wait4star_rise()
{
    DATE 	d,ut, lt;//Universal and Local Time
    double	LST; //Local Sidereal Time, in hours
    float	hourAngle_obj, dec_deg_obj,ra_obj;
	int weather_flag;
	int Min_dec,Max_dec;
	char systemCmd[BUFSIZ];
	char temp_string[BUFSIZ];
	FILE *fp;

	Min_dec = DoGetValue("MIN_DEC");
	Max_dec = DoGetValue("MAX_DEC");
	
	//Get the coordinates from the shared memory
	ra_obj = p_tat_info->obs_info.RA;
	dec_deg_obj = p_tat_info->obs_info.DEC;
	
    do
    {
		// check weather
		weather_flag = check_previous_weather_conditions_print(0);//Don't be conservative
		if(weather_flag != NORMAL_STATUS) return weather_flag;
		
		//Check if emergency status
		if( (fp=fopen(AUTO_CMD_FILENAME,"r")) != NULL)
		{
			fscanf(fp,"%s",temp_string);
			fclose(fp);
			remove( AUTO_CMD_FILENAME);
			if(!strcmp(BREAK_ALL_OBSERVATION ,temp_string))
			{
				step("WARNING: Observation canceled by observer.");
				return EMERGENCY_STATUS;
			}
			else if(!strcmp(DO_NEXT_OBSERVATION ,temp_string))
			{
				step("WARNING: Observer requested to move to next observation.");
				return NEXT_OBSERVATION;
			}
		}

		d=getSystemTime();
		// convert given local time to UT
		ut= convert2UT(d);
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
		// compute the hour angle of the object:
		hourAngle_obj = LST - p_tat_info->obs_info.RA;
		dec_deg_obj = p_tat_info->obs_info.DEC;

		if(dec_deg_obj < Min_dec || dec_deg_obj > Max_dec)
		{
			sprintf(temp_string,"ERROR: DEC = %f. Out of range (%d, %d)! Observation stops!",
				   dec_deg_obj,Min_dec,Max_dec);
			step(temp_string);
			return NEXT_OBSERVATION;
		}
		// set the range of HA to (-12, 12)
		while (hourAngle_obj < -12.0) hourAngle_obj +=24.0;
		while (hourAngle_obj >  12.0) hourAngle_obj -=24.0;
		if(hourAngle_obj < -12.0 || hourAngle_obj > 12.0)//GET OUT
		{
			sprintf(temp_string,"ERROR: Stopping observation because hour angle is out of range (HA=%16.6f).",
				    hourAngle_obj);
			step(temp_string);
			return NEXT_OBSERVATION;
		}
		sprintf(temp_string,"HourAngle= %12.6f, Dec=%12.6f\n", hourAngle_obj, dec_deg_obj);
		step(temp_string);
		if(hourAngle_obj < MIN_ALTITUDE)//WAIT 60 sec
		{
			sprintf(temp_string,"WARNING: HA = %f. Star has not risen yet! (<-5.3hr)\nWait 1 minute\n", hourAngle_obj);
            		step(temp_string);
			sleep(60);
		}
		else if(hourAngle_obj > 5.3)//GET OUT
		{
			sprintf(temp_string,"WARNING: Stopping observation because HA= %f. Star has already set! (>5.3hr)",
				    hourAngle_obj);
			step(temp_string);
			return NEXT_OBSERVATION;
		}
		else //go for observation
		{
			step("### Target is observable");
			return NORMAL_STATUS;
		}
    }while(1);
    
}


/////////////////////////
//Check if we have multiple observation night
/////////////////////
int check_multiple_observation(char *flat_filter_option, char *dark_exp_option)
{
	int i,N,Nobs=0,begin[7],end[7];
	int filter_present,exp_present;
	DATE begin_date,end_date,now;
	char buf[BUFFER_SIZE];
	char open_flag[2], begin_string[20],end_string[20];
	char filter_string[100];
	FILE *fp;

	if( ( fp=fopen(TIME_TABLE_FILENAME, "r") ) == NULL)
	{
		sprintf(buf,"ERROR: Open %s failed.",TIME_TABLE_FILENAME);
		step(buf);
		return 0;
	}

	while(!feof(fp))
	{
		fgets(buf, BUFFER_SIZE, fp);
		if( !strncmp(buf,"DATA",4) )
					break;
	}
	//After DATA read 3 more lines
	fgets(buf, BUFFER_SIZE, fp);
	fgets(buf, BUFFER_SIZE, fp);
	fgets(buf, BUFFER_SIZE, fp);
	
	now= getSystemTime();
//printf("Now = %04d/%02d/%02d %02d:%02d:%02d\n",now.year,now.mon,now.day,now.hr,now.min,now.sec);
	//zero the variables
	Nobs=0;
	flat_filter_option[0]='\0';
	dark_exp_option[0]='\0';
	
	while(fgets(buf, BUFFER_SIZE, fp)&&Nobs<5)
	{
		i=sscanf( buf,"%s %s %s %*s %*s %*s %s",
				open_flag, begin_string,end_string,filter_string);

		if(i<4)continue; //Some input is missing
		
		open_flag[1]='\0';
		if(open_flag[0] != 'Y' && open_flag[0] != 'y') continue; //only if y or Y
		
		
		begin_date = string2date(begin_string);
		end_date = string2date(end_string);
			
		//Check if it is observing time
		if(begin_date.timestamp < now.timestamp && 
				end_date.timestamp > now.timestamp)
		{
			if(Nobs)
			{
				step("ERROR: Several observations have the same schedule.");
				fclose(fp);
				return 0;
			}
			begin[Nobs] = begin_date.timestamp;
			end[Nobs] = end_date.timestamp;
			Nobs++;
			update_flat_dark_strings(filter_string,flat_filter_option,dark_exp_option);
		}
			
		if(Nobs)
		{//check if the next observation happens whithin ten minutes
			if(begin_date.timestamp < (end[Nobs-1] +TENMINUTES) &&
					begin_date.timestamp > begin[Nobs-1])
			{
				if(end_date.timestamp < begin_date.timestamp+ONEHOUR)
				{
					step("ERROR: End Observation has to be at least one hour higher than Start Observation.");
					fclose(fp);
					return 0;
				}
				else
				{
					begin[Nobs] = begin_date.timestamp;
					end[Nobs] = end_date.timestamp;
					Nobs++;
					update_flat_dark_strings(filter_string,flat_filter_option,dark_exp_option);
				}
			}
		}
	}
	fclose(fp);
	return Nobs;
}


////////////////

int set_current_observation(char *flat_filter_option, char *dark_exp_option)
{
	int i,ra_h,ra_m,ra_s;
	int dec_d,dec_m,dec_s;
	DATE begin_date,end_date,now;
	char buf[BUFFER_SIZE];
	char begin_string[20],end_string[20],ra_string[15],dec_string[15];
	char open_flag[2],filter_string[100],target_name[100];
	FILE *fp;
	struct tm *ptr_time;
	time_t rawtime;
	fpos_t *fposition;
	
	if( ( fp=fopen(TIME_TABLE_FILENAME, "r+") ) == NULL)
	{
		sprintf(buf,"ERROR: Open %s failed.",TIME_TABLE_FILENAME);
		step(buf);
		return 0;
	}

	while(!feof(fp) )
	{
		fgets(buf, BUFFER_SIZE, fp);
		if( !strncmp(buf,"DATA",4) )
					break;
	}
	//After DATA read 3 more lines
	fgets(buf, BUFFER_SIZE, fp);
	fgets(buf, BUFFER_SIZE, fp);
	fgets(buf, BUFFER_SIZE, fp);
	
	now= getSystemTime();

	fposition = (fpos_t *) malloc (sizeof(fpos_t));
	
	while(!feof(fp))
	{
		fgetpos(fp, fposition);
		if(!fgets(buf, BUFFER_SIZE, fp)) break;
		
		i=sscanf( buf,"%s %s %s %s %s %*s %s %*s %s",
				open_flag,begin_string,end_string,
				ra_string,dec_string,filter_string,target_name);
	
		if(i<7) continue; // Some input is missing
		
		open_flag[1]='\0';
		if(!(open_flag[0] == 'Y' || open_flag[0] == 'y')) continue; //only if y or Y
		
		begin_date = string2date(begin_string);
		end_date = string2date(end_string);
		
		//Check if it is observing time
		if(begin_date.timestamp <= (now.timestamp + TENMINUTES)&& 
				end_date.timestamp >= now.timestamp + HALFHOUR)
		{
			time (&rawtime);
			ptr_time = localtime(&rawtime);
			
			ptr_time->tm_year= end_date.yr - 1900;
			ptr_time->tm_mon= end_date.mon - 1;
			ptr_time->tm_mday= end_date.day;
			ptr_time->tm_hour= end_date.hr;
			ptr_time->tm_min= end_date.min;
			ptr_time->tm_sec= end_date.sec;
			ptr_time->tm_isdst= 0;
			p_tat_info->obs_info.end_time=mktime(ptr_time);
			
			sscanf(ra_string,"%d:%d:%d",&ra_h,&ra_m,&ra_s);
			sscanf(dec_string,"%d:%d:%d",&dec_d,&dec_m,&dec_s);
			if(dec_d <0){dec_m *= -1; dec_s *= -1;}
			
			//for the shared memory
			p_tat_info->obs_info.RA  = (float)ra_h + (float)ra_m/60.0 + (float)ra_s/3600.0;
			p_tat_info->obs_info.DEC = (float)dec_d + (float)dec_m/60.0 + (float)dec_s/3600.0;
			strcpy(p_tat_info->obs_info.target_name,target_name);
			
			//CHANGE THE FLAG
			fsetpos(fp, fposition);
			fseek(fp, 0, SEEK_CUR);
			fputc('D', fp);
			fclose(fp);
			free(fposition);
			
			update_flat_dark_strings(filter_string,flat_filter_option,dark_exp_option);
			
			if(ra_h ==0 && ra_m ==0 && ra_s ==0 && dec_d ==0 && dec_m ==0 && dec_s ==0) return -1;
			else return 1;
		}
		else if(end_date.timestamp <= (now.timestamp + HALFHOUR)) // Observation pass
		{
			//CHANGE THE FLAG
			fsetpos(fp, fposition);
			fseek(fp, 0, SEEK_CUR);
			fputc('N', fp);
			fseek(fp, 0, SEEK_CUR);
			continue;
		}
		else if(begin_date.timestamp <= (now.timestamp + FOURHOURS) 
			&& begin_date.timestamp >= (now.timestamp + TENMINUTES))
		{
			//It is not observing time, but there is an observation later
			//Take darks until the begining of next observation
			time (&rawtime);
			ptr_time = localtime(&rawtime);
			ptr_time->tm_year= begin_date.yr - 1900;
			ptr_time->tm_mon= begin_date.mon - 1;
			ptr_time->tm_mday= begin_date.day;
			ptr_time->tm_hour= begin_date.hr;
			ptr_time->tm_min= begin_date.min;
			ptr_time->tm_sec= begin_date.sec;
			ptr_time->tm_isdst= 0;
			p_tat_info->obs_info.end_time=mktime(ptr_time);
			
			fclose(fp);
			free(fposition);
			return -2; //do flat using dark_exp_option
		}
	}
	//no result
	fclose(fp);
	free(fposition);
	return 0;
}

void update_flat_dark_strings(char *filter_string,char *for_flat,char *for_dark)
{
	int i,j,k,l,exp_time;
	char str[100],filter_char[2],filter_pos[FILTER_TOTAL_NUMBER];
	
	//get the char of the filters
	get_filter_array_char(filter_pos);
	//Read the filter string
	
	k=0;
	i=0;
	while(k<FILTER_TOTAL_NUMBER)
	{
		j=0;
		while(filter_string[i]!=')')
		{
			if(filter_string[i] == '\0')break;
			str[j] = filter_string[i];
			i++;
			j++;
		}
		if(filter_string[i] == '\0')break; //get out of main loop
		i++;
		k++;
		str[j]='\0';
		
		for(l=0;l<FILTER_TOTAL_NUMBER;l++)
		{
			if(str[0] == filter_pos[l])
			{
				//Add filter to string
				filter_char[0] = str[0];
				filter_char[1] = '\0';
				add_option_tostring(for_flat,filter_char,0);
				
				//now add the eposure time for dark
				str[0] = ' ';
				str[1] = ' ';
				exp_time = atoi(str);
				if(exp_time >0 && exp_time < 3000)
				{
					sprintf(str,"%d", exp_time);
					add_option_tostring(for_dark,str,1);
				}
			}
		}
	}
}




void add_option_tostring(char *string, char *option, int fordark)
{
	char string_compare[100], option_compare[30];
	
	if(fordark)
	{
		sprintf(string_compare,",%s,",string);
		sprintf(option_compare,",%s,",option);
	}
	else
	{
		strcpy(string_compare,string);
		strcpy(option_compare,option);
	}
	
	if(strstr(string_compare,option_compare) == NULL)
	{
		if(string[0] == '\0') strcpy(string,option);
		else 
		{
			if(fordark)strcat(string,",");
			
			strcat(string,option);
		}
	}
}

