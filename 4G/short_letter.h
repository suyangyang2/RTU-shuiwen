#ifndef _SHORT_LETTER_H
#define _SHORT_LETTER_H

#include "sys.h"

void GetTeleCmd(char *tele_number, char *cmd);
int Send_short_letter(char *tele_number, char *buff, int length);
int GetGPS_Inforation(unsigned char *buff, int *length);

































#endif

