#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fitsio.h"
#include <string.h>
#include "symblc.const.h" /* here is the information about pixels*/
#include "adjust_fov.h"

int main(int argc, char* argv[])
{
	char filename[100],log_filename[100];
	int fov_flag;
	float exposure_time;

	sprintf(log_filename,"/home/observer/log/test.log");
	fov_flag = FOV_TBC;
	filter_type[0]='V';
	filter_type[1]='\0';

	if(argc!=2)
	{
		printf("############Do not forget########################\n");
		printf("./correl [fits name]\n");
		printf("##############################################\n");
	}
	else
	{
		sprintf(filename,"%s",argv[1]);
		
 		fov_flag = correlation(filename, fov_flag,&exp_time,log_filename);
		printf("FOV_STATUS = %d\nexp_time=%f\n",fov_flag,exposure_time);
	}
	return 0;

}
