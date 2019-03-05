/*
	Version 5.1
 	(2008/05/06) 1. Add the commands to operate with the wheel
		     2. Start using 'temp' to check if the dspcommand was read
	
	A serial daemon which frequently checks for the command file
	and then sends the command to DSP
	27.Oct 2003	
	

Change log:

(2006/05/06) 1. Modify read part.
(2006/04/18) 1. Avoid reply data in a tangle.
(2004/10/04) 1. Add: RA and DEC pulse number limitation comamnd
	     2. ADD: Hydraulic Press command
(2004/04/24) 1. delete dsp command file before serial thread create.
(2004/02/09) 1. Add: output request data to RADEC_REQUEST_FILENAME

////////////////////////////////////////////////////////////
Origin Change log:
1. Added Enclosure command, the Motor Type command is now: RA, DEC, ROT, TRAN,  PD or ENC;
2. CmdMode is "RESET,  REQUEST or FREQUENCY" for RA and DEC.
		"RUN, STOP, or REQUEST" for PD
		"NONE" for ROT and TRAN.
		"CLOSE, OPEN, OFF" for ENC (Enclosure)

Previous version: (12.Feb 2003)
1. DSPCommand is 34 Characters, eg. ATB12345ZTE12345ZRF+1234567890123E
2. MotorType is either RA, DEC, ROT, TRAN or PD;				<--CHANGED!!!
3. CmdMode is "RESET, REQUEST or FREQUENCY" for RA and DEC.	<--CHANGED!!!
	      "RUN, STOP, or REQUEST" for PD
	      "NONE" for ROT and TRAN.
4. Dir is either "POS" or "NEG"
5. CmdStart, CmdEnd is the Starting/Ending time(seconds)for the specify command, maximum of 5 digits.
6. RegCmd1 is the register values to be put in DSP timer register counter for RA/DEC, Step Count for ROT/TRAN,
   and Minima value for PD in RA direction.The maximum length should be less than 10 digits, which is the maxima
   of "long" data type.
7. PdSignRa, PdSignDec, RegCmd2, RegCmd3 is available for PD command only, it should set to 0 for RA/DEC/ROT/TRAN command.
8. PdSignRa, PdSignDec either "POS" or "NEG", it is use to toggle either
   ( (A+B)-(C+D) or -(A+B)+(C+D)) for PD in RA and DEC Direction.
9. RegCmd2 and RegCmd3 set the minima threshold value for Dec Direction and Total Pd Voltage which is 4 digits length.
10. request output file name is vlaidresultfropmdsp.log

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <termios.h>
#include <pthread.h>
#include "symblc_const.h"
#include "tat_info.h"
#define BAUDRATE	B19200//B19200
#define MODEMDEVICE	"/dev/ttyS0"
#define _POSIX_SOURCE	1
#define CYCLE_TIME 100000	/* micro-second */


void *serial_read(void *);
int fd;
st_tat_info *p_tat_info;


int main()
{
		
	char DSPCommand[35], DSPCmdReq[35], MotorType[5], CmdMode[10], Dir[4], CmdStart[6], CmdEnd[6];
	char RegCmd1[11], PdSignRa[4], RegCmd2[5], PdSignDec[4], RegCmd3[5];
	int RegCnt, StartTime, EndTime;
	char TBegin[10], TEnd[9], RegValue[15];
	char PdMinRa[5], PdMinDec[5], PdMinTotal[6];

	char *endptr;
	int res, temp = 0, i=0;
	struct termios oldtio,newtio;
	pthread_t serial_read_thread;
	FILE *fin;
	pid_t pid;
	
	/* for log */
	time_t currTime;
	char *pcCurrTime;
	FILE *fp;
	char time_string[18]; /* 12/31/06 01:23:45 */
	
	int ra_motor,dec_motor,rot_motor,tran_motor,pd_motor,enc_motor;
	int	hyp_motor,wheel_motor,msg_motor;

	while(1)
	{
		pid=fork();
		if(pid==-1)
		{
			/* error  */
			perror("-1");
		}
		else if(pid!= 0)
		{
			/* parent  */
			printf("wait child pid =%d\n",pid);
			waitpid(pid,(int *)0,0);
			printf("child pid= %d exit\n", pid);
			time(&currTime);
			pcCurrTime= ctime( &currTime);
			fp= fopen(DAEMON_LOG_FILENAME, "a+");
			fprintf( fp, "%s dspdaemon child pid=%d exit.\n", pcCurrTime, pid);
			fclose( fp);
		}
		else
		{
			/* child  */
			break;
		}
	} /* end while  */



	fd=open( MODEMDEVICE, O_RDWR | O_NOCTTY);
	if(fd<0) {perror(MODEMDEVICE);exit(-1);}

	tcgetattr(fd,&oldtio); /* save current serial port settings */
	bzero (&newtio, sizeof(newtio)); /* clear struct for new port settings */

	/*
	//    Set baud rate to 19200
	//    CRTSCTS : output hardware flow control (only used if the cable has
	//    all necessary lines. See sect. 7 of Serial-HOWTO)
	//    CS8     : 8n1 (8bit,no parity,1 stopbit)
	//    CLOCAL  : local connection, no modem contol
	//    CREAD   : enable receiving characters
	*/
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	
	/*
	//    IGNPAR  : ignore bytes with parity errors
	//    ICRNL   : map CR to NL (otherwise a CR input on the other computer
	//              will not terminate input)
	//    otherwise make device raw (no other input processing)
	*/
	newtio.c_iflag = IGNPAR | ICRNL;

	/*
	//   Raw output.
	*/
	newtio.c_oflag = 0;
	/*
	//    ICANON  : enable canonical input
	//    disable all echo functionality, and don't send signals to calling program
	*/
	newtio.c_lflag = ICANON;
  
	newtio.c_cc[VINTR]		= 0;	/* Ctrl-c */
	newtio.c_cc[VQUIT]		= 0;	/* Ctrl-'\' */
	newtio.c_cc[VERASE]		= 0;	/* del */
	newtio.c_cc[VKILL]		= 0;	/* @ */
	newtio.c_cc[VEOF]		= 4;	/* Ctrl-d */
	newtio.c_cc[VTIME]		= 0;	/* inter-character timer unused */
	newtio.c_cc[VMIN]		= 1;	/* blocking read until 1 character arrives */
	newtio.c_cc[VSWTC]		= 0;	/* '\0' */
	newtio.c_cc[VSTART]   		= 0;	/* Ctrl-q */
	newtio.c_cc[VSTOP]		= 0;	/* Ctrl-s */
	newtio.c_cc[VSUSP]		= 0;	/* Ctrl-z */
	newtio.c_cc[VEOL]		= 0;	/* '\0' */
	newtio.c_cc[VREPRINT]		= 0;	/* Ctrl-r */
	newtio.c_cc[VDISCARD]		= 0;	/* Ctrl-u */
	newtio.c_cc[VWERASE]		= 0;	/* Ctrl-w */
	newtio.c_cc[VLNEXT]		= 0;	/* Ctrl-v */
	newtio.c_cc[VEOL2]		= 0;	/* '\0' */

	/*    now clean the modem line and activate the settings for the port*/
	tcflush(fd,TCIFLUSH);
	tcsetattr(fd,TCSANOW, &newtio);

	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);
	
	init_dsp_info( &(p_tat_info->dsp_info) );	
	
	umask (0);  
  	remove (DSP_CMD_FILENAME);
	remove (DSP_RPY_FILENAME);

	printf("Creates Serial_read thread.\n");
	res=pthread_create(&serial_read_thread,NULL,serial_read,NULL);
	if(res!=0)
	{
		perror("Serial_read_thread Creation Failed.\n");
		exit(EXIT_FAILURE);
	}
	else {printf("Serial_read_thread Created.\n");}


	while(1)
	{

		/*Check the command file */
		if( ( fin=fopen(DSP_CMD_FILENAME, "r") ) == NULL )
		{
			usleep(CYCLE_TIME); 
		}
		else	/*Parses commands */
		{
			while(1)
			{
				if (fscanf(fin, "%s %s %s %s %s %s %s %s %s %s",
					MotorType, Dir, CmdMode, CmdStart, CmdEnd, 
					RegCmd1, PdSignRa, RegCmd2, PdSignDec, RegCmd3) == EOF)
				{
					if(temp!=i)break;
					else{rewind(fin);continue;}
				}
				i++;
				ra_motor=!strcmp(MotorType, "RA");
				dec_motor=!strcmp(MotorType, "DEC");
				rot_motor=!strcmp(MotorType, "ROT");
				tran_motor=!strcmp(MotorType, "TRAN");
				pd_motor=!strcmp(MotorType, "PD");
				enc_motor=!strcmp(MotorType, "ENC");
				hyp_motor=!strcmp(MotorType, "HYP");
				wheel_motor=!strcmp(MotorType, "WHEEL");
				msg_motor=!strcmp(MotorType, "MSG");
				
				printf("%s %s %s %s %s %s %s %s %s %s\n",MotorType, Dir, CmdMode,
					 CmdStart, CmdEnd, RegCmd1, PdSignRa, RegCmd2, PdSignDec, RegCmd3);

				if( ra_motor || dec_motor || rot_motor || tran_motor ||
				    pd_motor || enc_motor || hyp_motor || wheel_motor )
				{
					
					strcpy(DSPCommand,"");/*First clear the DSPCommand*/
	
					StartTime	= strtol(CmdStart, &endptr, 10);
					EndTime	= strtol(CmdEnd, &endptr, 10);
		
					sprintf(TBegin, "ATB%05dZ", StartTime);
					sprintf(TEnd, "TE%05dZ", EndTime);
					strcat(DSPCommand, TBegin);
					strcat(DSPCommand, TEnd);

					if( ra_motor || dec_motor || rot_motor || tran_motor || wheel_motor)
					{
						if( ra_motor || dec_motor )
						{
							/* motor type */				
							if( ra_motor)	strcat(DSPCommand,"R");/*Ra*/
							else					strcat(DSPCommand,"D");/*Dec*/
							/* function  */
							if( strcmp(CmdMode, "RESET"  ) ==0 )	strcat(DSPCommand,"T");/*Reset*/
							else
							if( strcmp(CmdMode, "LIMIT") ==0 )	strcat(DSPCommand,"L");
							else					strcat(DSPCommand,"F");/*Frequency*/
							/* direction */
							if(strcmp(Dir, "POS")==0)		strcat(DSPCommand,"+");/*Positive*/
							else					strcat(DSPCommand,"-");/*Negative*/
						}
						else if( rot_motor )
						{
							strcat(DSPCommand,"M1+");
						}
						else if( tran_motor )
						{
							strcat(DSPCommand,"M2+");
						}
						else if( wheel_motor )
						{
							strcat(DSPCommand,"M3+");
						}

						RegCnt	 = strtol(RegCmd1, &endptr, 10);
						sprintf(RegValue, "%013dE", RegCnt);
						strcat(DSPCommand, RegValue);
					}
					else if( pd_motor)
					{
						/* function */
						if( strcmp(CmdMode, "RUN"  )  ==0 )	strcat(DSPCommand,"PR");/*Run PD Mode*/
						else if( strcmp(CmdMode, "STOP")   ==0 )	strcat(DSPCommand,"PT");/*Stop PD Mode*/

						/* sign of min. differential value of RA */
						if( strcmp(PdSignRa, "POS") == 0)		strcat(DSPCommand,"1");
						else							strcat(DSPCommand,"0");
						/* min. differential value of RA */
						RegCnt	 = strtol(RegCmd1, &endptr, 10);
						sprintf(PdMinRa, "%04d", RegCnt);	strcat(DSPCommand, PdMinRa);
						/* sign of min. differential value of DEC */
						if( strcmp(PdSignDec, "POS") == 0)	strcat(DSPCommand,"1");
						else					strcat(DSPCommand,"0");
						/* min. differential value of DEC */
						RegCnt	 = strtol(RegCmd2, &endptr, 10);
						sprintf(PdMinDec, "%04d", RegCnt);	strcat(DSPCommand, PdMinDec);
						/* threshold of total value */
						RegCnt	 = strtol(RegCmd3, &endptr, 10);
						sprintf(PdMinTotal,"%04dE",RegCnt);	strcat(DSPCommand, PdMinTotal);
					}
					else if( enc_motor)
					{
						if( strcmp(CmdMode, "OPEN"  )  ==0 )	strcat(DSPCommand,"C10");/*Open  Enclosure(M0)*/
						else if( strcmp(CmdMode, "CLOSE" ) ==0) strcat(DSPCommand,"C01");/*Close Enclosure(M1)*/
						else if( strcmp(CmdMode, "RESET" ) ==0) strcat(DSPCommand,"CTT");/*RESET Enclosure(M2)*/
						else 					strcat(DSPCommand,"C00");/*OFF line (M0 and M1)*/

						strcat(DSPCommand,"0000000000000E");
					}
					else if( hyp_motor)
					{
						if( strcmp(CmdMode, "DOWN"  )  ==0 )	strcat(DSPCommand,"H10");/*Open  Enclosure(M0)*/
						else if( strcmp(CmdMode, "UP" ) ==0) strcat(DSPCommand,"H01");/*Close Enclosure(M1)*/
						else if( strcmp(CmdMode, "RESET" ) ==0) strcat(DSPCommand,"HTT");/*RESET Enclosure(M1)*/
						else 					strcat(DSPCommand,"H00");/*OFF line (M0 and M1)*/
						strcat(DSPCommand,"0000000000000E");
						
					}
					else if(  msg_motor)
					{
						strcat(DSPCommand,"U110000000000000E");
					}
					/* clear MotorType for next command */
					strcpy(MotorType, "");
					
					/* DSPCommand */
					/* printf("DSPCommand=%s\n", DSPCommand); */
					write(fd, DSPCommand,34);
					printf("i=%d, DSPCommand --:%s:--\n", i, DSPCommand);
					
					/* log dsp command */
					fp=fopen(DSP_LOG_FILENAME,"a+");
					if(  fp != NULL)
					{
						time(&currTime);
						strftime( time_string, 18, "%D %T", localtime(&currTime));
						fprintf( fp, "%s %s\n", time_string, DSPCommand);
						fflush(fp);
						fclose (fp);
					}
				}/*if( strcmp(MotorType, "RA",  "DEC", "ROT", "TRAN", "PD", "ENC")==0 )*/

			}/*while(!feof(fin))*/
			fclose(fin);
			printf("Removind file : %s\n",DSP_CMD_FILENAME);
			remove (DSP_CMD_FILENAME);
			temp=i;
		}/*else*/
	}/*while(1)*/


	printf("serial_comm_function end\n");
	pthread_exit("serial_comm_thread END");
	

}/*end of serial_comm_thread*/

/**********************************************************************/

void *serial_read(void *arg)
{
	int res,i;
	char read_buf[255];
	char request_status;
	unsigned int csum, csumacc;
	unsigned short dsp_cmd_length; 
	/* unsigned short dsp_csum_length; not used*/
	int msg[29];
	int csum_error=0;
	char shell_command[BUFSIZ];
	while(1)
	{
		res=read(fd,read_buf,255);
		if(res > 5 )
		{
			read_buf[res]=read_buf[res-1]=0;
			printf("readbuf = |%s|; ",read_buf );
			if( strstr(read_buf, "Near") != NULL	)
			{
				if( strstr(read_buf, "Ra") != NULL )
				{
					sprintf(shell_command,"echo \"%s\" > /tmp/ra.ref", read_buf);
					system(shell_command);
				}
				else
				{
					sprintf(shell_command,"echo \"%s\" > /tmp/dec.ref", read_buf);
					system(shell_command);
				}
			}

			printf("csum error = %d\n",csum_error );
			/*Check whether the returned data from DSP is valid, e.g. terminated with char '*'*/
			for(i=0;  i < sizeof(read_buf) && request_status !=1 ; i++)
			{
				if( read_buf[i]  =='*' )
					request_status=1;
				else
					request_status=0;  
			}
		
			if( request_status )
			{
				request_status = 0;
				/*
				printf("request status=1: readbuf = |%s|\n",read_buf );
				printf("DSP sentence valid: performing checksum\n");
				*/
				csum=0;	/* Checksum from message */
				for(i=0,dsp_cmd_length=0; (i < sizeof(read_buf)) && (read_buf[i] != 'C') ; i++)
					dsp_cmd_length++;
		
				for(i=0, csumacc=0; i<dsp_cmd_length ; i++)
					csumacc^=read_buf[i];       /* Compute checksum*/

				sscanf(&read_buf[i+1],"%2x",&csum); /* Read the given checksum */

				if( csum == csumacc)/*A Valid result returned.*/
				{
					sscanf(read_buf,"%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\
							,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\
							,%x,%x,%x,%x,%x,%x,%x,%x,%x",
						&msg[0],&msg[1],&msg[2],&msg[3],&msg[4],
						&msg[5],&msg[6],&msg[7],&msg[8],&msg[9],
						&msg[10],&msg[11],&msg[12],&msg[13],&msg[14],
						&msg[15],&msg[16],&msg[17],&msg[18],&msg[19],
						&msg[20],&msg[21],&msg[22],&msg[23],&msg[24],
						&msg[25],&msg[26],&msg[27],&msg[28]);
					update_dsp_info ( &(p_tat_info->dsp_info), msg);
					/*printf("%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\
							,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\
							,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",
						msg[0],msg[1],msg[2],msg[3],msg[4],
						msg[5],msg[6],msg[7],msg[8],msg[9],
						msg[10],msg[11],msg[12],msg[13],msg[14],
						msg[15],msg[16],msg[17],msg[18],msg[19],
						msg[20],msg[21],msg[22],msg[23],msg[24],
						msg[25],msg[26],msg[27],msg[28]);*/
				}/*if(csum== csumacc)*/
				else{ 
					csum_error++;
					printf("Checksum failed!\n");
					
				}
				
			}/*if(request_status)*/
			else
			{
				printf("request_status!=1\n");
			}
		}/*if(res!=0)*/
		strcpy(read_buf,"");
		usleep(900);
	} /*while(1);  */
}  /* end of void serial_read() */


