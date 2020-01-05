#include "picture.h"
#include "string.h"
#include "usart.h"
#include "sd3078rtc.h"
#include "ff.h"


 /*将单个字节的BCD码转化为字符串*/
static void BCDToString(unsigned char bcd_data, unsigned char *des_string)
{
	unsigned char temp = 0;
	unsigned char i = 0;
	unsigned char string_length = 2;

	for (i = 0; i < string_length; i++)
	{
		temp = (bcd_data >> (4 * (string_length - 1 - i))) & 0x0f;
		des_string[i] = temp + '0';
	}
}

/*通过时间生成一个路径目录,并返回完整的路径名，能删除超过2个月间隔的月份的文件夹,注意all_path,指针指向的内存需要足够大*/
//图片名字加上秒，这样防止两张图片重名
RESULT_MY create_directory(unsigned char *all_path)
{
	unsigned char directory_path[30] = { 0 };
	unsigned char picture_name[30] = { 0 };
	unsigned char path[20] = { 0 };
	unsigned int i = 0;
	static unsigned char execute_flag = 0;
	unsigned char temp[6][5] = { 0 };	 //由于有4级文件夹目录 ，19-1-1-1
	FRESULT res;
	
	Time_Def real_time;
	RTC_ReadDate(&real_time);
	if((real_time.day == 0x01) && (execute_flag == 0)) //在1号删除文件夹
	{
		delete_first_folder((const Time_Def *)&real_time);
		execute_flag = 1;
	}
	else
	{
		if(real_time.day == 0x02)
		{
			execute_flag = 0;
		}
	}
	BCDToString(real_time.year, temp[0]);  //根目录
	BCDToString(real_time.month, temp[1]);  //二级,下同
	BCDToString(real_time.day, temp[2]);
	BCDToString(real_time.hour, temp[3]);
	BCDToString(real_time.minute, temp[4]);
	BCDToString(real_time.second, temp[5]);
	sprintf((char *)directory_path, "%s/%s/%s/%s", temp[0], temp[1], temp[2], temp[3]); //获得完整的路径名
	for (i = 0; i < 4; i++)
	{
		memcpy(path, directory_path, 3 * i + 2);  //在这里依次创建文件夹
		res = f_mkdir((const TCHAR *)path);
		if(!(res == FR_OK || res == FR_EXIST))
		{
			printf("mkdir failed\r\n");
			return ERRO;
		}
	}
	sprintf((char *)picture_name, "%s%s%s-%s%s%s.jpg", temp[0], temp[1], temp[2], temp[3], temp[4],temp[5]);  //获得图片名字
	//strcpy((char *)all_path, (const char *)directory_path);
	//strcpy((char *)pic_name, (const char *)picture_name);
	sprintf((char *)all_path,"0:%s/%s",directory_path, picture_name);
	
	return OK;
}

/*每隔2个月，删除最前面的一个月的文件夹中所有图片*/
RESULT_MY delete_first_folder(const Time_Def *real_time)
{
	unsigned char path[20] = { 0 };
	unsigned char date = 0;

	switch (real_time->month)
	{
	case 0x01:
		date = ((real_time->year) & 0x0f) + (((real_time->year) >> 4) & 0x0f) * 10;
		date -= 1;
		sprintf((char *)path, "0:%02d/11", date);
		break;
	case 0x02:
		date = ((real_time->year) >> 4) * 10 + ((real_time->year) & 0x0f);
		date -= 1;
		sprintf((char *)path, "0:%02d/12", date);
		printf("path2 is %s\r\n", path);
		break;
	default:
		date = ((real_time->month) >> 4) * 10 + ((real_time->month) & 0x0f);
		printf("date is %d\r\n", date);
		date -= 2;
		sprintf((char *)path, "0:%02x/%02d", real_time->year, date);
		printf("path3 is %s\r\n", path);
		break;
	}
	if(f_deldir((const TCHAR *) path) != FR_OK)
	{
		printf("delete folder failed\r\n");
		return (RESULT_MY)ERRO;
	}
	return (RESULT_MY)OK;
}


unsigned int Figure_num=1;

#if 1

char path_my[20];    //代表路径
//////获取文件路径
char * getcontent(u8 day,u8 hour)
{
	char a[8];

	switch (day)
	{
	case 1: strcpy(a, "Mon"); break;
	case 2: strcpy(a, "Tues"); break;
	case 3: strcpy(a, "Wed"); break;
	case 4:	strcpy(a, "Thurs"); break;
	case 5: strcpy(a, "Fri"); break;
	case 6: strcpy(a, "Sat"); break;
	case 7: strcpy(a, "Sun"); break;
	default:
		break;
	}
	sprintf(path_my, "0:PICTURE/%s/%d", a, hour);
	return path_my;
}


char * getcontent_bynum(unsigned  int pic_num)
{
	
	u8 day = 0;
	u8 hour = 0;
	printf("pic_num is %d\r\n", pic_num);
	if (pic_num <= 0 || pic_num > 2016)
	{
		printf("图片数量超过一周范围\r\n");
	}
	day = pic_num / 288+1;
	hour = ((pic_num % 288)-1 )/ 12;      ////
	return getcontent(day, hour);
}

#endif
