/*
	TEN Hardware Powers Control daemon
	----------------------------------
Control the hardware powers through parallel port IO card.
How to compile:
cc pportctr.daemon-v3a.c -O2
Optimized option is needed when compile for inb/outb functions.

6.Feb 2003
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include "symblc_const.h"
#include "ppcdaemon.h"
#include "tat_info.h"
#include "par_func.h"
/*
Note for Parallel Port DB25 PINS:(PRIME '  mean that bit is invert logic)
PIN1	:	IN/OUT			(C0')
PIN2-9	:	IN/OUT(grouped)		(D0-D7)
PIN10	:	IN, INTERRUPT GENERATOR (S6)
PIN11	:	IN			(S7')
PIN12	:	IN			(S5)
PIN13	:	IN			(S4)
PIN14	:	IN/OUT			(C1')
PIN15	:	IN			(S3)
PIN16	:	IN/OUT			(C2)
PIN17	:	IN/OUT			(C3')
PIN18-25:	GND

*/

/*
In Linux, the address of the PCI IO card can be found using the command:
'lspci -v' (as root)
or type
'more /proc/pci'
*/

/*#define DATA		0xefe0*/
/*#define DATA		0x378*/

/********************************************/

#define DATA		PPC_DATA	/* @symblc.const.h */

#define	STATUS	DATA+1
#define CONTROL	DATA+2
/********************************************/
/*
  Usually,PULL DOWN for on.
  
 MAINPWR	C0'	PIN 01	PULL UP->ON

 RAPWR		D0	PIN 02
 DECPWR		D1	PIN 03
 ROTPWR		D2	PIN 04
 TRANPWR	D3      PIN 05
 CCDPWR		D4      PIN 06
 WHEELPWR	D5      PIN 07
 DSPPWR		D6      PIN 08
 DSPJP	D7      PIN 09
 VDCPWR	C2	PIN 16
 
*/     
#define MAINPWR		20
#define RAPWR		21
#define DECPWR		22
#define ROTPWR		23
#define TRANPWR		24
#define CCDPWR		25
#define WHEELPWR	26
#define DSPPWR		27	
#define DSPJP		28	
#define VDCPWR		32

#define ON	1	/* set voltage high*/
#define OFF	0	/* set voltage low*/


/***************************************************/


#define CYCLE_TIME	100000	/* microseconds usleep */

void SetRelaySwitch	(unsigned char type, unsigned char mode);
void inb_value_parsing(unsigned char *status,int inb_value1);

st_tat_info *p_tat_info;

int main(int argc, char *argv[])
{
	pid_t pid;
	FILE *CommandFile;
	int PowerSource=0, PowerSwitch=0;

	char CmdSource[12];
	char CmdOnOff[12];
/*	char systemCmd[20];*/
	unsigned char startAsDaemon=0;
	
	time_t currTime;
	char *pcCurrTime;
	FILE *fp;

	unsigned char msg[3];
	int dsp_time_count=0, chk_dsp_time_count=0;
	
	if(argc==2 )
	{
		if( strcmp(argv[1],"-d")==0)
		{
			startAsDaemon=1;
		}
	}
	
	while(1)
	{
		pid=fork();
		if(pid==-1)
		{
		/*      error*/
				perror("-1");
		}
		else if(pid!= 0)
		{
		/*      parent*/
			if(startAsDaemon)
			{
						printf("PPCDaemon start as Daemon now\n");
						exit( 0);
				}
				else
			{
				printf("wait child pid =%d\n",pid);
				waitpid(pid,(int *)0,0);
				printf("child pid= %d exit\n", pid);
				
				time(&currTime);
				pcCurrTime= ctime( &currTime);
				fp= fopen(DAEMON_LOG_FILENAME, "a+");
				fprintf( fp, "%s ppcdaemon child pid=%d exit.\n", pcCurrTime, pid);
				fclose( fp);
				
			}
		}
		else
		{
                /*      child*/
			break;
		}
	}       /*end while */

	/*
	Use iopl(3) to grant permission to access IO ports beyond 0x3ff
	*/
	iopl(3);
	umask (0);
	remove (PPC_CMD_FILENAME);
	printf("PPC Saemon Start.\n");
	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
                
	
	while(1)
	{
		if( ( CommandFile = fopen(PPC_CMD_FILENAME, "r") ) == NULL )
		{
// 		        if( !(inb(DATA) & DSPPWRREG) )
// 		        {
// 		        	if( chk_dsp_time_count == 3000 )
// 		        	{
// 		        		chk_dsp_time_count = 0;
// 		        		if( dsp_time_count==p_tat_info->dsp_info.time_count)
// 		        		{
// 		        			printf("DSP count didn't increase in the past 30 seconds\nMaybe DSP is hang\n");
// 		        			printf("Turn off DSP power. After 5 seconds, will turn on DSP power.\n");
// 		 				SetRelaySwitch(DSPPWR, OFF);       			
// 		 				sleep(5);
// 		 				SetRelaySwitch(DSPPWR, ON);
// 					}
// 					else
// 						dsp_time_count=p_tat_info->dsp_info.time_count;
//                         	}
//                         	else
//                         		chk_dsp_time_count++;
// 			}
			usleep(CYCLE_TIME);	/*Command file not FOUND!*/
		}
		else
		{
			while(!feof(CommandFile))
			{
				fscanf(CommandFile, "%s %s\n",	CmdSource, CmdOnOff);
				printf("PowerSource:%s, On/Off:%s,\n",
					CmdSource, CmdOnOff);

				if( strcmp(CmdOnOff, "ON") == 0)
					PowerSwitch=ON;
				else	PowerSwitch=OFF;

				if( strcmp(CmdSource, "MAINPOWER")  == 0 )	PowerSource= MAINPWR;
				else
				if( strcmp(CmdSource, "RAPOWER")  == 0 )	PowerSource= RAPWR;
				else
				if( strcmp(CmdSource, "DECPOWER") == 0 )	PowerSource= DECPWR;
				else
				if( strcmp(CmdSource, "ROTPOWER") == 0 )	PowerSource= ROTPWR;
				else
				if( strcmp(CmdSource, "TRANPOWER")== 0 )	PowerSource= TRANPWR;
				else
				if( strcmp(CmdSource, "CCDPOWER") == 0 )	PowerSource= CCDPWR;
				else
				if( strcmp(CmdSource, "WHEELPOWER")== 0 )	PowerSource= WHEELPWR;
				else
				if( strcmp(CmdSource, "DSPPOWER") == 0 )	PowerSource= DSPPWR;
				else
				if( strcmp(CmdSource, "DSPJUMPER")== 0 )	PowerSource= DSPJP;
				else
				if( strcmp(CmdSource, "VDCPOWER") == 0 )	PowerSource= VDCPWR;
				else
					perror("Unknown device");
				SetRelaySwitch(PowerSource, PowerSwitch);
			}
			remove(PPC_CMD_FILENAME);
			
			
		}
		/* update ppc info */
		msg[0]=inb(CONTROL);
		msg[1]=inb(DATA);
		msg[2]=inb(STATUS);
//		printf("%x %x %x\n", msg[0], msg[1], msg[2]);
		update_ppc_info( &(p_tat_info->ppc_info), msg);
	}	/* while(1)*/


	return 0;
}

void SetRelaySwitch(unsigned char type, unsigned char mode)	
/*PARALLEL PORT PIN1: C0'INVERTED LOGIC*/
{
/* unused variable */
/*
	unsigned char inb_value;
*/      
	unsigned char status[9]={2,2,2,2,2,2,2,2,'\0'};
	char temp_string[40];
	FILE *fp;
	
	switch(type)
	{
		case MAINPWR:
			switch(mode)
			{
				case ON:	
				        /* main power (C0' PIN01): pull up for on*/
					pio_wr( DATA, 1, 0);
					printf("Turning ON MainPower\n");
					
					/* vdc power: pull down for on */
					#ifdef UZBEKISTAN
					  /* D7 PIN09 */
					  pio_wr( DATA, 9, 0);
					#else
					  /* C2 PIN16 */
					  pio_wr( DATA, 16, 0);
					#endif
					 
					printf("Turning ON VDC Power\n");
				break;
				case OFF:	
				        /* main power (C0' PIN01): pull up for on*/
					pio_wr( DATA, 1, 1);
					printf("Turning OFF MainPower\n");
					
					/* vdc power: pull down for on */
					#ifdef UZBEKISTAN
					  /* D7 PIN09 */
					  pio_wr( DATA, 9, 1);
					#else
					  /* C2 PIN16 */
					  pio_wr( DATA, 16, 1);
					#endif
					 
					printf("Turning OFF VDC Power\n");
				break;
				default:
				break;
			}
		break;

		case VDCPWR:
			switch(mode)
			{
			        case ON:	
					/* vdc power: pull down for on */
					#ifdef UZBEKISTAN
					  /* D7 PIN09 */
					  pio_wr( DATA, 9, 0);
					#else
					  /* C2 PIN16 */
					  pio_wr( DATA, 16, 0);
					#endif
					 
					printf("Turning ON VDC Power\n");
				break;
				case OFF:	
					/* vdc power: pull down for on */
					#ifdef UZBEKISTAN
					  /* D7 PIN09 */
					  pio_wr( DATA, 9, 1);
					#else
					  /* C2 PIN16 */
					  pio_wr( DATA, 16, 1);
					#endif
					 
					printf("Turning OFF VDC Power\n");
				break;
				default:
				break;
			}
		break;
		
		case DSPJP:
			switch(mode)
			{
			        case ON:	
					  /* D7 PIN09 */
					  pio_wr( DATA, 9, 0);
					 
					printf("Turning ON DSPJP \n");
				break;
				case OFF:	
					  /* D7 PIN09 */
					  pio_wr( DATA, 9, 1);
					  
					 
					printf("Turning OFF DSPJP\n");
				break;
				default:
				break;
			}
		break;

		case RAPWR:
			switch(mode)
			{
				case ON:
					/* RA power (D0 PIN02): pull down for on */
					pio_wr_msg( "RA", 2, 0);
				break;
				case OFF:
					pio_wr_msg( "RA", 2, 1);
				break;	
				default:
				break;
			}
		break;

		case DECPWR:
			switch(mode)
			{
				case ON:
					/* DEC power (D1 PIN03): pull down for on */
					pio_wr_msg( "DEC", 3, 0);
				break;
				case OFF:
					pio_wr_msg( "DEC", 3, 1);
				break;	
				default:
				break;
			}
		break;

		case ROTPWR:
			switch(mode)
			{
				case ON:
					/* ROT power (D2 PIN04): pull down for on */
					pio_wr_msg( "ROT", 4, 0);
				break;
				case OFF:
					pio_wr_msg( "ROT", 4, 1);
				break;	
				default:
				break;			
			}
		break;

		case TRANPWR:
			switch(mode)
			{
				case ON:
					/* TRANS power (D3 PIN05): pull down for on */
					pio_wr_msg( "TRANS", 5, 0);
				break;
				case OFF:
					pio_wr_msg( "TRANS", 5, 1);
				break;	
				default:
				break;			
						}
		break;

		case CCDPWR:
			switch(mode)
			{
				case ON:
					/* CCD power (D4 PIN06): pull down for on */
					pio_wr_msg( "CCD", 6, 0);
				break;
				case OFF:
					pio_wr_msg( "CCD", 6, 1);
				break;	
				default:
				break;			
						}
		break;

		case WHEELPWR:
			switch(mode)
			{
				case ON:
					/* WHEEL power (D5 PIN07): pull down for on */
					pio_wr_msg( "WHEEL", 7, 0);
				break;
				case OFF:
					pio_wr_msg( "WHEEL", 7, 1);
				break;	
				default:
				break;			
			}
		break;

		case DSPPWR:
			switch(mode)
			{
				case ON:
					/* DSP power (D6 PIN08): pull down for on */
					pio_wr_msg( "DSP", 8, 0);
				break;
				case OFF:
					pio_wr_msg( "DSP", 8, 1);
				break;	
				default:
				break;			
						}
		break;

	}/*switch(type)*/
}/*void SetRelaySwitch(unsigned char type, unsigned char mode)*/

void inb_value_parsing(unsigned char *status,int inb_value1)
{
        int i;

/*debug	printf("inb_value1=0x%x\n",inb_value1);*/
        for(i=7;i>-1;i--)
        {
/*debug        	printf("%d/%d=%d\n",inb_value1,(int)pow(2,i),inb_value1/(int)pow(2,i));*/
                if((inb_value1/(int)pow(2,i)) == 0)
                {
                        status[i]='0';
                }
                else
                {
                        status[i]='1';
                        inb_value1=inb_value1-(int)pow(2,i);
                }
        }

}



void pio_wr_msg( char device_name[], int pin, int dat)
{
	pio_wr( DATA, pin, dat);
	if( dat == 0)
	{
		printf("Turning %s Power On\n", device_name);
	}
	else
	{	
		 printf("Turning %s Power Off\n", device_name);
	}
}	
