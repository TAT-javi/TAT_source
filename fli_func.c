#include "fli_func.h"
#include "common_func.h"

int mvsend_cmd2fli(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=FLI_CMD_FILENAME;
	
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

/*
	send command to fli Daemon
*/

int send_cmd2fli(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=FLI_CMD_FILENAME;

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
	printf("send_cmd2fli=>\n%s",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
} /*send_cmd2fli*/

void home_focuser()
{
	char cmd[FLI_CMD_SIZE];
	sprintf(cmd,"%s\n", "FOCUS 0");
	send_cmd2fli(cmd);
}

void get_focuser_current_pos()
{
	char cmd[FLI_CMD_SIZE];
	sprintf(cmd,"%s\n", "FOCUS -1");
	send_cmd2fli(cmd);
}

void move_focuser_to_pos(long position)
{
	char cmd[FLI_CMD_SIZE];
	if(position >= 0 && position < 105000)
	{
		sprintf(cmd,"FOCUS %d\n",position);
		send_cmd2fli(cmd);
	}
}


void move_to_filter(int position)
{
	char cmd[FLI_CMD_SIZE];
	if(position >= 0 && position < FILTER_TOTAL_NUMBER)
	{
		sprintf(cmd,"WHEEL %d\n",position);
		send_cmd2fli(cmd);
	}
}


void refresh_FLI_values()
{
	get_focuser_current_pos();
	sleep(2);
	move_to_filter(0);
}
