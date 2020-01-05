#ifndef MEMORY_H
#define MEMORY_H

#include "sys.h"
/*
* @file  memory.h
* @brief Get the first address of the hourly rainfall data store from year, month , day and hour!
* @version 1.1
		��rainfall data length ��Ϊ1�ֽ�
* @author 
* @date 2019.8.22
*/
#pragma pack(1)

//BCD ��ʽ����
typedef struct RTU_operation_parameters_s
{
	unsigned char center_addr[4];          //************�޸ĳ���
	unsigned char rtu_remote_addr[5];  //վ��
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
	unsigned char time_interval_of_timing_message; //��ʱ��ʱ����
	unsigned char time_interval_of_plus_message;
	unsigned char rainfall_day_start_time;
	unsigned char sampling_interval[2];   //�������
	unsigned char waterlevel_save_interval;  //ˮλ���ݴ洢���
	unsigned char rainfall_resolution;		//�����Ʒֱ���
	unsigned char waterlevel_resolution;   //ˮλ�Ʒֱ���
	
	unsigned char threshold_rainfall_value_of_plusemessage;  //�����ӱ���ֵ
	unsigned char waterlevel_basevalue_1[4];
	unsigned char waterlevel_correction_value_1[3];
	unsigned char waterlevel_plusmessage_1[2];  	//�ӱ�ˮλ
	unsigned char above_threshold_waterlevel_value_of_plusmeessag[2]; //�ӱ�ˮλ������ֵ
	unsigned char below_threshold_waterlevel_value_of_plusmessage[2]; //�ӱ�ˮλ������ֵ
	unsigned char flow_threshold_plusmessage[3];
	
	
	
	
}RTU_operation_parameters_t;



#pragma pack()


#define IDR_RAINFALL_EVEN_BASE    0X000000   //ż���������洢����ַ ������ַ0X04 CF E0
#define IDR_WATERLEVLE_EVEN_BASE  0X400000
#define IDR_RAINFALL_ODD_BASE     0X800000
#define IDR_WATERLEVEL_ODD_BASE   0XC00000


#define IDR_SPECIAL_DATA_STORE_BASE 0X080000
#define RAINFALL_DAY_START_VALUE (IDR_SPECIAL_DATA_STORE_BASE + 0X00)   //����ֱ��ɾ���滻
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
* ��������GetAddress
* ����ֵ��0���ɹ� ��-1��ʧ��
* ��  ����date��ʱ�����ݣ�������ʱ��ֱ������ʮ��������ʾ�ģ�
					data_type: ������ˮλ��������
					*address :�õ�����Ӧ��ַ

* ��  ��������ͨ������ʱ�䣬�����������룬Ȼ��õ�FLASH�д洢����Ӧ��ַ
*
*********************************************************************************************
*/
int GetAddress(const date_t *date,const unsigned char data_type, int *address);

/*
********************************************************************************************
* ��������SaveDataToFlash
* ����ֵ��0�����ɹ� ��-1����ʧ��	
* ��  ����date���洢ʱ��
					data_type ��ĿǰRAINFALL_TYPE,WATERLEVEL_TYPE��������
					data����������
					data_len:���ݵĳ���
* ��  ����ͨ���洢ʱ�䡢�洢�������洢����ˮλ���ݵ�FLASH�У�
*
*********************************************************************************************
*/
int SaveDataToFlash(const date_t *date,const unsigned char data_type,const unsigned char *data,const int data_len);

/*
********************************************************************************************
* ��������ReadDataFromFlash
* ����ֵ��0�����ɹ� ��-1����ʧ��	
* ��  ����date�����FLASH�����ݵĴ洢��ʼʱ��
				  data_type ��ĿǰRAINFALL_TYPE,WATERLEVEL_TYPE��������
					data���õ�����������
					data_len:���ݵĳ���
* ��  ����ͨ���洢ʱ�䡢�洢���ͣ����õ�FLASH�еĴ洢������
*
*********************************************************************************************
*/
int ReadDataFromFlash(const date_t *date, const unsigned char data_type, unsigned char *data, const int data_len);

void SaveAndGetData_FiveMinute(unsigned char *rainfall_data, unsigned char *water_level_data);


/*
********************************************************************************************
* ��������GetMonthDays
* ����ֵ��
* ��  ����year:BCD�����һ���ֽ����ݣ�month_days�����ص�һ��ÿ���µ�����
					
* ��  ����ͨ��BCD��������ݣ��õ������ÿһ���µ�����
*********************************************************************************************
*/
void GetMonthDays(const unsigned char year, unsigned char *month_days);
void RTU_Operation_parameter_init(void);
void RTU_parameters_Writedata(RTU_operation_parameters_t *parameter);
void RTU_parameters_Readdata(RTU_operation_parameters_t *parameter);




extern RTU_operation_parameters_t g_rtu_parameters;




#endif
