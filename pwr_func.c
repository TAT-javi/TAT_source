#include <unistd.h>
#include "pwr_func.h"
#include "common_func.h"
#include "main_cmd_menu.h"

/*
	send command to Power Control Daemon for cmd menu
*/

int mvsend_cmd2pwr(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=PWR_CMD_FILENAME;
	
    	if( (fp=fopen(fpname,"w+")) == NULL )
        {
        	printf("Open %s failed\n.\n", fpname);
		return 0;
            
	}
	/* mvprintw(MSG_LINE, 0, "%s",daemonCmd); */
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
} /*send_cmd2pwr*/

/*
	send command to Power Control Daemon
*/

int send_cmd2pwr(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=PWR_CMD_FILENAME;
	
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
	printf("send_cmd2pwr=>\n%s",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
} /*send_cmd2pwr*/

/*
	030. turn power on.
*/
void pwr_rx_poweron()
{
	sprintf(daemonCmd,"%s", "R1POWER ON\n"
				"R2POWER ON\n"
				"R3POWER ON\n"
				"R4POWER ON\n"
				"R5POWER ON\n"
				"R6POWER ON\n");
	
	send_cmd2pwr(daemonCmd);
}

void pwr_rx_poweroff()
{
	sprintf(daemonCmd,"%s", "R1POWER OFF\n"
				"R2POWER OFF\n"
				"R3POWER OFF\n"
				"R4POWER OFF\n"
				"R5POWER OFF\n"
				"R6POWER OFF\n");
	
	send_cmd2pwr(daemonCmd);
}

void pwr_lx_poweron()
{
        sprintf(daemonCmd,"%s", "L1POWER ON\n"
                                "L2POWER ON\n"
                                "L3POWER ON\n"
                                "L4POWER ON\n"
                                "L5POWER ON\n"
                                "L6POWER ON\n");

        send_cmd2pwr(daemonCmd);
}

void pwr_lx_poweroff()
{
        sprintf(daemonCmd,"%s", "L1POWER OFF\n"
                                "L2POWER OFF\n"
                                "L3POWER OFF\n"
                                "L4POWER OFF\n"
                                "L5POWER OFF\n"
                                "L6POWER OFF\n");

        send_cmd2pwr(daemonCmd);
}

