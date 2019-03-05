#include "common_func.h"

//define colors in ansi C
#define ANSI_COLOR_RED       "\x1b[31m"
#define ANSI_COLOR_GREEN     "\x1b[32m"
#define ANSI_COLOR_BLUE      "\x1b[34m"
#define ANSI_COLOR_GRAY      "\x1b[37m"
#define ANSI_COLOR_RESET     "\x1b[0m"

#define TEMPSTRING_MAX_LENGTH 200
extern st_tat_info *p_tat_info;
/* 
	fexist(char*)  
*/
int fexist (char *filename)
{
	FILE *fp;
	if( (fp=fopen(filename,"r")) == NULL )
	{
		return 0;
	}
	else 
	{
		fclose(fp);
		return 1;
	}
}
/*
	faketiming
*/
void faketiming (time_t inputTime)
{
	int i;

	i=5;
	printf("timing(%ld) faketiming sleep %d\n",inputTime,i);
	sleep(i);
}
/*
	timing( [time] );  
*/
void timing (long wait2Time)
{
	char temp_string[100];
	time_t curTime;
	int diffTime;
	struct tm *ptm;
	
	/* get current and save to curTime */
	time (&curTime);
// 	ptm	= localtime (&curTime);
// 	fprintf (stdout, "curr time : %4d/%02d/%02d %02d:%02d:%02d\n",
// 			ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,
// 			ptm->tm_hour,ptm->tm_min,ptm->tm_sec);

	ptm = localtime (&wait2Time);
	sprintf (temp_string, "waiting until : %4d/%02d/%02d %02d:%02d:%02d",
			ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,
			ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
		
	step(temp_string);
	while(1)
	{
		time (&curTime);
		/* sleep if idle time is too long to save CPU cycle */
		if( ( diffTime = (wait2Time- curTime) ) > 2)
		{
			printf("Countdown %d \r",  diffTime);fflush(NULL);
			sleep(1);
		}
		else if(curTime==wait2Time)
		{
			break;
		}
		else
		{
			if( ( diffTime = (wait2Time- curTime) ) < 0)
			{
				printf("timing diffTime < 0 ");
				break;
			}
		}
	}/*while */
	printf("\n");
} /*timing*/



void doreverse(char s[])
{
	int i, j;
	char c;
	for (i = 0, j = strlen(s)-1; i < j; ++i, --j)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

void itoa(long n, char s[])
{
	long i,sign;

	if( (sign=n) < 0 ) n = -n;
	i=0;
	do
	{
		s[i++]='0'+ n%10;
	}
	while( ( n/=10) > 0);
	if( sign <0 ) s[i++]='-';
	s[i]='\0';
	doreverse(s);
}

/*
	long DoGetValueStringExt( char *argName )
	function:
		search argName from "VAR_DEF_FILENAME",and return correspondent value ;
function:
	search argName from fpname,and return correspondent value ;
	if no match,return null.
*/

long DoGetValue( char *argname)
{
	FILE *fp;
	char tempname[50];
	int i;
	long tempnum;
	/*	char dot='.';*/
	char tempString[TEMPSTRING_MAX_LENGTH];

	for(i=0;i<10;i++)
	{
		if( ( fp=fopen(VAR_DEF_FILENAME, "r") ) == NULL)
		{
			if( i == 9) return 0;
			sprintf(tempString,"Open %s failed. Try again after %d sec", 
						VAR_DEF_FILENAME, REOPEN_FILE_SECOND);
			step(tempString);
			sleep(REOPEN_FILE_SECOND);
		}
		else break;
	}

	do
	{
		fgets( tempString, TEMPSTRING_MAX_LENGTH-1, fp);
		/* printf( "tempString= %s\n", tempString);*/
		sscanf( tempString,"%s %ld", tempname, &tempnum);
		if(!strcmp( argname, tempname))
		{
			/* printf("%s\t=%d\n",argname,tempnum);*/
			fclose(fp);
			return tempnum;
		}
	}while( feof(fp) == 0 );
	
	/*
		if there aren't matched variable name, function will return NULL.
	*/
	fclose(fp);
	return (long)NULL;
}

/*
	char* DoGetValueStringExt( char *argName, char *returnValue, char *fpname )
	function:
		search argName from fpname,and store correspondent value in returnValue;
		if no match,return null.
*/
char* DoGetValueString( char *argName, char *returnValue ) 
{
	FILE *fp;

	char tempName[50];
	int i;
	char tempString[TEMPSTRING_MAX_LENGTH];

	for(i=0;i<10;i++)
	{
		if( ( fp=fopen(VAR_DEF_FILENAME, "r") ) == NULL)
		{
			if( i == 9) return 0;
			sprintf(tempString,"DoGetValueStringExt:Open %s failed. Try again after %d sec", 
						VAR_DEF_FILENAME, REOPEN_FILE_SECOND);
			step(tempString);
			sleep(REOPEN_FILE_SECOND);
		}
		else break;
	}

	do
	{
		fgets( tempString, TEMPSTRING_MAX_LENGTH-1, fp);
		sscanf( tempString,"%s %s", tempName, returnValue);
		if(!strcmp( argName, tempName))
		{
			/* printf("%s\t=%s\n",argName,returnValue);*/
			fclose(fp);
			return returnValue;
		}
	}while( feof(fp) == 0 );
	
	/*
		if there aren't matched variable name, function will return NULL.
		strcpy(returnValue,"NO_MATCHED");
		return returnValue;
	*/
    fclose(fp);
    return NULL;
}

/*
	char* DoGetValueChar( char *argName, char returnValue)
	function:
		search argName from fpname,and store correspondent value in returnValue;
		if no match,return null.
*/

char DoGetValueChar( char *argName) 
{
	FILE *fp;

	char tempName[50];
	int i;
	char returnValue = 'X';
	char tempString[TEMPSTRING_MAX_LENGTH];

	for(i=0;i<10;i++)
	{
		if( ( fp=fopen(VAR_DEF_FILENAME, "r") ) == NULL)
		{
			if( i == 9) return 0;
			
			sprintf(tempString,"DoGetValueStringExt:Open %s failed. Try again after %d sec", 
							VAR_DEF_FILENAME, REOPEN_FILE_SECOND);
			step(tempString);
			sleep(REOPEN_FILE_SECOND);
		}
		else break;
	}

	do
	{
		fgets( tempString, TEMPSTRING_MAX_LENGTH-1, fp);
		sscanf( tempString,"%s %c", tempName, &returnValue);
		if(!strcmp( argName, tempName))
		{
			/* printf("%s\t=%s\n",argName,returnValue);*/
			fclose(fp);
			return returnValue;
		}
	}
	while( feof(fp) == 0 );

	fclose(fp);
	return 'X';
}

//read the string of the input file and get all the parameters
int read_filter_string(char *filter_string, int *filter_seq, int *filter_expose, int *filter_obs)
{
	int i,j,k,l,end,N_filters;
	char str[10],temp_string[200];
	char filter_pos[FILTER_TOTAL_NUMBER];

	get_filter_array_char(filter_pos);

	for(i=0;i<FILTER_TOTAL_NUMBER;i++) //reset the slots
	{
			filter_seq[i]=70;
			filter_expose[i]=0;
	}
	
	k=-1;
	i=0;
	while(k < FILTER_TOTAL_NUMBER)
	{
		j=0;
		while(filter_string[i] !=')')
		{
			if(filter_string[i]=='\0') break;
			str[j]=filter_string[i];
			i++;
			j++;
		}
		if(filter_string[i]=='\0') break;
		i++;
		end =i;
		k++;
		str[j]='\0';
        
		if (str[0]=='D') //Choose dark current instead
		{
			filter_seq[k]=-1;
		}
		else
		{
			for(l=0;l<FILTER_TOTAL_NUMBER;l++)
			{
				if(str[0] == filter_pos[l])
				{
					filter_seq[k]=l;
					l=10; //exit loop
				}
			}
			if(l<8) 
			{
				sprintf(temp_string,"ERROR: Can not recognize %c filter",str[0]);
				step(temp_string);
				return 0; //ERROR IN READING
			}
		}
		str[0]=' ';str[1]=' ';
        //get the exposure time
		filter_expose[k]= atoi(str);
// 		printf("%d %d == >%s %d\n",k,filter_seq[k],str,filter_expose[k]);
	}

	N_filters = k+1;
	for(i=0;i<N_filters;i++)
		filter_obs[i] = filter_expose[i] + CCD_READOUT_TIME + ADJUST_TIME;

	return N_filters;
}

void get_filter_array_char(char *filters)
{
	FILE *fp;
	char tempName[50],tempString[TEMPSTRING_MAX_LENGTH],returnValue[60];
	int i,out;

	if(!(fp = fopen(VAR_DEF_FILENAME,"r")))
	{
		
		sprintf(tempString,"ERROR: Could not open %s",VAR_DEF_FILENAME);
		step(tempString);
		exit(0);
	}

	out =1;
	while(out && !feof(fp))
	{
		fgets( tempString, TEMPSTRING_MAX_LENGTH-1, fp);
		sscanf( tempString,"%s %s", tempName, returnValue);
		if(!strcmp(tempName,"FILTER_POS0"))
		{
			filters[0]= returnValue[0];
			for(i=1;i<FILTER_TOTAL_NUMBER;i++)
				fscanf( fp,"%*s %c\n",&filters[i] );
			
			out =0;
		}
	}
	fclose(fp);
}

void generate_web_image(char *image_file, int type) // 0 = CCD, 1 = MASK
{
	char out_file[100],cmd[350]; 
	FILE *fin;

	if(!type) //0 = CCD
	{
		strcpy(out_file,WEB_CCD_JPG);
		sprintf(cmd,"convert %s -flip -contrast-stretch 95%%  %s",image_file,out_file);
	}
	else //1 = MASK
	{
		strcpy(out_file,WEB_FOV_JPG);
		sprintf(cmd,"convert %s -flip -contrast-stretch 1  %s",image_file,out_file);
	}

	if((fin=fopen(image_file,"r"))!=NULL) //Check if file exists.
	{
		fclose(fin);
		system(cmd);
		sprintf(cmd,"convert %s -resize 320x320 %s",out_file,out_file);
		system(cmd);
	}
}

/*
 * Will give the correct log name for any
 */

int get_log_filename(char *filename,int type, int mode)//1 - create new, 0 -join other file
{
	char first_line[200];
	FILE *fp;
	struct tm *tm_ptr;
	time_t date;

	time(&date);
	tm_ptr=localtime(&date);
	
	switch(type)
	{
		case AUTO_OBSERVE_LOG_TYPE:
			sprintf(filename,"%sauto_%04d%02d%02d.log",AUTO_OBSERVE_LOG_PATH,
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
			sprintf(first_line,"### Log file for Auto_observation on %04d%02d%02d ###\n",
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
		break;
		case STAR_TRACK_LOG_TYPE:
			sprintf(filename,"%strack_%04d%02d%02d.log",STAR_TRACK_LOG_PATH,
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
			sprintf(first_line,"### Log file for Star_track on %04d%02d%02d ###\n",
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
		break;
		case AUTO_FLAT_LOG_TYPE:
			sprintf(filename,"%sflat_%04d%02d%02d.log",AUTO_FLAT_LOG_PATH,
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
			sprintf(first_line,"### Log file for Auto_flat on %04d%02d%02d ###\n",
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
		break;
		case AUTO_DARK_LOG_TYPE:
			sprintf(filename,"%sdark_%04d%02d%02d.log",AUTO_DARK_LOG_PATH,
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
			sprintf(first_line,"### Log file for Auto_dark on %04d%02d%02d ###\n",
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
		break;
		case AUTO_REPORT_LOG_TYPE:
			sprintf(filename,"%sReport_%s_%04d%02d%02d.txt",AUTO_REPORT_PATH,SITE,
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
			sprintf(first_line,"### Report file for Observation on %04d%02d%02d ###\n",
				tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
		break;
		case LST_DAEMON_LOG_TYPE:
			strcpy(filename,LSTDAEMON_LOG_FILENAME);
			return 1;
		break;
		default:
			return 0;
		break;
	}

	
	if((fp=fopen(filename,"r"))==NULL) //no file
	{
		if(mode) // 1 = create new 
		{
			if((fp=fopen(filename,"w"))==NULL)
			{
				printf("ERROR: Could not create file %s\n",filename);
				return 0;
			}
			fprintf(fp,first_line);
			fclose(fp);
// 			printf("Created file: %s\n",filename);
			return 1;
		}
		else //mode 0, try to join to previous day
		{
			tm_ptr->tm_mday--;
			mktime(tm_ptr);
			switch(type)
			{
				case AUTO_OBSERVE_LOG_TYPE:
					sprintf(filename,"%sauto_%04d%02d%02d.log",AUTO_OBSERVE_LOG_PATH,
						tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
				break;
				case STAR_TRACK_LOG_TYPE:
					sprintf(filename,"%strack_%04d%02d%02d.log",STAR_TRACK_LOG_PATH,
						tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
				break;
				case AUTO_FLAT_LOG_TYPE:
					sprintf(filename,"%sflat_%04d%02d%02d.log",AUTO_FLAT_LOG_PATH,
						tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
				break;
				case AUTO_DARK_LOG_TYPE:
					sprintf(filename,"%sdark_%04d%02d%02d.log",AUTO_DARK_LOG_PATH,
						tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
				break;
				case AUTO_REPORT_LOG_TYPE:
					sprintf(filename,"%sReport_%s_%04d%02d%02d.txt",AUTO_REPORT_PATH,SITE,
								tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
				break;
				default:
					return 0;
				break;
				
			}
			
			if((fp=fopen(filename,"r"))==NULL) //if no file
			{
// 				printf("No file found\n");
				return 0;
			}
			fclose(fp);
			return 1;
		}
	}
	//the file exist
	fclose(fp);
	return 1; //success
}

void steplog(char *string, int type)
{
	step(string);
	log_this(string,type,0);
}

/*
	step()
*/
void step(char *string)
{
	int i, len,len_limit;
	char color[10],time_string[10];
	time_t now;
	struct tm *tm_ptr;
	now = time (NULL);
	tm_ptr=localtime(&now);
	
	len = strlen(string);
	len_limit = len-2;
	
	sprintf(time_string,"%02d:%02d:%02d",
					tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
	
	if(string[0] == 'E' && string[1] == 'R' && string[2] == 'R')//ERROR MESSAGE
		sprintf (color,"%s",ANSI_COLOR_RED);
	else if(string[0] =='W' && string[1] == 'A' && string[2] == 'R')//WARNING MESSAGE
		sprintf(color,"%s",ANSI_COLOR_BLUE);
	else if(string[0] =='#' && string[1] == '#' && string[2] == '#')//GOOD MESSAGE
		sprintf(color,"%s",ANSI_COLOR_GREEN);
	else
		sprintf(color,"%s",ANSI_COLOR_RESET);
	
	printf("%s[%s] %s",ANSI_COLOR_GRAY,time_string,color);
	for(i=0;i<len;i++)
	{
		printf("%c",string[i]);
		if(string[i] == '\n' && i < len_limit)
			printf("%s[%s] %s",ANSI_COLOR_GRAY,time_string,color);
	}
	
	printf("%s\n",ANSI_COLOR_RESET);
	fflush(stdout);
}


void log_this(char *string, int type, int mode)
{
	char filename[200];
	FILE *fp;
	struct tm *tm_ptr;
	time_t now;

	if(get_log_filename(filename,type,mode)) //1=Success 0=failure 
	{
		if((fp=fopen(filename,"a"))!=NULL)
		{
			//get current time
			time(&now);
			tm_ptr=localtime(&now);
			
			if(type == AUTO_REPORT_LOG_TYPE || type == LST_DAEMON_LOG_TYPE)
			{
			//write in file
				fprintf(fp,"[%04d/%02d/%02d %02d:%02d] %s\n",
					tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday,
					tm_ptr->tm_hour,tm_ptr->tm_min,string);
			}
			else
			{
				fprintf (fp,"[%04d/%02d/%02d %02d:%02d:%02d]\n\t%s\n",
							tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday,
							tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec,string);
			}
			fflush (fp);
			fclose(fp);
		}
	}
}
