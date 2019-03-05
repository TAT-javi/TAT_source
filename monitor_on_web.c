#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include <time.h>
#include "ppcdaemon.h"
#include "dsp_func.h"
#include "symblc_const.h"
#include "pwrdaemon.h"

st_tat_info    *p_tat_info;
char obs_color[15],tel_color[15],pwr_color[15],ppc_color[15],ccd_color[15];




void draw_obs_status( st_obs_info* p_object);
void draw_tel_status( st_dsp_info* p_object);
void draw_ccd_status( st_ccd_info* p_object);
void draw_enc_status( st_dsp_info* p_object);
void draw_ppc_status( st_ppc_info* p_object);
void draw_pwr_status( st_ppc_info* p_object);
void get_last_shift (void);

int main()
{
	/* query */
	char *query, *ptr, *varname, *eq, *value,trash[100];
	char cmd[300];
	int i,refresh=10;
	int shift_file_found;

	sprintf(tel_color,"\"#F88017\"");
	sprintf(ppc_color,"\"#b93b8f\"");
	sprintf(pwr_color,"\"#0000FF\"");
	sprintf(ccd_color,"\"#800000\"");
	
// 	query = getenv ("QUERY_STRING");
// 	ptr=query;
// 	if(ptr)
// 	{
// 		varname=ptr;
// 		eq=strchr(varname, '=');
// 		if(eq)
// 		{
// 			*eq='\0';
// 			value=eq+1;
// 			if(strcmp(varname,"refresh") == 0)
// 				refresh = atoi(value);
// 		}
// 	}
	printf("Content-type: text/html;charset=utf-8\n\n");
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);

	printf("<html><head>");
	printf("<title>TAT Monitor</title>");
	printf("<META HTTP-EQUIV=\"refresh\" CONTENT=\"60\">");
	printf("</head><body bgcolor=#e0e0f8>");
	printf("<table border=\"0\"><td><table border=\"1\">");

	if (p_tat_info->obs_info.ccd_status == CCD_IMAGE)
		printf( "<tr><td colspan=4><H1><font color=\"#ff0000\">***CCD is taking Exposures***</font></H1>");
	else if (p_tat_info->obs_info.ccd_status == CCD_FLAT || p_tat_info->obs_info.ccd_status == CCD_DOING_FLAT)
		printf( "<tr><td colspan=4><H1><font color=\"#ff0000\">***CCD is taking Flat Field***</font></H1>");
	else if (p_tat_info->obs_info.ccd_status == CCD_DARK)
		printf( "<tr><td colspan=4><H1><font color=\"#ff0000\">***CCD is taking Dark Current***</font></H1>");
	printf( "<tr><td colspan=4>");

	draw_obs_status(&(p_tat_info->obs_info));
	draw_tel_status(&(p_tat_info->dsp_info));
	draw_ccd_status(&(p_tat_info->ccd_info));
	draw_enc_status(&(p_tat_info->dsp_info));
	draw_ppc_status(&(p_tat_info->ppc_info));
	draw_pwr_status( &(p_tat_info->pwr_info));
	printf("</td></table></td><td>");
	printf("<table border=\"1\"> ");
	printf("<tr><td colspan=2><img src=\"total_shift.php\"> <br></td>");
	printf("<tr><td colspan=2><img src=\"local_shift.php\"> <br></td>");
 
	printf("<tr><td><p style=\"text-align:center\">Current Mask</p></td>");	
	printf("<td><p style=\"text-align:center\">Recent Image</p></td>");
	
// 	sprintf(cmd,"convert %s -flip -contrast-stretch 1  %s",REF_FOV_MASK,WEB_FOV_JPG);
// 	system(cmd);
// 	sprintf(cmd,"convert %s -resize 320x320 %s",WEB_FOV_JPG,WEB_FOV_JPG);
// 	system(cmd);
		
// 	printf("<tr><td><img src='http://%s/current_FOV.jpg'> </td>",IP);
	printf("<tr><td><img src=\"current_FOV.jpg\"> </td>");
	
	if(p_tat_info->obs_info.recent_image)
	{
		sprintf(cmd,"convert %s -flip -contrast-stretch 95%%  %s",p_tat_info->obs_info.recent_image,WEB_CCD_JPG);
		system(cmd);
		sprintf(cmd,"convert %s -resize 320x320 %s",WEB_CCD_JPG,WEB_CCD_JPG);
		system(cmd);
		
// 		printf("<td><center><img src='http://%s/recent_ccd_image.jpg'> </center></td>",IP);
		printf("<td><center><img src=\"recent_ccd_image.jpg\"> </center></td>");
	}
	
	printf("</table></td></table></body></html>");
	return 0;
}


void draw_obs_status( st_obs_info* p_object)
{
	int i,j;
	struct tm tm_time;
	char time_string[17];
	char temp_string[200],filter_string[100];
	time_t	now;
	char filters[6]={'N','B','V','R','A','C'};
	
// 	get_filter_array_char(filters); No permission
	
	
	if(!p_tat_info->obs_info.auto_observing)
		sprintf(obs_color,"\"#510450\"");
	else
		sprintf(obs_color,"\"#00CC00\"");
		
	printf( "<tr><td><font color=%s>Auto Observing</font><td>%-3s", obs_color,p_tat_info->obs_info.auto_observing?"YES":"NO");
	switch( p_tat_info->obs_info.status )
	{
		case Pursuing:
			sprintf( temp_string, "Pursuing");
		break;
		case Tracking:
			sprintf( temp_string, "Tracking");
		break;
		case Returning:
			sprintf( temp_string, "Returning");
		break;
		case STOP:
			sprintf( temp_string, "STOP");
		break;
		case InOrigin:
			sprintf( temp_string, "InOrigin");
		break;
		case Moving:
			sprintf( temp_string, "Moving");
		break;
		case Flating:
			sprintf( temp_string, "Flating   ");
		break;
		case Darking:
			sprintf( temp_string, "Darking   ");
		break;

		default:
			sprintf( temp_string, "N.A.");

		break;
	}
	printf( "<td><font color=%s>Status</font><td>%s",obs_color, temp_string);

	time(&now);
	gmtime_r(&now, &tm_time);
	strftime( time_string, 17, "20%y/%m/%d %H:%M", &tm_time);
	printf( "<tr><td colspan=2><p style=\"text-align:center\"><font color=%s>Current Time  (UTC)</font> </p></td>",obs_color);
	printf( "<td colspan=2><p style=\"text-align:center\">%15s</p>",time_string);
	
	gmtime_r(&(p_object->begin_time), &tm_time);
	strftime( time_string, 17, "20%y/%m/%d %H:%M", &tm_time);
	printf( "<tr><td><font color=%s>Begin Time  (UTC) </font><td>%15s",obs_color, time_string);

	gmtime_r(&(p_object->end_time), &tm_time);
	strftime( time_string, 17, "20%y/%m/%d %H:%M", &tm_time);
	printf( "<td><font color=%s>End Time (UTC) </font><td>%15s",obs_color, time_string);

	printf( "<tr><td><font color=%s>Curr. Expose Time </font><td>  %.0f",obs_color, p_object->expose_time);
	printf( "<td><font color=%s>Curr. Filter </font><td>  %c",obs_color, p_object->filter_type);
	
	sprintf(filter_string,"");
	for(i=0;i<p_object->N_filters;i++)
	{
		j = p_object->filter_seq[i];
		sprintf(filter_string,"%s %c(%d)",
				filter_string,filters[j],p_object->filter_exp_time[i]);
	}
	printf( "<tr><td colspan=2><p style=\"text-align:center\"><font color=%s>Filter Sequence </font>",obs_color);
	printf( "<td colspan=2><p style=\"text-align:center\">%15s</p><tr>",filter_string);
	

	switch( p_tat_info->obs_info.FOV )
	{
		case FOV_CORRECT:
			sprintf( temp_string, "<font color=\"#254117\">Correct");
		break;
		case FOV_TBC:
			sprintf( temp_string, "<font color=\"#0000A0\">Checking");
		break;
		case FOV_INCORRECT:
			sprintf( temp_string, "<font color=\"#ff0000\"> Incorrect");
		break;
		case FOV_ERROR:
			sprintf( temp_string, "<font color=\"#ff0000\">Error");
		break;
		default:
			sprintf( temp_string, "<font color=\"#ff0000\">N.A.");
		break;
	}
	printf( "<td colspan=2><p style=\"text-align:center\"><font color=%s>FOV </font> <td colspan=2> <p style=\"text-align:center\"> %s ",obs_color,temp_string);
	get_last_shift ();
	printf( "</font>");
}


void draw_tel_status( st_dsp_info *p_object)
{
	static char *msg[]=	{
				 "NA",
				 /* 1     2 */
				 "POS","NEG",
				 /*    3          4      5        6            7 */
				 "RETURN","InOrigin","STOP","MOVING","NearOrigin",
				 /*    8          9      10 */
				 "DISABLE", "ENABLE", "DONE",
				 /*11   12  */
			    	 "NO","YES",0
			    	 };
	static char option;


	printf( "<tr><td colspan=2><font color=\"#F88017\">DSP Time Count</font>=%-6d",p_object->time_count);

        /* RA */
    printf( "<tr><td><font color=%s>RA Reg</font>=%-6d",tel_color,p_object->ra.reg);
    printf( "<td><font color=%s>Pulse</font>=%-8d", tel_color,p_object->ra.pulse);

	switch( p_object->ra.dir )
	{
		case 1:
			option=1;
			break;
		case 0:
			option=2;
			break;
		default:
			option=0;
	}
	printf( "<td><font color=%s>Dir</font>=%-3s",tel_color,msg[option]);

	switch( p_object->ra.status )
	{
		case 10:
			option=3;
			break;
		case 11:
			option=4;
			break;

		case 12:
			option=5;
			break;

		case 13:
			option=6;
			break;
	case 14:
		option=7;
		break;
	default:
		option=0;
	}
	printf( "<td><font color=%s>Status</font>=%-10s",tel_color,msg[option]);

	switch( p_object->ra.pulsectl )
	{
		case 0:
			option=8;
			break;
	case 1:
		option=9;
		break;
	case 2:
		option=10;
		break;
	default:
		option=0;
	}
        printf( "<tr><td><font color=%s>RA PulseCtl</font>=%-7s",tel_color,msg[option]);
        printf( "<td><font color=%s>MovePulse</font>=%-8d",tel_color,p_object->ra.refpulse);
        printf( "<td><font color=%s>Remainder</font>=%-8d",tel_color,p_object->ra.purpulse);

	switch( p_object->ra.origin )
	{
		case 0:
			option=11;
			break;
		case 1:
			option=12;
			break;
		default:
			option=0;
	}
	printf( "<td><font color=%s>In Origin</font>=%-3s",tel_color,msg[option]);

	/* DEC */
	printf( "<tr><td><font color=%s>DEC Reg</font>=%-6d",tel_color,p_object->dec.reg);
	printf( "<td><font color=%s>Pulse</font>=%-8d",tel_color, p_object->dec.pulse);

	switch( p_object->dec.dir )
	{
		case 1:
			option=1;
			break;
		case 0:
			option=2;
			break;
		default:
			option=0;
	}
	printf( "<td><font color=%s>Dir</font>=%-3s",tel_color,msg[option]);

	switch( p_object->dec.status )
	{
		case 10:
			option=3;
			break;
		case 11:
			option=4;
			break;

		case 12:
			option=5;
			break;

		case 13:
			option=6;
			break;
		case 14:
			option=7;
			break;
		default:
			option=0;
	}
	printf( "<td><font color=%s>Status</font>=%-10s",tel_color,msg[option]);

	switch( p_object->dec.pulsectl )
	{
		case 0:
			option=8;
			break;
		case 1:
			option=9;
			break;
		case 2:
			option=10;
			break;
		default:
			option=0;
	}
	printf( "<tr><td><font color=%s>DEC PulsCtl</font>=%-7s",tel_color,msg[option]);
	printf( "<td><font color=%s>MovePulse</font>=%-8d",tel_color,p_object->dec.refpulse);
	printf( "<td><font color=%s>Remainder</font>=%-8d",tel_color,p_object->dec.purpulse);

	switch( p_object->dec.origin )
	{
		case 0:
			option=11;
			break;
		case 1:
			option=12;
			break;
		default:
			option=0;
	}
	printf( "<td><font color=%s>In Origin</font>=%-3s",tel_color,msg[option]);


// 	printf( "<tr><td>ROT move=%-6d",p_object->rot.move);
// 	printf( "<td>mode=%-2d",p_object->rot.mode);
// 	printf( "<td>TRANS move=%-4d",p_object->tran.move);
// 	printf( "<td>mode=%-2d",p_object->tran.mode);
	printf( "<tr><td><font color=%s>WHEEL filter</font><td>%-4d",tel_color,p_object->wheel.mode);
}


void draw_ccd_status( st_ccd_info* p_object)
{
	char temp_string[BUFSIZ];

	printf( "<tr><td><font color=%s>Current point</font> <td> %-7.5f", ccd_color,p_object->curr_point);
	printf( "<td><font color=%s>Set point </font><td> %-7.5f", ccd_color,p_object->set_point);

// 		#define	Apn_Status_Idle	 0
// 	#define	Apn_Status_Exposing  1
// 	#define	Apn_Status_ImagingActive  2
// 	#define	Apn_Status_ImageReady  3
// 	#define	Apn_Status_Flushing  4
// 	#define	Apn_Status_WaitingOnTrigger 5
// 	#define	Apn_Status_ConnectionError 6
	switch( p_object->camera_status)
	{
		case 0:
			strcpy( temp_string,"Idle");
		break;
		case 1:
			strcpy( temp_string,"Exposing");
		break;
		case 2:
			strcpy( temp_string,"ImagingActive");
		break;
		case 3:
			strcpy( temp_string,"ImageReady");
		break;
		case 4:
			strcpy( temp_string,"Flushing");
		break;
		case 5:
			strcpy( temp_string,"WaitingOnTrigger");
		break;
		case 6:
			strcpy( temp_string,"ConnectionError");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	printf( "<tr><td><font color=%s>CCD status </font><td> %-20s", ccd_color,temp_string);

// #define	Apn_CoolerStatus_Off 0
// 	#define	Apn_CoolerStatus_RampingToSetPoint 1
// 	#define	Apn_CoolerStatus_AtSetPoint 2
// 	#define	Apn_CoolerStatus_Revision 3
	switch( p_object->cooler_status)
	{
		case 0:
			strcpy( temp_string,"Off");
		break;
		case 1:
			strcpy( temp_string,"RampingToSetPoint");
		break;
		case 2:
			strcpy( temp_string,"AtSetPoint");
		break;
		case 3:
			strcpy( temp_string,"Revision");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	printf( "<td><font color=%s>Cooler status </font><td> %-20s", ccd_color,temp_string);
// 	#define	Apn_FanMode_Off	0
// 	#define	Apn_FanMode_Low 1
// 	#define	Apn_FanMode_Medium 2
// 	#define	Apn_FanMode_High 3
	switch( p_object->fan_status)
	{
		case 0:
			strcpy( temp_string, "Off");
		break;
		case 1:
			strcpy( temp_string, "Low");
		break;
		case 2:
			strcpy( temp_string, "Medium");
		break;
		case 3:
			strcpy( temp_string, "High");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	printf( "<tr><td><font color=%s>Fan status </font><td> %-20s",ccd_color, temp_string);

	printf( "<tr><td colspan=4><font color=%s>Recent:</font> %s",ccd_color, 
			*(p_tat_info->obs_info.recent_image)?p_tat_info->obs_info.recent_image:"None");
}

void draw_enc_status( st_dsp_info* p_object)
{
	static char temp_string[BUFSIZ];

	if( p_object->enc.open )
		sprintf( temp_string, "Opening");
	else if( p_object->enc.close )
		 sprintf( temp_string, "Closing");
	else
		sprintf( temp_string, "No Action");

	printf( "<tr><td><font color=%s>Enc </font>%-9s", tel_color,temp_string);
	printf( "<td><font color=%s>Time</font>=%-3d",tel_color,p_object->enc.time);

	if( p_object->latch.lock )
		sprintf( temp_string, "Locking");
	else if( p_object->latch.unlock )
		sprintf( temp_string, "Unlocking");
	else
		sprintf( temp_string, "No Action");

	printf( "<td><font color=%s>Latch </font>%-9s",tel_color,temp_string);
	printf( "<td><font color=%s>Time</font>=%-3d",tel_color,p_object->latch.time);
}

void draw_ppc_status( st_ppc_info* p_object)
{
	printf( "<tr><td><font color=%s>MAIN POWER</font><td>%3s",ppc_color,(p_object->control)&MAINPWRREG?"OFF":"ON");

	printf( "<tr><td><font color=%s>RA POWER</font><td>%3s",ppc_color, (p_object->data)&RAPWRREG?"OFF":"ON");
	printf( "<td><font color=%s>DEC POWER</font><td>%3s",ppc_color,(p_object->data)&DECPWRREG?"OFF":"ON");

// 	printf( "<tr><td>ROT POWER<td>%3s",(p_object->data)&ROTPWRREG?"OFF":"ON");
// 	printf( "<td>TRAN POWER<td>%3s",(p_object->data)&TRANPWRREG?"OFF":"ON");

	printf( "<tr><td><font color=%s>DSP POWER</font><td>%3s",ppc_color,(p_object->data)&DSPPWRREG?"OFF":"ON");
	printf( "<td><font color=%s>CCD POWER</font><td>%3s",ppc_color,(p_object->data)&CCDPWRREG?"OFF":"ON");

	printf( "<tr><td><font color=%s>WHEEL POWER</font><td>%3s",ppc_color,(p_object->data)&WHEELPWRREG?"OFF":"ON");
	printf( "<td><font color=%s>VDC POWER</font><td>%3s",ppc_color,(p_object->control)&VDCPWRREG?"OFF":"ON");

	printf( "<tr><td><font color=%s>DSP JUMPER</font><td>%3s",ppc_color,(p_object->data)&DSPJPREG?"OFF":"ON");

	printf( "<td colspan=2><font color=%s>STATUS </font>%d%d%d%d %d",ppc_color,
		p_object->status&RAEASTREG?1:0,
		p_object->status&RAWESTREG?0:1,
		p_object->status&DECSOUTHREG?1:0,
		p_object->status&DECNORTHREG?1:0,
		p_object->status&RADECLIMREG?1:0);
}

void draw_pwr_status( st_ppc_info* p_object)
{
	printf(  "<tr><td><font color=%s>NO UPS 1 POWER</font><td> %3s", pwr_color,(p_object->control)&PIN01REG?"OFF":"ON"); /*C0'*/
	printf(  "<td><font color=%s>UPS 1 POWER</font><td> %3s", pwr_color,(p_object->data)&PIN09REG?"ON":"OFF");

	printf(  "<tr><td><font color=%s>NO UPS 2 POWER</font> <td>%3s", pwr_color,(p_object->control)&PIN14REG?"OFF":"ON"); /*C1' */
	printf(  "<td><font color=%s>UPS 2 POWER</font> <td>%3s", pwr_color,(p_object->data)&PIN08REG?"ON":"OFF");

	printf(  "<tr><td><font color=%s>NO UPS 3 POWER</font><td> %3s",pwr_color, (p_object->data)&PIN02REG?"ON":"OFF");
	printf(  "<td><font color=%s>UPS 3 POWER</font><td> %3s", pwr_color,(p_object->data)&PIN07REG?"ON":"OFF");

	printf(  "<tr><td><font color=%s>NO UPS 4 POWER</font><td> %3s", pwr_color,(p_object->data)&PIN03REG?"ON":"OFF");
	printf(  "<td><font color=%s>UPS 4 POWER </font><td>%3s", pwr_color,(p_object->data)&PIN06REG?"ON":"OFF");

	printf(  "<tr><td><font color=%s>NO UPS 5 POWER</font> <td>%3s", pwr_color,(p_object->control)&PIN16REG?"ON":"OFF"); /*C2 */
	printf(  "<td><font color=%s>UPS 5 POWER</font><td> %3s", pwr_color,(p_object->data)&PIN05REG?"ON":"OFF");

	printf(  "<tr><td><font color=%s>NO UPS 6 POWER</font><td>%3s", pwr_color,(p_object->data)&PIN04REG?"ON":"OFF");
	printf(  "<td><font color=%s>UPS 6 POWER </font><td>%3s", pwr_color,(p_object->control)&PIN17REG?"OFF":"ON"); /*C3' */
}

void get_last_shift (void)
{
	static MYSQL my_connection;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int num_fields, i,ra,dec;
	char query_string[200] ;
	time_t	now;
	
	time(&now);
	now -=36000;
	sprintf(query_string,"select RA,Declination from Shift where Timestamp > %d order by id",now);
	//printf("%s\n",query_string);
	//Connect to database
	mysql_init(&my_connection);
	if (!mysql_real_connect(&my_connection, "localhost", "fov", "public", "fov", 0, NULL, 0))
			exit(0);
 	if (mysql_real_query (&my_connection, query_string, strlen (query_string))) exit(0);
	if ((result = mysql_store_result (&my_connection))==NULL) exit(0);

	num_fields = mysql_num_fields(result);
	i=0;
	while ((row = mysql_fetch_row(result)))
	{
		i++;
		ra = atoi (row[0]);
		dec = atoi (row[1]);
	}
	
	if(i>1)printf("(%d,%d)",ra,dec);
	mysql_close(&my_connection);
}
