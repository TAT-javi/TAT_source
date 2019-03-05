/*
	CCDdaemon version for Apogee Alta f6
	It asumes the apogee driver is installed 
	and the scripts tcl are working
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pwd.h>
#include "fitsio.h"
#include <sys/wait.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include "symblc_const.h"

#define CLIENT_PROGRAM "/opt/apogee/apps/tcl/scripts/apogeeclient"
#define SERVER_PROGRAM "/opt/apogee/apps/tcl/scripts/startserver"
#define SERVER_PROGRAM_SHORT "startserver"
#define APOGEE_PROGRAM "/opt/apogee/apps/tcl/scripts/apogeeserver.tcl"

#define WAIT_START 7
#define CYCLE_TIME 100000	/*microsecond usleep*/ 
#define CCD_CMD_FILENAME "/home/observer/ccd_command.cmd"

#define USERNAME "observer"

void init_CCD(float temperature);
int check_if_server_alive(void);
void set_temperature(float temperature,int amb);
void shutdown_CCD(void);
float get_temperature(void);
void take_image(float exp_time,int shutter, char *filename);


/* Global variables*/
int server_connected=0;//,CCD_locked=0;
st_tat_info	*p_tat_info;

int main(void)
{
	//multiprocess variables
	pid_t pid, pid_exposing=0;

	//time variables
	time_t currTime;
	char *pcCurrTime;

	char cmd[2000],function[10],filename[200],parent_dir[200];

	float exp_time,temperature_current, temperature_set;
	int shutter,diff,frame_number=0;
	clock_t start,end;

	float msg[5];
	int i,count_sec=0,idlecount=0;

	FILE *fin,*fp,*ftemp;
// To get the user uid gid
	struct passwd *user;
	user = getpwnam(USERNAME);
	

	/* shm */
	int		shmid;
	
	/* get tat_info shared memory */
	create_tat_info_shm( &shmid, &p_tat_info);

	
	while(1)
	{
		pid=fork();
		if(pid==-1)return 1;  //  error
		else if(pid!= 0) //parent
		{	/* parent  */
			fprintf (stdout, "wait child pid =%d\n",pid);
			waitpid(pid,(int *)0,0);
			fprintf( stdout, "child pid= %d exit\n\n", pid);
		
		//kill the server process
			sprintf(cmd,"%s shutdown",CLIENT_PROGRAM);  		
			system(cmd);		

			time(&currTime);
			pcCurrTime= ctime( &currTime);
			ftemp= fopen(DAEMON_LOG_FILENAME, "a+");
			if( ftemp != NULL)
			{
				fprintf( ftemp, "%s ccddaemon child pid=%d exit.\n", pcCurrTime, pid);
				fclose( ftemp);
			}
			else
				printf("fopen(DAEMON_LOG_FILENAME) failed\n");
		}
		else  /* child */
  			break;
	}//end while(1)

	// clear old command
	remove(CCD_CMD_FILENAME);

	//MAIN LOOP
	while(1)
	{
		if((fin=fopen(CCD_CMD_FILENAME,"r"))==NULL)
		{ //Wait for the file
			usleep(CYCLE_TIME);
			count_sec++;
		}
		else
		{
			fscanf(fin, "camera %s %s %f %d %*s\n",
				function, filename, &exp_time, &shutter);

			printf ("function:%s, file:%s, exp:%f, light:%d\n",
						function, filename, exp_time, shutter);
			
			fclose(fin);
			remove( CCD_CMD_FILENAME);
			
			if(!strcmp(function,"on"))
			{
				printf ("Cmd:Cooler-On\n");
				//In this case exp_time equal set point (temperature)
				temperature_set = exp_time;
				if (server_connected)
				{
					printf ("Cooler already active\nset point %f", temperature_set);
					set_temperature(temperature_set,0);
				}
				else 
				{
					/* Turn on the CCD server */
					init_CCD(temperature_set);
				}
			}
			else if(!server_connected) //if server not connected, don't do anything else
			{
				printf("Server not ON!!!");
			}
			else if(!strcmp(function,"shutdown"))
			{//For historical reasons shutdown means cooler off
				printf ("Cooler-OFF\n");
				sprintf(cmd,"%s setpoint off",CLIENT_PROGRAM);
				system(cmd);
			}
			else if(!strcmp(function,"reset"))
			{//to avoid problems when ccd is locked
				p_tat_info->ccd_info.ccd_locked=0;
			}
			else if(!strcmp(function,"off"))
			{//For historical reasons off means shutdown the server
				//Before shutdown the server we shutdown the cooler first
				printf ("Server shutdown\n");
				sprintf(cmd,"%s setpoint off;sleep 2;%s shutdown",
						CLIENT_PROGRAM,CLIENT_PROGRAM);
				system(cmd);
				server_connected =0;
 				p_tat_info->ccd_info.server_connected=0;
				p_tat_info->ccd_info.ccd_locked=0;
				// reset the shared memory information
				msg[0]=msg[1]=999;
				for(i=2;i<5;i++)msg[i]=0; 
				
				update_ccd_info(&(p_tat_info->ccd_info),msg);
			}
			else if(!strcmp(function,"abort") && p_tat_info->ccd_info.ccd_locked)
			{
				//need to abort the taking exposure
				printf ("Abort exposure\n");
				sprintf(cmd,"%s abort",CLIENT_PROGRAM);
				system(cmd);
				//kill child
				kill(pid_exposing, SIGKILL);
				sleep(1);
				waitpid(pid_exposing,(int *)0,0);
				p_tat_info->ccd_info.ccd_locked=0;
			}
			else if(!strcmp(function,"takeimage"))
			{
 				if(!p_tat_info->ccd_info.ccd_locked)
//				if(!CCD_locked)
				{
					frame_number++;
					p_tat_info->ccd_info.ccd_locked=1;
					printf("++Lock takeimage\n");
					pid_exposing=fork();
					if(pid_exposing==0)
					{
						//child
						strcpy( parent_dir, filename);
						dirname( parent_dir );
						mkdir(parent_dir, S_IRWXU|S_IROTH|S_IXOTH|S_IRGRP|S_IXGRP);
// 						chown(const char *file, int id-owner, int id-group);
						chown(parent_dir,user->pw_uid,user->pw_gid);
						printf("==Take image NO. %d (Begin).\n", frame_number);
								
						start= time(NULL);
						take_image(exp_time, shutter, filename);
						end= time(NULL);
						diff = end - start;
 						p_tat_info->ccd_info.ccd_locked = 0;
						strcpy( p_tat_info->obs_info.recent_image, filename);
						printf ("==Take image NO. %d (Finish)\n%s\nTime used: %d sec\n", 
								frame_number,filename,diff);
						chown(filename,user->pw_uid,user->pw_gid);
						return 0;
					}
				}
				else
					printf("Locked!! CCD is taking image now. Try later\n");
			}
		}
		
		//harvest the child process (if any)
		if(pid_exposing != 0 && !p_tat_info->ccd_info.ccd_locked)
		{
			printf("Waiting for child...\n");
			waitpid(pid_exposing,(int *)0,0);
			pid_exposing =0;
		}
			
		
		
		if(count_sec >=10) //update every sec
		{
			count_sec=0;
			if(server_connected) //Show Status
			{
				msg[1] = temperature_set;
				sprintf(cmd,"%s status",CLIENT_PROGRAM);
				fp = popen(cmd, "r");
				if(fp == NULL)return 0;

				fscanf(fp,"%*s , %f , %f , %f , %f\n",
						&msg[0],&msg[2],&msg[3],&msg[4]);
				pclose(fp);
				if(p_tat_info->ccd_info.ccd_locked)
				{
					if(msg[3] == 0 || msg[3] == 4)
					{
						idlecount++;
						if(idlecount > 20)
						{
							idlecount =0;
							p_tat_info->ccd_info.ccd_locked = 0;
						}
					}
					else idlecount =0;
				}
				update_ccd_infof6(&(p_tat_info->ccd_info),msg);
			}
		}
	}
	return 0;
}

void init_CCD(float temperature)
{
	int out=0;
	char cmd[2000];
	while(!out)
	{
		//try to kill any previous program
		if(p_tat_info->ccd_info.server_connected)
			shutdown_CCD();

		//start the server
		printf("%s &\n",SERVER_PROGRAM);
		sprintf(cmd,"runuser -l root -c %s &",SERVER_PROGRAM);
		system(cmd);
		sleep(WAIT_START);
		out = check_if_server_alive();
// 		printf("OUT = %d\n",out);
	}
	server_connected = 1;
 	p_tat_info->ccd_info.server_connected=1;
	//set temperature
	set_temperature(temperature,0);
}

void shutdown_CCD(void)
{
	char cmd[1000];
	printf("pkill %s\n",SERVER_PROGRAM_SHORT);
	sprintf(cmd,"pkill %s",SERVER_PROGRAM_SHORT);
	system(cmd);
	server_connected = 0;
}

void set_temperature(float temperature,int amb)
{
	char cmd[200];
	//set the temperature
	if(!amb)
		sprintf(cmd,"%s setpoint set %.2f",CLIENT_PROGRAM,temperature);
	else
		sprintf(cmd,"%s setpoint amb",CLIENT_PROGRAM);
		
	system(cmd);
}

int check_if_server_alive(void)
{
	FILE *fp;
	char cmd[2000],c;
	
	sprintf(cmd,"%s status",CLIENT_PROGRAM);
	fp = popen(cmd, "r");
	if(fp == NULL)return 0;

	fscanf(fp,"%c",&c);
	pclose(fp);
// 	printf("READ - %c\n",c);
	if(c=='0') return 1; //success
	else return 0; // failed
}

float get_temperature(void)
{
	float temp =100000;
	char cmd[2000];
	FILE *fp;

	sprintf(cmd,"%s getpoint",CLIENT_PROGRAM);

	fp = popen(cmd,"r");
	if(fp!=NULL)//NO ERROR
	{
		fscanf(fp,"%f",&temp);
		fclose(fp);
	}
	return temp;
}

void take_image(float exp_time,int shutter, char *filename)
{
	int sleeptime,ccd_status,i;
	int ra_h,ra_m,ra_s,dec_d,dec_m,dec_s,sign;
	float temp1,temp2,temp_avg,ra,dec;
	char cmd[2000],temp_string[50],comment[100];
	char ra_str[50],dec_str[50],filter_type[2];
	
	//FITS variables;
	fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
	int status=0;
	char site[BUFSIZ],alti[50],imagetype[5];
	FILE *fp;

	//read temperature from shm first
	temp1 = p_tat_info->ccd_info.curr_point;
	
	//get the filter type now
	filter_type[0] = p_tat_info->obs_info.filter_type;
	filter_type[1] = '\0';
	
	sprintf(cmd,"%s snapshot %s %.2f %d",CLIENT_PROGRAM,
			filename,exp_time,shutter);
	system(cmd);

	sleeptime = (int) exp_time;
	sleep(sleeptime);
		//wait for the fits file to exist
	i=0;
	while(i<CCD_READOUT_TIME)
	{
		sleep(1);
		if((fp=fopen(filename,"r"))!=NULL) 
		{
			fclose(fp);
			i=CCD_READOUT_TIME;
			break;
		}
		i++;
	}
	
	
	temp2 = p_tat_info->ccd_info.curr_point;
	temp_avg = (temp1+temp2)/2.0;
	printf("Temperature average = %.2f\n",temp_avg);
	
	/////////////////
	//Add information to the header
	/////////////////
	fits_open_file(&fptr, filename, READWRITE,&status);
	if(status)//error so soon?
	{
		printf("ERROR: Reading %s\n",filename);
		fits_report_error(stderr,status);
	}
	else
	{
		ccd_status =p_tat_info->obs_info.ccd_status;
		switch( ccd_status )
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

		//Remove old strings
		fits_delete_key(fptr, "TELESCOP", &status);
		fits_delete_key(fptr, "INSTRUME", &status);
		fits_delete_key(fptr, "DETECTOR", &status);
		fits_delete_key(fptr, "INSTID",   &status);
		fits_delete_key(fptr, "OBSERVER", &status);
		fits_delete_key(fptr, "OBJECT",   &status);
		fits_delete_key(fptr, "OBSTYPE",  &status);
		fits_delete_key(fptr, "OBSERVAT", &status);
		fits_delete_key(fptr, "LONGITUD", &status);
		fits_delete_key(fptr, "LATITUDE", &status);
		
		fits_update_key(fptr,TFLOAT,"CCDTEMP",&temp_avg,"Average temperature of the CCD",&status);
		fits_update_key(fptr,TFLOAT,"EXPTIME",&exp_time, "Exposure time of the image",&status);
		
		
		fits_write_key(fptr, TSTRING, "IMGTYPE",imagetype,"Type of exposure", &status);
		fits_write_key(fptr, TSTRING, "LOCATION",OBSERVATORY, "Site of the telescope", &status);
		sprintf(temp_string,"%f",LOCAL_LAT);
		fits_write_key(fptr, TSTRING, "LATITUDE" ,temp_string,"Latitude of the telescope", &status);
		sprintf(temp_string,"%f",LOCAL_LONG);
		fits_write_key(fptr, TSTRING, "LONGITUD",temp_string,"Longitude of the telescope (Positive to West)", &status);
		sprintf(temp_string,"%f",LOCAL_ALT);
		fits_write_key(fptr, TSTRING, "ALTITUDE", temp_string,"Altitude of the telescope in meters", &status);
		
		if(ccd_status==CCD_IMAGE || ccd_status==CCD_FLAT)
		{
			fits_write_key(fptr, TSTRING, "FILTER" , filter_type ,
						   "Filter used on the exposure", &status);
			
			sprintf(temp_string,"%ld",p_tat_info->fli_info.focuser_curr_position);
			fits_write_key(fptr, TSTRING, "FOC_P", temp_string,"Focuser position in steps", &status);
		}
		
		if(ccd_status==CCD_IMAGE)
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
		
		fits_close_file(fptr,&status);
	}
}


