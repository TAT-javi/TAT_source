/*
	TEN Hardware Powers Control daemon
	----------------------------------
Control the hardware powers through parallel port IO card.
How to compile:
cc pwrdaemon.c -O2
Optimized option is needed when compile for inb/outb functions.

6.Feb 2003

--------------------------------------------------------------------------------------------------
>/sbin/lspci -v 
...
02:04.0 Communication controller: NetMos Technology VScom 021H-EP2 2 port parallel adaptor (rev 01)
        Subsystem: LSI Logic / Symbios Logic: Unknown device 0020
        Flags: medium devsel, IRQ 10
        I/O ports at 9000 [size=8]
        I/O ports at 9400 [size=8]
        I/O ports at 9800 [size=8]
        I/O ports at 9c00 [size=8]
        I/O ports at a000 [size=8]
        I/O ports at a400 [size=16]
...
ppc1
	#define DATA 9000
ppc2
	#define DATA 9800

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include "par_func.h"
/*
Note for Parallel Port DB25 PINS:(PRIME '  mean that bit is invert logic)
PIN1	:	IN/OUT			(C0')
PIN2-9	:	IN/OUT(grouped)		(D0-D7)
PIN10	:	IN, INTERRUPT GENERATOR (S6)
PIN11	:	IN			(S7')
PIN12	:	IN			(S5)
PIN13	:	IN			(S4)
PIN14	:	IN/OUT			(C1')
PIN15	:	IN			(S3)
PIN16	:	IN/OUT			(C2)
PIN17	:	IN/OUT			(C3')
PIN18-25:	GND

*/

/*
In Linux, the address of the PCI IO card can be found using the command:
'lspci -v' (as root)
or type
'more /proc/pci'
*/

void pio_wr( int base_address, int pin_num, int dat )
{
  int DATA=base_address;
  int CONTROL=base_address+2;
  switch(pin_num)
  {
    case 1:	/* C0' */
      if (dat!=0) outb( inb(CONTROL) | PIN01REG, CONTROL);	/* wr 1 */
      else outb( inb(CONTROL) & ~PIN01REG ,CONTROL); /* wr 0 */
    break;
    case 2:	/* D0 */
      if (dat!=0) outb( inb(DATA) | PIN02REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN02REG, DATA);	/* wr 0 */
    break;
    case 3:	/* D1 */
      if (dat!=0) outb( inb(DATA) | PIN03REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN03REG, DATA);	/* wr 0 */
    break;
    case 4:	/* D2 */
      if (dat!=0) outb( inb(DATA) | PIN04REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN04REG, DATA);	/* wr 0 */
    break;
    case 5:	/* D3 */
      if (dat!=0) outb( inb(DATA) | PIN05REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN05REG, DATA);	/* wr 0 */
    break;
    case 6:	/* D4 */
      if (dat!=0) outb( inb(DATA) | PIN06REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN06REG, DATA);	/* wr 0 */
    break;
    case 7:	/* D5 */
      if (dat!=0) outb( inb(DATA) | PIN07REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN07REG, DATA);	/* wr 0 */
    break;
    case 8:	/* D6 */
      if (dat!=0) outb( inb(DATA) | PIN08REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN08REG, DATA);	/* wr 0 */
    break;
    case 9:	/* D7 */
      if (dat!=0) outb( inb(DATA) | PIN09REG ,DATA);	/* wr 1 */
      else outb( inb(DATA) & ~PIN09REG, DATA);	/* wr 0 */
    break;
    case 14:	/* C1' */
      if (dat!=0) outb( inb(CONTROL) | PIN14REG, CONTROL);	/* wr 1 */
      else outb( inb(CONTROL) & ~PIN14REG ,CONTROL);	/* wr 0 */
    break;

    case 16:	/* C2 */
      if (dat!=0) outb( inb(CONTROL) | PIN16REG ,CONTROL);	/* wr 1 */
      else outb( inb(CONTROL) & ~PIN16REG, CONTROL);	/* wr 0 */
    break;

    case 17:	/* C3' */
      if (dat!=0) outb( inb(CONTROL) | PIN17REG, CONTROL);	/* wr 1 */
      else outb( inb(CONTROL) & ~PIN17REG ,CONTROL);	/* wr 0 */
    break;

    default:
    break;
    
  }


}
int pio_rd( int base_address, int pin_num)
{
  int DATA=base_address;
  int CONTROL=base_address+2;
  int STATUS=base_address+1;
  switch( pin_num )
  {
    case 1:	/* C0' */
      return (inb( CONTROL ) & PIN01REG)==0?0:1;
    break;
    case 2:	/* D0 */
      return (inb( DATA ) & PIN02REG)==0?0:1;
    break;
    case 3:	/* D1 */
      return (inb( DATA ) & PIN03REG)==0?0:1;
    break;
    case 4:	/* D2 */
      return (inb( DATA ) & PIN04REG)==0?0:1;
    break;
    case 5:	/* D3 */
      return (inb( DATA ) & PIN05REG)==0?0:1;
    break;
    case 6:	/* D4 */	
      return (inb( DATA ) & PIN06REG)==0?0:1;
    break;
    case 7:	/* D5 */
      return (inb( DATA ) & PIN07REG)==0?0:1;
    break;
    case 8:	/* D6 */
      return (inb( DATA ) & PIN08REG)==0?0:1;
    break;
    case 9:	/* D7 */
      return (inb( DATA ) & PIN09REG)==0?0:1;
    break;
    case 10:	/* S6 */
      return (inb( DATA ) & PIN10REG)==0?0:1;
    break;
    case 11:	/* S7' */
      return (inb( STATUS ) & PIN11REG)==0?0:1;
    break;
    case 12:	/* S5 */
      return (inb( STATUS ) & PIN12REG)==0?0:1;
    break;
    case 13:	/* S4 */
      return (inb( STATUS ) & PIN13REG)==0?0:1;
    break;
    case 14:	/* C1' */
      return (inb( CONTROL ) & PIN14REG)==0?0:1;
    break;
    case 15:	/* S3 */
      return (inb( STATUS ) & PIN15REG)==0?0:1;
    break;
    case 16:	/* C2 */
      return (inb( CONTROL ) & PIN16REG)==0?0:1;
    break;
    case 17:	/* C3' */
      return (inb( CONTROL ) & PIN17REG)==0?0:1;
    break;
    
  
  }  
  return -1;


}
