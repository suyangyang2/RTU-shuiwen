#ifndef SD3078RTC_H
#define SD3078RTC_H

#include "sys.h"

#define  SCL_H         GPIOB->BSRR = GPIO_Pin_6 
#define  SCL_L         GPIOB->BRR  = GPIO_Pin_6 

#define  SDA_H         GPIOB->BSRR = GPIO_Pin_7
#define  SDA_L         GPIOB->BRR  = GPIO_Pin_7

#define SDA_read        (GPIOB->IDR  & GPIO_Pin_7)

enum Freq{F_0Hz, F32KHz, F4096Hz, F1024Hz, F64Hz, F32Hz, F16Hz, F8Hz, \
					F4Hz, F2Hz, F1Hz, F1_2Hz, F1_4Hz, F1_8Hz, F1_16Hz, F_1s};

enum clk_Souce{S_4096Hz, S_1024Hz, S_1s, S_1min};

/*�˽ṹ�嶨����ʱ����Ϣ�����ꡢ�¡��ա����ڡ�ʱ���֡���*/
typedef	struct
{
	u8	second;
	u8	minute;
	u8	hour;
	u8	week;
	u8	day;
	u8	month;
	u8	year;
} Time_Def;

/*�˽ṹ�嶨���˵���ʱ�жϿɹ����õ�Ƶ��Դ��IM�ͳ�ֵ��Ҫ����*/
typedef	struct
{
	enum clk_Souce d_clk;
	u8   IM;	//IM=1:�������ж�
	u32  init_val;
} CountDown_Def;

/*
 * ͨ���궨��ѡ��ʹ�õ�RTC�ͺ�
 * SD30XX_25XX -> ʹ��SD30��SD25ϵ��
 * SD20XX_24XX -> ʹ��SD20��SD24ϵ��
 */
#define		SD30XX_25XX
#undef		SD20XX_24XX

/********************************************************/
#define		H               1
#define		L               0
#define		Chg_enable			0x82
#define		Chg_disable			0

/******************** Device Address ********************/
#define		RTC_Address     0x64 

/******************** Alarm register ********************/
#define		Alarm_SC				0x07
#define		Alarm_MN				0x08
#define		Alarm_HR				0x09
#define		Alarm_WK				0x0A
#define		Alarm_DY				0x0B
#define		Alarm_MO				0x0C
#define		Alarm_YR				0x0D
#define		Alarm_EN				0x0E

/******************** Control Register *******************/
#define		CTR1            0x0F
#define		CTR2            0x10
#define		CTR3            0x11
#define		CTR4			0x19
#define		CTR5			0X1A

/***************** Timer Counter Register ****************/
#define		Timer_Counter1	0x13
#define		Timer_Counter2	0x14
#define		Timer_Counter3	0x15

/******************** Battery Register ********************/
#define		Chg_MG          0x18		//������Ĵ�����ַ
#define		Bat_H8          0x1A		//�������λ�Ĵ�����ַ
#define		Bat_L8          0x1B		//�����Ͱ�λ�Ĵ�����ַ

/*********************** ID Register **********************/
#define		ID_Address			0x72		//ID����ʼ��ַ

/********************** �����жϺ궨�� *********************/
#define		sec_ALM					0x01
#define		min_ALM					0x02
#define		hor_ALM					0x04
#define		wek_ALM					0x08
#define		day_ALM					0x10
#define		mon_ALM					0x20
#define		yar_ALM					0x40

/********************** �ж�ʹ�ܺ궨�� **********************/
#define		INTDE						0x04		//����ʱ�ж�
#define		INTAE						0x02		//�����ж�
#define		INTFE						0x01		//Ƶ���ж�

#define		ELECTRIC_INT_FLAG			0X00
#define		ALARM_INT_FLAG				0x10
#define		FREQUENCY_INT_FLAG			0X20
#define		COUNTDOWN_INT_FLAG			0X30

/********************** �ж���ʾ�궨�� **********************/
#define 	FREQUENCY				0				//Ƶ���ж�
#define 	ALARM					1				//�����ж�
#define 	COUNTDOWN				2				//����ʱ�ж�
#define 	DISABLE_RTC_INT				3				//��ֹ�ж�

/*************** �ж��������ѡ��������ѡ�� ****************/
#define 	INT_TYPE			COUNTDOWN //�����ж��������

/***********��дʱ�亯��*************/
u8 RTC_WriteDate(Time_Def	*psRTC);
u8 RTC_ReadDate(Time_Def	*psRTC);

/*******I2C���ֽ�������д����********/
u8 I2CWriteSerial(u8 DeviceAddress,u8 Address,u8 length,u8 *ps);
u8 I2CReadSerial(u8 DeviceAddress,u8 Address,u8 length,u8 *ps);

/*********I2C�˿ڳ�ʼ������*********/
void SD3078_Init(void);

/*********RTC�ж����ú���*********/
void Set_CountDown(CountDown_Def *CountDown_Init);
void Set_Alarm(u8 Enable_config, Time_Def *psRTC);
void SetFrq(enum Freq F_Out);
void ClrINT(u8 int_EN);



/*��ʱ��������һ��׼��ǰ����*/
void SetNextFiveMinuteAlarm(Time_Def *curr_time);
unsigned short GetSD3078Voltage(void);
void SetSD3078LowVoltage_Alarm(void);


void read_date(void);
void write_date(u8 year, u8 month, u8 day, u8 hour, u8 minute, u8 seconds);

#endif /* __RTC_H */
