#include <stdio.h>
#include <stdlib.h>
#include "adjust_fov.h"
#include "dsp_func.h"

st_tat_info    *p_tat_info;


int main(int argc, char *argv[])
{
	int i;
	float ra=-1,dec=0;

	if(argc == 4)
	{
		ra = atof(argv[2]);
		dec = atof(argv[3]);
		
		if(ra < 0 || ra > 24)
		{
			printf("ERROR: Input RA out of bounds (%f)!!!",ra);
			return 1;
		}
		if(dec < -30 || dec > 55)
		{
			printf("ERROR: Input DEC out of bounds (%f)!!!",dec);
			return 1;
		}
	}
	else if(argc!=2)
	{
		printf("### Usage ###\n");
		printf("%s [fit name] (Optional: RA(hours) DEC)\n",argv[0]);
		printf("#############\n");
		return 1;
	}
	
	i=get_RA_DEC_astrometry(argv[1],&ra,&dec);
	if(!i)
	{
		printf("Coordinates of %s are:\n",argv[1]);
		printf("RA = %f\tDEC = %f\n",ra,dec);
	}
	else
		printf("WARNING: Could not get RA and DEC\n");
	
	printf("### END OF PROGRAM ###\n");
	return 0;
}

