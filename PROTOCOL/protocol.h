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
#define SET_ENTER_STOP_MODE		0X11  //���е���
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
	unsigned short hour_five_waterlevel_id;    //ˮ�����Ǳ�֮ǰ���ֻ��˳ʱˮλ��������ͨ��Э��Ϊ׼
	unsigned short hour_five_waterlevel_data[12];
	unsigned short water_level_id;
	unsigned char water_level_data[4];
	unsigned short voltage_id;
	unsigned char voltage_data[2];

}hour_message_content_t;


//�ӱ�����
typedef struct plus_message_s 
{
	message_header_t header;
	plus_message_content_t  plus_message_content;
	unsigned char content_end_id;
	unsigned short crc;
}plus_message_t;

//Сʱ������
typedef struct hour_message_s
{
	message_header_t header;  
	hour_message_content_t hour_message_content;	
	unsigned char content_end_id;
	unsigned short crc;
}hour_message_t;

//�±���������ң��վʱ��
typedef struct calibration_time_downmsssage_s
{
	message_header_t header;
	unsigned short sn;
	pkt_time_data_t pkt_time;         //��ΪУ׼ʱ��
	unsigned char content_end_id;
	unsigned short crc;
}calibration_time_downmsssage_t;

//�ӱ���Сʱ�� �±�����
typedef struct down_message_s
{
	message_header_t header;
	unsigned short sn;
	pkt_time_data_t pkt_time;
}down_message_t;

//�ϱ���������ң��վʱ��
typedef struct calibration_time_upmsssage_s
{
	message_header_t header;
	unsigned short sn;
	pkt_time_data_t pkt_time;         //��ΪУ׼ʱ��
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
	unsigned char precipit_data[3];            //������
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
	unsigned short precipit_id;      //������
	
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
	unsigned int start_addr;   //FLASH�еĵ�ַ
	unsigned int data_length;  //FLASH�е����ݳ���

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
	unsigned char  time_interval_of_timing_message; //��ʱ��ʱ����
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
* ��������GetByteLength
* ����ֵ���ֽڵĳ���
* ��  ����start_time:��ʼʱ���ַ��end_time:��ֹʱ���ַ,data_type:�������ͣ�����ˮλ��address_start:�õ�����ʼ��ַ��address_end:�õ�����ֹ��ַ
* ��  ����
*
*********************************************************************************************
*/
static int GetByteLength(const timeperiod_t *start_time, const timeperiod_t *end_time, const unsigned char data_type, int *address_start, int *address_end);

/*
********************************************************************************************
* ��������BCDToInteger
* ����ֵ��ʮ��������
* ��  ����src:��BCD��ʽ��һ���ֽ�����
* ��  ������BCD��ʽ����ת��Ϊʮ��������
*
*********************************************************************************************
*/
static unsigned char BCDToInteger(unsigned char src);

/*
********************************************************************************************
* ��������JudegeBCD
* ����ֵ��0 ��BCD�� -1����BCD
* ��  ����src:�ַ����� len: �ַ�������
* ��  �����ж�һ���ַ��������Ƿ���BCD���ʽ�ģ����Ƿ���-1��
*
*********************************************************************************************
*/
signed char JudegeBCD(unsigned char *src, int len);

/*
********************************************************************************************
* ��������ReadData_ByTimePeriod
* ����ֵ������FLASH�ֽ����ݸ���
* ��  ����start_time: ��ʼʱ�䣬 end_time:��ֹʱ��, addr_start:��ʼ��ַ�� addr_end����ֹ��ַ
					data_type : �������ͣ�������ˮλ
* ��  ����
*
*********************************************************************************************
*/
int ReadData_ByTimePeriod(const timeperiod_t *start_time,const timeperiod_t *end_time, const unsigned char data_type, unsigned char *data);


/*
********************************************************************************************
* ��������SaveDayStartRainfall
* ����ֵ����
* ��  ������
* ��  ��������ǰ�������ݴ洢���ض��洢λ�ã�����ʵ������ʼ���ݼ���
*
*********************************************************************************************
*/
void SaveDayStartRainfall(void);

/*
********************************************************************************************
* ��������ReadDayStartRainfall
* ����ֵ����FLASH�ж�ȡ������ʼ����
* ��  ������
* ��  ������FLASH�ж�ȡ֮ǰ�洢������ʼ����
*
*********************************************************************************************
*/
unsigned int ReadDayStartRainfall(void);

/*
********************************************************************************************
* ��������HourRainFallSaveData_To_FiveMinuteRelativeValue
* ����ֵ����
* ��  ����src_buff��ԭ�����洢��һ��Сʱ3*13�ֽڵ�ʵʱ�������ݣ�des_buff:�õ���������ֵ������12���ֽ�
* ��  �������洢��һ��Сʱ��13�����ݣ�ת��Ϊ12�ֽ� F460����������ֵ����
*
*********************************************************************************************
*/
static void HourRainFallSaveData_To_FiveMinuteRelativeValue(const unsigned char *src_buff, unsigned char *des_buff);


/*
********************************************************************************************
* ��������WaterLevelBCDString_To_FloatValue
* ����ֵ��float��������
* ��  ����src_buff:һ��Сʱ�ڵ�13������buff, i:���
* ��  �����Ӵ洢��13�飬�����еõ�ĳһ�鷵�ص�float��������
*
*********************************************************************************************
*/
static float WaterLevelBCDString_To_FloatValue(const unsigned char *src_buff, const int i);


/*
********************************************************************************************
* ��������HourWaterLevelSaveData_To_Value
* ����ֵ����
* ��  ����src_buff��ԭˮλ�洢��һ��Сʱ4*13�ֽڵ�ʵʱˮλ���ݣ�des_buff:�õ���ˮλ��ֵ��ˮλ24���ֽ�
* ��  �������洢��һ��Сʱ��13�����ݣ�ת��Ϊ24�ֽ� F5C0����������ֵ����
*
*********************************************************************************************
*/
static void HourWaterLevelSaveData_To_Value(const unsigned char *src_buff, unsigned short *des_buff);
/*
********************************************************************************************
* ��������test_SendTimePeriodData
* ����ֵ����
* ��  ������
* ��  ����ģ��һ����ʼʱ�䡢һ����ֹʱ�������ֱ���ȡÿһ��Ĵ洢��ĳһҪ�ص����ݣ�Ȼ����ÿ����ÿ�����ݣ����з��ͣ�
*-
*********************************************************************************************
*/
void test_SendTimePeriodData(querrytimeperiod_downmessage_t *querrytimeperiod_downmessage, unsigned char serial);



/*
********************************************************************************************
* ��������print_struct
* ����ֵ����
* ��  ����strp:��Ҫ��ӡ�ĵ�ַ��size:��Ҫ��ӡ�����ڴ�ռ�ĳ���
* ��  �����ӱ��������ݳ�ʼ��
*
*********************************************************************************************
*/
void print_struct(void *strp, int size);

/*
********************************************************************************************
* ��������plus_message_init
* ����ֵ����
* ��  ����0-�ɹ� 
* ��  �����ӱ��������ݳ�ʼ��
*
*********************************************************************************************
*/
void plus_message_init(void); 

/*
********************************************************************************************
* ��������hour_message_init
* ����ֵ����
* ��  ����0-�ɹ� 
* ��  ����Сʱ�������ݳ�ʼ��
*
*********************************************************************************************
*/
void hour_message_init(void);

/*
********************************************************************************************
* ��������QueryRealData_Init
* ����ֵ����
* ��  ������
* ��  ����ʵʱ�������ݳ�ʼ��
*
*********************************************************************************************
*/
void QueryRealData_Init(void);

/*
********************************************************************************************
* ��������QueryTimePeriodData_Init
* ����ֵ����
* ��  ����querrytimeperiod_upmessage���±������ݽṹ�ĵ�ַ
* ��  ����ʱ������ݱ��ĳ�ʼ��
*
*********************************************************************************************
*/
void QueryTimePeriodData_Init(querrytimeperiod_upmessage_t *querrytimeperiod_upmessage, unsigned short elemen_id);



/*
********************************************************************************************
* ��������QueryParameters_Init
* ����ֵ����
* ��  ����querryparameter_upmessage:��ѯ������������ָ�룬RTU_operation_parameters�����Ĳ�������
* ��  ������ѯRTU����ز���
*
*********************************************************************************************
*/
void QueryParameters_Init(querryparameter_upmessage_t *querryparameter_upmessage, RTU_operation_parameters_t *rtu_operation_parameters);


/*
********************************************************************************************
* ��������SetParameters
* ����ֵ����
* ��  ����setparameter_downmessage���޸Ĳ����±������ݽṹ�ĵ�ַ
* ��  ��������RTU�Ĳ���
*
*********************************************************************************************
*/
void SetParameters(setparameter_downmessage_t *setparameter_downmessage);


/*
********************************************************************************************
* ��������JudgeReceiveTimePeriod
* ����ֵ���±����Ľ�����
* ��  ������
* ��  ������������2��gprs)�������ݷ��ؽ�����
*
*********************************************************************************************
*/
int JudgeReceiveTimePeriod(void);

/*
********************************************************************************************
* ��������GetDownFrameFunId
* ����ֵ�����صĹ�����
* ��  ����
* ��  ����ͨ����������֡��ȡ
*
*********************************************************************************************
*/
unsigned char GetDownFrameFunId(const unsigned char serial);

/*
********************************************************************************************
* ��������ExecuteFunctionByFunId
* ����ֵ��0-�ɹ� ����-ʧ��
* ��  ����fun_id:�����룬serial:����
* ��  ����ͨ��ĳһ�����±��õ��Ĺ����룬ִ����Ӧ������
*
*********************************************************************************************
*/
int ExecuteFunctionByFunId(const unsigned char fun_id, const unsigned char serial);

/*
********************************************************************************************
* ��������ResponseUpMessage
* ����ֵ��0-�ɹ� ����-ʧ��
* ��  ����fun_id:�����룬serial:����
* ��  �����ϱ�����
*
*********************************************************************************************
*/
int ResponseUpMessage(const unsigned char fun_id, const unsigned char serial);

/*
********************************************************************************************
* ��������Calibration_time_UpMessage_init
* ����ֵ����
* ��  ����calibration_time_upmsssage������ʱ���ϱ�����ָ��
* ��  ����������ʱ����ϱ����������ݳ�ʼ����ֵ
*
*********************************************************************************************
*/
void Calibration_time_UpMessage_init(calibration_time_upmsssage_t *calibration_time_upmsssage);


/*
********************************************************************************************
* ��������GetBCDSendata
* ����ֵ���õ��ĺ��ʵķ��ϱ��ĸ�ʽ������
* ��  ����element_id��Ҫ�ر�ʶ������5���ӽ�ˮ��Ϊ0X2219��src_data:��������
* ��  ����
*
*********************************************************************************************
*/
int GetBCDSendata(const unsigned short element_id, const float src_data);

/*
********************************************************************************************
* ��������SendataToFloat
* ����ֵ���õ���float��������
* ��  ����element_id��Ҫ�ر�ʶ������5���ӽ�ˮ��Ϊ0X2219��src_buff:�洢��buff
* ��  ���������ĸ�ʽBCD�����ʽ������ת��Ϊfloat��������
*
*********************************************************************************************
*/
float BCDSendataToFloat(const unsigned short element_id, unsigned char *src_buff);




void TimeDefTodate(Time_Def *time, date_t *date);

/*
********************************************************************************************
* ��������FloatoHex
* ����ֵ��ʵ���Ϸ������ݵ�����
* ��  ����total_digits:����λ������26 19 00 13 06 ,Ϊ6λ���ݣ�decimals:������С����������ݸ�����26 19��Ϊ1��data :Ϊԭ��ʵ���ݣ�
* ��  ������һ����ʵ���ݰ��չ涨�ĸ�ʽ�����ͳ�ȥ��������Ϊˮλ3923,����Ϊ27.3������Ϊ00 02 73 00 
*
*********************************************************************************************
*/
int FloatoHex(const int total_digits,const int decimals,float data);


/*
********************************************************************************************
* ��������HextoFloat
* ����ֵ���õ���float��������
* ��  ����total_byte:�ֽ�����decimal:С��λ����src_buff:BCD��ʽ�ַ���
* ��  ���������ĸ�ʽBCD�����ʽ������ת��Ϊfloat��������
*
*********************************************************************************************
*/
float HextoFloat(int total_bytes, int decimals, unsigned char *src_buff);

extern plus_message_t g_plus_message;
extern hour_message_t g_hour_message;
extern unsigned char g_plus_message_flag;

#endif
