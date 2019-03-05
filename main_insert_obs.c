#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "symblc_const.h"
#include "check_input_file.h"


int main(int argc, char *argv[])
{
	int i,ccd_temp;
	char flat_flag[20],ra_string[20],dec_string[20],filter_string[100];
	char start_string[20],end_string[20],target_name[50];
	
	if(argc!=9)
	{
		printf("Program to insert an observation in %s\n",TIME_TABLE_FILENAME);
		printf("#############  USAGE   ##################\n");
		printf("%s [start observation] [end observation] [RA] [DEC] [CCD temp] [Filter Sequence] [flat_flag] [Target_name]\n",argv[0]);
		printf("######\nstart/end observation format = YYYYMMDDHHMM [ex. 201705272330]\n");
		printf("RA format = HH:MM:SS [ex. 03:04:23]\nDEC format = +-DD:MM:SS [ex. 02:43:11]\n");
		printf("Filter Sequence format = Filter1(expTime1)Filter2(expTime2)... [ex. N(10)V(30)R(40)]\n");
		printf("Flat fag format = a or b or t or n [ex. a]\n####################\n");
		return 1;
	}
	
	strcpy(start_string,argv[1]);
	strcpy(end_string,argv[2]);
	strcpy(ra_string,argv[3]);
	strcpy(dec_string,argv[4]);
	ccd_temp = atoi(argv[5]);
	strcpy(filter_string,argv[6]);
	strcpy(flat_flag,argv[7]);
	strcpy(target_name,argv[8]);
	
	i = insert_obs(start_string,end_string,ra_string,dec_string,ccd_temp,filter_string,flat_flag[0],target_name);

	printf("### END OF PROGRAM ###\n");
	return 0;
}

