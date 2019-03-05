#include <unistd.h>
#include <string.h>
#include "ccd_func.h"
#include "symblc_const.h"
#include "common_func.h"
#include "main_cmd_menu.h"

extern st_tat_info *p_tat_info;
/*
	send command to CCD daemon for cmd menu
*/
int mvsend_cmd2ccd(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=CCD_CMD_FILENAME;
	
	for(i=0;i<10;i++)
	{
		if( ( fp=fopen(fpname, "w+") ) == NULL)
		{
			if( i > 8) return 0;
			printf("Open %s failed\n.Try again after %d sec\n", fpname, REOPEN_FILE_SECOND);
			sleep(REOPEN_FILE_SECOND);
		}
		else break;
	}
	mvprintw(MSG_LINE, 0, "%s",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
} /*send_cmd2ccd*/
/*
	sned command to CCD daemon
*/
int send_cmd2ccd(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=CCD_CMD_FILENAME;
	
	for(i=0;i<10;i++)
	{
		if( ( fp=fopen(fpname, "w+") ) == NULL)
		{
			if( i > 8) return 0;
			printf("Open %s failed\n.Try again after %d sec\n", fpname, REOPEN_FILE_SECOND);
			sleep(REOPEN_FILE_SECOND);
		}
		else break;
	}
	printf("send_cmd2ccd=> %s\n",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
} /*send_cmd2ccd*/

void ccd_cooler_on(float temperature)
{
	char daemonCmd[CCD_CMD_SIZE];  
	sprintf(daemonCmd,"camera on none %d 0 0 0",(int)temperature);
	send_cmd2ccd( daemonCmd);
}

void ccd_cooler_off()
{
	char daemonCmd[CCD_CMD_SIZE];  
	sprintf(daemonCmd,"camera off skip 0 1 1\n");
	send_cmd2ccd( daemonCmd);
}

void ccd_cooler_shutdown()
{
	char daemonCmd[CCD_CMD_SIZE];  
	sprintf(daemonCmd,"camera shutdown skip 0 1 1\n");
	send_cmd2ccd( daemonCmd);
}

void ccd_takeimage(char* filename, float second, char shutter, char bin)
{
	char daemonCmd[CCD_CMD_SIZE];  
	sprintf(daemonCmd,"camera takeimage %s %f %d %d\n",
		filename,second,shutter,bin);
	send_cmd2ccd( daemonCmd);
}

void ccd_abort_takeimage(void)
{
	char daemonCmd[CCD_CMD_SIZE];  
	sprintf(daemonCmd,"camera abort skip 0 1 1\n");
	send_cmd2ccd( daemonCmd);
}
