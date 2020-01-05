#ifndef _PICTURE_H
#define _PICTURE_H


#include "sys.h"
#include "sd3078rtc.h"





RESULT_MY create_directory(unsigned char *all_path);
RESULT_MY delete_first_folder(const Time_Def *real_time);
extern unsigned int Figure_num;
#if 1

char * getcontent(u8 day,u8 hour);
char * getcontent_bynum(unsigned  int pic_num);
#endif





























#endif
