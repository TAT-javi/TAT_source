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
void print_menu(int moving_step);

char *list_name[MAX_DEVICES];
long list_domain[MAX_DEVICES];


int main(int argc, char *argv[])
{
	int out=0,Ndevs,choice,tempi;
	char  libver[1024],model[3000];
	long status,max_focuser,position,remain_steps,moving_step;
	flidev_t dev;
	double temperature_in,temperature_out;
	
	if(FLIGetLibVersion(libver, 1024) != 0)
	{
		printf("Unable to retrieve library version!\n");
		return 1;
	}
	printf("Library version '%s'\n", libver); 
	Ndevs = get_devices(FLIDOMAIN_USB | FLIDEVICE_FOCUSER);
	
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
		FLIReadTemperature(dev,FLI_TEMPERATURE_INTERNAL,&temperature_in);
		FLIReadTemperature(dev,FLI_TEMPERATURE_EXTERNAL,&temperature_out);

		if(remain_steps)
			printf("Focuser moving. Remainding steps %d\n",remain_steps);
		else
			printf("Focuser is NOT moving\n");
		
		printf("Focuser max extent = %ld\nCurrent position = %ld\n",max_focuser,position);
		printf("Temperature IN = %.02lf\n",temperature_in);
		printf("Temperature OUT = %.02lf\n",temperature_out);
		print_menu(moving_step);
		
		scanf("%d",&choice);
		switch(choice)
		{
			case 1:
				printf("Moving to which position (0 -- 105000)? \n=> ");
				scanf("%d",&tempi);
				if(tempi < 0 || tempi >105000)
					printf("ERROR: Input out of bounds (%d)\n",tempi);
				else
					FLIStepMotorAsync(dev, ((long)tempi)-position);

			break;
			case 2:
				FLIStepMotor(dev, moving_step);
			break;
			case 3:
				FLIStepMotor(dev, moving_step*(-1));
			break;
			case 4:
				
				printf("New moving step? \n=> ");
				scanf("%d",&tempi);
				
				if(tempi < 0 || tempi >100000)
					printf("ERROR: Input out of bounds (%d)\n",tempi);
				else
					moving_step = (long) tempi;
				
			break;
			case 5:
				FLIHomeFocuser(dev);
			break;
			case 6:
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
			if(name[0] == 'A') // Check if it is the Atlas Focuser
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


void print_menu(int moving_step)
{
	printf("1 - Move to position\n");
	printf("2 - Move %d out\n",moving_step);
	printf("3 - Move %d in\n",moving_step);
	printf("4 - Change moving step (%d)\n",moving_step);
	printf("5 - Move to Home position\n");
	printf("6 - Exit\n");
	printf("Any key - Update current step count\n=> ");
}
