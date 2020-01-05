#ifndef DMAUSART_H
#define DMAUSART_H


#include "sys.h"







#define DMA_Rec_Len   200

extern u8 DMA_usart2_Buf[DMA_Rec_Len];
extern u32 g_usart2_receive_cnt;
extern u8 g_usart2_receive_stat;






void gprs_usart_init(u32 bound);
void Usart2_Send(u8 *buf,u32 len);























#endif
