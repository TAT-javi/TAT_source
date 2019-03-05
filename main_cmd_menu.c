/*
	Menu to control the TAT
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <curses.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include "tat_info.h"
#include "dsp_func.h"
#include "main_cmd_menu.h"
#include "ppcdaemon.h"
#include "pwrdaemon.h"
#include "ctl_func.h"
#include "ccd_func.h"
#include "ppc_func.h"
#include "pwr_func.h"
#include "Apogee.h"
#include "symblc_const.h"
#include "adjust_fov.h"
#include "myAstro.h"
#include "common_func.h"
#include "fli_func.h"


DATE getSystemTime();
const int image_download_time=CCD_READOUT_TIME+ADJUST_TIME;

#define LED_DELAY 7  /*delay to make sure LED is off during the sequence*/
#define STEP_DELAY CCD_READOUT_TIME  /*delay between 2 steps in the sequence*/

const char *head_menu[]=
{
	"A)Motor",
	"B)Power",
	"C)CCD",
	"D)Daemon",
	"E)Power2",
	"F)AutoObs",
	"G)Monitor",
	"H)Manual",
	"I)FLI",
	0,
};

const int head_pos[] = {1,10,19,26,36,46,57,68,78};

const char *tel_menu[]=
{
	" 1) Move RA to Reference Position",	" 2) Move DEC to Reference Position",
	" 3) Move RA (settings are below)",	" 4) Move DEC (settings are below)",
	" 5) Stop RA",				" 6) Stop DEC",
	" 7) Move RA with Star's Speed",	" 8) Move RA with Moon's Speed",
	"****  ****",	"**** **** ",
	" 9) Move WHEEL to filter ",	" q) Change filter setting",
	"===================================",	"===================================",
	"0) RA and DEC default setting",	" ",
	"*******    a-d are for settings of 5   **************",	"",
	"a) Set RA Direction (POS/NEG)",	" ",
	"b) Set RA Starting Time",		" ",
	"c) Set RA End Time",			" ",
	"d) Set RA Register",			" ",
	"*******    e-h are for settings of 6   **************",	"",
	"e) Set DEC Direction (POS/NEG)",	" ",
	"f) Set DEC Starting Time",		" ",
	"g) Set DEC End Time",			" ",
	"h) Set DEC Register",			" ",
	0,
};

const char *ppc_menu[]=
{
	"a1) Close Enclosure",		"a4) Enclosure Close time (second) :",
	"a2) Open Enclosure",		"a5) Enclosure  Open time (second) :",
	"a3) Stop Enclosure Movement",	"a6) Reset RF",
	"b1) Lock Enclosure",		"b2) Unlock Enclosure",
	"b3) Stop Locking Movement",	" ",
	"c1) All Power OFF except CCD",		"c2) All Power ON",
	"d1) Main power OFF",		"d2) Main Power ON",
	"e1) RA Power OFF",		"e2) RA Power ON",
	"f1) DEC Power OFF",		"f2) DEC Power ON",
	"i1) DSP Power OFF",		"i2) DSP Power ON",
	"j1) CCD Power OFF",		"j2) CCD Power ON",
	"k1) VDC Power OFF",		"k2) VDC Power ON",
// 	"m1) Wheel Power OFF",		"m2) Wheel Power ON",
	"n1) DSP Jumper Normal",	"n2) DSP Jumper Debug",
	0,
};
const char *ccd_menu[]=
{
	" a) Set CCD temperature :",	"",
	" b) CCD Cooler ON (required after changing temperature)",	"",
	" c) CCD Cooler OFF",			"",
	" d) Expose Time (float in second) :",	"",
	" f) Shutter (0 -- close; 1 -- open) :",			"",
	" g) Image Binning :",			"",
	" h) Set All to Default Setting",	"",
	"",					"",
	" j) Take Image",			"k) Abort Take Image",
	" l) CCD warm up",			"",
	" m) Image prefix :",			"",
	"",					"",
	"Before turn off the ccd cooler [c], let the ccd warm up [k] until ambient temperature","",
	0,
};

const char *ctl_menu[]=
{
	" a) Stop ALL daemon",			"b) Start ALL daemon",
	" c) Restart ALL daemon",		"",
	"----------------------",		"-------------------",
	" 1) Stop DSP daemon",			"2) Start DSP daemon",
	" 3) Restart DSP daemon",		"",
	" 4) Stop CCD daemon",			"5) Start CCD daemon",
	" 6) Restart CCD daemon",		"",
	" 7) Stop PPC daemon",			"8) Start PPC daemon",
	" 9) Restart PPC daemon",		"",
	" d) Stop PWR daemon",			"e) Start PWR daemon",
	" f) Restart PWR daemon",		"",
	" g) Stop FLI daemon",			"h) Start FLI daemon",
	" i) Restart FLI daemon",		"",
	" j) Stop LST daemon",			"k) Start LST daemon",
	" l) Restart LST daemon",		"",
	0,
};

const char *pwr_menu[]=
{
	" a1) UPS 1 Power OFF",		"a2) UPS 1 Power ON ",
	" b1) UPS 2 Power OFF",		"b2) UPS 2 Power ON ",
	" c1) UPS 3 Power OFF",		"c2) UPS 3 Power ON ",
	" d1) UPS 4 Power OFF",		"d2) UPS 4 Power ON ",
	" e1) UPS 5 Power OFF",		"e2) UPS 5 Power ON ",
	" f1) UPS 6 Power OFF",		"f2) UPS 6 Power ON ",
	"",				"",
	" g1) NO UPS 1 Power OFF",	"g2) NO UPS 1 Power ON",
	" h1) NO UPS 2 Power OFF",	"h2) NO UPS 2 Power ON",
	" i1) NO UPS 3 Power OFF",	"i2) NO UPS 3 Power ON",
	" j1) NO UPS 4 Power OFF",	"j2) NO UPS 4 Power ON",
	" k1) NO UPS 5 Power OFF",	"k2) NO UPS 5 Power ON",
	" l1) NO UPS 6 Power OFF",	"l2) NO UPS 6 Power ON",
	"",				"",
	" m1) UPS Power OFF",		"m2) UPS Power ON",
	" n1) NO UPS Power OFF",	"n2) NO UPS Power ON",
	" p1) ALL Power OFF",		"p2) ALL Power ON",
	0,
};

const char *obs_menu[]=
{
	" a) Change End Time (UTC) :",		"",
	" b) Change Filter Sequence :",		"",
	"------------------------------------",		"",
	" c) Move FOV in RA direction",			"",
	"    1) Change RA direction:",			"",
	"    2) Change RA pixels:",			"",

	" d) Move FOV in DEC direction",			"",
	"    3) Change DEC direction:",			"",
	"    4) Change DEC pixels:",		"",
	" e) Stop RA 10 seconds",
	0,
};

const char *manual_menu[]=
{
	" 1) Move RA and DEC to Reference Position",	"",
	"",						"",
	" 2) Goto ???????? ????????  and tracking" ,	"",
	" _) Change RA (a)hh (b)mm (c)ss",		"_) Change DEC : (d)-  (e)dd (f)mm (g)ss",
	"",						"",
	" 3) Take Image",				"",
	"    l) Stop Taking Image",			"  m) Integral minute :",
	"    h) Change Expose Time :",			"  j) Change Cycle Time :",
	"    i) Change Image Prefix : ",		"  k) Change Image Number :",
	" 4) Stop Moving RA and DEC",			"",
		"--------------------------------------------",		"",
	" 5) Move FOV in RA direction",			"",
	"    w) Change RA direction:",			"",
	"    v) Change RA pixels:",			"",
// 		"------------------------------------",		"",
	" 6) Move FOV in DEC direction",			"",
	"    y) Change DEC direction:",			"",
	"    z) Change DEC pixels:",			"",
	"---------------------------------------------",		"",
	" 7) Stop RA 10 seconds","",
	" 8) Move RA and DEC to safe Position",		"",
	0,
};

const char *fli_menu[]=
{
	"----------------Focuser-------------------------",	"",
	" a) Change step",	"",
	" b) Move out",	" c) Move in",
	" d) Home Focuser",			"",
	" e) Move Focuser to an specific position",		"",
	" ",	"",
	" ",	"",
	"----------------Filter Wheel---------------------",	"",
	" 0-6) Move Filter Wheel to position [0-6]",	"",
	"",		"",
	"",		"",
	0,
};

/* curses */
WINDOW	*star_win;

/* cmd */
st_tel_cmd ra, dec;
st_enc_cmd enc;
st_ccd_cmd ccd;
st_manual_cmd manual;
st_fli_cmd fli;
st_tat_info *p_tat_info;
st_pix_move pix;
void *star_track(void*);

pthread_mutex_t mutex_send_cmd2dsp;
char filter_pos[FILTER_TOTAL_NUMBER],location[20];

void main(void)
{
	int		i=0,int_current_tab=0;
	char	choice[3];
	int		i_choice;
	int		menu_id;
	int		curr_menu_id;
	char 	temp_string[8];

	/* pid */
	pid_t	pid;

	/* select */
	fd_set		readfds;
	struct timeval	timeout;

	/* initialize screen */
	initscr();
	start_color();
	init_pair( 1, COLOR_RED, COLOR_BLACK);
	init_pair( 2, COLOR_GREEN, COLOR_BLACK);
	init_pair( 3, COLOR_YELLOW, COLOR_BLACK);
	init_pair( 4, COLOR_BLUE, COLOR_BLACK);
	init_pair( 5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair( 6, COLOR_CYAN, COLOR_BLACK);
	init_pair( 7, COLOR_WHITE, COLOR_RED);
	star_win = newwin( STAR_WIN_LINES, STAR_WIN_COLS, STAR_WIN_Y, STAR_WIN_X);
	scrollok(star_win, 1);
	
	/*get shared memory pointer*/
	int shmid;
	create_tat_info_shm( &shmid, &p_tat_info);

	DoGetValueString("LOCATION", location);
	
	/* init vars */
	i_choice= (int)NULL;
	curr_menu_id = menu_id= 'G';
	init_tel_setting();
	init_enc_setting();
	init_ccd_setting();
	init_manual_setting();
	init_fli_setting();
	int_filters_setting();
	/* pthread */
	pthread_mutex_init( &mutex_send_cmd2dsp, NULL);

	sleep(1);
	erase();

	/* main loop */
	while(1)
	{
		if(curr_menu_id != menu_id)
		{
			curr_menu_id=menu_id;
			erase();
		}
		draw_head_menu( head_menu );
		draw_line(HEAD_BODY_SPLIT_LINE,'=');

		switch( menu_id )
		{
			case 'A':
				int_current_tab=0;
				run_tel_select(i_choice);
				draw_body_menu( tel_menu );
				draw_tel_setting( &ra, TEL_SETTING_LINE, TEL_SETTING_COL);
				draw_tel_setting( &dec, TEL_SETTING_LINE+5, TEL_SETTING_COL);
				attrset( COLOR_PAIR(3)|A_BOLD);
				mvprintw( MX_SETTING_LINE, MX_SETTING_COL-1, "%d(%c)", 
						  fli.filter_pos_setting,filter_pos[fli.filter_pos_setting]);
				attrset( COLOR_PAIR(0));
				draw_tel_status( &(p_tat_info->dsp_info), INFO_LINE);
			break;

			case 'B':
				int_current_tab=1;
				run_ppc_select(i_choice);
				draw_body_menu( ppc_menu );
				draw_enc_setting( &enc, ENC_SETTING_LINE, ENC_SETTING_COL);
				draw_enc_status( &(p_tat_info->dsp_info), ENC_INFO_LINE);
				draw_ppc_status( &(p_tat_info->ppc_info), INFO_LINE);
			break;

			case 'C':
				int_current_tab=2;
				run_ccd_select( i_choice);
				draw_body_menu( ccd_menu);
				draw_ccd_setting( &ccd, CCD_SETTING_LINE, CCD_SETTING_COL);

				draw_ccd_status_f6( &(p_tat_info->ccd_info), INFO_LINE);

			break;

			case 'D':
				int_current_tab=3;
				run_ctl_select( i_choice);
				draw_body_menu( ctl_menu );
				draw_ctl_status( &(p_tat_info->ctl_info), INFO_LINE);
			break;

			case 'E':
				int_current_tab=4;
				run_pwr_select( i_choice);
				draw_body_menu( pwr_menu );
				draw_pwr_status( &(p_tat_info->pwr_info), INFO_LINE);
			break;

			case 'F':
				int_current_tab=5;
				if( p_tat_info->obs_info.auto_observing == 0)
				{
					mvprintw(MSG_LINE, 0, "No auto-observing now");
					mvprintw(MSG_LINE2, 0, "Try (H) manual observing ?");
					break;
				}
				run_obs_select( i_choice);
			    draw_body_menu( obs_menu );
				draw_obs_status( &(p_tat_info->obs_info), INFO_LINE);
				draw_obs_setting( BODY_LINE, 30);
			break;
			case 'G':
				int_current_tab=6;
				run_mon_select( i_choice);
				i=3;
				mvprintw( i, 0, "auto-observing program [%8s]", p_tat_info->ctl_info.aobs?"active":"inactive");
				i++;
				draw_obs_status( &(p_tat_info->obs_info), i);
				i+=3;
				draw_line(i, '-');
				i+=1;
				draw_tel_status( &(p_tat_info->dsp_info), i);
				//i+=8;
				i+=6;
				draw_fli_status( &(p_tat_info->fli_info), i);
				i+=4;
				draw_line(i, '-');
				i+=1;
				draw_ccd_status_f6( &(p_tat_info->ccd_info), i);
				i+=4;
				draw_line(i, '-');
				i = MSG_LINE2-1;
				draw_line(i, '-');
				draw_enc_status( &(p_tat_info->dsp_info), ++i);
				i+=2;
				draw_ppc_status( &(p_tat_info->ppc_info), i);
			break;
			case 'H':
				int_current_tab=7;
				if( p_tat_info->obs_info.auto_observing == 1)
				{
					mvprintw(MSG_LINE, 0, "It's auto-observing now");
					break;
				}
				run_manual_select( i_choice);
				draw_body_menu( manual_menu );
// 				draw_line( STAR_WIN_Y-1, '=');
				draw_line( STAR_WIN_Y+STAR_WIN_LINES, '=');
				draw_manual_status(INFO_LINE);
				draw_manual_setting( BODY_LINE, 9);
			break;
			case 'I':
				int_current_tab=8;
				run_fli_select( i_choice);
				draw_body_menu( fli_menu );
				draw_line( STAR_WIN_Y+STAR_WIN_LINES, '=');
				draw_fli_status(&(p_tat_info->fli_info),INFO_LINE);
				draw_fli_setting( &fli,BODY_LINE+1, 20);
			break;
		}
		
		//MARK CURRENT TAB
		attrset(COLOR_PAIR(7)|A_BOLD);
		mvprintw(HEAD_LINE+1,head_pos[int_current_tab],head_menu[int_current_tab]);
		
		//WARN IF CCD IS WORKING
		attrset( COLOR_PAIR(1)|A_BOLD);
		if (p_tat_info->obs_info.ccd_status == CCD_IMAGE)
			mvprintw( CMD_LINE - 1, 15, "******CCD is taking images******");
		else if (p_tat_info->obs_info.ccd_status == CCD_FLAT 
			|| p_tat_info->obs_info.ccd_status == CCD_DOING_FLAT)
				mvprintw( CMD_LINE - 1, 15, "******CCD is taking Flat******");
		else if (p_tat_info->obs_info.ccd_status == CCD_DARK)
			mvprintw( CMD_LINE - 1, 15, "******CCD is taking Dark Current******");
		else
			draw_line(CMD_LINE - 1, ' ');
		//INPUT STRING
		attrset( COLOR_PAIR(0) |A_DIM);
		draw_cmd_line("selection? ");
		refresh();

		/* set up arguments for select with timeout */
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);            /* for standard input */
		timeout.tv_sec  = 1;
		timeout.tv_usec = 500;
 		i_choice=0;
		/* wait for either input or the end of the delay period */
		if (select(32, &readfds, (fd_set *)NULL, (fd_set *)NULL, &timeout) > 0)
 		{
 			get_string( choice, 2);
 			i_choice = wordexp( choice );

 			if( i_choice )
 			{
 			        /*	for debug
 				draw_line( MSG_LINE2, ' ');
 				mvprintw( MSG_LINE2, 0, "choice = %d", i_choice);
 				*/
				run_menu_select( &menu_id, &i_choice );
			}
			else
			{
				for( i=MSG_LINE; i<=LINES;i++)
						draw_line( i, ' ');
				/* erase residual message which was print by send_cmd2xxx */
			}
 		}
	}
	/* terminate screen */
	endwin();
	exit(EXIT_SUCCESS);
}

/* utility */
int wordexp( char *p_word)
{
	int a,b;
	b = *(p_word+1)?(*(p_word+1)):0;
	a = (*p_word)*(b?10:1);
	*p_word=*(p_word+1)='\0';
	return a+b;
}

/* draw function */
void draw_cmd_line(char prompt[COLS])
{
	draw_line(CMD_LINE, ' ');
	attrset( COLOR_PAIR(CMD_LINE_COLOR) | A_BOLD);
	mvprintw(CMD_LINE,0, prompt);
	attrset( COLOR_PAIR(0) | A_DIM);
}

void draw_line(int line, char symbol)
{
	int col;
	for( col=0;col<COLS;col++)
		mvaddch( line, col, symbol);
}

void draw_head_menu(const char *menu[])
{
	static char **p_menu;
	static char *p_ch;
	static struct tm *tm_now;
	static time_t t_now;

	p_menu=menu;

	time( &t_now);
	tm_now = localtime( &t_now);

	move( HEAD_LINE, 0);
	printw( "Host: %s \t\t%04d/%02d/%02d %02d:%02d:%02d\tSite: %s", getenv( "HOSTNAME"),
			tm_now->tm_year+1900,
			tm_now->tm_mon+1,
        	tm_now->tm_mday,
        	tm_now->tm_hour,
        	tm_now->tm_min,
        	tm_now->tm_sec,
			location);
	move( HEAD_LINE+1, 1);
	while(*p_menu)
	{
		p_ch = *p_menu;
		attrset( COLOR_PAIR(HEAD_MENU_COLOR) |A_BOLD);
		for( ; *p_ch != ')' && *p_ch; p_ch++)
		{
			addch( *p_ch );
		}
		attrset( COLOR_PAIR(0) |A_DIM);
		printw("%s  ",p_ch);
		p_menu++;
	}
}

void draw_body_menu(const char *menu[])
{
	char **p_menu;
	char *p_ch;
	int items=0;
	int line, col;
	int par_pos;	/* position of the first right parenthesis */

	line=BODY_LINE;
	col=BODY_COL;
	p_menu=menu;

	while(*p_menu)
	{
		p_ch = *p_menu;
		move( items%2?line:line++, (items++)%2?col:0 );
		attrset( COLOR_PAIR(BODY_MENU_COLOR) |A_BOLD);
		for( ; *p_ch != ')' && *p_ch; p_ch++)
		{
			addch( *p_ch );
		}
		attrset( COLOR_PAIR(0) |A_DIM);

		printw("%s",p_ch);
		p_menu++;
	}
}

void draw_tel_setting(st_tel_cmd* tel, int line, int col )
{
	mvprintw( line, col, "Dir=");
	attrset( COLOR_PAIR(1)|A_BOLD);
	printw( "%8s", tel->dir?"POS":"NEG");
	attrset( COLOR_PAIR(0));

	mvprintw( ++line, col, "CmdStart=");
	attrset( COLOR_PAIR(2)|A_BOLD);
	printw( "%3d", tel->start);
	attrset( COLOR_PAIR(0));

	mvprintw( ++line, col, "CmdEnd=");
	attrset( COLOR_PAIR(3)|A_BOLD);
	printw( "%5d", tel->end);
	attrset( COLOR_PAIR(0));

	mvprintw( ++line, col, "Reg=");
	attrset( COLOR_PAIR(4)|A_BOLD);
	printw( "%8d", tel->reg);

	attrset( COLOR_PAIR(0));
}

void draw_mx_setting( st_mx_cmd* p_object, int line, int col)
{
	mvprintw( line, col, "%-6d", p_object->move);
}

void draw_tel_status( st_dsp_info *p_object, int line)
{
	static int col;
	static int col_len=20;
	static int col_mx=15;
	char wheel_string[15];
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
	col=0;
	mvprintw( line, 0, "DSP Time Count=%-6d",p_object->time_count);
	/* RA */
	line++;
	mvprintw( line, 0, "RA reg=%-6d",p_object->ra.reg);
	mvprintw( line, col_len*1, "pulse=%-8d", p_object->ra.pulse);

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
	mvprintw( line, col_len*2,"dir=%-3s",msg[option]);

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
	mvprintw( line, col_len*3,"status=%-10s",msg[option]);

	line++;
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
	mvprintw( line, 0,"RA PulseCtl=%-7s",msg[option]);
	mvprintw( line, col_len*1,"MovePulse=%-8d",p_object->ra.refpulse);
	mvprintw( line, col_len*2,"Remainder=%-8d",p_object->ra.purpulse);

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
	mvprintw( line, col_len*3,"In Origin=%-3s",msg[option]);
	line++;
	draw_line( line,'-');
	/* DEC */
	line++;
	mvprintw( line, 0, "DEC reg=%-6d",p_object->dec.reg);
	mvprintw( line, col_len*1, "pulse=%-8d", p_object->dec.pulse);

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
	mvprintw( line, col_len*2,"dir=%-3s",msg[option]);

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
	mvprintw( line, col_len*3,"status=%-10s",msg[option]);

	line++;
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
	mvprintw( line, 0,"DEC PulsCtl=%-7s",msg[option]);
	mvprintw( line, col_len*1,"MovePulse=%-8d",p_object->dec.refpulse);
	mvprintw( line, col_len*2,"Remainder=%-8d",p_object->dec.purpulse);

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
	mvprintw( line, col_len*3,"In Origin=%-3s",msg[option]);
	line++;
	draw_line( line,'-');
	line++;

	if( p_tat_info->fli_info.wheel_moving) sprintf(wheel_string,"Wheel Moving");
	else sprintf(wheel_string,"                ");
	
	mvprintw( line, 0, "Current Filter = %d(%c) %s",
			  p_tat_info->fli_info.wheel_curr_position,p_tat_info->obs_info.filter_type,
			  wheel_string);

}

void draw_enc_status( st_dsp_info* p_object, int line)
{
	int col;
	int col_len=20;
	col=0;
	static char temp_string[BUFSIZ];

        /* enc/lock */

	if( p_object->enc.open )
	{
		sprintf( temp_string, "Opening");
	}
	else if( p_object->enc.close )
	{
		 sprintf( temp_string, "Closing");
	}
	else
	{
		sprintf( temp_string, "No Action");
	}
	mvprintw( line, col_len*0, "Enc %-9s", temp_string);
	mvprintw( line, col_len*1, "Time=%-3d",p_object->enc.time);

	if( p_object->latch.lock )
	{
		sprintf( temp_string, "Locking");
	}
	else if( p_object->latch.unlock )
	{
		sprintf( temp_string, "Unlocking");
	}
	else
	{
		sprintf( temp_string, "No Action");
	}
	mvprintw( line, col_len*2, "Latch %-9s",temp_string);
	mvprintw( line, col_len*3, "Time=%-3d",p_object->latch.time);
	line++;
	mvprintw( line, col_len*0, "Enc Closed %3s", p_tat_info->dsp_info.enc.closed_ls?"YES":"NO ");
}

void draw_ppc_status( st_ppc_info* p_object, int line)
{
	int col=0, col_len=20;

	/* enc/lock */
	mvprintw( line, 0,"MAIN POWER\t%3s",(p_object->control)&MAINPWRREG?"OFF":"ON");
	line++;
	mvprintw( line, col_len*0, "RA POWER\t%3s", (p_object->data)&RAPWRREG?"OFF":"ON");
	mvprintw( line, col_len*2, "DEC POWER\t%3s",(p_object->data)&DECPWRREG?"OFF":"ON");
	line++;
	mvprintw( line, col_len*0, "DSP POWER\t%3s",(p_object->data)&DSPPWRREG?"OFF":"ON");
	mvprintw( line, col_len*2, "CCD POWER\t%3s",(p_object->data)&CCDPWRREG?"OFF":"ON");

	line++;
// 	mvprintw( line, col_len*0, "WHE POWER\t%3s",(p_object->data)&WHEELPWRREG?"OFF":"ON");
	mvprintw( line, col_len*0, "VDC POWER\t%3s",(p_object->control)&VDCPWRREG?"OFF":"ON");
	
	line++;
	mvprintw( line, col_len*0, "DSP JUMPER\t%3s",(p_object->data)&DSPJPREG?"Debug ":"Normal");
	
	line++;
	mvprintw( line, col_len*0, "STATUS %d%d%d%d %d",
        	p_object->status&RAEASTREG?1:0,
        	p_object->status&RAWESTREG?0:1,
        	p_object->status&DECSOUTHREG?1:0,
        	p_object->status&DECNORTHREG?1:0,
        	p_object->status&RADECLIMREG?1:0);
	attrset( COLOR_PAIR(0) |A_DIM);
}

void draw_fli_status( st_fli_info* p_object, int line)
{
	char wheel_string[15],focuser_string[15];
	
	if(p_object->wheel_moving) sprintf(wheel_string,"Wheel Moving");
	else sprintf(wheel_string,"                ");
	
	
	if(p_object->focuser_moving) sprintf(focuser_string,"Focuser Moving");
	else sprintf(focuser_string,"              ");
	
	draw_line( line++, '-');
	//mvprintw( line++, 1, "%s",focuser_string);
	mvprintw( line++, 0, "Current Focuser Position %d     %s",
			  p_object->focuser_curr_position,focuser_string);
	draw_line( line++, '-');
	//mvprintw( line++, 1, "%s",wheel_string);
	mvprintw( line, 0, "Current Filter %d(%c)        %s",
			  p_object->wheel_curr_position,p_tat_info->obs_info.filter_type,wheel_string);
}

void draw_ccd_status_f6( st_ccd_info* p_object, int line)
{
	int col=0, col_len=20;
	char temp_string[BUFSIZ];

	mvprintw( line, col_len*0, "current point = %4.2f", p_object->curr_point);
	mvprintw( line, col_len*2, "set point = %4.2f", p_object->set_point);
	line++;
	
	if(p_object->set_point > 200) //Cooler is OFF
	{
		mvprintw( line, col_len*0, "camera status = %-20S","Cooler ON first" );
			mvprintw( line, col_len*2, "cooler status = %-20s", "Cooler ON first");
		line++;
		mvprintw( line, col_len*0, "Fan status = %-20s",  "Cooler ON first" );
		line++;
	}
	else
	{
		mvprintw( line, col_len*0, "camera status = %-20s", Camera_status_string[p_object->camera_status]);
		mvprintw( line, col_len*2, "cooler status = %-20s", Cooler_status_string[p_object->cooler_status]);
		line++;
		mvprintw( line, col_len*0, "Fan status = %-20s",  Fan_status_string[ p_object->fan_status]);
		line++;
	}
//DataError PatternError Idle Exposing ImagingActive ImageReady Flushing WaitingOnTrigger
// 	switch( p_object->camera_status)
// 	{
// 		case Apn_Status_DataError:
// 			strcpy( temp_string,"DataError");
// 		break;
// 		case Apn_Status_PatternError:
// 			strcpy( temp_string,"PatternErr");
// 		break;
// 		case Apn_Status_Idle:
// 			strcpy( temp_string,"Idle");
// 		break;
// 		case Apn_Status_Exposing:
// 			strcpy( temp_string,"Exposing");
// 		break;
// 		case Apn_Status_ImagingActive:
// 			strcpy( temp_string,"ImageActive");
// 		break;
// 		case Apn_Status_ImageReady:
// 			strcpy( temp_string,"ImageReady");
// 		break;
// 		case Apn_Status_Flushing:
// 			strcpy( temp_string,"Flushing");
// 		break;
// 		case Apn_Status_WaitingOnTrigger:
// 			strcpy( temp_string,"WaitOnTrig");
// 		break;
// 		case Apn_Status_ConnectionError:
// 			strcpy( temp_string,"ConnErr");
// 		break;
// 		default:
// 			strcpy( temp_string,"cooler on first");
// 		break;
// 
// 	}
// 	mvprintw( line, col_len*0, "camera status = %-20s", Camera_status_string[p_object->camera_status]);

/*
#define Apn_CoolerStatus int
#define Apn_CoolerStatus_Off 0
#define Apn_CoolerStatus_RampingToSetPoint 1
#define Apn_CoolerStatus_AtSetPoint 2
#define Apn_CoolerStatus_Revision 3
*/
// 	switch( p_object->cooler_status)
// 	{
// 		case Apn_CoolerStatus_Off:
// 			strcpy( temp_string,"Off");
// 		break;
// 		case Apn_CoolerStatus_RampingToSetPoint:
// 			strcpy( temp_string,"RampingToSetPoint");
// 		break;
// 		case Apn_CoolerStatus_AtSetPoint:
// 			strcpy( temp_string,"AtSetPoint");
// 		break;
// 		case Apn_CoolerStatus_Revision:
// 			strcpy( temp_string,"Revision");
// 		break;
// 		default:
// 			strcpy( temp_string,"cooler on first");
// 		break;
// 
// 	}
// 	mvprintw( line, col_len*2, "cooler status = %-20s", temp_string);
// 	line++;

/*	
	#define	Apn_FanMode_Off	0
	#define	Apn_FanMode_Low 1
	#define	Apn_FanMode_Medium 2
	#define	Apn_FanMode_High 3
*/	
// 	switch( p_object->fan_status)
// 	{
// 		case Apn_FanMode_Off:
// 			strcpy( temp_string, "Off");
// 		break;
// 		case Apn_FanMode_Low:
// 			strcpy( temp_string, "Slow");
// 		break;
// 		case Apn_FanMode_Medium:
// 			strcpy( temp_string, "Medium");
// 		break;
// 		case Apn_FanMode_High:
// 			strcpy( temp_string, "Fast");
// 		break;
// 		default:
// 			strcpy( temp_string,"cooler on first");
// 		break;
// 
// 	}
// 	mvprintw( line, col_len*0, "Fan status = %-20s",  temp_string);
// 	line++;
	mvprintw( line, col_len*0, "Recent Image: %s", *(p_tat_info->obs_info.recent_image)?p_tat_info->obs_info.recent_image:"None");

	attrset( COLOR_PAIR(0) |A_DIM);

}

void draw_ccd_status_u6( st_ccd_info* p_object, int line)
{
	int col=0, col_len=20;
	char temp_string[BUFSIZ];

	mvprintw( line, col_len*0, "current point = %4.2f", p_object->curr_point);
	mvprintw( line, col_len*2, "set point = %4.2f", p_object->set_point);
	line++;
/*
#define Apn_Status int
#define Apn_Status_DataError -2
#define Apn_Status_PatternError  -1
#define Apn_Status_Idle  0
#define Apn_Status_Exposing  1
#define Apn_Status_ImagingActive  2
#define Apn_Status_ImageReady  3
#define Apn_Status_Flushing  4
#define Apn_Status_WaitingOnTrigger 5
#define Apn_Status_ConnectionError 6
*/
	switch( p_object->camera_status)
	{
		case Apn_Status_DataError:
			strcpy( temp_string,"DataError");
		break;
		case Apn_Status_PatternError:
			strcpy( temp_string,"PatternErr");
		break;
		case Apn_Status_Idle:
			strcpy( temp_string,"Idle");
		break;
		case Apn_Status_Exposing:
			strcpy( temp_string,"Exposing");
		break;
		case Apn_Status_ImagingActive:
			strcpy( temp_string,"ImageActive");
		break;
		case Apn_Status_ImageReady:
			strcpy( temp_string,"ImageReady");
		break;
		case Apn_Status_Flushing:
			strcpy( temp_string,"Flushing");
		break;
		case Apn_Status_WaitingOnTrigger:
			strcpy( temp_string,"WaitOnTrig");
		break;
		case Apn_Status_ConnectionError:
			strcpy( temp_string,"ConnErr");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	mvprintw( line, col_len*0, "camera status = %-20s", temp_string);

/*
#define Apn_CoolerStatus int
#define Apn_CoolerStatus_Off 0
#define Apn_CoolerStatus_RampingToSetPoint 1
#define Apn_CoolerStatus_AtSetPoint 2
#define Apn_CoolerStatus_Revision 3
*/
	switch( p_object->cooler_status)
	{
		case Apn_CoolerStatus_Off:
			strcpy( temp_string,"Off");
		break;
		case Apn_CoolerStatus_RampingToSetPoint:
			strcpy( temp_string,"RampingToSetPoint");
		break;
		case Apn_CoolerStatus_AtSetPoint:
			strcpy( temp_string,"AtSetPoint");
		break;
		case Apn_CoolerStatus_Revision:
			strcpy( temp_string,"Revision");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	mvprintw( line, col_len*2, "cooler status = %-20s", temp_string);
	line++;

/*	
	#define	Apn_FanMode_Off	0
	#define	Apn_FanMode_Low 1
	#define	Apn_FanMode_Medium 2
	#define	Apn_FanMode_High 3
*/	
	switch( p_object->fan_status)
	{
		case Apn_FanMode_Off:
			strcpy( temp_string, "Off");
		break;
		case Apn_FanMode_Low:
			strcpy( temp_string, "Slow");
		break;
		case Apn_FanMode_Medium:
			strcpy( temp_string, "Medium");
		break;
		case Apn_FanMode_High:
			strcpy( temp_string, "Fast");
		break;
		default:
			strcpy( temp_string,"cooler on first");
		break;

	}
	mvprintw( line, col_len*0, "Fan status = %-20s",  temp_string);
	line++;
	mvprintw( line, col_len*0, "Recent Image: %s", *(p_tat_info->obs_info.recent_image)?p_tat_info->obs_info.recent_image:"None");

	attrset( COLOR_PAIR(0) |A_DIM);

}


void draw_pwr_status( st_ppc_info* p_object, int line)
{
	int col;
	int col_len=40;

	col=0;

	mvprintw( line, col_len*0, "NO UPS 1 POWER %3s", (p_object->control)&PIN01REG?"OFF":"ON"); /*C0'*/
	mvprintw( line, col_len*1, "UPS 1 POWER %3s", (p_object->data)&PIN09REG?"ON":"OFF");
	line++;

	mvprintw( line, col_len*0, "NO UPS 2 POWER %3s", (p_object->control)&PIN14REG?"OFF":"ON"); /*C1' */
	mvprintw( line, col_len*1, "UPS 2 POWER %3s", (p_object->data)&PIN08REG?"ON":"OFF");
	line++;

	mvprintw( line, col_len*0, "NO UPS 3 POWER %3s", (p_object->data)&PIN02REG?"ON":"OFF");
	mvprintw( line, col_len*1, "UPS 3 POWER %3s", (p_object->data)&PIN07REG?"ON":"OFF");
	line++;

	mvprintw( line, col_len*0, "NO UPS 4 POWER %3s", (p_object->data)&PIN03REG?"ON":"OFF");
	mvprintw( line, col_len*1, "UPS 4 POWER %3s", (p_object->data)&PIN06REG?"ON":"OFF");
	line++;

	mvprintw( line, col_len*0, "NO UPS 5 POWER %3s", (p_object->control)&PIN16REG?"ON":"OFF"); /*C2 */
	mvprintw( line, col_len*1, "UPS 5 POWER %3s", (p_object->data)&PIN05REG?"ON":"OFF");
	line++;
	mvprintw( line, col_len*0, "NO UPS 6 POWER %3s", (p_object->data)&PIN04REG?"ON":"OFF");
	mvprintw( line, col_len*1, "UPS 6 POWER %3s", (p_object->control)&PIN17REG?"OFF":"ON"); /*C3' */

	attrset( COLOR_PAIR(0) |A_DIM);


}

void draw_ctl_status( st_ctl_info* p_object, int line)
{
	int col;
	int col_len=20;

	col=0;

	mvprintw( line++, col_len*0, "dsp daemon [%8s]", (p_object->dspd)?"active":"inactive");
	mvprintw( line++, col_len*0, "ccd daemon [%8s]", (p_object->ccdd)?"active":"inactive");
	mvprintw( line++, col_len*0, "ppc daemon [%8s]", (p_object->ppcd)?"active":"inactive");
	mvprintw( line++, col_len*0, "pwr daemon [%8s]", (p_object->pwrd)?"active":"inactive");
	mvprintw( line++, col_len*0, "fli daemon [%8s]", (p_object->flid)?"active":"inactive");
	mvprintw( line++, col_len*0, "lst daemon [%8s]", (p_object->lstd)?"active":"inactive");
	line++;
	draw_line( line++, '=');
	line++;
	mvprintw( line, col_len*0, "auto-observing program [%8s]", (p_object->aobs)?"active":"inactive");
}

void draw_obs_status( st_obs_info* p_object, int line)
{
	int col,i;
	int col_len=20;
	struct tm tm_time;
	char time_string[13],Filter_string[100];
	static char temp_string[BUFSIZ];

	col=0;


	mvprintw( line, col_len*0, "Auto Observing=%-3s", p_tat_info->obs_info.auto_observing?"YES":"NO");
	switch( p_tat_info->obs_info.status )
	{
		case Pursuing:
			sprintf( temp_string, "Pursuing  ");
		break;
		case Tracking:
			sprintf( temp_string, "Tracking  ");
		break;
		case Returning:
			sprintf( temp_string, "Returning ");
		break;
		case STOP:
			sprintf( temp_string, "STOP     ");
		break;
		case InOrigin:
			sprintf( temp_string, "InOrigin  ");
		break;
		case Moving:
			sprintf( temp_string, "Moving    ");
		break;
		case Flating:
			sprintf( temp_string, "Flating   ");
		break;
		case Darking:
			sprintf( temp_string, "Darking   ");
		break;
		default:
			sprintf( temp_string, "N.A.       ");

		break;
	}
	mvprintw( line, col_len*2, "Status=%s", temp_string);
	line++;

	gmtime_r(&(p_object->begin_time), &tm_time);
	strftime( time_string, 13, "20%y%m%d%H%M", &tm_time);
	mvprintw( line, col_len*0, "Begin  Time   %12s", time_string);

	gmtime_r(&(p_object->end_time), &tm_time);
	strftime( time_string, 13, "20%y%m%d%H%M", &tm_time);
	mvprintw( line, col_len*2, "End    Time   %12s", time_string);

//	line++;
//	mvprintw( line, col_len*0, "Expose Time   %f", p_object->expose_time);
//	mvprintw( line, col_len*2, "Cycle  Time   %d", p_object->cycle_time);

	line++;
	attrset( COLOR_PAIR(4)|A_BOLD);
	fill_filter_string(Filter_string);
	mvprintw( line, col_len*0, "%s", Filter_string);
	attrset( COLOR_PAIR(0)|A_DIM);
	mvprintw( line, col_len*2, "Total Cycles %d", p_object->cycle_number);
}


void draw_manual_status(int line)
{
	int col;
	int col_len=20;
	static char temp_string[BUFSIZ];
	col=0;


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

		case InOrigin:
			sprintf( temp_string, "InOrigin");
		break;

		case STOP:
		default:
			sprintf( temp_string, "STOP");
		break;

		case Flating:
			sprintf( temp_string, "Flating");
		break;

		case Darking:
			sprintf( temp_string, "Darking");
		break;
	}

	mvprintw( line, col_len*0, "Observing Status=%-9s",temp_string);
	line++;

	mvprintw( line, col_len*0, "DSP Time Count=%-6d",p_tat_info->dsp_info.time_count);
	line++;

	mvprintw( line, col_len*0, "RA  dir=%-3s",p_tat_info->dsp_info.ra.dir?"POS":"NEG");
	switch( p_tat_info->dsp_info.ra.status )
	{

		case NormalObsCycle:
			if( p_tat_info->dsp_info.ra.dir == 1)
			{
				sprintf( temp_string, "Moving");
				break;
			}
		case ReturnToOrigin:
		case NearOrigin:

			sprintf( temp_string, "Moving");
		break;

		case InOrigin:
			if(  p_tat_info->dsp_info.ra.origin == 1 )
				sprintf( temp_string, "In Origin");
			else
				sprintf( temp_string, "Moving");
		break;

		case NormalStop:
			sprintf( temp_string, "Stop");
	}
	mvprintw( line, col_len*1, "Status=%-9s",temp_string);
	mvprintw( line, col_len*2, "CurrPulse=%-7d",p_tat_info->dsp_info.ra.pulse);
	if( p_tat_info->obs_info.status == Pursuing )
		mvprintw( line, col_len*3, "Remainder=%-7d",p_tat_info->dsp_info.ra.purpulse);
	line++;

	/* DEC */

	mvprintw( line, col_len*0, "DEC dir=%-3s",p_tat_info->dsp_info.dec.dir?"POS":"NEG");
	switch( p_tat_info->dsp_info.dec.status )
	{

		case NormalObsCycle:
			if( p_tat_info->dsp_info.dec.dir == 1)
			{
				sprintf( temp_string, "Moving");
				break;
			}
		case ReturnToOrigin:
		case NearOrigin:

			sprintf( temp_string, "Moving");
		break;

		case InOrigin:
			if(  p_tat_info->dsp_info.dec.origin == 1 )
				sprintf( temp_string, "In Origin");
			else
				sprintf( temp_string, "Moving");
		break;

		case NormalStop:
			sprintf( temp_string, "Stop");
	}
	mvprintw( line, col_len*1, "Status=%-9s",temp_string);
	mvprintw( line, col_len*2, "CurrPulse=%-7d",p_tat_info->dsp_info.dec.pulse);
	if( p_tat_info->obs_info.status == Pursuing )
		mvprintw( line, col_len*3, "Remainder=%-7d",p_tat_info->dsp_info.dec.purpulse);
	line++;

	mvprintw( line, col_len*0, "Recent Image: %-s", *(p_tat_info->obs_info.recent_image)?p_tat_info->obs_info.recent_image:"None");

}
void draw_manual_setting( int line, int col )
{

	line+=2;
	attrset( COLOR_PAIR(1)|A_BOLD);
	mvprintw( line, col, "%8.5f %8.5f",
		p_tat_info->obs_info.curr_ra, p_tat_info->obs_info.curr_dec);
	line++;
	col=17;
	attrset( COLOR_PAIR(2)|A_BOLD);
	mvprintw( line, col, "%02d", manual.ra.hh);
	col+=6;
	mvprintw( line, col, "%02d", manual.ra.mm);
	col+=6;
	mvprintw( line, col, "%02d", manual.ra.ss);

	col=BODY_COL+19;
	attrset( COLOR_PAIR(3)|A_BOLD);
	mvprintw( line, col, "%2s", manual.dec.hh<0?"-":"+");
	col+=6;
	mvprintw( line, col, "%02d", manual.dec.dd);
	col+=6;
	mvprintw( line, col, "%02d", manual.dec.mm);
	col+=6;
	mvprintw( line, col, "%02d", manual.dec.ss);

	line+=2;
	col=30;
	if( manual.ccd_ctrl )
	{
		attrset( COLOR_PAIR(1)|A_BOLD);
		mvprintw( line, col, "%5s", "Busy");
	}
	else
	{
		attrset( COLOR_PAIR(2)|A_BOLD);
		mvprintw( line, col, "%5s", "Ready");
	}

	line+=1;
	col=BODY_COL-7;
	attrset( COLOR_PAIR(1)|A_BOLD);
	col+=35;
	mvprintw( line, col, "%4s", manual.ccd_int_min>0?"YES":"NO");

	line+=1;
	col=BODY_COL-7;
	attrset( COLOR_PAIR(3)|A_BOLD);
	mvprintw( line, col, "%7.2f",  p_tat_info->obs_info.expose_time);


	col+=35;
	attrset( COLOR_PAIR(4)|A_BOLD);
	mvprintw( line, col, "%4d [%4d]", manual.ccd_cycle_time, manual.time_counter);

	line+=1;
	col=BODY_COL-7;
	attrset( COLOR_PAIR(5)|A_BOLD);
	mvprintw( line, col, "%7s",  ccd.prefix);
	col+=35;
	attrset( COLOR_PAIR(3)|A_BOLD);
	mvprintw( line, col, "%4d [%4d]", manual.ccd_image_number, manual.ccd_curr_number);

	attrset( COLOR_PAIR(6)|A_DIM);
	col-=20;
	line+=4;
	mvprintw( line, col, "%s", pix.ra_dir?"East":"West");
	line++;
	mvprintw( line, col, "%3d", pix.ra_pix);
	attrset( COLOR_PAIR(2)|A_DIM);

	line+=2;
	mvprintw( line, col, "%s", !pix.dec_dir?"North":"South");
	line++;
	mvprintw( line, col, "%3d", pix.dec_pix);



	attrset( COLOR_PAIR(0)|A_DIM);

}

void draw_enc_setting( st_enc_cmd* enc,  int line, int col )
{
	attrset( COLOR_PAIR(1)|A_BOLD);
	mvprintw( line++, col, "%-3d", enc->close_time);
	attrset( COLOR_PAIR(2)|A_BOLD);
	mvprintw( line, col, "%-3d", enc->open_time);
	attrset( COLOR_PAIR(0) |A_DIM);
}

void draw_ccd_setting( st_ccd_cmd* ccd, int line, int col )
{

	attrset( COLOR_PAIR(1)|A_BOLD);
	mvprintw( line, col, "%-3d", ccd->set_point);

	line+=3;
	attrset( COLOR_PAIR(2)|A_BOLD);
	mvprintw( line, col, "%-5.2f", p_tat_info->obs_info.expose_time);

	line++;
	attrset( COLOR_PAIR(4)|A_BOLD);
	mvprintw( line, col, "%-1d", ccd->shutter);

	line++;
	attrset( COLOR_PAIR(5)|A_BOLD);
	mvprintw( line, col, "%-1d", ccd->bin);

	line+=5;
	attrset( COLOR_PAIR(6)|A_BOLD);
	mvprintw( line, col, "%-8s", ccd->prefix);

	attrset( COLOR_PAIR(0) |A_DIM);
}

void draw_fli_setting( st_fli_cmd* fli, int line, int col )
{
	attrset( COLOR_PAIR(1)|A_BOLD);
	mvprintw( line, col, "%d", fli->move_rel_pos);
	//line +=6;
	//mvprintw( line, col+30, "%d", fli->filter_pos_setting);
	attrset( COLOR_PAIR(0) |A_DIM);
}

void draw_obs_setting( int line, int col )
{
	int i;
	char Filter_string[100],time_string[13];
	struct tm tm_time;

	gmtime_r(&(p_tat_info->obs_info.end_time), &tm_time);
	strftime( time_string, 13, "20%y%m%d%H%M", &tm_time);
	
	attrset( COLOR_PAIR(3)|A_DIM);
	mvprintw( line, col+7, "%12s",time_string);

	sprintf(Filter_string,"None");
	fill_filter_string(Filter_string);
	line ++;	
	attrset( COLOR_PAIR(4)|A_BOLD);
	mvprintw( line, col+7, "%s",Filter_string);
	
	attrset( COLOR_PAIR(5)|A_BOLD);
	line+=3;
	col +=3;
	mvprintw( line, col, "%s", pix.ra_dir?"East":"West");
	line++;
	mvprintw( line, col, "%3d", pix.ra_pix);
	attrset( COLOR_PAIR(2)|A_DIM);

	line+=2;
	mvprintw( line, col, "%s", !pix.dec_dir?"North":"South");
	line++;
	mvprintw( line, col, "%3d", pix.dec_pix);

	attrset( COLOR_PAIR(0)|A_DIM);

}

/* menu function */
#define MAX_STRLEN	32

void run_menu_select( int *p_menu_id, int *i_choice )
{

	switch( *i_choice )
	{
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'I':
			*p_menu_id = *i_choice;
			*i_choice = 0;
		break;
		case 'H':
			touchwin( star_win );
			wrefresh( star_win) ;
			*p_menu_id = *i_choice;
			*i_choice = 0;
			/* mvprintw( MSG_LINE2, 0, "menu_id=%d",*p_menu_id); */
		break;
	}
}



void run_tel_select(int i_choice)
{
	char 	dsp_cmd[80];
	char	temp_string[BUFSIZ];

	switch( i_choice )
	{
		case '1':
			//sprintf( dsp_cmd, "RA NEG RESET 1 9999 0 0 0 0 0");
			sprintf( dsp_cmd, "RA NEG FREQUENCY 1 9973 208 0 0 0 0");
			mvsend_cmd2dsp( dsp_cmd );
		break;
		case '2':
			//sprintf( dsp_cmd, "DEC NEG RESET 1 9999 0 0 0 0 0");
			sprintf( dsp_cmd, "DEC NEG FREQUENCY 1 9973 208 0 0 0 0");
			mvsend_cmd2dsp( dsp_cmd );
		break;
		case '3':
			sprintf( dsp_cmd, "RA %s FREQUENCY %d %d %d 0 0 0 0",
				ra.dir?"POS":"NEG", ra.start, ra.end, ra.reg);
			mvsend_cmd2dsp( dsp_cmd );
		break;
		case '4':
			sprintf( dsp_cmd, "DEC %s FREQUENCY %d %d %d 0 0 0 0",
				dec.dir?"POS":"NEG", dec.start, dec.end, dec.reg);
			mvsend_cmd2dsp( dsp_cmd );
		break;
		case '5':
			sprintf( dsp_cmd, "RA POS FREQUENCY 1 1 9973 0 0 0 0");
			mvsend_cmd2dsp( dsp_cmd );
		break;
		case '6':
			sprintf( dsp_cmd, "DEC POS FREQUENCY 1 1 9973 0 0 0 0");
			mvsend_cmd2dsp( dsp_cmd );
		break;
		case '7':
			sprintf( dsp_cmd, "RA POS FREQUENCY 1 30030 9973 0 0 0 0");
			mvsend_cmd2dsp( dsp_cmd );
		break;
		case '8':
			sprintf( dsp_cmd, "RA POS FREQUENCY 1 30030 10200 0 0 0 0");
			mvsend_cmd2dsp( dsp_cmd );
		break;
		case '9':
			sprintf( dsp_cmd, "WHEEL %d",fli.filter_pos_setting);
			mvsend_cmd2fli( dsp_cmd );
		break;
		case 'q':
			if(++fli.filter_pos_setting>=FILTER_TOTAL_NUMBER) fli.filter_pos_setting =0;
			
		break;

		case '0':
			init_tel_setting();
		break;

		case 'a':
			ra.dir=ra.dir?0:1;
		break;
		case 'b':
			draw_cmd_line( "RA starting time? ");
			get_string( temp_string, 1);
			if( *temp_string )
				ra.start=atoi(temp_string);

		break;
		case 'c':
			draw_cmd_line( "RA end time? ");
			get_string( temp_string, 4);
			if( *temp_string )
				ra.end=atoi(temp_string);

		break;
		case 'd':
			if(ra.reg > 208) ra.reg /= 2;
			else  ra.reg = 417 *4;

		break;

		case 'e':
			dec.dir=dec.dir?0:1;
		break;
		case 'f':
			draw_cmd_line( "DEC starting time? ");
			get_string( temp_string, 1);
			if( *temp_string )
				dec.start=atoi(temp_string);

		break;
		case 'g':
			draw_cmd_line( "DEC end time? ");
			get_string( temp_string, 4);
			if( *temp_string )
				dec.end=atoi(temp_string);

		break;
		case 'h':
			if(dec.reg > 208) dec.reg /= 2;
			else  dec.reg = 417 *4;
		break;

		case 0:

		break;

		default:
			draw_line(MSG_LINE,' ');
			mvprintw( MSG_LINE, 0,"Error command");
	}
}

void run_ppc_select(int i_choice)
{
	static	char cmd[80];
	char temp_string[BUFSIZ];
	switch( i_choice )
	{

		case 'a' *10+ '1':
			sprintf( cmd, "ENC NONE CLOSE 1 %d 0 0 0 0 0", enc.close_time);
			mvsend_cmd2dsp( cmd );
		break;
		case 'a' *10+ '2':
			sprintf( cmd, "ENC NONE OPEN 1 %d 0 0 0 0 0", enc.open_time);
			mvsend_cmd2dsp( cmd );
		break;
		case 'a' *10+ '3':
			sprintf( cmd, "ENC NONE OFF 1 0 0 0 0 0 0");
			mvsend_cmd2dsp( cmd );
		break;
		case 'a' *10+ '4':
			draw_cmd_line( "ENC close time? ");
			get_string( temp_string, 3);
			if( *temp_string )
				enc.close_time=atoi(temp_string);
		break;
		case 'a' *10+ '5':
			draw_cmd_line( "ENC open time? ");
			get_string( temp_string, 3);
			if( *temp_string )
				enc.open_time=atoi(temp_string);
		break;
		case 'a' *10+ '6':
			sprintf( cmd, "ENC NONE RESET 1 1 0 0 0 0 0");
			mvsend_cmd2dsp( cmd );


		break;
		case 'b' *10+ '1':
			sprintf( cmd, "HYP NONE UP 1 %d 0 0 0 0 0", enc.lock_time);
			mvsend_cmd2dsp( cmd );
		break;
		case 'b' *10+ '2':
			sprintf( cmd, "HYP NONE DOWN 1 %d 0 0 0 0 0", enc.unlock_time);
			mvsend_cmd2dsp( cmd );
		break;
		case 'b' *10+ '3':
			sprintf( cmd, "HYP NONE OFF 1 %d 0 0 0 0 0", enc.unlock_time);
			mvsend_cmd2dsp( cmd );
		break;
		case 'c' *10+ '1':
			sprintf( cmd, "DSPPOWER OFF\nVDCPOWER OFF\nWHEELPOWER OFF\nTRANPOWER OFF\nROTPOWER OFF\nDECPOWER OFF\nRAPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'c' *10+ '2':
			sprintf( cmd, "MAINPOWER ON\nVDCPOWER ON\nCCDPOWER ON\nWHEELPOWER ON\nDECPOWER ON\nRAPOWER ON\nDSPPOWER ON\n");
			mvsend_cmd2ppc( cmd );
		break;
		case 'd' *10+ '1':
			sprintf( cmd, "MAINPOWER OFF\nCCDPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'd' *10+ '2':
			sprintf( cmd, "MAINPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'e' *10+ '1':
			sprintf( cmd, "RAPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'e' *10+ '2':
			sprintf( cmd, "RAPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'f' *10+ '1':
			sprintf( cmd, "DECPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'f' *10+ '2':
			sprintf( cmd, "DECPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'g' *10+ '1':
			sprintf( cmd, "ROTPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'g' *10+ '2':
			sprintf( cmd, "ROTPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'h' *10+ '1':
			sprintf( cmd, "TRANPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'h' *10+ '2':
			sprintf( cmd, "TRANPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'i' *10+ '1':
			sprintf( cmd, "DSPPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'i' *10+ '2':
			sprintf( cmd, "DSPPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'i' *10+ '3':
			sprintf( cmd, "DSPPOWER OFF");
			mvsend_cmd2ppc( cmd );
			sleep(1);
			sprintf( cmd, "DSPPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'j' *10+ '1':
			sprintf( cmd, "CCDPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'j' *10+ '2':
			sprintf( cmd, "CCDPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'k' *10+ '1':
			sprintf( cmd, "VDCPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'k' *10+ '2':
			sprintf( cmd, "VDCPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'm' *10+ '1':
			sprintf( cmd, "WHEELPOWER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'm' *10+ '2':
			sprintf( cmd, "WHEELPOWER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 'n' *10+ '2':
			sprintf( cmd, "DSPJUMPER OFF");
			mvsend_cmd2ppc( cmd );
		break;
		case 'n' *10+ '1':
			sprintf( cmd, "DSPJUMPER ON");
			mvsend_cmd2ppc( cmd );
		break;
		case 0:

		break;

		default:
			draw_line(MSG_LINE,' ');
			mvprintw( MSG_LINE, 0,"Error command");
	}
}

void run_ccd_select(int i_choice)
{

	static char 	cmd[80];
	static char 	temp_string[BUFSIZ];
	static char 	time_string[16];
	static char 	image_dir[80];
	static time_t	now;
	static struct tm tm_now;

	switch( i_choice )
	{
		case 'a':
			draw_cmd_line( "Enter set point? ");
			get_string( temp_string, 8);
			if( *temp_string )
				ccd.set_point=atof(temp_string);
		break;
		case 'b':
			sprintf( cmd, "camera on none %d 0 0 0", ccd.set_point);
			mvsend_cmd2ccd( cmd );
		break;
		case 'c':
			sprintf( cmd, "camera off none 0 0 0 0");
			mvsend_cmd2ccd( cmd );
		break;
		case 'd':
			draw_cmd_line( "Enter exposure time (float in second): ");
			get_string( temp_string, 10);
			if( *temp_string )
				p_tat_info->obs_info.expose_time=atof(temp_string);
		break;
		case 'e':

		break;
		case 'f':
		        ccd.shutter=ccd.shutter?0:1;

		break;
		case 'g':
			draw_cmd_line( "Enter image bin (1~8): ");
			get_string( temp_string, 1);
			if( *temp_string )
				ccd.bin=atoi(temp_string);
		break;
		case 'h':
		        init_ccd_setting();
		break;

		case 'i':
			/*sprintf( cmd, "camera query none 0 0 0 0");
			mvsend_cmd2ccd( cmd );
			*/
		break;

		case 'j':
			time(&now);
			localtime_r(&now, &tm_now);
			strftime( time_string, 16, "20%y%m%d%H%M%S", &tm_now);
			sprintf(image_dir,"%s/%d%02d%02d",
				CCD_IMAGE_DIR,
				tm_now.tm_year+1900,
				tm_now.tm_mon+1,
				tm_now.tm_mday);
					mkdir( image_dir, S_IRWXU|S_IRWXG);
					ccd.expose_time = p_tat_info->obs_info.expose_time;
			sprintf( cmd, "camera takeimage %s/%s%s_ex%.2f.fit %.2f %d %d",
						image_dir, ccd.prefix, time_string, ccd.expose_time, 
						ccd.expose_time, ccd.shutter, ccd.bin);
			mvsend_cmd2ccd( cmd );
		break;
		case 'k':
			sprintf( cmd, "camera abort none 0 0 0 0");
			mvsend_cmd2ccd( cmd );
		break;
		case 'l':
		        sprintf( cmd, "camera shutdown none 0 0 0 0");
			mvsend_cmd2ccd( cmd );
		break;
		case 'm':
			draw_cmd_line( "Enter prefix name of image: ");
			get_string( temp_string, CCD_PREFIX_SIZE-1);
			if( *temp_string )
				strcpy(ccd.prefix , temp_string);
		break;
		case 0:

		break;

		default:
			draw_line(MSG_LINE,' ');
			mvprintw( MSG_LINE, 0,"Error command");
	}

}

void run_fli_select(int i_choice)
{
	char fli_cmd[80];
	char temp_string[BUFSIZ];
	long move_focuser;
	

	switch( i_choice )
	{
		case 'a':
			draw_cmd_line( "New step? ");
			get_string( temp_string, 8);
// 			if( *temp_string )
			fli.move_rel_pos=atoi(temp_string);
		break;

		case 'b':
			move_focuser = p_tat_info->fli_info.focuser_curr_position + fli.move_rel_pos;
			sprintf(fli_cmd,"FOCUS %ld",move_focuser);
			mvsend_cmd2fli(fli_cmd);
		break;

		case 'c':
			move_focuser = p_tat_info->fli_info.focuser_curr_position -fli.move_rel_pos;
			sprintf(fli_cmd,"FOCUS %ld",move_focuser);
			mvsend_cmd2fli(fli_cmd);
		break;

		case 'd':
			sprintf(fli_cmd,"FOCUS 0");
			mvsend_cmd2fli(fli_cmd);

		break;

		case 'e':
			draw_cmd_line( "Move to position (0-105000)? ");
			get_string( temp_string, 8);
			if( *temp_string )
				move_focuser=atol(temp_string);
			
			sprintf(fli_cmd,"FOCUS %ld",move_focuser);
			mvsend_cmd2fli(fli_cmd);
		break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
			fli.filter_pos_setting = i_choice-48;//48 is zero in ASCII
			sprintf(fli_cmd,"WHEEL %d",fli.filter_pos_setting);
			mvsend_cmd2fli(fli_cmd);
		break;
		case 0:
		break;
		default:
			draw_line(MSG_LINE,' ');
			mvprintw(MSG_LINE, 0,"Error command");
		break;
	}//switch( i_choice )

}

void run_obs_select(int i_choice)
{

	static char 	cmd[80];
	static char 	temp_string[BUFSIZ];
	int pixel,i;
	int N_filters, filter_seq[6],filter_exp_time[6],filter_obs_time[6];
	char Filter_string[50];

	switch( i_choice )
	{
		case 'a':
			draw_cmd_line( "Enter New End Time? (YYYYmmddHHMM) ");
			get_string( temp_string, 12);
			if( *temp_string )
				p_tat_info->obs_info.end_time=posixtime(temp_string,13);
		break;



		case 'b':
			draw_cmd_line( "Enter Filter sequence? (char(int)char(int)..) ");
			get_string( Filter_string, 50);
			
			N_filters = read_filter_string(Filter_string,filter_seq,filter_exp_time,filter_obs_time);
			
			if(N_filters)
			{
				p_tat_info->obs_info.N_filters= N_filters;
				for(i=0; i< N_filters;i++)
				{
					p_tat_info->obs_info.filter_seq[i]= filter_seq[i];
					p_tat_info->obs_info.filter_exp_time[i]= filter_exp_time[i];
					p_tat_info->obs_info.filter_obs_time[i]= filter_obs_time[i];
				}
				
			}
			else 
				mvprintw( MSG_LINE, 0,"Error Filter sequence");
			
		break;


		case 'c':/* Move RA direction */
			if(!pix.ra_pix)break; //avoid dividing by 0
			else pixel= pix.ra_pix;
			if(pix.ra_dir) pixel*= -1;//Negative movement

			move_pixel(pixel,0,0);
		break;

		case '1'://Change RA direction
			if( pix.ra_dir ) pix.ra_dir =0;
			else pix.ra_dir =1;
		break;
		case '2':
			if( pix.ra_pix < 510 ) pix.ra_pix +=50;
			else pix.ra_pix =0;
// 		draw_cmd_line( "Enter RA pixels: ");
// 		get_string( temp_string, 3);
// 		if( *temp_string )
// 		{
// 				pix.ra_pix=atoi(temp_string);
// 				if(pix.ra_pix<0)pix.ra_pix =0;
// 				else if(pix.ra_pix>510)pix.ra_pix =510;
//
// 				if((pix.ra_pix % 10) < 5) {pix.ra_pix /= 10;pix.ra_pix *= 10;}//only multiples of 10
// 				else {pix.ra_pix /= 10;pix.ra_pix = (pix.ra_pix*10)+10;}
// 		}
		break;
		case 'd':/* Move DEC direction */
			if(!pix.dec_pix)break; //avoid dividing by 0
			else pixel = pix.dec_pix;
			if(pix.dec_dir) pixel*= -1;//Negative movement

			move_pixel(0,pixel,0);
		break;
		case '3'://Change DEC direction
			if( pix.dec_dir ) pix.dec_dir =0;
			else pix.dec_dir =1;

		break;
		case '4':
			if( pix.dec_pix < 510 ) pix.dec_pix +=50;
			else pix.dec_pix =0;
// 		draw_cmd_line( "Enter DEC pixels: ");
// 		get_string( temp_string, 3);
// 		if( *temp_string )
// 		{
// 				pix.dec_pix=atoi(temp_string);
// 				if(pix.dec_pix<0)pix.dec_pix =0;
// 				else if(pix.dec_pix>510)pix.dec_pix =510;
//
// 				if((pix.dec_pix % 10) < 5) {pix.dec_pix /= 10;pix.dec_pix *= 10;}
// 				else {pix.dec_pix /= 10;pix.dec_pix = (pix.dec_pix*10)+10;}
// 		}
		break;
		case 'e':/* Stop telescope for 10 seconds */
			sprintf( cmd, "RA POS FREQUENCY 1 1 9973 0 0 0 0\n");
			mvsend_cmd2dsp( cmd );
			sleep(10);
			sprintf( cmd, "RA POS FREQUENCY 1 30030 9973 0 0 0 0\n");
			mvsend_cmd2dsp( cmd );
		break;
		case 0:
			/* must have this case. */
		break;

		default:
			draw_line(MSG_LINE,' ');
			mvprintw( MSG_LINE, 0,"Error command");
	}

}

void run_mon_select(int i_choice)
{
	static char 	cmd[80];
	static char 	temp_string[BUFSIZ];

	switch( i_choice )
	{

		case 0:
			/* must have this case. */
		break;

		default:
			draw_line(MSG_LINE,' ');
			mvprintw( MSG_LINE, 0,"Error command");
	}

}

void run_manual_select(int i_choice)
{
	static char 	cmd[80];
	static char 	temp_string[BUFSIZ];
	int		res,pixel;
	pthread_t	thread_id;
	time_t 		now;
	struct tm 	tm_now;
	static char	image_dir[80];
	char *dsp_cmd;
	dsp_cmd = temp_string;

	switch( i_choice )
	{
		case '2':
			/* star_track */

			manual.ra_stop_moving=0;
			manual.dec_stop_moving=0;
			res=pthread_create(&thread_id,NULL,star_track,NULL);
			if(res!=0)
			{
				draw_line(MSG_LINE,' ');
				mvprintw( MSG_LINE, 0,"pthread_create(manual_return_ra) failed");

			}
			mvprintw( MSG_LINE, 0, "run star_track");
		break;

		case 'a':
			/* Change RA hours */
			draw_cmd_line( "Enter New RA hours? ");
			get_string( temp_string, 2);
			if( *temp_string )
			{
				manual.ra.hh=atoi(temp_string);
				p_tat_info->obs_info.curr_ra=(float)manual.ra.hh+(float)manual.ra.mm/60.0+(float)manual.ra.ss/3600.0;
			}
		break;

		case 'b':
			/* Change RA minutes */
			draw_cmd_line( "Enter New RA minutes? ");
			get_string( temp_string, 2);
			if( *temp_string )
			{
				manual.ra.mm=atoi(temp_string);
				p_tat_info->obs_info.curr_ra=(float)manual.ra.hh+(float)manual.ra.mm/60.0+(float)manual.ra.ss/3600.0;
			}
		break;
		case 'c':
			/* Change RA */
			draw_cmd_line( "Enter New RA seconds? ");
			get_string( temp_string, 2);
			if( *temp_string )
			{
				manual.ra.ss=atoi(temp_string);
				p_tat_info->obs_info.curr_ra=(float)manual.ra.hh+(float)manual.ra.mm/60.0+(float)manual.ra.ss/3600.0;
			}
		break;


		case 'd':
			/* Change DEC sign */
				manual.dec.hh=manual.dec.hh<0?1:-1;
				p_tat_info->obs_info.curr_dec=(float)manual.dec.hh*((float)manual.dec.dd+(float)manual.dec.mm/60.0+(float)manual.dec.ss/3600.0);
		break;

		case 'e':
			/* Change DEC degree */
			draw_cmd_line( "Enter New DEC degrees? ");
			get_string( temp_string, 2);
			if( *temp_string )
			{
				manual.dec.dd=atoi(temp_string);
				p_tat_info->obs_info.curr_dec=(float)manual.dec.hh*((float)manual.dec.dd+(float)manual.dec.mm/60.0+(float)manual.dec.ss/3600.0);
			}
		break;

		case 'f':
			/* Change DEC minutes */
			draw_cmd_line( "Enter New DEC minutes? ");
			get_string( temp_string, 2);
			if( *temp_string )
			{
				manual.dec.mm=atoi(temp_string);
				p_tat_info->obs_info.curr_dec=(float)manual.dec.hh*((float)manual.dec.dd+(float)manual.dec.mm/60.0+(float)manual.dec.ss/3600.0);
			}
		break;
		case 'g':
			/* Change DEC */
			draw_cmd_line( "Enter New DEC seconds? ");
			get_string( temp_string, 2);
			if( *temp_string )
			{
				manual.dec.ss=atoi(temp_string);
				p_tat_info->obs_info.curr_dec=(float)manual.dec.hh*((float)manual.dec.dd+(float)manual.dec.mm/60.0+(float)manual.dec.ss/3600.0);
			}
		break;

		case 'h':
			/* Change Expose Time */
			draw_cmd_line( "Enter New Expose Time? ");
			get_string( temp_string, 8);
			if( *temp_string )
				p_tat_info->obs_info.expose_time=atof(temp_string);
		break;


		case '3':
			/* Take Image */

			res=pthread_create(&thread_id,NULL,take_image,NULL);
			if(res!=0)
			{
				draw_line(MSG_LINE,' ');
				mvprintw( MSG_LINE, 0,"pthread_create(take_image) failed");

			}
			mvprintw( MSG_LINE, 0, "run take_image");

		break;

		case 'i':
			/* Get image prefix */
			draw_cmd_line( "Enter prefix name of image: ");
			get_string( temp_string, CCD_PREFIX_SIZE-1);
			if( *temp_string )
				strcpy(ccd.prefix , temp_string);
		break;

		case 'j':
			/* change cycle time*/
			draw_cmd_line( "Enter New Cycle Time (int) ? ");
			get_string( temp_string, 8);
			if( *temp_string )
			{
				if( atoi(temp_string) < (int)(p_tat_info->obs_info.expose_time+0.5) + image_download_time)
				{
					/* give message ? */
				}
				else
				{
					manual.ccd_cycle_time = atoi(temp_string);
				}
			}
		break;
		case 'k':
			/* change image number */
			draw_cmd_line( "Enter New Image Number ? ");
			get_string( temp_string, 8);
			if( *temp_string )
			{
				if( atoi(temp_string) < 1 )
				{
					manual.ccd_image_number=1;
				}
				else
					manual.ccd_image_number=atoi(temp_string);
			}
		break;
		case 'l':
			manual.ccd_stop_taking_image=1;
		break;
		case 'm':
			/* Change Integral minute flag */
				manual.ccd_int_min=manual.ccd_int_min>0?0:1;

		break;

		case '1':
			wclear(star_win);
			/* Return RA and DEC */
			res=pthread_create(&thread_id,NULL,manual_return_both,NULL);
			if(res!=0)
			{
				draw_line(MSG_LINE,' ');
				mvprintw( MSG_LINE2, 0,"pthread_create(manual_return_both) failed");

			}
		break;

		case '4':
			/* Stop RA and DEC */
			p_tat_info->obs_info.status = STOP;
			manual.ra_stop_moving=manual.dec_stop_moving=1;
			sprintf( dsp_cmd, "RA %s FREQUENCY 1 1 65534 0 0 0 0\nDEC %s FREQUENCY 1 1 65534 0 0 0 0\n",
				p_tat_info->dsp_info.ra.dir?"POS":"NEG", p_tat_info->dsp_info.dec.dir?"POS":"NEG");
			pthread_mutex_lock( &mutex_send_cmd2dsp );
			mvsend_cmd2dsp( dsp_cmd );
			pthread_mutex_unlock( &mutex_send_cmd2dsp );
		break;


		case '5':/* Move RA direction */
			if(!pix.ra_pix)break; //avoid dividing by 0
			else pixel= pix.ra_pix;
			if(pix.ra_dir) pixel*= -1;//Negative movement

			move_pixel(pixel,0,0);
		break;

		case 'w'://Change RA direction
			if( pix.ra_dir ) pix.ra_dir =0;
			else pix.ra_dir =1;
		break;
		case 'v':
			if( pix.ra_pix < 510 ) pix.ra_pix +=50;
			else pix.ra_pix =0;
// 		draw_cmd_line( "Enter RA pixels: ");
// 		get_string( temp_string, 3);
// 		if( *temp_string )
// 		{
// 				pix.ra_pix=atoi(temp_string);
// 				if(pix.ra_pix<0)pix.ra_pix =0;
// 				else if(pix.ra_pix>510)pix.ra_pix =510;
//
// 				if((pix.ra_pix % 10) < 5) {pix.ra_pix /= 10;pix.ra_pix *= 10;}//only multiples of 10
// 				else {pix.ra_pix /= 10;pix.ra_pix = (pix.ra_pix*10)+10;}
// 		}
		break;
		case '6':/* Move DEC direction */
			if(!pix.dec_pix)break; //avoid dividing by 0
			else pixel = pix.dec_pix;

			if(pix.dec_dir) pixel*= -1;//Negative movement

			move_pixel(0,pixel,0);
		break;
		case 'y'://Change DEC direction
			if( pix.dec_dir ) pix.dec_dir =0;
			else pix.dec_dir =1;

		break;
		case 'z':
			if( pix.dec_pix < 510 ) pix.dec_pix +=50;
			else pix.dec_pix =0;
// 		draw_cmd_line( "Enter DEC pixels: ");
// 		get_string( temp_string, 3);
// 		if( *temp_string )
// 		{
// 				pix.dec_pix=atoi(temp_string);
// 				if(pix.dec_pix<0)pix.dec_pix =0;
// 				else if(pix.dec_pix>510)pix.dec_pix =510;
//
// 				if((pix.dec_pix % 10) < 5) {pix.dec_pix /= 10;pix.dec_pix *= 10;}
// 				else {pix.dec_pix /= 10;pix.dec_pix = (pix.dec_pix*10)+10;}
// 		}
		break;
		case '7':/* Stop telescope for 10 seconds */
			sprintf( cmd, "RA POS FREQUENCY 1 1 9973 0 0 0 0\n");
			mvsend_cmd2dsp( cmd );
			sleep(10);
			sprintf( cmd, "RA POS FREQUENCY 1 30030 9973 0 0 0 0\n");
			mvsend_cmd2dsp( cmd );
		break;
		case '8':
			/* Move to safe position */
			res=pthread_create(&thread_id,NULL,manual_park_both,NULL);
			if(res!=0)
			{
				draw_line(MSG_LINE,' ');
				mvprintw( MSG_LINE2, 0,"pthread_create(manual_park_both) failed");
			}
		case 0:
			/* must have this case. */
		break;

		default:
			draw_line(MSG_LINE,' ');
			mvprintw( MSG_LINE, 0,"Error command");
	}
	wrefresh(star_win);

}


/* run ctl daemon select */
void run_ctl_select(int i_choice)
{
	static char 	cmd[80];
	static char 	temp_string[BUFSIZ];

	switch( i_choice )
	{
		case 'a':
			sprintf( cmd, "alldaemon stop");
					mvsend_cmd2ctl( cmd );
		break;
		case 'b':
			sprintf( cmd, "alldaemon start");
					mvsend_cmd2ctl( cmd );
		break;
		case 'c':
			sprintf( cmd, "alldaemon restart");
                        mvsend_cmd2ctl( cmd );
		break;
		case '1':
			sprintf( cmd, "dspdaemon stop");
                        mvsend_cmd2ctl( cmd );
		break;
		case '2':
			sprintf( cmd, "dspdaemon start");
                        mvsend_cmd2ctl( cmd );
		break;
		case '3':
			sprintf( cmd, "dspdaemon restart");
                        mvsend_cmd2ctl( cmd );
		break;
		case '4':
			sprintf( cmd, "ccddaemon stop");
                        mvsend_cmd2ctl( cmd );
		break;
		case '5':
			sprintf( cmd, "ccddaemon start");
                        mvsend_cmd2ctl( cmd );
		break;

		case '6':
			sprintf( cmd, "ccddaemon restart");
                        mvsend_cmd2ctl( cmd );
		break;

		case '7':
			sprintf( cmd, "ppcdaemon stop");
                        mvsend_cmd2ctl( cmd );
		break;
		case '8':
			sprintf( cmd, "ppcdaemon start");
                        mvsend_cmd2ctl( cmd );
		break;
		case '9':
			sprintf( cmd, "ppcdaemon restart");
				mvsend_cmd2ctl( cmd );
		break;

		case 'd':
			sprintf( cmd, "pwrdaemon stop");
                        mvsend_cmd2ctl( cmd );
		break;
		case 'e':
			sprintf( cmd, "pwrdaemon start");
                        mvsend_cmd2ctl( cmd );
		break;
		case 'f':
			sprintf( cmd, "pwrdaemon restart");
                        mvsend_cmd2ctl( cmd );
		break;
		case 'g':
			sprintf( cmd, "flidaemon stop");
                        mvsend_cmd2ctl( cmd );
		break;
		case 'h':
			sprintf( cmd, "flidaemon start");
                        mvsend_cmd2ctl( cmd );
		break;
		case 'i':
			sprintf( cmd, "flidaemon restart");
                        mvsend_cmd2ctl( cmd );
		break;
		case 'j':
			sprintf( cmd, "lstdaemon stop");
                        mvsend_cmd2ctl( cmd );
		break;
		case 'k':
			sprintf( cmd, "lstdaemon start");
                        mvsend_cmd2ctl( cmd );
		break;
		case 'l':
			sprintf( cmd, "lstdaemon restart");
                        mvsend_cmd2ctl( cmd );
		break;
		case 0:

		break;

		default:
			draw_line(MSG_LINE,' ');
			mvprintw( MSG_LINE, 0,"Error command");
	}

}

/* run pwr select */
void run_pwr_select(int i_choice)
{
	static	char cmd[80];
	char	temp_string[BUFSIZ];

	switch( i_choice )
	{
		case 'a' *10+ '1':
			sprintf( cmd, "R1POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'a' *10+ '2':
			sprintf( cmd, "R1POWER ON");
			mvsend_cmd2pwr( cmd );
		break;

		break;
		case 'b' *10+ '1':
			sprintf( cmd, "R2POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'b' *10+ '2':
			sprintf( cmd, "R2POWER ON");
			mvsend_cmd2pwr( cmd );
		break;

		case 'c' *10+ '1':
			sprintf( cmd, "R3POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'c' *10+ '2':
			sprintf( cmd, "R3POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'd' *10+ '1':
			sprintf( cmd, "R4POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'd' *10+ '2':
			sprintf( cmd, "R4POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'e' *10+ '1':
			sprintf( cmd, "R5POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'e' *10+ '2':
			sprintf( cmd, "R5POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'f' *10+ '1':
			sprintf( cmd, "R6POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'f' *10+ '2':
			sprintf( cmd, "R6POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'g' *10+ '1':
			sprintf( cmd, "L1POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'g' *10+ '2':
			sprintf( cmd, "L1POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'h' *10+ '1':
			sprintf( cmd, "L2POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'h' *10+ '2':
			sprintf( cmd, "L2POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'i' *10+ '1':
			sprintf( cmd, "L3POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'i' *10+ '2':
			sprintf( cmd, "L3POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'j' *10+ '1':
			sprintf( cmd, "L4POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'j' *10+ '2':
			sprintf( cmd, "L4POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'k' *10+ '1':
			sprintf( cmd, "L5POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'k' *10+ '2':
			sprintf( cmd, "L5POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'l' *10+ '1':
			sprintf( cmd, "L6POWER OFF");
			mvsend_cmd2pwr( cmd );
		break;
		case 'l' *10+ '2':
			sprintf( cmd, "L6POWER ON");
			mvsend_cmd2pwr( cmd );
		break;
		case 'm' *10+ '1':
			sprintf( cmd, "R1POWER OFF\nR2POWER OFF\nR3POWER OFF\nR4POWER OFF\nR5POWER OFF\nR6POWER OFF\n");
			mvsend_cmd2pwr( cmd );
		break;
		case 'm' *10+ '2':
			sprintf( cmd, "R1POWER ON\nR2POWER ON\nR3POWER ON\nR4POWER ON\nR5POWER ON\nR6POWER ON\n");
			mvsend_cmd2pwr( cmd );
		break;
		case 'n' *10+ '1':
			sprintf( cmd, "L1POWER OFF\nL2POWER OFF\nL3POWER OFF\nL4POWER OFF\nL5POWER OFF\nL6POWER OFF\n");
			mvsend_cmd2pwr( cmd );
		break;
		case 'n' *10+ '2':
			sprintf( cmd, "L1POWER ON\nL2POWER ON\nL3POWER ON\nL4POWER ON\nL5POWER ON\nL6POWER ON\n");
			mvsend_cmd2pwr( cmd );
		break;
		case 'p' *10+ '1':
			sprintf( cmd, "R1POWER OFF\nR2POWER OFF\nR3POWER OFF\nR4POWER OFF\nR5POWER OFF\nR6POWER OFF\nL1POWER OFF\nL2POWER OFF\nL3POWER OFF\nL4POWER OFF\nL5POWER OFF\nL6POWER OFF\n");
			mvsend_cmd2pwr( cmd );
		break;
		case 'p' *10+ '2':
			sprintf( cmd, "R1POWER ON\nR2POWER ON\nR3POWER ON\nR4POWER ON\nR5POWER ON\nR6POWER ON\nL1POWER ON\nL2POWER ON\nL3POWER ON\nL4POWER ON\nL5POWER ON\nL6POWER ON\n");
			mvsend_cmd2pwr( cmd );
		break;

		case 0:

		break;

		default:
			draw_line(MSG_LINE,' ');
			mvprintw( MSG_LINE, 0,"Error command");
	}
}


void sendCmd2( char *cmd )
{
	mvprintw( MSG_LINE, 20, "%s", cmd);

}

void init_tel_setting()
{
	ra.dir=1;
	ra.reg=208;
	ra.start=1;
	ra.end=15;
	dec.dir=1;
	dec.reg=208;
	dec.start=1;
	dec.end=15;
}


void init_enc_setting()
{
    enc.open_time=38;
    enc.close_time=60;
    enc.reset_time=10;
    enc.lock_time=20;
    enc.unlock_time=20;
}

void init_ccd_setting()
{
	ccd.set_point=-5;
	ccd.bin=1;
	ccd.shutter=1;
	sprintf( ccd.prefix, "test");
	if( p_tat_info->obs_info.expose_time == 0) p_tat_info->obs_info.expose_time = 0.05;
}

void init_fli_setting() 	//init values for the Sequence
{
	fli.home_device=0;
	fli.move_rel_pos=500;
	fli.filter_pos_setting=0;
}

void int_filters_setting()
{
	get_filter_array_char(filter_pos);
}


void init_manual_setting()
{
	manual.ra.hh = (int)    p_tat_info->obs_info.curr_ra;
	manual.ra.mm = (int)  ((p_tat_info->obs_info.curr_ra - manual.ra.hh )*60.0);
	manual.ra.ss = (int) (((p_tat_info->obs_info.curr_ra - manual.ra.hh )*60.0 - manual.ra.mm )*60.0);

	manual.dec.hh = p_tat_info->obs_info.curr_dec<0?-1:1;
	manual.dec.dd = (int)    fabs(p_tat_info->obs_info.curr_dec);
	manual.dec.mm = (int)  ((fabs(p_tat_info->obs_info.curr_dec) - manual.dec.dd )*60.0);
	manual.dec.ss = (int) (((fabs(p_tat_info->obs_info.curr_dec) - manual.dec.dd )*60.0 - manual.dec.mm )*60.0);
	manual.ccd_image_number=1;
	manual.ccd_cycle_time=(int)(p_tat_info->obs_info.expose_time+0.5)+image_download_time;
	manual.time_counter=0;
	manual.ccd_curr_number=0;
	manual.ccd_int_min=0;

}

void get_string(char *string, int max_string)
{
    int len;
    attrset( COLOR_PAIR(GET_STRING_COLOR)| A_BOLD);
    getnstr(string, max_string);
    attrset( COLOR_PAIR(0)| A_DIM);
    len = strlen(string);
    if (len > 0 && string[len - 1] == '\n')
        string[len - 1] = '\0';
/*        else
        	*string = '\0';
  */

}

void *manual_return_ra(void *arg)
{
	char dsp_cmd[80];

	if( manual.ra_ctrl ==0 )
	{
		manual.ra_ctrl = 1;
		manual.ra_stop_moving =0;
		while(1)
		{
			if( manual.ra_stop_moving )
			{
				/* Stop RA moving */
				manual.ra_ctrl = 0;
				pthread_exit(NULL);

			}
			switch( p_tat_info->dsp_info.ra.status )
			{
				case NearOrigin:

				break;
				case InOrigin:
					manual.ra_ctrl =0;
					pthread_exit(NULL);
				break;
				case ReturnToOrigin:
					sprintf( dsp_cmd, "RA NEG FREQUENCY 1 9999 208 0 0 0 0\n");
					pthread_mutex_lock( &mutex_send_cmd2dsp );
					mvsend_cmd2dsp( dsp_cmd );
					pthread_mutex_unlock( &mutex_send_cmd2dsp );

				break;
				case NormalStop:
					sprintf( dsp_cmd, "RA NEG RESET 1 9999 417 0 0 0 0\n");
					pthread_mutex_lock( &mutex_send_cmd2dsp );
					mvsend_cmd2dsp( dsp_cmd );
					pthread_mutex_unlock( &mutex_send_cmd2dsp );
				break;
				case NormalObsCycle:
					if( p_tat_info->dsp_info.ra.dir == 1 )
					{
						sprintf( dsp_cmd, "RA POS FREQUENCY 1 1 65534 0 0 0 0\n");
						pthread_mutex_lock( &mutex_send_cmd2dsp );
						mvsend_cmd2dsp( dsp_cmd );
						pthread_mutex_unlock( &mutex_send_cmd2dsp );
					}
				break;
			}
			sleep (5);

		}
	}
}

void *manual_return_dec(void *arg)
{
	char dsp_cmd[80];


	if( manual.dec_ctrl ==0 )
	{
		manual.dec_ctrl = 1;
		manual.dec_stop_moving =0;
		while(1)
		{
			if( manual.dec_stop_moving )
			{
				/* Stop DEC moving */
				manual.dec_ctrl = 0;
				pthread_exit(NULL);
			}
			switch( p_tat_info->dsp_info.dec.status )
			{
				case NearOrigin:

				break;
				case InOrigin:
					manual.dec_ctrl =0;
					pthread_exit(NULL);
				break;
				case ReturnToOrigin:
					sprintf( dsp_cmd, "DEC NEG FREQUENCY 1 9999 208 0 0 0 0\n");
					pthread_mutex_lock( &mutex_send_cmd2dsp );
					mvsend_cmd2dsp( dsp_cmd );
					pthread_mutex_unlock( &mutex_send_cmd2dsp );


				break;
				case NormalStop:
					sprintf( dsp_cmd, "DEC NEG RESET 1 9999 417 0 0 0 0\n");
					pthread_mutex_lock( &mutex_send_cmd2dsp );
					mvsend_cmd2dsp( dsp_cmd );
					pthread_mutex_unlock( &mutex_send_cmd2dsp );
				break;
				case NormalObsCycle:
					if( p_tat_info->dsp_info.ra.dir == 1)
					{
						sprintf( dsp_cmd, "DEC POS FREQUENCY 1 1 65534 0 0 0 0\n");
						pthread_mutex_lock( &mutex_send_cmd2dsp );
						mvsend_cmd2dsp( dsp_cmd );
						pthread_mutex_unlock( &mutex_send_cmd2dsp );
					}
				break;
			}
			sleep (5);

		}
	}

}

void *manual_return_both(void *arg)
{
	int             res;
	pthread_t       thread_id_ra, thread_id_dec;

	/* return RA */
	res=pthread_create(&thread_id_ra,NULL,manual_return_ra,NULL);
	if(res!=0)
	{
		draw_line(MSG_LINE,' ');
		mvprintw( MSG_LINE2, 0,"pthread_create(manual_return_ra) failed");

	}

	sleep (1);
	/* return DEC */
	res=pthread_create(&thread_id_dec,NULL,manual_return_dec,NULL);
	if(res!=0)
	{
		draw_line(MSG_LINE,' ');
		mvprintw( MSG_LINE2, 0,"pthread_create(manual_return_dec) failed");

	}
	p_tat_info->obs_info.status = Returning;
	pthread_join( thread_id_ra, NULL);
	pthread_join( thread_id_dec, NULL);
	if( 	p_tat_info->dsp_info.ra.status == InOrigin &&
		p_tat_info->dsp_info.dec.status == InOrigin )
	{
		p_tat_info->obs_info.status = InOrigin;
	}

}


void *manual_park_dec(void *arg)
{
	char dsp_cmd[80];
	int res;
	pthread_t	thread_id;

	if( manual.dec_ctrl ==0 )
	{
		manual.dec_ctrl = 1;
		manual.dec_stop_moving =0;

		while(1)
		{
			if( p_tat_info->dsp_info.dec.origin )
			{
				/* move forward with 1 kHz */
				sprintf( dsp_cmd, "DEC POS FREQUENCY 1 15 417 0 0 0 0\n");
				mvsend_cmd2dsp_chk( dsp_cmd );

				/* wait 10 seconds */
				for( res=0;res<10;res++)
				{
					sleep(1);
					if(manual.dec_stop_moving)
						pthread_exit( (void*) 0 );
				}

				/* accelerate to 2kHz */
				sprintf( dsp_cmd, "DEC POS FREQUENCY 1 10 208 0 0 0 0\n");
				mvsend_cmd2dsp_chk( dsp_cmd );

				while(1)
				{
					if( 	p_tat_info->dsp_info.dec.status == NormalStop ||
						manual.dec_stop_moving)
					{
						manual.dec_ctrl = 0;
						pthread_exit( (void *)0 );
					}
					sleep(1);
				}
			}
		manual.dec_ctrl = 0;
		/* give ctrl permission to manual_return_dec */
		res=pthread_create(&thread_id, NULL, manual_return_dec, NULL);
		if( res !=0 )
		{
			mvprintw( MSG_LINE, 0, "pthread_create(manual_return_dec) failed\n");
			pthread_exit( (void *) 0);
		}
		pthread_join( thread_id, NULL);
		manual.dec_ctrl = 1;
		if( manual.dec_stop_moving )
		{
			manual.dec_ctrl = 0;
			pthread_exit( (void *) 0 );
		}
		} /* end while(1) */

	}

}
void *manual_park_ra(void *arg)
{
	char dsp_cmd[80];
	int res;
	pthread_t	thread_id;

	if( manual.ra_ctrl ==0 )
	{
		manual.ra_ctrl = 1;
		manual.ra_stop_moving =0;

		while(1)
		{

		if( p_tat_info->dsp_info.ra.origin )
		{
			/* move forward with 1 kHz */
			sprintf( dsp_cmd, "RA POS FREQUENCY 1 15 417 0 0 0 0\n");
			mvsend_cmd2dsp_chk( dsp_cmd );

			/* wait 10 seconds */
			for( res=0;res<10;res++)
			{
				sleep(1);
				if(manual.ra_stop_moving)
					pthread_exit( (void*) 0 );
			}

			/* accelerate to 2kHz */
			sprintf( dsp_cmd, "RA POS FREQUENCY 1 10 208 0 0 0 0\n");
			mvsend_cmd2dsp_chk( dsp_cmd );

			while(1)
			{
				if( 	p_tat_info->dsp_info.ra.status == NormalStop ||
					manual.ra_stop_moving)
				{
					manual.ra_ctrl = 0;
					pthread_exit( (void *)0 );
				}
				sleep(1);
			}
		}

		manual.ra_ctrl = 0;
		/* give ctrl permission to manual_return_ra */
		res=pthread_create(&thread_id, NULL, manual_return_ra, NULL);
		if( res !=0 )
		{
			mvprintw( MSG_LINE, 0, "pthread_create(manual_return_ra) failed\n");
			pthread_exit( (void *) 0);
		}
		pthread_join( thread_id, NULL);
		manual.ra_ctrl = 1;
		if( manual.ra_stop_moving )
		{
			manual.ra_ctrl = 0;
			pthread_exit( (void *) 0);
		}

		} /* end while(1) */

	}

}


void *manual_park_both(void *arg)
{
	int             res;
	pthread_t       thread_id_ra, thread_id_dec;
	/* return RA */
	res=pthread_create(&thread_id_ra,NULL,manual_park_ra,NULL);
	if(res!=0)
	{
		draw_line(MSG_LINE,' ');
		mvprintw( MSG_LINE2, 0,"pthread_create(manual_return_ra) failed");
	}
	sleep (1);
	/* return DEC */
	res=pthread_create(&thread_id_dec,NULL,manual_park_dec,NULL);
	if(res!=0)
	{
		draw_line(MSG_LINE,' ');
		mvprintw( MSG_LINE2, 0,"pthread_create(manual_return_dec) failed");
	}
	p_tat_info->obs_info.status = Returning;
	pthread_join( thread_id_ra, NULL);
	pthread_join( thread_id_dec, NULL);
	if( 	p_tat_info->dsp_info.ra.status == NormalStop &&
		p_tat_info->dsp_info.dec.status == NormalStop )
	{
		p_tat_info->obs_info.status = STOP;
	}
}

void *take_image(void *arg)
{
	char ccd_cmd[80];
	time_t now;
	time_t next;
	struct tm tm_now;
	char time_string[BUFSIZ];
	char image_dir[BUFSIZ];

	/* some code to check CCD being ready */
	//mvprintw( MSG_LINE2, 0,"Check CCD being ready");

	/* main loop for taking image */
	if( manual.ccd_ctrl == 0 )
	{

		manual.ccd_ctrl=1;

		/* initialize */
		manual.time_counter=manual.ccd_cycle_time;
		manual.ccd_stop_taking_image=0;

		if( manual.ccd_int_min == 1)
		{
		time(&now);
		now%=60;
		for(; now>0;)
		{

			wattrset( star_win, COLOR_PAIR(1)|A_BOLD);
			wprintw(star_win, "Waiting for integral minute: %2ld\r", 60-now);
			wattrset( star_win, COLOR_PAIR(0)|A_DIM);
			if( manual.ccd_stop_taking_image==1 )
			{
				 manual.ccd_ctrl=0;
				 pthread_exit( (void *) 0);
			}
			if( now < 3 )
				usleep(100000);
			else sleep(1);
			time(&now);
			next=now;
			now%=60;
		}
		wprintw( star_win,"\n");
		}

		for( manual.ccd_curr_number=0;
			manual.ccd_curr_number<manual.ccd_image_number;
			manual.ccd_curr_number++)
		{

			time(&now);
			next=now+manual.ccd_cycle_time;

			localtime_r(&now, &tm_now);
			strftime( time_string, 16, "20%y%m%d%H%M%S", &tm_now);
			sprintf(image_dir,"%s/%d%02d%02d",
					CCD_IMAGE_DIR,
					tm_now.tm_year+1900,
					tm_now.tm_mon+1,
					tm_now.tm_mday);
			mkdir( image_dir, S_IRWXU|S_IRWXG);
			ccd.expose_time = p_tat_info->obs_info.expose_time;
			sprintf( ccd_cmd, "camera takeimage %s/%s%s_ex%.2f.fit %.2f %d %d",
			  image_dir, ccd.prefix, time_string, ccd.expose_time, ccd.expose_time, ccd.shutter, ccd.bin);
			mvsend_cmd2ccd( ccd_cmd );
			wprintw(star_win, "%s/%s%s_ex%.2f.fit\n",
				image_dir, ccd.prefix, time_string, ccd.expose_time);

			manual.time_counter=manual.ccd_cycle_time;
			for(; manual.time_counter>0;)
			{
				if( manual.ccd_stop_taking_image == 1 )
				{
					if( manual.time_counter == 1)
					{
						manual.time_counter=0;

						manual.ccd_stop_taking_image = 0;
						manual.ccd_ctrl=0;
						pthread_exit( (void *) 0);
					}
					else
					{
						 wattrset( star_win, COLOR_PAIR(1)|A_BOLD);
						 wprintw( star_win, "Waiting to download image\r");
						 wattrset( star_win, COLOR_PAIR(0)|A_DIM);
					}
				}
				time(&now);
				manual.time_counter= (time_t)difftime(next,now);
				if( manual.time_counter <3 )
					usleep(100000);
				else
					sleep(1);
			}
		}
		manual.ccd_ctrl=0;
	}
	else
	{
		/* ccd resource is occupied  */
		attrset( COLOR_PAIR(1)|A_BOLD);
		mvprintw( MSG_LINE2, 0,"CCD resource is occupied");
		attrset( COLOR_PAIR(0)|A_DIM);
		pthread_exit( (void *) 0);
	}
}

void fill_filter_string(char * Filter_string)
{
	char temp[10],filtertype;
	int i,ff;
	
	if(p_tat_info->obs_info.N_filters)
	{
		sprintf(Filter_string,"");//reset the string
		for(i=0;i<p_tat_info->obs_info.N_filters;i++)
		{
			ff =p_tat_info->obs_info.filter_seq[i];
			if(ff>=0 && ff <FILTER_TOTAL_NUMBER)
				filtertype =  filter_pos[ff];
			else if( ff==-1)
				filtertype='D';
			else
				filtertype = 'X';

			sprintf(temp,"%c(%d) ",filtertype, p_tat_info->obs_info.filter_exp_time[i]);
			strcat(Filter_string,temp);
		}
	}
}
