#include <stdio.h>
#include <time.h>
#include "dsp_func.h"
#include "ppc_func.h"
#include "pwr_func.h"
#include "common_func.h"

void OpenPart(FILE *stepLogFile)
{
	char daemonCmd[40]; 
	steplog ("<< OpenPart >>", stepLogFile);
/*
	TurnDspPowerOff();
	timing (time(NULL)+3);
	steplog ("All power on", stepLogFile);

	TurnAllPowerOn();	
	timing (time(NULL)+5);	
*/
	steplog("Unlock Latch", stepLogFile);
	UnlockLatch_Timing();
	
	steplog("Reset Enclosure", stepLogFile);
	ResetEnclosure_Timing();

	steplog("Open enclosure", stepLogFile);
	OpenEnclosure_Timing();
	
	//Turn off the heater pad
	sprintf(daemonCmd,"L6POWER ON\n");
	send_cmd2pwr(daemonCmd);
}

