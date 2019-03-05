/* This daemon monitors the socket in HQ waiting for
 * notifications from the TAT
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "symblc_const.h"
#include "common_func.h"
#include "send_remote_func.h"
#include "weather_func.h"

#define WAIT 1
#define BUFF_SIZE 2000


st_tat_info	*p_tat_info;

void parse_command(char *,char *);

int main(int argc, char *argv[])
{
	int listenfd = 0, connfd = 0,n;
	struct sockaddr_in serv_addr;
	int port=PORT_HQ;
	FILE *fp;

	char recvBuff[BUFF_SIZE],sendBuff[BUFF_SIZE],temp_string[300];
	time_t now;
	pid_t pid;

	/* shm */
	int		shmid;
	/* get east_info shared memory */
	create_tat_info_shm( &shmid, &p_tat_info);

	
	while(1)
	{
		pid=fork();
		if(pid==-1)
		{
			steplog("ERROR: fork()",LST_DAEMON_LOG_TYPE);
			return -1;
		}
		else if(pid!= 0) //parent
		{
			//wait for child
			waitpid(pid, (int *)0,0);

		}
		else break; //child

	}
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(listenfd, 10);

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(listenfd, (struct sockaddr *)&sin, &len) == -1)
	{
		steplog("ERROR: getsockname",LST_DAEMON_LOG_TYPE);
		return -1;
	}
	else
	{
		if(PORT_HQ != ntohs(sin.sin_port)) 
		{
			sleep(2);
			return -1;
		}

		sprintf(temp_string,"Port number %d", ntohs(sin.sin_port));
		steplog(temp_string,LST_DAEMON_LOG_TYPE);
	}

    while(1)
    {
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		if((n=read(connfd,recvBuff,sizeof(recvBuff)))>0)
		{
			recvBuff[n]='\0';
			sprintf(temp_string,"Received: %s",recvBuff);
			steplog(temp_string,LST_DAEMON_LOG_TYPE);
			parse_command(recvBuff,sendBuff);
		}
		else
		{
			steplog("WARNING: Something wrong reading socket",LST_DAEMON_LOG_TYPE);
			strcpy(sendBuff,MSG_WRONG);
		}

		//Send Reply
		write(connfd, sendBuff, strlen(sendBuff));
		close(connfd);
		sprintf(temp_string,"Reply: %s",sendBuff);
		steplog(temp_string,LST_DAEMON_LOG_TYPE);
		sleep(WAIT);
     }
}

void parse_command(char *received, char *reply)
{
	int i;
	time_t now;
	char site[3],note[BUFF_SIZE],cmd[BUFF_SIZE];
	char openFlag[5],yyyy[10],mm[10],dd[10];
	char beginString[50],endString[50];
	char ra_h[10],ra_m[10],ra_s[10],dec_d[10],dec_m[10],dec_s[10];
	char temperature[10],filterString[50],cycle_number[10],flatFlag[5];
	
	sscanf(received,"%s %s",site,note);

	if(strcmp(site,SITE))
	{
		steplog("ERROR: Wrong site",LST_DAEMON_LOG_TYPE);
		sprintf(reply,"%s %s",MSG_WRONG,ERROR_SITE_RPLY);
	}
	else if(!strcmp(note,CHECK_WEATHER_CMD))
	{
		//CHECK THE WEATHER AND WRITE THE REPLY
		i=check_previous_weather_conditions_print(1);
		if(i==NORMAL_STATUS)
			sprintf(reply,"%s %s",MSG_OK,WEATHER_OK_RPLY);
		else if(i==ERROR_STATUS)
			sprintf(reply,"%s %s",MSG_OK,WEATHER_ERROR_RPLY);
		else
			sprintf(reply,"%s %s",MSG_OK,WEATHER_BAD_RPLY);
	}
	else if(!strcmp(note,STOP_OBS_CMD))
	{
		//STOP THE OBSERVATION
		sprintf(reply,"%s NOT YET",MSG_OK);
	}
	else if(!strcmp(note,CHECK_OBS_CMD))
	{
		//CHECK IF THE AUTO_OBSERVATION
		if(p_tat_info->obs_info.auto_observing == 1)
			sprintf(reply,"%s %s",MSG_OK,IS_OBS_RPLY);
		else
			sprintf(reply,"%s %s",MSG_OK,IDLE_RPLY);
	}
	else if(!strcmp(note,ADD_OBS_LINE_CMD))
	{
		//ADD TODO
		i=sscanf(note,"%*s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			openFlag,yyyy,mm,dd,beginString,endString,
			ra_h,ra_m,ra_s,dec_d,dec_m,dec_s,temperature,
			filterString,cycle_number,flatFlag);
		
		
		if(i<16)
		{
			steplog("ERROR: Not enough input parameters",LST_DAEMON_LOG_TYPE);
			sprintf(reply,"%s %s",MSG_WRONG,LINE_ADD_ERROR_RPLY);
		}
		else
		{
// 			sprintf(cmd,"echo \"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\" >> %s",
// 				openFlag,yyyy,mm,dd,beginString,endString,
// 				ra_h,ra_m,ra_s,dec_d,dec_m,dec_s,temperature,
// 				filterString,cycle_number,flatFlag,TIME_TABLE_FILENAME);
			
			steplog(cmd,LST_DAEMON_LOG_TYPE);
			system(cmd);
			sprintf(reply,"%s %s",MSG_OK,LINE_ADD_OK_RPLY);
		}
	}
	else
	{
		steplog("ERROR: Can't understand the message",LST_DAEMON_LOG_TYPE);
		sprintf(reply,"%s %s",MSG_WRONG,ERROR_READ_RPLY);
	}
}

