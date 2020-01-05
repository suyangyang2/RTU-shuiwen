

#include "sd3078rtc.h"
#include "sys.h"
#include "usart.h"
#include "malloc.h"	 
#include "misc.h"
#include "stm32f10x_dma.h" 
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include <stdarg.h>
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��os,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os ʹ��	  
#endif

#define USART_DEBUG 1
#define		TRUE            1
#define		FALSE           0


#define LOG_MAX_OUTPUT_LEN 1024
static int sw_log_level = SW_LOG_LEVEL_PANIC;      //
	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u16 USART_RX_STA=0;       //����״̬���	
u8 DMA_usart1_Buf[USART_REC_LEN] = {0};
u32 g_usart1_recveive_cnt = 0;
u8 g_usart1_receive_stat = 0;


//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 


const char *get_level_str(int level)
{
    switch (level) {
    case SW_LOG_LEVEL_QUIET:
        return "quiet";
    case SW_LOG_LEVEL_DEBUG:
        return "debug";
    case SW_LOG_LEVEL_VERBOSE:
        return "verbose";
    case SW_LOG_LEVEL_INFO:
        return "info";
    case SW_LOG_LEVEL_WARNING:
        return "warning";
    case SW_LOG_LEVEL_ERROR:
        return "error";
    case SW_LOG_LEVEL_FATAL:
        return "fatal";
    case SW_LOG_LEVEL_PANIC:
        return "panic";
    default:
        return "";
    }
}


void sw_log_default_callback(char* module, int level, const char* fmt, va_list vl)
{
	char *fmt_buf = (char *)mymalloc(SRAMEX, LOG_MAX_OUTPUT_LEN);
	char *msg_buf = (char *)mymalloc(SRAMEX, LOG_MAX_OUTPUT_LEN);

	Time_Def real_time;

	if(level > sw_log_level)
	{
		myfree(SRAMEX, fmt_buf);
		myfree(SRAMEX, msg_buf);
		return;
	}
	if(RTC_ReadDate(&real_time) != TRUE)
	{
		printf("SD3078 Can't work success\r\n");
		myfree(SRAMEX, fmt_buf);
		myfree(SRAMEX, msg_buf);
		return;
	}
	snprintf(fmt_buf, LOG_MAX_OUTPUT_LEN, "[%02x-%02x-%02x:%02x:%02x:%02x]-[%s]-%s: %s",real_time.year, real_time.month, real_time.day, real_time.hour, real_time.minute, \
	 real_time.second, module, get_level_str(level), fmt);
	vsnprintf(msg_buf, LOG_MAX_OUTPUT_LEN , fmt_buf, vl);
	printf("%s", msg_buf);
	myfree(SRAMEX, fmt_buf);
	myfree(SRAMEX, msg_buf);
	return;
}

void sw_vlog(char *module, int level, const char *fmt, va_list vl)
{
	void (*log_callback)(char*, int, const char*, va_list) = sw_log_default_callback;
	if(log_callback)
	{
		log_callback(module, level, fmt, vl);
	}
}

void sw_log(char *module, int level, const char* fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	sw_vlog(module, level, fmt, vl);
	va_end(vl);
}

int sw_log_get_level(void)
{
	return sw_log_level;
}

void sw_log_set_level(int level)
{
	sw_log_level = level;
}

void RS232_1_Init(u32 bound)
{
    //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 	//EXTI_InitTypeDef EXTI_InitStructure; 
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOG, ENABLE);	//ʹ��USART1��GPIOAʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
 	USART_DeInit(USART1);  //��λ����1

	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
 
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������

#if USART1_IDLE_ENABLE
  USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
#else
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
#endif
  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
	
	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_usart1_Buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = USART_REC_LEN;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��

	DMA_Cmd(DMA1_Channel5, ENABLE);
}
	
static void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
    DMA_Cmd(DMA_CHx, DISABLE);  //�ر�USART2 TX DMA1��ָʾ��ͨ��    
    DMA_SetCurrDataCounter(DMA_CHx,USART_REC_LEN);//DMAͨ����DMA����Ĵ�С
    DMA_Cmd(DMA_CHx, ENABLE);  //��USART2 TX DMA1��ָʾ��ͨ��  
}

void Usart1_Send(u8 *buf,u32 len)
{
	u8 t;
	for(t=0;t<len;t++)      //ѭ����������
	{          
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);   
		USART_SendData(USART1,buf[t]);
	}    
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); 

}

void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		if((USART_RX_STA&0x8000)==0)//����δ���
			{
			if(USART_RX_STA&0x4000)//���յ���0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
				}
			else //��û�յ�0X0D
				{	
				if(Res == 0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
					}		 
				}
			}   		 
     } 
	else if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		//sw_log("USART1_IRQHandler", SW_LOG_LEVEL_DEBUG,"enter idle it\r\n");
		g_usart1_recveive_cnt = USART_REC_LEN - DMA_GetCurrDataCounter(DMA1_Channel5);
		(g_usart1_recveive_cnt > 0)?(g_usart1_receive_stat = 1):(g_usart1_receive_stat = 0);
		USART_ReceiveData(USART1);
		USART_ClearITPendingBit(USART1, USART_IT_IDLE);
		MYDMA_Enable(DMA1_Channel5);                  //�ָ�DMAָ�룬�ȴ���һ�εĽ���
	}
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
} 





