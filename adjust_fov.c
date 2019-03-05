#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fitsio.h"
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <mysql/mysql.h>
#include "dsp_func.h" /*to send cmd to dsp*/
#include "symblc_const.h" /* here is the information about pixels*/
#include "adjust_fov.h"
#include <curses.h>
#include "main_cmd_menu.h"


#define STAR_SPEED 	 		1./(20*120e-9*9973) /*Star speed in pulse per second*/
#define PIX_STAR_SPEED 		STAR_SPEED/PULSEPERPIX_RA  /*Star speed in pixel per second*/

////////////////////////////////////////////////////////////////////////////////
////Function to move the telescope a small amount of pulses
////////////////////////////////////////////////////////////////////////
void move_pulse(int ra_pulse, int dec_pulse)
{
	int ra_isneg, dec_isneg;
	int ra_speed,dec_speed;
	int dec_seconds,ra_seconds;
	int ra_reg,dec_reg,wait_seconds,ra_diff;
	float temp;
	char dec_cmd[200],ra_cmd[200],cmd[300];
	
	if(ra_pulse <0){ra_isneg = 1; ra_pulse *= -1;}
	else ra_isneg = 0;
	
	if(dec_pulse <0){dec_isneg = 1; dec_pulse *= -1;}
	else dec_isneg = 0;
	
	if(dec_pulse)
	{
		dec_seconds=  (int) (0.5+ (dec_pulse/1000));
		if(dec_seconds < 3) dec_seconds =3; //slower is more precise
		else if(dec_seconds > 10) dec_seconds /= 2; 
		
		dec_speed = dec_pulse/dec_seconds;
		dec_reg =(int)(0.5 + 1./(20*120.0e-9*dec_speed));

		if(dec_reg > 65534)dec_reg=65534;
		else if(dec_reg < 200)dec_reg=208;

		sprintf( dec_cmd, "DEC %s FREQUENCY 1 %d %d 0 0 0 0\n",dec_isneg?"NEG":"POS", dec_seconds ,dec_reg);

		if(!ra_pulse)//if no command for RA send the cmd right now
			send_cmd2dsp(dec_cmd);
	}
	
	if(ra_pulse)
	{
		//estimate the time it takes
		ra_seconds=  (int) (0.5+ (ra_pulse/1000));
		if(ra_seconds < 3) ra_seconds =3; //slower is more precise
		else if(ra_seconds > 10) ra_seconds /= 2; 
		
		if(ra_isneg)
			ra_speed = (ra_pulse/ra_seconds) - STAR_SPEED;
		else
			ra_speed = (ra_pulse/ra_seconds) + STAR_SPEED;
		
		if(ra_speed <0)
		{
			//Stop telescope ra_seconds seconds
			sprintf(ra_cmd, "RA POS FREQUENCY 1 1 65534 0 0 0 0\n");
		}
		else
		{
			ra_reg =(int)(0.5 + 1./(20*120.0e-9*ra_speed));
		
			if(ra_reg > 65534)ra_reg=65534;
			else if(ra_reg < 200)ra_reg=208;
		
			sprintf( ra_cmd, "RA %s FREQUENCY 1 %d %d 0 0 0 0\n",ra_isneg?"NEG":"POS",ra_seconds,ra_reg);
		}
		
		if(dec_pulse)
			sprintf(cmd,"%s\n%s\n",ra_cmd,dec_cmd);
		else
			sprintf(cmd,"%s\n",ra_cmd);

		send_cmd2dsp(cmd);
		sleep(ra_seconds);
		//Continue moving with star speed
		sprintf( ra_cmd, "RA POS FREQUENCY 1 30030 9973 0 0 0 0\n");
		send_cmd2dsp(ra_cmd);
	}
}




////////////////////////////////////////////////////////////////////////////////
////Function to move the telescope a certain number of pixels in RA and DEC
////////////////////////////////////////////////////////////////////////
// 		printf("Initial DEC seconds %d\n",dec_seconds);
void move_pixel(int RA, int DEC,int mode)//Move RA and DEC)
{
	int ra_sign,dec_sign;
	int ra_speed,dec_speed;
	int dec_seconds;
	int ra_reg,dec_reg,wait_seconds,ra_pixel;
	float temp;
	char dec_cmd[200],ra_cmd[200],cmd[300];
	FILE *daemon;

	if(mode) printf("Moving telescope RA: %d and DEC:  %d pixels\n",RA,DEC);
	else mvprintw(MSG_LINE, 0, "Moving telescope RA: %d and DEC:  %d pixels\n",RA,DEC);

	if(DEC > 1 && DEC < 4 ) dec_seconds =1; //To avoid a register bigger than 65535
	else dec_seconds =2;
	
	if(RA < 0){ra_sign = 1; RA *= -1;}
	else ra_sign=0;

	if(DEC < 0){dec_sign = 0; DEC *=-1;}
	else dec_sign=1;

	//MOVE DEC Easiest first :)
	if(DEC)
	{
// 		dec_speed = (DEC*PULSEPERPIX_DEC)/2; //speed to move the telescope that amount of pixels in 2 sec
		dec_speed = (DEC*PULSEPERPIX_DEC)/dec_seconds;
		dec_reg =(int)(0.5 + 1./(20*120.0e-9*dec_speed));
		if(dec_reg > 65534)dec_reg=65534;
		else if(dec_reg < 200)dec_reg=208;
// 		sprintf( dec_cmd, "DEC %s FREQUENCY 1 2 %d 0 0 0 0\n",!dec_sign?"POS":"NEG", dec_reg);
		sprintf( dec_cmd, "DEC %s FREQUENCY 1 %d %d 0 0 0 0\n",!dec_sign?"POS":"NEG", dec_seconds ,dec_reg);
		if(!RA)//if no command for RA send the cmd right now
		{
			daemon=fopen(DSP_CMD_FILENAME,"w");
			fprintf(daemon,"%s",dec_cmd);
			fclose(daemon);
		if(mode) printf("Sent to DSP ==> %s\n",dec_cmd);
		}
	}

	if(RA)
	{
		if(!ra_sign)//move EAST
		{
 			ra_speed = (RA*PULSEPERPIX_RA)/3; //speed to move the telescope that amount of pixels in 3 sec
			ra_speed += STAR_SPEED;
			ra_reg =(int)(0.5 + 1./(20*120.0e-9*ra_speed));
			if(ra_reg > 65534)ra_reg=65534;
			else if(ra_reg < 200)ra_reg=208;
			sprintf( ra_cmd, "RA POS FREQUENCY 1 3 %d 0 0 0 0\n",ra_reg);
			if(DEC)
				sprintf(cmd,"%s\n%s\n",ra_cmd,dec_cmd);
			else
				sprintf(cmd,"%s\n",ra_cmd);

			daemon=fopen(DSP_CMD_FILENAME,"w");
			fprintf(daemon,"%s",cmd);
			fclose(daemon);
			if(mode) printf("Sent to DSP ==> %s\n",cmd);

			sleep(3);
			sprintf( ra_cmd, "RA POS FREQUENCY 1 30030 9973 0 0 0 0\n");

			daemon=fopen(DSP_CMD_FILENAME,"w");
			fprintf(daemon,"%s",ra_cmd);
			fclose(daemon);
			if(mode) printf("Sent to DSP ==> %s\n",ra_cmd);
		}
		else//move WEST
		{
			temp = PIX_STAR_SPEED;
			//compute how many pixels do we at least have to wait
			wait_seconds= (int)(0.5+ RA/temp) +1;
			//calculate de difference long wait and the pixel
			ra_pixel = (int) (0.5+( PIX_STAR_SPEED * wait_seconds - RA));
			//move to that amount of pixel to wait
			ra_speed = (ra_pixel*PULSEPERPIX_RA/3);
			ra_reg =(int)(0.5 + 1./(20*120.0e-9*ra_speed));
			if(ra_reg > 65534)ra_reg=65534;
			else if(ra_reg < 200)ra_reg=208;
			sprintf( ra_cmd, "RA POS FREQUENCY 1 3 %d 0 0 0 0\n",ra_reg);

			if(DEC)
				sprintf(cmd,"%s\n%s\n",ra_cmd,dec_cmd);
			else
				sprintf(cmd,"%s\n",ra_cmd);

			daemon=fopen(DSP_CMD_FILENAME,"w");
			fprintf(daemon,"%s",cmd);
			fclose(daemon);
			if(mode) printf("Sent to DSP ==> %s\n",cmd);
void move_degrees(float move_ra, float move_dec)
{
	int ra_sign,dec_sign;
	
	ra_sign =1;
	dec_sign =1;
	
	if(move_ra < 0) {ra_sign = -1;move_ra *= -1;}
	if(move_dec < 0) {dec_sign = -1;move_dec *= -1;}
	
	
	
}
			sleep(wait_seconds);//wait for the star
			sprintf( ra_cmd, "RA POS FREQUENCY 1 30030 9973 0 0 0 0\n");

			daemon=fopen(DSP_CMD_FILENAME,"w");
			fprintf(daemon,"%s",ra_cmd);
			fclose(daemon);
			if(mode) printf("Sent to DSP ==> %s\n",ra_cmd);
		}
	}
}



float sorting(float *array, int N)
{
	int pos;
	int i,j;
	float sigma,min;

	for(i=0;i<N-1;i++)
	{
		min = array[i];
		pos =i;
		for(j=i;j<N;j++)
		{
			if(min > array[j])
			{min=array[j];pos=j;}
		}
        array[pos]=array[i];
        array[i]=min;
    }
	sigma = abs(array[1]-array[N-1])/2;

	return sigma;
}


int masquerade(float *image_array, float *background)
//out- 0= no problem, 1-too bright , 2- few pixel amount, 3- few saturated pixels
{
	int i,j,k;
	int x_coord,y_coord, total_coord;
	int nonzero_pixel_count =0,sat_pixel_count=0;
	FILE *fin,*fout;
	int z[1024][1024];
	float sigma,median,neighbor[9],histogram[66000]={0};
	double temp,avg,sum;
	int num_pixels = 1024*1024;

	for(x_coord=0;x_coord<1024;x_coord++)
	{
		for(y_coord=0;y_coord<1024;y_coord++)
		{
			total_coord=(y_coord*1024)+x_coord;
			z[x_coord][y_coord] = image_array[total_coord];
		}
	}

	for(x_coord=1;x_coord<1023;x_coord++)//Eliminate hot pixels
	{
		for(y_coord=1;y_coord<1023;y_coord++)
		{
			for(i=-1,k=0;i<=1;i++)
			{
				for(j=-1;j<=1;j++,k++) neighbor[k]=z[x_coord+i][y_coord+j];
			}
			sigma = sorting(neighbor,k);
			//sigma *=sigma;
			if((z[x_coord][y_coord] > neighbor[5] + sigma)||
				(z[x_coord][y_coord] < neighbor[5] - sigma))
				z[x_coord][y_coord] = neighbor[5];

		}
	}

	for(x_coord=0,sum=0;x_coord<1024;x_coord++) //To compute the average pixel count
	{
		for(y_coord=0;y_coord<1024;y_coord++)
			sum += z[x_coord][y_coord];
	}
	avg = sum/(double)num_pixels;

	if(avg>MAX_AVG) return 1;//Exposure too bright

		// 		//Calculate  sigma
	for(x_coord=0,sigma=0;x_coord<1024;x_coord++) //To compute the average pixel count
	{
		for(y_coord=0;y_coord<1024;y_coord++)
			sigma += (avg -z[x_coord][y_coord]) * (avg -z[x_coord][y_coord]);
	}

	sigma = sqrt(sigma / num_pixels);

	//printf("Average = %f, sigma = %f\n",avg,sigma);

// 	avg+=SIGMA_FACTOR*sigma;
	*background= avg +(SIGMA_FACTOR*sigma);
	for(x_coord=0;x_coord<1024;x_coord++) //do not consider the border of 4 pixels
	{
		for(y_coord=0;y_coord<1024;y_coord++)
		{
			if(z[x_coord][y_coord]<*background || x_coord <=4 || x_coord>=1020 || y_coord <=4 || y_coord>=1020 )
				z[x_coord][y_coord]=0;
// 				else
// 					z[x_coord][y_coord]=1;
		}
	}

	for(i=1;i<=10;i++) // Repeat the cleaning process many times
	{
		for(x_coord=4;x_coord<=1020;x_coord++) //remove hot pixels
		{
			for(y_coord=4;y_coord<=1020;y_coord++)
			{
				if(z[x_coord][y_coord])
				{
					if(!((z[x_coord+1][y_coord] ||  z[x_coord-1][y_coord]) && (z[x_coord][y_coord+1] || z[x_coord][y_coord-1])))
						z[x_coord][y_coord]=0;
					else if(!(z[x_coord+2][y_coord] ||  z[x_coord-2][y_coord] || z[x_coord][y_coord+2] || z[x_coord][y_coord-2]))
						z[x_coord][y_coord]=0;
// 						else if(!(z[x_coord+3][y_coord] ||  z[x_coord-3][y_coord] || z[x_coord][y_coord+3] || z[x_coord][y_coord-3]))
// 							z[x_coord][y_coord]=0;
// 						else if(!(z[x_coord+4][y_coord] ||  z[x_coord-4][y_coord] || z[x_coord][y_coord+4] || z[x_coord][y_coord-4]))
// 							z[x_coord][y_coord]=0;
				}
			}
		}
	}

	for(x_coord=0,nonzero_pixel_count=0,sat_pixel_count=0;x_coord<1024;x_coord++) //copy the value for the output
	{
		for(y_coord=0;y_coord<1024;y_coord++)
		{
			if(z[x_coord][y_coord]>SATURATED)sat_pixel_count++;
			if(z[x_coord][y_coord])	nonzero_pixel_count++;

			total_coord=(y_coord*1024)+x_coord;
			image_array[total_coord]=z[x_coord][y_coord];
		}
	}

	if(nonzero_pixel_count<MIN_PIXEL) return 2; //Too few pixel number
	else if(sat_pixel_count) return 3;
	else	return 0; //No problem
}

void write_shift_data(SHIFT db)
{
	pid_t pid;
	pid = fork();
	if(pid==0) //child proccess
	{
		MYSQL *conn;
		MYSQL_RES *res;
		MYSQL_ROW row;

		char *server = "localhost";
		char *user = "writer";
		char *password = "secret"; /* set me first */
		char *database = "fov";
		char insert[] = "insert into Shift(Timestamp,Status,RA,Declination,R,Frame) ";
		char insert2[300];
		time_t tm_now;
		time(&tm_now);
		db.timestamp= tm_now; 

		sprintf(insert2," values(%d,'%s',%d,%d,%.7f,'%s')",db.timestamp,
			db.status,db.ra,db.dec,db.R,db.frame);
		strcat(insert,insert2);

		conn = mysql_init(NULL);
		 /* Connect to database */
		if (!mysql_real_connect(conn, server,user, password, database, 0, NULL, 0)) 
		{
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(1);
		}
		//Insert data
		if(mysql_query(conn,insert))
		{
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(1);
		}
		res = mysql_use_result(conn);
//		while ((row = mysql_fetch_row(res)) != NULL)
//			printf("%s \n", row[0]);

		   /* close connection */
		mysql_free_result(res);
		mysql_close(conn);
		exit(0);	
	}
	else //Father 
	{
		wait(&pid);
	}
}
////////////////////////
///ASTROMETRY PROGRAM
////////////////////////

int get_RA_DEC_astrometry(char *filename, float *ra_astro, float *dec_astro)
{
	int i,success=0,N_index=0,indexes[4];
	char cmd[BUFF_SIZE],buf[BUFF_SIZE],index_parameter[500],filename_noext[300];
	FILE *fp;
	float ra=-1,dec=0,scale;
	pid_t pid;

	
 /* pointer to the FITS file, defined in fitsio.h */
	int status=0;
	
	sprintf(index_parameter," -i '%s/index*.fits'",INDEX_PATH);
	ra = *ra_astro;
	dec = *dec_astro;
	fitsfile *fptr;      
	
	if(ra > -1)// There are initial ra and dec
	{
		N_index = deg_to_index(ra,dec,indexes);
		//printf("N_index =%d\n",N_index);
// 		for(i=0;i<N_index;i++) printf("%d %d\n",i,indexes[i]);
		if(N_index) 
		{
			sprintf(index_parameter," ");
		
			for(i=0;i<N_index;i++)
			{
				sprintf(index_parameter,"%s -i '%s/index*-%02d.fits'",index_parameter,INDEX_PATH,indexes[i]);
			}
			
			//printf("Using specific index files:\n%s\n",index_parameter);
		}
	}
	

	if((fp=fopen(filename,"r"))==NULL)
	{
		printf("ERROR: File %s not found\n",filename);
		return 1;
	}
	fclose(fp);
	
	//get filename without extension
	i=0;
	success=0;
	while(!success)
	{
		if(filename[i]== '.' &&  filename[i+1]== 'f' &&
			filename[i+2]== 'i' &&  filename[i+3]== 't')
		{
			success =1;
			filename_noext[i] ='\0';
		}
		else
		{
			filename_noext[i] = filename[i];
			i++;
		}
	}
	
	pid=fork();
	if(pid==-1)
	{
		printf("ERROR: Fork() failed.\n");
		return -1;
	}       
	else if(!pid) // Child
	{
		sprintf(cmd,"%s %s %s %s>%s",FIELD_PROGRAM,filename,FIELD_OPTIONS,AUG_FILE,INFILE);
		system(cmd);
		
		if((fp=fopen(AUG_FILE,"r"))==NULL)
		{
			printf("ERROR: %s did not create  %s file\n",FIELD_PROGRAM,AUG_FILE);
			return 1;
		}
		fclose(fp);
		
		sprintf(cmd,"%s -C %s -s %s %s %s > %s",ASTRO_PROGRAM,
				ASTRO_CANCEL_FILE,ASTRO_SOLVED_FILE,index_parameter,AUG_FILE,INFILE);
		system(cmd);

		exit(0);
	}    //parent from here
	
	i=0;
	success =0;
	while(i<WAIT)
	{
		if((fp=fopen(ASTRO_SOLVED_FILE,"r"))!=NULL)
		{
			fclose(fp);
			success=1;
			break;
		}
		i++;
		sleep(1);
	}

	if(!success) //
	{
		printf("ERROR: Field not solved after timeout\n");
		//create cancel file;
		fp=fopen(ASTRO_CANCEL_FILE,"w");
		fclose(fp);
		
		sleep(2);
		//Kill and collect
		kill(pid, SIGKILL);
		sleep(2);
		waitpid(pid,(int *)0,0); //
		
		remove_astrometry_files(filename_noext);
		return 1;
	}
	
	waitpid(pid,(int *)0,0);
	
	if((fp=fopen(INFILE,"r"))==NULL)
	{
		printf("ERROR: File %s not found\n",INFILE);
		remove_astrometry_files(filename_noext);
		return 1;
	}
	
	while(fgets(buf, BUFF_SIZE, fp))
	{
		if(buf[2]=='R' && buf[3]=='A') // get the correct line
		{
				i=2;
				while(buf[i]!='(')buf[i++]=' ';//remove the first char

				buf[i]=' ';

				sscanf(buf,"%f,%f), %*s %*s %f",&ra,&dec,&scale);
// 				printf("%s\n",buf);
				break;
		}

	}
	fclose(fp);
	remove_astrometry_files(filename_noext);
	ra /= 15.0;
// 	printf("RA=%f h, DEC=%f d\t pixel scale = %f\n",ra,dec,scale);
	*ra_astro = ra;
	*dec_astro = dec;
	
	//Insert the information in the fit file.
	status=0;
	fits_open_file(&fptr, filename, READWRITE,&status);
	if(status)
	{
		printf("ERROR : Cannot open fits file\n");
		return 1;
	}
	else
	{
		fits_update_key(fptr, TFLOAT, "ASTR-RA" ,&ra,
					"R.A. in hours of image from Astrometry", &status);
		fits_update_key(fptr, TFLOAT, "ASTR-DEC" ,&dec,
					"Dec. in deg of image from Astrometry", &status);
		fits_update_key(fptr, TFLOAT, "ASTR-SCL" ,&scale,
					"Pixel scale of imgae from Astrometry", &status);

		fits_close_file(fptr,&status);
	}
	return 0;
}

void remove_astrometry_files(char *filename)
{
	char cmd[2000];
	
	sprintf(cmd,"rm %s.match %s.corr %s.rdls",filename,filename,filename);
	system(cmd);
	remove(AUG_FILE);
	remove(ASTRO_SOLVED_FILE);
	remove(ASTRO_CANCEL_FILE);
	remove(INFILE);
	
}

int deg_to_index(float RA, float DEC, int *indexes)
{
	int N_index=0,repeated;
	int i,j,l,index=0;
	int new_arr_len=0;
	float scope[4][2]={{RA+0.73,DEC},{RA-0.73,DEC},{RA,DEC+0.73},{RA,DEC-0.73}};
	float PI = 3.14159265358979323846;
	
	for(j=0; j < 4 ; j++)
	{
		for(i=0; i < 4 ; i++)
		{
// 			printf("scope = %f\n",scope[j][0]);
			if (90*i <= scope[j][0] && scope[j][0]  < 90*(i+1))
			{
				if (scope[j][1] >= fabs(asin(2.0/3.0)*180.0/PI/45.0*scope[j][0]-(2.0*(double)i+1.0)*asin(2.0/3.0)*180.0/PI))
					index=i%4;
				else if (scope[j][1] <= -fabs(asin(2.0/3.0)*180/PI/45.0*scope[j][0]-(2*(double)i+1)*asin(2.0/3.0)*180/PI))
					index=8+i%4;
				else
				{
					if (scope[j][0] <= 45+i*90)
						index=4+i%4;
					else
						index=4+(i+1)%4;
				}
// 				printf("INDEX = %d\n",index);
			}
		}
		
		repeated =0;
		for(l=0; l < N_index ; l++)
		{
			if(indexes[l] == index) repeated++;
		}
			
		if(!repeated)
		{
			indexes[N_index]=index;
			N_index++;
		}
		
// 		printf("%d: %d %d\n",j,index,N_index);
	}

	//Check the indexes
	return N_index;
}
