#include <unistd.h>
#include "ppc_func.h"
#include "common_func.h"
#include "main_cmd_menu.h"

/*
	send command to Power Control Daemon for cmd menu
*/
int mvsend_cmd2ppc(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=PPC_CMD_FILENAME;

	for(i=0;i<10;i++)
	{
    	if( (fp=fopen(fpname,"w+")) == NULL )
        {
        	if( i>8 ) return 0;
            printf("Open %s failed\n.Try again after %d sec\n", fpname, REOPEN_FILE_SECOND);
            sleep(REOPEN_FILE_SECOND);
		}
        else break;
	}
	/* mvprintw( MSG_LINE, 0, "%s",daemonCmd); */
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
} /*send_cmd2ppc*/

/*
	send command to Power Control Daemon
*/

int send_cmd2ppc(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=PPC_CMD_FILENAME;

	for(i=0;i<10;i++)
	{
    	if( (fp=fopen(fpname,"w+")) == NULL )
		{
			if( i>8 ) return 0;
			printf("Open %s failed\n.Try again after %d sec\n", fpname, REOPEN_FILE_SECOND);
			sleep(REOPEN_FILE_SECOND);
		}
		else break;
	}
	printf("send_cmd2ppc=>\n%s",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
} /*send_cmd2ppc*/

void TurnDspPowerOff()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"%s\n", "DSPPOWER OFF");
	send_cmd2ppc(cmd);
}

void ResetDsp()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"%s\n", "DSPPOWER OFF");
	send_cmd2ppc(cmd);
	sleep(1);
	sprintf(cmd,"%s\n", "DSPPOWER ON");
	send_cmd2ppc(cmd);
	sleep(2);
}


/*
	030. turn power on.
*/
void TurnAllPowerOn()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"MAINPOWER ON\nDSPJUMPER ON\nVDCPOWER ON\n\
			DSPPOWER ON\nCCDPOWER ON\nWHEELPOWER OFF\n");
	send_cmd2ppc(cmd);
	sleep(2);
	sprintf(cmd,"TRANPOWER OFF\nROTPOWER OFF\nRAPOWER ON\nDECPOWER ON\n");
	send_cmd2ppc(cmd);
}

/*
	reset dsp
*/
void ResetDsp_Timing()
{
	char cmd[PPC_CMD_SIZE];
	/*reset dsp */
	sprintf(cmd,"DSPPOWER OFF\nDSPJUMPER ON\n");
	send_cmd2ppc(cmd);
	timing(time(NULL)+10);
	sprintf(cmd,"%s\n",
		"DSPPOWER ON");
	send_cmd2ppc(cmd);
	sleep(3);
}

/*
	150. turn power off
*/
void TurnPartPowerOff()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"RAPOWER OFF\n\
			DECPOWER OFF\n\
			ROTPOWER OFF\n\
			TRANPOWER OFF\n\
			WHEELPOWER OFF\n\
			DSPPOWER OFF\n\
			VDCPOWER OFF\n");
	send_cmd2ppc( cmd );
}

void TurnMotorPowerOff()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"RAPOWER OFF\n\
			DECPOWER OFF\n\
			ROTPOWER OFF\n\
			TRANPOWER OFF\n\
			WHEELPOWER OFF\n");
	send_cmd2ppc( cmd );
}


void TurnCcdMainPowerOff()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"CCDPOWER OFF\nMAINPOWER OFF\n");
	send_cmd2ppc(cmd);
}

/*
	turn power off
*/
void TurnAllPowerOff()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"%s","MAINPOWER OFF\n\
			RAPOWER OFF\n\
			DECPOWER OFF\n\
			ROTPOWER OFF\n\
			TRANPOWER OFF\n\
			WEATHERPOWER OFF\n\
			DSPPOWER OFF\n\
			CCDPOWER OFF\n\
			VDCPOWER OFF\n");

	send_cmd2ppc(cmd);
}

void TurnWheelOff()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"WHEELPOWER OFF\n");
	send_cmd2ppc( cmd );
}

void TurnWheelOn()
{
	char cmd[PPC_CMD_SIZE];
	sprintf(cmd,"WHEELPOWER ON\n");
	send_cmd2ppc( cmd );
}
