#include "apogee-test.h"
#include <stdio.h>
#include <stdlib.h>

/* Declare globals to store the input command line options */

char imagename[256];
double texposure=1.0;
int  shutter=1;
int ip[4]={0,0,0,0};
int xbin=1;
int ybin=1;
int xstart=0;
int xend=0;
int ystart=0;
int yend=0;
int biascols=0;
int fanmode=0;
double cooling=99.0;
int numexp=1;
int ipause=0;
int verbose=0;
int camnum=1;
int highspeed=0;

int main (int argc, char **argv) 
{
    unsigned short ir, Reg;
    
    alta = (CApnCamera *)new CApnCamera();
    
    if (!alta->InitDriver(camnum,0,0)) {
        printf("ERROR: Failed to initialize driver\n");
        exit(0);
    }
    alta->ResetSystem();
    
    if (verbose == 99) {
       for(ir=0;ir<106;ir++) {
          alta->Read(ir,Reg);
          printf ("Register %d = %d (%x)\n",ir,Reg,Reg);
       }
       exit(0);
    }

    if (argc > 1) {
        fanmode = atoi(argv[1]);
    }
    printf("Setting fan mode to %d\n",fanmode);
    alta->write_FanMode(fanmode);
    
    alta->CloseDriver();
    
    return 0;
}

void *CCD_locate_buffer(char *name, int idepth, short imgcols, short imgrows, short hbin, short vbin)
{
   return 0;
}
