#include <stdio.h>
#include "symblc_const.h"

st_tat_info *p_tat_info;
#define ANSI_COLOR_GREEN     "\x1b[32m"
#define ANSI_COLOR_RED       "\x1b[31m"
#define ANSI_COLOR_RESET     "\x1b[0m"

int main(int argc, char* argv[])
{
	int Nerrors;
	
	if(argc == 2 && !strcmp(argv[1],"--help"))
	{
		printf("#####################################################################################\n");
		printf("Program to check the input information in the file\n%s\n",TIME_TABLE_FILENAME);
		printf("This program will ensure the observation goes smoothly\n");
		printf("######################################################################################\n");
		printf("Usage: %s\n\n",argv[0]);
		return 1;
	}
	if(argc>2)
	{
		printf("##############################Do not forget######################################\n");
		printf("Usage: %s\n",argv[0]);
		printf("#################################################################################\n");
		printf("For more help type: %s --help\n\n",argv[0]);
		return 1;
	}
	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
	
	//run the program
	printf("Checking for errors in:\n%s\n",TIME_TABLE_FILENAME);
	Nerrors = check_input_file();
    if(Nerrors)
        printf("\n %s Total %d errors ",ANSI_COLOR_RED,Nerrors);
    else
        printf("\n %s No errors ",ANSI_COLOR_GREEN);
    
    printf("%s \n",ANSI_COLOR_RESET);
        
	return 0;
}
