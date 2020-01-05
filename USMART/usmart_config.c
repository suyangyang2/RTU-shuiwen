#include "usmart.h"
#include "usmart_str.h"	   
////////////////////////////�û�������///////////////////////////////////////////////
//������Ҫ�������õ��ĺ�����������ͷ�ļ�(�û��Լ����) 
#include "delay.h"	
#include "usart.h"		
#include "sram.h"
#include "malloc.h"
#include "sd3078rtc.h"
#include "st16c554.h"
 
//�������б��ʼ��(�û��Լ����)
//�û�ֱ������������Ҫִ�еĺ�����������Ҵ�
struct _m_usmart_nametab usmart_nametab[]=
{
#if USMART_USE_WRFUNS==1 	//���ʹ���˶�д����
	(void*)read_addr,(const u8 * )"u32 read_addr(u32 addr)",
	(void*)write_addr,(const u8 * )"void write_addr(u32 addr,u32 val)",	 
#endif
	(void*)delay_ms,(const u8 * )"void delay_ms(u16 nms)",
	(void*)delay_us,(const u8 * )"void delay_us(u32 nus)",	 
	(void*)fsmc_sram_test_write,(const u8 * )"void fsmc_sram_test_write(u32 addr,u8 data)", 
	(void*)fsmc_sram_test_read,(const u8 * )"u8 fsmc_sram_test_read(u32 addr)", 
 
	(void*)read_date,(const u8 * )"void read_date(void)",
	(void*)write_date,(const u8 * )"void write_date(u8 year, u8 month, u8 day, u8 hour, u8 minute, u8 seconds)",
	(void*)ST16C554D_WriteBuff,(const u8 * )"void ST16C554D_WriteBuff(unsigned char port,unsigned char* pBuff,int len)",
};						  
///////////////////////////////////END///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//�������ƹ�������ʼ��
//�õ������ܿغ���������
//�õ�����������
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),//��������
	0,	  	//��������
	0,	 	//����ID
	1,		//������ʾ����,0,10����;1,16����
	0,		//��������.bitx:,0,����;1,�ַ���	    
	0,	  	//ÿ�������ĳ����ݴ��,��ҪMAX_PARM��0��ʼ��
	0,		//�����Ĳ���,��ҪPARM_LEN��0��ʼ��
};   



















