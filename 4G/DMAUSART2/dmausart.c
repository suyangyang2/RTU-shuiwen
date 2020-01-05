/*
* @file dmausart.c
* @brief 

* @version 1.0   2019.8.30       4G����2ͨ��DMA+IDLE��������֡��������һ֡�����ղ���,�ڶ�֡��һ���ֽ�
				mainʵ�����
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
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.2
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART2, &USART_InitStructure); //��ʼ������2
  USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//�������ڽ����ж�
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2
	
	DMA_DeInit(DMA1_Channel6);   //��DMA��ͨ��5�Ĵ�������Ϊȱʡֵ  ����1��Ӧ����DMAͨ��5
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR; //DMA����usart����ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_usart2_Buf;  //DMA�ڴ����ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴������ȡ���͵��ڴ�
  DMA_InitStructure.DMA_BufferSize = DMA_Rec_Len;  //DMAͨ����DMA����Ĵ�С
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����ַ�Ĵ�������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //���ݿ��Ϊ8λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��

	//DMA_Cmd(DMA1_Channel6, ENABLE);

}


void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
    DMA_Cmd(DMA_CHx, DISABLE );  //�ر�USART2 TX DMA1��ָʾ��ͨ��    
    DMA_SetCurrDataCounter(DMA_CHx,DMA_Rec_Len);//DMAͨ����DMA����Ĵ�С
    DMA_Cmd(DMA_CHx, ENABLE);  //��USART2 TX DMA1��ָʾ��ͨ��  
}


void Usart2_Send(u8 *buf,u32 len)
{
	u32 t;
	for(t=0;t<len;t++)      //ѭ����������
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

		USART_ClearITPendingBit(USART2,USART_IT_IDLE);         //����жϱ�־
    MYDMA_Enable(DMA1_Channel6);                  //�ָ�DMAָ�룬�ȴ���һ�εĽ���

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
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.2
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
 	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

 	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
 	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//�������ڽ����ж�
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2
		
	DMA_DeInit(DMA1_Channel6);   //��DMA��ͨ��5�Ĵ�������Ϊȱʡֵ  ����1��Ӧ����DMAͨ��5
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR; //DMA����usart����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_usart2_Buf;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴������ȡ���͵��ڴ�
	DMA_InitStructure.DMA_BufferSize = DMA_Rec_Len;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��

	DMA_Cmd(DMA1_Channel6, ENABLE);

}


void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
    DMA_Cmd(DMA_CHx, DISABLE);  //�ر�USART2 TX DMA1��ָʾ��ͨ��    
    DMA_SetCurrDataCounter(DMA_CHx,DMA_Rec_Len);//DMAͨ����DMA����Ĵ�С
    DMA_Cmd(DMA_CHx, ENABLE);  //��USART2 TX DMA1��ָʾ��ͨ��  
}


void Usart2_Send(u8 *buf,u32 len)
{
	u32 t;
	for(t=0;t<len;t++)      //ѭ����������
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
	
		USART_ClearITPendingBit(USART2,USART_IT_IDLE);         //����жϱ�־
    MYDMA_Enable(DMA1_Channel6);                  //�ָ�DMAָ�룬�ȴ���һ�εĽ���

	}
	#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
	#endif
}



#endif













