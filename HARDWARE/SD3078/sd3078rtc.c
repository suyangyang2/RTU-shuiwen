
#include "sd3078rtc.h"
#include "usart.h"
#include "delay.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "misc.h"


/* Private define ------------------------------------------------------------*/
#define WAITEDELAY  50
#define WAKE_UP_SECOND  0X50          //���ѵ�ʱ����ֵ
#define		TRUE            1
#define		FALSE           0
static Time_Def s_sysTime;

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configure the used I/O ports pin
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/


void read_date(void)
{
	if(RTC_ReadDate(&s_sysTime) != TRUE)
	printf("read time failed\r\n");
	printf("%02x-%02x-%02x---%02x:%02x:%02x\r\n",s_sysTime.year, s_sysTime.month, s_sysTime.day, s_sysTime.hour,\
		s_sysTime.minute, s_sysTime.second);
}

void write_date(u8 year, u8 month, u8 day, u8 hour, u8 minute, u8 seconds)
{
	Time_Def write_time = {0};
	
	write_time.year = year;
	write_time.month = month;
	write_time.day = day;
	write_time.hour = hour;
	write_time.minute = minute;
	write_time.second = seconds;

	if(RTC_WriteDate(&write_time) != TRUE)
	{
		printf("rtc write date failed\r\n");
	}
}

static void GPIO_Configuration(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
  NVIC_InitTypeDef  NVIC_InitStructure;
  EXTI_InitTypeDef 	EXTI_InitStructure;
  

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA ,ENABLE);
    
  /* Configure I2C1 pins: SCL and SDA */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;//��Ϊ��©���ⲿ����������Ϊ������ܻ�������
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;//��Ϊ��©���ⲿ����
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //SD3078_INT PA0
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);


  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
 
 
}


/*********************************************
 * ��������I2Cdelay
 * ��  ����I2C��ʱ����
 * ��  �룺��
 * ��  ������
 ********************************************/
static void I2Cdelay(void)
{	
   u8 i=WAITEDELAY; //��������Ż��ٶȣ�ͨ����ʱ3~10us��������ʾ����������������
   while(i) 
   { 
     i--; 
   }
}

/*********************************************
 * ��������IIC_Init
 * ��  ����I2C�˿ڳ�ʼ��
 * ��  �룺��
 * ��  ������
 ********************************************/
void SD3078_Init(void)
{
  /* GPIO configuration */
  GPIO_Configuration();
}

/*********************************************
 * ��������I2CStart
 * ��  ��������I2C����
 * ��  �룺��
 * ��  ����TRUE:�����ɹ���FALSE:����ʧ��
 ********************************************/
static u8 I2CStart(void)
{
    SDA_H;
		I2Cdelay();
    SCL_H;
    I2Cdelay();
    if(!SDA_read)return FALSE;	//SDA��Ϊ�͵�ƽ������æ,�˳�
    SDA_L;
    I2Cdelay();
    SCL_L;
    I2Cdelay();
    return TRUE;
}

/*********************************************
 * ��������I2CStop
 * ��  �����ͷ�I2C����
 * ��  �룺��
 * ��  ������
 ********************************************/
static void I2CStop(void)
{
    SCL_L;
    I2Cdelay();
    SDA_L;
    I2Cdelay();
    SCL_H;
    I2Cdelay();
    SDA_H;
    I2Cdelay();
}

/*********************************************
 * ��������I2CAck
 * ��  ��������ASK
 * ��  �룺��
 * ��  ������
 ********************************************/
static void I2CAck(void)
{
    SCL_L;
    I2Cdelay();
    SDA_L;
    I2Cdelay();
    SCL_H;
    I2Cdelay();
    SCL_L;
    I2Cdelay();
}

/*********************************************
 * ��������I2CNoAck
 * ��  ��������NOASK
 * ��  �룺��
 * ��  ������
 ********************************************/
static void I2CNoAck(void)
{
    SCL_L;
    I2Cdelay();
    SDA_H;
    I2Cdelay();
    SCL_H;
    I2Cdelay();
    SCL_L;
    I2Cdelay();
}

/*********************************************
 * ��������I2CWaitAck
 * ��  ������ȡACK�ź�
 * ��  �룺��
 * ��  ����TRUE=��ACK,FALSE=��ACK
 ********************************************/
static u8 I2CWaitAck(void)
{
    SCL_L;
    I2Cdelay();
    SDA_H;			
    I2Cdelay();
    SCL_H;
    I2Cdelay();
    if(SDA_read)
    {
			SCL_L;
			return FALSE;
    }
    SCL_L;
    return TRUE;
}

/*********************************************
 * ��������I2CSendByte
 * ��  ����MCU����һ���ֽ�
 * ��  �룺��
 * ��  ������
 ********************************************/
static void I2CSendByte(u8 SendByte) //���ݴӸ�λ����λ
{
    u8 i=8;
		while(i--)
		{
			SCL_L;
			I2Cdelay();
			if(SendByte&0x80)
			SDA_H;  
			else 
			SDA_L;   
			SendByte<<=1;
			I2Cdelay();
			SCL_H;
			I2Cdelay();
		}
    SCL_L;
}

/*********************************************
 * ��������I2CReceiveByte
 * ��  ����MCU����һ���ֽ�
 * ��  �룺��
 * ��  ����ReceiveByte
 ********************************************/
static u8 I2CReceiveByte(void)
{
    u8 i=8;
    u8 ReceiveByte=0;

    SDA_H;				
    while(i--)
    {
      ReceiveByte<<=1;         
      SCL_L;
      I2Cdelay();
      SCL_H;
      I2Cdelay();	
      if(SDA_read)
      {
        ReceiveByte|=0x1;
      }
    }
    SCL_L;
    return ReceiveByte;   
}

/*********************************************
 * ��������WriteRTC_Enable
 * ��  ����RTCд�������
 * ��  �룺��
 * ��  ����TRUE:�����ɹ���FALSE:����ʧ��
 ********************************************/
u8 WriteRTC_Enable(void)
{
    if(FALSE == I2CStart()) return FALSE;
    I2CSendByte(RTC_Address); 
    if(I2CWaitAck()== FALSE){I2CStop();return FALSE;}
    I2CSendByte(CTR2);      
    I2CWaitAck();	
    I2CSendByte(0x80);//��WRTC1=1      
    I2CWaitAck();
    I2CStop(); 
										
    I2CStart();
    I2CSendByte(RTC_Address);      
    I2CWaitAck();   
    I2CSendByte(CTR1);
    I2CWaitAck();	
    I2CSendByte(0x84);//��WRTC2,WRTC3=1      
    I2CWaitAck();
    I2CStop(); 
    return TRUE;
}

/*********************************************
 * ��������WriteRTC_Disable
 * ��  ����RTCд��ֹ����
 * ��  �룺��
 * ��  ����TRUE:�����ɹ���FALSE:����ʧ��
 ********************************************/
u8 WriteRTC_Disable(void)
{
    if(FALSE == I2CStart()) return FALSE;
    I2CSendByte(RTC_Address); 
    if(!I2CWaitAck()){I2CStop(); return FALSE;}
    I2CSendByte(CTR1);//����д��ַ0FH      
    I2CWaitAck();	
    I2CSendByte(0x0) ;//��WRTC2,WRTC3=0      
    I2CWaitAck();
    I2CSendByte(0x0) ;//��WRTC1=0(10H��ַ)      
    I2CWaitAck();
    I2CStop(); 
    return TRUE; 
}

/*********************************************
 * ��������RTC_WriteDate
 * ��  ����дRTCʵʱ���ݼĴ���
 * ��  �룺ʱ��ṹ��ָ��
 * ��  ����TRUE:�����ɹ���FALSE:����ʧ��
 ********************************************/
u8 RTC_WriteDate(Time_Def	*psRTC)	//дʱ�����Ҫ��һ�ζ�ʵʱʱ��Ĵ���(00H~06H)����д�룬
{                               //�����Ե�����7��ʱ�������е�ĳһλ����д����,������ܻ�����ʱ�����ݵĴ����λ. 
                                //Ҫ�޸�����ĳһ������ , Ӧһ����д��ȫ�� 7 ��ʵʱʱ������.

		WriteRTC_Enable();				//ʹ�ܣ�����

		I2CStart();
		I2CSendByte(RTC_Address); 
		if(!I2CWaitAck()){I2CStop(); return FALSE;}
		I2CSendByte(0);			//����д��ʼ��ַ      
		I2CWaitAck();	
		I2CSendByte(psRTC->second);		//second     
		I2CWaitAck();	
		I2CSendByte(psRTC->minute);		//minute      
		I2CWaitAck();	
		I2CSendByte(psRTC->hour|0x80);//hour ,ͬʱ����Сʱ�Ĵ������λ��0��Ϊ12Сʱ�ƣ�1��Ϊ24Сʱ�ƣ�
		I2CWaitAck();	
		I2CSendByte(psRTC->week);		//week      
		I2CWaitAck();	
		I2CSendByte(psRTC->day);		//day      
		I2CWaitAck();	
		I2CSendByte(psRTC->month);		//month      
		I2CWaitAck();	
		I2CSendByte(psRTC->year);		//year      
		I2CWaitAck();	
		I2CStop();

		WriteRTC_Disable();				//����
		return	TRUE;
}

/*********************************************
 * ��������RTC_ReadDate
 * ��  ������RTCʵʱ���ݼĴ���
 * ��  �룺ʱ��ṹ��ָ��
 * ��  ����TRUE:�����ɹ���FALSE:����ʧ��
 ********************************************/
u8 RTC_ReadDate(Time_Def	*psRTC)
{
		I2CStart();
		I2CSendByte(RTC_Address);      
		if(!I2CWaitAck()){I2CStop(); return FALSE;}
		I2CSendByte(0);
		I2CWaitAck();
		I2CStart();	
		I2CSendByte(RTC_Address+1);
		I2CWaitAck();
		psRTC->second=I2CReceiveByte();
		I2CAck();
		psRTC->minute=I2CReceiveByte();
		I2CAck();
		psRTC->hour=I2CReceiveByte() & 0x7F;
		I2CAck();
		psRTC->week=I2CReceiveByte();
		I2CAck();
		psRTC->day=I2CReceiveByte();
		I2CAck();
		psRTC->month=I2CReceiveByte();
		I2CAck();
		psRTC->year=I2CReceiveByte();		
		I2CNoAck();
		I2CStop(); 
		return	TRUE;
}
/*********************************************
 * ������     ��I2CWriteSerial
 * ��  ��     ��I2C��ָ����ַдһ�ֽ�����
 * Device_Addr��I2C�豸��ַ
 * Address    ���ڲ���ַ
 * length     ���ֽڳ���
 * ps         ��������ָ��
 * ���       ��TRUE �ɹ���FALSE ʧ��
 ********************************************/	
u8 I2CWriteSerial(u8 DeviceAddress, u8 Address, u8 length, u8 *ps)
{
		if(DeviceAddress == RTC_Address)  WriteRTC_Enable();

		I2CStart();
		I2CSendByte(DeviceAddress);   
		if(!I2CWaitAck()){I2CStop(); return FALSE;}
		I2CSendByte(Address);			
		I2CWaitAck();
		for(;(length--)>0;)
		{ 	
			I2CSendByte(*(ps++));		
			I2CAck();			
		}
		I2CStop(); 

		if(DeviceAddress == RTC_Address)  WriteRTC_Disable();
		return	TRUE;
}

/*********************************************
 * ������     ��I2CReadSerial
 * ��  ��     ��I2C��ָ����ַдһ�ֽ�����
 * Device_Addr��I2C�豸��ַ
 * Address    ���ڲ���ַ
 * length     ���ֽڳ���
 * ps         ��������ָ��
 * ���       ��TRUE �ɹ���FALSE ʧ��
 ********************************************/	
u8 I2CReadSerial(u8 DeviceAddress, u8 Address, u8 length, u8 *ps)
{
		I2CStart();
		I2CSendByte(DeviceAddress);      
		if(!I2CWaitAck()){I2CStop(); return FALSE;}
		I2CSendByte(Address);
		I2CWaitAck();
		I2CStart();	
		I2CSendByte(DeviceAddress+1);
		I2CWaitAck();
		for(;--length>0;ps++)
		{
			*ps = I2CReceiveByte();
			I2CAck();
		}
		*ps = I2CReceiveByte();	
		I2CNoAck();
		I2CStop(); 
		return	TRUE;
}

/*********************************************
 * ��������Set_CountDown
 * ��  �������õ���ʱ�ж�
 * ��  �룺CountDown_Init ����ʱ�жϽṹ��ָ�� 
 * ��  ������
 ********************************************/
void Set_CountDown(CountDown_Def *CountDown_Init)					
{
		u8 buf[6];
		u8 tem=0xF0;
		buf[0] = (CountDown_Init->IM<<6)|0xB4;							//10H
		buf[1] = CountDown_Init->d_clk<<4;									//11H
		buf[2] = 0;																					//12H
		buf[3] = CountDown_Init->init_val & 0x0000FF;				//13H
		buf[4] = (CountDown_Init->init_val & 0x00FF00) >> 8;//14H
		buf[5] = (CountDown_Init->init_val & 0xFF0000) >> 16;//15H
		I2CWriteSerial(RTC_Address,CTR2,1,&tem);
		I2CWriteSerial(RTC_Address,CTR2,6,buf);
}

/*********************************************
 * ��������Set_Alarm
 * ��  �������ñ����жϣ����ӹ��ܣ�
 * Enable_config��ʹ������  
 * psRTC������ʱ���ʱ��ṹ��ָ��
 * ��  ������
 ********************************************/
void Set_Alarm(u8 Enable_config, Time_Def *psRTC)					
{
		u8 buf[10];
		buf[0] = psRTC->second;
		buf[1] = psRTC->minute;
		buf[2] = psRTC->hour;
		buf[3] = 0;
		buf[4] = psRTC->day;
		buf[5] = psRTC->month;
		buf[6] = psRTC->year;
		buf[7] = Enable_config;
		buf[8] = 0xFF;
		buf[9] = 0x92;	
		I2CWriteSerial(RTC_Address,Alarm_SC,10,buf);
}

/*********************************************
 * ��������SetFrq
 * ��  ��������RTCƵ���жϣ���INT�����Ƶ�ʷ���
 * ��  �룺Ƶ��ֵ
 * ��  ������
 ********************************************/
void SetFrq(enum Freq F_Out)					
{
		u8 buf[2];
		buf[0] = 0xA1;
		buf[1] = F_Out;
		I2CWriteSerial(RTC_Address,CTR2,2,buf);
}

/*********************************************
 * ��������ClrINT
 * ��  ������ֹ�ж�
 * int_EN���ж����� INTDE��INTDE��INTDE
 * ��  ������
 ********************************************/
void ClrINT(u8 int_EN)         
{
		u8 buf;
		buf = 0x80 & (~int_EN);
		I2CWriteSerial(RTC_Address,CTR2,1,&buf);
}

void SetNextFiveMinuteAlarm(Time_Def *curr_time)  
{
	Time_Def alarm = {0};
	unsigned char alarm_minutes = 0;
	unsigned char minutes = ((curr_time->minute >> 4) * 10) + ((curr_time->minute) & 0x0f);	
	
	alarm_minutes = ((minutes / 5) + 1) * 5;
	alarm_minutes -= 1;
	alarm.minute = (alarm_minutes / 10) * 16 + (alarm_minutes % 10);
	alarm.second = WAKE_UP_SECOND;
	if(curr_time->minute >= alarm.minute)  //�����ǰʱ���Ѿ��ӽ�����ʱ�䣬���ó���һ������ʱ��
	{
		if(alarm.minute < 0x59)
		{
			alarm.minute += 0x05;
		}
		else
		{
			alarm.minute = 0x04;
		}
	}
#if 0
	printf("minute is %02x", alarm.minute);
#endif
	Set_Alarm(min_ALM | sec_ALM, &alarm);
}




unsigned short GetSD3078Voltage(void)
{
	unsigned short voltage = 0;
	unsigned char read_buf[8] = { 0 };

	I2CReadSerial(RTC_Address, Bat_H8, 2, read_buf);
	voltage = ((read_buf[0] >> 7) * 255 + read_buf[1]);
	return voltage;
}


void SetSD3078LowVoltage_Alarm(void)
{
	unsigned char dat = 0x61;
	unsigned char read_dat = 0;

	I2CWriteSerial(RTC_Address, CTR4, 1, &dat);
	I2CReadSerial(RTC_Address, CTR4, 1, &read_dat);
	printf("read_dat is %02x\r\n", read_dat);

}








/*********************************************END OF FILE**********************/

