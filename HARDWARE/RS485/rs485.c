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
///RS485_1程序在system文件夹中的usart.c

/*
********************************************************************************************
* 函数名：RS485_2_Init
* 返回值：无
* 参  数：bound:串口波特率值
* 描  述：485_2串口初始化
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
	 //Usart1 NVIC 配置

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART2, &USART_InitStructure); //初始化串口
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
  USART_Cmd(USART2, ENABLE);                    //使能串口 
	
	RS485_2_TX_EN=1;   //默认发送模式


}

/*
********************************************************************************************
* 函数名：RS485_2_Send_Data
* 返回值：无
* 参  数：*buf:待发送的数据
*					len:发送的数据的长度
* 描  述：485_2发送指定的长度的数据
*
*********************************************************************************************
*/
void RS485_2_Send_Data(u8 *buf,u8 len)
{
	u8 t;
	RS485_2_TX_EN=1;			//设置为发送模式
  	for(t=0;t<len;t++)		//循环发送数据
	{		   
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART2,buf[t]);
	}	 
 
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		
	RS485_2_TX_EN = 0;				//设置为接收模式	
}

/*
********************************************************************************************
* 函数名：RS485_2_Send_String
* 返回值：无
* 参  数：*buf:待发送的字符串数据
* 描  述：485_2发送字符串
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

#ifdef EN_USART2_RX   	//如果使能了接收

u8 RS485_2_RX_BUF[64];  	//接收缓冲,最大64个字节.
u8 RS485_2_RX_CNT=0;   //接收到的数据长度


/*
********************************************************************************************
* 函数名：USART2_IRQHandler
* 返回值：无
* 参  数：无
* 描  述：串口2中断服务函数
*
*********************************************************************************************
*/
/**
void USART2_IRQHandler(void)
{
	u8 res;	
 
 	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收到数据
	{	 
	 			 
		res =USART_ReceiveData(USART2); 	//读取接收到的数据
	

		if(RS485_2_RX_CNT<64)
		{
			RS485_2_RX_BUF[RS485_2_RX_CNT]=res;		//记录接收到的值
			RS485_2_RX_CNT++;						//接收数据增加1 
		} 
	}  											 
} 
**/
#endif

/*
********************************************************************************************
* 函数名：RS485_2_Receive_Data
* 返回值：无
* 参  数：*buf:接收到的数据
*					len:接收到的数据长度
* 描  述：串口2中断服务函数
*
*********************************************************************************************
*/
void RS485_2_Receive_Data(u8 *buf,u8 *len)
{
	u8 rxlen=RS485_2_RX_CNT;
	u8 i=0;
	*len=0;				//默认为0
	delay_ms(5);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束
	if(rxlen==RS485_2_RX_CNT&&rxlen)//接收到了数 ,且接收完成了
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_2_RX_BUF[i];	
		}		
		
		*len=RS485_2_RX_CNT;	//记录本次数据长度
		RS485_2_RX_CNT=0;		//清零
	}
}

#endif

