#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "symblc_const.h"
#include <stdlib.h> 

typedef struct
{
	int filter_position; //Specify the filter position
}MOVEWHEEL;

void* move_wheel(void* moveto);
