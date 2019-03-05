#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "symblc_const.h"
#include "ppcdaemon.h"
#include <stdlib.h> 



/*  
  Function prototypes
*/

	int fexist (char*);
	void faketiming (long );
	void timing (long );
	int timing_and_do_something (long );
 	long DoGetValue (char*);
 	char DoGetValueChar( char *argName);
 	char* DoGetValueString (char*, char*);
	void doreverse (char*);
	void itoa (long, char*);
	void step (char*);
	void steplog (char*, int);
	int read_filter_string(char *Filter_seq, int *filter, int *filter_expose, int *filter_obs);
	void generate_web_image(char *image_file, int type);
	int get_log_filename(char *filename, int type,int mode);//mode 1=create new, 0=join to previous
	void log_this(char *string,int type, int mode);//mode 1=create new, 0=join to previous
 	void get_filter_array_char(char *filters);
    
