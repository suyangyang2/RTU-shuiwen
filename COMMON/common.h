#ifndef _COMMON_H
#define _COMMON_H

#include "memory_my.h"
#include "delay.h"
//#include "sys.h"
#include "usart.h"
#include "includes.h"
#include "wk2124.h"
#include "rs485.h"
#include "rs232.h"
#include "dm9000.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "sram.h"
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h" 
#include "rtc.h"
#include "ftpd.h"
#include "sd3078rtc.h"
#include "adc.h"
#include "4g.h"
#include "short_letter.h"
#include "pwctrl.h"
#include "gps.h"
//#include "usmart.h"
#include "st16c554.h"
#include "wkup.h"
//#include "rtc.h"
#include "sensor_water.h"
#include "hc05.h"
#include "bluetooth.h"
#include "protocol.h"
#include "dmausart.h"
#include "malloc.h"



void Reduce_Power(void);
void System_ReInit(void);

#endif
