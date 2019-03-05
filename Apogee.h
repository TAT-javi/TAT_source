#include "symblc_const.h"


enum Camera_Status{
	Camera_Status_Idle = 0,
	Camera_Status_Waiting,
	Camera_Status_Exposing,
	Camera_Status_Downloading,
	Camera_Status_LineReady,
	Camera_Status_ImageReady,
	Camera_Status_Flushing
};

enum Camera_CoolerStatus{
	Camera_CoolerStatus_Off = 0,
	Camera_CoolerStatus_RampingToSetPoint,
	Camera_CoolerStatus_Correcting,
	Camera_CoolerStatus_RampingToAmbient,
	Camera_CoolerStatus_AtAmbient,
	Camera_CoolerStatus_AtMax,
	Camera_CoolerStatus_AtMin,
	Camera_CoolerStatus_AtSetPoint
};

enum Camera_CoolerMode{
	Camera_CoolerMode_Off = 0,	// shutdown immediately
	Camera_CoolerMode_On,		// enable cooler, starts searching for set point
	Camera_CoolerMode_Shutdown	// ramp to ambient, then shutdown
};

	#if !defined(AFX_APOGEE__INCLUDED_)
	#define AFX_APOGEE__INCLUDED_


	#define	Apn_Interface int
	#define	Apn_Interface_NET 0
	#define	Apn_Interface_USB 1

	#define	Apn_NetworkMode int
	#define	Apn_NetworkMode_Tcp	0
	#define	Apn_NetworkMode_Udp	1

	#define	Apn_Resolution int
	#define	Apn_Resolution_SixteenBit 0
	#define	Apn_Resolution_TwelveBit 1

	#define	Apn_CameraMode int
	#define	Apn_CameraMode_Normal 0
	#define	Apn_CameraMode_TDI 1
	#define	Apn_CameraMode_Test 2
	#define	Apn_CameraMode_ExternalTrigger 3
	#define	Apn_CameraMode_ExternalShutter 4
	#define	Apn_CameraMode_Kinetics 5

	#define	Apn_Status int
	#define	Apn_Status_DataError -2
	#define	Apn_Status_PatternError	 -1
	#define	Apn_Status_Idle	 0
	#define	Apn_Status_Exposing  1
	#define	Apn_Status_ImagingActive  2
	#define	Apn_Status_ImageReady  3
	#define	Apn_Status_Flushing  4
	#define	Apn_Status_WaitingOnTrigger 5
	#define	Apn_Status_ConnectionError 6

	#define	Apn_LedMode int
	#define	Apn_LedMode_DisableAll 0
	#define	Apn_LedMode_DisableWhileExpose 1
	#define	Apn_LedMode_EnableAll 2

	#define	Apn_LedState int
	#define	Apn_LedState_Expose 0
	#define	Apn_LedState_ImageActive 1
	#define	Apn_LedState_Flushing 2
	#define	Apn_LedState_ExtTriggerWaiting 3
	#define	Apn_LedState_ExtTriggerReceived 4
	#define	Apn_LedState_ExtShutterInput 5
	#define	Apn_LedState_ExtStartReadout 6
	#define	Apn_LedState_AtTemp 7

	#define	Apn_CoolerStatus int
	#define	Apn_CoolerStatus_Off 0
	#define	Apn_CoolerStatus_RampingToSetPoint 1
	#define	Apn_CoolerStatus_AtSetPoint 2
	#define	Apn_CoolerStatus_Revision 3

	#define	Apn_FanMode int
	#define	Apn_FanMode_Off	0
	#define	Apn_FanMode_Low 1
	#define	Apn_FanMode_Medium 2
	#define	Apn_FanMode_High 3

	#define ApnUsbParity int
	#define ApnNetParity int
	#define Apn_SerialParity int
	#define Apn_SerialFlowControl bool
	#define ApnUsbParity_None 0
	#define ApnUsbParity_Odd 1
	#define ApnUsbParity_Even 2
	#define ApnNetParity_None 0
	#define ApnNetParity_Odd 1
	#define ApnNetParity_Even 2
	#define Apn_SerialFlowControl_Unknown 0
	#define Apn_SerialFlowControl_Off 0
	#define Apn_SerialFlowControl_On 1
	#define Apn_SerialParity_Unknown 0
	#define Apn_SerialParity_None 0
	#define Apn_SerialParity_Odd 1
	#define Apn_SerialParity_Even 2

	#define	Camera_Status int
	#define	Camera_Status_Idle 0
	#define	Camera_Status_Waiting 1
	#define	Camera_Status_Exposing 2
	#define	Camera_Status_Downloading 3
	#define	Camera_Status_LineReady 4
	#define	Camera_Status_ImageReady 5
	#define	Camera_Status_Flushing 6

	#define	Camera_CoolerStatus int
	#define	Camera_CoolerStatus_Off	 0
	#define	Camera_CoolerStatus_RampingToSetPoint 1
	#define	Camera_CoolerStatus_Correcting 2
	#define	Camera_CoolerStatus_RampingToAmbient 3
	#define	Camera_CoolerStatus_AtAmbient 4
	#define	Camera_CoolerStatus_AtMax 5
	#define	Camera_CoolerStatus_AtMin 6 
	#define	Camera_CoolerStatus_AtSetPoint 7

	#define	Camera_CoolerMode int
	#define	Camera_CoolerMode_Off 0
	#define	Camera_CoolerMode_On  1
	#define	Camera_CoolerMode_Shutdown 2

	#endif

#define CAM_IDLE_STR "Idle"
#define CAM_WAIT_STR "Waiting"
#define CAM_WAITON_STR "WaitingOnTrigger"
#define CAM_EXPOSING_STR "Exposing"
#define CAM_DOWN_STR "Downloading"
#define CAM_LINEREADY_STR "LineReady"
#define CAM_IMAGEREADY_STR "ImageReady"
#define CAM_IMAGINGACTIVE_STR "ImagingActive"
#define CAM_FLUSHING_STR "Flushing"
#define CAM_DATAERR_STR "DataError"
#define CAM_PATTERNERR_STR "PatternError"

#define COOL_OFF_STR "Off"
#define COOL_RAMPSETPOINT_STR "RampingToSetPoint"
#define COOL_CORRECT_STR "Correcting"
#define COOL_RAMPAMBIENT_STR "RampingToAmbient"
#define COOL_ATAMBIENT_STR "AtAmbient"
#define COOL_ATMAX_STR "AtMax"
#define COOL_ATMIN_STR "AtMin"
#define COOL_ATSETPOINT_STR "AtSetPoint"
#define COOL_REVISION_STR "Revision"

#define FAN_OFF_STR "Off"
#define FAN_LOW_STR "Low"
#define FAN_MEDIUM_STR "Medium"
#define FAN_HIGH_STR "High"

#ifdef U6
const char *Camera_status_string[] = {CAM_IDLE_STR,CAM_WAIT_STR,
				CAM_EXPOSING_STR,CAM_DOWN_STR,CAM_LINEREADY_STR,
				CAM_IMAGEREADY_STR,CAM_FLUSHING_STR};
const char *Cooler_status_string[] = {COOL_OFF_STR,COOL_RAMPSETPOINT_STR,
				COOL_CORRECT_STR,COOL_RAMPAMBIENT_STR,COOL_ATAMBIENT_STR,
				COOL_ATMAX_STR,COOL_ATMIN_STR,COOL_ATSETPOINT_STR};
#elif defined F6
const char *Camera_status_string[] = {CAM_DATAERR_STR,CAM_PATTERNERR_STR,
				CAM_IDLE_STR,CAM_EXPOSING_STR,CAM_IMAGINGACTIVE_STR,
				CAM_IMAGEREADY_STR,CAM_FLUSHING_STR,CAM_WAITON_STR};
const char *Cooler_status_string[] = {COOL_OFF_STR,COOL_RAMPSETPOINT_STR,
				COOL_ATSETPOINT_STR,COOL_REVISION_STR};
#endif
const char *Fan_status_string[] = {FAN_OFF_STR,FAN_LOW_STR,
				FAN_MEDIUM_STR,FAN_HIGH_STR};

