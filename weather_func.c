/*
	cc -lmysqlclient -lz -L/usr/lib/mysql
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysql/mysql.h>
#include "weather_func.h"
#include "tat_info.h"

#define EVENT_LIMIT 5 /*Number of positives for cloud and rain*/
#define HOST "localhost"
#define PASSWD "public"
#define DB_WEATHER "weather"
#define USER_WEATHER "weather"
#define DB_CLOUD "cloud"
#define USER_CLOUD "cloud"
#define DB_DUST "dust"
#define USER_DUST "dust"
#define TABLE_WEATHER "data"
#define TABLE_CLOUD "aurora"
#define TABLE_DUST "dust"

extern st_tat_info *p_tat_info;


int get_weather_condition (weather_info *weather)
{
	MYSQL mysql,*mysock;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char *query_string = "select timestamp,wind_avg,wind_speed,rainrate,pressure,humid_in,humid_out,temper_in,temper_out from data order by timestamp desc limit 1";

	//CONNECT TO DB
	mysql_init(&mysql);
	if (!(mysock = mysql_real_connect(&mysql,HOST,USER_WEATHER,PASSWD,DB_WEATHER,0,NULL,0))) 
			return -1;
	
	if (mysql_real_query (mysock, query_string, strlen (query_string))) return -1;
	if ((result = mysql_store_result (mysock))==NULL) return -1;
	
	row = mysql_fetch_row(result);
	
	weather->timekey = atol (row[0]);
	weather->wind_avg =  atof (row[1]);
	weather->wind_speed = atof (row[2]);
	weather->rainrate = atof (row[3]);
	weather->pressure = atof (row[4]);
	weather->humid_in = atof (row[5]);
	weather->humid_out = atof (row[6]);
	weather->temper_in = atof (row[7]);
	weather->temper_out = atof (row[8]);
	//CLOSE 
	mysql_close(mysock);
	return 0;
}


int get_cloud_condition (cloud_info *cloud)
{
	MYSQL mysql,*mysock;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char *query_string = "select timestamp,clarity,rain,light from aurora order by timestamp desc limit 1";

	//CONNECT TO DB
	mysql_init(&mysql);
	if (!(mysock = mysql_real_connect(&mysql,HOST,USER_CLOUD,PASSWD,DB_CLOUD,0,NULL,0))) 
			return -1;
	
	if (mysql_real_query (mysock, query_string, strlen (query_string))) return -1;
	if ((result = mysql_store_result (mysock))==NULL) return -1;
	
	row = mysql_fetch_row(result);
	
	cloud->timekey = atol (row[0]);
	cloud->clarity =  atof (row[1]);
	cloud->rain = atof (row[2]);
	cloud->light = atof (row[3]);
	//CLOSE 
	mysql_close(mysock);
	return 0;
}



int check_previous_weather (int interval_time,int wind_limit, int humid_limit)
{
	MYSQL mysql,*mysock;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int weather_data_count, init;
	char wind_query[300],humid_query[300];
	time_t now;
	int wind_rows, humid_rows;
	float wind,humid;

	time(&now);
	init = now-interval_time;

	sprintf(wind_query,"select wind_avg from data  where timestamp>= %d",init);
	sprintf(humid_query,"select humid_out from data  where timestamp>= %d",init);
	
	weather_data_count = (int)(interval_time/60.0)-2;//number of the least results
	//CONNECT TO DB
	mysql_init(&mysql);
	if (!(mysock = mysql_real_connect(&mysql,HOST,USER_WEATHER,PASSWD,DB_WEATHER,0,NULL,0))) 
			return ERROR_STATUS;
//WIND first
	if (mysql_real_query (mysock, wind_query, strlen (wind_query))) return ERROR_STATUS;
	if ((result = mysql_store_result (mysock))==NULL) return ERROR_STATUS;

	wind_rows=0;
	while (row = mysql_fetch_row(result))
	{
		wind_rows++;
		wind = atof(row[0]);
		if(wind>=wind_limit){mysql_close(mysock);return WIND_STATUS;}
	}
	if(wind_rows< weather_data_count){mysql_close(mysock); return WEATHER_DATA_BREAK;}

//HUMID
	if (mysql_real_query (mysock, humid_query, strlen (humid_query))) return ERROR_STATUS;
	if ((result = mysql_store_result (mysock))==NULL) return ERROR_STATUS;

	humid_rows=0;
	while (row = mysql_fetch_row(result))
	{
		humid_rows++;
		humid = atof(row[0]);
		if(humid>=humid_limit) {mysql_close(mysock);return HUMIDITY_STATUS;}
	}
	if(humid_rows< weather_data_count){mysql_close(mysock); return WEATHER_DATA_BREAK;}

	//CLOSE 
	mysql_close(mysock);
	return NORMAL_STATUS;
}


int check_previous_cloud (int interval_time,int clarity_limit,int rain_limit)
{
	MYSQL mysql,*mysock;
	MYSQL_RES *result;
	MYSQL_ROW row;
	time_t now;
	int clarity_row,rain_row;
	float clarity,rain;
	int event,init,cloud_data_count;
	char query[300];
	
	time(&now);
	init = now-interval_time;
	
	cloud_data_count = (int)(interval_time/120)-1;//number of the least results
	
	//CONNECT TO DB
	mysql_init(&mysql);
	if (!(mysock = mysql_real_connect(&mysql,HOST,USER_CLOUD,PASSWD,DB_CLOUD,0,NULL,0))) 
			return ERROR_STATUS;
	
	//////////CLOUD///////
	sprintf(query,"select clarity from aurora where timestamp >= %d",init);
	
	if (mysql_real_query (mysock, query, strlen (query))) return ERROR_STATUS;
	if ((result = mysql_store_result (mysock))==NULL) return ERROR_STATUS;
	
	clarity_row=0; event=0;
	while ((row = mysql_fetch_row(result)))
	{
		clarity_row++;
		clarity = atof(row[0]);
		if(clarity< clarity_limit) event++;
	}
	if(event>EVENT_LIMIT){mysql_close(mysock);return CLOUD_STATUS;}
	else if(clarity_row< cloud_data_count){mysql_close(mysock);return CLOUD_DATA_BREAK;}
	
	////////////RAIN///////////////////
	sprintf(query,"select rain from aurora where timestamp >= %d",init);
	
	if (mysql_real_query (mysock, query, strlen (query))) return ERROR_STATUS;
	if ((result = mysql_store_result (mysock))==NULL) return ERROR_STATUS;
	
	rain_row=0; event=0;
	while ((row = mysql_fetch_row(result)))
	{
		rain_row++;
		rain = atof(row[0]);
		if(rain>rain_limit) event++;
	}
	if(event>EVENT_LIMIT){mysql_close(mysock);return RAIN_STATUS;}
	else if(rain_row< cloud_data_count) {mysql_close(mysock);return CLOUD_DATA_BREAK;}
	
	//CLOSE 
	mysql_close(mysock);
	return NORMAL_STATUS;
}

//////
// 	Check dust. 
//	Since this measurement is from the web,
//	it only returns a bad signal if the dust is too high. 
/////

int check_previous_dust(int interval_time,float dust_limit)
{
	MYSQL mysql,*mysock;
	MYSQL_RES *result;
	MYSQL_ROW row;
	time_t now;
	int dust_row;
	float dust;
	int event,init;
	char query[300];
	
	time(&now);
	init = now-interval_time;
	
	//CONNECT TO DB
	mysql_init(&mysql);
	if (!(mysock = mysql_real_connect(&mysql,HOST,USER_DUST,PASSWD,DB_DUST,0,NULL,0))) 
			return NORMAL_STATUS;
	//////////CLOUD///////
	sprintf(query,"select dust from dust where timestamp >= %d",init);
	
	if (mysql_real_query (mysock, query, strlen (query))) return NORMAL_STATUS;
	if ((result = mysql_store_result (mysock))==NULL) return NORMAL_STATUS;
	
	dust_row=0; event=0;
	while ((row = mysql_fetch_row(result)))
	{
		dust_row++;
		dust = atof(row[0]);
		if(dust> dust_limit) event++;
	}
	
	//CLOSE 
	mysql_close(mysock);
	
	if(event>0)return DUST_STATUS;
	return NORMAL_STATUS;
}


int check_previous_weather_conditions_print(int be_conservative)
{
	char temp_string[200];
	int flag,humidity_criterion,wind_criterion,weather_interval_time;
	int cloud_criterion,rain_criterion,cloud_interval_time;
	
	humidity_criterion = DoGetValue( "HUMIDITY_CRITERION");
	wind_criterion = DoGetValue("WIND_CRITERION");
	weather_interval_time = DoGetValue("GOOD_WEATHER_CRITERION_TIME");

	cloud_criterion = DoGetValue("CLOUD_CRITERION");
	rain_criterion = DoGetValue( "RAIN_CRITERION");
	cloud_interval_time = DoGetValue("NO_CLOUD_CRITERION_TIME");
	
	if(be_conservative)
	{
		wind_criterion -=CONSERVA_CONSIDER_WIND;
		humidity_criterion -=CONSERVA_CONSIDER_HUMID;
	}
	
	
	flag =check_previous_weather (weather_interval_time,wind_criterion, humidity_criterion);
	
	switch(flag)
	{
		case WIND_STATUS:
			sprintf(temp_string,"WARNING: Strong wind detected during last %d seconds.",weather_interval_time);
		break;

		case HUMIDITY_STATUS:
			sprintf(temp_string,"WARNING: High humidity detected during last %d seconds.",weather_interval_time);
		break;

		case NORMAL_STATUS:
			// nothing to do.
		break;

		case WEATHER_DATA_BREAK:
			sprintf(temp_string,"ERROR: Not enough weather data for %d seconds.",weather_interval_time);
		break;
		case ERROR_STATUS:
			sprintf(temp_string,"ERROR: Reading weather condition failed.");
		break;
	}
	
	if(flag != NORMAL_STATUS)
	{
		step(temp_string);
		return flag;
	}
	flag = check_previous_cloud (cloud_interval_time,cloud_criterion,rain_criterion);
	
	switch( flag )
	{
		case CLOUD_STATUS:
			sprintf(temp_string,"WARNING: Cloud detected during last %d seconds.",cloud_interval_time);
		break;

		case RAIN_STATUS:
			sprintf(temp_string,"WARNING: Rain detected during last %d seconds.",cloud_interval_time);
		break;
		case NORMAL_STATUS:
			//do nothing
		break;
		case CLOUD_DATA_BREAK:
			sprintf(temp_string,"ERROR: Not enough cloud data for %d seconds.",cloud_interval_time);
		break;
		case ERROR_STATUS:
			sprintf(temp_string,"ERROR: Reading cloud condition failed.");
		break;
	}
	
	if(flag != NORMAL_STATUS)
		step(temp_string);
#ifdef DUST_SENSOR
	flag = check_previous_dust(6000,0.01);
	
	if(flag == DUST_STATUS)
		step("WARNING: High dust detected.");
#endif
	return flag;
}


int get_avg_min_max_db(char *variable,float *avg, float *min, float *max, int init)
{
	MYSQL mysql,*mysock;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char query_string[200];
	char db[10],user[10],table[10];
	
	if(!strcmp(variable,"clarity") || !strcmp(variable,"rain") )
	{
		strcpy(db,DB_CLOUD);
		strcpy(user,USER_CLOUD);
		strcpy(table,TABLE_CLOUD);
	}
	else
	{
		strcpy(db,DB_WEATHER);
		strcpy(user,USER_WEATHER);
		strcpy(table,TABLE_WEATHER);		
	}
	
	sprintf(query_string,"select avg(%s),min(%s),max(%s) from %s where timestamp >= %d",
			variable,variable,variable,table,init);
	
	//CONNECT TO DB
	mysql_init(&mysql);
	if (!(mysock = mysql_real_connect(&mysql,HOST,user,PASSWD,db,0,NULL,0))) 
			return -1;
	
	if (mysql_real_query (mysock, query_string, strlen (query_string))) return -1;
	if ((result = mysql_store_result (mysock))==NULL) return -1;
	
	row = mysql_fetch_row(result);
	
	*avg = atof(row[0]);
	*min = atof(row[1]);
	*max = atof(row[2]);

	//CLOSE 
	mysql_close(mysock);
	return 0;
}
