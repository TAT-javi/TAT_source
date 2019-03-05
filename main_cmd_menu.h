#ifndef _MAIN_DSP_INFO_
#define _MAIN_DSP_INFO_
#include <curses.h>
#include "tat_info.h"
#define XXX     exit( EXIT_FAILURE);
#define HEAD_LINE	0
#define HEAD_HEIGHT	2
#define HEAD_BODY_SPLIT_LINE HEAD_LINE+HEAD_HEIGHT
#define BODY_LINE       HEAD_BODY_SPLIT_LINE+1
#define BODY_COL	35
#define TEL_SETTING_LINE	BODY_LINE+9
#define TEL_SETTING_COL		BODY_COL
#define MX_SETTING_LINE		BODY_LINE+5
#define MX_SETTING_COL		31
#define ENC_SETTING_LINE	BODY_LINE
#define ENC_SETTING_COL		71
#define CCD_SETTING_LINE	BODY_LINE
#define CCD_SETTING_COL		38

#define CMD_LINE        BODY_LINE+22
#define MSG_LINE	CMD_LINE+1
#define MSG_LINE2	CMD_LINE+2
#define INFO_LINE	CMD_LINE+3
#define ENC_INFO_LINE	INFO_LINE+7

#define STAR_WIN_Y	16
#define STAR_WIN_X	1
#define STAR_WIN_LINES	7
#define STAR_WIN_COLS	COLS-1

#define CCD_PREFIX_SIZE	16
#define CCD_IMAGE_DIR	"/home/observer/test_image"

/* color */
#define HEAD_MENU_COLOR	3
#define BODY_MENU_COLOR 6
#define CMD_LINE_COLOR	2
#define GET_STRING_COLOR 7

typedef struct
{
	int dir;
	int reg;
	int start;
	int end;
} st_tel_cmd;

typedef struct
{
	int move;
} st_mx_cmd;

typedef struct
{
	int open_time;
	int close_time;
	int reset_time;
	int lock_time;
	int unlock_time;
} st_enc_cmd;

typedef struct
{
	int set_point;
	float expose_time;
	int shutter;
	int bin;
	char prefix[CCD_PREFIX_SIZE];
} st_ccd_cmd;

typedef struct
{
	int hh;
	int dd;
	int mm;
	int ss;
} st_coord;

typedef struct
{
	char 	ra_ctrl;
	char 	dec_ctrl;
	char 	ra_stop_moving;
	char 	dec_stop_moving;
	st_coord	ra;
	st_coord	dec;
	char ccd_ctrl;
	char ccd_stop_taking_image;
	int ccd_image_number;
	int ccd_cycle_time;
	int ccd_curr_number;
	int time_counter;
	char ccd_int_min;
} st_manual_cmd;

typedef struct
{
	long move_rel_pos; //Move the focuser to a relative position
	int home_device; // wether move the device to home or not
	int filter_pos_setting; 
} st_fli_cmd;

typedef struct
{
	int ra_dir;//0=E , 1=W
	int ra_pix;
	int dec_dir;//0=N , 1=S
	int dec_pix;
} st_pix_move;


/* function prototype */
void draw_head_menu(const char **);
void draw_body_menu(const char **);
void draw_cmd_line(char*);
void draw_line(int, char);
void draw_tel_setting(st_tel_cmd* , int, int );
void draw_enc_setting( st_enc_cmd*, int, int );
void draw_ccd_setting( st_ccd_cmd*, int, int );
void draw_mx_setting( st_mx_cmd*, int, int );
void draw_manual_setting( int , int );
void draw_fli_setting( st_fli_cmd* , int , int );
void draw_obs_setting( int line, int col );

void run_menu_select(int*, int*);
void run_tel_select(int);
void run_ppc_select(int);
void run_enc_select(int);
void run_ccd_select(int);
void run_ctl_select(int);
void run_pwr_select(int);
void run_obs_select(int);
void run_manual_select(int);
void run_mon_select(int);
void run_fli_select(int);

int wordexp( char *);
void init_tel_setting();
void init_enc_setting();
void init_ccd_setting();
// void init_mx_setting();
void init_manual_setting();
void init_fli_setting();
void int_filters_setting();
void init_pix_move();

void get_string(char *, int);
void fill_filter_string(char * );
void sendCmd2( char * );

void draw_tel_status( st_dsp_info*, int);
void draw_enc_status( st_dsp_info*, int);
void draw_ccd_status_f6( st_ccd_info*, int);
void draw_ccd_status_u6( st_ccd_info*, int);
void draw_ppc_status( st_ppc_info*, int);
void draw_pwr_status( st_ppc_info*, int);
void draw_ctl_status( st_ctl_info*, int);
void draw_obs_status( st_obs_info*, int);
void draw_manual_status(int);
void draw_fli_status( st_fli_info* , int);

void *manual_return_ra(void *);
void *manual_return_dec(void *);
void *manual_return_both(void *);
void *manual_park_both(void *);
void *manual_park_ra(void *);
void *manual_park_dec(void *);
void *take_image(void *);


#endif
