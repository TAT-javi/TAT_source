
#include <assert.h>
#include <sys/io.h>
#include <sys/time.h>                                                           
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


#include "usb.h"

#define VND_ANCHOR_LOAD_INTERNAL		0xA0
#define VND_APOGEE_CMD_BASE			0xC0
#define VND_APOGEE_STATUS			( VND_APOGEE_CMD_BASE + 0x0 )
#define VND_APOGEE_CAMCON_REG			( VND_APOGEE_CMD_BASE + 0x2 )
#define VND_APOGEE_BUFCON_REG			( VND_APOGEE_CMD_BASE + 0x3 )
#define VND_APOGEE_SET_SERIAL			( VND_APOGEE_CMD_BASE + 0x4 )
#define VND_APOGEE_SERIAL			( VND_APOGEE_CMD_BASE + 0x5 )
#define VND_APOGEE_EEPROM			( VND_APOGEE_CMD_BASE + 0x6 )
#define VND_APOGEE_SOFT_RESET			( VND_APOGEE_CMD_BASE + 0x8 )
#define VND_APOGEE_GET_IMAGE			( VND_APOGEE_CMD_BASE + 0x9 )
#define VND_APOGEE_STOP_IMAGE			( VND_APOGEE_CMD_BASE + 0xA )

#define USB_ALTA_VENDOR_ID	0x125c
#define USB_ALTA_PRODUCT_ID	0x0010
#define USB_DIR_IN  USB_ENDPOINT_IN
#define USB_DIR_OUT USB_ENDPOINT_OUT
struct usb_dev_handle	*g_hSysDriver;
struct usb_dev_handle *hDevice;
int ApnUsbReadReg( unsigned short FpgaReg, unsigned short *FpgaData );
int ApnUsbWriteReg( unsigned short FpgaReg, unsigned short FpgaData );


int main(int argc,char **argv)
{
	char deviceName[128];
	struct usb_bus *bus;
	struct usb_device *dev;
        int Success;
        unsigned short FpgaReg;
        unsigned short FpgaData;
	char string[256];
        unsigned char buf[64];
        unsigned char *cbuf;

	usb_init();

	usb_find_busses();
	usb_find_devices();


	int found = 0;

	/* find ALTA device */
	for(bus = usb_busses; bus && !found; bus = bus->next) {
		for(dev = bus->devices; !found && dev; dev = dev->next) {
			if (dev->descriptor.idVendor == USB_ALTA_VENDOR_ID && 
			    dev->descriptor.idProduct == USB_ALTA_PRODUCT_ID) {
				hDevice = usb_open(dev);
				found = 1;
				if (hDevice) {
				}
				else return -1;
			}
		}
	}

	if (!found) return -1;
/*	if (!usb_set_configuration(hDevice, 0x1)) return -1; */
/*	if (!usb_claim_interface(hDevice, 0x0)) return -1;  */
  
	printf("DRIVER: opened device\n");
        FpgaReg = 53;
        FpgaData = 0xa55a;
        ApnUsbWriteReg(FpgaReg,FpgaData);
        ApnUsbReadReg(FpgaReg,&FpgaData);
        FpgaData = 0x5aa5;
        ApnUsbWriteReg(FpgaReg,FpgaData);
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=90;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=91;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=92;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=93;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=94;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=95;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=96;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=97;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=2;
        ApnUsbReadReg(FpgaReg,&FpgaData);
	FpgaReg=3;
        ApnUsbReadReg(FpgaReg,&FpgaData);
        FpgaReg = 2;
        FpgaData = 0x2000;
        ApnUsbWriteReg(FpgaReg,FpgaData);
	FpgaReg=91;
        ApnUsbReadReg(FpgaReg,&FpgaData);

}

int ApnUsbReadReg( unsigned short FpgaReg, unsigned short *FpgaData )
{
    int Success;
    unsigned short RegData;

    Success = usb_control_msg((struct usb_dev_handle *)hDevice, 
                                USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, VND_APOGEE_CAMCON_REG,
                                            FpgaReg, FpgaReg, (char *)&RegData, 2, 50);
    *FpgaData = RegData;

    printf("DRIVER: usb read reg=%d data=%4.4x\n",FpgaReg,*FpgaData);
    return(Success);
}

int ApnUsbWriteReg( unsigned short FpgaReg, unsigned short FpgaData )
{
    char *cbuf;
    int Success;

    cbuf = (char *)&FpgaData;
    Success = usb_control_msg((struct usb_dev_handle *)hDevice, 
                                  USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,  VND_APOGEE_CAMCON_REG,
                                   0, FpgaReg, cbuf, 2, 50);
    printf("DRIVER: usb write reg=%d data=%4.4x\n",FpgaReg,FpgaData);


}



