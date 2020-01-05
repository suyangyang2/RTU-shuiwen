#ifndef _THREAD_H
#define _THREAD_H

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
#include "iwdg.h"
//#include "rtc.h"
#include "sensor_water.h"
#include "hc05.h"
#include "bluetooth.h"
#include "protocol.h"
#include "dmausart.h"
#include "malloc.h"

//#include "mass_mal.h"
//#include "usb_lib.h"
//#include "hw_config.h"
//#include "usb_pwr.h"
//#include "memory.h"	    
//#include "usb_bot.h" 



#define IWDG_TASK_PRIO 3
#define IWDG_STK_SIZE  32
static OS_STK IWDG_TASK_STK[IWDG_STK_SIZE];
void iwdg_task(void *pdata);

//6、7优先级已经在lwip任务中使用
//FTP TASK
#define FTP_TASK_PRIO	9
#define FTP_STK_SIZE	350  //500
static OS_STK	FTP_TASK_STK[FTP_STK_SIZE];
void ftp_task(void *pdata);

//GPRS 任务
#define GPRS_TASK_PRIO			10
#define GPRS_STK_SIZE		800 //800
static OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
void gprs_task(void *pdata);


//START 任务
#define START_TASK_PRIO			11  ///开始任务的优先级为最低
#define START_STK_SIZE			128
extern OS_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *pdata);

//BASIC_MESSAGE
#define BASIC_MESSAGE_TASK_PRIO  13
#define BASIC_MESSAGE_TASK_STK_SIZE 	256 //300
static OS_STK BASIC_MESSAGE_TASK_STK[BASIC_MESSAGE_TASK_STK_SIZE];
void basic_message_task(void *pdata);


//232_1 Receive command
#define  Receive_COMMAND_TASK_PRIO 15
#define  Receive_COMMAND_TASK_STK_SIZE   256 // 400
static   OS_STK Receive_COMMAND_TASK_STK[Receive_COMMAND_TASK_STK_SIZE];
void    receive_command(void *pdata);

//bluetooth task
#define BLUETOOTH_TASK_PRIO  16
#define BLUETOOTH_TASK_STK_SIZE 256
static  OS_STK BLUETOOTH_TASK_STK[BLUETOOTH_TASK_STK_SIZE];
void bluetooth_task(void *pdata);

//enter stop mode
#define STOP_MODE_TASK_PRIO 17
#define STOP_MODE_TASK_SIZE 256
static OS_STK STOP_MODE_TASK_STK[STOP_MODE_TASK_SIZE];
void stopmode_task(void *pdata);


//统计任务优先级
#define SYSTEM_DATA_BROADCAST_PRIO	18
#define SYSTEM_DATA_BROADCAST_STK_SIZE 10 //150
void system_data_broadcast(void *pdata);


#define ENTER_STOP_MODE  1
#define WAKE_UP_MODE     2






#endif

