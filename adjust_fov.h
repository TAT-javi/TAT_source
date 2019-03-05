typedef struct
{
	int ra,dec;
	double R;
	int timestamp;
	char status[32],frame[300];
}SHIFT;

typedef struct
{
	int X,Y,saturated_count,N,total_coord;
}STAR;

typedef struct
{
	int X,Y,Nsat,star;
	float peak;
	int Z[50][50];
}SATURA;

typedef struct
{
	int X,Y,total_coord;
}STAR_HEADER;



#define MAX_AVG				4000 /* Maximum average pixel count to consider as good file*/
#define SIGMA_FACTOR		3 /*count pixel above the average to consider it from a star*/
#define MIN_PIXEL			100 /*Minimum of final pixels to consider it a good model*/
#define	GOOD_PIXEL_COUNT	45000 /* good amount of counts for saturated pixels*/
#define MIN_PIXEL		100 /*Minimum of final pixels to consider it a good mask*/
#define SATURATED		62000 /*pixels count to consider it saturated*/
#define BOX_RADIUS		12 /*Box which have saturated pixels estimation*/
#define GOOD_SHIFT		 5 /*Amount of shifted pixels to consider the FOV correct*/
#define GOOD_MOVE_SHIFT		 3 /*Amount of shifted pixels to move the FOV */
#define MIN_EXPOSURE_TIME 	60.0	/*For correcting saturation problem*/


//For astrometry
#define WAIT 120 
#define BUFF_SIZE 1024

#define ASTRO_PROGRAM "/opt/astrometry/bin/astrometry-engine"
#define ASTRO_CANCEL_FILE "cancel_astrometry.txt"
#define ASTRO_SOLVED_FILE "solved_field.txt"
#define FIELD_PROGRAM "/opt/astrometry/bin/solve-field"
#define FIELD_OPTIONS "--just-augment -O --axy"
#define AUG_FILE "augment_file.axy"
#define INFILE "infile.txt"
#define INDEX_PATH "/opt/astrometry/data"


void move_pixel(int RA, int DEC, int mode);
void move_pulse(int RA, int DEC);
float sorting(float *array, int N);
int masquerade(float *image_array, float *background);
void write_shift_data(SHIFT db);

//For astrometry
int deg_to_index(float RA, float DEC, int *indexes);
int get_RA_DEC_astrometry(char *filename, float *ra, float *dec);
void remove_astrometry_files(char *filename);
