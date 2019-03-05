#!/bin/bash

#echo " Script to configure the TAT source code"
echo " "
set -e # Exit on error

if [ $# -ne 1 ]; then
	echo " ########################### USAGE ############################"
	echo " "$0" [Location] "
	echo " "
	echo "  where [Location] can be:"
	echo "     TF = Tenerife"
	echo "     KU = Li-Jiang"
	echo "     HS = Hsinchu"
	echo " ##############################################################"
	exit 1
elif [ "TF" == $1 ]; then # TENERIFE
	SITE=Tenerife
	SITE_UPPER=TENERIFE
	CCD=ccddaemonf6
elif [ "KU" == $1 ]; then # LI-JIANG
	SITE=Li-Jiang
	SITE_UPPER=KUNMING
	CCD=ccddaemonu6
elif [ "HS" == $1 ]; then # HSINCHU
	SITE=Hsinchu
	SITE_UPPER=HSINCHU
	CCD=ccddaemonf6
else
	echo " ERROR: Site $1 not found."
	echo " "
	exit 1
fi

if [ ! -f Makefile ]; then
    echo "ERROR: Makefile not found!"
    exit 1
fi

if [ ! -f ref_parameter_$1.dat ];then
	echo "ERROR: ref_parameter_$1.dat not found!"
	echo " "
	exit 1
fi

if [ ! -f tat_parameter_$1.dat ];then
	echo "ERROR: tat_parameter_$1.dat not found!"
	echo " "
	exit 1
fi

if [ ! -f ./libfli/libfli.a ];then
	echo "ERROR: libfli.a not found!"
	echo "Compile the FLI library in the libfli directory"
	echo " "
	exit 1
fi

if [ ! -f /opt/apogee/install ];then
	echo "WARNING: Apogee driver not found!"
	echo "For using the CCD software, compile the"
	echo "apogee apps in the /opt/ directory"
	echo " "
fi

if [ ! -f /opt/astrometry/bin/solve-field ];then
	echo "WARNING: Astrometry.net software not found!"
	echo "For using the astrometry.net software, compile"
	echo " the astrometry programs in the /opt/ directory"
	echo " "
fi

# Copy the files
cp ref_parameter_$1.dat ref_parameter.dat
cp tat_parameter_$1.dat tat_parameter.dat

#Modify Makefile
#1st get old SITE and CCD lines
SITELINE="$(grep 'SITE =' Makefile)"
CCDLINE="$(grep 'CCD =' Makefile)"
#2nd define the new SITE and CCD lines
NEWSITELINE="SITE = $1"
NEWCCDLINE="CCD = $CCD"
#3rd replace the old SITE and CCD lines
sed -ie "s/$SITELINE/$NEWSITELINE/" Makefile
sed -ie "s/$CCDLINE/$NEWCCDLINE/" Makefile

#Modify symbl_const
LINE="$(grep 'for configure.sh' symblc_const.h)"
NEWSITELINE="define $SITE_UPPER"

LINE=$(echo $LINE | tr "#" "\n")
SITELINE=" "
i=0
for x in $LINE; do
	SITELINE=$(echo $SITELINE $x)
	i=$((i+1))
	if [[ $i == 2 ]]; then
		break
	fi
done

sed -ie "s/$SITELINE/$NEWSITELINE/" symblc_const.h


##### END OF PROGRAM ##########
echo " TAT source is configured for '$SITE'."
echo " Type 'make' and then 'make install'."
echo " "
