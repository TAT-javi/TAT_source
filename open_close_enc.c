#include "open_close_enc.h"

void OpenPart(void)
{
	
	steplog ("<< OpenPart >>", AUTO_OBSERVE_LOG_TYPE);
/*
	TurnDspPowerOff();
	timing (time(NULL)+3);
	steplog ("All power on", AUTO_OBSERVE_LOG_TYPE);

	TurnAllPowerOn();	
	timing (time(NULL)+5);	
*/
	steplog("Unlock Latch", AUTO_OBSERVE_LOG_TYPE);
	UnlockLatch_Timing();
	
	steplog("Reset Enclosure", AUTO_OBSERVE_LOG_TYPE);
	ResetEnclosure_Timing();

	steplog("Open enclosure", AUTO_OBSERVE_LOG_TYPE);
	OpenEnclosure_Timing();
	
	//Turn off the dehumidifier
	pwr_lx_poweroff();
	
}


void ClosePart(void)
{
	steplog("<< ClosePart >>", AUTO_OBSERVE_LOG_TYPE);

	steplog("Reset Enclosure", AUTO_OBSERVE_LOG_TYPE);
	ResetEnclosure_Timing();
		
	steplog("Close enclosure", AUTO_OBSERVE_LOG_TYPE);
	CloseEnclosure_Timing();
		
		
	steplog("Lock Latch", AUTO_OBSERVE_LOG_TYPE);
	LockLatch_Timing();
	
	//Turn on the dehumidifier
	pwr_lx_poweron();

}


