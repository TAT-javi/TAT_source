#include <stdio.h>
#include "symblc_const.h"

/*
    Function prototype
*/
    int send_cmd2ccd (char*);
    int mvsend_cmd2ccd (char*);
    void ccd_takeimage (char*, float , char, char);
    void ccd_cooler_on (float);
    void ccd_cooler_off();
	void ccd_abort_takeimage(void);
    void ccd_cooler_shutdown ();
