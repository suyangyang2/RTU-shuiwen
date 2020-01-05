#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include "sys.h"


#define BT_START_ID 0X7E7E
#define BT_SETTIME_UPDOWN_UP_ID 0X0008
#define BT_CONTENT_START_ID  0X02
#define BT_REMOTE_ADDR_LEN   5


#define TIME_GUIDE_ID          0XF0
#define CENTER_ADDR_GUIDE_ID   0XF1
#define REMOTE_ADDR_GUIDE_ID   0XF2
#define SAMPLING_INTERVAL_GUIDE_ID 0XF3
#define CENTER1_TYPE_GUIDE_ID  	   0XE4
#define CENTER2_TYPE_GUIDE_ID	   0XE5
#define PORT1_GUIDE_ID             0XD4
#define PORT2_GUIDE_ID			   0XD5
#define TCP_UDP_GUIDE_ID		   0XE6
#define SHORT_MESSAGE_GUIDE_ID	   0XE7
#define VOLATAGE_GUIDE_ID		   0X38
#define WATERLEVEL_GUIDE_ID		   0X39
#define TEMPERATURE_GUIDE_ID	   0X02


#define BT_QUIT_FUNID		 0X1B
#define BT_SETTIME_FUNID     0X4A
#define BT_QUERY_TIME_FUNID  0X50
#define BT_READ_RTU_FUNID	 0X41
#define BT_MODIFY_RTU_PARAMETERS_FUNID   0X42
#define BT_READ_4G_PARAMETERS_FUNID		 0X43
#define BT_MODIFY_4G_PARAMETERS_FUNID	 0X44
#define	BT_QUERY_REALDATA_FUNID		     0X3A
#define BT_MANUAL_PLACEMENT_FUNID   	 0X35
#define BT_WORKING_ALARM_INFORMATION_FUNID 0X46
#define BT_DATA_DOWNLOAD_FUNID			   0X38
#define BT_QUERY_SOFTWARE_VERSION_FUNID    0X45
#define BT_RESET_FUNID					   0X48
#define BT_SENDATA_MANUALLY_FUNID		 0X33

#pragma pack(1)

typedef struct time_data_s
{
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
}time_data_t;

typedef struct bluetooth_message_header_s
{
	unsigned short start_id;
	unsigned char fun_id;
	unsigned short updownid_contentlength;
	unsigned char content_start_id;
}bluetooth_message_header_t;

typedef struct set_time_conten_s
{
	unsigned char guide_id;
	unsigned char data_length;
	time_data_t time;
}set_time_conten_t;

typedef struct bluetooth_frame_s
{
	bluetooth_message_header_t header;
	unsigned char guide_id;
	unsigned char data_length;
}bluetooth_frame_t;


typedef struct set_time_frame_s
{
	bluetooth_message_header_t header;
	set_time_conten_t content;
	unsigned short end_id;
}set_time_frame_t;

typedef struct querry_time_content_s
{
	unsigned char guide_id;
	unsigned char data_length;
	time_data_t  time;

}querry_time_content_t;

typedef struct querry_time_upframe_s
{
	bluetooth_message_header_t header;
	querry_time_content_t content;
	unsigned short endid;
}querry_time_upframe_t;


typedef struct querry_realdata_content_s
{
	unsigned char waterlevel_guide_id;
	unsigned char waterlevel_data_length;
	unsigned char waterlevel_data[3];
	unsigned char temperature_guide_id;
	unsigned char temperature_data_length;
	unsigned char temperature_data[2];
	unsigned char voltage_guide_id;
	unsigned char voltage_data_length;
	unsigned char voltage_data[2];
}querry_realdata_content_t;

typedef struct querry_realdata_upfram_s
{
	bluetooth_message_header_t header;
	querry_realdata_content_t content;
	unsigned short endid;

}querry_realdata_upfram_t;








#pragma pack()





int Execute_commanad(const u8 *buff , const u32 length, const u8 fun_id);
int Response_upframe(const u8 *buff,const u32 length, const u8 fun_id);



/*
********************************************************************************************
* 函数名：Get_IP_Information
* 返回值：0——成功 ，-1——失败	
* 参  数： src_buff ：BCD编码格式的数据buff;
		length: 数据字节长度 
		des_buff: 解析出来的目标IP字符串；
		
* 描  述：将特定编码格式的IP信息解析出来,如{0x10, 0x60, 0x15, 0x23, 0x41, 0x81},解析出106.015.234.181;
*
*********************************************************************************************
*/
int Get_IP_Information(const unsigned char *src_buff, const unsigned int length, unsigned char *des_buff);

























#endif

