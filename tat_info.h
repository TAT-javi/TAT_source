#ifndef _TATINFO_
#define _TATINFO_
#include <curses.h>
#include <time.h>
#define KEY_TAT_INFO	3205
#define PARAM_MAX_NUMBER 200

#define FILTER_TOTAL_NUMBER 7  /*Number of filer in the filter wheel */


#define Pursuing	0x01
#define Tracking	0x02
#define Returning	0x04
#define STOP		0x08
#define Moving		0x10
#define Flating		0x20
#define Darking		0x40

/* structure */
typedef struct
{
	int		reg;
	int		pulse;
	char	dir;
	char	status;
	char	pulsectl;
	int		refpulse;
	int		purpulse;
	char	origin;
} st_tel_info;

typedef struct
{
	int		move;
	char	mode;
} st_mx_info;


typedef struct
{
	char open;
	char close;
	char time;
	int opened_ls;
	int closed_ls;
} st_enc_info;

typedef struct
{
	char lock;
	char unlock;
	char time;
} st_latch_info;

typedef struct
{
	int			time_count;
	st_tel_info		ra;
	st_tel_info		dec;
	st_mx_info		rot;
	st_mx_info		tran;
	st_mx_info		wheel;
	st_enc_info		enc;
	st_latch_info	latch;
} st_dsp_info;

typedef struct
{
	float	curr_point;
	float	set_point;
    int     camera_status;
    int     server_connected;
    int     ccd_locked;
    int     cooler_status;
	int		cooler_mode;
    int     fan_status;
	int present;
} st_ccd_info;

typedef struct
{
	char	control;
	char	data;
	char	status;
} st_ppc_info;

typedef struct
{
  int	dspd;
  int	ccdd;
  int	ppcd;
  int	pwrd;
  int	flid;
  int	lstd;
  int	aobs;
} st_ctl_info;

typedef struct
{
	time_t  begin_time, end_time;
	int 	auto_observing;
	char 	status; /* Pursuing Tracking Returning Stop*/
	float   curr_ra;
	float   curr_dec;
	float   dest_ra;
	float   dest_dec;
	float	astro_ra; //from astrometry
	float	astro_dec; // from astrometry
	char 	target_name[100];
	char	flat_start[2]; //a = after, b = before, t = after and before, n = no flat
	int 	ccd_status; /* 0= idle,1= taking image,2=taking flat,3=taking dark*/
	int		cycle_time;
	int 	cycle_number;
	float	expose_time;
	char	recent_image[BUFSIZ];
	float	RA,DEC;
	int 	FOV;
	int		N_filters,current_filter;
	int		filter_seq[FILTER_TOTAL_NUMBER],filter_exp_time[FILTER_TOTAL_NUMBER];
	int 	filter_obs_time[FILTER_TOTAL_NUMBER],exp_time_changed[FILTER_TOTAL_NUMBER];
	char	filter_type;
} st_obs_info;


typedef struct
{
	int focuser_moving;
	long focuser_curr_position;
	long focuser_extent;
	long focuser_remainder_steps;
	int wheel_moving;
	int wheel_curr_position;
}st_fli_info;


typedef struct
{
	st_dsp_info dsp_info;
	st_ccd_info ccd_info;
	st_ppc_info ppc_info;
	st_ppc_info pwr_info;
	st_ctl_info ctl_info;
	st_obs_info obs_info;
	st_fli_info fli_info;
} st_tat_info;


/* function prototype */
void create_tat_info_shm( int *, st_tat_info **);
void remove_tat_info_shm( int *, st_tat_info *);

void init_dsp_info( st_dsp_info*);
void init_tel_info( st_tel_info*);
void init_mx_info( st_mx_info*);
void init_enc_info( st_enc_info*);
void init_latch_info( st_latch_info*);
void init_ccd_info( st_ccd_info*);
void init_ppc_info( st_ppc_info*);
void init_pwr_info( st_ppc_info*);
void init_ctl_info( st_ctl_info*);
void init_obs_info( st_obs_info*);
void init_fli_info( st_fli_info*);

void disp_dsp_info( st_dsp_info*);
void disp_tel_info( st_tel_info*);
void disp_mx_info( st_mx_info*);
void disp_enc_info( st_enc_info*);
void disp_latch_info( st_latch_info*);
void disp_ccd_info( st_ccd_info*);
void disp_ppc_info( st_ppc_info*);
void disp_pwr_info( st_ppc_info*);
void disp_ctl_info( st_ctl_info*);
void disp_obs_info( st_obs_info*);


void update_dsp_info(st_dsp_info *, int *);
void update_ccd_infof6( st_ccd_info*, float *);
void update_ccd_info( st_ccd_info*, float *);
void update_ppc_info( st_ppc_info*, char *);
void update_pwr_info( st_ppc_info*, char *);
void update_ctl_info( st_ctl_info*, char *);
void update_obs_info( st_obs_info *, st_obs_info *);
void update_obs_end_time( st_obs_info *, time_t );
void update_obs_expose_time( st_ccd_info *, float );
void update_obs_cycle_time( st_ccd_info *, int);
void update_obs_cycle_number( st_ccd_info *, int);

#endif


