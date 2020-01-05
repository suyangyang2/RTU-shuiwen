#ifndef _PROTOCOL_H
#define _PROTOCOL_H


#include "sd3078rtc.h"
#include "memory_my.h"





#define TIME_PERIOD_DATA_LEN 576
#define TIME_PERIOD_MAX_LEN  800
#define TIME_PERIOD_MESSAGE_MAX_LEN 1000

#define RECEIVE_ACK 0
#define	RECEIVE_NAK 1
#define RECEIVE_ERRO -1
#define RECEIVE_DELAY 500
#define RAINFALL_STEP 0.5
#define TIMEPERIOD_CONTENT_HOURGROUPS 12


#define REMOTE_ADDR_DATA_LEN   5
#define MATCH_OK 				0
#define NO_MATCH_FUNID  1
#define NO_MATCH_SERIAL 2
#define PLUS_MESSAGE_FUNID 		0X33
#define HOUR_MESSAGE_FUNID 		0X34
#define QUERY_REALDATA_FUNID  0X37
#define QUERY_TIME_PERIOD_FUNID 0X38
#define SET_REAL_TIME			0X4A
#define SET_PARAMETER_FUNID     0X41
#define QUERY_PARAMETER_FUNID	0X40
#define SET_ENTER_STOP_MODE		0X11  //进行调试
#define ENTER_RESPONSE_MODE 	0X01
#define ENTER_NORMAL_MODE		  0X02


#define REMOTE_ADDR_ID		0XF1F1

#define FIVE_MIN_PRECIPIT_ID  	 		0X2219
#define CUMULA_PRECIPIT_ID     			0X2619
#define WATER_LEVEL_ID				 	0X3923
#define VOLTAGE_ID						0X3812
#define HOUR_FIVE_MIN_PRECIPIT_ID       0XF460 
#define HOUR_FIVE_WATERLEVEL_ID 		0XF5C0
#define SERIAL_1		1
#define SERIAL_2		2
#define RS232_3			3


//parameter guide id
#define CENTER_ADDRESS_GUIDE_ID 0X0120
#define REMOTE_ADDRESS_GUIDE_ID 0X0228
#define PASSWOD_GUIDE_ID	 0X0310
#define CENTER_1_MAIN_CHANNEL_GUIDE_ID	 0X0450
#define CENTER_1_SPARE_CHANNEL_GUIDE_ID  0X0550
#define CENTER_2_MAIN_CHANNEL_GUIDE_ID	 0X0650
#define CENTER_2_SPARE_CHANNEL_GUIDE_ID  0X0750
#define CENTER_3_MAIN_CHANNEL_GUIDE_ID	 0X0850
#define CENTER_3_SPARE_CHANNEL_GUIDE_ID  0X0950
#define CENTER_4_MAIN_CHANNEL_GUIDE_ID	 0X0A50
#define CENTER_4_SPARE_CHANNEL_GUIDE_ID  0X0B50
#define WORK_STYLE_GUIDE_ID				 0X0C08
#define REMOTE_COLLECT_FACTOR_GUIDE_ID   0X0D20
#define TIMING_TIME_INTERVAL_GUIDE_ID    0X2008
#define PLUS_INTERVAL_GUIDE_ID			 0X2108
#define RAINFALL_DAY_STARTTIME_GUIDE_ID  0X2208
#define COLLECT_INTERVAL_GUIDE_ID 		 0X2310
#define WATERLEVEL_SAVE_INTERVAL_GUIDE_ID 0X2408
#define RAINFALL_RESOLUTION_GUIDE_ID      0X2509
#define WATERLEVEL_RESOLUTION_GUIDE_ID    0X2609
#define RAINFALL_PLUS_THRESHOLD_GUIDE_ID  0X2708
#define WATERLEVEL_PLUS_GUIDE_ID     0X3812
#define WATERLEVEL_PLUS_UP_THRESHOLD_GUIDE_ID 0X4012
#define WATERLEVEL_PLUS_DOWN_THRESHOLD_GUIDE_ID 0X4112
#define FLOW_PLUS_THRESHOLD_GUIDE_ID 0X421B






#pragma pack(1)

typedef struct time_step_id_s
{
	unsigned char time_step_leader_id;
	unsigned char data_definition;
	unsigned char step_day;
	unsigned char step_hour;
	unsigned char step_minute;
}time_step_id_t;

typedef struct message_header_s
{
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_DATA_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;         
	unsigned char content_start_id;
}message_header_t;

typedef struct observ_time_s{
    unsigned short id;
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
}observ_time_t;


typedef struct pkt_time_data_s{
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
        
}pkt_time_data_t;

typedef struct timeperiod_s
{
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
		
}timeperiod_t;

typedef struct  upmessage_header_content_s
{
	unsigned short sn;
	pkt_time_data_t pkt_time;
	unsigned short remote_addr_id;
	unsigned char remote_addr[REMOTE_ADDR_DATA_LEN];
	unsigned char class_id;
	observ_time_t observ_time;
}upmessage_header_content_t;



typedef struct plus_message_content_s
{
	unsigned short sn;
	pkt_time_data_t pkt_time;
	unsigned short remote_addr_id;
  unsigned char remote_addr[REMOTE_ADDR_DATA_LEN];
  unsigned char class_id;
	observ_time_t observ_time;
	unsigned short five_min_precipit_id;
	unsigned char five_min_precipit_data[3];
	unsigned short water_level_id;
	unsigned char water_level_data[4];
	unsigned short cumula_precipit_id;
	unsigned char cumula_precipit_data[3];
	unsigned short voltage_id;
	unsigned char voltage_data[2];
}plus_message_content_t;

typedef struct hour_message_content_s
{
	unsigned short sn;
	pkt_time_data_t pkt_time;
	unsigned short remote_addr_id;
	unsigned char remote_addr[REMOTE_ADDR_DATA_LEN];
	unsigned char class_id;
	observ_time_t observ_time;
	unsigned short hour_five_precipit_id;
	unsigned char  hour_five_prcipit_data[12];
	unsigned short cumula_precipit_id;
	unsigned char cumula_precipit_data[3];
	unsigned short hour_five_waterlevel_id;    //水文所那边之前测的只有顺时水位，这里以通信协议为准
	unsigned short hour_five_waterlevel_data[12];
	unsigned short water_level_id;
	unsigned char water_level_data[4];
	unsigned short voltage_id;
	unsigned char voltage_data[2];

}hour_message_content_t;


//加报报文
typedef struct plus_message_s 
{
	message_header_t header;
	plus_message_content_t  plus_message_content;
	unsigned char content_end_id;
	unsigned short crc;
}plus_message_t;

//小时报报文
typedef struct hour_message_s
{
	message_header_t header;  
	hour_message_content_t hour_message_content;	
	unsigned char content_end_id;
	unsigned short crc;
}hour_message_t;

//下报报文设置遥测站时钟
typedef struct calibration_time_downmsssage_s
{
	message_header_t header;
	unsigned short sn;
	pkt_time_data_t pkt_time;         //作为校准时间
	unsigned char content_end_id;
	unsigned short crc;
}calibration_time_downmsssage_t;

//加报、小时报 下报报文
typedef struct down_message_s
{
	message_header_t header;
	unsigned short sn;
	pkt_time_data_t pkt_time;
}down_message_t;

//上报报文设置遥测站时钟
typedef struct calibration_time_upmsssage_s
{
	message_header_t header;
	unsigned short sn;
	pkt_time_data_t pkt_time;         //作为校准时间
	unsigned short remote_addr_id;
 	unsigned char remote_addr[REMOTE_ADDR_DATA_LEN];
	unsigned char content_end_id;
	unsigned short crc;
}calibration_time_upmsssage_t;


typedef struct queryrealdata_up_content_s
{
	unsigned short sn;
	pkt_time_data_t pkt_time_data;
	unsigned short remote_addr_id;
  unsigned char remote_addr[REMOTE_ADDR_DATA_LEN];
  unsigned char class_id;
	observ_time_t observ_time;
	unsigned short precipit_id;
	unsigned char precipit_data[3];            //待完善
	unsigned short cumula_precipit_id;
	unsigned char cumula_precipit_data[3];
	unsigned short water_level_id;
	unsigned char water_level_data[4];
	unsigned short voltage_id;
	unsigned char voltage_data[2];

	
}queryrealdata_up_content_t;


typedef struct querrytimeperiod_down_content_s
{
	unsigned short sn;
	pkt_time_data_t pkt_time_data;
	timeperiod_t start_time;
	timeperiod_t end_time;
	unsigned char  time_period_id[5];
	unsigned short precipit_id;      //待完善
	
}querrytimeperiod_down_content_t;

typedef struct querrytimeperiod_up_content_s
{
	unsigned short sn;
	pkt_time_data_t pkt_time_data;
	unsigned short remote_addr_id;
 	 unsigned char remote_addr[REMOTE_ADDR_DATA_LEN];
 	unsigned char class_id;
	observ_time_t observ_time;
	unsigned char time_period_id[5];
	unsigned short element_id;
	unsigned int start_addr;   //FLASH中的地址
	unsigned int data_length;  //FLASH中的数据长度

}querrytimeperiod_up_content_t;

typedef struct queryrealdata_downmessage_s
{
	message_header_t message_header;
	unsigned short sn;
	pkt_time_data_t pkt_time_data;
	unsigned char content_end_id;
	unsigned short crc;

}queryrealdata_downmessage_t;
	
typedef struct queryrealdata_upmessage_s
{
		message_header_t message_header;
		queryrealdata_up_content_t queryrealdata_content;
		unsigned char content_end_id;
		unsigned short crc;

}queryrealdata_upmessage_t;

typedef struct querrytimeperiod_downmessage_s
{
	message_header_t message_header;
	querrytimeperiod_down_content_t querrytimeperiod_down_content;
	unsigned char content_end_id;
	unsigned short crc;

}querrytimeperiod_downmessage_t;

typedef struct querrytimeperiod_upmessage_s
{
	message_header_t message_header;
	unsigned char total_pkts_sn[3];
	querrytimeperiod_up_content_t querrytimeperiod_up_content;
	unsigned char content_end_id;
	unsigned short crc;

}querrytimeperiod_upmessage_t;

typedef struct setparameter_content_s
{
	unsigned short sn;
	pkt_time_data_t pkt_time;
	unsigned short center_addr_guide_id;  //0x01
	unsigned char  center_addr[4];
	unsigned short remote_addr_guide_id; //0x02
	unsigned char  rtu_remote_addr[5];
	unsigned short  password_guide_id;  //0x03
	unsigned char  password[2];
	unsigned short  center_1_main_channel_guide_id; //0x04
	unsigned char  center_1_main_channel[10];
	unsigned short  center_1_spare_channel_guide_id; //0x05
	unsigned char  center_1_spare_channel[10];
	unsigned short  center_2_main_channel_guide_id; //0x06
	unsigned char  center_2_main_channel[10];
	unsigned short  center_2_spare_channel_guide_id;  //0x07
	unsigned char  center_2_spare_channel[10];
	unsigned short  center_3_main_channel_guide_id; //0x08
	unsigned char  center_3_main_channel[10];
	unsigned short  center_3_spare_channel_guide_id;  //0x09
	unsigned char  center_3_spare_channel[10];
	unsigned short center_4_main_channel_guide_id; //0x0A
	unsigned char  center_4_main_channel[10];
	unsigned short center_4_spare_channel_guide_id;  //0x0B
	unsigned char  center_4_spare_channel[10];
	unsigned short work_style_guide_id;   //0x0c
	unsigned char  work_style; 
	unsigned short remote_collect_factor_guide_id; //0x0d
	unsigned char  remote_collect_factor[4];
	unsigned short time_interval_of_timing_message_id;  //0x20
	unsigned char  time_interval_of_timing_message; //定时报时间间隔
	unsigned short time_interval_of_plus_message_guide_id;  //0x21
	unsigned char  time_interval_of_plus_message;
	unsigned short rainfall_day_start_time_guide_id; // 0x22
	unsigned char  rainfall_day_start_time;
	unsigned short sampling_interval_guide_id; //0x23
	unsigned char  sampling_interval[2];
	unsigned short waterlevel_save_interval_guide_id; //0x24
	unsigned char  waterlevel_save_interval;
	unsigned short rainfall_resolution_guide_id; //0x25
	unsigned char  rainfall_resolution;
	unsigned short waterlevel_resolution_guide_id; //0x26
	unsigned char  waterlevel_resolution;
	unsigned short threshold_rainfall_value_of_plusemessage_guide_id; //0x27
	unsigned char  threshold_rainfall_value_of_plusemessage;
	unsigned short waterlevel_plusmessage_1_guide_id;  //0x38
	unsigned char  waterlevel_plusmessage_1[2];
	unsigned short above_threshold_waterlevel_value_of_plusmeessag_guide_id; //0x40
	unsigned char  above_threshold_waterlevel_value_of_plusmeessage[2];
	unsigned short below_threshold_waterlevel_value_of_plusmessage_guide_id; //0x41
	unsigned char  below_threshold_waterlevel_value_of_plusmessage[2];
	unsigned short flow_threshold_plusmessage_guide_id; //0x42
	unsigned char  flow_threshold_plusmessage[3];

}setparameter_content_t;

typedef struct setparameter_downmessage_s
{
	message_header_t message_header;
	setparameter_content_t content;
	unsigned char end_id;
	unsigned short crc;
}setparameter_downmessage_t;

typedef struct querryparameter_upmessage_s
{
	message_header_t message_header;
	setparameter_content_t content;
	unsigned char end_id;
	unsigned short crc;
}querryparameter_upmessage_t;





#pragma pack()


/*
********************************************************************************************
* 函数名：GetByteLength
* 返回值：字节的长度
* 参  数：start_time:起始时间地址，end_time:终止时间地址,data_type:数据类型，雨量水位，address_start:得到的起始地址，address_end:得到的终止地址
* 描  述：
*
*********************************************************************************************
*/
static int GetByteLength(const timeperiod_t *start_time, const timeperiod_t *end_time, const unsigned char data_type, int *address_start, int *address_end);

/*
********************************************************************************************
* 函数名：BCDToInteger
* 返回值：十进制数据
* 参  数：src:以BCD格式的一个字节数据
* 描  述：将BCD格式数据转换为十进制数据
*
*********************************************************************************************
*/
static unsigned char BCDToInteger(unsigned char src);

/*
********************************************************************************************
* 函数名：JudegeBCD
* 返回值：0 是BCD， -1不是BCD
* 参  数：src:字符串， len: 字符串长度
* 描  述：判断一组字符串数据是否是BCD码格式的，不是返回-1，
*
*********************************************************************************************
*/
signed char JudegeBCD(unsigned char *src, int len);

/*
********************************************************************************************
* 函数名：ReadData_ByTimePeriod
* 返回值：读到FLASH字节数据个数
* 参  数：start_time: 起始时间， end_time:终止时间, addr_start:起始地址， addr_end：终止地址
					data_type : 数据类型，雨量和水位
* 描  述：
*
*********************************************************************************************
*/
int ReadData_ByTimePeriod(const timeperiod_t *start_time,const timeperiod_t *end_time, const unsigned char data_type, unsigned char *data);


/*
********************************************************************************************
* 函数名：SaveDayStartRainfall
* 返回值：无
* 参  数：无
* 描  述：将当前雨量数据存储在特定存储位置，便于实现日起始数据记忆
*
*********************************************************************************************
*/
void SaveDayStartRainfall(void);

/*
********************************************************************************************
* 函数名：ReadDayStartRainfall
* 返回值：从FLASH中读取的日起始数据
* 参  数：无
* 描  述：从FLASH中读取之前存储的日起始数据
*
*********************************************************************************************
*/
unsigned int ReadDayStartRainfall(void);

/*
********************************************************************************************
* 函数名：HourRainFallSaveData_To_FiveMinuteRelativeValue
* 返回值：无
* 参  数：src_buff：原雨量存储的一个小时3*13字节的实时雨量数据，des_buff:得到的雨量差值，雨量12个字节
* 描  述：将存储的一个小时的13组数据，转化为12字节 F460类型雨量差值数据
*
*********************************************************************************************
*/
static void HourRainFallSaveData_To_FiveMinuteRelativeValue(const unsigned char *src_buff, unsigned char *des_buff);


/*
********************************************************************************************
* 函数名：WaterLevelBCDString_To_FloatValue
* 返回值：float类型数据
* 参  数：src_buff:一个小时内的13组数据buff, i:组号
* 描  述：从存储的13组，数据中得到某一组返回的float类型数据
*
*********************************************************************************************
*/
static float WaterLevelBCDString_To_FloatValue(const unsigned char *src_buff, const int i);


/*
********************************************************************************************
* 函数名：HourWaterLevelSaveData_To_Value
* 返回值：无
* 参  数：src_buff：原水位存储的一个小时4*13字节的实时水位数据，des_buff:得到的水位差值，水位24个字节
* 描  述：将存储的一个小时的13组数据，转化为24字节 F5C0类型雨量差值数据
*
*********************************************************************************************
*/
static void HourWaterLevelSaveData_To_Value(const unsigned char *src_buff, unsigned short *des_buff);
/*
********************************************************************************************
* 函数名：test_SendTimePeriodData
* 返回值：无
* 参  数：无
* 描  述：模拟一个起始时间、一个终止时间来，分辨提取每一天的存储的某一要素的数据，然后以每包（每天数据）进行发送；
*-
*********************************************************************************************
*/
void test_SendTimePeriodData(querrytimeperiod_downmessage_t *querrytimeperiod_downmessage, unsigned char serial);



/*
********************************************************************************************
* 函数名：print_struct
* 返回值：无
* 参  数：strp:需要打印的地址，size:需要打印变量内存空间的长度
* 描  述：加报报文数据初始化
*
*********************************************************************************************
*/
void print_struct(void *strp, int size);

/*
********************************************************************************************
* 函数名：plus_message_init
* 返回值：无
* 参  数：0-成功 
* 描  述：加报报文数据初始化
*
*********************************************************************************************
*/
void plus_message_init(void); 

/*
********************************************************************************************
* 函数名：hour_message_init
* 返回值：无
* 参  数：0-成功 
* 描  述：小时报文数据初始化
*
*********************************************************************************************
*/
void hour_message_init(void);

/*
********************************************************************************************
* 函数名：QueryRealData_Init
* 返回值：无
* 参  数：无
* 描  述：实时报文数据初始化
*
*********************************************************************************************
*/
void QueryRealData_Init(void);

/*
********************************************************************************************
* 函数名：QueryTimePeriodData_Init
* 返回值：无
* 参  数：querrytimeperiod_upmessage：下报文数据结构的地址
* 描  述：时间段数据报文初始化
*
*********************************************************************************************
*/
void QueryTimePeriodData_Init(querrytimeperiod_upmessage_t *querrytimeperiod_upmessage, unsigned short elemen_id);



/*
********************************************************************************************
* 函数名：QueryParameters_Init
* 返回值：无
* 参  数：querryparameter_upmessage:查询参数报文数据指针，RTU_operation_parameters读到的参数数据
* 描  述：查询RTU的相关参数
*
*********************************************************************************************
*/
void QueryParameters_Init(querryparameter_upmessage_t *querryparameter_upmessage, RTU_operation_parameters_t *rtu_operation_parameters);


/*
********************************************************************************************
* 函数名：SetParameters
* 返回值：无
* 参  数：setparameter_downmessage：修改参数下报文数据结构的地址
* 描  述：设置RTU的参数
*
*********************************************************************************************
*/
void SetParameters(setparameter_downmessage_t *setparameter_downmessage);


/*
********************************************************************************************
* 函数名：JudgeReceiveTimePeriod
* 返回值：下报报文结束符
* 参  数：无
* 描  述：分析串口2（gprs)缓存数据返回结束符
*
*********************************************************************************************
*/
int JudgeReceiveTimePeriod(void);

/*
********************************************************************************************
* 函数名：GetDownFrameFunId
* 返回值：返回的功能码
* 参  数：
* 描  述：通过报文数据帧获取
*
*********************************************************************************************
*/
unsigned char GetDownFrameFunId(const unsigned char serial);

/*
********************************************************************************************
* 函数名：ExecuteFunctionByFunId
* 返回值：0-成功 其它-失败
* 参  数：fun_id:功能码，serial:串口
* 描  述：通过某一串口下报得到的功能码，执行相应地命令
*
*********************************************************************************************
*/
int ExecuteFunctionByFunId(const unsigned char fun_id, const unsigned char serial);

/*
********************************************************************************************
* 函数名：ResponseUpMessage
* 返回值：0-成功 其它-失败
* 参  数：fun_id:功能码，serial:串口
* 描  述：上报反馈
*
*********************************************************************************************
*/
int ResponseUpMessage(const unsigned char fun_id, const unsigned char serial);

/*
********************************************************************************************
* 函数名：Calibration_time_UpMessage_init
* 返回值：无
* 参  数：calibration_time_upmsssage：设置时间上报报文指针
* 描  述：将设置时间的上报报文中数据初始化赋值
*
*********************************************************************************************
*/
void Calibration_time_UpMessage_init(calibration_time_upmsssage_t *calibration_time_upmsssage);


/*
********************************************************************************************
* 函数名：GetBCDSendata
* 返回值：得到的合适的符合报文格式的数据
* 参  数：element_id：要素标识符，如5分钟降水量为0X2219；src_data:输入数据
* 描  述：
*
*********************************************************************************************
*/
int GetBCDSendata(const unsigned short element_id, const float src_data);

/*
********************************************************************************************
* 函数名：SendataToFloat
* 返回值：得到的float类型数据
* 参  数：element_id：要素标识符，如5分钟降水量为0X2219；src_buff:存储的buff
* 描  述：将报文格式BCD编码格式的数据转换为float类型数据
*
*********************************************************************************************
*/
float BCDSendataToFloat(const unsigned short element_id, unsigned char *src_buff);




void TimeDefTodate(Time_Def *time, date_t *date);

/*
********************************************************************************************
* 函数名：FloatoHex
* 返回值：实际上发送数据的整合
* 参  数：total_digits:数据位数，如26 19 00 13 06 ,为6位数据，decimals:数据中小数点后面数据个数，26 19即为1，data :为原真实数据，
* 描  述：将一个真实数据按照规定的格式给发送出去，如数据为水位3923,数据为27.3，则发送为00 02 73 00 
*
*********************************************************************************************
*/
int FloatoHex(const int total_digits,const int decimals,float data);


/*
********************************************************************************************
* 函数名：HextoFloat
* 返回值：得到的float类型数据
* 参  数：total_byte:字节数，decimal:小数位数，src_buff:BCD格式字符串
* 描  述：将报文格式BCD编码格式的数据转换为float类型数据
*
*********************************************************************************************
*/
float HextoFloat(int total_bytes, int decimals, unsigned char *src_buff);

extern plus_message_t g_plus_message;
extern hour_message_t g_hour_message;
extern unsigned char g_plus_message_flag;

#endif
