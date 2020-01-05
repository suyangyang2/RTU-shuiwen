#include "gps.h"
#include "usart.h"
#include "wk2124.h"


#define BEIDOU_MAX_SEND_LEN		600					//最大发送缓存字节数
u8  BEIDOU_TX_BUF[BEIDOU_MAX_SEND_LEN]; 			//发送缓冲,最大USART3_MAX_SEND_LEN字节

/*
********************************************************************************************
* 函数名：SkyTra_Cfg_Rate
* 返回值：0,发送成功;其他,发送失败.
* 参  数：Frep:（取值范围:1,2,4,5,8,10,20,25,40,50）测量时间间隔，单位为Hz，最大不能大于50Hz
* 描  述：配置SkyTraF8-BD的更新速率	   
*
*********************************************************************************************
*/
u8 SkyTra_Cfg_Rate(u8 Frep)
{
	SkyTra_PosRate *cfg_rate=(SkyTra_PosRate *)BEIDOU_TX_BUF;
 	cfg_rate->sos=0XA1A0;	    //cfg header(小端模式)
	cfg_rate->PL=0X0300;			//有效数据长度(小端模式)
	cfg_rate->id=0X0E;	      //cfg rate id
	cfg_rate->rate=Frep;	 	  //更新速率
	cfg_rate->Attributes=0X01;	   	//保存到SRAM&FLASH	.
	cfg_rate->CS=cfg_rate->id^cfg_rate->rate^cfg_rate->Attributes;//脉冲间隔,us
	cfg_rate->end=0X0A0D;       //发送结束符(小端模式)
//	RS485_Send_Data((u8 *)cfg_rate,sizeof(SkyTra_PosRate));
//	for(i = 0;i <sizeof(SkyTra_PosRate);i ++)
//	{
//		printf("%02X\t",BEIDOU_TX_BUF[i]);
//	}
	
//	SkyTra_Send_Date((u8*)cfg_rate,sizeof(SkyTra_PosRate));//发送数据给NEO-6M 
//	return SkyTra_Cfg_Ack_Check();
	return 0;
}

/*
********************************************************************************************
* 函数名：SkyTra_Send_Date
* 返回值：无
* 参  数：dbuf：数据缓存首地址
*					len：要发送的字节数
* 描  述：发送一批数据给SkyTraF8-BD，这里通过串口3发送   
*
*********************************************************************************************
*/
void SkyTra_Send_Date(u8* dbuf,u16 len)
{
	Wk2xxxSendBuf(BEIDOU_PORT,dbuf,len);
}
