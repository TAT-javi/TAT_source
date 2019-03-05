#include "myAstro.h"
#define ONEHOUR  3600
#define HALFHOUR 1800
#define TENHOURS 36000 
#define SIXTEENHOURS 57600 


int check_input_file(void);
int check_single_line(int,DATE,DATE,char *,char *,int,int,int,char *,char);
int insert_obs(char *,char *,char *,char *,int , char *, char , char *);
void getSunriseSunset(DATE , double *, double *, double *);
