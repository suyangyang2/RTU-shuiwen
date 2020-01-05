#include "memory_my.h"
#include "protocol.h"
#include "sensor_water.h"
#include "sd3078rtc.h"
#include "w25qxx.h"
#include "usart.h"
#include "malloc.h"
#include <string.h>

static unsigned short s_monthday[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
static void Isleapyear(unsigned short year);
RTU_operation_parameters_t g_rtu_parameters;


/*
********************************************************************************************
* 函数名：GetBaseAddr
* 返回值：返回基地址，如IDR_RAINFALL_EVEN_BASE，IDR_WATERLEVLE_EVEN_BASE，IDR_RAINFALL_ODD_BASE，IDR_WATERLEVEL_ODD_BASE
* 参  数：date：起始时间，所用数据仅年,
					data_type: 数据的类型，如雨量、水位
* 描  述：从FLASH中根据时间、数据类型来返回基地址
*
*********************************************************************************************
*/

static int GetBaseAddr(const date_t *date,const u8 data_type);

static void Isleapyear(unsigned short year)
{
	if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
	{
		s_monthday[1] = 29;
	}
	else
	{
		s_monthday[1] = 28;
	}
}

static int GetBaseAddr(const date_t *date,const u8 data_type)
{
	int temp_address = 0;

	switch (date->year % 2)
	{
	case 0:
		(!data_type) ? (temp_address = IDR_RAINFALL_EVEN_BASE) : (temp_address = IDR_WATERLEVLE_EVEN_BASE);
		break;
	case 1:
		(!data_type) ? (temp_address = IDR_RAINFALL_ODD_BASE) : (temp_address = IDR_WATERLEVEL_ODD_BASE);
		break;
	default:
		break;
	}
	return temp_address;
}

void GetMonthDays(const unsigned char year, unsigned char *month_days)
{
	int i = 0;
	unsigned short years_short = 0;

	years_short = 2000 + (year >> 4) *10 + (year & 0x0f);
	Isleapyear(years_short);
	for(i = 0; i < 12; i++)
	{
		month_days[i] = s_monthday[i];
	}
}

int GetAddress(const date_t *date,const unsigned char data_type, int *address)
{
	const int minute_interval = 5; 
	const int hour_to_minutes = 60;
	int total_days = 0;
	int total_hours = 0;
	int total_groups = 0;
	u8 single_group_length = 0;
	int i = 0;
	int temp_address = 0;
	int temp_base_address = 0;
	u8 month = date->month;
	u8 day = date->day;
	
	(!data_type) ? (single_group_length = RAINFALL_SINGLE_DATALEN) : (single_group_length = WATERLEVEL_SINGLE_DATALEN);  //雨量3字节，水位4字节
	Isleapyear(date->year + 2000);  	//重新初始化确定2月天数
	if(month > 1)
	{
		for(i = 0; i < month-1; i++)
		{
			total_days += s_monthday[i];
		}	
	}
	total_days += (day-1);
	//printf("total_days is %d\r\n",total_days);
	total_hours = total_days*24 + date->hour;
	if(date->minute % minute_interval == 0)
	{
		total_groups = total_hours*(hour_to_minutes/minute_interval) + (date->minute/minute_interval);  //得到以5分钟为间隔的次数
	}
	else
	{
		printf("minutes is not the multiple of minute_inteval\r\n");
		return -1;
	}
	//printf("total_group is %d\r\n",total_groups);
	temp_base_address = GetBaseAddr(date, data_type);
	temp_address = temp_base_address + single_group_length * total_groups;
	//printf("temp_address is %06x\r\n", temp_address);
	*address = temp_address;
	return 0;
}

int SaveDataToFlash(const date_t *date,const unsigned char data_type,const unsigned char *data,const int data_len)
{
	int address = 0;

	if (GetAddress(date, data_type, &address) == 0)
	{
		if(!data_type)
		{
			W25QXX_Write((u8 *)data, address, data_len*RAINFALL_SINGLE_DATALEN); 
			//printf("rainfall save write data address is %06x\r\n",address);
		}
		else
		{
			W25QXX_Write((u8 *)data, address, data_len*WATERLEVEL_SINGLE_DATALEN); 
			//printf("waterlevel save  write data address is %06x\r\n",address);
		}
	}
	else
	{
		printf("get address failed\r\n");
		return -1;
	}
	return 0;
}

int ReadDataFromFlash(const date_t *date, const unsigned char data_type, unsigned char *data, const int data_len)
{
	int address = 0;

	if (GetAddress(date, data_type, &address) == 0)
	{
		//printf("data_type is %d, adddress is %06x\r\n",data_type, address);
		W25QXX_Read(data, address, data_len);
	}
	else
	{
		printf("get address failed,time is not the multiple of interval\r\n");
		return -1;
		
	}
	return 0;

}


void SaveAndGetData_FiveMinute(unsigned char *rainfall_data, unsigned char *water_level_data)  //雨量3字节，水位4字节,都是存储累积值
{
	Time_Def real_time;
	date_t date;
	float rainfall_float_data = 0;
	float waterlevel_float_data = 0;
	int rainfall_save_data = 0;
	int waterlevel_save_data = 0;
	unsigned char *rainfall_data_base = rainfall_data;       //用于存储数据首地址
	unsigned char *water_level_data_base = water_level_data;
	RTU_operation_parameters_t rtu_parameters = {0};
	float waterlevel_basic_value = 0;
	
	RTC_ReadDate(&real_time);
	TimeDefTodate(&real_time, &date);  //得到存储的时间
	RTU_parameters_Readdata(&rtu_parameters);
	
	waterlevel_basic_value = BCDSendataToFloat(0x23, rtu_parameters.waterlevel_basevalue_1);
	
	rainfall_float_data = g_rainfall_pluse_count * RAINFALL_STEP;
	rainfall_save_data = GetBCDSendata(CUMULA_PRECIPIT_ID, rainfall_float_data);
	//printf("rainfall save data is %06x\r\n", rainfall_save_data);
	*rainfall_data++ = (unsigned char)(rainfall_save_data >> 16);
	*rainfall_data++ = (unsigned char)(rainfall_save_data >> 8);
	*rainfall_data = (unsigned char)(rainfall_save_data >> 0);
	SaveDataToFlash(&date, RAINFALL_TYPE, rainfall_data_base, 1);
	
	waterlevel_float_data = (float)GetWaterLevelData() / 100.0;
	waterlevel_float_data += waterlevel_basic_value;
	waterlevel_save_data = GetBCDSendata(WATER_LEVEL_ID, waterlevel_float_data);
	*water_level_data++ = (unsigned char)(waterlevel_save_data >> 24);
	*water_level_data++ = (unsigned char)(waterlevel_save_data >> 16);
	*water_level_data++ = (unsigned	char)(waterlevel_save_data >> 8);
	*water_level_data = (unsigned char)(waterlevel_save_data >> 0);
	SaveDataToFlash(&date, (unsigned char)WATER_LEVEL_ID, water_level_data_base, 1);
	sw_log("SaveAndGetData_FiveMinute", SW_LOG_LEVEL_DEBUG, "save rainfall and waterlevel data is %06x,%08x\r\n", rainfall_save_data, waterlevel_save_data);
	//printf("rainfall save data is %06x,  waterlevel save data is %08x\r\n",rainfall_save_data, waterlevel_save_data);
}



//目前函数中部分参数现在是无意义值,目前只针对与于水位基值1，进行存读，改变参数值实验
//开机复位默认参数
void RTU_Operation_parameter_init(void)
{
	unsigned char center_addr[4] = {0x00,0x00,0x00,0x00};
	
	g_rtu_parameters.time_interval_of_timing_message = 0x01;
	g_rtu_parameters.time_interval_of_plus_message = 0x05;
	g_rtu_parameters.sampling_interval[0] = 0x03;
	g_rtu_parameters.sampling_interval[1] = 0x00;
	g_rtu_parameters.waterlevel_save_interval = 0x05;
	g_rtu_parameters.rainfall_resolution = 0x01;   //雨量计分辨率
	g_rtu_parameters.waterlevel_save_interval = 0x01;	//水位计分辨率
	g_rtu_parameters.threshold_rainfall_value_of_plusemessage = 0x00;
	g_rtu_parameters.waterlevel_basevalue_1[0] = 0x01;    //水位基准值
	g_rtu_parameters.waterlevel_basevalue_1[1] = 0x00;
	g_rtu_parameters.waterlevel_basevalue_1[2] = 0x00;
	g_rtu_parameters.waterlevel_basevalue_1[3] = 0x00;
	g_rtu_parameters.waterlevel_correction_value_1[0] = 0x00;
	g_rtu_parameters.waterlevel_correction_value_1[1] = 0x00;
	g_rtu_parameters.waterlevel_correction_value_1[2] = 0x00;
	g_rtu_parameters.waterlevel_plusmessage_1[0] = 0x00;
	g_rtu_parameters.waterlevel_plusmessage_1[1] = 0x00;
	g_rtu_parameters.above_threshold_waterlevel_value_of_plusmeessag[0] = 0x00;
	g_rtu_parameters.above_threshold_waterlevel_value_of_plusmeessag[1] = 0x00;
	g_rtu_parameters.below_threshold_waterlevel_value_of_plusmessage[0] = 0x00;
	g_rtu_parameters.below_threshold_waterlevel_value_of_plusmessage[1] = 0x00;
	
	g_rtu_parameters.rtu_remote_addr[0] = 0x00;
	g_rtu_parameters.rtu_remote_addr[1] = 0x55;
	g_rtu_parameters.rtu_remote_addr[2] = 0x00;
	g_rtu_parameters.rtu_remote_addr[3] = 0x00;
	g_rtu_parameters.rtu_remote_addr[4] = 0x01;
 
	memcpy(g_rtu_parameters.center_addr,center_addr,sizeof(center_addr));
	W25QXX_Write((u8 *)&g_rtu_parameters, IDR_RTU_PARAMETERS, sizeof(g_rtu_parameters));
}

void RTU_parameters_Writedata(RTU_operation_parameters_t *parameter)
{
	W25QXX_Write((u8 *)parameter, IDR_RTU_PARAMETERS, sizeof(*parameter));	
}

void RTU_parameters_Readdata(RTU_operation_parameters_t *parameter)
{
	W25QXX_Read((u8 *)parameter, IDR_RTU_PARAMETERS, sizeof(*parameter));
}
