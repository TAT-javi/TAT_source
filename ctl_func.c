#include <stdio.h>
#include "symblc_const.h"
#include "main_cmd_menu.h"
/* 
	send command to CTL Daemon for cmd menu
*/
int mvsend_cmd2ctl(char daemonCmd[256])
{
	int i;
	FILE *fp;
	char fpname[]=CTL_CMD_FILENAME;
	
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
}

int send_cmd2ctl(char daemonCmd[256])
{
	int i;
	FILE *fp;
	char fpname[]=CTL_CMD_FILENAME;
	
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
	printf("%s",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
}
