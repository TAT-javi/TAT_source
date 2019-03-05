// needed file: posixtm.h, posixtm.c;
#include <stdio.h>
#include <time.h>
#include "posixtm.c"

//format: YYYYMMDDHHmm
#define SPRING "200303210231"
#define SUMMER "200306220231"
#define AUTUMN "200309230231"
#define WINTER "200312220231"

int main()
{
	time_t time_current;
	char usChoice;
	int usDegree=0, time_differ;
	
	char* refdate[4] = {"Spring Equinox","Summer Solstice","Autumn Equinox","Winter Solstice"};

//	scanf("%c",&usChoice);
for( usChoice=1; usChoice < 5; usChoice++)
{	
	switch(usChoice)
	{
		
		case 1:
			printf("%s\n",refdate[usChoice-1]);
			usDegree = 0;
			time_differ = time(NULL) - posixtime(SPRING, 13);
			usDegree -= ( time_differ%(60*60*24)/60/4 + time_differ/(60*60*24) );
		break;
		case 2:
			printf("%s\n",refdate[usChoice-1]);
			usDegree = 270;
			time_differ = time(NULL) - posixtime(SUMMER, 13);
			usDegree -= ( time_differ%(60*60*24)/60/4 + time_differ/(60*60*24) );
		break;
		case 3:
			printf("%s\n",refdate[usChoice-1]);
			usDegree = 180;
			time_differ = time(NULL) - posixtime(AUTUMN, 13);
			usDegree -= ( time_differ%(60*60*24)/60/4 + time_differ/(60*60*24) );
		break;
		case 4:
			printf("%s\n",refdate[usChoice-1]);
			usDegree = 90;
			time_differ = time(NULL) - posixtime(WINTER, 13);
			usDegree -= ( time_differ%(60*60*24)/60/4 + time_differ/(60*60*24) );
		break;
	}
	if( usDegree > 0 )
	{
		while(usDegree > 360)
		{
			usDegree -= 360;
		}
	}
	else
	{
		while(usDegree < 0)
		{
			usDegree += 360;
		}
	}
	 
	printf("Degree = %d\n\n", usDegree ); 
}//for
}//main

			 

