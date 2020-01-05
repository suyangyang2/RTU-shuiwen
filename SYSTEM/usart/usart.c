

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
//如果使用os,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif

#define USART_DEBUG 1
#define		TRUE            1
#define		FALSE           0


#define LOG_MAX_OUTPUT_LEN 1024
static int sw_log_level = SW_LOG_LEVEL_PANIC;      //
	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u16 USART_RX_STA=0;       //接收状态标记	
u8 DMA_usart1_Buf[USART_REC_LEN] = {0};
u32 g_usart1_recveive_cnt = 0;
u8 g_usart1_receive_stat = 0;


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
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
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 	//EXTI_InitTypeDef EXTI_InitStructure; 
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOG, ENABLE);	//使能USART1，GPIOA时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
 	USART_DeInit(USART1);  //复位串口1

	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
 
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口

#if USART1_IDLE_ENABLE
  USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
#else
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
#endif
  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(USART1, ENABLE);                    //使能串口 
	
	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_usart1_Buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = USART_REC_LEN;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道

	DMA_Cmd(DMA1_Channel5, ENABLE);
}
	
static void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
    DMA_Cmd(DMA_CHx, DISABLE);  //关闭USART2 TX DMA1所指示的通道    
    DMA_SetCurrDataCounter(DMA_CHx,USART_REC_LEN);//DMA通道的DMA缓存的大小
    DMA_Cmd(DMA_CHx, ENABLE);  //打开USART2 TX DMA1所指示的通道  
}

void Usart1_Send(u8 *buf,u32 len)
{
	u8 t;
	for(t=0;t<len;t++)      //循环发送数据
	{          
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);   
		USART_SendData(USART1,buf[t]);
	}    
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); 

}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		if((USART_RX_STA&0x8000)==0)//接收未完成
			{
			if(USART_RX_STA&0x4000)//接收到了0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
				}
			else //还没收到0X0D
				{	
				if(Res == 0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
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
		MYDMA_Enable(DMA1_Channel5);                  //恢复DMA指针，等待下一次的接收
	}
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
} 





