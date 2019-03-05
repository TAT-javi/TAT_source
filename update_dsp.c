/*
  A remote serial port daemon which uploads user DSP program (*.hex)into DSP FLASH.
  9.Feb 2003

  Modifications: (Friday, November 13 2009)
  1- Fix 2 bugs which make the program to fail always
  2- Make the program stand alone by including the TAT  programs

  How to compile this program:
  cc update_dsp.c -lpthread
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <pthread.h>

#define BAUDRATE	B19200
#define MODEMDEVICE	"/dev/ttyS0"
#define _POSIX_SOURCE	1

#define PPC_CMD_FILENAME	"/home/observer/ppc_command.cmd"
#define CTL_CMD_FILENAME	"/home/observer/ctl_command.cmd"


// Serial port routines, using multithread
void *serial_comm(void *arg);//sending command throught serial port, create serial_read thread
void *serial_read(void *arg);
int send_cmd2ctl(char *daemonCmd);
int send_cmd2ppc(char *daemonCmd);


char read_buf[255]="";
char carriage_return[2];
char char_buf[2];
int buf, buf_tmp;
int cr=13;
int fd;
char fname1[32];
char fname2[32];
char cmdString[100];
FILE *fn1, *fn2;
FILE *logFile;

int main(int argc, char* argv[])
{
	pthread_t serial_comm_thread;
	int	res,i;
	void	*thread_result;
	if(argc!=2)
	{
		printf("*******************************\n");
		printf("* Usage: update_hex (hexfile) *\n");
		printf("*******************************\n");
		return 1;
	}
	sprintf(fname1, "L_29f400.hex");
	sprintf(fname2, "%s", argv[1]);

	// create process for serial port
	res=pthread_create(&serial_comm_thread,NULL,serial_comm, NULL);

	if(res!=0)
	{
		perror("Serial_comm_thread creation failed");
		return 1;
	}
	else  printf("Creating Serial_comm_thread\n");

	//Wait for the results
	pthread_join (serial_comm_thread, NULL);


	return 0;

}

void *serial_comm(void *arg)
{
	int res,i,status,tmp;
	struct termios oldtio,newtio;
	pthread_t serial_read_thread;

	fd=open( MODEMDEVICE, O_RDWR | O_NOCTTY);
	if(fd<0) {perror(MODEMDEVICE);exit(-1);}

	tcgetattr(fd,&oldtio); // save current serial port settings
	bzero(&newtio, sizeof(newtio)); // clear struct for new port settings

	//    Set baud rate to 19200
	//    CRTSCTS : output hardware flow control (only used if the cable has
	//    all necessary lines. See sect. 7 of Serial-HOWTO)
	//    CS8     : 8n1 (8bit,no parity,1 stopbit)
	//    CLOCAL  : local connection, no modem contol
	//    CREAD   : enable receiving characters
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;

	//    IGNPAR  : ignore bytes with parity errors
	//    ICRNL   : map CR to NL (otherwise a CR input on the other computer
	//              will not terminate input)
	//    otherwise make device raw (no other input processing)
	newtio.c_iflag = IGNPAR | ICRNL;

	//   Raw output.
	newtio.c_oflag = 0;

	//    ICANON  : enable canonical input
	//    disable all echo functionality, and don't send signals to calling program
	newtio.c_lflag = ICANON;

	newtio.c_cc[VINTR]    = 0;     // Ctrl-c
	newtio.c_cc[VQUIT]    = 0;     // Ctrl-'\'
	newtio.c_cc[VERASE]   = 0;     // del
	newtio.c_cc[VKILL]    = 0;     // @
	newtio.c_cc[VEOF]     = 4;     // Ctrl-d
	newtio.c_cc[VTIME]    = 0;     // inter-character timer unused
	newtio.c_cc[VMIN]     = 1;     // blocking read until 1 character arrives
	newtio.c_cc[VSWTC]    = 0;     // '\0'
	newtio.c_cc[VSTART]   = 0;     // Ctrl-q
	newtio.c_cc[VSTOP]    = 0;     // Ctrl-s
	newtio.c_cc[VSUSP]    = 0;     // Ctrl-z
	newtio.c_cc[VEOL]     = 0;     // '\0'
	newtio.c_cc[VREPRINT] = 0;     // Ctrl-r
	newtio.c_cc[VDISCARD] = 0;     // Ctrl-u
	newtio.c_cc[VWERASE]  = 0;     // Ctrl-w
	newtio.c_cc[VLNEXT]   = 0;     // Ctrl-v
	newtio.c_cc[VEOL2]    = 0;     // '\0'

	//    now clean the modem line and activate the settings for the port
	tcflush(fd,TCIFLUSH);
	tcsetattr(fd,TCSANOW, &newtio);

	printf("Creates Serial_read thread.\n");
	res=pthread_create(&serial_read_thread,NULL,serial_read,NULL);
	if(res!=0)
	{
		perror("Serial_read_thread Creation Failed.\n");
		exit(EXIT_FAILURE);
	}
	else	{printf("Serial_read_thread Created.\n");}


	//Following are the procedures to download hex file to DSP
	/////////////////////////////////////////////////////////////////////////////////
	//Step1: Uninstall the DSP J2.38&40 Debug Jumper by shutting off DSP Jumper Power
	/////////////////////////////////////////////////////////////////////////////////
	//Step1.1: send command file to Power Control daemon
	printf("turn off dsp power and jumper(4 sec)\n");
	sprintf(cmdString, "DSPPOWER OFF\nDSPJUMPER OFF\n");
	tmp = send_cmd2ppc(cmdString);
	if(tmp)
	{
		printf("Error sending command to ppcdaemon");
		exit(0);
	}
// 	Waits for Power Control daemon to do its job.
	sleep(4);
	printf("Turn  DSP power on (6 sec)\n");
	sprintf(cmdString, "VDCPOWER ON\nDSPPOWER ON\n");
	tmp =send_cmd2ppc(cmdString);
	if(tmp)
	{
		printf("Error sending command to ppcdaemon");
		exit(0);
	}
	sleep(6);
	//Deactivate the ppcdaemon
	printf("Neutralize the ppcdaemon\n");
	sprintf(cmdString, "PPCDAEMON OFF\n");
	tmp=send_cmd2ctl(cmdString);
	if(tmp)
	{
		printf("Error sending command to ctldaemon");
		exit(0);
	}


	sprintf(carriage_return, "%c", cr);
	printf("Write to serial port.\n");
	write(fd, "D", 1);
	write(fd, carriage_return, 1);
	sleep(2);
	printf("Ready to send file to DSP\n");
	if( (fn1= fopen(fname1, "r")) == NULL )
	{
		printf("File %s could not be opened.\n",fname1);
		exit(0);
	}
	else
	{
		buf=fgetc(fn1);
		do
		{
			buf_tmp=buf;
			if( (buf=fgetc(fn1) ) != EOF )
			{
				sprintf(char_buf, "%c", buf_tmp);
				write(fd, char_buf, 1);
			}
		}while( buf !=EOF);
	}
	fclose(fn1);

	printf("Sleeping for 2 sec b4 sending G04000\n");
	sleep(4);

	write(fd, carriage_return, 1);
	sleep(2);
	printf("Sending G04000 command\n");
	write(fd,"G04000", 7);

	write(fd, carriage_return, 1);

	printf("Sleeping for 15 sec\n");
	sleep(15);
	printf("Sleeping done\n");
	printf("Ready to send user hex file to DSP\n");


	if( (fn2= fopen(fname2, "r")) == NULL )
	{
		printf("File %s could not be opened.\n",fname2);
		exit(0);
	}
	else
	{
		buf=fgetc(fn2);
		do
		{
			buf_tmp=buf;
			if( (buf=fgetc(fn2) ) != EOF )
			{
				sprintf(char_buf, "%c", buf_tmp);
				write(fd, char_buf, 1);
			}
		}while( buf !=EOF);
	}
	fclose(fn2);
	printf("Sending User hex file done!\n");

	printf("Sleeping for 8 sec b4 sending G80000\n");
	sleep(8);
	printf("Sending G80000\n");
	write(fd, "G80000", 7);
	write(fd, carriage_return, 1);

	//Allow the user to read the first lines
	//to check if it really worked
	i=0;
	while (i++ <=5)
		sleep(1);

//	Cancel thread for reading
	res = pthread_cancel(serial_read_thread);

	//Reactivate the ppcdaemon
	printf("Reactivating the ppcdaemon\n");
	sprintf(cmdString, "PPCDAEMON ON\n");
	tmp=send_cmd2ctl(cmdString);
	if(tmp)
	{
		printf("Error sending command to ctldaemon");
		exit(0);
	}

	sleep(4);
	printf("Set the normal status of dsp and turning everything OFF\n");
	sprintf(cmdString,"%s","MAINPOWER OFF\nRAPOWER OFF\nDECPOWER OFF\nROTPOWER OFF\n\
		TRANPOWER OFF\nWHEELPOWER OFF\nDSPPOWER OFF\nCCDPOWER OFF\nVDCPOWER OFF\nDSPJUMPER OFF\n");
	tmp =send_cmd2ppc(cmdString);
	if(tmp)
	{
		printf("Error sending command to ppcdaemon");
		exit(0);
	}
}//void *serial_comm(void *arg)


/**************************************************************************/

void *serial_read(void *arg)
{
	int res,i,j;
	//  global variable: char read_buf[255]

	printf("This is serial read thread.\n");

	while(1)
	{
		res=read(fd,read_buf,255);
		if(res!=0)
		{
			read_buf[res]=read_buf[res-1]=0;
			printf("From DSP: readbuf = -%s-\n",read_buf );
			//Check if the DSP return the menu correctly

		        strcpy(read_buf,"");
		}//if(res!=0)
		usleep(100);
	}  //while(1);
}  //end of void serial_read()


int send_cmd2ctl(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=CTL_CMD_FILENAME;


	for(i=0;i<10;i++)
	{
			if( ( fp=fopen(fpname, "w+") ) == NULL)
			{
					if( i > 8) return 1;
					printf("Open %s failed\n.Try again after 10 sec\n", fpname);
					sleep(10);
			}
			else break;
	}
	printf("%s",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 0;
}


int send_cmd2ppc(char *daemonCmd)
{
	int i;
	FILE *fp;
	char fpname[]=PPC_CMD_FILENAME;

	for(i=0;i<10;i++)
	{
    	if( (fp=fopen(fpname,"w+")) == NULL )
        {
        	if( i>8 ) return 1;
            printf("Open %s failed\n.Try again after 10 sec\n", fpname);
            sleep(10);
		}
        else break;
	}
	printf("send_cmd2ppc=>\n%s",daemonCmd);
	fprintf(fp,"%s",daemonCmd);
	fclose(fp);
	return 0;
} /*send_cmd2ppc*/


