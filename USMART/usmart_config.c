#include "usmart.h"
#include "usmart_str.h"	   
////////////////////////////用户配置区///////////////////////////////////////////////
//这下面要包含所用到的函数所申明的头文件(用户自己添加) 
#include "delay.h"	
#include "usart.h"		
#include "sram.h"
#include "malloc.h"
#include "sd3078rtc.h"
#include "st16c554.h"
 
//函数名列表初始化(用户自己添加)
//用户直接在这里输入要执行的函数名及其查找串
struct _m_usmart_nametab usmart_nametab[]=
{
#if USMART_USE_WRFUNS==1 	//如果使能了读写操作
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
//函数控制管理器初始化
//得到各个受控函数的名字
//得到函数总数量
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),//函数数量
	0,	  	//参数数量
	0,	 	//函数ID
	1,		//参数显示类型,0,10进制;1,16进制
	0,		//参数类型.bitx:,0,数字;1,字符串	    
	0,	  	//每个参数的长度暂存表,需要MAX_PARM个0初始化
	0,		//函数的参数,需要PARM_LEN个0初始化
};   



















