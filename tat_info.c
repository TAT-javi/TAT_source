#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "tat_info.h"

/* create shm */
void create_tat_info_shm ( int *p_shmid, st_tat_info **p_shared_memory)
{
	*p_shmid = shmget( (key_t) KEY_TAT_INFO, sizeof(st_tat_info), 0666|IPC_CREAT);
	if (*p_shmid == -1)
	{
		fprintf( stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		*p_shared_memory = shmat( *p_shmid,(void *) 0, 0);
		if (p_shared_memory == (void *) -1)
		{
			fprintf( stderr, "shmat failed\n");
			exit(EXIT_FAILURE);
		}
//		else
//		{
//			printf("create_tat_info_shm::Memory attached at %x\n", (int)*p_shared_memory);
//		}
	}
}

/* remove shm */
void remove_tat_info_shm (int *p_shmid, st_tat_info *p_shared_memory)
{
	if (shmdt( p_shared_memory) == -1)
	{
		fprintf( stderr,"shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		if (shmctl (*p_shmid, IPC_RMID, 0) == -1)
		{
			fprintf( stderr,"shmctl failed\n");
			exit(EXIT_FAILURE);
		}
	}
		
}


/* update */
void update_dsp_info ( st_dsp_info *p_object, int *msg)
{
	int *p_msg;
	p_msg=msg;
/*	for( i=0 ;i<25; i++, p_msg++)
	{
		printf("update_dsp_info::%d: %d\n", i, *p_msg);
	}
*/	p_msg=msg;
	p_object->time_count	= *(p_msg++);
	p_object->ra.reg	= *(p_msg++); 
	p_object->ra.pulse	= *(p_msg++);
	p_object->ra.dir	= *(p_msg++);
	p_object->ra.status	= *(p_msg++);
	p_object->ra.pulsectl	= *(p_msg++);
	p_object->ra.refpulse	= *(p_msg++);
	p_object->ra.purpulse	= *(p_msg++);
	p_object->dec.reg	= *(p_msg++);
	p_object->dec.pulse	= *(p_msg++);
	p_object->dec.dir	= *(p_msg++);
	p_object->dec.status	= *(p_msg++);
	p_object->dec.pulsectl	= *(p_msg++);
	p_object->dec.refpulse	= *(p_msg++);
	p_object->dec.purpulse	= *(p_msg++);
	p_object->enc.opened_ls	= *(p_msg++);
	p_object->enc.closed_ls	= *(p_msg++);
	p_object->tran.move	= *(p_msg++);
	p_object->tran.mode	= *(p_msg++);
	p_object->wheel.move	= *(p_msg++);
	p_object->wheel.mode	= *(p_msg++);
	p_object->latch.unlock	= *(p_msg++);
	p_object->latch.lock	= *(p_msg++);
	p_object->latch.time	= *(p_msg++);
	p_object->enc.open	= *(p_msg++);
	p_object->enc.close	= *(p_msg++);
	p_object->enc.time	= *(p_msg++);
	p_object->ra.origin	= *(p_msg++);
	p_object->dec.origin	= *(p_msg++);
}

void update_ccd_infof6( st_ccd_info *p_object, float *p_msg)
{
	p_object->curr_point=*(p_msg++);
	p_object->set_point=*(p_msg++);
	p_object->fan_status=*(p_msg++);
	p_object->camera_status=*(p_msg++);
	p_object->cooler_status=*(p_msg++);
}

void update_ccd_info( st_ccd_info *p_object, float *p_msg)
{
	p_object->curr_point=*(p_msg++);
	p_object->set_point=*(p_msg++);
	p_object->camera_status=(char)*(p_msg++);
	p_object->cooler_status=(char)*(p_msg++);
	p_object->cooler_mode=(char)*(p_msg++);
	p_object->present=(char)*(p_msg++);
}

void update_ppc_info( st_ppc_info *p_object, char *p_msg)
{
	p_object->control=*(p_msg++);
	p_object->data=*(p_msg++);
	p_object->status=*(p_msg++);
}

void update_pwr_info( st_ppc_info *p_object, char *p_msg)
{
	p_object->control=*(p_msg++);
	p_object->data=*(p_msg++);
	p_object->status=*(p_msg++);
}

void update_ctl_info( st_ctl_info *p_object, char *p_msg)
{
	p_object->dspd=*(p_msg++);
	p_object->ccdd=*(p_msg++);
	p_object->ppcd=*(p_msg++);
	p_object->pwrd=*(p_msg++);
	p_object->flid=*(p_msg++);
	p_object->lstd=*(p_msg++);
	p_object->aobs=*(p_msg++);
}

/* init */
void init_tel_info( st_tel_info *p_object)
{
	p_object->reg=0;
	p_object->pulse=0;
	p_object->dir=0;
	p_object->status=0;
	p_object->pulsectl=0;
	p_object->refpulse=0;
	p_object->purpulse=0;
}

void init_mx_info( st_mx_info *p_object)
{
	p_object->move=0;
	p_object->mode=0;
}

void init_enc_info( st_enc_info *p_object)
{
	p_object->open=0;
	p_object->close=0;
	p_object->time=0;
}

void init_latch_info( st_latch_info *p_object)
{
	p_object->lock=0;
	p_object->unlock=0;
	p_object->time=0;
}

void init_dsp_info( st_dsp_info *p_object)
{
	init_tel_info( &(p_object->ra) );
	init_tel_info( &(p_object->dec) );
	init_mx_info( &(p_object->rot) );
	init_mx_info( &(p_object->tran) );
	init_mx_info( &(p_object->wheel) );
}


void init_ccd_info( st_ccd_info *p_object)
{
	p_object->curr_point=99;
	p_object->set_point=99;
	p_object->camera_status=-10;
	p_object->cooler_status=-10;
	p_object->fan_status=-10;
	p_object->server_connected=0;
	p_object->ccd_locked=0;
}

void init_ppc_info( st_ppc_info *p_object)
{	
	p_object->control=0;
	p_object->data=0;
	p_object->status=0;	
}

void init_pwr_info( st_ppc_info *p_object)
{
	p_object->control=0;
	p_object->data=0;
	p_object->status=0;
}

void init_ctl_info( st_ctl_info *p_object)
{
	p_object->dspd=0;
	p_object->ccdd=0;
	p_object->ppcd=0;
	p_object->pwrd=0;
	p_object->flid=0;
	p_object->lstd=0;
}

void init_fli_info(st_fli_info *p_object)
{
	p_object->focuser_moving=0;
	p_object->focuser_curr_position=0;
	p_object->wheel_moving=0;
	p_object->wheel_curr_position=0;
}

/* display */
void disp_tel_info( st_tel_info *p_object)
{	
	printf("reg=%d\tpulse=%d\tdir=%d\tstatus=%d\n\
		\rpulsectl=%d\trefpulse=%d\tpurpulse=%d\n",
		p_object->reg,
		p_object->pulse,
		p_object->dir,
		p_object->status,
		p_object->pulsectl,
		p_object->refpulse,
		p_object->purpulse);
}

void disp_mx_info( st_mx_info *p_object)
{	
	printf("move=%d\tmode=%d\n",
		p_object->move,
		p_object->mode);
}

void disp_enc_info( st_enc_info *p_object)
{	
	printf("open=%d\tclose=%d\ttime=%d\n",
		p_object->open,
		p_object->close,
		p_object->time);
}

void disp_latch_info( st_latch_info *p_object)
{	
	printf("lock=%d\tunlock=%d\ttime=%d\n",
		p_object->lock,
		p_object->unlock,
		p_object->time);
}

void disp_dsp_info ( st_dsp_info *p_object)
{
	printf("time_count=%d\n",p_object->time_count);
	disp_tel_info ( &(p_object->ra));
	disp_tel_info ( &(p_object->dec));
	disp_mx_info ( &(p_object->rot));
	disp_mx_info ( &(p_object->tran));
	disp_enc_info ( &(p_object->enc));
	disp_latch_info ( &(p_object->latch));
}

void disp_ccd_info( st_ccd_info *p_object)
{
	printf( "curr point=%5f\nset  point=%5f\ncamera status=%3d\ncooler status=%3d\nfan status=%3d\n",
		p_object->curr_point,p_object->set_point,p_object->camera_status,p_object->cooler_status,p_object->fan_status);
}

void disp_ppc_info( st_ppc_info *p_object)
{
	printf( "control=%3d\ndata=%3d\nstatus=%3d\n",
		p_object->control,
		p_object->data,
		p_object->status);
}

void disp_pwr_info( st_ppc_info *p_object)
{
	printf( "control=%3d\ndata=%3d\nstatus=%3d\n",
		p_object->control,
		p_object->data,
		p_object->status);
}



