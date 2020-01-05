/*
* @file dmausart.c
* @brief 

* @version 1.0   2019.8.30       4G串口2通过DMA+IDLE接受数据帧，开机第一帧数据收不到,第二帧多一个字节
				main实验代码
				if(g_usart2_receive_stat == 1)
				{
					fun_id = GetFunId();
					if(fun_id != 1)
					{
						printf("%02x\r\n",fun_id);
					}
					else
					{
						printf("no receive fun_id\r\n");
					}
				}
* @author 
* @date 2019.8.30
*/



#include "dmausart.h"
#include "usart.h"
#include "ucos_ii.h"
#include <string.h>


u8 DMA_usart2_Buf[DMA_Rec_Len] = {0};
u32 g_usart2_receive_cnt = 0;
u8 g_usart2_receive_stat =  0;



void gprs_usart_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	USART_DeInit(USART2);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART2, &USART_InitStructure); //初始化串口2
  USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启串口接受中断
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(USART2, ENABLE);                    //使能串口2
	
	DMA_DeInit(DMA1_Channel6);   //将DMA的通道5寄存器重设为缺省值  串口1对应的是DMA通道5
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR; //DMA外设usart基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_usart2_Buf;  //DMA内存基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设读取发送到内存
  DMA_InitStructure.DMA_BufferSize = DMA_Rec_Len;  //DMA通道的DMA缓存的大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度为8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道

	//DMA_Cmd(DMA1_Channel6, ENABLE);

}


void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
    DMA_Cmd(DMA_CHx, DISABLE );  //关闭USART2 TX DMA1所指示的通道    
    DMA_SetCurrDataCounter(DMA_CHx,DMA_Rec_Len);//DMA通道的DMA缓存的大小
    DMA_Cmd(DMA_CHx, ENABLE);  //打开USART2 TX DMA1所指示的通道  
}


void Usart2_Send(u8 *buf,u32 len)
{
	u32 t;
	for(t=0;t<len;t++)      //循环发送数据
	{          
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);   
		USART_SendData(USART2,buf[t]);
	}    
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);     
}

void USART2_IRQHandler(void)     
{
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(USART2);
		g_usart2_receive_cnt = DMA_Rec_Len - DMA_GetCurrDataCounter(DMA1_Channel6);
		(g_usart2_receive_cnt > 0) ? (g_usart2_receive_stat = 1):(g_usart2_receive_stat = 0);

		USART_ClearITPendingBit(USART2,USART_IT_IDLE);         //清除中断标志
    MYDMA_Enable(DMA1_Channel6);                  //恢复DMA指针，等待下一次的接收

	}
	#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntExit();  
	#endif
}



#if 0
#include "dmausart.h"
#include "protocol.h"
#include "usart.h"
#include <string.h>


u8 DMA_usart2_Buf[DMA_Rec_Len] = {0};
u32 g_usart2_receive_cnt = 0;
u8 g_usart2_receive_stat =  0;



void gprs_usart_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	USART_DeInit(USART2);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
 	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

 	USART_Init(USART2, &USART_InitStructure); //初始化串口2
 	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启串口接受中断
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	USART_Cmd(USART2, ENABLE);                    //使能串口2
		
	DMA_DeInit(DMA1_Channel6);   //将DMA的通道5寄存器重设为缺省值  串口1对应的是DMA通道5
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR; //DMA外设usart基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_usart2_Buf;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设读取发送到内存
	DMA_InitStructure.DMA_BufferSize = DMA_Rec_Len;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道

	DMA_Cmd(DMA1_Channel6, ENABLE);

}


void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
    DMA_Cmd(DMA_CHx, DISABLE);  //关闭USART2 TX DMA1所指示的通道    
    DMA_SetCurrDataCounter(DMA_CHx,DMA_Rec_Len);//DMA通道的DMA缓存的大小
    DMA_Cmd(DMA_CHx, ENABLE);  //打开USART2 TX DMA1所指示的通道  
}


void Usart2_Send(u8 *buf,u32 len)
{
	u32 t;
	for(t=0;t<len;t++)      //循环发送数据
	{          
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);   
		USART_SendData(USART2,buf[t]);
	}    
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);     
}

void USART2_IRQHandler(void)     
{
	#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
	#endif
	
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(USART2);
		g_usart2_receive_cnt = DMA_Rec_Len - DMA_GetCurrDataCounter(DMA1_Channel6);
		printf("g_usart2_receive_cnt is %d\r\n", g_usart2_receive_cnt);
		(g_usart2_receive_cnt > 0) ? (g_usart2_receive_stat = 1):(g_usart2_receive_stat = 0);
	//	printf("the length is %d\r\n", g_usart2_receive_cnt);
	//	Usart2_Send(DMA_usart2_Buf, g_usart2_receive_cnt);
	
		USART_ClearITPendingBit(USART2,USART_IT_IDLE);         //清除中断标志
    MYDMA_Enable(DMA1_Channel6);                  //恢复DMA指针，等待下一次的接收

	}
	#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
	#endif
}



#endif













