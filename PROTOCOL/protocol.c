/*
* @file  protocol.c
* @brief 

* @version 1.0   2019.7.1       小时报，加报报文数据初始化
	@version 1.1   2019.8.29     串口接受指定通信协议数据后，反馈数据报文
	@version 1.3   2019.8.30     查询实时数据报文，目前数据为常量
	@version 1.4   2019.8.31
	@version 2.1   2019.9.11	时间段数据初始化,
* @author 
* @date 2019.7.1
*/
#include "sensor_water.h"
#include "protocol.h"
#include "dmausart.h"
#include "usart.h"
#include "w25qxx.h"
#include "4g.h"
//#include "usart.h"
#include "sd3078rtc.h"
#include "memory_my.h"
#include "rs232.h"
#include "rs485.h"
#include "wkup.h"
#include "malloc.h"
#include "delay.h"
#include <string.h>


#define		TRUE            1
#define		FALSE           0
#define HOUR_MESSAGE_DEBUG 0

/*
********************************************************************************************
* 函数名：
* 返回值：
* 参  数：
* 描  述：
*
*********************************************************************************************
*/

/** 
	ETX-没有下文   ETB-后续后报文
	
**/
plus_message_t g_plus_message;
hour_message_t g_hour_message;
queryrealdata_upmessage_t g_queryrealdata_upmessage = {0};
querrytimeperiod_upmessage_t g_querrytimeperiod_upmessage = {0};
unsigned char g_plus_message_flag = 0;
unsigned short sn = 0;
extern u8 DMA_usart2_Buf[DMA_Rec_Len];
extern u8 g_usart2_receive_stat;



/*
********************************************************************************************
* 函数名：
* 返回值：
* 参  数：
* 描  述：
*
*********************************************************************************************
*/


static int GetByteLength(const timeperiod_t *start_time, const timeperiod_t *end_time, const unsigned char data_type, int *address_start, int *address_end)
{
	int byte_length = 0;
	date_t date_start;
	date_t date_end;

	date_start.year = BCDToInteger(start_time->year);
	date_start.month = BCDToInteger(start_time->month);
	date_start.day = BCDToInteger(start_time->day);
	date_start.hour = BCDToInteger(start_time->hour);

	date_end.year = BCDToInteger(end_time->year);
	date_end.month = BCDToInteger(end_time->month);
	date_end.day = BCDToInteger(end_time->day);
	date_end.hour = BCDToInteger(end_time->hour);
	date_start.minute = 0;
	date_end.minute = 0;

	if(GetAddress(&date_start, data_type, address_start) != 0)
	{
		printf("get start_address failed\r\n");
		return -1;
	}
	if(GetAddress(&date_end, data_type, address_end) != 0)
	{
		printf("get end_address failed\r\n");
		return -1;
	}
	byte_length = *address_end - *address_start;
	return byte_length;
}

static unsigned char BCDToInteger(unsigned char src)
{
	unsigned char temp;
	unsigned char des;

	temp = (src >> 4);
	des = temp*10 + (src&0x0f);
	return des;
}

signed char JudegeBCD(unsigned char *src, int len)
{
	unsigned char temp;

	while (len--)
	{
		temp = *src++;
		if ((temp >> 4) >= 10 || (temp & 0x0f) >= 10)
		{
			return -1;
		}
	}
	return 0;
}

/*从时间区间内读取5分钟为间隔的所有数据并返回字节个数*/
int ReadData_ByTimePeriod(const timeperiod_t *start_time,const timeperiod_t *end_time, const unsigned char data_type, unsigned char *data)
{
	int addr_start = 0;
	int addr_end = 0;
	int byte_length = 0;
	
	byte_length = GetByteLength(start_time, end_time, data_type, &addr_start, &addr_end);
	W25QXX_Read(data, addr_start, byte_length);
	return byte_length;
}

void SaveDayStartRainfall(void)
{
	unsigned int day_start_rainfall;
	#if 0
	g_rainfall_pluse_count = 100;
	#endif
	day_start_rainfall = g_rainfall_pluse_count;
	W25QXX_Write((unsigned char *)&day_start_rainfall, RAINFALL_DAY_START_VALUE, sizeof(day_start_rainfall));
}

unsigned int ReadDayStartRainfall(void)
{
	unsigned int day_start_rainfall;
	W25QXX_Read((unsigned char *)&day_start_rainfall, RAINFALL_DAY_START_VALUE, sizeof(day_start_rainfall) );
	return day_start_rainfall;
}

//将一个特定格式BCD存储数据按照序号i,转换为float类型数据
static float RainFallBCDString_To_FloatValue(const unsigned char *src_buff, const int i)
{
	unsigned int des_data = 0;
	unsigned char des_str[3] = { 0 };

	memcpy(des_str, src_buff + 3 * i, 3);      //也可以通过BCDSendataToFloat
	if(JudegeBCD(des_str, 3) != 0)
	{
		//printf("%d data is not BCD \r\n", i);
		return 0;
	}
	des_data = ((des_str[2] >> 0) & 0x0f);
	des_data += ((des_str[2] >> 4) & 0x0f)*10;
	des_data += ((des_str[1] >> 0) & 0x0f)*100;
	des_data += ((des_str[1] >> 4) & 0x0f)*1000;
	des_data += ((des_str[0] >> 0) & 0x0f)*10000;
	des_data += ((des_str[0] >> 4) & 0x0f)*100000;
	//printf("des_data is %d\r\n", des_data);
	return (float)des_data / 10.0;
}

//从5分钟开始差值 *src_buff 长度应该13*3字节,地址为， *des_buff应该12个字节  
//针对于获得F460类型的数据
static void HourRainFallSaveData_To_FiveMinuteRelativeValue(const unsigned char *src_buff, unsigned char *des_buff)
{
	float last_value = 0;
	float curr_value = 0;
	int i = 0;

	for (i = 0; i < 12; i++)
	{
		last_value = RainFallBCDString_To_FloatValue(src_buff, i);
		curr_value = RainFallBCDString_To_FloatValue(src_buff, i+1);
		if(curr_value >= last_value )
		{
			des_buff[i] = (unsigned char)((curr_value - last_value)*10);
		}
		else
		{
			des_buff[i] = 0xff; //代表数据量在减小
		}
	
	}
}

//从5分钟差值 *src_buff长度为13*4个字节，
static float WaterLevelBCDString_To_FloatValue(const unsigned char *src_buff, const int i)
{
	unsigned char des_str[4] = { 0 };
	unsigned int des_data = 0;
	//unsigned char j = 0;
	float ans = 0;

	memcpy(des_str, src_buff + i * 4, 4); 
	if(JudegeBCD(des_str, 3) != 0)
	{
		//printf("%d data is not BCD \r\n", i);
		return 0;
	}      
	des_data = ((des_str[3] >> 0) & 0x0f);
	des_data += ((des_str[3] >> 4) & 0x0f) * 10;
	des_data += ((des_str[2] >> 0) & 0x0f) * 100;
	des_data += ((des_str[2] >> 4) & 0x0f) * 1000;
	des_data += ((des_str[1] >> 0) & 0x0f) * 10000;
	des_data += ((des_str[1] >> 4) & 0x0f) * 100000;
	des_data += ((des_str[0] >> 0) & 0x0f) * 1000000;
	des_data += ((des_str[0] >> 4) & 0x0f) * 10000000;
	//printf("waterlevelbcd to int des_data is %d\r\n", des_data);
	ans = (float)des_data / 1000.0;
	return ans;
}

static void HourWaterLevelSaveData_To_Value(const unsigned char *src_buff, unsigned short *des_buff)
{
	int i = 0;
	float value = 0;
	float relative_value = 0;
	unsigned short des_value = 0;
	RTU_operation_parameters_t rtu_parameters = {0};
	float waterlevel_basic_value = 0;
	
	RTU_parameters_Readdata(&rtu_parameters);
	waterlevel_basic_value = BCDSendataToFloat(0x23, rtu_parameters.waterlevel_basevalue_1);

	for(i = 0; i < 12; i++)
	{
		value = WaterLevelBCDString_To_FloatValue(src_buff, i);
		relative_value = value -  waterlevel_basic_value;
		des_value = (unsigned short)(relative_value * 100);
		des_buff[i] = (des_value << 8) | (des_value >> 8);
	}





	#if 0
	int i = 0;
	float last_value = 0;
	float curr_value = 0;

	for (i = 0; i < 12; i++)
	{
		last_value = WaterLevelBCDString_To_FloatValue(src_buff, i);
		curr_value = WaterLevelBCDString_To_FloatValue(src_buff, i+1);
		if(curr_value >= last_value)
		{
			des_buff[i] = (unsigned short)((curr_value - last_value)*100);
		}
		else
		{
			des_buff[i] = 0xffff; //代表数据量在减小
		}
#if 0
		des_buff[i] = ((des_buff[i] >> 8) | (des_buff[i] << 8));
#endif
		//printf("last_value is %f, curr_value is %f, des_buff is %04x\r\n", last_value, curr_value,des_buff[i]);
	}

	#endif
}

//查询实时数据
void QueryRealData_Init(void)
{
	Time_Def systime;
	u16 data_len;
	unsigned short crc;
	unsigned int current_rainfall = 0;
	float current_rainfall_float = 0;
	unsigned int  current_rainfall_store = 0;
	float cumulate_rainfall_float = 0;
	unsigned int cumulate_rainfall_store = 0;
	float water_level_float = 0;
	unsigned int  water_level_store = 0;
	unsigned int voltage_store = 0x1200;
	RTU_operation_parameters_t rtu_parameters;
	static int last_rainfall_pluse_count = 0;

	RTU_parameters_Readdata(&rtu_parameters);
	RTC_ReadDate(&systime);
	g_queryrealdata_upmessage.message_header.start_id = NETWORK_S0H;
	g_queryrealdata_upmessage.message_header.center_addr = rtu_parameters.center_addr[0];
	memcpy(g_queryrealdata_upmessage.message_header.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	g_queryrealdata_upmessage.message_header.password = (((unsigned short)rtu_parameters.password[0] << 8) & 0xff00)| \
		((unsigned short)rtu_parameters.password[1] &0x00ff);
	g_queryrealdata_upmessage.message_header.fun_id = QUERY_REALDATA_FUNID;
	data_len = (unsigned char *)&g_queryrealdata_upmessage.content_end_id - (unsigned char *)&g_queryrealdata_upmessage.queryrealdata_content.sn;
	g_queryrealdata_upmessage.message_header.frame_id_len = ((data_len & 0X00FF) << 8)|((data_len & 0XFF00) >> 8);
	g_queryrealdata_upmessage.message_header.content_start_id = NETWORK_ETX;
	g_queryrealdata_upmessage.queryrealdata_content.sn =  ((sn & 0X00FF) << 8)|((sn & 0XFF00) >> 8); 
	g_queryrealdata_upmessage.queryrealdata_content.pkt_time_data.year = systime.year;
	g_queryrealdata_upmessage.queryrealdata_content.pkt_time_data.month = systime.month;
	g_queryrealdata_upmessage.queryrealdata_content.pkt_time_data.day = systime.day;
	g_queryrealdata_upmessage.queryrealdata_content.pkt_time_data.hour = systime.hour;
	g_queryrealdata_upmessage.queryrealdata_content.pkt_time_data.minute = systime.minute;
	g_queryrealdata_upmessage.queryrealdata_content.pkt_time_data.second = systime.second;

	g_queryrealdata_upmessage.queryrealdata_content.remote_addr_id = 0xF1F1;
	memcpy(g_queryrealdata_upmessage.queryrealdata_content.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	g_queryrealdata_upmessage.queryrealdata_content.class_id = 0x48;
	g_queryrealdata_upmessage.queryrealdata_content.observ_time.id = 0xF0F0;
	g_queryrealdata_upmessage.queryrealdata_content.observ_time.year = systime.year;
	g_queryrealdata_upmessage.queryrealdata_content.observ_time.month = systime.month;
	g_queryrealdata_upmessage.queryrealdata_content.observ_time.day = systime.day;
	g_queryrealdata_upmessage.queryrealdata_content.observ_time.hour = systime.hour;
	g_queryrealdata_upmessage.queryrealdata_content.observ_time.minute = systime.minute;

	//当前降水量 ，这个不确定
	current_rainfall = g_rainfall_pluse_count - last_rainfall_pluse_count;
	current_rainfall_float = RAINFALL_STEP * current_rainfall;
	current_rainfall_store = GetBCDSendata(FIVE_MIN_PRECIPIT_ID, current_rainfall_float);
	//printf("current_rainfall_store is %06x\r\n", current_rainfall_store);

	//累积降水量
	cumulate_rainfall_float = g_rainfall_pluse_count * RAINFALL_STEP;
	cumulate_rainfall_store =  GetBCDSendata(CUMULA_PRECIPIT_ID, cumulate_rainfall_float);

	//瞬时水位值
	water_level_float = (float)GetWaterLevelData() / 100.0;
	water_level_store =  GetBCDSendata(WATER_LEVEL_ID, water_level_float);

	//电压值
	
	g_queryrealdata_upmessage.queryrealdata_content.precipit_id = (unsigned short)((FIVE_MIN_PRECIPIT_ID >> 8) |(FIVE_MIN_PRECIPIT_ID << 8));
	g_queryrealdata_upmessage.queryrealdata_content.precipit_data[0] = (unsigned char)(current_rainfall_store >> 16);
	g_queryrealdata_upmessage.queryrealdata_content.precipit_data[1] = (unsigned char)(current_rainfall_store >> 8);
	g_queryrealdata_upmessage.queryrealdata_content.precipit_data[2] = (unsigned char)current_rainfall_store;

	g_queryrealdata_upmessage.queryrealdata_content.cumula_precipit_id = (unsigned short)((CUMULA_PRECIPIT_ID >> 8) | (CUMULA_PRECIPIT_ID << 8));
	g_queryrealdata_upmessage.queryrealdata_content.cumula_precipit_data[0] = (unsigned char)(cumulate_rainfall_store >> 16);
	g_queryrealdata_upmessage.queryrealdata_content.cumula_precipit_data[1] = (unsigned char)(cumulate_rainfall_store >> 8);
	g_queryrealdata_upmessage.queryrealdata_content.cumula_precipit_data[2] = (unsigned char)(cumulate_rainfall_store);

	g_queryrealdata_upmessage.queryrealdata_content.water_level_id = (unsigned short)((WATER_LEVEL_ID >> 8) | (WATER_LEVEL_ID << 8));
	g_queryrealdata_upmessage.queryrealdata_content.water_level_data[0] = (unsigned char)(water_level_store >> 24);
	g_queryrealdata_upmessage.queryrealdata_content.water_level_data[1] = (unsigned char)(water_level_store >> 16);
	g_queryrealdata_upmessage.queryrealdata_content.water_level_data[2] = (unsigned char)(water_level_store >> 8);
	g_queryrealdata_upmessage.queryrealdata_content.water_level_data[3] = (unsigned char)water_level_store;

	g_queryrealdata_upmessage.queryrealdata_content.voltage_id = (unsigned short)((VOLTAGE_ID >> 8) | (VOLTAGE_ID << 8));
	g_queryrealdata_upmessage.queryrealdata_content.voltage_data[0] = (unsigned char)(voltage_store >> 8);
	g_queryrealdata_upmessage.queryrealdata_content.voltage_data[1] = (unsigned char)voltage_store;
	g_queryrealdata_upmessage.content_end_id = NETWORK_ETX;

  	crc = CRC16_MODBUS((unsigned char *)&g_queryrealdata_upmessage, sizeof(g_queryrealdata_upmessage) - 2);
	g_queryrealdata_upmessage.crc = crc;
	last_rainfall_pluse_count = g_rainfall_pluse_count;
	sn++;

}

/*sn,pkt_time, data, crc 是变量*/
void QueryTimePeriodData_Init(querrytimeperiod_upmessage_t *querrytimeperiod_upmessage , unsigned short element_id)
{
	unsigned char time_period_id[] = { 0x04,0x18,0x00,0x00,0x05 };
	Time_Def systime;
	RTU_operation_parameters_t rtu_parameters;
	RTC_ReadDate(&systime);
	RTU_parameters_Readdata(&rtu_parameters);

	querrytimeperiod_upmessage->message_header.start_id = NETWORK_S0H;
	querrytimeperiod_upmessage->message_header.center_addr = 0x00;
	memcpy(querrytimeperiod_upmessage->message_header.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	querrytimeperiod_upmessage->message_header.password = (((unsigned short)rtu_parameters.password[0] << 8) & 0xff00)| \
		((unsigned short)rtu_parameters.password[1] &0x00ff);
	querrytimeperiod_upmessage->message_header.fun_id = QUERY_TIME_PERIOD_FUNID;
	querrytimeperiod_upmessage->message_header.content_start_id = NETWORK_STX;

	querrytimeperiod_upmessage->message_header.frame_id_len = (unsigned short)((0x025E >> 8) | (0x025E << 8));  //字节数规定为常量
	querrytimeperiod_upmessage->querrytimeperiod_up_content.sn = 0x0000;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.pkt_time_data.year = systime.year;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.pkt_time_data.month = systime.month;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.pkt_time_data.day = systime.day;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.pkt_time_data.hour = systime.hour;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.pkt_time_data.minute = systime.minute;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.pkt_time_data.second = systime.second;

	querrytimeperiod_upmessage->querrytimeperiod_up_content.remote_addr_id = REMOTE_ADDR_ID;
	memcpy(querrytimeperiod_upmessage->querrytimeperiod_up_content.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	querrytimeperiod_upmessage->querrytimeperiod_up_content.class_id = REMOTE_ADDR_CLASS_ID_HEDAO;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.observ_time.id = OBSERVE_TIME_ID;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.observ_time.year = systime.year;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.observ_time.month = systime.month;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.observ_time.day = systime.day;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.observ_time.hour = systime.hour;
	querrytimeperiod_upmessage->querrytimeperiod_up_content.observ_time.minute = systime.minute;
	
	memcpy(querrytimeperiod_upmessage->querrytimeperiod_up_content.time_period_id, time_period_id, sizeof(time_period_id));
	querrytimeperiod_upmessage->querrytimeperiod_up_content.element_id = (unsigned short)((element_id >> 8) |(element_id << 8));
	querrytimeperiod_upmessage->content_end_id = NETWORK_ETB;
}

/*
	5分钟雨量数据为假设值，电压为假设值，观察时间与发报时间相同
	s
*/
void plus_message_init(void)
{
	static int real_rainfall_data_count = 0;
	static int last_fiveminute_rainfall_data_count = 0;
	float fiveminute_rainfall_float_data = 0;		//float意味真实float意义数据
	int real_waterlevel_integer_data = 0;	        //通过格雷码转换后的十进制数
	float real_waterlevel_float_data = 0;
	float cumcumula_rainfall_float_data = 0;
	int real_waterlevel_store_data = 0;    //store 发送数据拼接
	int fiveminute_rainfall_store_data = 0;
	int cumcumula_rainfall_store_data = 0;
	int volatage_store_data  = 0x1200;
	RTU_operation_parameters_t rtu_parameters;
	unsigned short data_len; //表示报文上下标识和长度
	unsigned short crc;
	Time_Def systime;
	float waterlevel_basic_value = 0;
	
	
	RTU_parameters_Readdata(&rtu_parameters);
	waterlevel_basic_value = BCDSendataToFloat(0x23, rtu_parameters.waterlevel_basevalue_1);
	g_plus_message.header.start_id = NETWORK_S0H;
	#if 0
	printf("plus message parameters\r\n");
	for(i = 0; i < sizeof(rtu_parameters); i++)
	{
		printf("%02x\t", ((unsigned char *)&rtu_parameters)[i]);
	}
	printf("\r\n");

	#endif
	g_plus_message.header.center_addr = rtu_parameters.center_addr[0];
	memcpy(&g_plus_message.header.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	g_plus_message.header.password = (((unsigned short)rtu_parameters.password[0] << 8) & 0xff00)| \
		((unsigned short)rtu_parameters.password[1] &0x00ff);
	g_plus_message.header.fun_id = PLUS_MESSAGE_FUNID;
	data_len = (unsigned char *)&g_plus_message.content_end_id - (unsigned char *)&g_plus_message.plus_message_content.sn;
	g_plus_message.header.frame_id_len = ((data_len & 0X00FF) << 8)|((data_len & 0XFF00) >> 8); //高4位为上下行标识，此处为0000,所以无需处理
	g_plus_message.header.content_start_id = NETWORK_STX;
	g_plus_message.plus_message_content.sn =  ((sn & 0X00FF) << 8)|((sn & 0XFF00) >> 8); 
	if(RTC_ReadDate(&systime) == FALSE)
	{
		printf("read time failed\r\n");
	}
	g_plus_message.plus_message_content.pkt_time.year = systime.year;	
	g_plus_message.plus_message_content.pkt_time.month = systime.month;
	g_plus_message.plus_message_content.pkt_time.day = systime.day;
	g_plus_message.plus_message_content.pkt_time.hour	= systime.hour;
	g_plus_message.plus_message_content.pkt_time.minute = systime.minute;
	g_plus_message.plus_message_content.pkt_time.second = systime.second;
	g_plus_message.plus_message_content.remote_addr_id = 0XF1F1;
	memcpy(g_plus_message.plus_message_content.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	g_plus_message.plus_message_content.class_id = REMOTE_ADDR_CLASS_ID_HEDAO;
	g_plus_message.plus_message_content.observ_time.id = 0xF0F0;
	g_plus_message.plus_message_content.observ_time.year =systime.year;       //此处值待修改      				
	g_plus_message.plus_message_content.observ_time.month = systime.month;
	g_plus_message.plus_message_content.observ_time.day = systime.day;
	g_plus_message.plus_message_content.observ_time.hour = systime.hour;
	g_plus_message.plus_message_content.observ_time.minute = systime.minute;

	/*5分钟降水量*/
	real_rainfall_data_count = g_rainfall_pluse_count;
	fiveminute_rainfall_float_data = (real_rainfall_data_count - last_fiveminute_rainfall_data_count) * RAINFALL_STEP;
	last_fiveminute_rainfall_data_count = real_rainfall_data_count;
	fiveminute_rainfall_store_data = FloatoHex(6, 1, fiveminute_rainfall_float_data);          //6位数，1位小数
	//printf("five minute rainfall data is %06x\r\n", fiveminute_rainfall_store_data);
	
	/*瞬时水位值*/
	real_waterlevel_integer_data = GetWaterLevelData();  //2730十进制表示的数据
	real_waterlevel_float_data = (float)real_waterlevel_integer_data/100.0;
	real_waterlevel_float_data += waterlevel_basic_value;       //加上水位基准值
	real_waterlevel_store_data = FloatoHex(8, 3, real_waterlevel_float_data);     		//8位数，3位小数
	//printf("waterlevel is %08x\r\n", real_waterlevel_store_data);

	/*累积降水量*/
	cumcumula_rainfall_float_data = real_rainfall_data_count * RAINFALL_STEP;
	cumcumula_rainfall_store_data = FloatoHex(6, 1, cumcumula_rainfall_float_data);
	//printf("cucumula_rainfall data is %06x\r\n", cumcumula_rainfall_store_data);

	/*电压*/
	
	
	
	#if 0
	printf("five minute rainfall data is %06x, waterlevel is %08x, cucumula_rainfall data is %06x, voltage is %04x\r\n", \
	fiveminute_rainfall_store_data, real_waterlevel_store_data, cumcumula_rainfall_store_data, volatage_store_data);
	#endif
	
	g_plus_message.plus_message_content.five_min_precipit_id = ((FIVE_MIN_PRECIPIT_ID >> 8)&0X00FF)\
		|((FIVE_MIN_PRECIPIT_ID&0X00FF)	<< 8);   //数据3个字节,小数点后1位 
	g_plus_message.plus_message_content.five_min_precipit_data[0] = (unsigned char)(fiveminute_rainfall_store_data >> 16);         	
	g_plus_message.plus_message_content.five_min_precipit_data[1] = (unsigned char)(fiveminute_rainfall_store_data >> 8);
	g_plus_message.plus_message_content.five_min_precipit_data[2] = (unsigned char)(fiveminute_rainfall_store_data);        //该数据雨量值就是2.5mm
	
	g_plus_message.plus_message_content.water_level_id = ((WATER_LEVEL_ID >> 8)&0X00FF)\
		|((WATER_LEVEL_ID&0X00FF)	<< 8);  //数据4个字节，小数点后3位
	g_plus_message.plus_message_content.water_level_data[0] = (unsigned char )(real_waterlevel_store_data >> 24);										
	g_plus_message.plus_message_content.water_level_data[1] = (unsigned char )(real_waterlevel_store_data >> 16);
	g_plus_message.plus_message_content.water_level_data[2] = (unsigned char )(real_waterlevel_store_data >> 8);
	g_plus_message.plus_message_content.water_level_data[3] = (unsigned char )(real_waterlevel_store_data);    //该水位值位27.3mm
	
	g_plus_message.plus_message_content.cumula_precipit_id = ((CUMULA_PRECIPIT_ID >> 8)&0X00FF)\
		|((CUMULA_PRECIPIT_ID&0X00FF)	<< 8); //数据3个字节，小数点后1位
	g_plus_message.plus_message_content.cumula_precipit_data[0] = (unsigned char)(cumcumula_rainfall_store_data >> 16);							
	g_plus_message.plus_message_content.cumula_precipit_data[1] = (unsigned char )(cumcumula_rainfall_store_data >> 8);
	g_plus_message.plus_message_content.cumula_precipit_data[2] = (unsigned char)cumcumula_rainfall_store_data;  //累积雨量数据为23.5mm
	
	g_plus_message.plus_message_content.voltage_id = ((VOLTAGE_ID >> 8)&0X00FF)\
		|((VOLTAGE_ID&0X00FF)	<< 8);   //数据2字节，小数点后2位，
	g_plus_message.plus_message_content.voltage_data[0] = (unsigned char)(volatage_store_data >> 8);												
	g_plus_message.plus_message_content.voltage_data[1] = (unsigned char)volatage_store_data;  // 电压13.6V 
	 	
	g_plus_message.content_end_id = NETWORK_ETX;
	crc = CRC16_MODBUS((unsigned char *)&g_plus_message,sizeof(g_plus_message)-2);
	g_plus_message.crc = crc;        //**水文所之前版本那边好像还是高字节先发，没有按照通信协议来

	sn++;
}



/*
	发报时间与观察时间一致

*/
void hour_message_init(void)
{
	static unsigned char last_month = 0;
	unsigned short data_len;
	Time_Def systime;
	date_t read_date;
	float cumcumula_rainfall_float_data = 0;
	int cumcumula_rainfall_store_data = 0;
	int volatage_store_data  = 0x1200;
	int real_waterlevel_integer_data = 0;
	float real_waterlevel_float_data = 0;
	int	real_waterlevel_store_data = 0;
	unsigned char flash_rainfall_hour_data[39] = {0};  //13*3
	unsigned char flash_waterlevel_hour_data[52] = {0};      //13*4
	unsigned char f4_rainfall_data[12] = {0};   //
	unsigned short f5_waterlevel_data[12] = {0};
	unsigned char months_days[12] = {0};
	RTU_operation_parameters_t rtu_parameters;
	
	
	RTU_parameters_Readdata(&rtu_parameters);
	g_hour_message.header.start_id = NETWORK_S0H;
	g_hour_message.header.center_addr = rtu_parameters.center_addr[0];
	memcpy(g_hour_message.header.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	g_hour_message.header.password = (((unsigned short)rtu_parameters.password[0] << 8) & 0xff00)| \
		((unsigned short)rtu_parameters.password[1] &0x00ff);
	g_hour_message.header.fun_id = HOUR_MESSAGE_FUNID;
	data_len = (unsigned char *)&g_hour_message.content_end_id - \
		(unsigned char *)&g_hour_message.hour_message_content.sn;
	g_hour_message.header.frame_id_len =((data_len & 0X00FF) << 8)|((data_len & 0XFF00) >> 8);
	g_hour_message.header.content_start_id = NETWORK_STX;
	
	g_hour_message.hour_message_content.sn = ((sn & 0X00FF) << 8)|((sn & 0XFF00) >> 8);
	delay_ms(1000);
	RTC_ReadDate(&systime);	
	g_hour_message.hour_message_content.pkt_time.year = systime.year;											
	g_hour_message.hour_message_content.pkt_time.month = systime.month;
	g_hour_message.hour_message_content.pkt_time.day = systime.day;
	g_hour_message.hour_message_content.pkt_time.hour = systime.hour;
	g_hour_message.hour_message_content.pkt_time.minute = systime.minute;
	g_hour_message.hour_message_content.pkt_time.second = systime.second;
	g_hour_message.hour_message_content.remote_addr_id = 0XF1F1;
	memcpy(g_hour_message.header.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	g_hour_message.hour_message_content.class_id = REMOTE_ADDR_CLASS_ID_HEDAO;
	g_hour_message.hour_message_content.observ_time.id = 0XF0F0;
	g_hour_message.hour_message_content.observ_time.year = systime.year;									
	g_hour_message.hour_message_content.observ_time.month = systime.month;
	g_hour_message.hour_message_content.observ_time.day = systime.day;
	g_hour_message.hour_message_content.observ_time.hour = systime.hour;
	g_hour_message.hour_message_content.observ_time.minute = systime.minute;

	read_date.year = BCDToInteger(systime.year);
	read_date.month = BCDToInteger(systime.month);
	read_date.day = BCDToInteger(systime.day);
	read_date.hour = BCDToInteger(systime.hour);

	GetMonthDays(systime.year, months_days);
	if(read_date.month == last_month)
	{
		if((read_date.hour == 0) & (read_date.minute == 0))  //23：59 d到0：0 ,还有月底到月初之间的转换1月31日23：59
		{
			read_date.day -= 1;
			read_date.hour = 24;
		}
	}
	else
	{
		if((read_date.hour == 0) & (read_date.minute == 0))
		{
			read_date.month -= 1;
			read_date.day = months_days[read_date.month - 1];
			printf("read_date.day is %d\r\n", read_date.day);						//day
			read_date.hour = 24;	
		}
	}
	read_date.minute = 0;               ///一小时的数据起始分钟为0；
	read_date.hour -= 1;
	#if HOUR_MESSAGE_DEBUG
	printf(" hourmessage  read flash date :%d-%d-%d-%d\r\n",read_date.year, read_date.month,read_date.day,read_date.hour);
	#endif
	last_month  = BCDToInteger(systime.month);
	/*5 min雨量变化 读取flash */
	ReadDataFromFlash(&read_date, RAINFALL_TYPE, flash_rainfall_hour_data, 39);  //13*3
	
	#if HOUR_MESSAGE_DEBUG
	printf("rainfall data:\r\n");
	for(i = 0; i < 39; i++ )
	{
		printf("%02x\t", flash_rainfall_hour_data[i]);
	}
	printf("\r\n");
	#endif
	HourRainFallSaveData_To_FiveMinuteRelativeValue(flash_rainfall_hour_data, f4_rainfall_data);
	memcpy(g_hour_message.hour_message_content.hour_five_prcipit_data, f4_rainfall_data, 12);

	/*累积降水量*/
	cumcumula_rainfall_float_data = g_rainfall_pluse_count * RAINFALL_STEP;
	cumcumula_rainfall_store_data = FloatoHex(6, 1, cumcumula_rainfall_float_data);
	//printf("cucumula_rainfall data is %06x\r\n", cumcumula_rainfall_store_data);

	/*5 min 水位变化，读取flash*/
	ReadDataFromFlash(&read_date, WATERLEVEL_TYPE, flash_waterlevel_hour_data, 52);
	#if HOUR_MESSAGE_DEBUG
	printf("waterlevl data :\r\n");
	for(i = 0; i < 52; i++ )
	{
		printf("%02x\t", flash_waterlevel_hour_data[i]);
	}
	printf("\r\n");
	#endif
	HourWaterLevelSaveData_To_Value(flash_waterlevel_hour_data, f5_waterlevel_data);
	memcpy(g_hour_message.hour_message_content.hour_five_waterlevel_data, (u8 *)f5_waterlevel_data, 24);

	/*瞬时水位值*/
	real_waterlevel_integer_data = GetWaterLevelData();  //2730十进制表示的数据
	real_waterlevel_float_data = (float)real_waterlevel_integer_data/100.0;
	real_waterlevel_store_data = FloatoHex(8, 3, real_waterlevel_float_data);     		//8位数，3位小数

	g_hour_message.hour_message_content.hour_five_precipit_id = ((HOUR_FIVE_MIN_PRECIPIT_ID >> 8)&0X00FF)\
		|((HOUR_FIVE_MIN_PRECIPIT_ID&0X00FF) << 8);   //数据定义看通信协议41页

	g_hour_message.hour_message_content.hour_five_waterlevel_id = ((HOUR_FIVE_WATERLEVEL_ID >> 8)&0X00FF)\
		|((HOUR_FIVE_WATERLEVEL_ID&0X00FF)	<< 8);

	g_hour_message.hour_message_content.cumula_precipit_id = ((CUMULA_PRECIPIT_ID >> 8)&0X00FF)\
		|((CUMULA_PRECIPIT_ID&0X00FF)	<< 8);
	g_hour_message.hour_message_content.cumula_precipit_data[0] = (unsigned char)(cumcumula_rainfall_store_data >> 16);					
	g_hour_message.hour_message_content.cumula_precipit_data[1] = (unsigned char)(cumcumula_rainfall_store_data >> 8);
	g_hour_message.hour_message_content.cumula_precipit_data[2] = (unsigned char)cumcumula_rainfall_store_data;
	
	g_hour_message.hour_message_content.water_level_id =  ((WATER_LEVEL_ID >> 8)&0X00FF)\
		|((WATER_LEVEL_ID&0X00FF)	<< 8);  //数据4个字节，小数点后3位
	g_hour_message.hour_message_content.water_level_data[0] = (unsigned char)(real_waterlevel_store_data >> 24);
	g_hour_message.hour_message_content.water_level_data[1] = (unsigned char)(real_waterlevel_store_data >> 16);
	g_hour_message.hour_message_content.water_level_data[2] = (unsigned char)(real_waterlevel_store_data >> 8);
	g_hour_message.hour_message_content.water_level_data[3] = (unsigned char)(real_waterlevel_store_data);

	g_hour_message.hour_message_content.voltage_id = ((VOLTAGE_ID >> 8)&0X00FF)\
		|((VOLTAGE_ID&0X00FF)	<< 8);
	g_hour_message.hour_message_content.voltage_data[0] = (unsigned char)(volatage_store_data >> 8);									
	g_hour_message.hour_message_content.voltage_data[1] = (unsigned char)volatage_store_data;

	
	g_hour_message.content_end_id = NETWORK_ETX;
	g_hour_message.crc = CRC16_MODBUS((unsigned char *)&g_hour_message,sizeof(g_hour_message)-2);
}

void QueryParameters_Init(querryparameter_upmessage_t *querryparameter_upmessage, RTU_operation_parameters_t *rtu_operation_parameters)
{
	unsigned char remote_addr[5] = {0x00,0x55,0x00,0x00,0x01};
	Time_Def systime;
	

	querryparameter_upmessage->message_header.start_id = NETWORK_S0H;
	querryparameter_upmessage->message_header.center_addr = 0x00;
	memcpy(querryparameter_upmessage->message_header.remote_addr,remote_addr,sizeof(remote_addr));
	querryparameter_upmessage->message_header.password = 0x0000;
	querryparameter_upmessage->message_header.fun_id = QUERY_PARAMETER_FUNID;
	querryparameter_upmessage->message_header.frame_id_len = (unsigned short)((0x807C << 8) | (0x807C >> 8));
	querryparameter_upmessage->message_header.content_start_id = NETWORK_STX;
	querryparameter_upmessage->content.sn =  (unsigned short)(((sn & 0X00FF) << 8)|((sn & 0XFF00) >> 8));
	RTC_ReadDate(&systime);
	querryparameter_upmessage->content.pkt_time.year = systime.year;
	querryparameter_upmessage->content.pkt_time.month = systime.month;
	querryparameter_upmessage->content.pkt_time.day = systime.day;
	querryparameter_upmessage->content.pkt_time.hour = systime.hour;
	querryparameter_upmessage->content.pkt_time.minute = systime.minute;
	querryparameter_upmessage->content.pkt_time.second = systime.second;
	querryparameter_upmessage->content.center_addr_guide_id = (unsigned short)((CENTER_ADDRESS_GUIDE_ID >> 8) | (CENTER_ADDRESS_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_addr,rtu_operation_parameters->center_addr, 4);
	querryparameter_upmessage->content.remote_addr_guide_id = (unsigned short)((REMOTE_ADDRESS_GUIDE_ID >> 8) | (REMOTE_ADDRESS_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.rtu_remote_addr, rtu_operation_parameters->rtu_remote_addr,5);
	querryparameter_upmessage->content.password_guide_id = (unsigned short)((PASSWOD_GUIDE_ID >> 8) | (PASSWOD_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.password, rtu_operation_parameters->password, 2);
	querryparameter_upmessage->content.center_1_main_channel_guide_id = (unsigned short)((CENTER_1_MAIN_CHANNEL_GUIDE_ID >> 8) | (CENTER_1_MAIN_CHANNEL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_1_main_channel,rtu_operation_parameters->center_1_main_channel, 10);
	querryparameter_upmessage->content.center_1_spare_channel_guide_id = (unsigned short)((CENTER_1_SPARE_CHANNEL_GUIDE_ID >> 8) | (CENTER_1_SPARE_CHANNEL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_1_spare_channel, rtu_operation_parameters->center_1_spare_channel, 10);

	querryparameter_upmessage->content.center_2_main_channel_guide_id = (unsigned short)((CENTER_2_MAIN_CHANNEL_GUIDE_ID >> 8) | (CENTER_2_MAIN_CHANNEL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_2_main_channel, rtu_operation_parameters->center_2_main_channel, 10);
	querryparameter_upmessage->content.center_2_spare_channel_guide_id = (unsigned short)((CENTER_2_SPARE_CHANNEL_GUIDE_ID >> 8) | (CENTER_2_SPARE_CHANNEL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_2_spare_channel, rtu_operation_parameters->center_2_spare_channel, 10);

	querryparameter_upmessage->content.center_3_main_channel_guide_id = (unsigned short)((CENTER_3_MAIN_CHANNEL_GUIDE_ID >> 8) | (CENTER_3_MAIN_CHANNEL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_3_main_channel, rtu_operation_parameters->center_3_main_channel, 10);
	querryparameter_upmessage->content.center_3_spare_channel_guide_id = (unsigned short)((CENTER_3_SPARE_CHANNEL_GUIDE_ID >> 8) | (CENTER_3_SPARE_CHANNEL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_3_spare_channel, rtu_operation_parameters->center_3_spare_channel, 10);

	querryparameter_upmessage->content.center_4_main_channel_guide_id = (unsigned short)((CENTER_4_MAIN_CHANNEL_GUIDE_ID >> 8) | (CENTER_4_MAIN_CHANNEL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_4_main_channel, rtu_operation_parameters->center_4_main_channel, 10);
	querryparameter_upmessage->content.center_4_spare_channel_guide_id = (unsigned short)((CENTER_4_SPARE_CHANNEL_GUIDE_ID >> 8) | (CENTER_4_SPARE_CHANNEL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.center_4_spare_channel, rtu_operation_parameters->center_4_spare_channel, 10);

	querryparameter_upmessage->content.work_style_guide_id = (unsigned short)((WORK_STYLE_GUIDE_ID >> 8) | (WORK_STYLE_GUIDE_ID << 8));
	querryparameter_upmessage->content.work_style = rtu_operation_parameters->work_style;
	querryparameter_upmessage->content.remote_collect_factor_guide_id = (unsigned short)((REMOTE_COLLECT_FACTOR_GUIDE_ID >> 8) | (REMOTE_COLLECT_FACTOR_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.remote_collect_factor, rtu_operation_parameters->remote_collect_factor, 4);

	querryparameter_upmessage->content.time_interval_of_plus_message_guide_id = (unsigned short)((PLUS_INTERVAL_GUIDE_ID>> 8) | (PLUS_INTERVAL_GUIDE_ID << 8));
	querryparameter_upmessage->content.time_interval_of_plus_message = rtu_operation_parameters->time_interval_of_plus_message;

	querryparameter_upmessage->content.time_interval_of_timing_message_id = (unsigned short)((TIMING_TIME_INTERVAL_GUIDE_ID >> 8) | (TIMING_TIME_INTERVAL_GUIDE_ID << 8));
	querryparameter_upmessage->content.time_interval_of_timing_message = rtu_operation_parameters->time_interval_of_timing_message;

	querryparameter_upmessage->content.rainfall_day_start_time_guide_id = (unsigned short)((RAINFALL_DAY_STARTTIME_GUIDE_ID >> 8) | (RAINFALL_DAY_STARTTIME_GUIDE_ID << 8));
	querryparameter_upmessage->content.rainfall_day_start_time = rtu_operation_parameters->rainfall_day_start_time;

	querryparameter_upmessage->content.sampling_interval_guide_id = (unsigned short)((COLLECT_INTERVAL_GUIDE_ID >> 8) | (COLLECT_INTERVAL_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.sampling_interval, rtu_operation_parameters->sampling_interval, 2);

	querryparameter_upmessage->content.waterlevel_save_interval_guide_id = (unsigned short)((WATERLEVEL_SAVE_INTERVAL_GUIDE_ID >> 8)| (WATERLEVEL_SAVE_INTERVAL_GUIDE_ID << 8));
	querryparameter_upmessage->content.waterlevel_save_interval = rtu_operation_parameters->waterlevel_save_interval;
	
	querryparameter_upmessage->content.rainfall_resolution_guide_id = (unsigned short)((RAINFALL_RESOLUTION_GUIDE_ID >> 8) | (RAINFALL_RESOLUTION_GUIDE_ID << 8));
	querryparameter_upmessage->content.rainfall_resolution = rtu_operation_parameters->rainfall_resolution;

	querryparameter_upmessage->content.waterlevel_resolution_guide_id = (unsigned short)((WATERLEVEL_RESOLUTION_GUIDE_ID >> 8) | (WATERLEVEL_RESOLUTION_GUIDE_ID << 8));
	querryparameter_upmessage->content.waterlevel_resolution = rtu_operation_parameters->waterlevel_resolution;

	querryparameter_upmessage->content.threshold_rainfall_value_of_plusemessage_guide_id = (unsigned short)((RAINFALL_PLUS_THRESHOLD_GUIDE_ID >> 8) | (RAINFALL_PLUS_THRESHOLD_GUIDE_ID << 8));
	querryparameter_upmessage->content.threshold_rainfall_value_of_plusemessage = rtu_operation_parameters->threshold_rainfall_value_of_plusemessage;

	querryparameter_upmessage->content.waterlevel_plusmessage_1_guide_id = (unsigned short)((WATERLEVEL_PLUS_GUIDE_ID >> 8) | (WATERLEVEL_PLUS_UP_THRESHOLD_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.waterlevel_plusmessage_1, rtu_operation_parameters->waterlevel_plusmessage_1, 2);

	querryparameter_upmessage->content.above_threshold_waterlevel_value_of_plusmeessag_guide_id = (unsigned short)((WATERLEVEL_PLUS_UP_THRESHOLD_GUIDE_ID >> 8) | (WATERLEVEL_PLUS_UP_THRESHOLD_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.above_threshold_waterlevel_value_of_plusmeessage, rtu_operation_parameters->above_threshold_waterlevel_value_of_plusmeessag, 2);

	querryparameter_upmessage->content.below_threshold_waterlevel_value_of_plusmessage_guide_id = (unsigned short)((WATERLEVEL_PLUS_DOWN_THRESHOLD_GUIDE_ID >>  8) | (WATERLEVEL_PLUS_DOWN_THRESHOLD_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.below_threshold_waterlevel_value_of_plusmessage, rtu_operation_parameters->below_threshold_waterlevel_value_of_plusmessage, 2);

	querryparameter_upmessage->content.flow_threshold_plusmessage_guide_id = (unsigned short )((FLOW_PLUS_THRESHOLD_GUIDE_ID >> 8) | (FLOW_PLUS_THRESHOLD_GUIDE_ID << 8));
	memcpy(querryparameter_upmessage->content.flow_threshold_plusmessage, rtu_operation_parameters->flow_threshold_plusmessage, 3);

	querryparameter_upmessage->end_id = 0x03;
	querryparameter_upmessage->crc = 0xffff;
	sn++;

}

void SetParameters(setparameter_downmessage_t *setparameter_downmessage)
{
	RTU_operation_parameters_t write_parameters;

	memcpy(write_parameters.center_addr,setparameter_downmessage->content.center_addr,4);
	memcpy(write_parameters.rtu_remote_addr, setparameter_downmessage->content.rtu_remote_addr, 5);
	memcpy(write_parameters.password, setparameter_downmessage->content.password, 2);
	memcpy(write_parameters.center_1_main_channel, setparameter_downmessage->content.center_1_main_channel, 10);
	memcpy(write_parameters.center_1_spare_channel, setparameter_downmessage->content.center_1_spare_channel, 10);
	
	memcpy(write_parameters.center_2_main_channel, setparameter_downmessage->content.center_2_main_channel, 10);
	memcpy(write_parameters.center_2_spare_channel, setparameter_downmessage->content.center_2_spare_channel, 10);

	memcpy(write_parameters.center_3_main_channel, setparameter_downmessage->content.center_3_main_channel, 10);
	memcpy(write_parameters.center_3_spare_channel, setparameter_downmessage->content.center_3_spare_channel, 10);

	memcpy(write_parameters.center_4_main_channel, setparameter_downmessage->content.center_4_main_channel, 10);
	memcpy(write_parameters.center_4_spare_channel, setparameter_downmessage->content.center_4_spare_channel, 10);

	write_parameters.work_style = setparameter_downmessage->content.work_style;
	memcpy(write_parameters.remote_collect_factor, setparameter_downmessage->content.remote_collect_factor, 4);
	write_parameters.time_interval_of_timing_message = setparameter_downmessage->content.time_interval_of_timing_message;
	write_parameters.time_interval_of_plus_message = setparameter_downmessage->content.time_interval_of_plus_message;
	write_parameters.rainfall_day_start_time = setparameter_downmessage->content.rainfall_day_start_time;
	memcpy(write_parameters.sampling_interval, setparameter_downmessage->content.sampling_interval, 2);
	write_parameters.waterlevel_save_interval = setparameter_downmessage->content.waterlevel_save_interval;
	write_parameters.rainfall_resolution = setparameter_downmessage->content.rainfall_resolution;
	write_parameters.waterlevel_resolution = setparameter_downmessage->content.waterlevel_resolution;
	write_parameters.threshold_rainfall_value_of_plusemessage = setparameter_downmessage->content.threshold_rainfall_value_of_plusemessage;
	
	//base value ignore

	memcpy(write_parameters.waterlevel_plusmessage_1,setparameter_downmessage->content.waterlevel_plusmessage_1, 2);
	memcpy(write_parameters.above_threshold_waterlevel_value_of_plusmeessag, setparameter_downmessage->content.above_threshold_waterlevel_value_of_plusmeessage, 2);
	memcpy(write_parameters.below_threshold_waterlevel_value_of_plusmessage, setparameter_downmessage->content.below_threshold_waterlevel_value_of_plusmessage, 2);
	memcpy(write_parameters.flow_threshold_plusmessage,setparameter_downmessage->content.flow_threshold_plusmessage, 3);

	RTU_parameters_Writedata(&write_parameters);
}


// 0 -代表接收到ACK, 1- 代表收到NAK；
int JudgeReceiveTimePeriod(void)
{
	querrytimeperiod_downmessage_t *buff = NULL;
	int i = 0;

	for (i = 0; i < 3; i++)
	{
		if (g_usart2_receive_stat == 1)
		{
			g_usart2_receive_stat = 0;
			
			buff = (querrytimeperiod_downmessage_t *)DMA_usart2_Buf;
			if (buff->content_end_id == NETWORK_ACK)
			{
				return RECEIVE_ACK;
			}
			else if (buff->content_end_id == NETWORK_NAK)
			{
				return RECEIVE_NAK;
			}
			else
			{
				return RECEIVE_ERRO;
			}
			

		}
		delay_ms(RECEIVE_DELAY);
	}
	return RECEIVE_ERRO;
	
}


//该函数使用前判断接受状态是否完成
unsigned char GetDownFrameFunId(const unsigned char serial)
{
	u8 buff[200] = {0};
	u8 ip[20] = {0};
	u8 port[10] = {0};
	unsigned char fun_id = 0;
	unsigned int i = 0;

	if(serial == SERIAL_2)
	{
			g_gprs_receive_data_length = Analysis_Data_FromGPRS(DMA_usart2_Buf,ip,port,g_gprs_receive_data_buff);
			memcpy(buff, g_gprs_receive_data_buff, g_gprs_receive_data_length);
			fun_id = ((message_header_t *)buff)->fun_id;	

	}
	else if(serial == RS232_3)
	{
			#if 1
			printf("rs232_3 receive data:");
			for (i = 0; i < RS232_3_Recv_Address; i++)
			{
				printf("%02x\t", RS232_3_Recv[i]);
			}
			printf("\r\n");
			#endif
			fun_id = ((message_header_t *)RS232_3_Recv)->fun_id;
	}
	else
	{
		/* code */
		printf("receive command task: no detect appropriate serial\r\n");
		return NO_MATCH_SERIAL;
	}
	switch(fun_id)
	{
		case PLUS_MESSAGE_FUNID:
			return PLUS_MESSAGE_FUNID;
		case HOUR_MESSAGE_FUNID:
			return HOUR_MESSAGE_FUNID;
		case QUERY_REALDATA_FUNID:
			return QUERY_REALDATA_FUNID;
		case QUERY_TIME_PERIOD_FUNID:
			return QUERY_TIME_PERIOD_FUNID;
		case SET_PARAMETER_FUNID:
			return SET_PARAMETER_FUNID;
		case QUERY_PARAMETER_FUNID:
			return QUERY_PARAMETER_FUNID;	
		case SET_REAL_TIME:
			return SET_REAL_TIME;
		case  ENTER_RESPONSE_MODE:
			return ENTER_RESPONSE_MODE;
		case  ENTER_NORMAL_MODE:
			return ENTER_NORMAL_MODE;
	
		default:
			return NO_MATCH_FUNID;
	}
}

//由于ST16拓展串口问题，串口接收指令只能通过232_1，上报通过232_3;
int ExecuteFunctionByFunId(const unsigned char fun_id, const unsigned char serial)
{
	calibration_time_downmsssage_t *calibration_time_downmsssage = NULL;
	querrytimeperiod_downmessage_t *querrytimeperiod_downmessage = NULL;
	setparameter_downmessage_t *setparameter_downmessage = NULL;
	querryparameter_upmessage_t	 querryparameter_upmessage;
	RTU_operation_parameters_t read_operation_parameters;

	Time_Def write_time = {0};

	switch (fun_id)
	{
	case SET_REAL_TIME:
	 if(serial == SERIAL_2)
		{
			calibration_time_downmsssage = (calibration_time_downmsssage_t *)g_gprs_receive_data_buff;
			/* code */
		}	
		else if(serial == RS232_3)
		{
			calibration_time_downmsssage = (calibration_time_downmsssage_t *)RS232_3_Recv;
		}
		printf("**************set time is %02x-%02x-%02x :  %02x:%02x:%02x\r\n", calibration_time_downmsssage->pkt_time.year, calibration_time_downmsssage->pkt_time.month, \
		calibration_time_downmsssage->pkt_time.day, calibration_time_downmsssage->pkt_time.hour, calibration_time_downmsssage->pkt_time.month, \
		calibration_time_downmsssage->pkt_time.second);
		write_time.year = calibration_time_downmsssage->pkt_time.year;
		write_time.month = calibration_time_downmsssage->pkt_time.month;
		write_time.day = calibration_time_downmsssage->pkt_time.day;
		write_time.hour = calibration_time_downmsssage->pkt_time.hour;
		write_time.minute = calibration_time_downmsssage->pkt_time.minute;
		write_time.second = calibration_time_downmsssage->pkt_time.second;
		if(RTC_WriteDate(&write_time) != TRUE)
		{
			printf("SET TIME failed\r\n");
		}
		printf("SET TIME success\r\n");
		break;
	case QUERY_REALDATA_FUNID:
		QueryRealData_Init();
		if(serial == SERIAL_2)
		{
			gprs_send_string_len((unsigned char *)&g_queryrealdata_upmessage, sizeof(g_queryrealdata_upmessage));
		}
		else if(serial == RS232_3)
		{
			RS232_3_Send_buf((unsigned char *)&g_queryrealdata_upmessage, sizeof(g_queryrealdata_upmessage));
		}
		break;
	
	case QUERY_TIME_PERIOD_FUNID:
		if(serial == SERIAL_2)
		{
			querrytimeperiod_downmessage = (querrytimeperiod_downmessage_t *)g_gprs_receive_data_buff;
		}
		else if(serial == RS232_3)
		{
			querrytimeperiod_downmessage = (querrytimeperiod_downmessage_t *)RS232_3_Recv;
		}
		printf("start time is %02x-%02x-%02x-%02x, end time is %02x-%02x-%02x-%02x\r\n", querrytimeperiod_downmessage->querrytimeperiod_down_content.start_time.year,querrytimeperiod_downmessage->querrytimeperiod_down_content.start_time.month,\
		querrytimeperiod_downmessage->querrytimeperiod_down_content.start_time.day,querrytimeperiod_downmessage->querrytimeperiod_down_content.start_time.hour, \
		querrytimeperiod_downmessage->querrytimeperiod_down_content.end_time.year, querrytimeperiod_downmessage->querrytimeperiod_down_content.end_time.month, \
		querrytimeperiod_downmessage->querrytimeperiod_down_content.end_time.day, querrytimeperiod_downmessage->querrytimeperiod_down_content.end_time.hour);
		test_SendTimePeriodData(querrytimeperiod_downmessage, serial);
		break;
	case QUERY_PARAMETER_FUNID:
		if(serial == RS232_3)
		{
			//querryparameter_upmessage 初始化赋值
			RTU_parameters_Readdata(&read_operation_parameters);
			QueryParameters_Init(&querryparameter_upmessage, &read_operation_parameters);
			RS232_3_Send_buf((unsigned char *)&querryparameter_upmessage, sizeof(querryparameter_upmessage));
		}
		break;
	case SET_PARAMETER_FUNID:       
		if(serial == RS232_3)
		{
			setparameter_downmessage = (setparameter_downmessage_t *)RS232_3_Recv;
			SetParameters(setparameter_downmessage);
		}
		break;
	default:
		break;
	}
	return 0;

}

int ResponseUpMessage(const unsigned char fun_id, const unsigned char serial)
{
	calibration_time_upmsssage_t calibration_time_upmsssage = {0};

	switch (fun_id)
	{
	case SET_REAL_TIME:
		Calibration_time_UpMessage_init(&calibration_time_upmsssage);
		print_struct(&calibration_time_upmsssage, sizeof(calibration_time_upmsssage));
		printf("\r\n");
		if(serial == RS232_3)
		{
			RS232_3_Send_buf((unsigned char *)&calibration_time_upmsssage, sizeof(calibration_time_upmsssage_t));
		}
		else if(serial == SERIAL_2)
		{
			gprs_send_string_len((unsigned char *)&calibration_time_upmsssage, sizeof(calibration_time_upmsssage_t));
		}
		break;
	
	default:
		break;
	}
	
	return 0;

}


void Calibration_time_UpMessage_init(calibration_time_upmsssage_t *calibration_time_upmsssage)
{
	unsigned short frame_id_len = 0;
	unsigned short crc = 0;
	Time_Def real_time;
	RTU_operation_parameters_t rtu_parameters;

	RTC_ReadDate(&real_time);
	RTU_parameters_Readdata(&rtu_parameters);

	calibration_time_upmsssage->header.start_id = NETWORK_S0H;
	calibration_time_upmsssage->header.center_addr = rtu_parameters.center_addr[0];
	memcpy(calibration_time_upmsssage->header.remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	calibration_time_upmsssage->header.password = (((unsigned short)rtu_parameters.password[0] << 8) & 0xff00)| \
		((unsigned short)rtu_parameters.password[1] &0x00ff);
	calibration_time_upmsssage->header.fun_id = SET_REAL_TIME;
	printf("SET_RAEL_TIME %02x\r\n", SET_REAL_TIME);
	calibration_time_upmsssage->header.content_start_id = NETWORK_STX;

	frame_id_len = (unsigned char *)(&calibration_time_upmsssage->content_end_id) - (unsigned char *)&(calibration_time_upmsssage->sn);
	calibration_time_upmsssage->sn = ((sn & 0X00FF) << 8)|((sn & 0XFF00) >> 8); 
	calibration_time_upmsssage->pkt_time.year = real_time.year;
	calibration_time_upmsssage->pkt_time.month = real_time.month;
	calibration_time_upmsssage->pkt_time.day = real_time.day;
	calibration_time_upmsssage->pkt_time.hour = real_time.hour;
	calibration_time_upmsssage->pkt_time.minute = real_time.minute;
	calibration_time_upmsssage->pkt_time.second = real_time.second;

	/*other code init*/
	calibration_time_upmsssage->remote_addr_id = REMOTE_ADDR_ID;
	memcpy(calibration_time_upmsssage->remote_addr, rtu_parameters.rtu_remote_addr, sizeof(rtu_parameters.rtu_remote_addr));
	calibration_time_upmsssage->header.frame_id_len = (frame_id_len << 8) | (frame_id_len >> 8);
	printf("frame_id_len calculate is %d\r\n", frame_id_len);
	calibration_time_upmsssage->content_end_id = NETWORK_ETX;
	calibration_time_upmsssage->crc = crc;
	sn++;

}

static void TimeperiodToDate(timeperiod_t *src_time, date_t *des_time)
{
	des_time->year = BCDToInteger(src_time->year);
	des_time->month = BCDToInteger(src_time->month);
	des_time->day = BCDToInteger(src_time->day);
	des_time->hour = BCDToInteger(src_time->hour);
	des_time->minute = 0;
}

static unsigned int GetHourGroupsByTimperiod(timeperiod_t *start_time, timeperiod_t *end_time)
{
	unsigned char month_days[12] = {0};
	date_t start_date;
	date_t end_date;
	unsigned int hour_groups = 0;
	unsigned char i = 0;
	unsigned char month_interval = 0;
	unsigned int temp = 0;

	TimeperiodToDate(start_time, &start_date);
	TimeperiodToDate(end_time, &end_date);
	GetMonthDays(start_time->year, month_days); 
	
	if(end_date.month == start_date.month)
	{	
		if(end_date.day == start_date.day)
		{
			if(end_date.hour == 0)
			{
				end_date.hour = 24;
			}
			hour_groups = end_date.hour - start_date.hour;
		}
		else
		{
			hour_groups = (end_date.day - start_date.day - 1) * 24;
			hour_groups += 24 - start_date.hour;
			hour_groups += end_date.hour;
		}
	}
	else              //不同月的数据
	{
		month_interval = end_date.month - start_date.month - 1;
		for(i = start_date.month; i < start_date.month +  month_interval; i++)
		{
			hour_groups += month_days[i] * 24;
		}
		printf("hour _groups month inteval is %d\r\n",hour_groups);
		temp = ((month_days[start_date.month - 1] - start_date.day) * 24 + (24 - start_date.hour));
		printf("temp1 is %d\r\n", temp);
		hour_groups += temp;
		temp = ((end_date.day - 1) * 24 + end_date.hour);
		hour_groups += temp;	
		printf("temp2 is %d\r\n", temp);
	}
	return hour_groups;
}


static void SendTimePerioadData_upframe(querrytimeperiod_upmessage_t *querrytimeperiod_upmessage, unsigned char serial)
{
	unsigned char buff[TIME_PERIOD_MAX_LEN] = {0};
	unsigned char *ptr = buff;
	int offset_start = (sizeof(querrytimeperiod_upmessage->message_header) + sizeof(querrytimeperiod_upmessage->total_pkts_sn) +\
	sizeof(querrytimeperiod_upmessage->querrytimeperiod_up_content) - 8);
	unsigned short crc = 0;

	querrytimeperiod_upmessage->querrytimeperiod_up_content.sn = ((sn & 0x00ff )<< 8) | ((sn & 0xff00) >> 8);
	memcpy(buff, (u8 *)querrytimeperiod_upmessage, offset_start);
	ptr += offset_start;
	W25QXX_Read(buff + offset_start, querrytimeperiod_upmessage->querrytimeperiod_up_content.start_addr, querrytimeperiod_upmessage->querrytimeperiod_up_content.data_length);
	ptr +=  querrytimeperiod_upmessage->querrytimeperiod_up_content.data_length;
	*ptr++ = NETWORK_ETB;
	crc = CRC16_MODBUS(buff, ptr - buff);
	*ptr++ = (unsigned char)(crc >> 8);
	*ptr++ = (unsigned char)(crc & 0x00ff);
	sn++;
	if(serial == SERIAL_2)
	{
		gprs_send_string_len(buff, ptr - buff);
	}
	else if(serial == RS232_3)
	{
		RS232_3_Send_buf(buff, ptr - buff);
	}
}


/*12个小时为包的单位数据，小于12个小时就为一个包,正文流水号没加上*/
void test_SendTimePeriodData(querrytimeperiod_downmessage_t *querrytimeperiod_downmessage, unsigned char serial)
{
	unsigned short element_id = 0;
	//unsigned char month_days[12] = {0};
	querrytimeperiod_upmessage_t querrytimeperiod_upmessage = { 0 };
	unsigned int hour_groups = 0;
	unsigned int data_length = 0; //包中读取FLASH字节长度
	unsigned int read_address = 0;  // FLASH地址
	unsigned int integer_total_pkts = 0;   //整数12组包数
	unsigned char remainder_pkts = 0;   //剩余12组数据的包数， 1或者0
	unsigned int remainder_hour_grops = 0;
	unsigned short total_pkts = 0;
	unsigned short  frame_id_length = 0; //报文SYN,与结束符之间的长度
	unsigned int i = 1;
	date_t start_date;

	TimeperiodToDate(&querrytimeperiod_downmessage->querrytimeperiod_down_content.start_time, &start_date);
	element_id = querrytimeperiod_downmessage->querrytimeperiod_down_content.precipit_id;
	element_id = (element_id >> 8) | (element_id << 8);
	QueryTimePeriodData_Init(&querrytimeperiod_upmessage, element_id);   //初始化常量数据
	//得到数据有多少组（小时为单位）
	hour_groups = GetHourGroupsByTimperiod(&querrytimeperiod_downmessage->querrytimeperiod_down_content.start_time, \
	&querrytimeperiod_downmessage->querrytimeperiod_down_content.end_time);
	printf("hour_groups is %d , element_id is %02x\r\n", hour_groups, element_id);

	if(hour_groups <= TIMEPERIOD_CONTENT_HOURGROUPS) //一个包
	{
		querrytimeperiod_upmessage.total_pkts_sn[0] = 0x00;
		querrytimeperiod_upmessage.total_pkts_sn[1] = 0x10;
		querrytimeperiod_upmessage.total_pkts_sn[2] = 0x01;
		
		if(element_id == CUMULA_PRECIPIT_ID)
		{
			//上下标识及长度
			frame_id_length = 3 + sizeof(querrytimeperiod_upmessage.querrytimeperiod_up_content) - 8 + hour_groups * 12 * RAINFALL_SINGLE_DATALEN;
			printf("frame_id_length is %d\r\n", frame_id_length);
			querrytimeperiod_upmessage.message_header.frame_id_len = (frame_id_length >> 8) | (frame_id_length << 8);
			GetAddress(&start_date, RAINFALL_TYPE, (int *)&read_address);
			printf(" rainfall sin :read_address is %08x\r\n", read_address);
			data_length = (hour_groups) * 12 * RAINFALL_SINGLE_DATALEN;  //每小时组中有12组数据
			querrytimeperiod_upmessage.querrytimeperiod_up_content.start_addr = read_address;
			querrytimeperiod_upmessage.querrytimeperiod_up_content.data_length = data_length;
			printf("sn is %04x\r\n",sn);
			SendTimePerioadData_upframe(&querrytimeperiod_upmessage, serial);
		}
		else if(element_id == WATER_LEVEL_ID)
		{
			//上下标识及长度
			frame_id_length = 3 + sizeof(querrytimeperiod_upmessage.querrytimeperiod_up_content) - 8 + hour_groups * 12 * WATERLEVEL_SINGLE_DATALEN;
			printf("frame_id_length is %d\r\n", frame_id_length);
			querrytimeperiod_upmessage.message_header.frame_id_len = (frame_id_length >> 8) | (frame_id_length << 8);
			GetAddress(&start_date, WATERLEVEL_TYPE, (int *)&read_address);
			printf("waterlevel sin: read_address is %08x\r\n", read_address);
			data_length = (hour_groups) * 12 * WATERLEVEL_SINGLE_DATALEN;
			querrytimeperiod_upmessage.querrytimeperiod_up_content.start_addr = read_address;
			querrytimeperiod_upmessage.querrytimeperiod_up_content.data_length = data_length;
			printf("sn is %04x\r\n",sn);
			SendTimePerioadData_upframe(&querrytimeperiod_upmessage,serial);
		}
		else
		{
			printf("no element id\r\n");
		}
			
	}
	else
	{
		integer_total_pkts = hour_groups / TIMEPERIOD_CONTENT_HOURGROUPS; //总包数
		remainder_hour_grops = hour_groups % TIMEPERIOD_CONTENT_HOURGROUPS; //剩余小时数据组数
		if(remainder_hour_grops != 0)
		{
			remainder_pkts = 1;  //最后一个包
		}
		total_pkts = integer_total_pkts + remainder_pkts;
		printf("total pkts is %d\r\n", total_pkts);
		//总包数及序号
		querrytimeperiod_upmessage.total_pkts_sn[0] = (unsigned char)(total_pkts >> 4);
		querrytimeperiod_upmessage.total_pkts_sn[1] = (unsigned char)((total_pkts << 4) & 0x00f0);
		querrytimeperiod_upmessage.total_pkts_sn[2] = 0x01;
		if(element_id == CUMULA_PRECIPIT_ID)
		{	
			
			GetAddress(&start_date, RAINFALL_TYPE, (int *)&read_address);
			//上下标识和长度
			frame_id_length = 3 + sizeof(querrytimeperiod_upmessage.querrytimeperiod_up_content) - 8 + TIMEPERIOD_CONTENT_HOURGROUPS * 12 * RAINFALL_SINGLE_DATALEN;
			querrytimeperiod_upmessage.message_header.frame_id_len = (frame_id_length >> 8) | (frame_id_length << 8);

			
			while(i <=  integer_total_pkts)
			{
				data_length = TIMEPERIOD_CONTENT_HOURGROUPS * 12 * RAINFALL_SINGLE_DATALEN;
				read_address += ((i - 1) * 12 * 12 * RAINFALL_SINGLE_DATALEN);
				printf("rainfall multi %d: read_address is %08x\r\n",i , read_address);
				querrytimeperiod_upmessage.querrytimeperiod_up_content.start_addr = read_address;
				querrytimeperiod_upmessage.querrytimeperiod_up_content.data_length = data_length;
				//包序号
				querrytimeperiod_upmessage.total_pkts_sn[1] &= 0xf0;
				querrytimeperiod_upmessage.total_pkts_sn[1] |= (unsigned char)((i >> 8) & 0x0f);
				querrytimeperiod_upmessage.total_pkts_sn[2] = (unsigned char)i;
				//发送数据
				SendTimePerioadData_upframe(&querrytimeperiod_upmessage, serial);
				i++;			}
			if(remainder_pkts == 1)
			{
				data_length = remainder_hour_grops * 12 * RAINFALL_SINGLE_DATALEN;
				read_address += 12 * 12 * RAINFALL_SINGLE_DATALEN;
				querrytimeperiod_upmessage.querrytimeperiod_up_content.start_addr = read_address;
				querrytimeperiod_upmessage.querrytimeperiod_up_content.data_length = data_length;
		
				//上下标识及长度
				frame_id_length = 3 + sizeof(querrytimeperiod_upmessage.querrytimeperiod_up_content) - 8 + data_length;
				querrytimeperiod_upmessage.message_header.frame_id_len = (frame_id_length >> 8) | (frame_id_length << 8);

				//包序号
				querrytimeperiod_upmessage.total_pkts_sn[1] &= 0xf0;
				querrytimeperiod_upmessage.total_pkts_sn[1] |= (unsigned char)((i >> 8) & 0x0f);
				querrytimeperiod_upmessage.total_pkts_sn[2] = (unsigned char)i;
				//发送数据
				SendTimePerioadData_upframe(&querrytimeperiod_upmessage, serial);
			}
		}
		else if(element_id == WATER_LEVEL_ID)
		{
			GetAddress(&start_date, WATERLEVEL_TYPE, (int *)&read_address);
			printf("waterlevel multi: start read_address is %08x\r\n", read_address);

			//上下标识和长度
			frame_id_length = 3 + sizeof(querrytimeperiod_upmessage.querrytimeperiod_up_content) - 8 + TIMEPERIOD_CONTENT_HOURGROUPS * 12 * RAINFALL_SINGLE_DATALEN;
			printf("frame_id_length is %d\r\n", frame_id_length);
			querrytimeperiod_upmessage.message_header.frame_id_len = (frame_id_length >> 8) | (frame_id_length << 8);
			while (i <= integer_total_pkts)
			{
				data_length = TIMEPERIOD_CONTENT_HOURGROUPS * 12 * WATERLEVEL_SINGLE_DATALEN;
				read_address +=  ((i - 1) * 12 * 12 * WATERLEVEL_SINGLE_DATALEN);
				printf("waterlevel multi %d: read_address is %08x\r\n", i ,read_address);
				querrytimeperiod_upmessage.querrytimeperiod_up_content.start_addr = read_address;
				querrytimeperiod_upmessage.querrytimeperiod_up_content.data_length = data_length;
				
				//包序号
				querrytimeperiod_upmessage.total_pkts_sn[1] &= 0xf0;
				querrytimeperiod_upmessage.total_pkts_sn[1] |= (unsigned char)((i >> 8) & 0x0f);
				querrytimeperiod_upmessage.total_pkts_sn[2] = (unsigned char)i;
				//发送数据
				SendTimePerioadData_upframe(&querrytimeperiod_upmessage, serial);
				i++;
			}
			if(remainder_pkts == 1)
			{
				data_length = remainder_hour_grops * 12 * WATERLEVEL_SINGLE_DATALEN;
				read_address += 12 * 12 * WATERLEVEL_SINGLE_DATALEN;
				querrytimeperiod_upmessage.querrytimeperiod_up_content.start_addr = read_address;
				querrytimeperiod_upmessage.querrytimeperiod_up_content.data_length = data_length;
				printf("remainder_hour_groups is %d, final data_length is %d\r\n", remainder_hour_grops, data_length);

				//上下标识及长度
				frame_id_length = 3 + sizeof(querrytimeperiod_upmessage.querrytimeperiod_up_content) - 8 + data_length;
				printf("frame_id_length is %d\r\n", frame_id_length);
				querrytimeperiod_upmessage.message_header.frame_id_len = (frame_id_length >> 8) | (frame_id_length << 8);

				//包序号
				querrytimeperiod_upmessage.total_pkts_sn[1] &= 0xf0;
				querrytimeperiod_upmessage.total_pkts_sn[1] |= (unsigned char)((i >> 8) & 0x0f);
				querrytimeperiod_upmessage.total_pkts_sn[2] = (unsigned char)i;
				//发送数据
				SendTimePerioadData_upframe(&querrytimeperiod_upmessage, serial);
			}
			
		}
		else
		{
			printf("no element id\r\n");
		}
		

	}
}


void print_struct(void *strp, int size)
{
	int i = 0;
	char *print = (char *)strp;
	
	for(i = 0; i < size; i++)
	{
		printf("%02x\t",print[i]);
	}
}



int FloatoHex(const int total_digits,const int decimals,float data)  //total_digits没用上
{
	int integer = 0;
	int i = 0;
	int j = 0;
	int dest_data = 0;
	int single_dat = 0; //依次取得一个数的每一位	

	for (; i < decimals; i++)
	{
		data *= 10;
	}
	integer = (int)data;
	while (integer > 0)
	{
		single_dat = integer % 10;
		for (i = 0; i < j; i++)
		{
			single_dat *= 16;
		}
		dest_data += single_dat;
		integer /= 10;
		j++;
	}
	return dest_data;

}


float HextoFloat(int total_bytes, int decimals, unsigned char *src_buff)
{
	int i = 0;
	int j = 0;
	int temp = 0;
	unsigned char high_ans = 0;
	unsigned char low_ans = 0;
	float ans = 0;

	for (i = 0; i < total_bytes; i++)
	{
		high_ans = (src_buff[i] >> 4) & 0x0f;
		low_ans = src_buff[i] & 0x0f;
		temp = (high_ans * 10 + low_ans) + temp * 100;
	}
	ans = (float)temp;
	for (j = 0; j < decimals; j++)
	{
		ans /= (float)10.0;
	}
	return ans;
}

int GetBCDSendata(const unsigned short element_id, const float src_data)
{
	int total_digits = 0;
	int decimals = 0;
	int des_data = 0;

	total_digits = (int)((element_id >> 3) & 0x001F);
	decimals = (int)(element_id & 0x0007);
	//printf("total_digits is %d,decimal is %d\r\n", total_digits, decimals);
	des_data = FloatoHex(total_digits, decimals, src_data);
	return des_data;

}

float BCDSendataToFloat(const unsigned short element_id, unsigned char *src_buff)
{
	int totol_bytes = (int)((element_id >> 3) & 0x001f);
	int decimals = (int)(element_id & 0x0007);
	float ans = 0;
	
	ans = HextoFloat(totol_bytes,decimals, src_buff);
	return ans;
}

void TimeDefTodate(Time_Def *time, date_t *date)
{
	date->year = BCDToInteger(time->year);
	date->month = BCDToInteger(time->month);
	date->day = BCDToInteger(time->day);
	date->hour = BCDToInteger(time->hour);
	date->minute = BCDToInteger(time->minute);

}

