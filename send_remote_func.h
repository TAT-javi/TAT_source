#define PORT_HQ 5197

void remote_backup(void);
int send_message_to_HQ(char *message);


/** MESSAGES BETWEEN PC **/
//COMMANDS
#define CHECK_WEATHER_CMD "CHECK_WEATHER"
#define CHECK_OBS_CMD "CHECK_OBS"
#define STOP_OBS_CMD "STOP_OBS"
#define ADD_OBS_LINE_CMD "ADD_LINE"

//REPLY
#define ERROR_READ_RPLY "ERROR_READ"
#define ERROR_SITE_RPLY "ERROR_SITE"

#define WEATHER_OK_RPLY "WEATHER_OK"
#define WEATHER_BAD_RPLY "WEATHER_BAD"
#define WEATHER_ERROR_RPLY "WEATHER_ERROR"

#define IS_OBS_RPLY "IS_OBS"
#define IDLE_RPLY "IDLE"
#define ERROR_OBS_RPLY "OBS_ERROR"
#define OBS_STOPPED_RPLY "OBS_STOP_OK"

#define LINE_ADD_OK_RPLY "OBS_LINE_ADDED"
#define LINE_ADD_ERROR_RPLY "OBS_LINE_ERROR"
#define LINE_NO_FILE_RPLY "OBS_NO_FILE"

//notify
#define ERROR_DSP_NOTE "ERROR_DSP"
#define ERROR_HS_NOTE "ERROR_HS"
#define ERROR_WEATHER_NOTE "ERROR_WEATHER"
#define ERROR_ENC_NOTE "ERROR_ENC"
#define ERROR_FOV_NOTE "ERROR_FOV"
#define ERROR_CCD_NOTE "ERROR_CCD"
#define UPDATE_DB_CMD "UPDATE_DB"
#define EXP_TIME_CHANGED_NOTE "EXP_TIME_CHANGE"

#define MSG_OK "1"
#define MSG_WRONG "0"

//FILE 
#define TAT_MESSAGE_FILE "/home/tat/tat_message.msg"