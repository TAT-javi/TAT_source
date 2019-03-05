#include "common_func.h"
#include "focuser_func.h"
#include <fcntl.h>
#include "libfli.h"

extern st_tat_info *p_tat_info;

#define MAX_PATH 1024
#define LABEL 'A' //C for filter wheel A for focuser

void* move_focuser(void* moveto)
{
	int N,home_focuser;
	long move_to_pos,move_rel_pos,curr_position;
	char name[MAX_PATH],file[MAX_PATH],temp_file[MAX_PATH];
	long status,domain,temp_domain,max_focuser,remain_steps,moving_step;
	flidev_t dev;
	MOVEFOCUSER *p;
	
	p = (MOVEFOCUSER*) moveto;
	
	home_focuser = p->home_focuser;
	move_rel_pos = p->move_rel_pos;
	move_to_pos = p->move_position;
	
	//Connect to the focuser device
	FLICreateList(FLIDOMAIN_USB | FLIDEVICE_FOCUSER);
	if(FLIListFirst(&temp_domain, temp_file, MAX_PATH, name, MAX_PATH))
	{
		printf("%s ERROR %s: Could not find the focuser\n",ANSI_COLOR_RED,ANSI_COLOR_RESET);
		return NULL;
	}
	//else
	N=0;
	do
	{
		if(name[0] == LABEL) //Make sure is the right device
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
		printf("%s ERROR %s: Could not find the focuser\n",ANSI_COLOR_RED,ANSI_COLOR_RESET);
		return NULL;
	}
	//OPEN THE DEVICE
	status=FLIOpen(&dev,file,domain);
	if(status)
	{
		printf("%s ERROR %s: Could not open the focuser (%d)\n",status);
		return NULL;
	}
	
	//Check if it is moving
	FLIGetStepsRemaining(dev,&remain_steps);
	if(remain_steps)
	{
		printf("%s Focuser moving %s. Remainder steps %d\n",
						ANSI_COLOR_BLUE,ANSI_COLOR_RESET,remain_steps);
		FLIClose(dev);
		return NULL;
	}
	
	//GET MAX EXTENT
	FLIGetFocuserExtent(dev,&max_focuser);
	p_tat_info->fli_info.focuser_extent=max_focuser;
	//GET CURRENT POSITION
	FLIGetStepperPosition(dev,&curr_position);
	
	//Check command
	if(home_focuser) //Home the device
	{
		p_tat_info->fli_info.focuser_moving=1;
		FLIHomeFocuser(dev);
	}
	else if(move_to_pos>=0 ) //move to position in bounds
	{
		if(move_to_pos >=max_focuser) move_to_pos = max_focuser -1;
		
		p_tat_info->fli_info.focuser_moving=1;
		FLIStepMotor(dev, move_to_pos-curr_position);
	}
	else if(move_rel_pos!=0) // move to a relative position
	{
		p_tat_info->fli_info.focuser_moving=1;
		FLIStepMotor(dev, move_rel_pos);
	}
	else
	{
		printf("%s ERROR %s: Command not valid\n",ANSI_COLOR_RED,ANSI_COLOR_RESET);
	}
	
	FLIGetStepperPosition(dev,&curr_position);
	FLIClose(dev);
	
	p_tat_info->fli_info.focuser_curr_position=curr_position;
	p_tat_info->fli_info.focuser_moving=0;
	
	return NULL;
}

