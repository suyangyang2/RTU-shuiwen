#ifndef _SENSOR_WATER_H
#define _SENSOR_WATER_H

#include "sys.h"



#define OE1_GPIO_PORT		GPIOC
#define OE1_PIN			    GPIO_Pin_4
#define OE2_GPIO_PORT		GPIOC
#define OE2_PIN				GPIO_Pin_5
#define OE1					PCout(4)
#define OE2					PCout(5)


#define 	S0_GPIO_PORT   		GPIOE
#define 	S0_PIN       		GPIO_Pin_2
#define 	S1_GPIO_PORT 		GPIOE
#define 	S1_PIN				GPIO_Pin_3
#define 	S2_GPIO_PORT 		GPIOE
#define 	S2_PIN 				GPIO_Pin_4
#define 	S3_GPIO_PORT 		GPIOE
#define		S3_PIN 				GPIO_Pin_5
#define	 	S4_GPIO_PORT 		GPIOE
#define 	S4_PIN				GPIO_Pin_6
#define 	S5_GPIO_PORT 		GPIOC
#define		S5_PIN				GPIO_Pin_13
#define 	S6_GPIO_PORT    	GPIOC
#define 	S6_PIN				GPIO_Pin_14
#define 	S7_GPIO_PORT 		GPIOC
#define 	S7_PIN				GPIO_Pin_15



/*
********************************************************************************************
* 函数名：bsp_timer3_cap_init
* 返回值：
* 参  数：arr，psc两值任意，
* 描  述：定时器3的CH4输入捕获，通过中断能捕获电平变化
*
*********************************************************************************************
*/

//void bsp_timer3_cap_init(u16 arr, u16 psc);
//void RainFall_Pluse_Init(void);

void RainFall_Init(void);
void DI_GPIO_Init(void);
unsigned int GetWaterLevelData(void);
int GetGrayWaterLevelData(void);
void Sensor_Water_GPIO_Init(void);



extern volatile unsigned int g_rainfall_pluse_count;







#if 0
/**  
	SO - PE2 ; S1 - PE3 ; S2 - PE4 ; S3 - PE5 ; S4 - PE6
  S5 - PC13; S6 - PC14; S7 - PC15; S8 - PA0 ; S9 - PA1  
	S10 - PC4; S11 - PC5; S12 - PB0; S13 - PB1; 
**/

#define 	S0_GPIO_PORT   	GPIOE
#define 	S0_PIN       		GPIO_Pin_2
#define 	S1_GPIO_PORT 		GPIOE
#define 	S1_PIN					GPIO_Pin_3
#define 	S2_GPIO_PORT 		GPIOE
#define 	S2_PIN 					GPIO_Pin_4
#define 	S3_GPIO_PORT 		GPIOE
#define		S3_PIN 					GPIO_Pin_5
#define	 	S4_GPIO_PORT 		GPIOE
#define 	S4_PIN					GPIO_Pin_6
#define 	S5_GPIO_PORT 		GPIOC
#define		S5_PIN					GPIO_Pin_13
#define 	S6_GPIO_PORT   	GPIOC
#define 	S6_PIN					GPIO_Pin_14
#define 	S7_GPIO_PORT 		GPIOC
#define 	S7_PIN					GPIO_Pin_15
#define 	S8_GPIO_PORT    GPIOA
#define 	S8_PIN					GPIO_Pin_0
#define 	S9_GPIO_PORT 		GPIOA
#define 	S9_PIN					GPIO_Pin_1
#define 	S10_GPIO_PORT  	GPIOC
#define		S10_PIN					GPIO_Pin_4
#define		S11_GPIO_PORT   GPIOC
#define 	S11_PIN					GPIO_Pin_5
#define		S12_GPIO_PORT 	GPIOB
#define		S12_PIN					GPIO_Pin_0
#define 	S13_GPIO_PORT 	GPIOB
#define		S13_PIN					GPIO_Pin_1


#endif

/**
//观测时间
typedef struct observ_time_s{		
    unsigned short id;
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
}observ_time_t;

//链路维持报正文
typedef struct linkeep_s{     
	unsigned short sn;
	pkt_time_t pkt_time;
}linkeep_t;

//降水量
typedef struct precipitation_s{    
	unsigned char curr_precip_id;
	float curr_precip;
	unsigned char precipi_cumula_value_id;
	float precipi_cumula_value;
}precipitation_t;

//水位
typedef struct water_level_s{
	unsigned char curr_water_level_id;
	float water_level_value;
}water_level_t;

//电压
typedef struct voltage_s{
	unsigned char volage_id;
	float volatge;
}volage_t;

//测试报正文
typedef struct up_frame_test_content_s {
	unsigned short sn;
	pkt_time_t  pkt_time;
	unsigned short remote_addr_id;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned char class_id;
	observ_time_t observ_time;
	precipitation_t precipi;
	water_level_t water_level;   //其它要素？

	volage_t volage;
}up_frame_test_content_t;

//均匀时段水文信息报正文
typedef struct up_frame_average_time_content_s {
	unsigned short sn;
	pkt_time_t pkt_time;
	unsigned short remote_addr_id;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned char class_id;
	observ_time_t observ_time;
	unsigned char time_step_id;
	unsigned char element;   //还有其它要素

	                         //数据类型？

}up_frame_average_time_content_t;

//定时报正文
typedef struct up_frame_timed_content_s {
	unsigned short sn;
	pkt_time_t pkt_time;
	unsigned short remote_addr_id;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned char class_id;
	observ_time_t observ_time;
	unsigned char elenment;
	float date;    //前一个要素的数据

}up_frame_timed_content_t;

//遥测站加报报正文
typedef struct up_frame_plus_content_s{
	unsigned short sn;
	pkt_time_t pkt_time;
	unsigned short remote_addr_id;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned char class_id;
	observ_time_t observ_time;
	unsigned char triger_element;    //激励要素
	float triger_element_value;

	volage_t volage;
}up_frame_plus_content_t;

//遥测站人工置数报正文
typedef struct up_frame_manual_set_content_s {
	unsigned short sn;
	pkt_time_t pkt_time;
	unsigned char manual_set_id;	//待修改
	unsigned char date[10];


}up_frame_manual_set_content_t;




//链路维持报
typedef struct up_frame_linkeep_s{             
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;         
	unsigned char content_start_id;
  unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	linkeep_t linkeep;
	unsigned char content_end_id;
	unsigned short crc;
}up_frame_linkeep_t;

//测试上报文
typedef struct up_frame_test_s{
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;          
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	up_frame_test_content_t up_frame_test_content;
	unsigned char content_end_id;
	unsigned short crc;

}up_frame_test_t;


//测试报下报文
typedef struct down_frame_test_s {
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;         
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	unsigned char sn;
	pkt_time_t pkt_time;
	unsigned char content_end_id;
	unsigned short crc;
}down_frame_test_t;

//均匀时段水文信息上报文
typedef struct up_frame_average_time_s {
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;         
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	unsigned char sn;
	up_frame_average_time_content_t up_frame_average_time_content;
	unsigned char content_end_id;
	unsigned short crc;
}up_frame_average_time_t;

//均匀时段水文信息下报文
typedef struct down_frame_average_time_s {
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;          
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	unsigned char sn;
	pkt_time_t pkt_time;
	unsigned char content_end_id;
	unsigned short crc;	
}down_frame_average_time_t;

//定时报上报文
typedef struct up_frame_timed_s {
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;          
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	up_frame_timed_content_t up_frame_timed_content;
	unsigned char content_end_id;
	unsigned short crc;
}up_frame_timed_t;

//定时报下报文
typedef struct down_frame_timed_s{
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;         
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	unsigned char sn;
	pkt_time_t pkt_time;
	unsigned char content_end_id;
	unsigned short crc;
}down_frame_timed_t;

//加报上报文
typedef struct up_frame_plus_s{
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;         
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	up_frame_plus_content_t up_frame_plus_content;
	unsigned char content_end_id;
	unsigned short crc;
}up_frame_plus_t;

//加报下报文
typedef struct down_frame_plus_s {
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;          
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	unsigned char sn;
	pkt_time_t pkt_time;
	unsigned char content_end_id;
	unsigned short crc;
}down_frame_plus_t;

//人工置数上报文
typedef struct up_frame_manual_set_s {
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	up_frame_manual_set_content_t up_frame_manual_set_content;
	unsigned char content_end_id;
	unsigned short crc;
}up_frame_manual_set_t;

//人工置数下报文
typedef struct down_frame_manual_set_s {
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;
	unsigned char content_start_id;
	unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	unsigned char sn;
	pkt_time_t pkt_time;
	unsigned char content_end_id;
	unsigned short crc;
}down_frame_manual_set_t;

**/









#endif
