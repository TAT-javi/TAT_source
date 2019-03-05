#!/bin/sh
#Apogee image tester -  Usage:
#         -i imagename    Name of image (required)
#         -t time         Exposure time is seconds (required)
#         -s 0/1          1 = Shutter open, 0 = Shutter closed (required)
#         -a a.b.c.d      IP address of camera e.g. 192.168.0.1 (required for ALTA-E models only)
#         -F 0/1          Fast readout mode (ALTA-U models only)
#         -u num          Camera number (default 1 , ALTA-U only)
#         -x num          Binning factor in x, default 1
#         -y num          Binning factor in y, default 1
#         -r xs,ys,xe,ye  Image subregion in the format startx,starty,endx,endy
#         -b biascols     Number of Bias columns to subtract
#         -f mode         Fanmode during exposure, off,slow,medium,fast (default medium)
#         -c temp         Required temperature for exposure, default is current value
#         -n num          Number of exposures
#         -p time         Number of seconds to pause between multiple exposures
#         -v verbosity    Print more details about exposure

temp=-6
while [ "$temp" -ge -20 ]
do
output=~singway/`date +%H%M%S`.fit

output=~singway/`date +%H%M%S`_$temp.fit
echo $output
/opt/apogee/src/apogee/test_altau -i $output -t 0.03 -s 0 -r 1,1,1024,1024 -c $temp
~singway/cfitsio_app/imlist $output[1015:1024,960:969]
#/opt/apogee/src/apogee/test_altau -i $output -t 0.6 -s 0 -r 1,1,10,10 -F 0
#~singway/cfitsio_app/imlist $output[1:10,1:10]
#exit
sleep 3
temp=$(($temp-2))
echo $temp
done
output=~singway/`date +%H%M%S`_temp.fit
/opt/apogee/src/apogee/test_altau -i $output -t 0.03 -s 0 -r 1,1,1024,1024
