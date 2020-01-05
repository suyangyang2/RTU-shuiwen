#ifndef _WK2124_H
#define _WK2124_H

#include "sys.h"



#define GPRS_PORT   3
#define BEIDOU_PORT 2
#define BLUETOOTH   1


#define MAX_RECEVE_LENGTH    200
#define WK2XXX_INT         PCout(6)

//如下为WK系列串口扩展芯片的寄存器地址定义，不同的芯片，可能寄存器少一些，所以具体的寄存器请看数据手册
//wkxxxx  Global rigister address defines
#define 	WK2XXX_GENA     0X00
#define 	WK2XXX_GRST     0X01
#define		WK2XXX_GMUT     0X02
#define 	WK2XXX_GIER     0X10
#define 	WK2XXX_GIFR     0X11
#define 	WK2XXX_GPDIR    0X21
#define 	WK2XXX_GPDAT    0X31
#define 	WK2XXX_GPORT    1//	/wkxxxx  Global rigister of PORT
//wkxxxx  slave uarts  rigister address defines

#define 	WK2XXX_SPAGE    0X03
//PAGE0
#define 	WK2XXX_SCR      0X04
#define 	WK2XXX_LCR      0X05
#define 	WK2XXX_FCR      0X06
#define 	WK2XXX_SIER     0X07
#define 	WK2XXX_SIFR     0X08
#define 	WK2XXX_TFCNT    0X09
#define 	WK2XXX_RFCNT    0X0A
#define 	WK2XXX_FSR      0X0B
#define 	WK2XXX_LSR      0X0C
#define 	WK2XXX_FDAT     0X0D
#define 	WK2XXX_FWCR     0X0E
#define 	WK2XXX_RS485    0X0F
//PAGE1
#define 	WK2XXX_BAUD1    0X04
#define 	WK2XXX_BAUD0    0X05
#define 	WK2XXX_PRES     0X06
#define 	WK2XXX_RFTL     0X07
#define 	WK2XXX_TFTL     0X08
#define 	WK2XXX_FWTH     0X09
#define 	WK2XXX_FWTL     0X0A
#define 	WK2XXX_XON1     0X0B
#define 	WK2XXX_XOFF1    0X0C
#define 	WK2XXX_SADR     0X0D
#define 	WK2XXX_SAEN     0X0E
#define 	WK2XXX_RRSDLY   0X0F

//WK串口扩展芯片的寄存器的位定义
//wkxxx register bit defines
// GENA
#define 	WK2XXX_UT4EN	0x08
#define 	WK2XXX_UT3EN	0x04
#define 	WK2XXX_UT2EN	0x02
#define 	WK2XXX_UT1EN	0x01
//GRST
#define 	WK2XXX_UT4SLEEP	0x80
#define 	WK2XXX_UT3SLEEP	0x40
#define 	WK2XXX_UT2SLEEP	0x20
#define 	WK2XXX_UT1SLEEP	0x10
#define 	WK2XXX_UT4RST	0x08
#define 	WK2XXX_UT3RST	0x04
#define 	WK2XXX_UT2RST	0x02
#define 	WK2XXX_UT1RST	0x01
//GIER
#define 	WK2XXX_UT4IE	0x08
#define 	WK2XXX_UT3IE	0x04
#define 	WK2XXX_UT2IE	0x02
#define 	WK2XXX_UT1IE	0x01
//GIFR
#define 	WK2XXX_UT4INT	0x08
#define 	WK2XXX_UT3INT	0x04
#define 	WK2XXX_UT2INT	0x02
#define 	WK2XXX_UT1INT	0x01
//SPAGE
#define 	WK2XXX_SPAGE0	0x00
#define 	WK2XXX_SPAGE1   0x01
//SCR
#define 	WK2XXX_SLEEPEN	0x04
#define 	WK2XXX_TXEN     0x02
#define 	WK2XXX_RXEN     0x01
//LCR
#define 	WK2XXX_BREAK	0x20
#define 	WK2XXX_IREN     0x10
#define 	WK2XXX_PAEN     0x08
#define 	WK2XXX_PAM1     0x04
#define 	WK2XXX_PAM0     0x02
#define 	WK2XXX_STPL     0x01
//FCR
//SIER
#define 	WK2XXX_FERR_IEN      0x80
#define 	WK2XXX_CTS_IEN       0x40
#define 	WK2XXX_RTS_IEN       0x20
#define 	WK2XXX_XOFF_IEN      0x10
#define 	WK2XXX_TFEMPTY_IEN   0x08
#define 	WK2XXX_TFTRIG_IEN    0x04
#define 	WK2XXX_RXOVT_IEN     0x02
#define 	WK2XXX_RFTRIG_IEN    0x01
//SIFR
#define 	WK2XXX_FERR_INT      0x80
#define 	WK2XXX_CTS_INT       0x40
#define 	WK2XXX_RTS_INT       0x20
#define 	WK2XXX_XOFF_INT      0x10
#define 	WK2XXX_TFEMPTY_INT   0x08
#define 	WK2XXX_TFTRIG_INT    0x04
#define 	WK2XXX_RXOVT_INT     0x02
#define 	WK2XXX_RFTRIG_INT    0x01


//TFCNT
//RFCNT
//FSR
#define 	WK2XXX_RFOE     0x80
#define 	WK2XXX_RFBI     0x40
#define 	WK2XXX_RFFE     0x20
#define 	WK2XXX_RFPE     0x10
#define 	WK2XXX_RDAT     0x08
#define 	WK2XXX_TDAT     0x04
#define 	WK2XXX_TFULL    0x02
#define 	WK2XXX_TBUSY    0x01
//LSR
#define 	WK2XXX_OE       0x08
#define 	WK2XXX_BI       0x04
#define 	WK2XXX_FE       0x02
#define 	WK2XXX_PE       0x01
//FWCR
//RS485
//常用波特率宏定义

#define   B600      1
#define 	B1200	    2
#define 	B2400	    3
#define 	B4800     4
#define 	B9600	    5
#define 	B19200	  6
#define 	B38400	  7
#define 	B76800	  8
#define 	B1800	    9
#define 	B3600	    10
#define		B7200	    11
#define 	B14400	  12
#define  	B28800	  13
#define	  B57600	  14
#define		B115200	  15
#define		B230400	  16



//extern u8 WK2XXX_Recv[MAX_RECEVE_LENGTH];
//extern int WK2XXX_address ;

void WK2124_Init(void);
void Wk2xxxWriteReg(unsigned char port,unsigned char reg,unsigned char dat);
unsigned char Wk2xxxReadReg(unsigned char port,unsigned char reg);
void Wk2xxxWriteFifo(unsigned char port,unsigned char *wbuf,unsigned int len);
void Wk2xxxReadFifo(unsigned char port,unsigned char *rbuf,unsigned int len);
void Wk2xxxInit(unsigned char port);
void Wk2xxxClose(unsigned char port);
void Wk2xxxSetBaud(unsigned char port,int baud);
unsigned int Wk2xxxGetBuf(unsigned char port,unsigned char *getbuf);
unsigned int Wk2xxxSendBuf(unsigned char port,unsigned char *sendbuf,unsigned int len);

void WK2XXX_IRQInit(void);
void SPI_CS_Init(void);
void SPI_BUS_Init(void);
void WK2XXX_SPI_Init(void);
void WK2XXX_Reset_Init(void);
void WK2XXX_RST_Init(void);
void mprintf(u8 *date);

#endif
