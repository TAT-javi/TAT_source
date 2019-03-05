#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "symblc_const.h"
#include "ppcdaemon.h"
#include <stdlib.h> 

typedef struct
{
	long move_position; //Specify the step position of focuser
	long move_rel_pos; //Move the focuser to a relative position
	int home_focuser; // wether move the focuser to home or not
}MOVEFOCUSER;


void* move_focuser(void* moveto);
