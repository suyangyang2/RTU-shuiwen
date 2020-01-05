#include "rs232.h"
#include "rs485.h"
#include "dm9000.h"
#include "st16c554.h"
#include "delay.h"

/*232_1  usart.c*/

u8 RS232_2_Recv[RS232_RECV_MAX] = {0};
int RS232_2_Recv_Address = 0;
u8 RS232_3_Recv[RS232_RECV_MAX]  = {0};
int RS232_3_Recv_Address = 0;
u8 RS232_4_Recv[RS232_RECV_MAX] = {0};
int RS232_4_Recv_Address = 0;



void RS232_2_Send_buf(u8 *buf, u32 len)
{
	ST16C554D_WriteBuff(1, buf, len);
}

void RS232_3_Send_buf(u8 *buf, u32 len)
{
	ST16C554D_WriteBuff(2, buf, len);
}

void RS232_4_Send_buf(u8 *buf, u32 len)
{
	ST16C554D_WriteBuff(3, buf, len);
}




