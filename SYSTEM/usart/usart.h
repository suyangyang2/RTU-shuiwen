#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
//change

#define RS232_1_RRECEIVE_WAKEUP_ENABLE     1 //使能串口接受中断唤醒
#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
#define USART1_IDLE_ENABLE      1      //空闲中断使能

typedef enum sw_log_level_s{
	SW_LOG_LEVEL_QUIET = 0,
  	SW_LOG_LEVEL_DEBUG ,
	SW_LOG_LEVEL_VERBOSE,
	SW_LOG_LEVEL_INFO,
	SW_LOG_LEVEL_WARNING,
	SW_LOG_LEVEL_ERROR,
	SW_LOG_LEVEL_FATAL,
	SW_LOG_LEVEL_PANIC

}sw_log_level_t;



	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记
extern u8 g_usart1_receive_stat;
extern u32 g_usart1_recveive_cnt;
extern u8 DMA_usart1_Buf[USART_REC_LEN];


void RS232_1_Init(u32 bound);
void Usart1_Send(u8 *buf,u32 len);

int sw_log_get_level(void);
void sw_log_set_level(int level);
void sw_log(char *module, int level, const char *fmt, ...);
const char *get_level_str(int level);


#endif
