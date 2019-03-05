#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "dsp_func.h"
#include "common_func.h"
#include "main_cmd_menu.h"
#include "symblc_const.h"
#include "ppc_func.h"


extern st_tat_info *p_tat_info;
//#define VERBOSE 1

/*
	send command to DSP Daemon for cmd menu
*/
int mvsend_cmd2dsp(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=DSP_CMD_FILENAME;

	for(i=0;i<10;i++)
        {
			if( ( fp=fopen(fpname, "w+") ) == NULL)
			{
					if( i > 8) return 0;
					printf("Open %s failed\n.Try again after %d sec\n", fpname, REOPEN_FILE_SECOND);
					sleep(REOPEN_FILE_SECOND);
			}
			else break;
        }
	mvprintw(MSG_LINE2, 0, "%s",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 1;
}

/*
	send command to DSP Daemon
*/
int send_cmd2dsp(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=DSP_CMD_FILENAME,tempString[200];

	for(i=0;i<10;i++)
	{
		if( ( fp=fopen(fpname, "w+") ) == NULL)
		{
			if( i > 8) return 0;
			printf("Open %s failed\n.Try again after %d sec\n", fpname, REOPEN_FILE_SECOND);
			sleep(REOPEN_FILE_SECOND);
		}
		else break;
	}
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	sprintf(tempString,"send_cmd2dsp=>\n%s",daemonCmd);
	step(tempString);
	return 1;
}

/*
	send command to DSP Daemon and check.
*/

int confirm_dsp_cmd(char *daemonCmd)
{
	char motor[16];
	char action[16];
	char dir[16];
	int reg;
	char trash[16];
	int  stime;
	int  etime;
	sscanf( daemonCmd,"%s %s %s %d %d %d %s",
		motor,dir,action,&stime,&etime,&reg,trash);
#ifdef VERBOSE
	printf("scanned vars: %s %s %s %d\n",
			motor, dir, action, reg);
#endif
	if( etime < 3 )
	{
#ifdef VERBOSE
		printf(" etime=%d < 3\n",
			etime);
#endif
		return 1;
	}


	if( !strcmp( motor, "RA") )
	{
		if( !strcmp( action ,"RESET"))
		{
			if( p_tat_info->dsp_info.ra.status != ReturnToOrigin)
			{
#ifdef VERBOSE
				printf("(dsp_info.ra.status=%d)!=%d\n",
					p_tat_info->dsp_info.ra.status, ReturnToOrigin);
#endif
				return 0;
			}
		}
		else if( !strcmp(action,"FREQUENCY") )
		{
			if( !strcmp(dir,"NEG") )
			{
				if( p_tat_info->dsp_info.ra.dir != 0)
				{
#ifdef VERBOSE
					printf(	"(dsp_info.ra.dir=%d) != 0\n",p_tat_info->dsp_info.ra.dir);
#endif
					return 0;
				}
				if( p_tat_info->dsp_info.ra.reg != reg )
				{
#ifdef VERBOSE
					printf("(dsp_info.ra.reg=%d)!=(reg=%d)\n",
						p_tat_info->dsp_info.ra.reg, reg);
#endif
					return 0;
				}
				if(  p_tat_info->dsp_info.ra.status != NormalObsCycle)
				{
#ifdef VERBOSE
					printf("(dsp_info.ra.status=%d)!=%d\n",
						p_tat_info->dsp_info.ra.status, NormalObsCycle);
#endif
					return 0;
				}
			}
			/* dir = pos */
			else
			{
				if( p_tat_info->dsp_info.ra.dir != 1)
				{
#ifdef VERBOSE
					printf(	"(dsp_info.ra.dir=%d) != 1\n",p_tat_info->dsp_info.ra.dir);
#endif
					return 0;
				}
				if( p_tat_info->dsp_info.ra.reg != reg )
				{
#ifdef VERBOSE
					printf("(dsp_info.ra.reg=%d)!=(reg=%d)\n",
						p_tat_info->dsp_info.ra.reg, reg);
#endif
					return 0;
				}
				if(  p_tat_info->dsp_info.ra.status != NormalObsCycle)
				{
#ifdef VERBOSE
					printf("(dsp_info.ra.status=%d)!=%d\n",
						p_tat_info->dsp_info.ra.status, NormalObsCycle);
#endif
					return 0;
				}
			}
		}
	}
	else if( !strcmp( motor, "DEC") )
	{
		if( !strcmp( action ,"RESET"))
		{
			if( p_tat_info->dsp_info.dec.status != ReturnToOrigin)
			{
#ifdef VERBOSE
				printf("(dsp_info.dec.status=%d)!=%d\n",
					p_tat_info->dsp_info.dec.status, ReturnToOrigin);
#endif
				return 0;
			}
		}
		else if( !strcmp(action,"FREQUENCY") )
		{
			if( !strcmp(dir,"NEG") )
			{
				if( p_tat_info->dsp_info.dec.dir != 0)
				{
#ifdef VERBOSE
					printf(	"(dsp_info.dec.dir=%d) != 0\n",p_tat_info->dsp_info.dec.dir);
#endif
					return 0;
				}
				if( p_tat_info->dsp_info.dec.reg != reg )
				{
#ifdef VERBOSE
					printf("(dsp_info.dec.reg=%d)!=(reg=%d)\n",
						p_tat_info->dsp_info.dec.reg, reg);
#endif
					return 0;
				}
				if(  p_tat_info->dsp_info.dec.status != NormalObsCycle)
				{
#ifdef VERBOSE
					printf("(dsp_info.dec.status=%d)!=%d\n",
						p_tat_info->dsp_info.dec.status, NormalObsCycle);
#endif
					return 0;
				}
			}
			/* dir = pos */
			else
			{
				if( p_tat_info->dsp_info.dec.dir != 1)
				{
#ifdef VERBOSE
					printf(	"(dsp_info.dec.dir=%d) != 0\n",p_tat_info->dsp_info.dec.dir);
#endif
					return 0;
				}
				if( p_tat_info->dsp_info.dec.reg != reg )
				{
#ifdef VERBOSE
					printf("(dsp_info.dec.reg=%d)!=(reg=%d)\n",
						p_tat_info->dsp_info.dec.reg, reg);
#endif
					return 0;
				}
				if(  p_tat_info->dsp_info.dec.status != NormalObsCycle)
				{
#ifdef VERBOSE
					printf("(dsp_info.dec.status=%d)!=%d\n",
						p_tat_info->dsp_info.dec.status, NormalObsCycle);
#endif
					return 0;
				}
			}
		}
	}

	else if( !strcmp( motor, "ENC") )
	{
		if( !strcmp( action, "OPEN"))
		{
			if( p_tat_info->dsp_info.enc.open != 1)
			{
#ifdef VERBOSE
				printf("dsp_info.enc.open=%d != 1\n",
					p_tat_info->dsp_info.enc.open);
#endif
				return 0;
			}
		}
		else if( !strcmp( action, "CLOSE"))
		{
			if( p_tat_info->dsp_info.enc.close != 1)
			{
#ifdef VERBOSE
				printf("dsp_info.enc.close=%d != 1\n",
					p_tat_info->dsp_info.enc.close);
#endif
				return 0;
			}
		}
	}
	else if( !strcmp( motor, "HYP"))
	{
		if( !strcmp( action, "UP"))
		{
			if( p_tat_info->dsp_info.latch.lock != 1)
			{
#ifdef VERBOSE
				printf("dsp_info.latch.lock=%d != 1\n",
					p_tat_info->dsp_info.latch.lock);
#endif
				return 0;
			}
		}
		else if( !strcmp( action, "DOWN"))
		{
			if( p_tat_info->dsp_info.latch.unlock != 1)
			{
#ifdef VERBOSE
				printf("dsp_info.latch.unlock=%d != 1\n",
					p_tat_info->dsp_info.latch.unlock);
#endif
				return 0;
			}
		}
	}
#ifdef VERBOSE
	printf("dsp command confirmed\n");
#endif
	return 1;
}



void send_cmd2dsp_chk( char *daemonCmd)
{
	int counter=0;
	char tempCmd[DSP_CMD_SIZE];
	strcpy(tempCmd,daemonCmd);
	do
	{
		if (counter++ == 5) 
		{
			counter =0;
			ResetDsp();
			sleep(1);
		}
	 	send_cmd2dsp( tempCmd);
	 	sleep(1);
	}while (!confirm_dsp_cmd(tempCmd));
}

void mvsend_cmd2dsp_chk( char *daemonCmd)
{
	do
	{
	 	mvsend_cmd2dsp( daemonCmd);
	 	sleep(1);
	}while (!confirm_dsp_cmd(daemonCmd));
}


/*
	037. move  hp down
*/
void UnlockLatch_Timing()
{
	int latch_unlock_time=20;
	latch_unlock_time=DoGetValue("LATCH_UNLOCK_TIME");
	sprintf( daemonCmd,"HYP NONE DOWN 1 %d 0 0 0 0 0\n",latch_unlock_time );
	send_cmd2dsp_chk(daemonCmd);
	timing(time(NULL)+latch_unlock_time+1);
}

/*
	040.  open enclosure
*/
void OpenEnclosure_Timing()
{
	int enc_open_time = 60;
	enc_open_time = DoGetValue("ENCLOSURE_OPEN_TIME");
	sprintf( daemonCmd,"ENC NONE OPEN 1 %ld 0 0 0 0 0\n",enc_open_time );
	send_cmd2dsp_chk(daemonCmd);

	/*
		050. wait for enclosure opening completed
	*/
	timing( time(NULL) + enc_open_time);
/*
	sprintf( daemonCmd,"ENC NONE OFF 1 0 0 0 0 0 0\n" );
	send_cmd2dsp( daemonCmd );
	Wait4DspProcessCmd();
*/
}

/*
	wait for dsp precessing cmd
*/
void Wait4DspProcessCmd()
{
	char temp_string[50];
	sprintf(temp_string,"sleep %d; wait for daemon process cmd\n",WAIT_FOR_DSP_PROCESS_SECOND);
	printf(temp_string);
	sleep(WAIT_FOR_DSP_PROCESS_SECOND);
}

void CloseEnclosure_Timing()
{
	int enclosure_close_time=50;
	enclosure_close_time=DoGetValue("ENCLOSURE_CLOSE_TIME");

	sprintf( daemonCmd,"ENC NONE CLOSE 1 %d 0 0 0 0 0\n",enclosure_close_time);
	send_cmd2dsp_chk(daemonCmd);

	/*
		140. wait for closing completed
	*/
	step("Wait for enclosure close completed");

	timing( time(NULL) + enclosure_close_time );
/*
	sprintf( daemonCmd,"ENC NONE OFF 1 0 0 0 0 0 0\n" );
	send_cmd2dsp( daemonCmd );
*/
}

void ResetEnclosure_Timing()
{
	sprintf( daemonCmd,"ENC NONE RESET 1 1 0 0 0 0 0\n");
	send_cmd2dsp(daemonCmd);

	/*
		050. wait for enclosure opening completed
	*/
	timing( time(NULL) + DoGetValue("ENCLOSURE_RESET_TIME") +1);
/*
	sprintf( daemonCmd,"ENC NONE OFF 1 0 0 0 0 0 0\n" );
	send_cmd2dsp( daemonCmd );
	Wait4DspProcessCmd();
*/
}



/*
	145. move hp up to lock enc
*/
void LockLatch_Timing()
{
	int latch_lock_time=20;
	latch_lock_time=DoGetValue("LATCH_LOCK_TIME");

	sprintf( daemonCmd,"HYP NONE UP 1 %d 0 0 0 0 0\n",latch_lock_time );
	send_cmd2dsp_chk(daemonCmd);
	timing(time(NULL)+latch_lock_time+1);
}

void ParkTelescope()
{

	RapidResetTelescope();
	/*
		when both motor returned Origin, leave Origin a little ~3 degree.
	*/
	step("Forward Telescope 3 Degree");
	ForwardTelescope3Degree();
	step("Finish parking Telescope");
}


void RapidResetTelescope()
{
	int ra_origin= 0;
	int dec_origin= 0;
	int ra_origin_counter=0,dec_origin_counter=0;
	time_t t_now;
	struct tm *tm_now;
	p_tat_info->obs_info.status =Returning;
	step("Waiting for both status become 11\nRefresh status every 3 seconds");
	
	while(1)
	{
		time( &t_now);
		tm_now = localtime( &t_now);
		printf( "RA status=%d (%s) | DEC status=%d (%s)\t%02d:%02d:%02d\r",
			p_tat_info->dsp_info.ra.status,p_tat_info->dsp_info.ra.origin?"In origin":" -Moving-",
			p_tat_info->dsp_info.dec.status,p_tat_info->dsp_info.dec.origin?"In origin":" -Moving-",
			tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);
		fflush(NULL);
		switch( p_tat_info->dsp_info.ra.status )
		{
			case NearOrigin:

			break;
			case InOrigin:
				if(!p_tat_info->dsp_info.ra.origin)
				{
					printf("\nRA is not in Origin!!!\n");
					sprintf( daemonCmd, "RA POS FREQUENCY 1 10 208 0 0 0 0");
					mvsend_cmd2dsp( daemonCmd );
				}
				else
					ra_origin=1;
			break;
			case ReturnToOrigin:
				if(p_tat_info->dsp_info.ra.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "RA POS FREQUENCY 1 10 208 0 0 0 0");
					mvsend_cmd2dsp( daemonCmd );
				}
			break;
			case NormalStop:
				printf("\n");
				sprintf( daemonCmd, "RA NEG FREQUENCY 1 60 208 0 0 0 0\n");
				send_cmd2dsp( daemonCmd );
			break;
			case NormalObsCycle:
				if( p_tat_info->dsp_info.ra.dir == 1 && !p_tat_info->dsp_info.ra.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "RA POS FREQUENCY 1 1 65534 0 0 0 0\n");
					send_cmd2dsp( daemonCmd );
				}
				
				if(p_tat_info->dsp_info.ra.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "RA POS FREQUENCY 1 20 208 0 0 0 0\n");
					send_cmd2dsp( daemonCmd );
				}
			break;
		}
		sleep(1);
		switch( p_tat_info->dsp_info.dec.status )
		{
			case NearOrigin:

			break;
			case InOrigin:
				if(!p_tat_info->dsp_info.dec.origin)
				{
					printf("\nDEC is not in Origin!!!\n");
					sprintf( daemonCmd, "DEC POS FREQUENCY 1 10 208 0 0 0 0");
					mvsend_cmd2dsp( daemonCmd );
				}
				else
					dec_origin=1;
			break;
			case ReturnToOrigin:
				if(p_tat_info->dsp_info.dec.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "DEC POS FREQUENCY 1 10 208 0 0 0 0");
					mvsend_cmd2dsp( daemonCmd );
				}
			break;
			case NormalStop:
				printf("\n");
				sprintf( daemonCmd, "DEC NEG FREQUENCY 1 60 208 0 0 0 0\n");
				send_cmd2dsp( daemonCmd );
			break;
			case NormalObsCycle:
				if( p_tat_info->dsp_info.dec.dir == 1 && !p_tat_info->dsp_info.dec.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "DEC POS FREQUENCY 1 1 65534 0 0 0 0\n");
					send_cmd2dsp( daemonCmd );
				}
				
				if(p_tat_info->dsp_info.dec.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "DEC POS FREQUENCY 1 20 208 0 0 0 0\n");
					send_cmd2dsp( daemonCmd );
				}
			break;
		}
		sleep (2);

		if( ra_origin && dec_origin )
		{
			printf("\nRA and DEC are both in origin\n");
			p_tat_info->obs_info.status = InOrigin;
			return;
		}

	}
}

void ResetTelescope()
{
	int ra_origin= 0,dec_origin= 0;
	int dec_origin_counter=0,ra_origin_counter=0;
	time_t t_now;
	struct tm *tm_now;
	p_tat_info->obs_info.status =Returning;
	step("Waiting for both status become 11\nRefresh status every 3 seconds");
	
	while(1)
	{
		time( &t_now);
		tm_now = localtime( &t_now);
		printf( "RA status=%d (%s) | DEC status=%d (%s)\t%02d:%02d:%02d\r",
			p_tat_info->dsp_info.ra.status,p_tat_info->dsp_info.ra.origin?"In origin":" -Moving-",
			p_tat_info->dsp_info.dec.status,p_tat_info->dsp_info.dec.origin?"In origin":" -Moving-",
			tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);
		fflush(NULL);
		switch( p_tat_info->dsp_info.ra.status )
		{
			case NearOrigin:

			break;
			case InOrigin:
				if(!p_tat_info->dsp_info.ra.origin)
				{
					printf("\nRA is not in Origin!!!\n");
					sprintf( daemonCmd, "RA POS FREQUENCY 1 10 417 0 0 0 0");
					mvsend_cmd2dsp( daemonCmd );
				}
				else
					ra_origin=1;
			break;
			case ReturnToOrigin:
				
				if(p_tat_info->dsp_info.ra.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "RA POS FREQUENCY 1 20 417 0 0 0 0");
					mvsend_cmd2dsp( daemonCmd );
				}
			break;
			case NormalStop:
				printf("\n");
				sprintf( daemonCmd, "RA NEG RESET 1 9999 417 0 0 0 0\n");
				send_cmd2dsp( daemonCmd );
			break;
			case NormalObsCycle:
				if( p_tat_info->dsp_info.ra.dir == 1 && !p_tat_info->dsp_info.ra.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "RA POS FREQUENCY 1 1 65534 0 0 0 0\n");
					send_cmd2dsp( daemonCmd );
				}
				
				if(p_tat_info->dsp_info.ra.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "RA POS FREQUENCY 1 20 417 0 0 0 0\n");
					send_cmd2dsp( daemonCmd );
				}
// 				
			break;
		}
		sleep(1);
		switch( p_tat_info->dsp_info.dec.status )
		{
			case NearOrigin:

			break;
			case InOrigin:
				if(!p_tat_info->dsp_info.dec.origin)
				{
					printf("\nDEC is not in Origin!!!\n");
					sprintf( daemonCmd, "DEC POS FREQUENCY 1 10 417 0 0 0 0");
					mvsend_cmd2dsp( daemonCmd );
				}
				else
					dec_origin=1;
			break;
			case ReturnToOrigin:
				
				if(p_tat_info->dsp_info.dec.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "DEC POS FREQUENCY 1 20 417 0 0 0 0");
					mvsend_cmd2dsp( daemonCmd );
				}
			break;
			case NormalStop:
				printf("\n");
				sprintf( daemonCmd, "DEC NEG RESET 1 9999 417 0 0 0 0\n");
				send_cmd2dsp( daemonCmd );
			break;
			case NormalObsCycle:
				
				if( p_tat_info->dsp_info.dec.dir == 1 && !p_tat_info->dsp_info.dec.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "DEC POS FREQUENCY 1 1 65534 0 0 0 0\n");
					send_cmd2dsp( daemonCmd );
				}
				
				if(p_tat_info->dsp_info.dec.origin)
				{
					printf("\n");
					sprintf( daemonCmd, "DEC POS FREQUENCY 1 20 417 0 0 0 0\n");
					send_cmd2dsp( daemonCmd );
				}

			break;
		}
		sleep (2);

		if( ra_origin && dec_origin )
		{
			step("### RA and DEC are both in origin");
			p_tat_info->obs_info.status = InOrigin;
			return;
		}

	}
}

void ForwardTelescope30Degree()
{
	p_tat_info->obs_info.status = Moving;
	/*
		when both motor arrive in Origin, leave Origin ~30 degree.
	*/

	step("Forward RA and DEC stage 1: 5 sec with 1kHz");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 15 417 0 0 0 0\nDEC POS FREQUENCY 1 15 417 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~1kHz*/

		timing(time(NULL)+5);

	step("Forward RA and DEC stage 2: 80 sec with 2kHz");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 80 208 0 0 0\nDEC POS FREQUENCY 1 80 208 0 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~2kHz*/
		timing(time(NULL)+80);

	step("Forward RA and DEC stage 3: 5 sec with 1kHz");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 5 417 0 0 0 0\nDEC POS FREQUENCY 1 5 417 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~1kHz*/
		timing(time(NULL)+5);

	/*
		timing time must be larger than motor actual move time ,
		in order to change Motor status to 'NormalStop=12'
	*/
	step("Wait for Telescope to stop");
	timing(time(NULL)+ 3 );
	/* plus 2 to let  motor stop.  */
	p_tat_info->obs_info.status = STOP;
}



void ForwardTelescope3Degree()
{
	p_tat_info->obs_info.status =Moving;

	/*
		when both motor arrive in Origin, leave Origin ~3 degree.
		Lets make it different
		RA - 3 degree
		DEC - 3 degree
	*/


// 	sprintf(daemonCmd,"RA POS FREQUENCY 1 15 417 0 0 0 0\nDEC POS FREQUENCY 1 15 417 0 0 0\n");
	step("Forward RA and DEC 3 Degree");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 5 417 0 0 0 0\nDEC POS FREQUENCY 1 5 417 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~1kHz*/

		timing(time(NULL)+5);

// 	step("Forward RA and DEC stage 2: 10 sec with 2kHz");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 10 417 0 0 0\nDEC POS FREQUENCY 1 10 417 0 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~2kHz*/
		timing(time(NULL)+15);//wait longer to be sure

// 	step("Forward RA and DEC stage 3: 5 sec with 1kHz");
// 	sprintf(daemonCmd,"RA POS FREQUENCY 1 5 417 0 0 0 0\nDEC POS FREQUENCY 1 5 417 0 0 0\n");
// 	send_cmd2dsp_chk(daemonCmd);	/*~1kHz*/
// 	timing(time(NULL)+8); //wait longer to be sure

	/* plus 2 to let  motor stop.  */
	p_tat_info->obs_info.status =STOP;
}

/*Move ~6 degrees the telescope, for safety*/
void ForwardTelescope6Degree()
{
	p_tat_info->obs_info.status =Moving;

	step("Forward RA and DEC ~6 Degree");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 5 417 0 0 0 0\nDEC POS FREQUENCY 1 5 417 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~1kHz*/

	timing(time(NULL)+5);

// 	step("Forward RA and DEC stage 2: 20 sec with 2kHz");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 20 208 0 0 0\nDEC POS FREQUENCY 1 20 208 0 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~2kHz*/
		timing(time(NULL)+20);

// 	step("Forward RA and DEC stage 3: 5 sec with 1kHz");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 5 417 0 0 0 0\nDEC POS FREQUENCY 1 5 417 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~1kHz*/
	timing(time(NULL)+8); //wait longer to be sure

	/* plus 2 to let  motor stop.  */
	p_tat_info->obs_info.status =STOP;
}



void BackwardTelescope3Degree()
{

	/*
		when both motor returned Origin, leave Origin a little ~3 degree.
	*/
	step("Backward RA and DEC stage 1: 10 sec with 1kHz");
	sprintf(daemonCmd,"RA NEG FREQUENCY 1 15 417 0 0 0 0\nDEC NEG FREQUENCY 1 15 417 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~1kHz*/

		timing(time(NULL)+10);

	step("Forward RA and DEC stage 2: 10 sec with 2kHz");
	sprintf(daemonCmd,"DEC NEG FREQUENCY 1 10 208 0 0 0 0\nRA NEG FREQUENCY 1 10 208 0 0 0\n");
	send_cmd2dsp_chk(daemonCmd);	/*~2kHz*/

	/*
		timing time must be larger than motor actual move time ,
		in order to change Motor status to 'NormalStop=12'
	*/
	step("wait for Telescope stop");
	timing(time(NULL)+ 15+ 2 );
	/* plus 2 to let  motor stop.  */
}

void StopTelescope()
{

	/*
		when both motor returned Origin, leave Origin a little ~3 degree.
	*/
	step("Stop RA and DEC : 2 sec with 374 Hz");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 2 11111 0 0 0 0\nDEC POS FREQUENCY 1 2 11111 0 0 0\n");
	send_cmd2dsp(daemonCmd);	/*~374 Hz*/

	/*
		timing time must be larger than motor actual move time ,
		in order to change Motor status to 'NormalStop=12'
	*/
	step("Wait for Telescope to stop");
	timing(time(NULL)+ 2 +2 );
	/* plus 2 to let  motor stop.  */
	p_tat_info->obs_info.status =STOP;
}

void MoveTelescope2Zenith()
{
	FILE *fp;
	double	ra_pulse_zenith, dec_pulse_zenith;
	long	ra_differ_pulse, dec_differ_pulse;


	fp=fopen( REF_PAR_FILENAME, "r");
	if( fp == NULL )
	{
		step("ERROR: Can't read reference parameter");
		exit(1);
	}
	fscanf( fp,"%lf", &ra_pulse_zenith); /* read value in unit degree */
	fscanf( fp,"%lf", &dec_pulse_zenith);/* degrees */
	fclose(fp);

	printf( "%lf %lf\n", ra_pulse_zenith, dec_pulse_zenith);

	/* RA : ratio of degree to pulse  = 1:10000 */
	/* DEC: ratio of degree to pulse  = 1:6000 */
	ra_pulse_zenith*=10000;
	dec_pulse_zenith*=6000;
	printf( "%lf %lf\n", ra_pulse_zenith, dec_pulse_zenith);

	/* calculate pulses needed to move telescope pointing zenith */
	ra_differ_pulse  =  ra_pulse_zenith+0.5 - p_tat_info->dsp_info.ra.pulse;
	dec_differ_pulse = dec_pulse_zenith+0.5 - p_tat_info->dsp_info.dec.pulse;

	printf("%ld %ld\n", ra_differ_pulse, dec_differ_pulse);

	/* Criterion Pulse = 9000? or setting a rough value, such as 3 or 5 degrees
	   ( 30000 or 50000 pulses)

	   candidate algorithm 1:
	   If abs(differ_pulse) > Criterion (9000),
	   moving RA with 1kHz in 3 seconds.
	   Then moving with 2kHz in at least 3 seconds.
	   If in lesser case, moving with 1kHz only.

	   candidate algorithm 2: the same.
	   But when lesser, don't move anything.

	   Anyway, we must move dec only or both in "greater" case .
	*/

	while(1)
	{
		sprintf(daemonCmd,"RA %s LIMIT 1 1 %ld 0 0 0 0\nDEC %s LIMIT 1 1 %ld 0 0 0 0\n",
			ra_differ_pulse>0?"POS":"NEG", abs(ra_differ_pulse),
			dec_differ_pulse>0?"POS":"NEG", abs(dec_differ_pulse));
		send_cmd2dsp(daemonCmd);
		sleep(1);
		if( p_tat_info->dsp_info.ra.pulsectl == 1 /* enable */
		    && p_tat_info->dsp_info.dec.pulsectl == 1)
		{
			break;
		}
		step("WARNING: Sending limit command to DSP failed.");
	}

	if( abs(dec_differ_pulse) > 9000 )
	{
		step("Stage 1: Moving DEC ");
		sprintf(daemonCmd,"DEC %s FREQUENCY 1 10 417 0 0 0 0\n",
			dec_differ_pulse>0?"POS":"NEG");
		send_cmd2dsp(daemonCmd);
		timing(time(NULL)+3);

		sprintf(daemonCmd,"DEC %s FREQUENCY 1 %d 208 0 0 0 0\n",
			dec_differ_pulse>0?"POS":"NEG",
			abs(dec_differ_pulse)/2000+60);
		send_cmd2dsp(daemonCmd);
		sleep(1);
	}
	else
	{
		sprintf(daemonCmd,"DEC %s FREQUENCY 1 10 417 0 0 0 0\n",
			dec_differ_pulse>0?"POS":"NEG");
		send_cmd2dsp(daemonCmd);
		sleep(1);
	}

	if( abs(ra_differ_pulse) > 9000 )
	{
		step("Stage 2: Moving RA");
		sprintf(daemonCmd,"RA %s FREQUENCY 1 10 417 0 0 0 0\n",
			ra_differ_pulse>0?"POS":"NEG");
		send_cmd2dsp(daemonCmd);
		timing(time(NULL)+3);

		sprintf(daemonCmd,"RA %s FREQUENCY 1 %d 208 0 0 0 0\n",
			ra_differ_pulse>0?"POS":"NEG",
			abs(ra_differ_pulse)/2000+60);
		send_cmd2dsp(daemonCmd);
	}
	else
	{
		sprintf(daemonCmd,"RA %s FREQUENCY 1 10 417 0 0 0 0\n",
			ra_differ_pulse>0?"POS":"NEG");
		send_cmd2dsp(daemonCmd);
	}

	step("Moving Telescope to Zenith Position");
	while(1)
	{

		if( p_tat_info->dsp_info.ra.pulsectl == 2 /* done */
		    && p_tat_info->dsp_info.dec.pulsectl == 2 )
		{
			step("### Telescope has pointed the zenith.");
			break;
		}
		sleep(1);
		printf("Remainder pulse: RA %7ld, DEC %7ld, Left time %4d\r",
			p_tat_info->dsp_info.ra.purpulse,
			p_tat_info->dsp_info.dec.purpulse,
			(p_tat_info->dsp_info.ra.purpulse>p_tat_info->dsp_info.dec.purpulse?
			p_tat_info->dsp_info.ra.purpulse:p_tat_info->dsp_info.dec.purpulse)/2000);
		fflush(NULL);
	}

	p_tat_info->obs_info.status =STOP;
}

void MoveTelescope2Dest(long ra_pulse_dest, long dec_pulse_dest)
{

	long ra_differ_pulse, dec_differ_pulse;
	printf( "%lf %lf\n", ra_pulse_dest, dec_pulse_dest);

	/* calculate pulses needed to move telescope pointing dest */
	ra_differ_pulse  =  ra_pulse_dest+0.5 - p_tat_info->dsp_info.ra.pulse;
	dec_differ_pulse = dec_pulse_dest+0.5 - p_tat_info->dsp_info.dec.pulse;

	printf("%ld %ld\n", ra_differ_pulse, dec_differ_pulse);

	/* Criterion Pulse = 9000? or setting a rough value, such as 3 or 5 degrees
	   ( 30000 or 50000 pulses)

	   candidate algorithm 1:
	   If abs(differ_pulse) > Criterion (9000),
	   moving RA with 1kHz in 3 seconds.
	   Then moving with 2kHz in at least 3 seconds.
	   If in lesser case, moving with 1kHz only.

	   candidate algorithm 2: the same.
	   But when lesser, don't move anything.

	   Anyway, we must move dec only or both in "greater" case .
	*/

	while(1)
	{
		sprintf(daemonCmd,"RA %s LIMIT 1 1 %ld 0 0 0 0\nDEC %s LIMIT 1 1 %ld 0 0 0 0\n",
			ra_differ_pulse>0?"POS":"NEG", abs(ra_differ_pulse),
			dec_differ_pulse>0?"POS":"NEG", abs(dec_differ_pulse));
		send_cmd2dsp(daemonCmd);
		sleep(1);
		if( p_tat_info->dsp_info.ra.pulsectl == 1 /* enable */
		    && p_tat_info->dsp_info.dec.pulsectl == 1)
		{
			break;
		}
		printf("Sending limit command to DSP failed,\n");
	}

	if( abs(dec_differ_pulse) > 9000 )
	{
		step("Stage 1-1: Moving DEC with 1 kHz in 3 seconds");
		sprintf(daemonCmd,"DEC %s FREQUENCY 1 10 417 0 0 0 0\n",
			dec_differ_pulse>0?"POS":"NEG");
		send_cmd2dsp(daemonCmd);
		timing(time(NULL)+3);

		step("Stage 1-2: Moving DEC with 2 kHz in not more than X seconds");
		sprintf(daemonCmd,"DEC %s FREQUENCY 1 %d 208 0 0 0 0\n",
			dec_differ_pulse>0?"POS":"NEG",
			abs(dec_differ_pulse)/2000+60);
		send_cmd2dsp(daemonCmd);
		sleep(1);
	}
	else
	{
		sprintf(daemonCmd,"DEC %s FREQUENCY 1 10 417 0 0 0 0\n",
			dec_differ_pulse>0?"POS":"NEG");
		send_cmd2dsp(daemonCmd);
		sleep(1);
	}

	if( abs(ra_differ_pulse) > 9000 )
	{
		step("Stage 2-1: Moving RA with 1 kHz in 3 seconds");
		sprintf(daemonCmd,"RA %s FREQUENCY 1 10 417 0 0 0 0\n",
			ra_differ_pulse>0?"POS":"NEG");
		send_cmd2dsp(daemonCmd);
		timing(time(NULL)+3);

		step("Stage 2-2: Moving RA with 2 kHz in not more than X seconds");
		sprintf(daemonCmd,"RA %s FREQUENCY 1 %d 208 0 0 0 0\n",
			ra_differ_pulse>0?"POS":"NEG",
			abs(ra_differ_pulse)/2000+60);
		send_cmd2dsp(daemonCmd);
	}
	else
	{
		sprintf(daemonCmd,"RA %s FREQUENCY 1 10 417 0 0 0 0\n",
			ra_differ_pulse>0?"POS":"NEG");
		send_cmd2dsp(daemonCmd);
	}

	printf("\nMoving Telescope to Destination Position\n");
	while(1)
	{

		if( p_tat_info->dsp_info.ra.pulsectl == 2 /* done */
		    && p_tat_info->dsp_info.dec.pulsectl == 2 )
		{
			printf("\n\nTelescope has pointed toward destination.\n");
			break;
		}
		sleep(1);
		printf("Remainder pulse: RA %7ld, DEC %7ld, Left time %4d\r",
			p_tat_info->dsp_info.ra.purpulse,
			p_tat_info->dsp_info.dec.purpulse,
			(p_tat_info->dsp_info.ra.purpulse>p_tat_info->dsp_info.dec.purpulse?
			p_tat_info->dsp_info.ra.purpulse:p_tat_info->dsp_info.dec.purpulse)/2000);
		fflush(NULL);
	}
	p_tat_info->obs_info.status =STOP;
}

void SafeResetTelescope(void)
{
	int i;
	char txt[30];
    /* Perhaps telescope is in origin, forward telescope 3 degree first */
	steplog("Forward Telescope 3 Degree", AUTO_OBSERVE_LOG_TYPE);
	ForwardTelescope3Degree();

	/* Return telescope */
	p_tat_info->obs_info.status = Returning;
	steplog("Reset Telescope", AUTO_OBSERVE_LOG_TYPE);
	RapidResetTelescope();

	/* After ResetTelescope(), RA and DEC should be in origin */
	/* make sure DEC and RA are both in origin */
	for(i=0;i<5;i++)
	{
		if( !p_tat_info->dsp_info.ra.origin ||	!p_tat_info->dsp_info.dec.origin )
		{
			ForwardTelescope6Degree();//6 degrees
	// 		ForwardTelescope30Degree();
			sprintf(txt,"Telescop is not at origin (%d try)",i+1);
			steplog(txt, AUTO_OBSERVE_LOG_TYPE);
			ResetTelescope();
		}
	}
	if( !p_tat_info->dsp_info.ra.origin ||	!p_tat_info->dsp_info.dec.origin )
	{
		/* can't confirm telescope position again*/
		steplog("Can't confirm telescope position again", AUTO_OBSERVE_LOG_TYPE);
		exit(1);
	}
	p_tat_info->obs_info.status = InOrigin;
}

void Origin2zenith(void)
{
	FILE *fp;
	double	ra_pulse_zenith, dec_pulse_zenith;
	long	ra_differ_pulse, dec_differ_pulse;
	long	init_ra_differ_pulse, init_dec_differ_pulse;
	int counter_dec, counter_ra, limit_counter=0;


	fp=fopen( REF_PAR_FILENAME, "r");
	if( fp == NULL )
	{
		step("ERROR: Can not read reference parameter");
		exit(1);
	}
	fscanf( fp,"%lf", &ra_pulse_zenith); /* read value in unit degree */
	fscanf( fp,"%lf", &dec_pulse_zenith);/* degrees */
	fclose(fp);

	printf( "%lf %lf ==> ", ra_pulse_zenith, dec_pulse_zenith);

	/* RA : ratio of degree to pulse  = 1:10000 */
	/* DEC: ratio of degree to pulse  = 1:6000 */
	/* pulses needed to move telescope pointing zenith */
	ra_differ_pulse  =  (ra_pulse_zenith*10000)+0.5;
	dec_differ_pulse = (dec_pulse_zenith*6000)+0.5;

	init_ra_differ_pulse = ra_differ_pulse;
	init_dec_differ_pulse = dec_differ_pulse;
	
	printf("%ld %ld\n", ra_differ_pulse, dec_differ_pulse);


	step("Stage 1: Moving RA and DEC with 1 kHz in 3 seconds");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 10 417 0 0 0 0\nDEC POS FREQUENCY 1 10 417 0 0 0 0\n");
	send_cmd2dsp(daemonCmd);
	timing(time(NULL)+3);

	counter_dec= (dec_differ_pulse/2000)+0.5;
	counter_ra= (ra_differ_pulse/2000)+0.5;
	
	if(counter_dec < counter_ra) limit_counter = counter_ra;
	else limit_counter = counter_dec;
	
	step("Stage 2: Moving RA and DEC with 2 kHz in not more than X seconds");
	sprintf(daemonCmd,"RA POS FREQUENCY 1 %d 208 0 0 0 0\nDEC POS FREQUENCY 1 %d 208 0 0 0 0\n",
			counter_ra,counter_dec);
	send_cmd2dsp_chk(daemonCmd);
	sleep(1);
	
	limit_counter+=3;
	printf("\n");
	while(limit_counter >= 0)
	{
		sleep(1);
		ra_differ_pulse = init_ra_differ_pulse - p_tat_info->dsp_info.ra.pulse;
		dec_differ_pulse= init_dec_differ_pulse - p_tat_info->dsp_info.dec.pulse;
		//To avoid the observer to freak out
		if(ra_differ_pulse < 0) ra_differ_pulse =0;
		if(dec_differ_pulse < 0)dec_differ_pulse=0;
		
// 		printf("Reminder Pulse: RA %7d (%d,%d), DEC %7d (%d,%d), Remainder time = %d  \t\r",
// 					ra_differ_pulse,init_ra_differ_pulse,p_tat_info->dsp_info.ra.pulse,
// 					dec_differ_pulse, init_dec_differ_pulse, p_tat_info->dsp_info.dec.pulse,
// 					limit_counter);
		printf("Remainder Pulse: RA %7d, DEC %7d , Remainder time = %d  \t\r",
					ra_differ_pulse,dec_differ_pulse,limit_counter);
		
		limit_counter--;
		fflush(NULL);
	}
	printf("\n\nTelescope has pointed toward zenith.\n");
}

