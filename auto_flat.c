/**********************************************************************************************************/
/* To compile it:
gcc auto_flat.c dsp_func-2.c ccd_func-2.c pwr.func.c common.func.c tat.info.c weather_func.o -lcfitsio -lm
*/
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "weather_func.h"
#include "myAstro.h"
#include "fitsio.h"
#include "symblc_const.h"
#include "fli_func.h"

st_tat_info *p_tat_info;

#define THREEMINUTES 1800
#define FAIL_COUNT_MAX 4
#define MAX_LIGHT 30
#define MIN_LIGHT 2
#define AFTER_OBS 0
#define BEFORE_OBS 1

int read_flat_filters(char *FilterString, int *filter_seq);

////////////////////MAIN////////////////////////////////
int main(int argc, char* argv[])
{
	int image_number,shutter_flag, countdown,end_type,i,j,next_k,k;
	int collateral_time;
	int fail_count =0,dark_count=0;
	float exposure_time;
	char date_string[200],date2_string[200], prefix_flatname[10];
	char prefix_darkname[10],command[200];
	char image_filename[200],ccd_command[200], dsp_command[200];
	char filter_type, temp_string[300];
	char ra_dir[4];
	int filter_seq[FILTER_TOTAL_NUMBER]; 
	int good_count[FILTER_TOTAL_NUMBER]={0};
	int seq_control[FILTER_TOTAL_NUMBER]={0};
	int N_filters,current_filter,next_filter,sum_seq_control;
	int max_flats,take_flat;
	DATE lt;
	cloud_info cloud;
	int rely_weather;
	char filter_pos[FILTER_TOTAL_NUMBER];
	
	//// fits variables//////////
	fitsfile *fits_pointer;
	long fpixel[2] = {1,1};
	float *image_array;
	int count_max,count_min;
	int status = 0;
	int num_pixels = ROW_PIXELS*COL_PIXELS;
	int min_value = 10000;
	int max_value = 60000;

	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
	
	
	// ////init values/////////

	image_number = 0;
	shutter_flag = 1;//1=open 0=close
	sprintf(prefix_flatname,"flat%s",SITE);
	sprintf(prefix_darkname,"dark%s",SITE);
	collateral_time = CCD_READOUT_TIME - 2;
	sprintf(ra_dir,"POS");
	
	exposure_time = (float)DoGetValue("FLAT_EXPOSURE_TIME");
	if(exposure_time <0) exposure_time =10;
	max_flats = (float)DoGetValue("MAX_GOOD_FLAT");
	if(max_flats <1) max_flats =6;
	
	////Check what type of flat we are doing
	if(!(argc==2 || argc==3))
	{
		printf("\n*********************************************************************************");
		printf("\nUsage: %s [flat: after/before observation(a/b)] Optional: FilterString\n",argv[0]);
		printf("*****\n The filter string the filter type [A,B,V,R,I,C or N] in order.\n");
                printf("EXAMPLE: %s a NVBR\n",argv[0]);
		printf ("**********************************************************************************\n");
		p_tat_info->obs_info.ccd_status = CCD_IDLE;//0==CCD idle
		return 1;
	}
//FLAT AFTER OR BEFORE OBSERVATION?
	if( !strcmp(argv[1],"a") || !strcmp(argv[1],"A") )
	{
		step("FLAT AFTER OBSERVATION MODE");
		log_this("Auto_flat: Flat after observation starts",AUTO_REPORT_LOG_TYPE,0);
		end_type = AFTER_OBS; //after observation the limit is a saturated exposure
		sprintf(ra_dir,"POS");//for moving away from sun
	}
	else if(!strcmp(argv[1],"b") || !strcmp(argv[1],"B"))
	{
		step("FLAT BEFORE OBSERVATION MODE");
		log_this("Auto_flat: Flat before observation starts",AUTO_REPORT_LOG_TYPE,0);
		end_type = BEFORE_OBS; //before observation the limit is a very dim exposure
		sprintf(ra_dir,"NEG"); //for moving away from sun
	}
	else
	{
		step("ERROR: Flat type unknown");
		p_tat_info->obs_info.ccd_status = CCD_IDLE;//0==CCD idle
		return 1;
	}

	//DID OBSERVER INPUT FILTERSTRING?
	if(argc==3)
	{
		N_filters = read_flat_filters(argv[2],filter_seq);
		if(!N_filters)
		{
			step("ERROR: Filter string has unknown filter");
			p_tat_info->obs_info.ccd_status = CCD_IDLE;//0==CCD idle
			return 1;
		}
		else
		{
			for(i=0;i<N_filters;i++)
					seq_control[i]=1;
		}
	}
	else //filter sequence from shared memory
	{
		N_filters = p_tat_info->obs_info.N_filters;
		if(!N_filters)
		{
			step("ERROR: Filter in shared memory has unknown filter");
			p_tat_info->obs_info.ccd_status = CCD_IDLE;//0==CCD idle
			return 1;
		}
		
		sprintf(temp_string," ");
		for(i=0;i<N_filters;i++)
		{
			filter_seq[i] = p_tat_info->obs_info.filter_seq[i];
			sprintf(temp_string,"%sFilter %d: %d \n",temp_string,i+1,filter_seq[i]);
			seq_control[i]=1;
		}
		step(temp_string);
	}
	//Get Date and create directory/////////////
	lt= getSystemTime();//function in date.c
	sprintf(date_string, "%d%02d%02d", lt.yr, lt.mon, lt.day);//Example :20080530

	sprintf(command,"mkdir -p %s/%s", CALIBRATE_PATH,date_string);
	system(command);

	log_this("Start auto_flat",AUTO_FLAT_LOG_TYPE,1);
	
	//start taking images
	p_tat_info->obs_info.ccd_status = CCD_DOING_FLAT;//4==taking flat
	
	current_filter = filter_seq[0];
	
	//Move to the filter
	move_to_filter(current_filter);
	

	get_filter_array_char(filter_pos);
	//Show filters
	sprintf(temp_string,"***");
	for(j=0,sum_seq_control=0;j<N_filters;j++)
	{
		i = filter_seq[j];
		sprintf(temp_string,"%s\n %d: Filter %d [%c] (%d)",
				temp_string,j+1,i,filter_pos[i], seq_control[j]);
		sum_seq_control += seq_control[j];
	}
	steplog(temp_string,AUTO_FLAT_LOG_TYPE);

	k=0;
	while(sum_seq_control) //do flat while the counter is ON
	{
		p_tat_info->obs_info.ccd_status = CCD_DOING_FLAT;//4==taking flat
		take_flat =1;
		if(p_tat_info->fli_info.wheel_moving)
		{
			printf("Waiting for the end of movement of filter wheel\n");
			while(p_tat_info->fli_info.wheel_moving) sleep(1);
			printf("\nFilter wheel movement finished\n");
		}
		
		p_tat_info->obs_info.ccd_status = CCD_DOING_FLAT;//4==taking flat
		printf("Current filter %d\n",current_filter);
		filter_type = p_tat_info->obs_info.filter_type;

		lt= getSystemTime();//function in date.c
		sprintf(date2_string, "%02d%02d%02d_%02d%02d%02d",lt.yr-2000, lt.mon, lt.day,lt.hr,lt.min,lt.sec);//080530_033211
		
		//get the cloud options
		rely_weather = get_cloud_condition (&cloud);
	
		if((lt.timestamp - cloud.timekey)>THREEMINUTES ) rely_weather =-1;
		else printf("Current light = %.1f\n",cloud.light);
	
		if(rely_weather>=0 && cloud.light < MIN_LIGHT)
		{
			take_flat=0; //Take dark if light measured is low
			if(end_type==BEFORE_OBS) 
			{
				sprintf(temp_string,"WARNING: Current light %.1f. Abort Flat Fielding",cloud.light);
				steplog(temp_string,AUTO_FLAT_LOG_TYPE);
				log_this("Auto_flat: Too dim. Abort Flat Fielding.",AUTO_REPORT_LOG_TYPE,0);
				break; //before observation ==> take flat finish 
			}
		}
		else if(rely_weather>=0 && cloud.light > MAX_LIGHT && end_type == AFTER_OBS)
		{
			sprintf(temp_string,"WARNING: Current light %.1f. Abort Flat Fielding",cloud.light);
			steplog(temp_string,AUTO_FLAT_LOG_TYPE);
			log_this("Auto_flat: Too bright. Abort Flat Fielding.",AUTO_REPORT_LOG_TYPE,0);
			break; //start observation ==> take flat finish
		}
		else
			take_flat=1; //Take flat


		if(take_flat)
		{
			sprintf(image_filename,"%s/%s/%c%s%sEx%d.fit", CALIBRATE_PATH, date_string, filter_type, prefix_flatname,
				date2_string,(int) exposure_time);
			shutter_flag = 1; // 1 = open 0 = close
		}
		else
		{
			sprintf(image_filename,"%s/%s/%s%sEx%d.fit", CALIBRATE_PATH, date_string,  prefix_darkname,
				date2_string,(int) exposure_time);
			
			shutter_flag = 0;// 1 = open 0 = close
			dark_count++;
		}
		
		image_number++;
		sprintf(temp_string,"****\n Image %d: \n%s\n", image_number, image_filename);
		step(temp_string);
		
		// creates file for ccd daemon to read
		sprintf(ccd_command, "camera takeimage %s %.2f  %d 1",image_filename, exposure_time, shutter_flag);
		send_cmd2ccd(ccd_command);
		
		sprintf(temp_string,"Image %d:\n%s",image_number,ccd_command);
		steplog(temp_string,AUTO_FLAT_LOG_TYPE);
		
		////Wait for read-out time
		for (countdown = (int) exposure_time + CCD_READOUT_TIME; countdown >= 1; countdown--)
		{
			printf("\rCountdown %4d, ",  countdown);fflush(NULL);
			p_tat_info->obs_info.ccd_status = CCD_DOING_FLAT;//4==taking flat
			if(countdown == CCD_READOUT_TIME && sum_seq_control > 1) //move to the next filter
			{
				next_filter = -1;
				next_k = k;
				while(next_filter <0)
				{
					next_k++;
					if(next_k >= N_filters)next_k=0;
					
					if(seq_control[next_k])
						next_filter = filter_seq[next_k];
				}
				if(next_k != k) // more than one filter
				{
					move_to_filter(next_filter);
					sprintf(temp_string,"Moving to filter %d",next_filter);
					steplog(temp_string,AUTO_FLAT_LOG_TYPE);
				}
			}
			if (good_count[current_filter] && countdown == collateral_time)//move a little the telescope every good exposure
			{
				sprintf(dsp_command,"RA %s FREQUENCY 1 3 417 0 0 0 0",ra_dir);
				send_cmd2dsp(dsp_command);
				steplog("Moving the telescope a little for 3 s",AUTO_FLAT_LOG_TYPE);
			}
			sleep(1);
		}
		sleep (1);

		if(take_flat) 
		{
			// /////////////analize image////////////////////////
			status = 0;
			fits_open_file(&fits_pointer, image_filename , READONLY, &status);
			image_array = (float *)malloc(sizeof(float)*(num_pixels+1));
			fits_read_pix(fits_pointer, TFLOAT,fpixel,num_pixels,NULL,image_array,NULL,&status);
			fits_close_file(fits_pointer,&status);
			if (status)
			{//if there is an error
				fits_report_error(stderr,status);
				sprintf(temp_string,"ERROR: Cannot read file %s (%d)",image_filename,fail_count++);
				steplog(temp_string,AUTO_FLAT_LOG_TYPE);
				//Exit if too many errors
				if(fail_count > FAIL_COUNT_MAX)
				{
					steplog("ERROR: Too many reading file errors",AUTO_FLAT_LOG_TYPE);
					log_this("Auto_flat: Too many reading file errors",AUTO_REPORT_LOG_TYPE,0);
					p_tat_info->obs_info.ccd_status = CCD_IDLE;//0==CCD idle
					free(image_array);
					return 1;
				}
			}
			else //if everything is ok
			{
				fail_count =0;
				for(i=1,count_max=count_min=0; i<num_pixels; i++)
				{
					if(image_array[i] > max_value)
						count_max++;
					if(image_array[i] < min_value)
						count_min++;
				}

				if (count_max > 550000)
				{
					sprintf(temp_string,"%s = SATURATED",image_filename);
					steplog(temp_string,AUTO_FLAT_LOG_TYPE);
					if(end_type==AFTER_OBS)seq_control[k]=0; //end_type=0 after observation the limit is a saturated exposure
					remove(image_filename);//Saturated-> remove it
					sprintf(temp_string,"Removing %s",image_filename);
					step(temp_string);
				}
				else if (count_min > 550000)
				{
					sprintf(temp_string,"%s = TOO DIM",image_filename);
					steplog(temp_string,AUTO_FLAT_LOG_TYPE);
					if(end_type==BEFORE_OBS)seq_control[k]=0;//end_type=1 before observation the limit is a dim exposure
					remove(image_filename);//Too dim -> remove it
					sprintf(temp_string,"Removing %s",image_filename);
					step(temp_string);
				}
				else
				{ //good flat
					sprintf(temp_string,"### %s = GOOD %d",image_filename,++good_count[current_filter]);
					steplog(temp_string,AUTO_FLAT_LOG_TYPE);
					if(good_count[current_filter]>=max_flats) //Too many good, end flat fielding
								seq_control[k]=0;
				}
			}//else (if status)
			//release memory 
			free(image_array);
			//end of analising image/////////////////////////////
		}
		//prepare for the next cycle
		k = next_k;
		current_filter = next_filter;
		//sum of the filter secuence counter
		for(j=0,sum_seq_control=0;j<FILTER_TOTAL_NUMBER;j++)
			sum_seq_control += seq_control[j];
	}
	
	steplog("### Normal auto_flat finish");
	p_tat_info->obs_info.ccd_status = CCD_IDLE;//0==CCD idle
	
	/*Report*/
	sprintf(temp_string,"Auto_flat taken images:\n");
	for(j=0;j<FILTER_TOTAL_NUMBER;j++)
	{
		i=filter_seq[j];
		if(good_count[j]) sprintf(temp_string,"%s Filter %c - %d\n",
							temp_string,filter_pos[i] ,good_count[j]);
		
	}
	if(dark_count) sprintf(temp_string,"%s Dark currents %d", 
									temp_string,dark_count);
	log_this(temp_string,AUTO_REPORT_LOG_TYPE,0);
	
	
	return 0;
}



int read_flat_filters(char *FilterString, int *filter_seq)
{
	int N_filters,i,j,l;
	char filter_pos[FILTER_TOTAL_NUMBER];
	
	get_filter_array_char(filter_pos);
	
	l=strlen(FilterString);
	N_filters=0;
	
	for(i=0;i<l;i++)
	{
		for(j=0;j<FILTER_TOTAL_NUMBER;j++)
		{
			if(FilterString[i] == filter_pos[j])
			{
				filter_seq[N_filters] = j;
				N_filters++;
				j = 10; //EXIT LOOP
			}
		}
	}
	return N_filters;
}

