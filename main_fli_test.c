#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "libfli.h"
#include <pthread.h>
#include <string.h>
#include "symblc_const.h" /* here is the information about pixels*/
#include "focuser_func.h"
#include "wheel_func.h"

st_tat_info *p_tat_info;

int main()
{
	int out,wheel_go,focuser_go,temp, choice,step=1000;
	pthread_t thread_wheel,thread_focuser;
	MOVEWHEEL wheel;
	MOVEFOCUSER foc;
	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
	
	out=0;
	while(!out)
	{
		wheel_go=0;
		focuser_go=0;
		foc.move_position =0;
		foc.move_rel_pos=0;
		foc.home_focuser=0;
		wheel.filter_position=0;
		
		system("clear");
		printf("############# FOCUSER ###################\n");
		if(p_tat_info->fli_info.focuser_moving) printf(" Focuser moving\n");
		else printf(" Focuser not moving\n");
		
		printf(" Focuser current position %ld\t Focuser Max extent %ld\n",
			p_tat_info->fli_info.focuser_curr_position,
		 p_tat_info->fli_info.focuser_extent);
		
		printf(" 1) Change step %d\n",step);
		printf(" 2) Move focuser out\t3) Move focuser in\n");
		printf(" 4) Home focuser\t5) Move focuser to a specific position\n\n");
		
		printf("############# FILTER WHEEL ###################\n");
		if(p_tat_info->fli_info.wheel_moving) printf(" Filter wheel moving\n");
		else printf(" Filter wheel not moving\n");
		
		printf(" Filter wheel current position %ld\n",
			p_tat_info->fli_info.wheel_curr_position);
		
		printf(" 6) Change filter\n\n\n\n 7) Exit\n\n\n\n Choice?\n => ");
		scanf("%d",&choice);
		
		switch(choice)
		{
			case 1:
				printf("\nNew step?\n => ");
				scanf("%d",&temp);
				if(temp > 10 && temp <50000) step = temp;
				else
				{
					printf("ERROR: New step out of bounds\n");
					getchar();
				}
			break;
			case 2:
				foc.move_rel_pos=step;
				focuser_go=1;
			break;
			case 3:
				foc.move_rel_pos=step*(-1);
				focuser_go=1;
			break;
			case 4:
				foc.home_focuser=1;
				focuser_go=1;
			break;
			case 5:
				printf("New position (0-%d)?\n => ", p_tat_info->fli_info.focuser_extent);
				scanf("%d",&temp);
				if(temp>=0 && temp< p_tat_info->fli_info.focuser_extent)
				{ 
					foc.move_position =temp -p_tat_info->fli_info.focuser_curr_position ;
					focuser_go=1;
				}
				else
				{
					printf("ERROR: Filter Wheel position out of bounds\n");
					getchar();
				}

			break;
			case 6:
				printf("New filter position (0-6)?\n => ");
				scanf("%d",&temp);
				if(temp>=0 && temp<7)
				{ 
					wheel.filter_position=temp;
					wheel_go=1;
				}
				else
				{
					printf("ERROR: Filter Wheel position out of bounds\n");
					getchar();
				}
			break;
			case 7:
				out =1;
			break;
			default:
				printf("Error command!!\n");
				getchar();
			break;
		}
		
		if(wheel_go)
		{
			pthread_create (&thread_wheel, NULL, &move_wheel, &wheel);
			pthread_join(thread_wheel,NULL); //harvest the moving filter thread
		}
		else if(focuser_go)
		{
			pthread_create (&thread_focuser, NULL, &move_focuser, &foc);
			pthread_join(thread_focuser,NULL); //harvest the moving focuser thread
		}
	}

	return 0;
}
