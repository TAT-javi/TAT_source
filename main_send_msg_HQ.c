/*
 * Program to send the data files to the HQ using rsync through ssh
 * It needs to create a ssh_authoritation between HQ and local computer
 */

#include "send_remote_func.h"
#include "symblc_const.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int result;
	char sendBuff[1024];
	
	if(argc == 1)
	{
		printf("#################################\n");
		printf(" Usage: %s [option]\n###\n[option]:\n",argv[0]);
		printf(" b : Send command to update database.\n");
		printf(" e [filter] [time] : Notify that exposure time changed to [time] for [filter].\n");
		printf(" dsp : Notify HQ there is problem with the DSP.\n");
		printf(" hs : Notify HQ there is problem with the Home Switches.\n");
		printf(" enc : Notify HQ there is problem with the Enclosure.\n");
		printf(" ccd : Notify HQ there is problem with the CCD.\n");
		printf(" wea : Notify HQ there is problem with the Weather console.\n###\n");

		return 1;
	} 

	if(!strcmp(argv[1],"b")) strcpy(sendBuff,UPDATE_DB_CMD);
	else if(!strcmp(argv[1],"dsp")) strcpy(sendBuff,ERROR_DSP_NOTE);
	else if(!strcmp(argv[1],"hs")) strcpy(sendBuff,ERROR_HS_NOTE);
	else if(!strcmp(argv[1],"enc")) strcpy(sendBuff,ERROR_ENC_NOTE);
	else if(!strcmp(argv[1],"ccd")) strcpy(sendBuff,ERROR_CCD_NOTE);
	else if(!strcmp(argv[1],"wea")) strcpy(sendBuff,ERROR_WEATHER_NOTE);
	else if(!strcmp(argv[1],"e"))
	{
		if(argc!=4)
		{
			step("ERROR: Not enough data for sending message");
			return -1;
		}
		sprintf(sendBuff,"%s %s %s %s",SITE,EXP_TIME_CHANGED_NOTE,argv[2],argv[3]);
	}
	else
	{
		sprintf(sendBuff,"ERROR: Incorrect option (%s)",argv[1]);
		step(sendBuff);
		return -1;
	}
	
	result = send_message_to_HQ(sendBuff);
	if(!result)
	{
		step("### Message received by the HQ.");
	}
	else
		step("ERROR: Some problems sending message");
	
	return 0;
}



