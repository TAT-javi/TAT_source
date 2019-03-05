#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

#include "libfli.h"

#define MAX_DEVICES 5
#define BUFF_SIZE 1024
#define MAX_PATH 1024

void EnumerateDevices(flidomain_t enumDomain);


long status = 0;

int numDevs = 0;
char *listName[MAX_DEVICES];
long listDomain[MAX_DEVICES];


int main(int argc, char *argv[])
{
	int i,status;
	long filter,f,N;
	char  libver[1024];
	flidev_t dev;	

	if(argc != 2)
	{
		printf("Usage: %s [filter Number]\n",argv[0]);
		return 1;
	}
	
	filter = atol(argv[1]);
	if(FLIGetLibVersion(libver, 1024) != 0)
	{
		printf("Unable to retrieve library version!\n");
		return 1;
	}
	printf("Library version '%s'\n", libver);
	
	//EnumerateDevices(FLIDOMAIN_USB | FLIDEVICE_FILTERWHEEL);
	EnumerateDevices(FLIDOMAIN_USB | FLIDEVICE_FOCUSER);

	
	
	if(numDevs == 0)
	{
		printf("\nNo FLI devices have been detected\n");
		return 1;
	}
	for(i=0;i<numDevs;i++)
	{
		printf("%s\n",listName[i]);
	}

	
	status=FLIOpen(&dev,listName[0],listDomain[0]);
	if(status)
	{
		printf("Error opening the dev %d\n",status);
		return 1;
	}
	//FLIGetFilterCount(dev,&N);

	//printf("Number of filters %ld\n",N);

	//FLIGetFilterPos(dev,&f);
	//printf("Current filter %ld\tMoving to %ld\n",f,filter);
	//FLISetFilterPos(dev,filter);
	//printf("END OF PROGRAM\n");
	FLIClose(dev);

	
	
	return 0;
}



void EnumerateDevices(flidomain_t enumDomain)
{
	numDevs = 0;
	char file[MAX_PATH], name[MAX_PATH];
	long domain;

	FLICreateList(enumDomain);

	if(FLIListFirst(&domain, file, MAX_PATH, name, MAX_PATH) == 0)
	{
		do
		{
			listName[numDevs] = (char*)malloc(strlen(file) + 1);
			strcpy(listName[numDevs], file);

			listDomain[numDevs] = domain;
			numDevs++;
		}
		while((FLIListNext(&domain, file, MAX_PATH, name, MAX_PATH) == 0) && (numDevs < MAX_DEVICES));
	}

	FLIDeleteList();
}
