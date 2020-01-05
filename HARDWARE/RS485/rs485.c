#include "rs485.h"
#include "st16c554.h"
#include "wk2124.h"
#include "delay.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

unsigned char RS485_1_Receive[RS485_MAX_RECEIVE_LEN];
int RS485_1_Address = 0;


void RS485_GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = RS485_1_EN_PIN | RS485_2_EN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RS485_1_EN_PORT, &GPIO_InitStructure);

}


void rs485_1_sendbuff(const unsigned char *buff, const unsigned int length)
{
	RS485_1_EN = 1;
	delay_ms(10);
	
	ST16C554D_WriteBuff(0, (unsigned char*)buff, length);
	
	delay_ms(10);
	RS485_1_EN = 0;

}

void rs485_2_sendbuff(const unsigned char *buff, const unsigned int length)
{
	RS485_2_EN = 1;
	delay_ms(10);

	Wk2xxxSendBuf(3,(unsigned char *)buff, length);

	delay_ms(10);
	RS485_2_EN =0;
}













#if 0
///RS485_1������system�ļ����е�usart.c

/*
********************************************************************************************
* ��������RS485_2_Init
* ����ֵ����
* ��  ����bound:���ڲ�����ֵ
* ��  ����485_2���ڳ�ʼ��
*
*********************************************************************************************
*/
void RS485_2_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE);
	USART_DeInit(USART2);
	
	//PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//USART2_TX  PA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//USART_RX  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	 //Usart1 NVIC ����

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART2, &USART_InitStructure); //��ʼ������
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
  USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
	
	RS485_2_TX_EN=1;   //Ĭ�Ϸ���ģʽ


}

/*
********************************************************************************************
* ��������RS485_2_Send_Data
* ����ֵ����
* ��  ����*buf:�����͵�����
*					len:���͵����ݵĳ���
* ��  ����485_2����ָ���ĳ��ȵ�����
*
*********************************************************************************************
*/
void RS485_2_Send_Data(u8 *buf,u8 len)
{
	u8 t;
	RS485_2_TX_EN=1;			//����Ϊ����ģʽ
  	for(t=0;t<len;t++)		//ѭ����������
	{		   
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART2,buf[t]);
	}	 
 
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		
	RS485_2_TX_EN = 0;				//����Ϊ����ģʽ	
}

/*
********************************************************************************************
* ��������RS485_2_Send_String
* ����ֵ����
* ��  ����*buf:�����͵��ַ�������
* ��  ����485_2�����ַ���
*
*********************************************************************************************
*/
void RS485_2_Send_String(u8 *buf)
{
	RS485_2_TX_EN = 1;
	while(*buf)
	{
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET);
		USART_SendData(USART2,*buf);
		buf++ ;
	}
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET);
	RS485_2_TX_EN = 0;
}

#ifdef EN_USART2_RX   	//���ʹ���˽���

u8 RS485_2_RX_BUF[64];  	//���ջ���,���64���ֽ�.
u8 RS485_2_RX_CNT=0;   //���յ������ݳ���


/*
********************************************************************************************
* ��������USART2_IRQHandler
* ����ֵ����
* ��  ������
* ��  ��������2�жϷ�����
*
*********************************************************************************************
*/
/**
void USART2_IRQHandler(void)
{
	u8 res;	
 
 	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //���յ�����
	{	 
	 			 
		res =USART_ReceiveData(USART2); 	//��ȡ���յ�������
	

		if(RS485_2_RX_CNT<64)
		{
			RS485_2_RX_BUF[RS485_2_RX_CNT]=res;		//��¼���յ���ֵ
			RS485_2_RX_CNT++;						//������������1 
		} 
	}  											 
} 
**/
#endif

/*
********************************************************************************************
* ��������RS485_2_Receive_Data
* ����ֵ����
* ��  ����*buf:���յ�������
*					len:���յ������ݳ���
* ��  ��������2�жϷ�����
*
*********************************************************************************************
*/
void RS485_2_Receive_Data(u8 *buf,u8 *len)
{
	u8 rxlen=RS485_2_RX_CNT;
	u8 i=0;
	*len=0;				//Ĭ��Ϊ0
	delay_ms(5);		//�ȴ�10ms,��������10msû�н��յ�һ������,����Ϊ���ս���
	if(rxlen==RS485_2_RX_CNT&&rxlen)//���յ����� ,�ҽ��������
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_2_RX_BUF[i];	
		}		
		
		*len=RS485_2_RX_CNT;	//��¼�������ݳ���
		RS485_2_RX_CNT=0;		//����
	}
}

#endif

