#ifndef _ST16C554_H
#define _ST16C554_H

#include "sys.h"

#define DEBUG 1

#define ST16C554D_RST		PGout(8)	//ST16C554D��λ����
#define ST16C554D_INT		PCin(7)			//ST16C554D�ж�


//ST16C554D��ַ�ṹ��
typedef struct
{
	vu16 REG;
	vu16 DATA;
}ST16C554D_TypeDef;

#define ST16C554D_BASE ((u32)0x60000000)
#define ST16C554D				((ST16C554D_TypeDef*)ST16C554D_BASE)

#define ST16C554D_RHR 			0X00 	//���ձ��ּĴ���
#define ST16C554D_THR 			0X00	//���ͱ��ּĴ���
#define	ST16C554D_IER				0X01	//�ж�ʹ�ܼĴ���
#define ST16C554D_FCR				0X02	//FIFO���ƼĴ���
#define ST16C554D_ISR				0X02	//�ж�״̬�Ĵ���
#define ST16C554D_LCR				0X03	//��·���ƼĴ���
#define ST16C554D_MCR				0X04	//Modem���ƼĴ���
#define ST16C554D_LSR				0X05	//��·״̬�Ĵ���
#define ST16C554D_MSR				0X06	//Modem״̬�Ĵ���
#define ST16C554D_SCR				0X07	//�м�������Ĵ���

#define DIVLSB			0X00	//���������ӼĴ�����λ
#define DIVMSB			0X01	//���������ӼĴ�����λ

//������
#define BAUDRATE_300	0
#define BAUDRATE_1200	1
#define BAUDRATE_2400	2
#define BAUDRATE_9600	3
#define BAUDRATE_19200	4
#define BAUDRATE_38400	5
#define BAUDRATE_56000	6
#define BAUDRATE_115200	7

//У��λ
#define NO_PARITY		0
#define ODD_PARITY		1
#define EVEN_PARITY		2

//ֹͣλ
#define STOP_BIT_1		0
#define STOP_BIT_2		1
#define STOP_BIT_1_5 	1

#define WORD_LENGTH_BIT_5	0
#define WORD_LENGTH_BIT_6	1
#define WORD_LENGTH_BIT_7	2
#define WORD_LENGTH_BIT_8	3

#define FIFO1 	0		//����FIFO��FIFO���м����ֽڽ��ᴥ���ж�
#define FIFO4 	1
#define FIFO8 	2
#define FIFO14 	3

#define CHA	0
#define CHB	1
#define CHC 2
#define CHD 3

#define CS_DISABLE 	1
#define CS_ENABLE   0



extern u8 RS232_Recvok_flag;   //1��ʾ������ɣ�0 ��ʾδ���ܽ���






u8 ST16C554D_Init(void);
unsigned char ST16C554D_Check_Port(unsigned char port);
void ST16C554D_Init_Port(unsigned char port,unsigned char baud,unsigned char parity,unsigned char stop,unsigned char wordLength,unsigned char fifo,unsigned char trigger);		
unsigned int ST16C554D_ReadBuff(unsigned char port,unsigned char* pBuff);
void ST16C554D_WriteBuff(unsigned char port,unsigned char* pBuff,int len);
unsigned char ST16C554D_ReadByte(unsigned char port,unsigned char reg);
void ST16C554D_WriteByte(unsigned char port,unsigned char reg,unsigned char value);
u16 ST16C554D_ReadReg(u8 port, u16 reg);  //���Ĵ�������
void ST16C554D_WriteReg(u8 port, u16 reg,u16 data);  //д�Ĵ�������
unsigned char ST16C554D_Check_Port(unsigned char port);



#endif
