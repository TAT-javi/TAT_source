#ifndef _DSPFUNC_
    #define _DSPFUNC_
    #include <stdio.h>
    #include <time.h>
    #include "symblc_const.h"
    
/* define */
#define ReturnToOrigin	10
#define InOrigin	11
#define NormalStop	12
#define NormalObsCycle	13
#define NearOrigin	14


/*
    structures
*/
typedef struct 
{
	char motortype[4];
	long currReg;
	long currPulse;
	int  currDir;
	int  currStatus;
	int  currTimeCnt;
} radec_request_info;


/* 
    variables
*/

/*
    Function prototypes
*/

void Wait4DspProcessCmd();
int send_cmd2dsp_file(char*);
int mvsend_cmd2dsp_file(char*);
int send_cmd2dsp_que(char*);
int mvsend_cmd2dsp_que(char*);
int confirm_dsp_cmd( char*);
void send_cmd2dsp_chk( char *);
void mvsend_cmd2dsp_chk(char*);
radec_request_info GetRadecRequestDataFromFile(int);
void standup();
void FastResetTelescope();
void RapidResetTelescope();
void ResetTelescope();
void StopTelescope();
void ParkTelescope();
void ForwardTelescope3Degree();
void ForwardTelescope6Degree();
void MoveTelescope2Zenith();
void MoveTelescope2Dest(long, long);
void MoveHpDown();
void OpenEnclosure();
void ResetDsp();
void CloseEnclosure_Timing();
void UnlockLatch_Timing();
int WaitToCheckPdTotalVolt();
void LockLatch_Timing();
void ResetEnclosure_Timing();
void OpenEnclosure_Timing();
void SafeResetTelescope(void);
#endif
