#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#include "myAstro.h" 
#include "symblc_const.h"

DATE getSystemTime() 
{
	DATE d;
	char 	buf[256];
	int 	yr, mon, day, hr, min, sec;
	struct tm *tm_ptr;
	time_t tm_now;

	time(&tm_now);
	tm_ptr=localtime(&tm_now);

	// read system time
	strftime(buf,256,"%m %d %Y %H %M %S",tm_ptr);
	sscanf(buf,"%d %d %d %d %d %d", &mon, &day, &yr, &hr, &min, &sec);
	d.mon=mon; d.day=day; d.yr=yr; d.hr=hr; d.min=min; d.sec=sec; d.year=d.yr-2000;

	d.timestamp = get_timestamp(d);
	sprintf(d.string,"%04d/%02d/%02d %02d:%02d:%02d",d.yr,d.mon,d.day,d.hr,d.min,d.sec);
	
	return d;
}//struct Date getSystemTime() 

DATE convert2UT(DATE d)
{
	DATE	ut;
	float	local_ut;
	int	ut_hr;

	local_ut=LOCAL_UT;
	ut_hr = d.hr - (int)local_ut;

	if( ut_hr < 0) { ut_hr +=24; d.day--;}

	if(d.day==0)
	{	
		d.mon--; 
		if(d.mon == 2)
		{
			if( (d.yr%4) == 0 )
				d.day = 29;
			else	d.day = 28;
		}
		else
		if(d.mon==1 || d.mon==3 || d.mon==5 || d.mon==7 || d.mon==8 || d.mon==10 || d.mon==12)
			d.day=31;
		else
		if(d.mon==4 || d.mon==6 || d.mon==9 || d.mon==11)
			d.day=30;
		else
		if(d.mon==0)
			{d.yr--; d.mon=12; d.day=31;}
		else
			{printf("error\n"); exit(0);}
	}//if(d.day==0)

	ut.yr	= d.yr;
	ut.mon	= d.mon;
	ut.day	= d.day;
	ut.hr	= ut_hr;
	ut.min	= d.min;
	ut.sec	= d.sec;
		
	return ut;
}//DATE convert2UT(DATE d)

double getLocalSidereal(DATE d)
{
        return fmod( getGreenwichSidereal(d) + (double)GmtOffSet , 24);
}
/*
Mean sidereal time at Greenwich for any instant in UT(expressed in degree),
and return in hours
*/
double getGreenwichSidereal(DATE d)
{
	double siderealTime;
	
	double JD = getJulianDay(d);
	double T  = getJulianCentury(d);

	siderealTime = fmod( 280.46061837 
			    + 360.98564736629*(JD - 2451545.0)
			    + 0.000387933*pow(T,2)
			    - (1/38710000)*pow(T,3)
			    , 360);
	if(siderealTime<0)	siderealTime += 360.0;
	siderealTime = siderealTime/360.0 * 24;
/*	printf("getGreenwichSidereal=%f\n", siderealTime);*/
        return siderealTime;
}

//---------------------------------
// Parameters associated with Time
//---------------------------------

/*
Return the Julian Day number, which is the number of elapsed days
since 1/1/4713 BC(Julian),12:00 GMT for the given time
*/
double getJulianDay(DATE d)
{
	double JD;

	JD  = 367.0 * d.yr
		- floor( 7 * (double)( d.yr + floor( ( d.mon + 9 ) /12.0) ) /4 )
		+ floor( (double)( 275.0 * d.mon ) / 9 )
		+ (double)( d.day - 730530 + 2451543.5)
		+ (double)( d.hr +  ( d.min/60.0) + ( d.sec /3600.0) ) /24.0;	
/*	printf("getJulianDay:=%16.6f\n",JD);
*/
	return JD;
}

/*
Return the time T, measured in Julian centuries of 36525 ephemeris days from
the epoch J2000.0 (2000 Jan 1.5 TD) 
which is the number of centuries after 1/1/1900 AD,12:00 GMT
*/
double getJulianCentury(DATE d)
{
	double T = ( getJulianDay(d) - 2451545.0) / 36525;	
	return T;
}

double getSidereal(DATE d)
{
        return fmod( getGreenwichSidereal(d), 24);
}

double getSid(DATE d)//to avoid problems with the previous function
{
	double p,q,oo;
	p = getGreenwichSidereal(d);
	q= fmod(p,24);
	oo = q*360./24.;
	return oo;
//         return fmod( getGreenwichSidereal(d), 24);
}

DATE string2date(char *timestring)
{
	DATE d;
	char year[5],mon[3],day[3],hour[3],min[3];
	
	year[0]=timestring[0];
	year[1]=timestring[1];
	year[2]=timestring[2];
	year[3]=timestring[3];year[4]='\0';
	mon[0]=timestring[4];
	mon[1]=timestring[5];mon[2]='\0';
	day[0]=timestring[6];
	day[1]=timestring[7];day[2]='\0';
	hour[0]=timestring[8];
	hour[1]=timestring[9];hour[2]='\0';
	min[0]=timestring[10];
	min[1]=timestring[11]; min[2]='\0';
	
	d.yr = atoi(year);
	d.mon = atoi(mon);
	d.day = atoi(day);
	d.hr = atoi(hour);
	d.min = atoi(min);
	d.sec=0;
	d.year = d.yr-2000;
	
	d.timestamp = get_timestamp( d);
	sprintf(d.string,"%04d/%02d/%02d %02d:%02d:%02d",d.yr,d.mon,d.day,d.hr,d.min,d.sec);
	
	return d;
}

///////////////////////////////////////////////////////////////
DATE timestamp2date(int t)
{
	DATE	d;
	char 	buf[256];
	int 	year, mon, mday, hour, min, sec;
	struct tm *tm_ptr;
	time_t tm_now;

	tm_now = t;
	
	d.timestamp = tm_now;
	tm_ptr=localtime(&tm_now);
	// read system time
	strftime(buf,256,"%m %d %Y %H %M %S",tm_ptr);
	sscanf(buf,"%d %d %d %d %d %d", &mon, &mday, &year, &hour, &min, &sec);
	d.mon=mon; d.day=mday; d.yr=year; d.hr=hour; d.min=min; d.sec=sec; d.year=d.yr-2000;
	
	sprintf(d.string,"%04d/%02d/%02d %02d:%02d:%02d",d.yr,d.mon,d.day,d.hr,d.min,d.sec);
	
	return d;
}//DATE timestamp2date(int t)

///////////////////////////////////////////////////////////////
int get_timestamp(DATE d)
{
	int t;
	struct tm ptr;
	time_t tm_now;
	
	ptr.tm_year= d.yr - 1900;
	ptr.tm_mon= d.mon - 1;
	ptr.tm_mday= d.day;
	ptr.tm_hour= d.hr;
	ptr.tm_min= d.min;
	ptr.tm_sec= d.sec;
	ptr.tm_isdst= 0;

	tm_now=mktime(&ptr);
	t=tm_now;

	return t;
}//int get_timestamp(DATE d)

