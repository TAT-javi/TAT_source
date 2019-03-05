/*
 * Program to send the data and messages to the HQ 
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include "symblc_const.h"
#include "common_func.h"
#include "send_remote_func.h"

void remote_backup(void)
{
	char cmd[1024],ip_hq[50],remote_user[100],img_remote_path[100],cal_remote_path[100];
	int i;
	
	DoGetValueString("REMOTE_USER",remote_user);
	DoGetValueString("IMG_REMOTE_PATH",img_remote_path);
	DoGetValueString("CAL_REMOTE_PATH",cal_remote_path);
	 
	
	steplog("Back up the files to the PC at the Head quarters",AUTO_OBSERVE_LOG_TYPE);
	
	// send image files
	sprintf(cmd,"rsync -a %s/ %s@%s:%s ",IMAGE_PATH,
				remote_user,ip_hq,img_remote_path);
	steplog(cmd,AUTO_OBSERVE_LOG_TYPE);
	system(cmd);
	
	
	//send calibrate files
	sprintf(cmd,"rsync -a %s/ %s@%s:%s ",CALIBRATE_PATH,
				remote_user,ip_hq,cal_remote_path);
	steplog(cmd,AUTO_OBSERVE_LOG_TYPE);
	system(cmd);
	
	
	steplog("Remote back up is finished\nNotify HQ to update database",AUTO_OBSERVE_LOG_TYPE);
	
	i = send_message_to_HQ(UPDATE_DB_CMD);
	
	if(!i)steplog("### Message sent and received",AUTO_OBSERVE_LOG_TYPE);
	else if(i==1) steplog("ERROR: Could not send message",AUTO_OBSERVE_LOG_TYPE);
	else if(i==2) steplog("ERROR: Message sent but we do not know if received correctly",AUTO_OBSERVE_LOG_TYPE);
}

int send_message_to_HQ(char *message)
{
	int sockfd = 0, n = 0;
	char ip_hq[50],remote_user[100],recvBuff[5000],sendBuff[2000],temp_string[5000];
	struct sockaddr_in serv_addr; 
	FILE *fp;


	DoGetValueString("HQ_IP",ip_hq);
	DoGetValueString("REMOTE_USER",remote_user);
	
 	memset(recvBuff, '0',sizeof(recvBuff));
 	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
 	{
 		step("ERROR: Could not create socket");
 		return 1;
 	} 
 
 	memset(&serv_addr, '0', sizeof(serv_addr)); 
 
 	serv_addr.sin_family = AF_INET;
 	serv_addr.sin_port = htons(PORT_HQ); 
 
 	if(inet_pton(AF_INET, ip_hq, &serv_addr.sin_addr)<=0)
 	{
 		step("ERROR: inet_pton error occured");
 		return 1;
 	} 
 
 	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
 	{
 		step("ERROR: Connect Failed");
 		return 1;
 	} 

	sprintf(sendBuff,"%s %s",SITE,message);
	sprintf(temp_string,"Sending: %s",sendBuff);
	step(temp_string);

	write(sockfd,sendBuff,strlen(sendBuff));
	while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
	{
		recvBuff[n] = 0;
// 		if(fputs(recvBuff, stdout) == EOF)
// 		{
// 			step("ERROR : fputs error");
// 		}
	} 

	if(n < 0)
	{
		step("ERROR: Reply error");
		return 2; //Send but not good reply
	}
	
	if(!strcmp(recvBuff,MSG_OK)) //Success
		return 0;

	return 1;
}

