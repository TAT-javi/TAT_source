#include "common_func.h"
#include "wheel_func.h"
#include <fcntl.h>
#include "libfli.h"

#define MAX_PATH 1024
#define LABEL 'C' //C for filter wheel; A for focuser

extern st_tat_info *p_tat_info;

void* move_wheel(void* moveto)
{
	int N,step, position,count;
	char filter_type,search_string[20];
	char file[MAX_PATH],name[MAX_PATH],temp_file[MAX_PATH];
	long status,domain,temp_domain,remain_steps;
	MOVEWHEEL *p;
	flidev_t dev;
	
	p = (MOVEWHEEL*) moveto;
	position= p->filter_position;
	

	//Connect to the filter wheel device
	FLICreateList(FLIDOMAIN_USB | FLIDEVICE_FILTERWHEEL);
	if(FLIListFirst(&temp_domain, temp_file, MAX_PATH, name, MAX_PATH))
	{
		printf("%s ERROR %s: Could not find the filter wheel\n",ANSI_COLOR_RED,ANSI_COLOR_RESET);
		return NULL;
	}
	//else
	N=0;
	do
	{
		if(name[0] == LABEL)//Make sure is the right device
		{
			strcpy(file,temp_file);
			domain = temp_domain;
			N++;
		}
	}
	while((FLIListNext(&temp_domain, temp_file, MAX_PATH, name, MAX_PATH) == 0));
	
	FLIDeleteList();
	
	if(!N)
	{
		printf("%s ERROR %s: Could not find the filter wheel\n",ANSI_COLOR_RED,ANSI_COLOR_RESET);
		return NULL;
	}
	//OPEN THE DEVICE
	status=FLIOpen(&dev,file,domain);
	if(status)
	{
		printf("%s ERROR %s: Could not open the filter wheel (%d)\n",status);
		return NULL;
	}
	
	//Check if it is moving
	FLIGetStepsRemaining(dev,&remain_steps);
	if(remain_steps)
	{
		printf("%s Filter wheel is moving %s. Remainder steps %d\n",
						ANSI_COLOR_BLUE,ANSI_COLOR_RESET,remain_steps);
		FLIClose(dev);
		return NULL;
	}
	
	//Move the filter
	p_tat_info->fli_info.wheel_moving=1;
	FLISetFilterPos(dev,(long)position);
	p_tat_info->fli_info.wheel_moving=0;
	FLIClose(dev);
	
	//get the filter position string from the tat_parameter.dat
	sprintf(search_string,"FILTER_POS%d",position);
	search_string[11]='\0';
	filter_type=DoGetValueChar(search_string);
	////////
	p_tat_info->obs_info.current_filter=position;
	p_tat_info->obs_info.filter_type = filter_type;
	p_tat_info->fli_info.wheel_curr_position=position;
	p_tat_info->fli_info.wheel_moving=0;

	return NULL;
}

