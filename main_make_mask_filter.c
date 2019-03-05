#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fitsio.h"
#include <string.h>
#include "symblc_const.h" /* here is the information about pixels*/
#include "adjust_fov.h"

st_tat_info *p_tat_info;

int main(int argc, char* argv[])
{
	char filename[100],pixel_temp[10],tempx[10],filter_type[2];
	int i,j,l,stars_count=0,s_position,f_position,limit;
	STAR star[10];
	int header_info[20]; //to send the header information

	
	if(argc == 2 && !strcmp(argv[1],"--help"))
	{
		printf("#####################################################################################\n");
		printf("Program to make a mask of a reference field of view for observation\n");
		printf("The resulting image will be read by the star-track program to adjust the FOV\n");
		printf("The resulting image will be stored in %s\n",REF_FOV_MASK);
		printf("######################################################################################\n");
		printf("Usage: make_mask [fits name] [Optional:-s <Object pixel coordinates> -f <filter>]\n\n");
		printf("\"<Object pixel coordinates>\" is the option to store the position\n");
		printf("of the center of the object stars in pixel coordinates (format:-s X,Y).\n");
		printf("\"<filter>\" is the option to declare the filter used\n");
		printf("Example: make_mask Star.fits -s 512,230 723,128 123,890 -f V\n\n");
		return 1;
	}
	if(argc==1)
	{
		printf("##############################Do not forget######################################\n");
		printf("Usage: make_mask [fits name] [Optional:-s <Object pixel coordinates> -f <filter>]\n");
		printf("#################################################################################\n");
		printf("For more help type: make_mask --help\n");
		return 1;
	}
	if(argc > 8)
	{
		printf("Error: Too many objects\n");
		return 1;
	}
	if(argc >2)//READ THE COORDINATES
	{
		//get the order of the parameters
		s_position =0;
		f_position=0;
		for(l=2;l<(argc-1);l++)
		{
			if(!strcmp(argv[l],"-s")) s_position=l;
			else if(!strcmp(argv[l],"-f")) f_position=l;
		}
		//read the filter description
		if(f_position)
		{
			if(!strcmp(argv[f_position+1],"U") || !strcmp(argv[f_position+1],"u")) 
				filter_type[0]='U';
			else if(!strcmp(argv[f_position+1],"B") || !strcmp(argv[f_position+1],"b")) 
				filter_type[0]='B';
			else if(!strcmp(argv[f_position+1],"V") || !strcmp(argv[f_position+1],"v")) 
				filter_type[0]='V';
			else if(!strcmp(argv[f_position+1],"R") || !strcmp(argv[f_position+1],"r")) 
				filter_type[0]='R';
			else if(!strcmp(argv[f_position+1],"I") || !strcmp(argv[f_position+1],"i")) 
				filter_type[0]='i';
			else if(!strcmp(argv[f_position+1],"N") || !strcmp(argv[f_position+1],"n")) 
				filter_type[0]='N';
			else
			{
				printf("Could not recognize the filter type (%s)\n",argv[f_position+1]);
				return 1;
			}
			filter_type[1]='\0';
		}
		
		if(s_position)
		{
			if(s_position < f_position) limit = f_position;
			else limit = argc;
			
			for(l=s_position+1;l<limit;l++)
			{
				for (j=0;j<=9;j++){pixel_temp[j]=0;tempx[j]=0;}
				if(strlen(argv[l])> 9)
				{
					printf("Format incorrect (%s): Length of pixels specification string is higher 9\
					For help: make_mask --help\n", argv[l]);
					return 1;
				}
				
				strcpy(pixel_temp,argv[l]);
				i=0;
				stars_count++;
				printf("Star %d : ",stars_count);
				do
				{
					tempx[i]=pixel_temp[i];
					pixel_temp[i] = ' ';
				}while(pixel_temp[++i]!= ',');
				pixel_temp[i] = ' ';
				
				star[stars_count].X = strtol(tempx,NULL,10);
				star[stars_count].Y = strtol(pixel_temp,NULL,10);
				star[stars_count].total_coord = star[stars_count].X + (1024*star[stars_count].Y);
				printf("%d %d\n",star[stars_count].X,star[stars_count].Y);
				if(star[stars_count].X<=20 || star[stars_count].X>=1000 || 
					star[stars_count].Y<=20 || star[stars_count].Y>=1000)
				{
					printf("Error (%d,%d): X or Y coordinates out of limits\n",
							star[stars_count].X,star[stars_count].Y);
							return 1;
				}
				
			}
		}
		
		if(!s_position && !f_position)
		{
			printf("Format incorrect: Only -s and -f are the accepted parameters\nFor help: make_mask --help\n");
			return 1;
		}
	}
	sprintf(filename,"%s",argv[1]);
	header_info[0]=stars_count;
	if(stars_count)
	{
		for(i=1,j=1;i<=stars_count;i++)
		{
			header_info[j] = star[i].total_coord;
			j++;
			header_info[j] = star[i].X;
			j++;
			header_info[j] = star[i].Y;
			j++;
		}
	}
	
// 	make_mask(filename,header_info); 
	make_mask(filename,header_info,filter_type); 
	return 0;
}
