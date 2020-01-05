#include "st16c554.h"
#include "delay.h"
#include "usart.h"
#include "rs485.h"


static unsigned int port_base_addr[CHD + 1];



u8 RS232_Recvok_flag = 0;   //1��ʾ������ɣ�0 ��ʾδ���ܽ���

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
* ������:ST16C554D_Reset
* ����ֵ����	
* ��  ��: ��
* ��  �������ڸ�λST16C554D
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
* ��������ST16C554D_Check_Port
* ����ֵ�������ļĴ�����ֵ
* ��  ����port���˿ں�
* ��  �����˶��Ƿ���д���������ڵļĴ���
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
* ��������ST16C554D_ReadReg
* ����ֵ��ST16C554Dָ���Ĵ�����ֵ
* ��  ����reg:Ҫ���ļĴ���
* ��  ������ȡST16C554Dָ���Ĵ�����ֵ
*
*********************************************************************************************
*/
u16 ST16C554D_ReadReg(u8 port, u16 reg)
{
	return *(u8 *)(port_base_addr[port] + reg);
}

/*
********************************************************************************************
* ��������ST16C554D_WriteReg
* ����ֵ����
* ��  ����reg:Ҫд��ļĴ���
*					data:Ҫд���ֵv
* ��  ������ST16C554Dָ���Ĵ�����д��ָ��ֵ
*
*********************************************************************************************
*/
void ST16C554D_WriteReg(u8 port, u16 reg,u16 data)
{
	*(u8 *)(port_base_addr[port] + reg) = data;
}

/*
********************************************************************************************
* ��������ST16C554D_Init
* ����ֵ��0:�ɹ� ��������ʧ��	
* ��  ������
* ��  ����ST16C554D��ʼ��
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
	FSMC_NORSRAMTimingInitTypeDef ReadWriteTiming; 	//DM9000�Ķ�дʱ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | \
						   RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG,ENABLE);	//ʹ��GPIOD E F Gʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);	//ʹ��FSMCʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	//ʹ�ܸ��ù���ʱ��
 
	//reset ST16C554 pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 		//PG8 ������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//�������
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	//interrupt ST16C554 pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 		//PC7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	//��������
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	//FSMC D0/D1/D2/D3/NE1/NOE/NWE
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4|GPIO_Pin_5 | GPIO_Pin_7 |\
								   GPIO_Pin_14 | GPIO_Pin_15; //PD0 1 7 5 7 14 15����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//�����������
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	
	//FSMC D4/D5/D6/D7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10; 			//PG7 8 9 10 11����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//�����������
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	//FSMC_A0/A1/A2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;		//PF0, 1, 2����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//�����������
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	
	//PC7�ⲿ�жϣ��ж���7
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);

	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	EXTI_ClearITPendingBit(EXTI_Line7); //����ж���7�����־λ
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//�ⲿ�ж���7
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0x02;	//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;			//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	
	ReadWriteTiming.FSMC_AddressSetupTime = 0;		//��ַ����ʱ��
	ReadWriteTiming.FSMC_AddressHoldTime = 0;
	ReadWriteTiming.FSMC_DataSetupTime = 3;		//���ݽ���ʱ��
	ReadWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	ReadWriteTiming.FSMC_CLKDivision = 0x00;
	ReadWriteTiming.FSMC_DataLatency = 0x00;
	ReadWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;//ʹ��ģʽA
	
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
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE); //ʹ��FSMC��Bank1_Bank1_NORSRAM1
	
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
* ��������ST16C554D_WriteByte
* ����ֵ����	
* ��  ����value:д���˿ڵ��ֽ�
* ��  ������˿�д�ֽ�
*
*********************************************************************************************
*/
void ST16C554D_WriteByte(unsigned char port,unsigned char reg,unsigned char value)
{	

	ST16C554D_WriteReg(port, reg, value);
}

/*
********************************************************************************************
* ��������ST16C554D_ReadByte
* ����ֵ���˿��ֽ�	
* ��  ����
* ��  ������ȡ�˿��ֽ�
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
* ��������ST16C554D_Init
* ����ֵ����	
* ��  ����
* ��  �������ڳ�ʼ��оƬ��������ͨ�������ò����ʣ���żУ��λ��ֹͣλ��	
*
*********************************************************************************************
*/
void ST16C554D_Init_Port(unsigned char port,unsigned char baud,unsigned char parity,unsigned char stop,unsigned char wordLength,unsigned char fifo,unsigned char trigger)
{
	char lcr_byte = 0;
	
	ST16C554D_WriteByte(port, ST16C554D_LCR, 0X80);		//���������ʷ�����λ
	ST16C554D_WriteByte(port, DIVLSB, baud_table[baud][0]);//дDIVLSB
	ST16C554D_WriteByte(port, DIVMSB, baud_table[baud][1]);//дDIVMSB
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
		fifo_byte = 0x07;			//ʹ��FIFO,��λ���ͺͽ���FIFO
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
		//����˴���ʾû�п���FIFO		
	}
	ST16C554D_WriteByte(port, ST16C554D_MCR,0X08);
	ST16C554D_WriteByte(port,ST16C554D_IER,0X01);
	
}


/*
********************************************************************************************
* ������:ST16C554D_ReadBuff
* ����ֵ����ȡ�������ݳ���	
* ��  ����pBuff:������ָ��
* ��  �������ڶ�ȡFIFO������	
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
		if((time_out --) == 0)		//���ݽ��ճ�ʱ�ж�
		{
			break;
		}
	}


	return len;
}

/*
********************************************************************************************
* ������:ST16C554D_WriteBuff
* ����ֵ����	
* ��  ����pBuff:���ͻ�����ָ�룬len��Ҫ���͵����ݳ���
* ��  �������ڷ�������	
*
*********************************************************************************************
*/
void ST16C554D_WriteBuff(unsigned char port,unsigned char* pBuff,int len)
{
	unsigned int i = 0;

	for(i = 0; i < len;)
	{
		if(ST16C554D_ReadByte(port, ST16C554D_LSR) & 0X20)	//�ж�оƬ�Ƿ�׼���ý����ֽ�
		{
			ST16C554D_WriteByte(port, ST16C554D_THR, pBuff[i++]);
		}
	}

}
