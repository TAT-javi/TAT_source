/*
A header file for myAstroAlgorithm.
Included here are all the definitions for the structures and function prototypes.
13. Dec 2002
*/

#define PI		M_PI
#define DEG2HOUR	24 / 360	// radians -> hours
#define DEG2RAD		PI / 180	// degrees -> radians
#define RAD2DEG		180 / PI	// radians -> degrees

#define SYNODIC_MONTH	29.530588853

typedef struct
{
	int yr,year,mon,day,hr,min,sec;
	int timestamp;
	char string[20];
}DATE;

typedef struct {
	//longitude and latitude are measured in degree
	double longitude;
	double latitude;
}ECLIPTIC;

typedef struct {
	//ascension and declination are measured in degree
	double ascension;
	double declination;
}EQUATOR;


typedef struct {
	double azimuth;
	double altitude;
}HORIZON;


typedef struct  {
	//Nutation in longitude and in obliquity (measured in degree)
	double longitude;
	double obliquity;
}NUTATION;


typedef struct {
	double rising;
	double transit;
	double setting;	
}RISETRANSET;

//All the following time associated function will be calculated
//in UT for a given UT Date and time. 

double getJulianDay(DATE);
double getJulianCentury(DATE);
double getLocalSidereal(DATE);
double getGreenwichSidereal(DATE);
double getDynamicalTime(DATE);

DATE convert2UT(DATE d);
DATE getSystemTime(void);
DATE string2date(char *timestring);
DATE timestamp2date(int t);
int get_timestamp(DATE d);

double getMoonDistance(DATE);
double getMoonParallax(DATE);
double getMoonAge(DATE);
double getMoonPhase(DATE);
double getMoonPositionAngle(EQUATOR SunEqua, EQUATOR MoonEqua);

ECLIPTIC getMoonOpticLibration(DATE);
NUTATION getNutation(DATE);

double EclipticObliquity(DATE);

struct Ecliptic getMoonPosition(DATE);

double getSunEclipLong(DATE);
double getSunApparentLong(DATE);
EQUATOR getSunPosition(DATE);

EQUATOR EquatorParallax (DATE, EQUATOR);
EQUATOR Ecliptic2Equator(DATE, ECLIPTIC);

HORIZON Equator2Horizon(DATE, EQUATOR);

RISETRANSET getMoonRiseTranSet(DATE);
RISETRANSET getSunRiseTranSet (DATE);
RISETRANSET getStarRiseTranSet(DATE, EQUATOR);

char   *Deg2Dms(double degree);
char   *Deg2Hms(double degree);
