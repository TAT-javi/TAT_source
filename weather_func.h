#include <time.h>
#include "symblc_const.h"

/*
	structure
*/
typedef struct
{
    float wind_dir,wind_avg,wind_speed;
    float rainrate, humid_out, humid_in;
    float pressure, temper_out,temper_in;
    time_t timekey;
} weather_info;	

typedef struct
{
	float rain,clarity,light;
	time_t timekey;
}cloud_info;


/*
	function prototype
*/

int get_weather_condition (weather_info *);
int get_cloud_condition (cloud_info *);
int check_previous_weather (int interval_time,int wind_limit, int humid_limit);
int check_previous_cloud (int interval_time,int clarity_limit,int rain_limit);
int get_avg_min_max_db(char *variable,float *avg, float *min, float *max, int init);
int check_previous_weather_conditions_print(int be_conservative);
int check_previous_dust(int interval_time,float dust_limit);
