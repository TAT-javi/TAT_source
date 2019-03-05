#include <stdio.h>
#include "symblc_const.h"
#include "main_cmd_menu.h"

int mvsend_cmd2fli(char *daemonCmd);
int send_cmd2fli(char *daemonCmd);

void home_focuser();
void get_focuser_current_pos();
void move_focuser_to_pos(long position);
void refresh_FLI_values(void);
void move_to_filter(int position);

