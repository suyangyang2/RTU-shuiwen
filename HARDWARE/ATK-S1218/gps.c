#include "gps.h"
#include "usart.h"
#include "wk2124.h"


#define BEIDOU_MAX_SEND_LEN		600					//����ͻ����ֽ���
u8  BEIDOU_TX_BUF[BEIDOU_MAX_SEND_LEN]; 			//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�

/*
********************************************************************************************
* ��������SkyTra_Cfg_Rate
* ����ֵ��0,���ͳɹ�;����,����ʧ��.
* ��  ����Frep:��ȡֵ��Χ:1,2,4,5,8,10,20,25,40,50������ʱ��������λΪHz������ܴ���50Hz
* ��  ��������SkyTraF8-BD�ĸ�������	   
*
*********************************************************************************************
*/
u8 SkyTra_Cfg_Rate(u8 Frep)
{
	SkyTra_PosRate *cfg_rate=(SkyTra_PosRate *)BEIDOU_TX_BUF;
 	cfg_rate->sos=0XA1A0;	    //cfg header(С��ģʽ)
	cfg_rate->PL=0X0300;			//��Ч���ݳ���(С��ģʽ)
	cfg_rate->id=0X0E;	      //cfg rate id
	cfg_rate->rate=Frep;	 	  //��������
	cfg_rate->Attributes=0X01;	   	//���浽SRAM&FLASH	.
	cfg_rate->CS=cfg_rate->id^cfg_rate->rate^cfg_rate->Attributes;//������,us
	cfg_rate->end=0X0A0D;       //���ͽ�����(С��ģʽ)
//	RS485_Send_Data((u8 *)cfg_rate,sizeof(SkyTra_PosRate));
//	for(i = 0;i <sizeof(SkyTra_PosRate);i ++)
//	{
//		printf("%02X\t",BEIDOU_TX_BUF[i]);
//	}
	
//	SkyTra_Send_Date((u8*)cfg_rate,sizeof(SkyTra_PosRate));//�������ݸ�NEO-6M 
//	return SkyTra_Cfg_Ack_Check();
	return 0;
}

/*
********************************************************************************************
* ��������SkyTra_Send_Date
* ����ֵ����
* ��  ����dbuf�����ݻ����׵�ַ
*					len��Ҫ���͵��ֽ���
* ��  ��������һ�����ݸ�SkyTraF8-BD������ͨ������3����   
*
*********************************************************************************************
*/
void SkyTra_Send_Date(u8* dbuf,u16 len)
{
	Wk2xxxSendBuf(BEIDOU_PORT,dbuf,len);
}
