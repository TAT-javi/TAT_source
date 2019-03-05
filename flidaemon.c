#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symblc_const.h"
#include "libfli.h"

#define CYCLE_TIME 100000	/* microseconds usleep */
#define MAX_PATH 1024

st_tat_info *p_tat_info;

int get_file_domain(char *file, long *domain, char label);
int move_wheel(char *file,long domain,int Movement);
int move_focuser(char *file,long domain,int Movement);

int main(int argc, char *argv[])
{
	int i,j;
	pid_t pid;
	FILE *CommandFile,*fp;
	char WhichDevice[5],label;
	int Movement;
	unsigned char startAsDaemon=0;
	
	//For libfli
	char name[MAX_PATH],file[MAX_PATH],temp_file[MAX_PATH];
	long domain,temp_domain, status,remain_steps;
	flidev_t dev;
	
	
	if(argc==2 )
	{
		if( strcmp(argv[1],"-d")==0)
		{
			startAsDaemon=1;
		}
	}
	
	while(1)
	{
		pid=fork();
		if(pid==-1)
		{
			/*      error*/
			perror("-1");
		}
		else if(pid!= 0)
		{
		/*      parent*/
			if(startAsDaemon)
			{
				printf("FLIDaemon start as Daemon now\n");
				exit( 0);
			}
			else
			{
				printf("wait child pid =%d\n",pid);
				waitpid(pid,(int *)0,0);
				printf("child pid= %d exit\n", pid);
				
// 				time(&currTime);
// 				pcCurrTime= ctime( &currTime);
				fp= fopen(DAEMON_LOG_FILENAME, "a+");
// 				fprintf( fp, "%s flidaemon child pid=%d exit.\n", pcCurrTime, pid);
				fprintf( fp, "flidaemon child pid=%d exit.\n",  pid);
				fclose( fp);
			}
		}
		else
		{
	/*      child*/
			break;
		}
	}       /*end while */

	remove (FLI_CMD_FILENAME);
	printf("FLI Daemon Start.\n");
	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);

	while(1)
	{
		if( ( CommandFile = fopen(FLI_CMD_FILENAME, "r") ) == NULL )
		{
			usleep(CYCLE_TIME);	/*Command file not FOUND!*/
		}
		else
		{
			fscanf(CommandFile, "%s %d\n", WhichDevice, &Movement);
			printf("Device:%s, where:%d,\n",WhichDevice, Movement);

			if( !strcmp(WhichDevice, "WHEEL"))label='C';
			else if(!strcmp(WhichDevice, "FOCUS"))label='A';
			else label='0';
			fclose(CommandFile);
			remove(FLI_CMD_FILENAME);
			///READ DEVICE
			
			//Connect to the focuser device
			if(label!='0')
			{
				i=get_file_domain(file, &domain, label);
				if(!i && label == 'A') //Focuser
					j=move_focuser(file,domain,Movement);
				else if(!i && label == 'C') //filter wheel
					j=move_wheel(file,domain,Movement);
				
				
				if(!j) printf("ERROR\n");
			}
		}
	}	/* while(1)*/
	return 0;
}

int get_file_domain(char *file, long *domain, char label)
{
	int N;
	char temp_file[MAX_PATH],name[MAX_PATH];
	long temp_domain;
	
	if(label=='A') //FOCUSER
		FLICreateList(FLIDOMAIN_USB | FLIDEVICE_FOCUSER);
	else //WHEEL
		FLICreateList(FLIDOMAIN_USB | FLIDEVICE_FILTERWHEEL);
				
				
	if(FLIListFirst(&temp_domain, temp_file, MAX_PATH, name, MAX_PATH))
	{
		printf("ERROR: Could not find the device\n");
		return 1;
	}
	//else
	N=0;
	do
	{
		if(name[0] == label) //Make sure is the right device
		{
			strcpy(file,temp_file);
			*domain = temp_domain;
			N++;
		}
	}
	while((FLIListNext(&temp_domain, temp_file, MAX_PATH, name, MAX_PATH) == 0));
	FLIDeleteList();
	return 0;
}


int move_focuser(char *file,long domain,int Movement)
{
	long status,remain_steps,max_focuser,curr_position;
	flidev_t dev;
	
	//OPEN THE DEVICE
	status=FLIOpen(&dev,file,domain);
	if(status)
	{
		printf(" ERROR : Could not open the focuser (%d)\n",
			   status);
		return 0;
	}
	
	//Check if it is moving
	FLIGetStepsRemaining(dev,&remain_steps);
	if(remain_steps)
	{
		printf(" Focuser moving. Remainder steps %d\n",
						remain_steps);
		FLIClose(dev);
		return 0;
	}
	
	//GET MAX EXTENT
	FLIGetFocuserExtent(dev,&max_focuser);
	p_tat_info->fli_info.focuser_extent=max_focuser;
	//GET CURRENT POSITION
	FLIGetStepperPosition(dev,&curr_position);

	//Check command
	if(!Movement) //Home the device
	{
		p_tat_info->fli_info.focuser_moving=1;
		FLIHomeFocuser(dev);
	}
	else if(Movement>=0 ) //move to position in bounds
	{
		if(Movement >=max_focuser) Movement = max_focuser -1;
		
		p_tat_info->fli_info.focuser_moving=1;
		FLIStepMotor(dev, Movement-curr_position);
	}
// 	else if(move_rel_pos!=0) // move to a relative position
// 	{
// 		p_tat_info->fli_info.focuser_moving=1;
// 		FLIStepMotor(dev, move_rel_pos);
// 	}
// 	else //Movement < 0 just send current position
// 	{
// 		printf(" ERROR : Command not valid\n");
// 	}
	FLIGetStepperPosition(dev,&curr_position);

	p_tat_info->fli_info.focuser_curr_position=curr_position;
	p_tat_info->fli_info.focuser_moving=0;
	FLIClose(dev);
	return 1;
}



int move_wheel(char *file,long domain,int Movement)
{
	char filter_type, search_string[12];
	long status,remain_steps;
	flidev_t dev;
	
	if(Movement >6 || Movement <0)
	{
		printf(" ERROR : Filter wheel position out of bounds (%d)\n",
			   Movement);
		return 0;
	}
	
	status=FLIOpen(&dev,file,domain);
	if(status)
	{
		
		printf(" ERROR : Could not open the filter wheel (%d)\n",
			   status);
		return 0;
	}
	
	//Check if it is moving
	FLIGetStepsRemaining(dev,&remain_steps);
	if(remain_steps)
	{
		printf("Filter wheel is moving . Remainder steps %d\n",
						remain_steps);
		FLIClose(dev);
		return 0;
	}
	
	//Move the filter
	p_tat_info->fli_info.wheel_moving=1;
	FLISetFilterPos(dev,(long)Movement);
	FLIClose(dev);
	
	//get the filter position string from the tat_parameter.dat
	sprintf(search_string,"FILTER_POS%d",Movement);
	search_string[11]='\0';
	filter_type=DoGetValueChar(search_string);
	
	////////
	p_tat_info->obs_info.current_filter=Movement;
	p_tat_info->obs_info.filter_type = filter_type;
	p_tat_info->fli_info.wheel_curr_position=Movement;
	p_tat_info->fli_info.wheel_moving=0;
	return 1;
}
