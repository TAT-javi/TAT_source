/*
Under developed astronomical algorithms based on 
Jean Meeus book: "Astronomical algorithms".
Define the implementation of astro algorithm which are in modular form.
18.Jan 2003
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "myAstro.h"
#include "symblc.const.h"
/*
// site location in "symblc.const.h"
//#define LOCAL_LONG	120.99209
//#define LOCAL_LAT	24.79431

//#define LOCAL_LONG	121.5180	//Taipei, East,  
//#define LOCAL_LAT	 25.09556

#define LOCAL_LONG	-16.5122
#define LOCAL_LAT	28.3003


#define GmtOffSet	LOCAL_LONG *24.0/360.0
*/
/*
//---------------------------------
// Parameters associated with Time
//---------------------------------
*/
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


/*
Returns the current local sidereal time, measured in hours
*/
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

double getDynamicalTime(DATE d)
{
	double T  = getJulianCentury(d);
	double deltaT = 102.3 + 123.5*T + 32.5*pow(T,2);
	/* deltaT = Dynamical Time(TD)- Universal Time(UT)	*/
	double TD, UT;

	UT= d.hr- GmtOffSet;
	if(UT<0) UT += 24;
	TD = deltaT + UT;
	return TD;
}


//----------------------------
//Coordinate transformations
//----------------------------
/*
  Transformation from Ecliptical into Equator coordinates
*/
EQUATOR Ecliptic2Equator(DATE d, ECLIPTIC eclip)
{
	EQUATOR equatorial;
	double epsilon = EclipticObliquity(d);
	
	//L=lambda=eclip.longitude, B=beta=eclip.latitude, E=epsilon=obliquity
	double sinL = sin(eclip.longitude * DEG2RAD);
	double cosL = cos(eclip.longitude * DEG2RAD);

	double sinB = sin(eclip.latitude * DEG2RAD);
	double cosB = cos(eclip.latitude * DEG2RAD);
	double tanB = tan(eclip.latitude * DEG2RAD);

	double cosE = cos(epsilon * DEG2RAD);
	double sinE = sin(epsilon * DEG2RAD);

	equatorial.ascension	= atan2( sinL * cosE - tanB * sinE , cosL )* RAD2DEG;
	equatorial.declination	= asin ( sinB * cosE + cosB * sinE * sinL )* RAD2DEG;

	if(equatorial.ascension <0) equatorial.ascension += 360.0;

	return equatorial;
}

HORIZON Equator2Horizon(DATE d, EQUATOR equator)
{
	HORIZON horizon;

	double H = (getLocalSidereal(d)*15.0) - equator.ascension;

	//LST is measure in hour, multiply by 1/24*360 = 15 to convert to degree
	double sinH = sin(H * DEG2RAD);
	double cosH = cos(H * DEG2RAD);

	double sinD = sin(equator.declination * DEG2RAD);
	double cosD = cos(equator.declination * DEG2RAD);
	double tanD = tan(equator.declination * DEG2RAD);

	double sinL = sin(LOCAL_LAT * DEG2RAD);
	double cosL = cos(LOCAL_LAT * DEG2RAD);

	//The azimuth which counts from south clockwise, i.e. West is +90 deg
	horizon.azimuth  = atan2( sinH , cosH * sinL - tanD * cosL )* RAD2DEG;
	horizon.altitude = asin ( sinL * sinD + cosL * cosD * cosH )* RAD2DEG;

	//Reckon the azimuth starting from North, i.e East is +90deg
	horizon.azimuth += 180.0;
		
	return horizon;
}


EQUATOR EquatorParallax (DATE d, EQUATOR equator)
{
	EQUATOR equatorParallax;

	double rho  = 1.0;	//Observer distance to the center of Earth	

	double sinP = sin( getMoonParallax(d) * DEG2RAD );	//Moon parallax

	double sinL = sin( LOCAL_LAT * DEG2RAD );		//Observer geocentric Latitude
	double cosL = cos( LOCAL_LAT * DEG2RAD );

	double sinD = sin( equator.declination * DEG2RAD );	//Moon Declination
	double cosD = cos( equator.declination * DEG2RAD );

	double H    = getGreenwichSidereal(d);			//HourAngle in hour

	double sinH = sin( ( H*15 + LOCAL_LONG - equator.ascension)*DEG2RAD );
	double cosH = cos( ( H*15 + LOCAL_LONG - equator.ascension)*DEG2RAD );

	double delta_asc = atan2( ( -rho*cosL*sinP*sinH ) ,
				  (cosD - rho*cosL*sinP*cosH) ) * RAD2DEG;

	double parallax_dec = atan2( ( (sinD - rho*sinL*sinP) * cos(delta_asc*DEG2RAD)),
					(cosD - rho*cosL*sinP*cosH ) )* RAD2DEG;
	equatorParallax.ascension  = equator.ascension + delta_asc;
	equatorParallax.declination= parallax_dec;

	return equatorParallax;
}

double EclipticObliquity(DATE d)
{
	double T= getJulianCentury(d);
	double U = T/100.0;

	//Obliquity of the ecliptic, angle between the ecliptic and celestial equator.
	double obliq    = (23.0 + (26.0/60) + (41.448/3600)) - (4680.93/3600)*U
			-( 1.55/3600)*pow(U,2) + (1999.25/3600)*pow(U,3)
			-(51.38/3600)*pow(U,4) - ( 249.67/3600)*pow(U,5)
			-(39.05/3600)*pow(U,6) + (   7.12/3600)*pow(U,7)
			+(27.87/3600)*pow(U,8) + (   5.79/3600)*pow(U,9)
			+( 2.45/3600)*pow(U,10);

	return obliq;
}


/*
Calculate the position of the Moon at the given time T(in Julian Century)
in Equator coordinates.
*/

ECLIPTIC getMoonPosition(DATE d)
{
	ECLIPTIC ecliptic;
	double T;
	double L_prime, M_prime, D, F;	//Moon parameters
	double M, e;			//Sun parameters
	double lambda, beta;		//Moon Ecliptical Longitude/Latitude
	double A1, A2, A3;

	T= getJulianCentury(d);
	/*
	Compute orbital parameters of moon to obtain the 
	geocentric ecliptical longitude lambda and latitude beta of the center of Moon,
	referred to the mean equinox of the date.
	*/

	//Moon's mean elongation  
	D = fmod(297.8502042 + 445267.1115168*T - 0.0016300*pow(T,2) + (1.0/545868)*pow(T,3)
	  	   - (1.0/113065000)*pow(T,4)	,360);

	//Sun's mean anomaly
	M = fmod(357.5291092 + 35999.0502909*T - 0.0001536*pow(T,2) + (1/24490000)*pow(T,3)
		   ,360);

	//Moon's mean anomaly
	M_prime = fmod(134.9634114 + 477198.8676313*T + 0.0089970*pow(T,2) + (1.0/69699)*pow(T,3)
			  - (1.0/14712000)*pow(T,4)	,360);

	//Mean distance of moon from its ascending node      
	F = fmod(93.2720993 + 483202.0175273*T - 0.0034029*pow(T,2) - (1.0/3526000)*pow(T,3)
	  	   + (1.0/863310000)*pow(T,4)	,360);
	  	   
	//Eccentricity of the Earth's orbit around the Sun
	e = 1 - 0.002516*T - 0.0000074*pow(T, 2);

	//Moon's mean longitude
	L_prime = fmod(218.3164591 + 481267.88134236*T - 0.0013268*pow(T,2) 
	  		+(1.0/538841)*pow(T,3) - (1.0/65194000)*pow(T,4)	,360);

	if(D<0)	D+=360;  
	if(M<0)	M+=360;  
	if(M_prime<0) M_prime+=360;
	if(F<0)	F+=360;
	if(L_prime<0) L_prime+=360;

	//Additive terms: A1,A2 and A3 are in degrees
	A1=fmod(119.75 +    131.849*T	, 360);		//Action due to Venus
	A2=fmod( 53.09 + 479264.290*T	, 360);		//Action due to Jupiter
	A3=fmod(313.45 + 481266.484*T	, 360);

	if(A1<0) A1+=360;
	if(A2<0) A2+=360;  
	if(A3<0) A3+=360;

	//The geocentric ecliptical longitude "lambda" of the center of moon
	lambda	  = L_prime + 6.288774* sin( M_prime * DEG2RAD)
		  + 1.274027*sin((2*D - M_prime) * DEG2RAD)
	          + 0.658314*sin((2*D) * DEG2RAD)
	          + 0.213618*sin((2*M_prime) * DEG2RAD)
	          - 0.185116*sin( M * DEG2RAD)*(e)
	          - 0.114332*sin( 2*F * DEG2RAD)
	          + 0.058793*sin((2*D - 2*M_prime) * DEG2RAD)
	          + 0.057066*sin((2*D - M - M_prime) * DEG2RAD)*(e)
	          + 0.053322*sin((2*D + M_prime) * DEG2RAD)
	          + 0.045758*sin((2*D - M) * DEG2RAD)*(e)
	          - 0.040923*sin((M - M_prime) * DEG2RAD)*(e)
	          - 0.034720*sin( D * DEG2RAD)
	          - 0.030383*sin((M + M_prime) * DEG2RAD)*(e)
	          + 0.015327*sin((2*D - 2*F) * DEG2RAD)
	          - 0.012528*sin((M_prime + 2*F) * DEG2RAD)
	          + 0.010980*sin((M_prime - 2*F) * DEG2RAD)
	          + 0.010675*sin((4*D - M_prime) * DEG2RAD)
	          + 0.010034*sin((3*M_prime) * DEG2RAD)
	          + 0.008548*sin((4*D - 2*M_prime) * DEG2RAD)
	          - 0.007888*sin((2*D + M - M_prime) * DEG2RAD)*(e)
	          - 0.006766*sin((2*D + M) * DEG2RAD)*(e)
	          - 0.005163*sin((D - M_prime) * DEG2RAD)
	          + 0.004987*sin((D + M) * DEG2RAD)*(e)
	          + 0.004036*sin((2*D - M + M_prime) * DEG2RAD)*(e)
	          + 0.003994*sin((2*D + 2*M_prime) * DEG2RAD)
	          + 0.003861*sin( 4*D * DEG2RAD)
	          + 0.003665*sin((2*D - 3*M_prime) * DEG2RAD)
	          - 0.002689*sin((M - 2*M_prime) * DEG2RAD)*(e)
	          - 0.002602*sin((2*D - M_prime + 2*F) * DEG2RAD)
	          + 0.002390*sin((2*D - M - 2*M_prime) * DEG2RAD)*(e)
	          - 0.002348*sin((D + M_prime) * DEG2RAD)
	          + 0.002236*sin((2*D - 2*M) * DEG2RAD)*pow(e, 2)
	          - 0.002120*sin((M + 2*M_prime) * DEG2RAD)*(e)
	          - 0.002069*sin( 2*M * DEG2RAD)*pow(e, 2)
	          + 0.002048*sin((2*D - 2*M - M_prime) * DEG2RAD)*pow(e, 2)
	          - 0.001773*sin((2*D + M_prime - 2*F) * DEG2RAD)
	          - 0.001595*sin((2*D + 2*F) * DEG2RAD)
	          + 0.001215*sin((4*D - M - M_prime) * DEG2RAD)*(e)
	          - 0.001110*sin((2*M_prime + 2*F) * DEG2RAD)
	          - 0.000892*sin((3*D - M_prime) * DEG2RAD)
	          - 0.000810*sin((2*D + M + M_prime)   * DEG2RAD)*(e)
	          + 0.000759*sin((4*D - M - 2*M_prime) * DEG2RAD)*(e)
	          - 0.000713*sin((2*M - M_prime) * DEG2RAD)*pow(e, 2)
	          - 0.000700*sin((2*D + 2*M - M_prime) * DEG2RAD)*pow(e, 2)
	          + 0.000691*sin((2*D + M - 2*M_prime) * DEG2RAD)*(e)
	          + 0.000596*sin((2*D - M - 2*F) * DEG2RAD)*(e)
	          + 0.000549*sin((4*D + M_prime) * DEG2RAD)
	          + 0.000537*sin( 4*M_prime * DEG2RAD)
	          + 0.000520*sin((4*D - M)  * DEG2RAD)*(e)
	          - 0.000487*sin((D - 2*M_prime) * DEG2RAD)
	          
	          - 0.000399*sin((2*D + M - 2*F)   * DEG2RAD)*(e)
	          - 0.000381*sin((2*M_prime - 2*F) * DEG2RAD)
	          + 0.000351*sin((D + M + M_prime) * DEG2RAD)*(e)
	          - 0.000340*sin((3*D - 2*M_prime) * DEG2RAD)
	          + 0.000330*sin((4*D - 3*M_prime) * DEG2RAD)
	          + 0.000327*sin((2*D - M + M_prime) * DEG2RAD)*(e)
	          - 0.000323*sin((2*M - M_prime)   * DEG2RAD)*pow(e,2)
	          + 0.000299*sin((D + M - M_prime) * DEG2RAD)*(e)
	          + 0.000294*sin((2*D + 3*M_prime) * DEG2RAD)
	          
	          + 0.003958*sin((A1) * DEG2RAD)
	          + 0.001962*sin((L_prime - F)   * DEG2RAD)
	          + 0.000318*sin((A2) * DEG2RAD);

	if(lambda <0)	lambda += 360.0;
  
	//The geocentric ecliptical latitude "beta" of the center of moon
	beta	  = 5.128122*sin( F * DEG2RAD)
	          + 0.280602*sin((M_prime + F) * DEG2RAD)
	          + 0.277693*sin((M_prime - F) * DEG2RAD)
	          + 0.173237*sin((2*D - F) * DEG2RAD)
	          + 0.055413*sin((2*D - M_prime + F) * DEG2RAD)
	          + 0.046271*sin((2*D - M_prime - F) * DEG2RAD)
	          + 0.032573*sin((2*D + F) * DEG2RAD)
	          + 0.017198*sin((2*M_prime + F) * DEG2RAD)
	          + 0.009266*sin((2*D + M_prime - F) * DEG2RAD)
	          + 0.008822*sin((2*M_prime - F) * DEG2RAD)
	          + 0.008216*sin((2*D - M - F) * DEG2RAD)*(e)
	          + 0.004324*sin((2*D - 2*M_prime - F) * DEG2RAD)
	          + 0.004200*sin((2*D + M_prime + F) * DEG2RAD)
	          - 0.003359*sin((2*D + M - F) * DEG2RAD)*(e)
	          + 0.002463*sin((2*D - M - M_prime - F) * DEG2RAD)*(e)
	          + 0.002211*sin((2*D - M + F) * DEG2RAD)*(e)
	          + 0.002065*sin((2*D - M - M_prime - F) * DEG2RAD)*(e)
	          - 0.001870*sin((M - M_prime - F) * DEG2RAD)*(e)
	          + 0.001828*sin((4*D - M_prime - F) * DEG2RAD)
	          - 0.001794*sin((M + F) * DEG2RAD)*(e)
	          - 0.001749*sin( 3*F * DEG2RAD)
	          - 0.001565*sin((M - M_prime + F) * DEG2RAD)*(e)
	          - 0.001491*sin((D + F) * DEG2RAD)
	          - 0.001475*sin((M + M_prime + F) * DEG2RAD)*(e)
	          - 0.001410*sin((M + M_prime - F) * DEG2RAD)*(e)
	          - 0.001344*sin((M - F) * DEG2RAD)*(e)
	          - 0.001335*sin((D - F) * DEG2RAD)
	          + 0.001107*sin((3*M_prime + F) * DEG2RAD)
	          + 0.001021*sin((4*D - F) * DEG2RAD)
	          + 0.000833*sin((4*D - M_prime + F) * DEG2RAD)
	          + 0.000777*sin((M_prime - 3*F) * DEG2RAD)
	          + 0.000671*sin((4*D - 2*M_prime + F) * DEG2RAD)
	          + 0.000607*sin((2*D - 3*F) * DEG2RAD)
	          + 0.000596*sin((2*D + 2*M_prime - F) * DEG2RAD)
	          + 0.000491*sin((2*D - M + M_prime - F) * DEG2RAD)*(e)
	          - 0.000451*sin((2*D - 2*M_prime + F) * DEG2RAD)
	          + 0.000439*sin((3*M_prime - F) * DEG2RAD)
	          + 0.000422*sin((2*D + 2*M_prime + F) * DEG2RAD)
	          + 0.000421*sin((2*D - 3*M_prime - F) * DEG2RAD)
	          - 0.000366*sin((2*D + M - M_prime - F) * DEG2RAD)*(e)
	          - 0.000351*sin((2*D + M + F) * DEG2RAD)*(e)
	          + 0.000331*sin((4*D + F) * DEG2RAD)
	          + 0.000315*sin((2*D - M + M_prime + F) * DEG2RAD)*(e)
	          + 0.000302*sin((2*D - 2*M - F) * DEG2RAD)*pow(e, 2)
	          - 0.000283*sin((M_prime + 3*F) * DEG2RAD)
	          
	          - 0.000229*sin((2*D + M + M_prime -F) * DEG2RAD)*(e)
	          + 0.000223*sin((D + M -F) * DEG2RAD)*(e)
	          + 0.000223*sin((D + M +F) * DEG2RAD)*(e)
	          - 0.000220*sin((M - 2*M_prime - F) * DEG2RAD)*(e)
	          - 0.000220*sin((2*D + M - M_prime - F) * DEG2RAD)*(e)
	          - 0.000185*sin((D + M_prime + F) * DEG2RAD)        
	          + 0.000181*sin((2*D - M - 2*M_prime - F) * DEG2RAD)*(e)
	          - 0.000177*sin((M + 2*M_prime + F) * DEG2RAD)*(e)
	          + 0.000176*sin((4*D - 2*M_prime - F) * DEG2RAD)
	          + 0.000166*sin((4*D - M - M_prime - F) * DEG2RAD)*(e)
	          - 0.000164*sin((D + M_prime - F) * DEG2RAD)
	          + 0.000132*sin((4*D + M_prime - F) * DEG2RAD)
	          - 0.000119*sin((D - M_prime - F) * DEG2RAD)
	          + 0.000115*sin((4*D - M - F) * DEG2RAD)*(e)
	          + 0.000107*sin((2*D - 2*M + F) * DEG2RAD)*pow(e,2)
	          
	          - 0.002235*sin((L_prime) * DEG2RAD)
	          + 0.000382*sin((A3) * DEG2RAD)
	          + 0.000175*sin((A1 - F) * DEG2RAD)
	          + 0.000175*sin((A1 + F) * DEG2RAD)
	          + 0.000127*sin((L_prime - M_prime) * DEG2RAD)
	          - 0.000115*sin((L_prime + M_prime) * DEG2RAD);

	ecliptic.longitude = lambda;
	ecliptic.latitude  = beta;
  
	return ecliptic;
}

double getMoonDistance(DATE d)
{
	double T;
	double D, M_prime, F;	//Moon parameters
	double M, e;		//Sun parameters
	double delta;		//Distance of moon from earth
	
	T= getJulianCentury(d);

	//Moon's mean elongation  
	D = fmod(297.8502042 + 445267.1115168*T - 0.0016300*pow(T,2) + (1.0/545868)*pow(T,3)
	  	   - (1.0/113065000)*pow(T,4)	,360);

	//Sun's mean anomaly
	M = fmod(357.5291092 + 35999.0502909*T - 0.0001536*pow(T,2) + (1/24490000)*pow(T,3)
		   ,360);
		   
	//Moon's mean anomaly
	M_prime = fmod(134.9634114 + 477198.8676313*T + 0.0089970*pow(T,2) + (1.0/69699)*pow(T,3)
			  - (1.0/14712000)*pow(T,4)	,360);

	//Mean distance of moon from its ascending node      
	F = fmod(93.2720993 + 483202.0175273*T - 0.0034029*pow(T,2) - (1.0/3526000)*pow(T,3)
	  	   + (1.0/863310000)*pow(T,4)	,360);
	  	   			  
	//Eccentricity of the Earth's orbit around the Sun
	e = 1 - 0.002516*T - 0.0000074*pow(T, 2);

	//Distance of the Moon from Earth
	delta	  = 385000.56 - 20905.255*cos( M_prime * DEG2RAD)
	          -  3699.111*cos((2*D - M_prime) * DEG2RAD)
	          -  2955.968*cos((2*D) * DEG2RAD)
	          -   569.925*cos((2*M_prime) * DEG2RAD)
	          +    48.888*cos( M * DEG2RAD)*(e)
	          -     3.149*cos( 2*F * DEG2RAD)
	          +   246.158*cos((2*D - 2*M_prime) * DEG2RAD)
	          -   152.138*cos((2*D - M - M_prime) * DEG2RAD)*(e)
	          -   170.733*cos((2*D + M_prime) * DEG2RAD)
	          -   204.586*cos((2*D - M) * DEG2RAD)*(e)
	          -   129.620*cos((M - M_prime) * DEG2RAD)*(e)
	          +   108.743*cos( D * DEG2RAD)
	          +   104.755*cos((M + M_prime) * DEG2RAD)*(e)
	          +    10.321*cos((2*D - 2*F) * DEG2RAD)
	
	          +    79.661*cos((M_prime - 2*F) * DEG2RAD)
	          -    34.782*cos((4*D - M_prime) * DEG2RAD)
	          -    23.210*cos((3*M_prime) * DEG2RAD)
	          -    21.636*cos((4*D - 2*M_prime) * DEG2RAD)
	          +    24.208*cos((2*D + M - M_prime) * DEG2RAD)*(e)
	          +    30.824*cos((2*D + M) * DEG2RAD)*(e)
	          -     8.379*cos((D - M_prime) * DEG2RAD)
	          -    16.675*cos((D + M) * DEG2RAD)*(e)
	          -    12.831*cos((2*D - M + M_prime) * DEG2RAD)*(e)
	          -    10.445*cos((2*D + 2*M_prime) * DEG2RAD)
	          -    11.650*cos( 4*D * DEG2RAD)
	          +    14.403*cos((2*D - 3*M_prime) * DEG2RAD)
	          -     7.003*cos((M - 2*M_prime) * DEG2RAD)*(e)
	
	          +    10.056*cos((2*D - M - 2*M_prime) * DEG2RAD)*(e)
	          +     6.322*cos((D + M_prime) * DEG2RAD)
	          -     9.884*cos((2*D - 2*M) * DEG2RAD)*pow(e, 2)
	          +     5.751*cos((M + 2*M_prime) * DEG2RAD)*(e)
	
	          -     4.950*cos((2*D - 2*M - M_prime) * DEG2RAD)*pow(e, 2)
	          +     4.130*cos((2*D + M_prime - 2*F) * DEG2RAD)
	
	          -     3.958*cos((4*D - M - M_prime) * DEG2RAD)*(e)
	
	          +     3.258*cos((3*D - M_prime) * DEG2RAD)
	          +     2.616*cos((2*D + M + M_prime)   * DEG2RAD)*(e)
	          -     1.897*cos((4*D - M - 2*M_prime) * DEG2RAD)*(e)
	          -     2.117*cos((2*M - M_prime) * DEG2RAD)*pow(e, 2)
	          +     2.354*cos((2*D + 2*M - M_prime) * DEG2RAD)*pow(e, 2)
	
	          -     1.423*cos((4*D + M_prime) * DEG2RAD)
	          -     1.117*cos( 4*M_prime * DEG2RAD)
	          -     1.571*cos((4*D - M)  * DEG2RAD)*(e)
	          -     1.739*cos((D - 2*M_prime) * DEG2RAD)
	          
	          -     4.421*cos((2*M_prime - 2*F) * DEG2RAD)
	
	          +     1.165*cos((2*M - M_prime)   * DEG2RAD)*pow(e,2)
		  +	8.752*cos((2*D - M_prime -2*F) * DEG2RAD);

	return delta;
}

double getMoonParallax(DATE d)
{
	double T;
	double D, M_prime, F;	//Moon parameters
	double M, e;		//Sun parameters
	double delta;		//Distance of moon from earth
	double parallax;
	
	T= getJulianCentury(d);

	//Moon's mean elongation  
	D = fmod(297.8502042 + 445267.1115168*T - 0.0016300*pow(T,2) + (1.0/545868)*pow(T,3)
	  	   - (1.0/113065000)*pow(T,4)	,360);

	//Sun's mean anomaly
	M = fmod(357.5291092 + 35999.0502909*T - 0.0001536*pow(T,2) + (1/24490000)*pow(T,3)
		   ,360);
		   
	//Moon's mean anomaly
	M_prime = fmod(134.9634114 + 477198.8676313*T + 0.0089970*pow(T,2) + (1.0/69699)*pow(T,3)
			  - (1.0/14712000)*pow(T,4)	,360);

	//Mean distance of moon from its ascending node      
	F = fmod(93.2720993 + 483202.0175273*T - 0.0034029*pow(T,2) - (1.0/3526000)*pow(T,3)
	  	   + (1.0/863310000)*pow(T,4)	,360);
	  	   			  
	//Eccentricity of the Earth's orbit around the Sun
	e = 1 - 0.002516*T - 0.0000074*pow(T, 2);

	//The parallax of the center of moon
	parallax= 0.950724
		+0.051818*cos( M_prime * DEG2RAD)
		+0.009531*cos((2*D - M_prime) * DEG2RAD)
		+0.007843*cos( 2*D * DEG2RAD)
		+0.002824*cos( 2*M_prime * DEG2RAD)
		+0.000857*cos((2*D + M_prime) * DEG2RAD)
		+0.000533*cos((2*D - M) * DEG2RAD)*(e)
		+0.000401*cos((2*D - M - M_prime) * DEG2RAD)*(e)
		+0.000320*cos((M_prime - M) * DEG2RAD)*(e)
		-0.000271*cos( D * DEG2RAD)
		-0.000264*cos((M + M_prime) * DEG2RAD)*(e)
		-0.000198*cos((2*F - M_prime) * DEG2RAD)
		+0.000173*cos((3*M_prime) * DEG2RAD)
		+0.000167*cos((4*D - M_prime) * DEG2RAD)
		-0.000111*cos( M * DEG2RAD)*(e)
		+0.000103*cos((4*D - 2*M_prime) * DEG2RAD)
		-0.000084*cos((2*M_prime - 2*D) * DEG2RAD)
		-0.000083*cos((2*D + M) * DEG2RAD)*(e)
		+0.000079*cos((2*D + 2*M_prime) * DEG2RAD)
		+0.000072*cos( 4*D * DEG2RAD)
		+0.000064*cos((2*D - M + M_prime) * DEG2RAD)*(e)
		-0.000063*cos((2*D + M - M_prime) * DEG2RAD)*(e)
		+0.000041*cos((M + D) * DEG2RAD)*(e)
		+0.000035*cos((2*M_prime - M) * DEG2RAD)*(e)
		-0.000033*cos((3*M_prime - 2*D) * DEG2RAD)
		-0.000030*cos((M_prime + D) * DEG2RAD)
		-0.000029*cos((2*F - 2*D) * DEG2RAD)
		-0.000029*cos((M + 2*M_prime) * DEG2RAD)*(e)
		+0.000026*cos((2*D - 2*M) * DEG2RAD)*pow(e,2)
		-0.000023*cos((2*F - 2*D + M_prime) * DEG2RAD)
		+0.000019*cos((4*D - M - M_prime) * DEG2RAD)*(e);	

	return parallax;
}


double  getMoonPositionAngle(EQUATOR SunEqua, EQUATOR MoonEqua)
{
	double cosSD  = cos( SunEqua.declination * DEG2RAD);
	double sinSD  = sin( SunEqua.declination * DEG2RAD);	
	double cosMD  = cos(MoonEqua.declination * DEG2RAD);	
	double sinMD  = sin(MoonEqua.declination * DEG2RAD);	
	double sinSMa = sin( (SunEqua.ascension - MoonEqua.ascension) * DEG2RAD);
	double cosSMa = cos( (SunEqua.ascension - MoonEqua.ascension) * DEG2RAD);	

	double PositionAngle = atan2(cosSD*sinSMa , sinSD*cosMD - cosSD*sinMD*cosSMa)*RAD2DEG;
	
	if(PositionAngle<0) PositionAngle+=360;
	
	return PositionAngle;
}

double getMoonAge(DATE d)
{
	ECLIPTIC MoonEclip;
	double MoonAge;

	MoonEclip	= getMoonPosition(d);
	MoonAge		= MoonEclip.longitude - getSunEclipLong(d);

	if(MoonAge < 0) MoonAge+=360.;
	MoonAge= fmod( MoonAge/360 * SYNODIC_MONTH, SYNODIC_MONTH);	

	return MoonAge;
}

double getMoonPhase(DATE d)
{
	return ( 0.5* ( 1 - cos( getMoonAge(d)*(360/SYNODIC_MONTH) *DEG2RAD ) ) );
}

ECLIPTIC getMoonOpticLibration(DATE d)
{
	ECLIPTIC MoonEclip;
	ECLIPTIC MoonLibEclip;

	NUTATION MoonNutation;

	double I,sinI, cosI;	
	double W,sinW, cosW;
	double sinB, cosB;
	double A, F, T, Omega;
	
	T		= getJulianCentury(d);
	MoonEclip	= getMoonPosition(d);
	MoonNutation	= getNutation(d);

	/*
	Longitude(tropical, as measured from the mean equinox of the date)of the
	Moon's (mean) ascending node:
	which is the position on the ecliptic where it is crossed by te moon's orbit
	as it crosses from the southern to the northern hemisphere.
	With this formula for Omega, we can find the instants when the (mean) ascending/descending 
	node of the lunar orbit coincides with the vernal equinox, that is, when Omega=0deg or 180deg
	*/

	Omega = fmod (125.0445550 - 1934.1361849*T + 0.0020762*pow(T,2)
			+ (1.0/467410)*pow(T,3) - (1.0/60616000)*pow(T,4)
			,	360);

	//Mean distance of moon from its ascending node      
	F = fmod(93.2720993 + 483202.0175273*T - 0.0034029*pow(T,2) - (1.0/3526000)*pow(T,3)
	  	   + (1.0/863310000)*pow(T,4)	,360);

	//Inclination of the mean lunar equator to ecliptic, this is the value adopted by IAU(in degree)
	I	= 1.54242;
	W	= MoonEclip.longitude - MoonNutation.longitude - Omega;

	sinW	= sin(W*DEG2RAD);
	cosW	= cos(W*DEG2RAD);
	sinB	= sin(MoonEclip.latitude*DEG2RAD);	
	cosB	= cos(MoonEclip.latitude*DEG2RAD);
	sinI	= sin(I*DEG2RAD);
	cosI	= cos(I*DEG2RAD);
	
	A	= atan2(sinW*cosB*cosI - sinB*sinI, cosW*cosB)*RAD2DEG;

	if(A<0) A+=360;
	if(F<0) F+=360;

	//Optical librations in longitude and in Latitude
	MoonLibEclip.longitude	= A - F;

	// A-F will give 3xx degree if A~35x and F~36x,
	// because A and F are rounding to 360 degree, F will become 36x-360=x
	// and A is still 35x. The libration for this is actually negative number.
		
	if(MoonLibEclip.longitude>=300)
		MoonLibEclip.longitude-=360;

	MoonLibEclip.latitude	= asin( - sinW*cosB*sinI - sinB*cosI)*RAD2DEG;

	return MoonLibEclip;
}


EQUATOR getSunPosition(DATE d)
{
	EQUATOR equator;
	double T;
	double Lo, M, e, C, theta, epsilon;	//Sun parameters

	T= getJulianCentury(d);

	//Geometric mean longitude of the Sun, referred to the mean equinox of the date
	Lo = fmod( 280.46645 + 36000.76983*T + 0.0003032*pow(T,2)	,360);

	//Mean anomaly of the Sun
	M = fmod(357.52910 + 35999.05030*T - 0.0001559*pow(T,2) - 0.00000048*pow(T,3)
		   ,360);

	if(Lo<0) Lo+=360.0;
	if(M <0) M +=360.0;
		  	   
	//Eccentricity of the Earth's orbit around the Sun
	e = 0.016708617 - 0.000042037*T - 0.0000001236*pow(T, 2);

	//Sun's equation of center
	C = (1.914600 - 0.004817*T - 0.000014*pow(T,2) ) * sin(M * DEG2RAD)
	  + (0.019993 - 0.000101*T)*sin(2*M * DEG2RAD)
	  +  0.000290*sin(3*M * DEG2RAD);
  
	//Sun's true geometric longitude
	theta = Lo + C;

	epsilon = EclipticObliquity(d);
	
	equator.ascension	= atan2( cos(epsilon*DEG2RAD)*sin(theta*DEG2RAD), cos(theta*DEG2RAD) ) * RAD2DEG;
	equator.declination	= asin ( sin(epsilon*DEG2RAD)*sin(theta*DEG2RAD) ) * RAD2DEG;

	if(equator.ascension<0) equator.ascension += 360.0;

	return equator;
}

double getSunEclipLong(DATE d)
{
	double T;
	double Lo, M, C, theta;	//Sun parameters

	T= getJulianCentury(d);

	//Geometric mean longitude of the Sun, referred to the mean equinox of the date
	Lo = fmod( 280.46645 + 36000.76983*T + 0.0003032*pow(T,2)	,360);

	//Mean anomaly of the Sun
	M = fmod(357.52910 + 35999.05030*T - 0.0001559*pow(T,2) - 0.00000048*pow(T,3)
		   ,360);

	if(Lo<0) Lo+=360.0;
	if(M <0) M +=360.0;
		  	   
	//Sun's equation of center
	C = (1.914600 - 0.004817*T - 0.000014*pow(T,2) ) * sin(M * DEG2RAD)
	  + (0.019993 - 0.000101*T)*sin(2*M * DEG2RAD)
	  +  0.000290*sin(3*M * DEG2RAD);
  
	//Sun's true geometric longitude
	theta = Lo + C;	
	
	return theta;	
}

double getSunApparentLong(DATE d)
{
	double T	= getJulianCentury(d);
	double Omega	= 125.04 -1934.136*T;
	double lambda	= getSunEclipLong(d) - 0.00569 - 0.00478*sin(Omega*DEG2RAD);
	
	return lambda;	
}


//Returns the fraction of day of moonrise/set for a given day
//True moon rise/set/trans
RISETRANSET getMoonRiseTranSet(DATE d)
{
	RISETRANSET RTS;
	ECLIPTIC MoonEclip;
	EQUATOR MoonEqua;

	double GMST, HourAngle;
	double ho;
	
	//The 'standard' altitude, i.e. the geometric altitude of the center of the body
	//at the time of apparent rising/setting. (degree)
	ho= 0.7275*(getMoonParallax(d)) - (34.0/60);
	
	//GMST is the apparent sidereal time at "0h" UT on day D
	//for the Greenwich meridian, expressed in degree.
	d.hr= d.min= d.sec= 0;
	GMST = getGreenwichSidereal(d);

	MoonEclip = getMoonPosition(d);
	MoonEqua  = Ecliptic2Equator(d, MoonEclip);
	
	//The hour angle corresponding to the time of rise/set of a celestial object
	HourAngle = acos( (sin(ho*DEG2RAD) - sin(LOCAL_LAT*DEG2RAD)* sin(MoonEqua.declination*DEG2RAD))
				/( cos(LOCAL_LAT*DEG2RAD)* cos(MoonEqua.declination*DEG2RAD) )  ) * RAD2DEG;

	//fraction of day in UT
	RTS.transit = (MoonEqua.ascension - LOCAL_LONG - (GMST/24*360.0))/360.0;
	//Add GmtOffSet/24 to get local transit/rising/setting time 
	RTS.transit += GmtOffSet/24.0;

	if(RTS.transit <0) RTS.transit+=1;

	RTS.rising  = RTS.transit - (HourAngle/360.0);
	RTS.setting = RTS.transit + (HourAngle/360.0);


//	if(RTS.rising  <0) RTS.rising +=1;
//	if(RTS.setting <0) RTS.setting+=1;

	return RTS;
}

//Returns the fraction of day of sunrise/set for a given day
//True sun rise/set/trans
RISETRANSET getSunRiseTranSet(DATE d)
{
	RISETRANSET RTS;
	EQUATOR SunEqua;

	double GMST, HourAngle;
	double ho;

	//The 'standard' altitude, i.e. the geometric altitude of the center of the body
	//at the time of apparent rising/setting. (degree)
	ho= -0.83333;

	//GMST is the apparent sidereal time at "0h" UT on day D
	//for the Greenwich meridian, expressed in degree.
	d.hr= d.min= d.sec= 0;
	GMST = getGreenwichSidereal(d);

	SunEqua = getSunPosition(d);
	
	//The hour angle corresponding to the time of rise/set of a celestial object
	HourAngle = acos( ( sin(ho*DEG2RAD) - ( sin(LOCAL_LAT*DEG2RAD)* sin(SunEqua.declination*DEG2RAD) ) )
				/( cos(LOCAL_LAT*DEG2RAD)* cos(SunEqua.declination*DEG2RAD) )  ) * RAD2DEG;

	//fraction of day in UT
	RTS.transit = (SunEqua.ascension - LOCAL_LONG - (GMST/24*360.0))/360.0;
	//Add GmtOffSet/24 to get local transit/rising/setting time 
	RTS.transit += GmtOffSet/24.0;

	if(RTS.transit <0) RTS.transit+=1;

	RTS.rising  = RTS.transit - (HourAngle/360.0);
	RTS.setting = RTS.transit + (HourAngle/360.0);

//	if(RTS.rising  <0) RTS.rising +=1;
//	if(RTS.setting <0) RTS.setting+=1;

	return RTS;
}


//Returns the fraction of day of star rise/set for a given day
//True star rise/set/trans
RISETRANSET getStarRiseTranSet(DATE d, EQUATOR equator)
{
	RISETRANSET RTS;

	double GMST, LST, HourAngle;
	double ho;
	
	//The 'standard' altitude, i.e. the geometric altitude of the center of the body
	//at the time of apparent rising/setting. (degree)
	ho= -0.5667;
	LST = getLocalSidereal(d);
	GMST=LST-8;
	//The hour angle corresponding to the time of rise/set of a celestial object
	HourAngle = LST - equator.ascension;
	printf("GMST=%.5f, LST=%.5f, HourAngle=%.5f\n",GMST, LST, HourAngle);

	RTS.transit = (equator.ascension - LOCAL_LONG - (GMST/24*360.0))/360.0;
	RTS.rising  = RTS.transit - (HourAngle/360.0);
	RTS.setting = RTS.transit + (HourAngle/360.0);
	if(RTS.transit <0) RTS.transit+=1;
	if(RTS.rising <0)  RTS.rising +=1;
	if(RTS.setting<0)  RTS.setting+=1;
		
	return RTS;
}

NUTATION getNutation(DATE d)
{
	NUTATION aNutation;
	double T, D, M, M_prime, F, Omega;
	double longitude, obliquity;

	T = getJulianCentury(d);

	//Moon's mean elongation  
	D = fmod(297.85036 + 445267.111480*T - 0.0019142*pow(T,2) + (1.0/189474)*pow(T,3),	360);

	//Sun's mean anomaly
	M = fmod(357.52772 + 35999.050340*T - 0.0001603*pow(T,2) + (1.0/300000)*pow(T,3),	360);	
	
	//Moon's mean anomaly
	M_prime = fmod(134.96298 + 477198.867398*T + 0.0086972*pow(T,2) + (1.0/56250)*pow(T,3),	360);

	//Mean distance of moon from its ascending node      
	F = fmod(93.27191 + 483202.017538*T - 0.0036825*pow(T,2) + (1.0/327270)*pow(T,3),	360);

	Omega = fmod (125.04452 - 1934.136261*T + 0.0020708*pow(T,2)+ (1.0/450000)*pow(T,3),	360);

	// In units of 0.0001"
	longitude	=   ( - 171996 - 174.2*T )* sin( (Omega)*DEG2RAD )
			  + ( -  13187 -   1.6*T )* sin( (-2*D + 2*F + 2*Omega ) *DEG2RAD )
			  + ( -   2274 -   0.2*T )* sin( ( 2*F + 2*Omega )*DEG2RAD)
			  + ( +   2062 +   0.2*T )* sin( (2*Omega)*DEG2RAD)
			  + ( +   1426 -   3.4*T )* sin( (M)*DEG2RAD)
			  + ( +    712 +   0.1*T )* sin( (M_prime)*DEG2RAD)
			  + ( -    517 +   1.2*T )* sin( (-2*D + M + 2*F + 2*Omega )*DEG2RAD)
			  + ( -    386 -   0.4*T )* sin( ( 2*F + Omega ) *DEG2RAD )
			  + ( -    301           )* sin( ( M_prime + 2*F + 2*Omega )*DEG2RAD)
			  + ( +    217 -   0.5*T )* sin( (-2*D - M + 2*F + 2*Omega)*DEG2RAD)
			  + ( -    158           )* sin( (-2*D + M_prime)*DEG2RAD)
			  + ( +    129 +   0.1*T )* sin( (-2*D + 2*F + Omega)*DEG2RAD)
			  + ( +    123           )* sin( (-M_prime + 2*F + 2*Omega )*DEG2RAD)
			  + ( +     63           )* sin( (2*D) *DEG2RAD )
			  + ( +     63 +   0.1*T )* sin( (M_prime + Omega ) *DEG2RAD )
			  + ( -     59           )* sin( (2*D - M_prime + 2*F + 2*Omega )*DEG2RAD)
			  + ( -     58 -   0.1*T )* sin( (-M_prime + Omega)*DEG2RAD)
			  + ( -     51           )* sin( (M_prime + 2*F + Omega)*DEG2RAD)
			  + ( +     48           )* sin( (-2*D + 2*M_prime)*DEG2RAD)
			  + ( +     46           )* sin( (-2*M_prime + 2*F + Omega)*DEG2RAD)
			  + ( -     38           )* sin( (2*D + 2*F + 2*Omega)*DEG2RAD)
			  + ( -     31           )* sin( (2*M_prime + 2*F + 2*Omega)*DEG2RAD)
			  + ( +     29           )* sin( (2*M_prime)*DEG2RAD)
			  + ( +     29           )* sin( (-2*D + M_prime + 2*F + 2*Omega)*DEG2RAD)
			  + ( +     26           )* sin( (2*F)*DEG2RAD)
			  + ( -     22           )* sin( (-2*D + 2*F)*DEG2RAD)
			  + ( +     21           )* sin( (-M_prime + 2*F + Omega)*DEG2RAD)
			  + ( +     17 -   0.1*T )* sin( (2*M)*DEG2RAD)
			  + ( +     16           )* sin( (2*D - M_prime + Omega)*DEG2RAD)
			  + ( -     16 +   0.1*T )* sin( (-2*D + 2*M + 2*F + 2*Omega)*DEG2RAD)
			  + ( -     15           )* sin( (M + Omega)*DEG2RAD)

			  - 13 * sin( (-2*D + M_prime + Omega )*DEG2RAD)
			  - 12 * sin( (-M + Omega )*DEG2RAD)
			  + 11 * sin( (2*M_prime - 2*F )*DEG2RAD)
			  - 10 * sin( (2*D - M_prime + 2*F + Omega )*DEG2RAD)
			  -  8 * sin( (2*D + M_prime + 2*F + 2*Omega )*DEG2RAD)
			  +  7 * sin( (M + 2*F+ 2*Omega )*DEG2RAD)
			  -  7 * sin( (-2*D + M + M_prime )*DEG2RAD)
			  -  7 * sin( (-M + 2*F + 2*Omega )*DEG2RAD)
			  -  7 * sin( (2*D + 2*F + Omega )*DEG2RAD)
			  +  6 * sin( (2*D + M_prime )*DEG2RAD)
			  +  6 * sin( (-2*D + 2*M_prime + 2*F + 2*Omega )*DEG2RAD)
			  +  6 * sin( (-2*D + M_prime + 2*F + Omega )*DEG2RAD)
			  -  6 * sin( (2*D - 2*M_prime + Omega )*DEG2RAD)
			  -  6 * sin( (2*D + Omega )*DEG2RAD)
			  +  5 * sin( (-M + M_prime )*DEG2RAD)
			  -  5 * sin( (-2*D - M + 2*F + Omega )*DEG2RAD)
			  -  5 * sin( (-2*D + Omega )*DEG2RAD)
			  -  5 * sin( (2*M_prime + 2*F + Omega )*DEG2RAD)
			  +  4 * sin( (-2*D + 2*M_prime + Omega )*DEG2RAD)
			  +  4 * sin( (-2*D + M + 2*F + Omega )*DEG2RAD)
			  +  4 * sin( (M_prime - 2*F )*DEG2RAD)
			  -  4 * sin( (-D + M_prime )*DEG2RAD)
			  -  4 * sin( (-2*D + M )*DEG2RAD)
			  -  4 * sin( (D)*DEG2RAD)
			  +  3 * sin( (M_prime + 2*F )*DEG2RAD)
			  -  3 * sin( (-2*M_prime + 2*F + 2*Omega )*DEG2RAD)
			  -  3 * sin( (-D - M + M_prime )*DEG2RAD)
			  -  3 * sin( (M + M_prime )*DEG2RAD)
			  -  3 * sin( (-M + M_prime + 2*F + 2*Omega )*DEG2RAD)
			  -  3 * sin( (2*D - M - M_prime + 2*F + 2*Omega )*DEG2RAD)
			  -  3 * sin( (3*M_prime + 2*F + 2*Omega )*DEG2RAD)
			  -  3 * sin( (2*D - M + 2*F + 2*Omega )*DEG2RAD);


	// In units of 0.0001"
	obliquity	=   ( + 92025 + 8.9*T )* cos( (Omega)*DEG2RAD )
			  + ( +  5736 - 3.1*T )* cos( (-2*D + 2*F + 2*Omega ) *DEG2RAD )
			  + ( +   977 - 0.5*T )* cos( ( 2*F + 2*Omega )*DEG2RAD)
			  + ( -   895 + 0.5*T )* cos( (2*Omega)*DEG2RAD)
			  + ( +    54 - 0.1*T )* cos( (M)*DEG2RAD)
			  + ( -     7         )* cos( (M_prime)*DEG2RAD)
			  + ( +   224 - 0.6*T )* cos( (-2*D + M + 2*F + 2*Omega )*DEG2RAD)
			  + ( +   200         )* cos( ( 2*F + Omega ) *DEG2RAD )
			  + ( +   129 - 0.1*T )* cos( ( M_prime + 2*F + 2*Omega )*DEG2RAD)
			  + ( -    95 + 0.3*T )* cos( (-2*D - M + 2*F + 2*Omega)*DEG2RAD)

			    -    70 * cos( (-2*D + 2*F + Omega)*DEG2RAD)
			    -    53 * cos( (-M_prime + 2*F + 2*Omega )*DEG2RAD)

			    -    33 * cos( (M_prime + Omega ) *DEG2RAD )
			    +    26 * cos( (2*D - M_prime + 2*F + 2*Omega )*DEG2RAD)
			    +    32 * cos( (-M_prime + Omega)*DEG2RAD)
			    +    27 * cos( (M_prime + 2*F + Omega)*DEG2RAD)

			    -    24 * cos( (-2*M_prime + 2*F + Omega)*DEG2RAD)
			    +    16 * cos( (2*D + 2*F + 2*Omega)*DEG2RAD)
			    +    13 * cos( (2*M_prime + 2*F + 2*Omega)*DEG2RAD)

			    -    12 * cos( (-2*D + M_prime + 2*F + 2*Omega)*DEG2RAD)


			    -    10 * cos( (-M_prime + 2*F + Omega)*DEG2RAD)

			    -     8 * cos( (2*D - M_prime + Omega)*DEG2RAD)
			    +     7 * cos( (-2*D + 2*M + 2*F + 2*Omega)*DEG2RAD)
			    +     9 * cos( (M + Omega)*DEG2RAD)

			    +     7 * cos( (-2*D + M_prime + Omega )*DEG2RAD)
			    +     6 * cos( (-M + Omega )*DEG2RAD)

			    +     5 * cos( (2*D - M_prime + 2*F + Omega )*DEG2RAD)
			    +     3 * cos( (2*D + M_prime + 2*F + 2*Omega )*DEG2RAD)
			    -     3 * cos( (M + 2*F+ 2*Omega )*DEG2RAD)

			    +     3 * cos( (-M + 2*F + 2*Omega )*DEG2RAD)
			    +     3 * cos( (2*D + 2*F + Omega )*DEG2RAD)

			    -     3 * cos( (-2*D + 2*M_prime + 2*F + 2*Omega )*DEG2RAD)
			    -     3 * cos( (-2*D + M_prime + 2*F + Omega )*DEG2RAD)
			    +     3 * cos( (2*D - 2*M_prime + Omega )*DEG2RAD)
			    +     3 * cos( (2*D + Omega )*DEG2RAD)

			    +     3 * cos( (-2*D - M + 2*F + Omega )*DEG2RAD)
			    +     3 * cos( (-2*D + Omega )*DEG2RAD)
			    +     3 * cos( (2*M_prime + 2*F + Omega )*DEG2RAD);


	aNutation.longitude = longitude*0.0001/3600;//Converts to degree
	aNutation.obliquity = obliquity*0.0001/3600;
	
	return aNutation; 
}



char *Deg2Dms(double degree)
{
	char  *dms;
        int   deg = (int)    degree;
        int   min = (int)   ((degree - deg)* 60);
        float sec = (float)((degree - deg)* 60 - min)* 60;

        dms=(char *)malloc(64*sizeof(char));
	sprintf(dms ,"%03dd %02dm %6.2fs", deg, abs(min), fabs(sec) );
        return dms;
}

char *Deg2Hms(double degree)
{
	char  *hms;
        int   hr = (int)   (degree*DEG2HOUR);
        int   min  = (int)  ((degree*DEG2HOUR - hr)* 60);
        float sec  = (float)((degree*DEG2HOUR - hr)* 60 - min)* 60;
        
        hms=(char *)malloc(64*sizeof(char));
	sprintf(hms ,"%03dh %02dm %6.2fs", hr, abs(min), fabs(sec));
        return hms;
}
    
