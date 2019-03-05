/*
	format: dspdaemon stop	==>0
			ccddaemon start	==> 1
			ppcdaemon restart	==> 2
			dspdaemon status
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "symblc_const.h"
#include "tat_info.h"
#include "ccd_func.h"

#define	DAEMON_TYPE_LENGTH	9
#define ACTION_LENGTH	7
#define DAEMON_NAME_LENGTH	20
#define CMD_STRING_LENGTH	90
#define COMMAND_MAX_NUMBER	5
#define COMMAND_SECTION		2

#define ALLDAEMON	11
#define DSPDAEMON	12
#define CCDDAEMON	13
#define PPCDAEMON	14
#define PWRDAEMON	15
#define FLIDAEMON	16
#define LSTDAEMON	17
#define CHKDAEMON	18

#define ACTION_RESTART		31
#define ACTION_START		32
#define ACTION_STOP		33

#define PPC_DAEMON_NAME "ppcdaemon-curr"
#define DSP_DAEMON_NAME "dspdaemon-curr"
#define CCD_DAEMON_NAME "ccddaemon-curr"
#define PWR_DAEMON_NAME "pwrdaemon-curr"
#define FLI_DAEMON_NAME "flidaemon-curr"
#define LST_DAEMON_NAME "lstdaemon-curr"
#define AUTO_OBSERVING_PROGRAM "auto_observing"

#define CYCLE_TIME	500000	/* micro-second */
#define DEBUG	
#undef  DEBUG

st_tat_info *p_tat_info;

//////////////////////////////
//	stop daemon
void stopDaemon(char daemon_name[DAEMON_NAME_LENGTH+1])
{
#ifdef F6 //For Apogee F6 we make sure the server is offr
	if(!strcmp(daemon_name,CCD_DAEMON_NAME))
	{
		ccd_cooler_off();
		sleep(1);
	}
#endif
	char temp_string[CMD_STRING_LENGTH+1];
	sprintf(temp_string,"pkill %s",daemon_name);
	printf("stop %s\n",daemon_name);
	system(temp_string);
}

//////////////////////////////
//	start daemon
void startDaemon(char daemon_name[DAEMON_NAME_LENGTH+1])
{
	char temp_string[CMD_STRING_LENGTH+1];
	sprintf(temp_string,"%s%s &",DAEMON_PATH,daemon_name);
	printf("start %s\n",daemon_name);
	system(temp_string);
}

//////////////////////////////
//	restart deamon
void restartDaemon()
{
	stopDaemon(PPC_DAEMON_NAME);
	stopDaemon(DSP_DAEMON_NAME);
	stopDaemon(CCD_DAEMON_NAME);
	stopDaemon(PWR_DAEMON_NAME);
	stopDaemon(FLI_DAEMON_NAME);
	stopDaemon(LST_DAEMON_NAME);
	sleep( 1);
	startDaemon(PPC_DAEMON_NAME);
	startDaemon(DSP_DAEMON_NAME);
	startDaemon(CCD_DAEMON_NAME);
	startDaemon(PWR_DAEMON_NAME);
	startDaemon(FLI_DAEMON_NAME);
	startDaemon(LST_DAEMON_NAME);
}

int parsingCmd( int cmd[COMMAND_MAX_NUMBER][COMMAND_SECTION], FILE* cmdFile)
{
	char daemonType[DAEMON_TYPE_LENGTH+1];
	char action[ACTION_LENGTH+1];
	int number= 0;
	
	while( !feof( cmdFile))
	{
		/* INITIALIZE AND VALIDATE */
		strcpy( daemonType, "");
		strcpy( action, "");
		fscanf( cmdFile, "%s %s", daemonType, action);
		if( (strlen( daemonType) || strlen(action)) == 0) continue;
		
		/* ALL DAEMON */	
		if( strcmp( daemonType, "alldaemon") == 0)
		{
			cmd[number][0]= ALLDAEMON;
			if( strcmp( action, "restart") == 0)
			{
				cmd[number][1]= ACTION_RESTART;
				number++;
			}
			else if( strcmp( action, "start") == 0)
			{
				cmd[number][1]= ACTION_START;
				number++;
			}
			else if( strcmp( action, "stop") == 0)
			{
				cmd[number][1]= ACTION_STOP;
				number++;
			}
			else
			{
				printf("Invalid action: %s %s\n", daemonType, action);
			}
		}
		/* DSP DAEMON */
		else if( strcmp( daemonType, "dspdaemon") == 0)
		{
			cmd[number][0]= DSPDAEMON;
			if( strcmp( action, "restart") == 0)
			{
				cmd[number][1]= ACTION_RESTART;
				number++;
			}
			else if( strcmp( action, "start") == 0)
			{
				cmd[number][1]= ACTION_START;
				number++;
			}
			else if( strcmp( action, "stop") == 0)
			{
				cmd[number][1]= ACTION_STOP;
				number++;
			}
			else
			{
				printf("Invalid action: %s %s\n", daemonType, action);
			}
		}
		/* CCD DAEON */
		else if( strcmp( daemonType, "ccddaemon") == 0)
		{
			cmd[number][0]= CCDDAEMON;
			if( strcmp( action, "restart") == 0)
			{
				cmd[number][1]= ACTION_RESTART;
				number++;
			}
			else if( strcmp( action, "start") == 0)
			{
				cmd[number][1]= ACTION_START;
				number++;
			}
			else if( strcmp( action, "stop") == 0)
			{
				cmd[number][1]= ACTION_STOP;
				number++;
			}
			else
			{
				printf("Invalid action: %s %s\n", daemonType, action);
			}
		}
		/* PPC DAEMON */
		else if( strcmp( daemonType, "ppcdaemon") == 0)
		{
			cmd[number][0]= PPCDAEMON;
			if( strcmp( action, "restart") == 0)
			{
				cmd[number][1]= ACTION_RESTART;
				number++;
			}
			else if( strcmp( action, "start") == 0)
			{
				cmd[number][1]= ACTION_START;
				number++;
			}
			else if( strcmp( action, "stop") == 0)
			{
				cmd[number][1]= ACTION_STOP;
				number++;
			}
			else
			{
				printf("Invalid action: %s %s\n", daemonType, action);
			}
		}
		/* PWR DAEMON */
		else if( strcmp( daemonType, "pwrdaemon") == 0)
		{
			cmd[number][0]= PWRDAEMON;
			if( strcmp( action, "restart") == 0)
			{
				cmd[number][1]= ACTION_RESTART;
				number++;
			}
			else if( strcmp( action, "start") == 0)
			{
				cmd[number][1]= ACTION_START;
				number++;
			}
			else if( strcmp( action, "stop") == 0)
			{
				cmd[number][1]= ACTION_STOP;
				number++;
			}
			else
			{
				printf("Invalid action: %s %s\n", daemonType, action);
			}
		}
		/* FLI DAEMON */
		else if( strcmp( daemonType, "flidaemon") == 0)
		{
			cmd[number][0]= FLIDAEMON;
			if( strcmp( action, "restart") == 0)
			{
				cmd[number][1]= ACTION_RESTART;
				number++;
			}
			else if( strcmp( action, "start") == 0)
			{
				cmd[number][1]= ACTION_START;
				number++;
			}
			else if( strcmp( action, "stop") == 0)
			{
				cmd[number][1]= ACTION_STOP;
				number++;
			}
			else
			{
				printf("Invalid action: %s %s\n", daemonType, action);
			}
		}
		/* FLI DAEMON */
		else if( strcmp( daemonType, "lstdaemon") == 0)
		{
			cmd[number][0]= FLIDAEMON;
			if( strcmp( action, "restart") == 0)
			{
				cmd[number][1]= ACTION_RESTART;
				number++;
			}
			else if( strcmp( action, "start") == 0)
			{
				cmd[number][1]= ACTION_START;
				number++;
			}
			else if( strcmp( action, "stop") == 0)
			{
				cmd[number][1]= ACTION_STOP;
				number++;
			}
			else
			{
				printf("Invalid action: %s %s\n", daemonType, action);
			}
		}
		else if( strcmp( daemonType, "chkdaemon") == 0)
		{
			cmd[number][0]= CHKDAEMON;
			cmd[number][1]= CHKDAEMON;
		}
		/* INVALID COMMAND */
		else
		{
			printf("Invalid command: %s %s\n", daemonType, action);
		}
		
		
	}
//	printf("number= %d\n", number);
	return number;

}

//////////////////////////////////////////////////
//	main
int main(void)
{
	FILE *cmdFile;
	int i, number;
	int cmd[COMMAND_MAX_NUMBER][COMMAND_SECTION];
	time_t currTime;
	char *pcCurrTime;
	pid_t pid;
	char  msg[7];

	/* get process name */
	FILE *fp;
	char *res;
	char proc_cmdline[BUFSIZ];
	char residual_num;

	DIR *p_dir;
	struct dirent *p_ent;	
	
	////////////////////////////////////////
	//	clone process
	while(1)
	{
		pid=fork();
		if( pid== -1)
		{
			//error
			perror("-1");
		}
		else if( pid!= 0)
		{
				printf("wait child pid =%d\n",pid);
				waitpid(pid,(int *)0,0);
				printf("child pid= %d crash\n", pid);
				time(&currTime);
				pcCurrTime= ctime( &currTime);
				cmdFile= fopen(DAEMON_LOG_FILENAME, "a+");
				fprintf( cmdFile, "%s \tctrl daemon child pid=%d crash.\n", pcCurrTime, pid);
				fclose( cmdFile);
		}
		else
		{
			//child
			break;
		}
	}       //end while

	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);

	remove( CTL_CMD_FILENAME);
	while( 1)
	{
		
		if(( cmdFile= fopen( CTL_CMD_FILENAME, "r")) != NULL )
		{
			if( ( number= parsingCmd( cmd,cmdFile) )> 0)
			{
				for( i=0; i< number; i++)
				{
					switch( cmd[i][0])
					{
						case ALLDAEMON:
							switch( cmd[i][1])
							{
								case ACTION_RESTART:
									restartDaemon();
								break;
								case ACTION_START:
									if( (p_tat_info->ctl_info.dspd ||
									    p_tat_info->ctl_info.ccdd ||
									    p_tat_info->ctl_info.ppcd ||
									    p_tat_info->ctl_info.flid ||
									    p_tat_info->ctl_info.lstd ||
									    p_tat_info->ctl_info.pwrd )!=1)
									{
										startDaemon(PPC_DAEMON_NAME);
										startDaemon(DSP_DAEMON_NAME);
										startDaemon(CCD_DAEMON_NAME);
										startDaemon(PWR_DAEMON_NAME);
										startDaemon(FLI_DAEMON_NAME);
										startDaemon(LST_DAEMON_NAME);
									}
								break;
								case ACTION_STOP:
									stopDaemon(PPC_DAEMON_NAME);
									stopDaemon(DSP_DAEMON_NAME);
									stopDaemon(CCD_DAEMON_NAME);
									stopDaemon(PWR_DAEMON_NAME);
									stopDaemon(FLI_DAEMON_NAME);
									stopDaemon(LST_DAEMON_NAME);
								break;
								
							}
						break;
						case DSPDAEMON:
							switch( cmd[i][1])
							{
								case ACTION_RESTART:
									stopDaemon(DSP_DAEMON_NAME);	
									sleep( 1);
									startDaemon(DSP_DAEMON_NAME);	
								break;
								case ACTION_START:
									if( p_tat_info->ctl_info.dspd ==0 )
										startDaemon(DSP_DAEMON_NAME);	
								break;
								case ACTION_STOP:
									stopDaemon(DSP_DAEMON_NAME);	
								break;
								
							}
						
						break;
						case CCDDAEMON:
							switch( cmd[i][1])
							{
								case ACTION_RESTART:
									stopDaemon(CCD_DAEMON_NAME);
									sleep( 1);
									startDaemon(CCD_DAEMON_NAME);
								break;
								case ACTION_START:
									if( p_tat_info->ctl_info.ccdd == 0)
										startDaemon(CCD_DAEMON_NAME);
								break;
								case ACTION_STOP:
									stopDaemon(CCD_DAEMON_NAME);
								break;
								
							}
						
						break;
						case PPCDAEMON:
							switch( cmd[i][1])
							{
								case ACTION_RESTART:
									stopDaemon(PPC_DAEMON_NAME);
									sleep( 1);
									startDaemon(PPC_DAEMON_NAME);
								break;
								case ACTION_START:
									if( p_tat_info->ctl_info.ppcd == 0 )
										startDaemon(PPC_DAEMON_NAME);
								break;
								case ACTION_STOP:
									stopDaemon(PPC_DAEMON_NAME);
								break;
								
							}
						
						break;
						case PWRDAEMON:
							switch( cmd[i][1])
							{
								case ACTION_RESTART:
									stopDaemon(PWR_DAEMON_NAME);
									sleep( 1);
									startDaemon(PWR_DAEMON_NAME);
								break;
								case ACTION_START:
									if( p_tat_info->ctl_info.pwrd == 0 )
										startDaemon(PWR_DAEMON_NAME);
								break;
								case ACTION_STOP:
									stopDaemon(PWR_DAEMON_NAME);
								break;
								
							}
						
						break;
						case FLIDAEMON:
							switch( cmd[i][1])
							{
								case ACTION_RESTART:
									stopDaemon(FLI_DAEMON_NAME);
									sleep( 1);
									startDaemon(FLI_DAEMON_NAME);
								break;
								case ACTION_START:
									if( p_tat_info->ctl_info.flid == 0 )
										startDaemon(FLI_DAEMON_NAME);
								break;
								case ACTION_STOP:
									stopDaemon(FLI_DAEMON_NAME);
								break;
							}
						
						break;
						case LSTDAEMON:
							switch( cmd[i][1])
							{
								case ACTION_RESTART:
									stopDaemon(LST_DAEMON_NAME);
									sleep( 1);
									startDaemon(LST_DAEMON_NAME);
								break;
								case ACTION_START:
									if( p_tat_info->ctl_info.lstd == 0 )
										startDaemon(LST_DAEMON_NAME);
								break;
								case ACTION_STOP:
									stopDaemon(LST_DAEMON_NAME);
								break;
							}
						
						break;
					}
				}
				remove(CTL_CMD_FILENAME);
			}	//if( parsing)
			else
			{
				printf("No any command in %s\n", CTL_CMD_FILENAME);
				remove(  CTL_CMD_FILENAME);
			}
		}	//if( fopen)
		
		for( i=0; i<7; i++)
			msg[i]=0;
			
		p_dir=opendir("/proc");
		if( p_dir != NULL)
		{
		#ifdef DEBUG
		printf("open dir\n");
		#endif
		while( p_ent=readdir(p_dir) )
		{
			if( p_ent->d_type != DT_DIR )
				continue;
			#ifdef DEBUG
				printf("\tread dir\n");
			#endif
			sprintf( proc_cmdline, "/proc/%s/cmdline", p_ent->d_name);
			fp = fopen( proc_cmdline, "r");
			if( fp != NULL )
			{
				fscanf( fp, "%s", proc_cmdline);
				if( (res = strstr( proc_cmdline, AUTO_OBSERVING_PROGRAM)) != NULL)
				{
						msg[6]=1;
				}
				res = strstr( proc_cmdline, "daemon");
				if( res != NULL )
				{
					if( (res = strstr( proc_cmdline, DSP_DAEMON_NAME)) != NULL)
					{
						msg[0]=1;
						residual_num--;
					}
					else if( (res = strstr( proc_cmdline, CCD_DAEMON_NAME)) != NULL)
					{
						msg[1]=1;
						residual_num--;
					}
					else if( (res = strstr( proc_cmdline, PPC_DAEMON_NAME)) != NULL)
					{
						msg[2]=1;
						residual_num--;
					}
					else if( (res = strstr( proc_cmdline, PWR_DAEMON_NAME)) != NULL)
					{
						msg[3]=1;
						residual_num--;
					}
					else if( (res = strstr( proc_cmdline, FLI_DAEMON_NAME)) != NULL)
					{
						msg[4]=1;
						residual_num--;
					}
					else if( (res = strstr( proc_cmdline, LST_DAEMON_NAME)) != NULL)
					{
						msg[5]=1;
						residual_num--;
					}
				}
				fclose( fp );
			}
		}
		closedir( p_dir);
		#ifdef DEBUG
			printf("close dir\n");
		#endif
		update_ctl_info ( &(p_tat_info->ctl_info), msg);
		}
		else
			perror ("Couldn't open the directory");
		#ifdef DEBUG
			printf( "dspd=%d; ccdd=%d; ppcd=%d; pwrd=%d; flid=%d;lstd=%d\n",
				p_tat_info->ctl_info.dspd,
				p_tat_info->ctl_info.ccdd,
				p_tat_info->ctl_info.ppcd,
				p_tat_info->ctl_info.pwrd,
				p_tat_info->ctl_info.flid,
				p_tat_info->ctl_info.lstd
  				);
		#endif
		usleep( CYCLE_TIME);
	}	// while
}
