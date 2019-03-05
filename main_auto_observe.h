#ifndef _AUTO_OBSERVE_FUNC_
	#define _AUTO_OBSERVE_FUNC_

/* function proto type */
	int DoGetObserveTime(float *, time_t *, time_t *);
	int wait4star_rise(void);
	int guard(int type);
	void finish_observation(char *,	char *,int );
	int take_flat_process(int , char *);
#endif
