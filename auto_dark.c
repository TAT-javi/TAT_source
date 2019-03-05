/**********************************************************************************************************/
/* To compile it:
gcc auto_dark.c getdate.c ccd_func.c ppc_func.c common.c tat.info.c -lncurses -lm
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "myAstro.h"
#include "symblc_const.h"


#define TEMP_TOLERANCE 2
#define MAX_EXP_TIMES 50

st_tat_info *p_tat_info;

void do_dark(int *exp_time,int Nexp, char *date_string,int N_cycles);
int get_exp_times(char *exp_string,int *exp);

////////////////////MAIN////////////////////////////////
int main(int argc, char* argv[])
{
	int i,N_cycles,Nexp, exp_time[MAX_EXP_TIMES];
	char date_string[50],command[200],temp_string[200];
	DATE lt;

	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
	
	//////check the command and get the exposure time////
	if (argc != 3)
	{
		printf ("\n************************************************************************************");
		printf ("\nUsage: %s [exposure times][number of each exposure]",argv[0]);
		printf ("\n*** Note: 'exposure times' = 0, use the exposure times in the shared memory\n");
		printf ("\n*** Note2: 'exposure times' could be several integers serparated by commas and no spaces:\n\t Example: %s 200,220,110 10\n",argv[0]);
		printf ("\n***********************************************************************************\n");
		p_tat_info->obs_info.ccd_status = CCD_IDLE;//CCD idle
		return 1;
	}

	Nexp=get_exp_times(argv[1],exp_time);
	if(!Nexp) 
	{
		steplog("ERROR: No acceptable exposure times!!",AUTO_OBSERVE_LOG_TYPE);
		p_tat_info->obs_info.ccd_status = CCD_IDLE;//CCD idle
		return 1;
	}
	
	//Check if the user want to use the shared memory
	if(Nexp==1 && !exp_time[0]) 
	{
		Nexp= p_tat_info->obs_info.N_filters;
		if(!Nexp)
		{
			steplog("ERROR: No exposure times in shared memory!!",AUTO_OBSERVE_LOG_TYPE);
			p_tat_info->obs_info.ccd_status = CCD_IDLE;//CCD idle
			return 1;
		}
		for(i=0;i<Nexp;i++)
			exp_time[i] = (int) p_tat_info->obs_info.filter_exp_time[i];
	}
	
	//Show the exp times
	sprintf(temp_string,"Number of exposures: %d\n",Nexp);
	for(i=0;i<Nexp;i++)
		sprintf(temp_string,"Exposure %d = %d\n",i+1,exp_time[i]);
	
	step(temp_string);
	
	N_cycles = atoi(argv[2]);

	if(N_cycles<1 || N_cycles > 5000)
	{
		sprintf(temp_string,"ERROR: Incorrect number of total images (%d), it must be 0 < N < 5000",
				N_cycles);
		steplog(temp_string,AUTO_OBSERVE_LOG_TYPE);
		p_tat_info->obs_info.ccd_status = CCD_IDLE;//CCD idle
		return 1;
	}

	//Get Date and create directory/////////////
	p_tat_info->obs_info.ccd_status = CCD_DARK;//3=taking dark
	lt= getSystemTime();//function in date.c
	sprintf(date_string, "%04d%02d%02d", lt.yr, lt.mon, lt.day);//Example :20080530

	sprintf(command,"mkdir -p %s/%s",CALIBRATE_PATH, date_string);
	system(command);
	
	//start taking images
	log_this("Auto_dark Starts",AUTO_DARK_LOG_TYPE,1);
	do_dark(exp_time,Nexp,date_string,N_cycles);
	steplog("### Auto_dark normal end",AUTO_DARK_LOG_TYPE);

	p_tat_info->obs_info.ccd_status = CCD_IDLE;//CCD idle
	return 0;
}


void do_dark(int *exp_time,int Nexp, char *date_string,int N_cycles)
{
	int image_number, total_images, countdown, counter;
	char image_filename[200],ccd_command[200],date2_string[200], prefix_name[10];
	char temp_string[300];
	DATE lt;

	sprintf(prefix_name,"dark%s",SITE);
	
	//////
	//Take a number of total_images for each exposure;
	total_images = N_cycles* Nexp;
	
	for(image_number=0,counter=0; image_number < total_images;image_number++,counter++ )
	{
		if(counter>=Nexp) counter=0;
		
		lt= getSystemTime();//function in date.c
		sprintf(date2_string, "%02d%02d%02d_%02d%02d%02d",
				lt.yr-2000, lt.mon, lt.day,lt.hr,lt.min,lt.sec);//080530_033211
		sprintf(image_filename,"%s/%s/%s%sEx%d.fit",CALIBRATE_PATH, date_string, 
				prefix_name, date2_string, exp_time[counter]);
		sprintf(temp_string,"********\n Image %d: \n%s\n", image_number+1, image_filename);
		step(temp_string);
		// creates file for ccd daemon to read
		sprintf(ccd_command, "camera takeimage %s %d  0 1",
					image_filename, exp_time[counter]);
		send_cmd2ccd(ccd_command);
		
		sprintf(temp_string,"Image %d: %s", image_number,ccd_command);
		steplog(temp_string,AUTO_DARK_LOG_TYPE);
		////Wait for read-out time
		for (countdown = exp_time[counter] + CCD_READOUT_TIME; countdown >= 2; countdown--)
		{
			printf("\rCountdown %4d, ",  countdown);fflush(NULL);
			sleep(1);
		}
		sleep(2);
	}//for(image_number=0,counter=0; image_number != total_images; image_number++,counter++)

	// Report
	sprintf(temp_string,"Auto dark images:");
	for(counter=0;counter<Nexp;counter++)
	{
		if(counter)sprintf(temp_string,"%s,",temp_string);
		sprintf(temp_string,"%s Exp time %d (%d)",
				temp_string, exp_time[counter], N_cycles);
	}
	log_this(temp_string,AUTO_REPORT_LOG_TYPE,0);
	steplog(temp_string,AUTO_OBSERVE_LOG_TYPE);
}//do_dark(float exposure_time,int total_images,char *date_string)


int get_exp_times(char *exp_string,int *exp)
{
	int Nexp,i,j,out;
	char str[50],c,temp_string[200];

	out =0;
	i=0;
	Nexp=0;
	while(!out && Nexp <= MAX_EXP_TIMES)
	{
		j=0;
		while(exp_string[i] !=',' && exp_string[i]!='\0')
		{
			str[j] = exp_string[i]; i++;j++;
		}
		if(j>0) //Everything OK
		{
			str[j]='\0';
			exp[Nexp] = atoi(str);
			if (exp[Nexp] > 10300 || exp[Nexp] < 0)
			{
				sprintf(temp_string,"ERROR: Incorrect exposure time (%d), it must be 0 < exp.time < 10300",exp[Nexp]);
				step(temp_string);
				p_tat_info->obs_info.ccd_status = CCD_IDLE;//CCD idle
				return 0;
			} else if (!(!exp[Nexp]&&Nexp)) Nexp++; //remove any exp= 0 if Nexp!=0

			if(exp_string[i] == ',') i++;
			else out=1;
		}else out=1;
	}
	
	//Check that the first image is zero and Nexp>2
	if(Nexp>1 && exp[0]==0)
	{
		for(i=1;i<Nexp;i++) 
			exp[i-1]=exp[i];
	
		Nexp--;
	}
	
	return Nexp;
}
