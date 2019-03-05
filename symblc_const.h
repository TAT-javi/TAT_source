//#ifndef _SYMBLC_CONST_
#define  _SYMBLC_CONST_

/* 	DEFINE THE TAT SITE	*/

#undef HSINCHU
#undef TENERIFE
#undef KUNMING

#define TENERIFE // for configure.sh do not change

#ifdef HSINCHU

#define  LOCAL_LAT		 24.79  //Hsinchu local latitude (in deg)
#define  LOCAL_LONG		121.0  //Hsinchu local longitude(in deg)
#define  LOCAL_ALT		100.0  //Hsinchu local altitude (in meters)
#define  LOCAL_UT		  0.0   //difference between system time and UT time
#define  SITE			"TW"	//short name for the site
#define  OBSERVATORY	"HSINCHU" //long name for the observatory

#define PPC_DATA 		0xb000
#define PWR_DATA 		0xb200

#define MIN_ALTITUDE 5.3 //Minimum altitude to start observation
#define F6

#define IP 		"140.114.80.239"

#elif defined TENERIFE

#define  LOCAL_LAT		 28.3003  //TF local latitude (in deg)
#define  LOCAL_LONG		-16.5122  //TF local longitude(in deg)
#define  LOCAL_ALT		2390.0  //TF local altitude (in meters)
#define  LOCAL_UT		  0.0   //difference between system time and UT time
#define  SITE			"TF"	//short name for the sitey
#define  OBSERVATORY	"TENERIFE" //long name for the observatory

#define PPC_DATA 		0xc000
#define PWR_DATA 		0xc200

#define DUST_SENSOR
#define F6
#define MIN_ALTITUDE -4.0 //Minimum altitude to start observation

#define IP "161.72.25.37"

#elif defined KUNMING

#define  LOCAL_LAT		26.696 //KU local latitude (in deg)
#define  LOCAL_LONG		100.03  //KU local longitude(in deg)
#define  LOCAL_ALT		3240.0  //KU local altitude (in meters)
#define  LOCAL_UT		0.0   //difference between system time and UT time
#define  SITE			"KU"	//short name for the site
#define  OBSERVATORY	"LI-JIANG" //long name for the observatory


#define PPC_DATA 		0xb000
#define PWR_DATA 		0xb200

#define U6
#define MIN_ALTITUDE -5.3 //Minimum altitude to start observation

#define IP "116.55.97.227"

#endif

/*CCD */
#define ROW_PIXELS 1024
#define COL_PIXELS 1024
#define CCD_READOUT_TIME 10

/* PULSE PER PIXELS*/
#define PULSEPERPIX_DEC 3.635 //for DEC
#define PULSEPERPIX_RA 7.0643   // for RA

/* Time and Filter wheel*/
#define GmtOffSet	(LOCAL_LONG *(24.0/360.0))
#define ADJUST_TIME		15 /*Time for adjust the fov during the observing cycle*/
#define MOVE_WHEEL_TIME 15 /*Time that takes the filter wheel to move to filter*/
#define FILTER_TOTAL_NUMBER 7  /*Number of filer in the filter wheel */


/*     common.func symbolic constants*/
	
#define DAEMON_PATH			"/home/tat/daemon/"
#define APP_PATH			"/home/observer/"
#define VAR_DEF_FILENAME	"/home/observer/tat_parameter.dat"
#define TIME_TABLE_FILENAME	"/home/observer/star/data/star_input.dat"
#define REF_PAR_FILENAME	"/home/observer/ref_parameter.dat"
#define CALIBRATE_PATH 		"/home/observer/star/calibrate"
#define IMAGE_PATH 			"/home/observer/star/image"

/* LOG */
#define AUTO_OBSERVE_LOG_PATH	"/home/observer/log/auto_observation/"
#define STAR_TRACK_LOG_PATH		"/home/observer/log/star_track/"
#define AUTO_FLAT_LOG_PATH		"/home/observer/log/auto_flat/"
#define AUTO_DARK_LOG_PATH		"/home/observer/log/auto_dark/"
#define AUTO_REPORT_PATH		"/home/observer/log/report/"
#define MANU_LOG_FILENAME		"/home/observer/log/manu_control.log"
#define AUTO_CMD_FILENAME		"/home/observer/auto_command.cmd"
#define DAEMON_LOG_FILENAME		"/home/observer/log/daemon-curr.log"
#define LSTDAEMON_LOG_FILENAME	"/home/observer/log/lstdaemon-curr.log"
#define REOPEN_FILE_SECOND		10
#define CMD_SIZE				256

#define AUTO_OBSERVE_LOG_TYPE 	0
#define STAR_TRACK_LOG_TYPE 	1
#define AUTO_FLAT_LOG_TYPE 		2
#define AUTO_DARK_LOG_TYPE 		3
#define AUTO_REPORT_LOG_TYPE 	4
#define LST_DAEMON_LOG_TYPE 	5

/*    dsp.fnuc symbolic constants   */
#define ReturnToOrigin	10
#define InOrigin		11
#define NormalStop		12
#define NormalObsCycle	13
#define RAQ				0
#define DECQ			1
#define PDQ				2
#define RADEC_REQUEST_DATA_SECOND		10
#define WAIT_FOR_DSP_PROCESS_SECOND		2
#define RESEND_REQUEST_SECOND			10
#define DSP_RPY_FILENAME		"/home/observer/dsp_reply.rpy"
#define DSP_CMD_FILENAME		"/home/observer/dsp_command.cmd"
#define PD_LOG_FILENAME			"/home/observer/log/pd_value.log"
#define DSP_CMD_VALIDATE_RESULT	"/home/observer/log/dsp_reply.log"
#define DSP_CMD_SIZE			CMD_SIZE
#define DSP_LOG_FILENAME		"/home/observer/log/dsp_daemon.log"

/*    ccd.func symbolic constants   */
#define CCD_CMD_FILENAME	"/home/observer/ccd_command.cmd"
#define CCD_RPY_FILENAME	"/home/observer/ccd_reply.rpy"
#define COOLER_OFF_REQUEST_DATA_SECOND	60
#define CCD_CMD_SIZE					CMD_SIZE
#define CCD_LOG_FILENAME	"/home/observer/log/ccd_daemon.log"
#define CCD_PID_FILENAME 	"/home/tat/daemon/ccddaemon.pid"

/*    ppc.func symbolic constants*/
#define PPC_CMD_FILENAME	"/home/observer/ppc_command.cmd"
#define PPC_RPY_FILENAME	"/home/observer/ppc_reply.rpy"
#define PPC_CMD_SIZE		CMD_SIZE
#define PPC_LOG_FILENAME	"/home/observer/log/ppc_daemon.log"

/*    pwr.func symbolic constants*/
#define PWR_CMD_FILENAME	"/home/observer/pwr_command.cmd"
#define PWR_RPY_FILENAME	"/home/observer/pwr_reply.rpy"
#define PWR_CMD_SIZE		CMD_SIZE
#define PWR_LOG_FILENAME    "/home/observer/log/pwr_daemon.log"

/*    ctl.func symbolic constant*/
#define CTL_CMD_FILENAME	"/home/observer/ctl_command.cmd"
/*    fli.func symbolic constant*/
#define FLI_CMD_FILENAME 	"/home/observer/fli_command.cmd"
#define FLI_CMD_SIZE		CMD_SIZE
#define FLI_LOG_FILENAME	"/home/observer/log/fli_daemon.log"

/*    auto.observing symbolic constants*/
#define ERROR_STATUS  		0
#define EMERGENCY_STATUS	1
#define NEXT_OBSERVATION	2
#define NORMAL_STATUS		3
#define HUMIDITY_STATUS		4
#define WIND_STATUS			5
#define WEATHER_DATA_BREAK	6
#define GET_WEATHER_FAILED  7
#define CLOUD_STATUS		10
#define RAIN_STATUS			11
#define CLOUD_DATA_BREAK	12
#define GET_CLOUD_FAILED	13
#define DUST_STATUS			15
#define BREAK_ALL_OBSERVATION	"BreakAllObservationNOW"
#define DO_NEXT_OBSERVATION		"DoNextObservation"
#define CONSERVA_CONSIDER_WIND	3//Being conservative with the weather conditions
#define CONSERVA_CONSIDER_HUMID	15//Being conservative with the weather conditions

#define FOV_CORRECT		0
#define FOV_TBC			1 //TO BE CONFIRMED
#define FOV_INCORRECT	2
#define FOV_ERROR		3

#define CCD_IDLE		0
#define CCD_IMAGE		1
#define CCD_FLAT		2
#define CCD_DARK		3
#define CCD_DOING_FLAT	4

////Web images
#define WEB_FOV_JPG "/var/www/htdocs/current_FOV.jpg"
#define WEB_CCD_JPG "/var/www/htdocs/recent_ccd_image.jpg"


#include "tat_info.h" //define the tat file
