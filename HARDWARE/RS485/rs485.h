#ifndef RS485_H
#define RS485_h


#define RS485_1_EN_PORT GPIOB
#define RS485_1_EN_PIN  GPIO_Pin_10
#define RS485_2_EN_PORT GPIOB
#define RS485_2_EN_PIN  GPIO_Pin_11

#define RS485_1_EN PBout(10)
#define RS485_2_EN PBout(11)
#define RS485_MAX_RECEIVE_LEN  255


extern unsigned char RS485_1_Receive[RS485_MAX_RECEIVE_LEN];
extern int RS485_1_Address;


void RS485_GPIO_Init(void);
void rs485_1_sendbuff(const unsigned char *buff, const unsigned int length);
void rs485_2_sendbuff(const unsigned char *buff, const unsigned int length);



































#endif
