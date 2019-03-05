#ifndef _AUTO_OBSERVE_FUNC_
	#define _AUTO_OBSERVE_FUNC_

/* function proto type */
	int DoGetObserveTime(float *, time_t *, time_t *);
	int timing_and_do_something(long );
	int check_multiple_observation(char *,char *);
	int set_current_observation(char *,char *);
	void update_flat_dark_strings(char *,char *,char *);
	void add_option_tostring(char *, char *, int );
#endif
