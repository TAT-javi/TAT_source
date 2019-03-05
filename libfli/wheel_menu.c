#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

#include "libfli.h"

#define MAX_DEVICES 5
#define BUFF_SIZE 1024
#define MAX_PATH 1024

int get_devices(flidomain_t domain);
void print_menu(void);

char *list_name[MAX_DEVICES];
long list_domain[MAX_DEVICES];


int main(int argc, char *argv[])
{
	int out=0,Ndevs,choice,tempi;
	char  libver[1024],model[3000];
	long status,max_focuser,position,remain_steps,moving_step;
	long current_filter;
	flidev_t dev;
	
	if(FLIGetLibVersion(libver, 1024) != 0)
	{
		printf("Unable to retrieve library version!\n");
		return 1;
	}
	printf("Library version '%s'\n", libver); 
	Ndevs = get_devices(FLIDOMAIN_USB | FLIDEVICE_FILTERWHEEL);
	
	if(!Ndevs)
	{
		printf("ERROR: No FLI devices have been detected\n");
		return 1;
	}
	else if(Ndevs>1)
	{
		printf("ERROR: More than one focuser detected (%d)\n",Ndevs);
		return 1;
	}
	
	status=FLIOpen(&dev,list_name[0],list_domain[0]);
	if(status)
	{
			printf("Error opening the dev (%d)\n",status);
			return 1;
	}

	
	if(FLIGetModel(dev, model, 3000) !=0)
	{
		printf("ERROR: Reading  FLI device model\n");
		return 1;
	}
	
	printf("FLI model %s\n",model);
	getchar();
	system("clear");
	moving_step = 1000;	
	FLIGetFocuserExtent(dev,&max_focuser);
	while(!out)
	{
		FLIGetStepperPosition(dev,&position);
		FLIGetStepsRemaining(dev,&remain_steps);
		FLIGetFilterPos(dev,&current_filter);	
		if(remain_steps)
			printf("Wheel moving. Remainding steps %d\n",remain_steps);
		else
			printf("Wheel is NOT moving\n");
		
		printf("Current filter = %ld\n",current_filter);
		
		print_menu();
		
		scanf("%d",&choice);
		switch(choice)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				FLISetFilterPos(dev,(long)choice);	
			break;
			case 7: 
				out =1; 
			break;
			default:
				continue;
			break;
		}
		printf("\n#################################################################\n");
	}
	FLIClose(dev);
	printf("### END OF PROGRAM ###\n");
	return 0;
}






int get_devices(flidomain_t enum_domain)
{
	int N = 0;
	char file[MAX_PATH], name[MAX_PATH];
	long domain;

	FLICreateList(enum_domain);

	if(FLIListFirst(&domain, file, MAX_PATH, name, MAX_PATH) == 0)
	{
		do
		{
			if(name[0] == 'C') //Check if it is the CFW
			{
				list_name[N] = (char*)malloc(strlen(file) + 1);
				strcpy(list_name[N], file);
				list_domain[N] = domain;
				N++;
			}
		}
		while((FLIListNext(&domain, file, MAX_PATH, name, MAX_PATH) == 0) && (N < MAX_DEVICES));
	}
	FLIDeleteList();
	
	return N;
}


void print_menu()
{
	printf("[0-6] - Move to filter\n");
	printf("7 - Exit\n=> ");
}
