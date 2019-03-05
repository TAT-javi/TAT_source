/* File : apogeeNET.i */
%module apogee_net
%{     
#include "ApnCamera.h" 
%}
%typemap(tcl,in) Camera_Status = int;
%typemap(tcl,in) Camera_CoolerMode = int;
%typemap(tcl,in) Camera_CoolerStatus  =int;
%typemap(tcl,in) Camera_Interface = int;
%typemap(tcl,in) Camera_SensorType = int;
%typemap(tcl,in) Apn_Resolution  =int;
%typemap(tcl,in) Apn_Status  =int;
%typemap(tcl,in) Apn_Interface = int;
%typemap(tcl,in) Apn_CameraMode  =int;
%typemap(tcl,in) Apn_NetworkMode = int;
%typemap(tcl,in) Apn_FanMode  =int;
%typemap(tcl,in) Apn_LedState = int;
%typemap(tcl,in) Apn_LedMode  =int;
%typemap(tcl,in) Apn_CoolerStatus = int;
%typemap(tcl,out) Camera_Status = int;
%typemap(tcl,out) Camera_CoolerMode = int;
%typemap(tcl,out) Camera_CoolerStatus  =int;
%typemap(tcl,out) Camera_Interface = int;
%typemap(tcl,out) Camera_SensorType = int;
%typemap(tcl,out) Apn_Resolution  =int;
%typemap(tcl,out) Apn_Status  =int;
%typemap(tcl,out) Apn_Interface = int;
%typemap(tcl,out) Apn_CameraMode  =int;
%typemap(tcl,out) Apn_NetworkMode = int;
%typemap(tcl,out) Apn_FanMode  =int;
%typemap(tcl,out) Apn_LedState = int;
%typemap(tcl,out) Apn_LedMode  =int;
%typemap(tcl,out) Apn_CoolerStatus = int;

%include "ApnCamera.i"








