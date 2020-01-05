#include "wkup.h"
#include "pwctrl.h"
#include "sd3078rtc.h"
#include "usart.h"




unsigned char g_stop_mode_flag = 0;

/*串口接受中断唤醒方式*/


/*
********************************************************************************************
* 函数名：WKUP_Init
* 返回值：无
* 参  数：无
* 描  述：WKUP唤醒初始化	   
*
*********************************************************************************************
*/
void WKUP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2 ;
	NVIC_Init(&NVIC_InitStructure);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling ;
	EXTI_Init(&EXTI_InitStructure);
}

void EXTI_Disable(void)
{
	//DM9000_INT
	GPIO_InitTypeDef GPIO_InitStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	#if 0   // st16_int EXTI_line7  PC7
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//外部中断线6
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = (FunctionalState)DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	#endif
	
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = (FunctionalState)DISABLE;
	EXTI_Init(&EXTI_InitStructure);

}

void EXTI_Enable(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	#if 0   // st16_int EXTI_line7  PC7
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//外部中断线6
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	#endif
	
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}



void Peripheral_Disable(void)
{
	EXTI_Disable();  //DM9000_INT 中断需要关闭，不然待机后会重复进入中断；

	PWR_OFF();
}

void Peripheral_Enable(void)
{
	EXTI_Enable();
	PWR_ON();
}


/*
********************************************************************************************
* 函数名：enter_stop_mode
* 返回值：无
* 参  数：无
* 描  述：进入停机模式，并设置wkup唤醒
*
*********************************************************************************************
*/
void enter_stop_mode(void)
{ 
	Time_Def real_time;
	g_stop_mode_flag = 1;

	RTC_ReadDate(&real_time);
	//Enable_RS232_1_ExternalHandler();   //使能串口外部中断唤醒
	SetNextFiveMinuteAlarm(&real_time);	//使能闹钟唤醒
	
	Peripheral_Disable();
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	//PWR_WakeUpPinCmd(ENABLE);  //WKUP 
	PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
}


//void EXTI0_IRQHandler(void)  protocol.c
