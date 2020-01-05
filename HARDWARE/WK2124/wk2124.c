#include "wk2124.h"
#include "pwctrl.h"
#include "spi.h"
#include "usart.h"
#include "delay.h"
#include "rs485.h"
#include "string.h"


//u8 WK2XXX_Recv[MAX_RECEVE_LENGTH];
//int WK2XXX_address = 0;

//打印字符串函数 
void mprintf(u8 *date)
{
	u8 *pbuf=date;
	while(*pbuf)
	{
		Wk2xxxSendBuf(1,pbuf,1);
		pbuf++;
	}
}


void WK2124_Init(void)
{
	WK2XXX_RST_Init();
	WK2XXX_SPI_Init();
	WK2XXX_Reset_Init();
	//WK2XXX_IRQInit();	    //暂不需要中断控制4G，
	Wk2xxxInit(1);
	Wk2xxxInit(2);
	Wk2xxxInit(3);
//Wk2xxxInit(4);
	Wk2xxxSetBaud(1,B9600); //蓝牙默认波特率
	Wk2xxxSetBaud(2,B115200);
	Wk2xxxSetBaud(3,B115200);
//Wk2xxxSetBaud(4,B115200);
}


/*
********************************************************************************************
* 函数名:WK2XXX_RST_Init
* 返回值：无
* 参  数：无
* 描  述：用于WK2XXX复位GPIOO初始化
*
*********************************************************************************************
*/
void WK2XXX_RST_Init(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);	 //使能PA,PD端口时钟
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 //PG.7  端口配置
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOG, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.4
	 GPIO_SetBits(GPIOG,GPIO_Pin_7);						 //PG.7 输出高	
}

/*
********************************************************************************************
* 函数名:WK2XXX_SPI_Init
* 返回值：无
* 参  数：无
* 描  述：用于WK2XXX的SPI片选GPIO初始化，SPI总线初始化
*
*********************************************************************************************
*/
void WK2XXX_SPI_Init(void)
{
	
	SPI_CS_Init();
	SPI_BUS_Init();
		
}

/*
********************************************************************************************
* 函数名:WK2XXX_IRQInit
* 返回值：无
* 参  数：无
* 描  述：用于WK2XXX的中断GPIO初始化，中断配置初始化
*
*********************************************************************************************
*/
void WK2XXX_IRQInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

	
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
  //GPIOC.6中断线以及中断初始化配置   下降沿触发
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource6);
		

  	EXTI_InitStructure.EXTI_Line=EXTI_Line6;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器


    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能按键KEY2所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);
}

/*
********************************************************************************************
* 函数名:WK2XXX_Reset_Init
* 返回值：无
* 参  数：无
* 描  述：用于WK2XXX的复位操作
*
*********************************************************************************************
*/
void WK2XXX_Reset_Init(void)
{
	GPIO_SetBits(GPIOG,GPIO_Pin_7);//1
	GPIO_ResetBits(GPIOG,GPIO_Pin_7);//0
	delay_ms(10);
	GPIO_SetBits(GPIOG,GPIO_Pin_7);//1	
	delay_ms(100);	
}

/*
********************************************************************************************
* 函数名:SPI_CS_Init
* 返回值：无
* 参  数：无
* 描  述：用于SPI片选GPIO初始化
*
*********************************************************************************************
*/
void SPI_CS_Init(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD, ENABLE);	 //使能PA,PD端口时钟
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	//PB.12 端口配置
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.12
	 GPIO_SetBits(GPIOB,GPIO_Pin_12);						 //PB.12  输出高	
}

/*
********************************************************************************************
* 函数名:SPI_BUS_Init
* 返回值：无
* 参  数：无
* 描  述：用于SPI2总线初始化
*
*********************************************************************************************
*/
void SPI_BUS_Init(void)
{

	SPI2_Init();		   //初始化SPI
	SPI2_SetSpeed(SPI_BaudRatePrescaler_8);	//设置为10M时钟,高速模式
}

/*
********************************************************************************************
* 函数名:SPI_CS_H
* 返回值：无
* 参  数：无
* 描  述：用于SPI片选拉高
*
*********************************************************************************************
*/
void SPI_CS_H(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

/*
********************************************************************************************
* 函数名:SPI_CS_L
* 返回值：无
* 参  数：无
* 描  述：用于SPI片选拉低
*
*********************************************************************************************
*/
void SPI_CS_L(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

/*
********************************************************************************************
* 函数名:Wk2xxxWriteReg
* 返回值：无
* 参  数：port:端口值
*					reg:寄存器的地址(A3A2A1)
*					dat:为要写入的数据
* 描  述：用于WK2XXX写寄存器数据函数
*
*********************************************************************************************
*/
void Wk2xxxWriteReg(unsigned char port,unsigned char reg,unsigned char dat)
{	 
	 SPI_CS_L();//片选使能
	 SPI2_ReadWriteByte(((port-1)<<4)+reg); //写控制字节
	 SPI2_ReadWriteByte(dat); //写数据
	 SPI_CS_H();//片选无效

}

/*
********************************************************************************************
* 函数名:Wk2xxxWriteReg
* 返回值: 读到的寄存器中的数据
* 参  数：port:端口值
*					reg:寄存器的地址(A3A2A1)
* 描  述：用于WK2XXX读寄存器数据函数
*
*********************************************************************************************
*/
unsigned char Wk2xxxReadReg(unsigned char port,unsigned char reg)
{	
	unsigned char rec_data; 
	SPI_CS_L();	//片选使能
	SPI2_ReadWriteByte(0x40+((port-1)<<4)+reg);//写控制字节，控制命令构成见数据手册
	rec_data=SPI2_ReadWriteByte(0);//接收返回的数据
	SPI_CS_H();	//片选无效	
	return rec_data;
}

/*
********************************************************************************************
* 函数名：Wk2xxxWriteFifo
* 返回值：无
* 参  数：port:端口值
*					*wbuf:写入数据部分
*					len：  写入数据长度
* 描  述：该函数为写FIFO函数，通过该函数写入的数据会直接进入子串口的发送FIFO，然后通过TX引脚发送
*
*********************************************************************************************
*/
void Wk2xxxWriteFifo(unsigned char port,unsigned char *wbuf,unsigned int len)
{	 unsigned char n;
	 SPI_CS_L(); // 片选有效
	 SPI2_ReadWriteByte(0x80+((port-1)<<4)); //写FIFO控制指令
	  for(n=0;n<len;n++)
	    {
	     SPI2_ReadWriteByte(*(wbuf+n));
		} 
	 SPI_CS_H();	//片选无效

}

/*
********************************************************************************************
* 函数名：Wk2xxxReadFifo
* 返回值：无
* 参  数：port:端口值
*					*rbuf:写入数据部分
*					len：  写入数据长度
* 描  述：该函数为读FIFO函数，通过该函数可以一次读出多个接收FIFO中的数据，最多256个字节
*
*********************************************************************************************
*/
void Wk2xxxReadFifo(unsigned char port,unsigned char *rbuf,unsigned int len)
{	 unsigned char n;
	 SPI_CS_L();//片选有效
	 SPI2_ReadWriteByte(0xc0+((port-1)<<4));	//写读fifo控制指令
	 for(n=0;n<len;n++)
	   {
		*(rbuf+n)=SPI2_ReadWriteByte(0); 
	   }
	 SPI_CS_H();//片选无效										
	 //return 0;
}

/*
********************************************************************************************
* 函数名：Wk2xxxInit
* 返回值：无
* 参  数：port:端口值
* 描  述：初始化基本端口的寄存器
*
*********************************************************************************************
*/
void Wk2xxxInit(unsigned char port)
{
    unsigned char gena,grst,sier,scr;
		unsigned char gier;
	//使能子串口时钟
    gena=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GENA); 
		switch (port)
    {
          case 1://使能子串口1的时钟
              gena|=WK2XXX_UT1EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		  case 2://使能子串口2的时钟
              gena|=WK2XXX_UT2EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 3://使能子串口3的时钟
              gena|=WK2XXX_UT3EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 4://使能子串口4的时钟
              gena|=WK2XXX_UT4EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
	 }	
	//软件复位子串口
	grst=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GRST); 
	switch (port)
    {
          case 1://软件复位子串口1
              grst|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		  case 2://软件复位子串口2
              grst|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 3://软件复位子串口3
              grst|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 4://软件复位子串口4
             grst|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
	 }	
  //使能子串口中断，包括子串口总中断和子串口内部的接收中断，和设置中断触点
	gier=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GIER); 
	switch (port)
   {
         case 1://软件复位子串口1
             gier|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
             break;
		  case 2://软件复位子串口2
             gier|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
             break;
		   case 3://软件复位子串口3
             gier|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
             break;
		   case 4://软件复位子串口4
             gier|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
             break;
	 }	 
	 //使能子串口接收触点中断和超时中断
	 sier=Wk2xxxReadReg(port,WK2XXX_SIER); 
	 sier |= WK2XXX_RFTRIG_IEN|WK2XXX_RXOVT_IEN;
	 Wk2xxxWriteReg(port,WK2XXX_SIER,sier);
	 // 初始化FIFO和设置固定中断触点
	 Wk2xxxWriteReg(port,WK2XXX_FCR,0XFF);
	 //设置任意中断触点，如果下面的设置有效，那么上面FCR寄存器中断的固定中断触点将失效
	 Wk2xxxWriteReg(port,WK2XXX_SPAGE,1);//切换到page1
	 Wk2xxxWriteReg(port,WK2XXX_RFTL,0X40);//设置接收触点为64个字节
	 Wk2xxxWriteReg(port,WK2XXX_TFTL,0X40);//设置发送触点为64个字节
	 Wk2xxxWriteReg(port,WK2XXX_SPAGE,0);//切换到page0 
	 //使能子串口的发送和接收使能
	 scr=Wk2xxxReadReg(port,WK2XXX_SCR); 
	 scr|=WK2XXX_TXEN|WK2XXX_RXEN;
	 Wk2xxxWriteReg(port,WK2XXX_SCR,scr);
}

/*
********************************************************************************************
* 函数名：Wk2xxxClose
* 返回值：无
* 参  数：port:端口值
* 描  述：关闭端口，复位默认值
*
*********************************************************************************************
*/
void Wk2xxxClose(unsigned char port)
{
    unsigned char gena,grst;
	//复位子串口
	grst=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GRST); 
	switch (port)
    {
          case 1://软件复位子串口1
              grst|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		  case 2://软件复位子串口2
              grst|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 3://软件复位子串口3
              grst|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 4://软件复位子串口4
              grst|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
	 }	
	//关闭子串口时钟
    gena=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GENA); 
	switch (port)
    {
          case 1://使能子串口1的时钟
              gena&=~WK2XXX_UT1EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		  case 2://使能子串口2的时钟
              gena&=~WK2XXX_UT2EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 3://使能子串口3的时钟
              gena&=~WK2XXX_UT3EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 4://使能子串口4的时钟
              gena&=~WK2XXX_UT4EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
	 }	
}
/*
********************************************************************************************
* 函数名：Wk2xxxSetBaud
* 返回值：无
* 参  数：port:端口值
*    			baud:波特率
* 描  述：设置子串口波特率函数、此函数中波特率的匹配值是根据11.0592Mhz下的外部晶振计算的
*
*********************************************************************************************
*/
void Wk2xxxSetBaud(unsigned char port,int baud)
{  
	unsigned char baud1,baud0,pres,scr;
	//如下波特率相应的寄存器值，是在外部时钟为11.0592的情况下计算所得，如果使用其他晶振，需要重新计算
	switch (baud) 
	{
      case 1:
			baud1=0x4;
			baud0=0x7f;
			pres=0;
      break;
      case 2:
			baud1=0x2;
			baud0=0x3F;
			pres=0;
			break;
      case 3:
			baud1=0x1;
			baud0=0x1f;
			pres=0;
			break;
      case 4:
			baud1=0x00;
			baud0=0x8f;
			pres=0;
			break;
      case 5:
			baud1=0x00;
			baud0=0x47;
			pres=0;
			break;
      case 6:
			baud1=0x00;
			baud0=0x23;
			pres=0;
			break;
      case 7:
			baud1=0x00;
			baud0=0x11;
			pres=0;
			break;
			
      case 8:
			baud1=0x00;
			baud0=0x08;
			pres=0;
			break; 
       
      case 9:
			baud1=0x01;
			baud0=0x7f;
			pres=0;
			break;
      case 10:
			baud1=0x00;
			baud0=0xbf;
			pres=0;
			break;
      case 11:
			baud1=0x00;
			baud0=0x5f;
			pres=0;
			break;
      case 12:
			baud1=0x00;
			baud0=0x2f;
			pres=0;
			break;
      case 13:
			baud1=0x00;
			baud0=0x17;
			pres=0;
			break;
      case 14:
			baud1=0x00;
			baud0=0x0b;
			pres=0;
      break;
      case 15:
			baud1=0x00;
			baud0=0x05;
			pres=0;
			break;
      case 16:
			baud1=0x00;
			baud0=0x02;
			pres=0;
			break;
      default:
			baud1=0x00;
			baud0=0x00;
			pres=0;
    }
	//关掉子串口收发使能
	scr=Wk2xxxReadReg(port,WK2XXX_SCR); 
	Wk2xxxWriteReg(port,WK2XXX_SCR,0);
	//设置波特率相关寄存器
	Wk2xxxWriteReg(port,WK2XXX_SPAGE,1);//切换到page1
	Wk2xxxWriteReg(port,WK2XXX_BAUD1,baud1);
	Wk2xxxWriteReg(port,WK2XXX_BAUD0,baud0);
	Wk2xxxWriteReg(port,WK2XXX_PRES,pres);
	Wk2xxxWriteReg(port,WK2XXX_SPAGE,0);//切换到page0 
	//使能子串口收发使能
	Wk2xxxWriteReg(port,WK2XXX_SCR,scr);
	
	
}

/*
********************************************************************************************
* 函数名：Wk2xxxSendBuf
* 返回值：实际成功发送的数据
* 参  数：port:端口值
*    			*sendbuf:需要发送的数据buf
*					len：需要发送数据的长度
* 描  述：本函数为子串口发送数据的函数，发送数据到子串口的FIFO.然后通过再发送,注意：1.首先确认子串口的发送FIFO有多少数据 2.确定写入FIFO数据的个数
*
*********************************************************************************************
*/
unsigned int Wk2xxxSendBuf(unsigned char port,unsigned char *sendbuf,unsigned int len)
{
	 unsigned int ret,tfcnt,sendlen;
	 unsigned char  fsr;
	  
	 fsr=Wk2xxxReadReg(port,WK2XXX_FSR);
	 if(~fsr&WK2XXX_TFULL )//子串口发送FIFO未满
	 {

	   tfcnt=Wk2xxxReadReg(port,WK2XXX_TFCNT);//读子串口发送fifo中数据个数
		 sendlen=256-tfcnt;//FIFO能写入的最多字节数
		 
		 if(sendlen<len)
		 {
			ret=sendlen; 
			Wk2xxxWriteFifo(port,sendbuf,sendlen);
		 }
		 else
		 {
			 Wk2xxxWriteFifo(port,sendbuf,len);
			 ret=len;
		 }
	  }
	 
	 return ret;
}

/*
********************************************************************************************
* 函数名：Wk2xxxGetBuf
* 返回值：实际成功发送的数据
* 参  数：port:端口值
*    			 *getbuf:接收到的数据buf
* 描  述：本函数为子串口接收数据函数
*
*********************************************************************************************
*/
unsigned int Wk2xxxGetBuf(unsigned char port,unsigned char *getbuf)
{
	unsigned int ret=0,rfcnt;
	unsigned char fsr;
	fsr=Wk2xxxReadReg(port,WK2XXX_FSR);
	if(fsr&WK2XXX_RDAT )//子串口接收FIFO未空
	  {
	     rfcnt=Wk2xxxReadReg(port,WK2XXX_RFCNT);//读子串口发送fifo中数据个数
		 if(rfcnt==0)//当RFCNT寄存器为0的时候，有两种情况，可能是256或者是0，这个时候通过FSR来判断，如果FSR显示接收FIFO不为空，就为256个字节
		 {rfcnt=256;}
		 Wk2xxxReadFifo(port,getbuf,rfcnt);
		 ret=rfcnt;
	   }
	 return ret;	
}
