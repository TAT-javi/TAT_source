#ifndef _PWRFUNC_
    #define  _PWRFUNC_
    #include <stdio.h>
    #include "symblc_const.h"
    
/*
    variables
*/
    char daemonCmd[PPC_CMD_SIZE];    
    
/*
    Function prototype
*/
    
    void pwr_rx_poweron();
    void pwr_rx_poweroff();
    void pwr_lx_poweron();
    void pwr_lx_poweroff();
    int send_cmd2pwr (char*);
    int mvsend_cmd2pwr (char*);

#endif
