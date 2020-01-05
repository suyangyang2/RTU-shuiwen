#include "wk2124.h"
#include "pwctrl.h"
#include "spi.h"
#include "usart.h"
#include "delay.h"
#include "rs485.h"
#include "string.h"


//u8 WK2XXX_Recv[MAX_RECEVE_LENGTH];
//int WK2XXX_address = 0;

//��ӡ�ַ������� 
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
	//WK2XXX_IRQInit();	    //�ݲ���Ҫ�жϿ���4G��
	Wk2xxxInit(1);
	Wk2xxxInit(2);
	Wk2xxxInit(3);
//Wk2xxxInit(4);
	Wk2xxxSetBaud(1,B9600); //����Ĭ�ϲ�����
	Wk2xxxSetBaud(2,B115200);
	Wk2xxxSetBaud(3,B115200);
//Wk2xxxSetBaud(4,B115200);
}


/*
********************************************************************************************
* ������:WK2XXX_RST_Init
* ����ֵ����
* ��  ������
* ��  ��������WK2XXX��λGPIOO��ʼ��
*
*********************************************************************************************
*/
void WK2XXX_RST_Init(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);	 //ʹ��PA,PD�˿�ʱ��
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 //PG.7  �˿�����
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	 GPIO_Init(GPIOG, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.4
	 GPIO_SetBits(GPIOG,GPIO_Pin_7);						 //PG.7 �����	
}

/*
********************************************************************************************
* ������:WK2XXX_SPI_Init
* ����ֵ����
* ��  ������
* ��  ��������WK2XXX��SPIƬѡGPIO��ʼ����SPI���߳�ʼ��
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
* ������:WK2XXX_IRQInit
* ����ֵ����
* ��  ������
* ��  ��������WK2XXX���ж�GPIO��ʼ�����ж����ó�ʼ��
*
*********************************************************************************************
*/
void WK2XXX_IRQInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��

	
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
  //GPIOC.6�ж����Լ��жϳ�ʼ������   �½��ش���
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource6);
		

  	EXTI_InitStructure.EXTI_Line=EXTI_Line6;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���


    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ�ܰ���KEY2���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//�����ȼ�2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure);
}

/*
********************************************************************************************
* ������:WK2XXX_Reset_Init
* ����ֵ����
* ��  ������
* ��  ��������WK2XXX�ĸ�λ����
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
* ������:SPI_CS_Init
* ����ֵ����
* ��  ������
* ��  ��������SPIƬѡGPIO��ʼ��
*
*********************************************************************************************
*/
void SPI_CS_Init(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD, ENABLE);	 //ʹ��PA,PD�˿�ʱ��
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	//PB.12 �˿�����
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.12
	 GPIO_SetBits(GPIOB,GPIO_Pin_12);						 //PB.12  �����	
}

/*
********************************************************************************************
* ������:SPI_BUS_Init
* ����ֵ����
* ��  ������
* ��  ��������SPI2���߳�ʼ��
*
*********************************************************************************************
*/
void SPI_BUS_Init(void)
{

	SPI2_Init();		   //��ʼ��SPI
	SPI2_SetSpeed(SPI_BaudRatePrescaler_8);	//����Ϊ10Mʱ��,����ģʽ
}

/*
********************************************************************************************
* ������:SPI_CS_H
* ����ֵ����
* ��  ������
* ��  ��������SPIƬѡ����
*
*********************************************************************************************
*/
void SPI_CS_H(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

/*
********************************************************************************************
* ������:SPI_CS_L
* ����ֵ����
* ��  ������
* ��  ��������SPIƬѡ����
*
*********************************************************************************************
*/
void SPI_CS_L(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

/*
********************************************************************************************
* ������:Wk2xxxWriteReg
* ����ֵ����
* ��  ����port:�˿�ֵ
*					reg:�Ĵ����ĵ�ַ(A3A2A1)
*					dat:ΪҪд�������
* ��  ��������WK2XXXд�Ĵ������ݺ���
*
*********************************************************************************************
*/
void Wk2xxxWriteReg(unsigned char port,unsigned char reg,unsigned char dat)
{	 
	 SPI_CS_L();//Ƭѡʹ��
	 SPI2_ReadWriteByte(((port-1)<<4)+reg); //д�����ֽ�
	 SPI2_ReadWriteByte(dat); //д����
	 SPI_CS_H();//Ƭѡ��Ч

}

/*
********************************************************************************************
* ������:Wk2xxxWriteReg
* ����ֵ: �����ļĴ����е�����
* ��  ����port:�˿�ֵ
*					reg:�Ĵ����ĵ�ַ(A3A2A1)
* ��  ��������WK2XXX���Ĵ������ݺ���
*
*********************************************************************************************
*/
unsigned char Wk2xxxReadReg(unsigned char port,unsigned char reg)
{	
	unsigned char rec_data; 
	SPI_CS_L();	//Ƭѡʹ��
	SPI2_ReadWriteByte(0x40+((port-1)<<4)+reg);//д�����ֽڣ���������ɼ������ֲ�
	rec_data=SPI2_ReadWriteByte(0);//���շ��ص�����
	SPI_CS_H();	//Ƭѡ��Ч	
	return rec_data;
}

/*
********************************************************************************************
* ��������Wk2xxxWriteFifo
* ����ֵ����
* ��  ����port:�˿�ֵ
*					*wbuf:д�����ݲ���
*					len��  д�����ݳ���
* ��  �����ú���ΪдFIFO������ͨ���ú���д������ݻ�ֱ�ӽ����Ӵ��ڵķ���FIFO��Ȼ��ͨ��TX���ŷ���
*
*********************************************************************************************
*/
void Wk2xxxWriteFifo(unsigned char port,unsigned char *wbuf,unsigned int len)
{	 unsigned char n;
	 SPI_CS_L(); // Ƭѡ��Ч
	 SPI2_ReadWriteByte(0x80+((port-1)<<4)); //дFIFO����ָ��
	  for(n=0;n<len;n++)
	    {
	     SPI2_ReadWriteByte(*(wbuf+n));
		} 
	 SPI_CS_H();	//Ƭѡ��Ч

}

/*
********************************************************************************************
* ��������Wk2xxxReadFifo
* ����ֵ����
* ��  ����port:�˿�ֵ
*					*rbuf:д�����ݲ���
*					len��  д�����ݳ���
* ��  �����ú���Ϊ��FIFO������ͨ���ú�������һ�ζ����������FIFO�е����ݣ����256���ֽ�
*
*********************************************************************************************
*/
void Wk2xxxReadFifo(unsigned char port,unsigned char *rbuf,unsigned int len)
{	 unsigned char n;
	 SPI_CS_L();//Ƭѡ��Ч
	 SPI2_ReadWriteByte(0xc0+((port-1)<<4));	//д��fifo����ָ��
	 for(n=0;n<len;n++)
	   {
		*(rbuf+n)=SPI2_ReadWriteByte(0); 
	   }
	 SPI_CS_H();//Ƭѡ��Ч										
	 //return 0;
}

/*
********************************************************************************************
* ��������Wk2xxxInit
* ����ֵ����
* ��  ����port:�˿�ֵ
* ��  ������ʼ�������˿ڵļĴ���
*
*********************************************************************************************
*/
void Wk2xxxInit(unsigned char port)
{
    unsigned char gena,grst,sier,scr;
		unsigned char gier;
	//ʹ���Ӵ���ʱ��
    gena=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GENA); 
		switch (port)
    {
          case 1://ʹ���Ӵ���1��ʱ��
              gena|=WK2XXX_UT1EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		  case 2://ʹ���Ӵ���2��ʱ��
              gena|=WK2XXX_UT2EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 3://ʹ���Ӵ���3��ʱ��
              gena|=WK2XXX_UT3EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 4://ʹ���Ӵ���4��ʱ��
              gena|=WK2XXX_UT4EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
	 }	
	//�����λ�Ӵ���
	grst=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GRST); 
	switch (port)
    {
          case 1://�����λ�Ӵ���1
              grst|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		  case 2://�����λ�Ӵ���2
              grst|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 3://�����λ�Ӵ���3
              grst|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 4://�����λ�Ӵ���4
             grst|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
	 }	
  //ʹ���Ӵ����жϣ������Ӵ������жϺ��Ӵ����ڲ��Ľ����жϣ��������жϴ���
	gier=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GIER); 
	switch (port)
   {
         case 1://�����λ�Ӵ���1
             gier|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
             break;
		  case 2://�����λ�Ӵ���2
             gier|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
             break;
		   case 3://�����λ�Ӵ���3
             gier|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
             break;
		   case 4://�����λ�Ӵ���4
             gier|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GIER,gier);
             break;
	 }	 
	 //ʹ���Ӵ��ڽ��մ����жϺͳ�ʱ�ж�
	 sier=Wk2xxxReadReg(port,WK2XXX_SIER); 
	 sier |= WK2XXX_RFTRIG_IEN|WK2XXX_RXOVT_IEN;
	 Wk2xxxWriteReg(port,WK2XXX_SIER,sier);
	 // ��ʼ��FIFO�����ù̶��жϴ���
	 Wk2xxxWriteReg(port,WK2XXX_FCR,0XFF);
	 //���������жϴ��㣬��������������Ч����ô����FCR�Ĵ����жϵĹ̶��жϴ��㽫ʧЧ
	 Wk2xxxWriteReg(port,WK2XXX_SPAGE,1);//�л���page1
	 Wk2xxxWriteReg(port,WK2XXX_RFTL,0X40);//���ý��մ���Ϊ64���ֽ�
	 Wk2xxxWriteReg(port,WK2XXX_TFTL,0X40);//���÷��ʹ���Ϊ64���ֽ�
	 Wk2xxxWriteReg(port,WK2XXX_SPAGE,0);//�л���page0 
	 //ʹ���Ӵ��ڵķ��ͺͽ���ʹ��
	 scr=Wk2xxxReadReg(port,WK2XXX_SCR); 
	 scr|=WK2XXX_TXEN|WK2XXX_RXEN;
	 Wk2xxxWriteReg(port,WK2XXX_SCR,scr);
}

/*
********************************************************************************************
* ��������Wk2xxxClose
* ����ֵ����
* ��  ����port:�˿�ֵ
* ��  �����رն˿ڣ���λĬ��ֵ
*
*********************************************************************************************
*/
void Wk2xxxClose(unsigned char port)
{
    unsigned char gena,grst;
	//��λ�Ӵ���
	grst=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GRST); 
	switch (port)
    {
          case 1://�����λ�Ӵ���1
              grst|=WK2XXX_UT1RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		  case 2://�����λ�Ӵ���2
              grst|=WK2XXX_UT2RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 3://�����λ�Ӵ���3
              grst|=WK2XXX_UT3RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
		   case 4://�����λ�Ӵ���4
              grst|=WK2XXX_UT4RST;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GRST,grst);
              break;
	 }	
	//�ر��Ӵ���ʱ��
    gena=Wk2xxxReadReg(WK2XXX_GPORT,WK2XXX_GENA); 
	switch (port)
    {
          case 1://ʹ���Ӵ���1��ʱ��
              gena&=~WK2XXX_UT1EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		  case 2://ʹ���Ӵ���2��ʱ��
              gena&=~WK2XXX_UT2EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 3://ʹ���Ӵ���3��ʱ��
              gena&=~WK2XXX_UT3EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
		   case 4://ʹ���Ӵ���4��ʱ��
              gena&=~WK2XXX_UT4EN;
		      Wk2xxxWriteReg(WK2XXX_GPORT,WK2XXX_GENA,gena);
              break;
	 }	
}
/*
********************************************************************************************
* ��������Wk2xxxSetBaud
* ����ֵ����
* ��  ����port:�˿�ֵ
*    			baud:������
* ��  ���������Ӵ��ڲ����ʺ������˺����в����ʵ�ƥ��ֵ�Ǹ���11.0592Mhz�µ��ⲿ��������
*
*********************************************************************************************
*/
void Wk2xxxSetBaud(unsigned char port,int baud)
{  
	unsigned char baud1,baud0,pres,scr;
	//���²�������Ӧ�ļĴ���ֵ�������ⲿʱ��Ϊ11.0592������¼������ã����ʹ������������Ҫ���¼���
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
	//�ص��Ӵ����շ�ʹ��
	scr=Wk2xxxReadReg(port,WK2XXX_SCR); 
	Wk2xxxWriteReg(port,WK2XXX_SCR,0);
	//���ò�������ؼĴ���
	Wk2xxxWriteReg(port,WK2XXX_SPAGE,1);//�л���page1
	Wk2xxxWriteReg(port,WK2XXX_BAUD1,baud1);
	Wk2xxxWriteReg(port,WK2XXX_BAUD0,baud0);
	Wk2xxxWriteReg(port,WK2XXX_PRES,pres);
	Wk2xxxWriteReg(port,WK2XXX_SPAGE,0);//�л���page0 
	//ʹ���Ӵ����շ�ʹ��
	Wk2xxxWriteReg(port,WK2XXX_SCR,scr);
	
	
}

/*
********************************************************************************************
* ��������Wk2xxxSendBuf
* ����ֵ��ʵ�ʳɹ����͵�����
* ��  ����port:�˿�ֵ
*    			*sendbuf:��Ҫ���͵�����buf
*					len����Ҫ�������ݵĳ���
* ��  ����������Ϊ�Ӵ��ڷ������ݵĺ������������ݵ��Ӵ��ڵ�FIFO.Ȼ��ͨ���ٷ���,ע�⣺1.����ȷ���Ӵ��ڵķ���FIFO�ж������� 2.ȷ��д��FIFO���ݵĸ���
*
*********************************************************************************************
*/
unsigned int Wk2xxxSendBuf(unsigned char port,unsigned char *sendbuf,unsigned int len)
{
	 unsigned int ret,tfcnt,sendlen;
	 unsigned char  fsr;
	  
	 fsr=Wk2xxxReadReg(port,WK2XXX_FSR);
	 if(~fsr&WK2XXX_TFULL )//�Ӵ��ڷ���FIFOδ��
	 {

	   tfcnt=Wk2xxxReadReg(port,WK2XXX_TFCNT);//���Ӵ��ڷ���fifo�����ݸ���
		 sendlen=256-tfcnt;//FIFO��д�������ֽ���
		 
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
* ��������Wk2xxxGetBuf
* ����ֵ��ʵ�ʳɹ����͵�����
* ��  ����port:�˿�ֵ
*    			 *getbuf:���յ�������buf
* ��  ����������Ϊ�Ӵ��ڽ������ݺ���
*
*********************************************************************************************
*/
unsigned int Wk2xxxGetBuf(unsigned char port,unsigned char *getbuf)
{
	unsigned int ret=0,rfcnt;
	unsigned char fsr;
	fsr=Wk2xxxReadReg(port,WK2XXX_FSR);
	if(fsr&WK2XXX_RDAT )//�Ӵ��ڽ���FIFOδ��
	  {
	     rfcnt=Wk2xxxReadReg(port,WK2XXX_RFCNT);//���Ӵ��ڷ���fifo�����ݸ���
		 if(rfcnt==0)//��RFCNT�Ĵ���Ϊ0��ʱ�������������������256������0�����ʱ��ͨ��FSR���жϣ����FSR��ʾ����FIFO��Ϊ�գ���Ϊ256���ֽ�
		 {rfcnt=256;}
		 Wk2xxxReadFifo(port,getbuf,rfcnt);
		 ret=rfcnt;
	   }
	 return ret;	
}
