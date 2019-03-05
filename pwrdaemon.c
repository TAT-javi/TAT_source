/*
	TEN Hardware Powers Control daemon
	----------------------------------
Control the hardware powers through parallel port IO card.
How to compile:
cc pwrdaemon.c -O2
Optimized option is needed when compile for inb/outb functions.

6.Feb 2003

--------------------------------------------------------------------------------------------------
>/sbin/lspci -v 
...
02:04.0 Communication controller: NetMos Technology VScom 021H-EP2 2 port parallel adaptor (rev 01)
        Subsystem: LSI Logic / Symbios Logic: Unknown device 0020
        Flags: medium devsel, IRQ 10
        I/O ports at 9000 [size=8]
        I/O ports at 9400 [size=8]
        I/O ports at 9800 [size=8]
        I/O ports at 9c00 [size=8]
        I/O ports at a000 [size=8]
        I/O ports at a400 [size=16]
...
ppc1
	#define DATA 9000
ppc2
	#define DATA 9800

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include "symblc_const.h"
#include "pwrdaemon.h"
#include "tat_info.h"
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
/* Hsinchu */
//#define DATA		0x9400
/* Tenerife */
//#define DATA		0x9800
/* Uzbekistan */
#define DATA		PWR_DATA	/* @symblc.const.h */
#define STATUS		DATA+1
#define CONTROL		DATA+2



#define R1PWR		20
#define R2PWR		21
#define R3PWR		22
#define R4PWR		23
#define R5PWR		24
#define R6PWR		25
#define L1PWR		26
#define L2PWR		27	
#define L3PWR		28	
#define L4PWR		29
#define L5PWR		30
#define L6PWR		31
#define POWERPANEL	99

#define VOLT_HIGH	11	/* set voltage high*/
#define VOLT_LOW	12	/* set voltage low*/

#define CYCLE_TIME	100000	/* microseconds usleep */

void SetRelaySwitch	(unsigned char type, unsigned char mode);
void pio_wr( int pin_num, int dat );
int pio_rd( int pin_num);

st_tat_info *p_tat_info;

int main(int argc, char *argv[])
{
	pid_t pid;
	FILE *CommandFile;
	unsigned char PowerSource, PowerSwitch;

	char CmdSource[12];
	char CmdOnOff[12];
/*	char systemCmd[20];*/
	unsigned char startAsDaemon=0;
	
	time_t currTime;
	char *pcCurrTime;
	FILE *fp;
	unsigned char msg[3];

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
						printf("PWRDaemon start as Daemon now\n");
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
				fprintf( fp, "%s pwrdaemon child pid=%d exit.\n", pcCurrTime, pid);
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
	remove (PWR_CMD_FILENAME);
	printf("PWR Saemon Start.\n");
	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
	
	while(1)
	{
		if( ( CommandFile = fopen(PWR_CMD_FILENAME, "r") ) == NULL )
		{
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
					PowerSwitch=VOLT_HIGH;
				else	PowerSwitch=VOLT_LOW;
/*
				if( strcmp(CmdSource, "R1POWER")== 0 ||
				    strcmp(CmdSource, "R2POWER")== 0 ||
				    strcmp(CmdSource, "L6POWER")== 0)	
				{
				  PowerSwitch = (PowerSwitch==VOLT_LOW) ? VOLT_HIGH:VOLT_LOW;
                                }
*/
				if( strcmp(CmdSource, "R1POWER")  == 0 )	PowerSource= R1PWR;
				else
				if( strcmp(CmdSource, "R2POWER") == 0 )	PowerSource= R2PWR;
				else
				if( strcmp(CmdSource, "R3POWER") == 0 )	PowerSource= R3PWR;
				else
				if( strcmp(CmdSource, "R4POWER")== 0 )	PowerSource= R4PWR;
				else
				if( strcmp(CmdSource, "R5POWER") == 0 )	PowerSource= R5PWR;
				else
				if( strcmp(CmdSource, "R6POWER")== 0 )	PowerSource= R6PWR;
				else
				if( strcmp(CmdSource, "L1POWER") == 0 )	PowerSource= L1PWR;
				else
				if( strcmp(CmdSource, "L2POWER")== 0 )	PowerSource= L2PWR;
				else
				if( strcmp(CmdSource, "L3POWER")== 0 )	PowerSource= L3PWR;
				else
				if( strcmp(CmdSource, "L4POWER")== 0 )	PowerSource= L4PWR;
				else
				if( strcmp(CmdSource, "L5POWER")== 0 )	PowerSource= L5PWR;
				else
				if( strcmp(CmdSource, "L6POWER")== 0 )	PowerSource= L6PWR;
				else
				if( strcmp(CmdSource, "POWERPANEL") == 0 )	PowerSource= POWERPANEL;
				else
				  break;

				SetRelaySwitch(PowerSource, PowerSwitch);
			}
			remove(PWR_CMD_FILENAME);
		}
		/* update ppc info */
                msg[0]=inb(CONTROL);
                msg[1]=inb(DATA);
                msg[2]=inb(STATUS);
                update_pwr_info( &(p_tat_info->pwr_info), msg);
  
	}	/* while(1)*/
	return 0;
}

void SetRelaySwitch(unsigned char type, unsigned char mode)	
{
	unsigned char inb_value;
	unsigned char status[9]={2,2,2,2,2,2,2,2,'\0'};
	char temp_string[40];
	FILE *fp;
	
	switch(type)
	{
		case L1PWR: /* C0' */
			switch(mode)
			{
				case VOLT_HIGH:	
				        pio_wr(1, 0);
					printf("Turning ON L1 Power\n");
				break;
				case VOLT_LOW:	
				        pio_wr(1, 1);
					printf("Turning OFF L1 Power\n");
				break;
				default:
				break;
			}
		break;

		case L2PWR:	/* C1' */
		        
			switch(mode)
			{
				case VOLT_HIGH:	
				        pio_wr( 14, 0);
					printf("Turning ON L2 Power\n");
				break;
				case VOLT_LOW:	/*Turn on Main Power*/
				        pio_wr( 14, 1);
					printf("Turning OFF L2 Power\n");
				break;
				default:
				break;
			}
		break;

		case L3PWR:	/* D0 */
                        
			switch(mode)
			{
				case VOLT_HIGH:	
				        pio_wr( 2, 1);
					printf("Turning ON L3 Power\n");
				break;
				case VOLT_LOW:
				        pio_wr( 2, 0);
					printf("Turning OFF L3 Power\n");
				break;
				default:
				break;
			}
		break;

		case L4PWR:	/* D1 */
		        
			switch(mode)
			{
				case VOLT_HIGH:
				        pio_wr( 3, 1);
					printf("Turning ON L4 Power\n");
				break;
				case VOLT_LOW:
					pio_wr( 3, 0);
					printf("Turning OFF L4 Power\n");
				break;
				default:
				break;
			}
		break;

		case L5PWR:	/* C2  */
		        
			switch(mode)
			{
				case VOLT_HIGH:
					pio_wr( 16, 1);
					printf("Turning ON L5 Power\n");
				break;

				case VOLT_LOW:
					pio_wr( 16, 0);
					printf("Turning OFF L5 Power\n");
				break;
				default:
				break;
			}
		break;

		case L6PWR:	/* D2 */
		        
			switch(mode)
			{
				case VOLT_HIGH:
					pio_wr( 4, 1);
					printf("Turning ON L6 Power\n");
				break;
				case VOLT_LOW:
					pio_wr( 4, 0);
					printf("Turning OFF L6 Power\n");
				break;
				default:
				break;
			}
		break;
		case R1PWR:	/* D7 */
			switch(mode)
			{
				case VOLT_HIGH:
				        pio_wr( 9, 1);
					printf("Turning ON L1 Power\n");
				break;
				case VOLT_LOW:	
					pio_wr( 9, 0);
					printf("Turning OFF L1 Power\n");
				break;
				default:
				break;
			}
		break;

		case R2PWR:	/* D6 */
			switch(mode)
			{
				case VOLT_HIGH:	
					pio_wr( 8, 1);
					printf("Turning ON L2 Power\n");
				break;
				case VOLT_LOW:	
					pio_wr( 8, 0);
					printf("Turning OFF L2 Power\n");
				break;
				default:
				break;
			}
		break;

		case R3PWR:	/* D5 */
			switch(mode)
			{
				case VOLT_HIGH:	
					pio_wr( 7, 1);
					printf("Turning ON L3 Power\n");
				break;
				case VOLT_LOW:	
					pio_wr( 7, 0);
					printf("Turning OFF L3 Power\n");
				break;
				default:
				break;
			}
		break;

		case R4PWR:	/* D4 */
		        
			switch(mode)
			{
				case VOLT_HIGH:
					pio_wr( 6, 1);
					printf("Turning ON L4 Power\n");
				break;
				case VOLT_LOW:	
					pio_wr( 6, 0);
					printf("Turning OFF L4 Power\n");
				break;
				default:
				break;
			}
		break;

		case R5PWR:	/* D3 */
			switch(mode)
			{
				case VOLT_HIGH:	
					pio_wr( 5, 1);
					printf("Turning ON L5 Power\n");
				break;

				case VOLT_LOW:	
					pio_wr( 5, 0);
					printf("Turning OFF L5 Power\n");
				break;
				default:
				break;
			}
		break;

		case R6PWR:	/* C3' */
		        
			switch(mode)
			{
				case VOLT_HIGH:	
					pio_wr( 17, 0);
					printf("Turning ON L6 Power\n");
				break;
				case VOLT_LOW:	
					pio_wr( 17, 1);
					printf("Turning OFF L6 Power\n");
				break;
				default:
				break;
			}
		break;

		case POWERPANEL:
			fp=fopen(PWR_RPY_FILENAME,"w");
			
		/*	inb_value_parsing(status,(int)inb(CONTROL));
		*/	
		/*	printf("CONTROL:%s\n",status);*/
		
			sprintf(temp_string,"Non-UPS 1 POWER:\t%s\n",(pio_rd(1)==0?"ON":"OFF"));
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"Non-UPS 2 POWER:\t%s\n",pio_rd(14)==0?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"Non-UPS 3 POWER:\t%s\n",pio_rd(2)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"Non-UPS 4 POWER:\t%s\n",pio_rd(3)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"Non-UPS 5 POWER:\t%s\n",pio_rd(16)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"Non-UPS 6 POWER:\t%s\n",pio_rd(4)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"UPS 1 POWER:\t%s\n",pio_rd(9)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"UPS 2 POWER:\t%s\n",pio_rd(8)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"UPS 3 POWER:\t%s\n",pio_rd(7)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"UPS 4 POWER:\t%s\n",pio_rd(6)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"UPS 5 POWER:\t%s\n",pio_rd(5)==1?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			sprintf(temp_string,"UPS 6 POWER:\t%s\n",pio_rd(17)==0?"ON":"OFF");
			printf("%s",temp_string); fprintf(fp,"%s",temp_string);
			
			fclose(fp);
			sprintf(temp_string,"/bin/chmod 666 %s",PWR_RPY_FILENAME);
			system(temp_string);
		break;
	}/*switch(type)*/
}/*void SetRelaySwitch(unsigned char type, unsigned char mode)*/


void pio_wr( int pin_num, int dat )
{
  switch(pin_num)
  {
    case 1:	/* C0' */
      if (dat!=0) outb( inb(CONTROL) | PIN01REG, CONTROL);	/* wr 1 */
      else outb( inb(CONTROL) & ~PIN01REG ,CONTROL); /* wr 0 */
    break;
    case 2:	/* D0 */
      if (dat!=0) outb( inb(DATA) | PIN02REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN02REG, DATA);	/* wr 0 */
    break;
    case 3:	/* D1 */
      if (dat!=0) outb( inb(DATA) | PIN03REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN03REG, DATA);	/* wr 0 */
    break;
    case 4:	/* D2 */
      if (dat!=0) outb( inb(DATA) | PIN04REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN04REG, DATA);	/* wr 0 */
    break;
    case 5:	/* D3 */
      if (dat!=0) outb( inb(DATA) | PIN05REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN05REG, DATA);	/* wr 0 */
    break;
    case 6:	/* D4 */
      if (dat!=0) outb( inb(DATA) | PIN06REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN06REG, DATA);	/* wr 0 */
    break;
    case 7:	/* D5 */
      if (dat!=0) outb( inb(DATA) | PIN07REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN07REG, DATA);	/* wr 0 */
    break;
    case 8:	/* D6 */
      if (dat!=0) outb( inb(DATA) | PIN08REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN08REG, DATA);	/* wr 0 */
    break;
    case 9:	/* D7 */
      if (dat!=0) outb( inb(DATA) | PIN09REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN09REG, DATA);	/* wr 0 */
    break;
    case 14:	/* C1' */
      if (dat!=0) outb( inb(CONTROL) | PIN14REG, CONTROL);	/* wr 1 */
      else outb( inb(CONTROL) & ~PIN14REG ,CONTROL);	/* wr 0 */
    break;

    case 16:	/* C2 */
      if (dat!=0) outb( inb(CONTROL) | PIN16REG ,CONTROL);	/* wr 1 */
      else outb( inb(CONTROL) & ~PIN16REG, CONTROL);	/* wr 0 */
    break;

    case 17:	/* C3' */
      if (dat!=0) outb( inb(CONTROL) | PIN17REG, CONTROL);	/* wr 1 */
      else outb( inb(CONTROL) & ~PIN17REG ,CONTROL);	/* wr 0 */
    break;

    default:
    break;
    
  }


}
int pio_rd( int pin_num)
{
  switch( pin_num )
  {
    case 1:	/* C0' */
      return (inb( CONTROL ) & PIN01REG)==0?0:1;
    break;
    case 2:	/* D0 */
      return (inb( DATA ) & PIN02REG)==0?0:1;
    break;
    case 3:	/* D1 */
      return (inb( DATA ) & PIN03REG)==0?0:1;
    break;
    case 4:	/* D2 */
      return (inb( DATA ) & PIN04REG)==0?0:1;
    break;
    case 5:	/* D3 */
      return (inb( DATA ) & PIN05REG)==0?0:1;
    break;
    case 6:	/* D4 */	
      return (inb( DATA ) & PIN06REG)==0?0:1;
    break;
    case 7:	/* D5 */
      return (inb( DATA ) & PIN07REG)==0?0:1;
    break;
    case 8:	/* D6 */
      return (inb( DATA ) & PIN08REG)==0?0:1;
    break;
    case 9:	/* D7 */
      return (inb( DATA ) & PIN09REG)==0?0:1;
    break;
    case 10:	/* S6 */
      return (inb( DATA ) & PIN10REG)==0?0:1;
    break;
    case 11:	/* S7' */
      return (inb( STATUS ) & PIN11REG)==0?0:1;
    break;
    case 12:	/* S5 */
      return (inb( STATUS ) & PIN12REG)==0?0:1;
    break;
    case 13:	/* S4 */
      return (inb( STATUS ) & PIN13REG)==0?0:1;
    break;
    case 14:	/* C1' */
      return (inb( CONTROL ) & PIN14REG)==0?0:1;
    break;
    case 15:	/* S3 */
      return (inb( STATUS ) & PIN15REG)==0?0:1;
    break;
    case 16:	/* C2 */
      return (inb( CONTROL ) & PIN16REG)==0?0:1;
    break;
    case 17:	/* C3' */
      return (inb( CONTROL ) & PIN17REG)==0?0:1;
    break;
    
  
  }  
  return -1;


}
