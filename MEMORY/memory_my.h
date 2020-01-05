#ifndef MEMORY_H
#define MEMORY_H

#include "sys.h"
/*
* @file  memory.h
* @brief Get the first address of the hourly rainfall data store from year, month , day and hour!
* @version 1.1
		将rainfall data length 改为1字节
* @author 
* @date 2019.8.22
*/
#pragma pack(1)

//BCD 格式数据
typedef struct RTU_operation_parameters_s
{
	unsigned char center_addr[4];          //************修改长度
	unsigned char rtu_remote_addr[5];  //站号
	unsigned char password[2];
	unsigned char center_1_main_channel[10];
	unsigned char center_1_spare_channel[10];
	unsigned char center_2_main_channel[10];
	unsigned char center_2_spare_channel[10];
	unsigned char center_3_main_channel[10];
	unsigned char center_3_spare_channel[10];
	unsigned char center_4_main_channel[10];
	unsigned char center_4_spare_channel[10];
	unsigned char work_style;
	unsigned char remote_collect_factor[4];
	unsigned char time_interval_of_timing_message; //定时报时间间隔
	unsigned char time_interval_of_plus_message;
	unsigned char rainfall_day_start_time;
	unsigned char sampling_interval[2];   //采样间隔
	unsigned char waterlevel_save_interval;  //水位数据存储间隔
	unsigned char rainfall_resolution;		//雨量计分辨力
	unsigned char waterlevel_resolution;   //水位计分辨力
	
	unsigned char threshold_rainfall_value_of_plusemessage;  //雨量加报阈值
	unsigned char waterlevel_basevalue_1[4];
	unsigned char waterlevel_correction_value_1[3];
	unsigned char waterlevel_plusmessage_1[2];  	//加报水位
	unsigned char above_threshold_waterlevel_value_of_plusmeessag[2]; //加报水位以上阈值
	unsigned char below_threshold_waterlevel_value_of_plusmessage[2]; //加报水位以下阈值
	unsigned char flow_threshold_plusmessage[3];
	
	
	
	
}RTU_operation_parameters_t;



#pragma pack()


#define IDR_RAINFALL_EVEN_BASE    0X000000   //偶数年雨量存储基地址 结束地址0X04 CF E0
#define IDR_WATERLEVLE_EVEN_BASE  0X400000
#define IDR_RAINFALL_ODD_BASE     0X800000
#define IDR_WATERLEVEL_ODD_BASE   0XC00000


#define IDR_SPECIAL_DATA_STORE_BASE 0X080000
#define RAINFALL_DAY_START_VALUE (IDR_SPECIAL_DATA_STORE_BASE + 0X00)   //可以直接删除替换
#define IDR_RTU_PARAMETERS	(IDR_SPECIAL_DATA_STORE_BASE + 0X10)

#define RAINFALL_SINGLE_DATALEN   3
#define WATERLEVEL_SINGLE_DATALEN 4
#define WATERLEVEL_TYPE 1
#define RAINFALL_TYPE   0



#pragma pack(1)
	
typedef struct date_s
{

	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
}date_t;

#pragma pack()

/*
********************************************************************************************
* 函数名：GetAddress
* 返回值：0：成功 ，-1：失败
* 参  数：date：时间数据，年月日时间直接是用十进制数表示的，
					data_type: 雨量、水位数据类型
					*address :得到的相应地址

* 描  述：可以通过任意时间，数据类型输入，然后得到FLASH中存储的相应地址
*
*********************************************************************************************
*/
int GetAddress(const date_t *date,const unsigned char data_type, int *address);

/*
********************************************************************************************
* 函数名：SaveDataToFlash
* 返回值：0——成功 ，-1——失败	
* 参  数：date：存储时间
					data_type ：目前RAINFALL_TYPE,WATERLEVEL_TYPE两种类型
					data：数据内容
					data_len:数据的长度
* 描  述：通过存储时间、存储类型来存储雨量水位数据到FLASH中，
*
*********************************************************************************************
*/
int SaveDataToFlash(const date_t *date,const unsigned char data_type,const unsigned char *data,const int data_len);

/*
********************************************************************************************
* 函数名：ReadDataFromFlash
* 返回值：0——成功 ，-1——失败	
* 参  数：date：想读FLASH中数据的存储起始时间
				  data_type ：目前RAINFALL_TYPE,WATERLEVEL_TYPE两种类型
					data：得到的数据内容
					data_len:数据的长度
* 描  述：通过存储时间、存储类型，来得到FLASH中的存储的数据
*
*********************************************************************************************
*/
int ReadDataFromFlash(const date_t *date, const unsigned char data_type, unsigned char *data, const int data_len);

void SaveAndGetData_FiveMinute(unsigned char *rainfall_data, unsigned char *water_level_data);


/*
********************************************************************************************
* 函数名：GetMonthDays
* 返回值：
* 参  数：year:BCD码年的一个字节数据，month_days：返回的一年每个月的天数
					
* 描  述：通过BCD码的年数据，得到该年的每一个月的天数
*********************************************************************************************
*/
void GetMonthDays(const unsigned char year, unsigned char *month_days);
void RTU_Operation_parameter_init(void);
void RTU_parameters_Writedata(RTU_operation_parameters_t *parameter);
void RTU_parameters_Readdata(RTU_operation_parameters_t *parameter);




extern RTU_operation_parameters_t g_rtu_parameters;




#endif
