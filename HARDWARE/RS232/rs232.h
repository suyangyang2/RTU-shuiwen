#ifndef RS232_H
#define RS232_H
#include "sys.h"

#define RS232_RECV_MAX 256


extern u8 RS232_2_Recv[RS232_RECV_MAX];
extern int RS232_2_Recv_Address;
extern u8 RS232_3_Recv[RS232_RECV_MAX];
extern int RS232_3_Recv_Address;
extern u8 RS232_4_Recv[RS232_RECV_MAX];
extern int RS232_4_Recv_Address;

//void RS232_1_Send_buf(u8 *buf, u32 len);
void RS232_2_Send_buf(u8 *buf, u32 len);
void RS232_3_Send_buf(u8 *buf, u32 len);
void RS232_4_Send_buf(u8 *buf, u32 len);



























#endif
