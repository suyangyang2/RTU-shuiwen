#include "sram.h"	  
#include "usart.h"



//ʹ��NOR/SRAM�� Bank1.sector3,��ַλHADDR[27,26]=10 
//��IS61LV25616/IS62WV25616,��ַ�߷�ΧΪA0~A17 
//��IS61LV51216/IS62WV51216,��ַ�߷�ΧΪA0~A18
#define Bank1_SRAM3_ADDR    ((u32)(0x68000000))		
  						   
/*
********************************************************************************************
* ��������FSMC_SRAM_Init
* ����ֵ����	
* ��  ������
* ��  ����FSMC_SRAM��ʼ��
*
*********************************************************************************************
*/
void FSMC_SRAM_Init(void)
{	
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;
	GPIO_InitTypeDef  GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF|RCC_APB2Periph_GPIOG,ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE);
  
	GPIO_InitStructure.GPIO_Pin = 0xFF33; 			 	//PORTD����������� 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //�����������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 0xFF83; 			 	//PORTE����������� 
 	GPIO_Init(GPIOE, &GPIO_InitStructure);

 	GPIO_InitStructure.GPIO_Pin = 0xF03F; 			 	//PORTD����������� 
 	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 0x043F; 			 	//PORTD����������� 
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 		  	  
	readWriteTiming.FSMC_AddressSetupTime = 0x00;	 //��ַ����ʱ�䣨ADDSET��Ϊ1��HCLK 1/36M=27ns
	readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //��ַ����ʱ�䣨ADDHLD��ģʽAδ�õ�	
	readWriteTiming.FSMC_DataSetupTime = 0x03;		 //���ݱ���ʱ�䣨DATAST��Ϊ3��HCLK 4/72M=55ns(��EM��SRAMоƬ)	 
	readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	readWriteTiming.FSMC_CLKDivision = 0x00;
	readWriteTiming.FSMC_DataLatency = 0x00;
	readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //ģʽA 


	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;//  ��������ʹ��NE3 ��Ҳ�Ͷ�ӦBTCR[4],[5]��
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; 
	FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//�洢�����ݿ��Ϊ16bit  
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//�洢��дʹ�� 
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;  
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable; // ��дʹ����ͬ��ʱ��
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &readWriteTiming; //��дͬ��ʱ��

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //��ʼ��FSMC����

	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);  // ʹ��BANK3										  
											
}

/*
********************************************************************************************
* ��������FSMC_SRAM_WriteBuffer
* ����ֵ����	
* ��  ����* pBuffer��Ҫд������ݴ洢��
*					WriteAddr��д�����ݵ���ʼ��ַ
*					n:���ݳ���
* ��  ������ָ����ַ��ʼ,��SRAM������д��n���ֽ�.
*
*********************************************************************************************
*/
void FSMC_SRAM_WriteBuffer(u8* pBuffer,u32 WriteAddr,u32 n)
{
	for(;n!=0;n--)  
	{										    
		*(vu8*)(Bank1_SRAM3_ADDR+WriteAddr)=*pBuffer;	  
		WriteAddr++; 
		pBuffer++;
	}   
}																			    

/*
********************************************************************************************
* ��������FSMC_SRAM_ReadBuffer
* ����ֵ����	
* ��  ����* pBuffer�����������ݴ洢��
*					WriteAddr�������ݵ���ʼ��ַ
*					n:���ݳ���
* ��  ������ָ����ַ��ʼ,��SRAM��������n���ֽ�.
*
*********************************************************************************************
*/
void FSMC_SRAM_ReadBuffer(u8* pBuffer,u32 ReadAddr,u32 n)
{
	for(;n!=0;n--)  
	{											    
		*pBuffer++=*(vu8*)(Bank1_SRAM3_ADDR+ReadAddr);    
		ReadAddr++; 
	}  
} 

/*
********************************************************************************************
* ��������fsmc_sram_test_write
* ����ֵ����	
* ��  ����data��Ҫд���ֽ�����
*					addr�������ݵ���ʼ��ַ
* ��  ������ָ����ַд��1���ֽ�
*
*********************************************************************************************
*/
void fsmc_sram_test_write(u8 data,u32 addr)
{			   
	FSMC_SRAM_WriteBuffer(&data,addr,1);//д��1���ֽ�
}

/*
********************************************************************************************
* ��������fsmc_sram_test_read
* ����ֵ���������ֽ�����
* ��  ����addr�������ݵ���ʼ��ַ
* ��  ������ָ����ַ��1���ֽ�����
*
*********************************************************************************************
*/
u8 fsmc_sram_test_read(u32 addr)
{
	u8 data;
	FSMC_SRAM_ReadBuffer(&data,addr,1);
	return data;
}	











