#ifndef _PPCFUNC_
    #define  _PPCFUNC_
    #include <stdio.h>
    #include "symblc_const.h"
    #include "main_cmd_menu.h"

/*
    variables
*/
    char daemonCmd[PPC_CMD_SIZE];

/*
    Function prototype
*/

    void TurnPartPowerOff ();
//    void WaitForCcdTemperatureRaise ();
    void TurnCcdMainPowerOff ();
    void TurnAllPowerOn ();
    void TurnAllPowerOff ();
    void TurnAllPowerOffexCCDVDC ();
    void TurnDspPowerOff ();
    int send_cmd2ppc (char*);
    int mvsend_cmd2ppc (char*);
	void TurnWheelOff();
	void ResetDsp();
	void TurnWheelOn();
#endif
