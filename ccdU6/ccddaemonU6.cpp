#define DEBUG	1
/*
 AP8PCam_v1c.cpp:	by Goh Chun Hsian (7,March 2003)
 Source Code Provided by Apogee Instruments Inc. and Random Factory.
  
 These are the minima routines needed to control the Ap8p Ccd Camera, 
 tested on Redhat linux.

 The values of all the properties of the Ccd camera are set in InitDefaults(). 

 Compile it with the following options:
 g++ AP8PCam-v2.cpp -lm -lcfitsio -I/opt/cfitsio -DLINUX -DWITHPPI\
 -o daemon2
(It needs a c version of fits io library to write to fits image)

 This will generate a executable file called "daemon", this daemon will wakeup
 every 1us and looks if there is a command file named "command.dat" which 
 defined what the user wants to do with the Ccd Camera.

singway change log:
2003.12.10	1. query temperature function will write value to a file (ccd_temperature.log)
2003.12.01	AP8PCam_v2.cpp: 
		1. add query function to query internal value. e.g temperture etc...



What's new:

1. Binning function added.(7,March 2003)

Changes log:
1. Fixed some bugs which cause the program to terminate when read in empty command function/filname.
									(27,Feb 2003)
2. The routine write directly and correctly to an unsigned short image. (17,Sep 2002)
*/

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "fitsio.h"
#include <math.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include "tcl.h"
#include "ApnCamera.h"
#include "ccd.h"

#include "../common_func.h"
#include "../symblc_const.h"
#include "../tat_info.h"
#include "../ppcdaemon.h"


#define CYCLE_TIME	100000	/*microsecond usleep*/
#define CYCLE_COUNT	10	/* ccd info update time = CYCLE_TIME*CYCLE_COUNT */
using namespace std;

int parse_options (int argc, char **argv);
int saveimage(unsigned short *src_buffer, char *filename, short nx, short ny);
int dobiassubtract(unsigned short *src,unsigned short *dest, int nx, int ny);

/* Declare the camera object. All camera functions and parameters are
 * accessed using the methods and instance variables of this object
 *
 * Their declarations can be found in ApnCamera.h
 */

CApnCamera *alta;
/* Declare globals to store the input command line options */

char imagename[256];
double texposure=1.0;
int  shutter=1;
double curr_point=99.0;
int camnum=1;
//Global Variable, to be used in writing keyword to fit file
char	date_str[12], time_str[12],filter_type[2];


/* Bias definitions used in the libccd functions */

extern int bias_start, bias_end, bcols;
st_tat_info *p_tat_info;

/* Functions from libccd, provide simple named image buffer management */
typedef struct {
	unsigned short *pixels;
	int            size;
	short          xdim;
	short          ydim;
	short          zdim;
	short          xbin;
	short          ybin;
	short          type;
	char           name[64];
	int            shmid;
	size_t         shmsize;
	char           *shmem;
} CCD_FRAME;

typedef void *PDATA;
#define MAX_CCD_BUFFERS  1000
PDATA CCD_locate_buffer(char *name, int idepth, short imgcols, short imgrows, short hbin, short vbin);
int   CCD_free_buffer();
int   CCD_locate_buffernum(char *name);
extern CCD_FRAME CCD_Frame[MAX_CCD_BUFFERS];
extern int CCD_free_buffer(char *name);



//////////////////////////////////////////////////
// user's code
//////////////////////////////////////////////////
extern int InitCCDCamera(double cooling);	// <== cooler on
extern void TakeCCDImage(double exp_time, int light_frame, int binning_factor, char *filename);
extern void ClosedCCDCamera (void);		// <== cooler off
extern void ShutdownCCDCamera (void);		// <== cooler shutdown ( equal warm up ? )


int InitCCDCamera(double cooling)
{
	int status=0;
	cout<<"InitCCDCamera.\n";
	
	status=alta->InitDriver(camnum,0,0);
	if(status)
	{
		printf("Initialize driver successfully\n");
		/*      Do a system reset to ensure known state, flushing enabled etc */
		alta->ResetSystem();
	
	/*      Set the required fan mode */
		alta->write_FanMode(Apn_FanMode_High);
		
	/*      Single image per download */
		alta->write_ImageCount(1);  	
			
		alta->write_CoolerEnable(1);
		alta->write_CoolerSetPoint(cooling);

		cout << "read_CoolerSetPoint="<< alta->read_CoolerSetPoint() <<"\nCcd Initialization done\n";
		return 1;
	}
	else
	{
		printf("Failed to initialize driver\n");
		return 0;
	}

}

void TakeCCDImage(double d_exp_time, int shutter, int binning_factor, char *filename)
{
	time_t waiting_time;
	time_t t0;
	int mon, day, yr, hr, min, sec;
	struct tm *tm_ptr;
	char buf[128];
	bool status;
	int bnum;
	int iexposure;
	
	// prepare global vars (date_str and time_str) for fits header
	// read system time 
	// write to header in saveimage()
	time(&t0);
	tm_ptr=gmtime(&t0);//Converts to UTC time
	strftime(buf, 256, "%m %d %Y %H %M %S", tm_ptr);
	sscanf(buf, "%d %d %d %d %d %d", &mon, &day, &yr, &hr, &min, &sec);
	sprintf(date_str, "%04d-%02d-%02d", yr, mon, day);
	sprintf(time_str, "%02d:%02d:%02d", hr, min, sec);
	
	filter_type[0] = p_tat_info->obs_info.filter_type;
	filter_type[1] = '\0';

	/*      Set up binning */
	alta->m_pvtRoiPixelsH = 1024 / binning_factor;
	alta->m_pvtRoiPixelsV = 1024 / binning_factor;
	alta->write_RoiBinningH(binning_factor);
	alta->write_RoiBinningV(binning_factor);
      
              
      /*          Start an exposure */
	alta->ResetSystem();
	status = alta->Expose(d_exp_time,shutter);
	iexposure=(int) d_exp_time +1;
	sleep( iexposure);

	status = alta->BufferImage("tempobs");

	/* 
	Prepare global vars ( ccd_temperature and exposure_time ) for fits header.
	double type.
	Write to header in saveimage().
	*/
	curr_point=alta->read_TempCCD();
	texposure=d_exp_time;
		
		
	/*          Use the libccd routine to find the corresponding buffer index */
	bnum = CCD_locate_buffernum("tempobs");
	/*          Obtain the memory address of the actual image data, and x,y dimensions */
	saveimage(CCD_Frame[bnum].pixels, filename, 
			CCD_Frame[bnum].xdim, CCD_Frame[bnum].ydim);   
}

void ShutdownCCDCamera (void)
{
	alta->write_CoolerEnable(0);
	/* RAMP_TO_AMBIENT */
	cout << "Shutdown Ccd Cooler\nCooler ramp to ambient\n";
}


void ClosedCCDCamera (void)
{
        /*       All done, tidy up */
	if(alta != NULL)
	{
		alta->CloseDriver();
		cout << "Close ccd driver\n";
	}
	else
	  cout << "CCD cannot be closed.\n";
}



main(int argc, char *argv[])
{
        /* vars for watchdog */
	pid_t pid, prev_child_pid=0, temp_child_pid, pid_exposing=0;
	int ccd_is_exposing=0;
	int pid_status;
	
	/* vars for command */
	FILE *fin, *ftemp;
	int CmdOk=0, CcdIsRdy=0, ReadFailedCnt=0;
	char function[10];
	char filename[200], exp1[5], exp2[5], light[5], image_bin[2], *endptr;
	char parent_dir[200];
	
	/* vars for takeimage */
	double d_exp_time;
	int exp_time1, exp_time2;
	int light_frame, binning_factor;
	double cooling;
	int locked=0;
	
	time_t currTime;
	char *pcCurrTime;

	clock_t start, end;
	double cpu_time_used;

	
	//To get the uid and gip of 'observer'
	struct passwd *user;
	user = getpwnam(USERNAME);
	
	/* for 96 frames bug  */
	int frame_number=0;

	/* cycle */
	int cycle_time=CYCLE_TIME; 
	int cycle_count=CYCLE_COUNT;           
	           
	
	float msg[5];	/* current point
			   set point
			   camera status
			   cooler status
			   cooler mode */
				   	
                

	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);

//#define MULTI_PROCESS
#ifdef MULTI_PROCESS
	///////////////////////////////////////////////////
	//watch dog
	///////////////////////////////////////////////////
	while(1) 
	{
		pid=fork();
		if(pid==-1)
		{
		//      error
				perror("-1");
		}
		else if(pid!= 0)
		{
			/* parent  */
			fprintf (stdout, "wait child pid =%d\n",pid);
			waitpid(pid,(int *)0,0);
			fprintf( stdout, "child pid= %d exit\n\n", pid);
			prev_child_pid= pid;
			
			time(&currTime);
			pcCurrTime= ctime( &currTime);
			ftemp= fopen(DAEMON_LOG_FILENAME, "a+");
			if( ftemp != NULL)
			{
			fprintf( ftemp, "%s ccddaemon child pid=%d exit.\n", pcCurrTime, pid);
			fclose( ftemp);
			}
			else
			{
			printf("fopen(DAEMON_LOG_FILENAME) failed\n");
			}
		}
		else
		{
			/* child */
			break;
		}
	} //end while
        
        
	
#ifdef FRAME_BUG	
/* for 96 frames bug  */
	if( (fp= fopen( "/home/tat/daemon/ccddaemon.pid", "r") )!= NULL )
	{
		fscanf( fp, "%d %f", &temp_child_pid, &curr_set_point);
		fprintf( stdout, "prev child pid=%d\tset point=%f\n", temp_child_pid, cooling);
		if( temp_child_pid == prev_child_pid )
		{
			InitCCDCamera( cooling);
			CcdIsRdy=1;
			cout<< "Restart CCD daemon successfully\n";
		}
		else
			init_ccd_info( &(p_tat_info->ccd_info) );
		
		fclose( fp);
		remove( "/home/tat/daemon/ccddaemon.pid");
	}

#endif 
#endif
        


	//////////////////////////////////////////////////	
	// let FILE cannot be deleted by observer.
	//////////////////////////////////////////////////
//    	umask(0);
    	
    	//////////////////////////////////////////////////
    	// clear old command and reply file.
    	//////////////////////////////////////////////////
	remove(CCD_CMD_FILENAME);
	remove(CCD_RPY_FILENAME);

	fprintf(stdout, "CCD Daemon Start.\n");        
	if( alta == NULL )
		fprintf(stdout, "alta is NULL\n");
	else
		fprintf(stdout, "alta is not NULL\n");
	while(1)
	{
		if( ( fin=fopen(CCD_CMD_FILENAME, "r") ) == NULL )
		{
			usleep(CYCLE_TIME); 
			//unlock CCD here.
			if( pid_exposing>0)
			{
				if( pid_exposing==waitpid(pid_exposing, &pid_status, WNOHANG) )
				{
					frame_number++;
					printf("++Unlock takeimage \n");
					pid_exposing=0;
					locked=0;
				}
			}
		}
		else
		{
			ReadFailedCnt= 0; CmdOk=0;

			do
			{
				strcpy(function,"");
				strcpy(filename,"");

				fscanf(fin, "camera %s %s %lf %d %d\n",
					&function, &filename, &d_exp_time, &light_frame, &binning_factor);
	
				fprintf (stdout, "string length for function:%d, filename=%d\n",
						strlen(function), strlen(filename) );

				if( ( strlen(function)>=2 ) && (strlen(filename)>=2) )
				{
                                        
					fprintf (stdout, "function:%s, file:%s, exp:%f, light:%d, bin:%d\n",
						function, filename, d_exp_time, light_frame, binning_factor);
		
					if( strcmp(function,"on")== 0 )
					{
						printf ("Cmd:Cooler-On\n");
						//In this case d_exp_time1 equal set point (temperature)
						cooling = (double)d_exp_time;
						if (CcdIsRdy == 1 )
						{
							printf ("Cooler already active\nset point %f", cooling);
							//alta->write_CoolerSetPoint(cooling);
							//alta->write_CoolerEnable(1);
							InitCCDCamera(cooling);
						}
						else 
						{
					/*      Create the camera object , this will reserve memory */
							alta = (CApnCamera *)new CApnCamera();
							CcdIsRdy=InitCCDCamera(cooling);
						}
					}
					else if( strcmp(function,"shutdown") ==0  )
					{
						printf ("Cooler-Shutdown\n");
						//ShutdownCCDCamera();
						//alta->ResetSystem();
						alta->write_CoolerEnable(1);
						alta->write_CoolerSetPoint(18);
						usleep (500000);
						alta->write_CoolerEnable(0);
						//InitCCDCamera(18.0);
						alta->write_FanMode(Apn_FanMode_Low);
						
					}
					else if( strcmp(function,"off") ==0  )
					{
						printf ("Cooler-Off\n");
						CcdIsRdy=0;
						ClosedCCDCamera();
						alta = (CApnCamera *)NULL;
						delete alta;
					}
					else if( CcdIsRdy==1)
					{
						if( strcmp(function,"takeimage") == 0 )
						{
							if(!locked)
							{
								if(alta->read_ImagingStatus()==4)       
								{
									locked=1;
									printf("++Lock takeimage\n");
									
									pid_exposing=fork();
									if(pid_exposing==0)
									{
										//child
										strcpy( parent_dir, filename);
										dirname( parent_dir );
										mkdir(parent_dir, S_IRWXU|S_IROTH|S_IXOTH|S_IRGRP|S_IXGRP);
										chown(parent_dir,user->pw_uid,user->pw_gid);
// 										chown(parent_dir,0,100);
										fprintf (stdout, "==Take image NO. %d (Begin).\n", frame_number+1);
											
												
										start= clock();
										TakeCCDImage(d_exp_time, light_frame, binning_factor, filename);
										end= clock();
										cpu_time_used= ((double) (end - start)) / CLOCKS_PER_SEC;
										chown(filename,user->pw_uid,user->pw_gid);
										strcpy( p_tat_info->obs_info.recent_image, filename);
										fprintf (stdout, "==Take image NO. %d (Finish). ?cputime used: %f sec\n", frame_number+1, cpu_time_used);
										return 0;
									}
								}
								else
								{
									printf("CCD ImagingStatus %d.Try later\n",alta->read_ImagingStatus());
								}
							}
							else
							{
								printf("Locked!! CCD is taking image now.Try later\n");
							}
						}
						else if( strcmp(function," query") == 0 )
						{
							printf("void\n");
								/*
							if( (ftemp=fopen(CCD_RPY_FILENAME,"w")) != NULL)
							{
								fprintf(ftemp,"void"); 
								fclose(ftemp);
							}
						*/
						}
					}
					else
					{
						printf("CCD Cooler ON first or error command\n");
						/*
						if( (ftemp= fopen( CCD_RPY_FILENAME, "w")) != NULL)
						{
							fprintf(ftemp, "CCD Cooler ON first or error command\n");
							fclose( ftemp);
						}
						*/
					}
				CmdOk= 1;
				}
				else
				{
					rewind(fin);				
					ReadFailedCnt++;
				}
			}while( ReadFailedCnt <=3 && CmdOk!=1 );
			fclose(fin);
			remove( CCD_CMD_FILENAME);
		}	//else
		/* update ccd info */
		if( CcdIsRdy == 1)
		{
			if( cycle_count == 0)
			{
				cycle_count=CYCLE_COUNT;
                                
			/* see Apogee.h */
				msg[0]=(float)alta->read_TempCCD();
				msg[1]=(float)alta->read_CoolerSetPoint();
				msg[2]=(float)alta->read_ImagingStatus(); //Apn_Status_Exposing ...
				msg[3]=(float)alta->read_CoolerStatus(); //Apn_CoolerStatus_AtSetPoint ...
				msg[4]=alta->read_CoolerDrive();
				msg[5]=alta->read_TempHeatsink();
				update_ccd_info( &(p_tat_info->ccd_info), msg);
				
			}
			cycle_count--;
			
		}
		else
		{
		  //printf("ccd is not ready\n init_ccd_info()\n");
		  init_ccd_info( &(p_tat_info->ccd_info) );	
		}	
		
  	}	// while(1)
}	// main

/*  Helper routines start here-------------------------------------------------*/

/*  This routine provides a very simple command line parser
 *  Unknown options should be ignored, but strict type
 *  checking is NOT done.
 */
#if 0
int parse_options (int argc, char **argv)
{
   int i;
   int goti,gott,gots,gota;

/* Zero out counters for required options */
   goti=0;
   gott=0;
   gots=0;
   gota=0;
   i=1;

/* Default fanmode to medium */
   fanmode = 2;

/* Loop thru all provided options */
   while (i<argc) {

/*     Image name */
       if (!strncmp(argv[i],"-i",2)) {
          strcpy(imagename,argv[i+1]);
          goti = 1;
       }

/*     Exposure time */
       if (!strncmp(argv[i],"-t",2)) {
          sscanf(argv[i+1],"%lf",&texposure);
          gott = 1;
       }

/*     Shutter state */
       if (!strncmp(argv[i],"-s",2)) {
          sscanf(argv[i+1],"%d",&shutter);
          gots= 1;
       }

/*     IP address for ALTA-E models */
       if (!strncmp(argv[i],"-a",2)) {
          sscanf(argv[i+1],"%d.%d.%d.%d",ip,ip+1,ip+2,ip+3);
          gota = 1;
       }

/*     Fast readout mode for ALTA-U models */
       if (!strncmp(argv[i],"-F",2)) {
          sscanf(argv[i+1],"%d",&highspeed);
       }

/*     Horizontal binning */
       if (!strncmp(argv[i],"-x",2)) {
          sscanf(argv[i+1],"%d",&xbin);
       }

/*     Vertical binning */
       if (!strncmp(argv[i],"-y",2)) {
          sscanf(argv[i+1],"%d",&ybin);
       }

/*     Region of interest */
       if (!strncmp(argv[i],"-r",2)) {
          sscanf(argv[i+1],"%d,%d,%d,%d",&xstart,&ystart,&xend,&yend);
       }

/*     Bias subtraction */
       if (!strncmp(argv[i],"-b",2)) {
          sscanf(argv[i+1],"%d",&biascols);
       }

/*     Fan mode */
       if (!strncmp(argv[i],"-f",2)) {
          if (!strncmp(argv[i+1],"off",3)==0) fanmode=0;
          if (!strncmp(argv[i+1],"slow",4)==0) fanmode=1;
          if (!strncmp(argv[i+1],"medium",6)==0) fanmode=2;
          if (!strncmp(argv[i+1],"fast",4)==0) fanmode=3;
       }

/*     Setpoint temperature */
       if (!strncmp(argv[i],"-c",2)) {
          sscanf(argv[i+1],"%lf",&curr_point);
       }

/*     Sequence of exposures */
       if (!strncmp(argv[i],"-n",2)) {
          sscanf(argv[i+1],"%d",&numexp);
       }

/*     USB camera number */
       if (!strncmp(argv[i],"-u",2)) {
          sscanf(argv[i+1],"%d",&camnum);
       }

/*     Interval to pause between exposures */
       if (!strncmp(argv[i],"-p",2)) {
          sscanf(argv[i+1],"%d",&ipause);
       }

/*     Be more verbose */
       if (!strncmp(argv[i],"-v",2)) {
          sscanf(argv[i+1],"%d",&verbose);
       }

/*     Print usage info */
       if (!strncmp(argv[i],"-h",2)) {
          printf("Apogee image tester -  Usage: \n \
	 -i imagename    Name of image (required) \n \
	 -t time         Exposure time is seconds (required)\n \
	 -s 0/1          1 = Shutter open, 0 = Shutter closed (required)\n \
	 -a a.b.c.d      IP address of camera e.g. 192.168.0.1 (required for ALTA-E models only)\n \
	 -F 0/1          Fast readout mode (ALTA-U models only)\n \
	 -u num          Camera number (default 1 , ALTA-U only) \n \
	 -x num          Binning factor in x, default 1 \n \
	 -y num          Binning factor in y, default 1 \n \
	 -r xs,ys,xe,ye  Image subregion in the format startx,starty,endx,endy \n \
	 -b biascols     Number of Bias columns to subtract \n \
	 -f mode         Fanmode during exposure, off,slow,medium,fast (default medium) \n \
	 -c temp         Required temperature for exposure, default is current value \n \
	 -n num          Number of exposures \n \
	 -p time         Number of seconds to pause between multiple exposures \n \
	 -v verbosity    Print more details about exposure\n");
         exit(0);
        }

/*      All options are 2 args long! */
        i = i+2;
   } 

/* Complain about missing required options, then give up */
   if ( goti == 0 ) printf("Missing argument  -i imagename\n");
   if ( gott == 0 ) printf("Missing argument  -t exposure time\n");
   if ( gots == 0 ) printf("Missing argument  -s shutter state\n");
#ifdef ALTA_NET
   if ( gota == 0 ) printf("Missing argument  -a IP address\n");
   if (goti+gots+gott+gota != 4) exit(1);
#else
   if (goti+gots+gott != 3) exit(1);
#endif

/* Print exposure details */
   if (verbose > 0) {
      printf("Apogee ALTA image test - V1.6\n");
      printf("	Image name is %s\n",imagename);
      printf("	Exposure time is %lf\n",texposure);
      if (numexp    > 1) printf("	Sequence of %d exposures requested\n",numexp);
      if (ipause    > 0.0) printf("	Pause of %d seconds between exposures\n",ipause);
      printf("	Shutter state during exposure will be %d\n",shutter);
#ifdef ALTA_NET
      if (ip[0]     != 0) printf("	ALTA-E ip address is %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
#endif
      if (xbin      > 1) printf("	X binning selected xbin=%d\n",xbin);
      if (ybin      > 1) printf("	Y binning selected ybin=%d\n",ybin);
      if (xstart    != 0) printf("	Subregion readout %d,%d,%d,%d\n",xstart,xend,ystart,yend);
      if (biascols  != 0) printf("	Bias subtraction using %d columns\n",biascols);
      if (fanmode > 0) printf("	Fan set to mode = %d\n",fanmode);
      if (curr_point < 99.0) printf("	Requested ccd temperature for exposure is %lf\n",curr_point);
   }
   return(0);

}
#endif 

/*  This routine provides simple FITS writer. It uses the routines
 *  provided by the fitsTcl/cfitsio libraries
 *
 *  NOTE : It will fail if the image already exists
 */

int saveimage(unsigned short *src_buffer, char *filename, short nx, short ny)
{
	fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
	long  fpixel, nelements;
	unsigned short *array;
	unsigned short *simg;
	int status;
	/* initialize FITS image parameters */
	int bitpix   =  USHORT_IMG; /* 16-bit unsigned short pixel values       */
	long naxis    =   2;  /* 2-dimensional image                            */
	long naxes[2];   
// 	char site[BUFSIZ],alti[50],longi[50],lat[50];
	char temp_string[50],imagetype[5];
	float f_var,ra,dec;
	int sign, ra_h,ra_m,ra_s,dec_d,dec_m,dec_s;
	char ra_str[20], dec_str[20] ,ctemp[5];
	
    naxes[0] = nx-bcols;
    naxes[1] = ny; 
    array = src_buffer;
    status = 0;         /* initialize status before calling fitsio routines */
	
	switch( p_tat_info->obs_info.ccd_status )
	{
			case CCD_IMAGE:
				sprintf( imagetype, "Star");
			break;
			case CCD_FLAT:
				sprintf( imagetype, "Flat");
			break;
			case CCD_DARK:
				sprintf( imagetype, "Dark");
			break;
			default:
				sprintf( imagetype, "Test");
			break;
	}
	imagetype[4]='\0';
	
//    simg = (unsigned short *)CCD_locate_buffer("stemp",2,nx-bcols,ny,1,1);
    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
 
    /* write the required keywords for the primary array image.     */
    /* Since bitpix = USHORT_IMG, this will cause cfitsio to create */
    /* a FITS image with BITPIX = 16 (signed short integers) with   */
    /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
    /* FITS uses to store unsigned integers.  Note that the BSCALE  */
    /* and BZERO keywords will be automatically written by cfitsio  */
    /* in this case.                                                */          
 
    if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
 
 
    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */
//    dobiassubtract(src_buffer,simg,naxes[0],naxes[1]);                                           
/* array: an original one. simg: bias subtract.

    /* write the array of unsigned integers to the FITS file */
    if ( fits_write_img(fptr, TUSHORT, fpixel, nelements, array, &status) )
        printerror( status );
      //write the array of unsigned integers to the FITS file

	fits_write_key(fptr, TSTRING, "DATE-OBS" , date_str,
			"YYYY-MM-DD, UT", &status);

	fits_write_key(fptr, TSTRING, "TIME-OBS" , time_str,
			"HH:MM:SS, UT", &status);
	f_var=(float)curr_point;
	fits_write_key(fptr, TFLOAT, "CCDTEMP", &f_var,
			"CCD Temperature", &status);
	f_var=(float)texposure;
	fits_write_key(fptr, TFLOAT, "EXPTIME" , &f_var,
			"Exposure time in seconds", &status);
	
	fits_write_key(fptr, TSTRING, "LOCATION",OBSERVATORY, "Site of the telescope", &status);
	sprintf(temp_string,"%.4f",LOCAL_LAT);
	fits_write_key(fptr, TSTRING, "LATITUDE" ,temp_string,"Latitude of the telescope", &status);
	sprintf(temp_string,"%.4f",LOCAL_LONG);
	fits_write_key(fptr, TSTRING, "LONGITUD",temp_string,"Longitude of the telescope (Positive to West)", &status);
	sprintf(temp_string,"%.0f",LOCAL_ALT);
	fits_write_key(fptr, TSTRING, "ALTITUDE", temp_string,"Altitude of the telescope in meters", &status);
	
	
// 	DoGetValueString("LOCATION", site);
// 	if (site != NULL)
// 	{
// 		fits_write_key(fptr, TSTRING, "LOCATION" , site, "site", &status);
// 		
// 		DoGetValueString("LAT", lat);
// 		if(lat != NULL)
// 			fits_write_key(fptr, TSTRING, "LAT" , lat,
// 					"Latitude of the telescope", &status);
// 		
// 		DoGetValueString("LONG", longi);
// 		if(longi != NULL)
// 			fits_write_key(fptr, TSTRING, "LONG" , longi,
// 					"Longitude of the telescope (Positive to West)", &status);
// 		
// 		DoGetValueString("ALT", alti);
// 		if(alti != NULL)
// 			fits_write_key(fptr, TSTRING, "ALT" , alti,
// 					"Altitude of the telescope in meters", &status);
// 	}

	fits_write_key(fptr, TSTRING, "IMGTYPE" , imagetype ,"Type of exposure", &status);
	
	if(!strcmp(imagetype,"Star") || !strcmp(imagetype,"Flat"))
	{
		fits_write_key(fptr, TSTRING, "FILTER" , filter_type ,"Filter used on the exposure", &status);
		
		sprintf(temp_string,"%ld",p_tat_info->fli_info.focuser_curr_position);
		fits_write_key(fptr, TSTRING, "FOC_P", temp_string,"Focuser position in steps", &status);
	}
	
	if(!strcmp(imagetype,"Star"))
	{
		ra =  p_tat_info->obs_info.RA;
		dec =  p_tat_info->obs_info.DEC;

		if(dec<0) sign=-1;
		else sign=1;

		dec *= sign;

		ra_h = (int) ra;
		ra_m = (int) (60. * (ra -ra_h));
		ra_s = (int) (60. *((60.*(ra -ra_h)) - ra_m));

		dec_d = (int) dec;
		dec_m = (int) (60. * (dec -dec_d));
		dec_s = (int) (60. *((60.*(dec -dec_d)) - dec_m));

		dec_d *= sign;

		sprintf(ra_str,"%02d:%02d:%02d",ra_h,ra_m,ra_s);
		sprintf(dec_str,"%02d:%02d:%02d",dec_d,dec_m,dec_s);
		fits_write_key(fptr, TSTRING, "RA" , ra_str ,"Right ascension of target", &status);
		fits_write_key(fptr, TSTRING, "DEC" , dec_str ,"Declination of target", &status);


		fits_write_key(fptr, TSTRING, "TARGET" , p_tat_info->obs_info.target_name ,"Name of the target", &status);
	}
    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
	else //GOOD
		generate_web_image(filename, 0);//Generate web 0 = CCD
  
    return(status);
}                                                                               


/*  This routine should do bias subtraction. At present it
 *  uses the minium pixel DN as the bias value, instead of
 *  averaging the bias columns. This is because on the 
 *  test unit I have, averaging these columns does not seem
 *  to give values consistently lower than those in the 
 *  exposed region.
 *
 *  src is the input image with bias columns
 *  dest is a smaller output image with the bias columns trimmed off
 *       and the "bias" subtracted from the image pixels.
 */

int dobiassubtract(unsigned short *src,unsigned short *dest, int nx, int ny)
{
   double biases[8192];
   double abiases;
   int ix,iy, oix;
   int ipix, opix;
   unsigned short minbias;
   minbias = 65535;
   if (bcols == 0) {
     for (iy=0;iy<ny;iy++) {
        biases[iy] = 0.0;
     }
     minbias = 0;
   } else {
     for (iy=0;iy<ny;iy++) {
        biases[iy] = 0.0;
        for (ix=bias_start;ix<=bias_end;ix++) {
            ipix = (nx+bcols)*iy + ix-1;
            biases[iy] = biases[iy] + (float)src[ipix];
            if (src[ipix]<minbias) minbias = src[ipix];
        }
        biases[iy] =  biases[iy] / (float)bcols;
     }
   }
  
   for (iy=0;iy<ny;iy++) {
      oix = 0;
      for (ix=0;ix<nx+bcols;ix++) {
        if (ix < bias_start || ix > bias_end) {
          ipix = (nx+bcols)*iy + ix;
          opix = nx*iy + oix;
          if (src[ipix] < minbias) {
             dest[opix] = 0;
          } else {
            dest[opix] = src[ipix] - (int)minbias;
          }
          oix++;
        }
      }
   }
   return(0);
}


