#include <stdio.h>
#include <stdlib.h>
#include "dsp_func-2.h" //*to send cmd to dsp*/
#include "symblc.const.h" /*here is the information about pixels*/

#define STAR_SPEED 				1./(20*120e-9*9973)
#define PIX_STAR_SPEED 			STAR_SPEED/PULSEPERPIX_RA  /*Star speed in pixel per second*/

void move_pixel(int RA, int DEC)//Move RA and DEC
{
	int ra_sign,dec_sign;
	int ra_speed,dec_speed;
	int ra_reg,dec_reg,wait_seconds,ra_pixel;
	float temp;
	char dec_cmd[200],ra_cmd[200];

	//MOVE DEC 
	if(DEC)
	{
		if(DEC < 0){dec_sign = 1; DEC *=-1;}
		else dec_sign=0;
	
		dec_speed = (DEC*PULSEPERPIX_DEC)/2; //speed to move the telescope that amount of pixels in 5 sec
		dec_reg =(int)(0.5 + 1./(20*120.0e-9*dec_speed));
		sprintf( dec_cmd, "DEC %s FREQUENCY 1 2 %d 0 0 0 0\n",dec_sign?"POS":"NEG", dec_reg);
		//fprintf(fp,"%s\n",dec_cmd);
#ifdef _CURSES_
		mvsend_cmd2dsp( dec_cmd );
#else
		send_cmd2dsp( dec_cmd ); 
#endif
	}

	if(RA)
	{
		if(RA < 0){ra_sign = 1; RA *= -1;}
		else ra_sign=0;

		if(!ra_sign)//move EAST
		{
			ra_speed = (RA*PULSEPERPIX_RA)/10; //speed to move the telescope that amount of pixels in 10 sec
			ra_speed += STAR_SPEED;
			ra_reg =(int)(0.5 + 1./(20*120.0e-9*ra_speed));
			sprintf( ra_cmd, "RA POS FREQUENCY 1 20 %d 0 0 0 0\n",ra_reg);
		//	fprintf(fp,"%s\n",ra_cmd);
#ifdef _CURSES_
			mvsend_cmd2dsp( ra_cmd );
#else
			send_cmd2dsp( ra_cmd );
#endif
			sleep(10);
			sprintf( ra_cmd, "RA POS FREQUENCY 1 30030 9973 0 0 0 0\n");
// 			fprintf(fp,"%s\n",ra_cmd);
#ifdef _CURSES_
			mvsend_cmd2dsp( ra_cmd );
#else
			send_cmd2dsp( ra_cmd );
#endif
		}
		else//move WEST
		{
			temp = PIX_STAR_SPEED;
			//compute how many pixels do we at least have to wait
			wait_seconds= (int)(0.5+ RA/temp) +10;
			//calculate de difference long wait and the pixel
			ra_pixel = (int) (0.5+( PIX_STAR_SPEED * wait_seconds - RA));
			//move to that amount of pixel to wait
			ra_speed = (ra_pixel*PULSEPERPIX_RA/3);
			ra_reg =(int)(0.5 + 1./(20*120.0e-9*ra_speed));
			sprintf( ra_cmd, "RA POS FREQUENCY 1 3 %d 0 0 0 0\n",ra_reg);
// 			fprintf(fp,"%s\n",ra_cmd);
#ifdef _CURSES_
			mvsend_cmd2dsp( ra_cmd );
#else
			send_cmd2dsp( ra_cmd );
#endif
			sleep(wait_seconds);//wait for the star
			sprintf( ra_cmd, "RA POS FREQUENCY 1 30030 9973 0 0 0 0\n");
// 			fprintf(fp,"%s\n",ra_cmd);
#ifdef _CURSES_
			mvsend_cmd2dsp( ra_cmd );
#else
			send_cmd2dsp( ra_cmd );
#endif
		}
	}
}
