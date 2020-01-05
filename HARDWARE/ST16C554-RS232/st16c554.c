#include "st16c554.h"
#include "delay.h"
#include "usart.h"
#include "rs485.h"


static unsigned int port_base_addr[CHD + 1];



u8 RS232_Recvok_flag = 0;   //1表示接受完成，0 表示未接受结束

//RTU clock input is 11.0592M, so the 16 bit divider(MSB(8bit), LSB(8bit) is clock/(baudrate * 16)
char baud_table[8][2] = {
{ 0x00, 0x09 }, /* 300 */
{ 0x40, 0x02 }, /* 1200 */
{ 0x20, 0x01 }, /* 2400 */
{ 0x48, 0x00 }, /* 9600 */
{ 0x24, 0x00 }, /* 19K */
{ 0x12, 0x00 }, /* 38k */
{ 0x0c, 0x00 }, /* 56k */
{ 0x06, 0x00 } /* 115k */
};

/*
********************************************************************************************
* 函数名:ST16C554D_Reset
* 返回值：无	
* 参  数: 无
* 描  述：用于复位ST16C554D
*
*********************************************************************************************
*/
static void ST16C554D_Reset()
{
	ST16C554D_RST = 1;
	delay_ms(30);
	ST16C554D_RST = 0;
	delay_ms(30);
}

/*
********************************************************************************************
* 函数名：ST16C554D_Check_Port
* 返回值：读到的寄存器的值
* 参  数：port：端口号
* 描  述：核对是否能写读各个串口的寄存器
*
*********************************************************************************************
*/
unsigned char ST16C554D_Check_Port(unsigned char port)
{
	int value;
	
	ST16C554D_WriteByte(port,ST16C554D_SCR, 0XAA);
	delay_ms(100);
	value = ST16C554D_ReadByte(port, ST16C554D_SCR);
	if(value == 0xAA)
	{
#if DEBUG
	 sw_log("ST16C554D_Check_Port", SW_LOG_LEVEL_QUIET, "Port %d check is sucess!\r\n", port);
#endif
	}
	else
	{
#if DEBUG
	 sw_log("ST16C554D_Check_Port", SW_LOG_LEVEL_PANIC, "Port %d check is failed!\r\n", port);
#endif
	}
	return (unsigned char)value;
}


/*
********************************************************************************************
* 函数名：ST16C554D_ReadReg
* 返回值：ST16C554D指定寄存器的值
* 参  数：reg:要读的寄存器
* 描  述：读取ST16C554D指定寄存器的值
*
*********************************************************************************************
*/
u16 ST16C554D_ReadReg(u8 port, u16 reg)
{
	return *(u8 *)(port_base_addr[port] + reg);
}

/*
********************************************************************************************
* 函数名：ST16C554D_WriteReg
* 返回值：无
* 参  数：reg:要写入的寄存器
*					data:要写入的值v
* 描  述：向ST16C554D指定寄存器中写入指定值
*
*********************************************************************************************
*/
void ST16C554D_WriteReg(u8 port, u16 reg,u16 data)
{
	*(u8 *)(port_base_addr[port] + reg) = data;
}

/*
********************************************************************************************
* 函数名：ST16C554D_Init
* 返回值：0:成功 ，其它：失败	
* 参  数：无
* 描  述：ST16C554D初始化
*
*********************************************************************************************
*/
u8 ST16C554D_Init(void)
{
	int i = 0;
	//char fifo_byte = 0;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef ReadWriteTiming; 	//DM9000的读写时序
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | \
						   RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG,ENABLE);	//使能GPIOD E F G时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);	//使能FSMC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	//使能复用功能时钟
 
	//reset ST16C554 pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 		//PG8 推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	//interrupt ST16C554 pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 		//PC7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	//下拉输入
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	//FSMC D0/D1/D2/D3/NE1/NOE/NWE
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4|GPIO_Pin_5 | GPIO_Pin_7 |\
								   GPIO_Pin_14 | GPIO_Pin_15; //PD0 1 7 5 7 14 15复用
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	
	//FSMC D4/D5/D6/D7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10; 			//PG7 8 9 10 11复用
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	//FSMC_A0/A1/A2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;		//PF0, 1, 2复用
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	
	//PC7外部中断，中断线7
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);

	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	EXTI_ClearITPendingBit(EXTI_Line7); //清除中断线7挂起标志位
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//外部中断线7
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0x02;	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;			//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	
	ReadWriteTiming.FSMC_AddressSetupTime = 0;		//地址建立时间
	ReadWriteTiming.FSMC_AddressHoldTime = 0;
	ReadWriteTiming.FSMC_DataSetupTime = 3;		//数据建立时间
	ReadWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	ReadWriteTiming.FSMC_CLKDivision = 0x00;
	ReadWriteTiming.FSMC_DataLatency = 0x00;
	ReadWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;//使用模式A
	
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;	//NE1
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &ReadWriteTiming;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &ReadWriteTiming;
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE); //使能FSMC的Bank1_Bank1_NORSRAM1
	
	ST16C554D_Reset();
	
	port_base_addr[0] = ST16C554D_BASE;		
	ST16C554D_Init_Port(0, BAUDRATE_115200, NO_PARITY, STOP_BIT_1, WORD_LENGTH_BIT_8, 1, FIFO1);
	
	port_base_addr[1] = ST16C554D_BASE | 0x4 << 13;		
	ST16C554D_Init_Port(1, BAUDRATE_115200, NO_PARITY, STOP_BIT_1, WORD_LENGTH_BIT_8, 1, FIFO1);
	
	port_base_addr[2] = ST16C554D_BASE | 0x2 << 13;		
	ST16C554D_Init_Port(2, BAUDRATE_115200, NO_PARITY, STOP_BIT_1, WORD_LENGTH_BIT_8, 1, FIFO1);
	
	port_base_addr[3] = ST16C554D_BASE | 0x6 << 13;		
	ST16C554D_Init_Port(3, BAUDRATE_115200, NO_PARITY, STOP_BIT_1, WORD_LENGTH_BIT_8, 1, FIFO1);


	for ( i = 0; i <= CHD; i ++)
	{
			ST16C554D_Check_Port(i);
	}


	
	return 0;		
} 

/*
********************************************************************************************
* 函数名：ST16C554D_WriteByte
* 返回值：无	
* 参  数：value:写到端口的字节
* 描  述：向端口写字节
*
*********************************************************************************************
*/
void ST16C554D_WriteByte(unsigned char port,unsigned char reg,unsigned char value)
{	

	ST16C554D_WriteReg(port, reg, value);
}

/*
********************************************************************************************
* 函数名：ST16C554D_ReadByte
* 返回值：端口字节	
* 参  数：
* 描  述：读取端口字节
*
*********************************************************************************************
*/
unsigned char ST16C554D_ReadByte(unsigned char port,unsigned char reg)
{		
	unsigned char value = 0;
	value = ST16C554D_ReadReg(port, reg);
	return value&0XFF;	
}


/*
********************************************************************************************
* 函数名：ST16C554D_Init
* 返回值：无	
* 参  数：
* 描  述：用于初始化芯片各个串口通道，设置波特率，奇偶校验位，停止位等	
*
*********************************************************************************************
*/
void ST16C554D_Init_Port(unsigned char port,unsigned char baud,unsigned char parity,unsigned char stop,unsigned char wordLength,unsigned char fifo,unsigned char trigger)
{
	char lcr_byte = 0;
	
	ST16C554D_WriteByte(port, ST16C554D_LCR, 0X80);		//锁定波特率发生器位
	ST16C554D_WriteByte(port, DIVLSB, baud_table[baud][0]);//写DIVLSB
	ST16C554D_WriteByte(port, DIVMSB, baud_table[baud][1]);//写DIVMSB
	lcr_byte |= wordLength;
	lcr_byte |=(stop << 2);
	if(parity != NO_PARITY)
	{
		lcr_byte |= (1 << 3);
		if(parity == EVEN_PARITY)
		{
			lcr_byte |= 0X10;
		}
		else
		{
			lcr_byte &= 0xEF;
		}
	}
	ST16C554D_WriteByte(port, ST16C554D_LCR, lcr_byte);
	if(fifo)
	{
		#if 1
		char fifo_byte = 0;
		
		fifo_byte = 0x01;
		ST16C554D_WriteByte(port, ST16C554D_FCR, fifo_byte);
		fifo_byte = 0x07;			//使能FIFO,复位发送和接收FIFO
		fifo_byte |= (trigger << 6);	
	//	printf("fifo_byte is %02x\r\n", fifo_byte);
		ST16C554D_WriteByte(port, ST16C554D_FCR, fifo_byte);

		#endif

	}
	else
	{
		char fifo_byte = 0x6;
		ST16C554D_WriteReg(port, ST16C554D_FCR, fifo_byte);
	}
	

	if(~(ST16C554D_ReadByte(port,ST16C554D_ISR)&0XC0))
	{
		//printf("port %d no open fifo\r\n",port);
		//进入此处表示没有开启FIFO		
	}
	ST16C554D_WriteByte(port, ST16C554D_MCR,0X08);
	ST16C554D_WriteByte(port,ST16C554D_IER,0X01);
	
}


/*
********************************************************************************************
* 函数名:ST16C554D_ReadBuff
* 返回值：读取到的数据长度	
* 参  数：pBuff:缓存区指针
* 描  述：用于读取FIFO中数据	
*
*********************************************************************************************
*/
unsigned int ST16C554D_ReadBuff(unsigned char port,unsigned char* pBuff)
{
	unsigned long time_out = 0X8F;
	unsigned int len = 0;

	while(1)
	{
		if(ST16C554D_ReadByte(port, ST16C554D_LSR) & 0X01)
		{
			pBuff[len++]=ST16C554D_ReadByte(port, ST16C554D_RHR);
		}
		else
		{
			break;
		}
		if((time_out --) == 0)		//数据接收超时判断
		{
			break;
		}
	}


	return len;
}

/*
********************************************************************************************
* 函数名:ST16C554D_WriteBuff
* 返回值：无	
* 参  数：pBuff:发送缓存区指针，len：要发送的数据长度
* 描  述：用于发送数据	
*
*********************************************************************************************
*/
void ST16C554D_WriteBuff(unsigned char port,unsigned char* pBuff,int len)
{
	unsigned int i = 0;

	for(i = 0; i < len;)
	{
		if(ST16C554D_ReadByte(port, ST16C554D_LSR) & 0X20)	//判断芯片是否准备好接收字节
		{
			ST16C554D_WriteByte(port, ST16C554D_THR, pBuff[i++]);
		}
	}

}
