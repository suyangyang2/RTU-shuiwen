#include "thread.h"


#define GPRS_DEBUG 0
#define FTP_TIME_INTERVAL_MINUTES 5
#define FTP_SAVE_MAX_WAIT_TIME 120
#define N_MESSAGE 5
#define PICTURE_DEBUG 0

void *Message_Group[N_MESSAGE];

OS_EVENT *gprs_task_en ;
OS_EVENT *Message_Q;
unsigned char g_respose_flag = 0;
unsigned char g_self_report_flag = 0;

OS_STK START_TASK_STK[START_STK_SIZE] = {0};
static OS_STK FTP_TASK_STK[FTP_STK_SIZE] = {0};                                 //文件服务器
static OS_STK GPRS_TASK_STK[GPRS_STK_SIZE] = {0};                               //4G网络GPRS
static OS_STK BASIC_MESSAGE_TASK_STK[BASIC_MESSAGE_TASK_STK_SIZE] = {0};        //基础信息测试
static OS_STK Receive_COMMAND_TASK_STK[Receive_COMMAND_TASK_STK_SIZE] = {0};		//接收指令
static OS_STK BLUETOOTH_TASK_STK[BLUETOOTH_TASK_STK_SIZE] = {0};								//蓝牙
static OS_STK STOP_MODE_TASK_STK[STOP_MODE_TASK_SIZE] = {0};										//休眠	
static OS_STK  SYSTEM_DATA_BROADCAST_STK[SYSTEM_DATA_BROADCAST_STK_SIZE] = {0};		//系统数据

OS_STK_DATA StackBytes;
__align(8) static OS_STK AppTaskXXXStk[200];

void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;

	OSStatInit();  //开启统计任务
	OS_ENTER_CRITICAL();  
	gprs_task_en = OSSemCreate(0);
	Message_Q = OSQCreate(&Message_Group[0], N_MESSAGE);
	//IWDG_Init(4, 1250); //溢出时间2秒

	//OSTaskCreateExt(iwdg_task, (void *)0, (OS_STK *)&IWDG_TASK_STK[IWDG_STK_SIZE - 1], IWDG_TASK_PRIO, \
		0, (OS_STK *)&IWDG_TASK_STK, IWDG_STK_SIZE, 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OSTaskCreateExt(gprs_task, (void *)0, (OS_STK *)&GPRS_TASK_STK[GPRS_STK_SIZE - 1],GPRS_TASK_PRIO, \
		0,(OS_STK *)&GPRS_TASK_STK, GPRS_STK_SIZE, 0,  OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#if !PICTURE_DEBUG	
	OSTaskCreateExt(ftp_task, (void *)0, (OS_STK *)&FTP_TASK_STK[FTP_STK_SIZE - 1], FTP_TASK_PRIO, \
		0,(OS_STK *)&FTP_TASK_STK, FTP_STK_SIZE, 0,  OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OSTaskCreateExt(basic_message_task, (void *)0, (OS_STK*)&BASIC_MESSAGE_TASK_STK[BASIC_MESSAGE_TASK_STK_SIZE-1], BASIC_MESSAGE_TASK_PRIO, \
		0,(OS_STK*)&BASIC_MESSAGE_TASK_STK, BASIC_MESSAGE_TASK_STK_SIZE,0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OSTaskCreateExt(receive_command,(void *)0, (OS_STK *)&Receive_COMMAND_TASK_STK[Receive_COMMAND_TASK_STK_SIZE-1], Receive_COMMAND_TASK_PRIO , \
		0, (OS_STK *)&Receive_COMMAND_TASK_STK,Receive_COMMAND_TASK_STK_SIZE,0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OSTaskCreateExt(bluetooth_task, (void *)0, (OS_STK *)&BLUETOOTH_TASK_STK[BLUETOOTH_TASK_STK_SIZE -1], BLUETOOTH_TASK_PRIO, \
		0, (OS_STK *)&BLUETOOTH_TASK_STK, BLUETOOTH_TASK_STK_SIZE, 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OSTaskCreateExt(stopmode_task, (void *)0, (OS_STK *)&STOP_MODE_TASK_STK[STOP_MODE_TASK_SIZE - 1], STOP_MODE_TASK_PRIO, \
		0, (OS_STK *)&STOP_MODE_TASK_STK, STOP_MODE_TASK_SIZE, 0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OSTaskSuspend(FTP_TASK_PRIO);
#endif
	OSTaskSuspend(START_TASK_PRIO);
	OS_EXIT_CRITICAL();  
}

void iwdg_task(void *pdata)
{
	while(1)
	{
		IWDG_Feed();
		OSTimeDlyHMSM(0, 0, 0, 500);
	}
}


void basic_message_task(void *pdata)
{
	Time_Def real_time;
	unsigned char minutes = 0;                           //分钟
	unsigned char hours = 0;										         //小时
	unsigned char last_minutes = 0xff;					         //上一分钟
	unsigned char last_hours = 0xff;						         //上以小时	
	unsigned char rainfall_data[3] = {0};				         //雨量数据
	unsigned char waterlevel_data[4] = {0};				       //水位数据
	float five_minute_rainfall = 0;								       //五分钟雨量
	float rainfall_threshold = 0;										     //降雨阈值
	RTU_operation_parameters_t rtu_paramters = {0};
	volatile int wait_time = 0;
	unsigned char message = 0;
	unsigned char ftp_time_interval = FTP_TIME_INTERVAL_MINUTES;   //FTP分钟时间间隔

	while(1)
	{
		RTC_ReadDate(&real_time);                            //读RTC实时数据寄存器 参数 TRUE/FALSE
		minutes = BCDTimeTodata(real_time.minute);           //分钟
		hours = BCDTimeTodata(real_time.hour);               //小时
		if ((minutes % 5 == 0) & (minutes != last_minutes))  //5 min 加报  数据采集,数据存储
		{
			
			RTU_parameters_Readdata(&rtu_paramters);
			SaveAndGetData_FiveMinute(rainfall_data, waterlevel_data);  //存储采集数据到FLASH，并将数据提取出来
			last_minutes = minutes;														          //记录上次时间		
			plus_message_init();															          //报文初始化
			if((minutes == 0) & (hours != last_hours))				          //小时报
			{
				last_hours = hours;
				hour_message_init();                                                               //使得发报时间与观察时间
				RS232_3_Send_buf((unsigned char *)&g_hour_message, sizeof(g_hour_message));        //RS232发送时间信息
				//gprs_send_string_len((unsigned char *)&g_hour_message, sizeof(g_hour_message));  //GPRS发送时间数据
				sw_log("basic_message_task", SW_LOG_LEVEL_QUIET, "send hour message\r\n");
			}
			five_minute_rainfall =  BCDSendataToFloat(FIVE_MIN_PRECIPIT_ID, g_plus_message.plus_message_content.five_min_precipit_data);
			rainfall_threshold = BCDSendataToFloat(0x0008, (unsigned char *)&rtu_paramters.threshold_rainfall_value_of_plusemessage);
			if(five_minute_rainfall >= rainfall_threshold)                      //五分钟的雨量大于雨量阈值
			{
				sw_log("basic_message_task", SW_LOG_LEVEL_QUIET, "send plus message\r\n");
				RS232_3_Send_buf((unsigned char *)&g_plus_message, sizeof(g_plus_message));
				//gprs_send_string_len((unsigned char *)&g_plus_message, sizeof(g_plus_message));	
			}	
			ftp_time_interval -= 5;     //FTP时间间隔（每隔5分钟）
			if(!ftp_time_interval)      //发报然后直接进入待机模式，后面应该加上响应下报的时间；
			{
				POE_ON();     //POE脚PD3打开
				ftp_time_interval = FTP_TIME_INTERVAL_MINUTES; //重新复位初始值；		
				OSTaskResume(FTP_TASK_PRIO);             //开启FTP任务
				sw_log("basic_message_task", SW_LOG_LEVEL_QUIET	, "wating to save picture,max time is 120s\r\n");
				wait_time = FTP_SAVE_MAX_WAIT_TIME;
				while(wait_time)                  //判断是否FTP存储图片成功，
				{
					if(g_save_picture_flag == 1)
					{
						g_save_picture_flag = 0;     //清除标志位
						break;
					}
					else
					{
						OSTimeDlyHMSM(0,0,1,0);
					}
					wait_time--;
				}
				if(wait_time == 0)    //存储失败直接进入待机模式，成功在gprs_任务中发送消息队列，进入待机模式
				{
					sw_log("basic_message_task", SW_LOG_LEVEL_QUIET, "ftp save picture failed\r\n");
					message = ENTER_STOP_MODE;
					OSQPost(Message_Q, &message);  //发出进入待机模式的消息
				}
				else
				{
					sw_log("basic_message_task", SW_LOG_LEVEL_QUIET, "saving picture successfully, next start to send picture by gprs\r\n");
					OSTaskSuspend(FTP_TASK_PRIO);
					OSSemPost(gprs_task_en);
				
				}
			}
			else
			{
				message = ENTER_STOP_MODE;
				OSQPost(Message_Q, &message);  //发出进入待机模式的消息
			}	
		}
		OSTimeDlyHMSM(0, 0 , 0, 800);
	}
}

//图片存储任务
void ftp_task(void *pdata)
{
	ftpd_start();
}

void gprs_task(void *pdata)
{
	int feedback = 0;
	OS_CPU_SR cpu_sr = 0;
	u8 message = ENTER_STOP_MODE;
	INT8U err;

	while(1)
	{
	#if !PICTURE_DEBUG
		OSSemPend(gprs_task_en, 0, &err);
		OSTaskSuspend(Receive_COMMAND_TASK_PRIO);   //防止下发数据，被认为是指令
	#endif
		OS_ENTER_CRITICAL();
		feedback = SendPictureToServer();
		if(feedback != 0)
		{
			sw_log("gprs_task", SW_LOG_LEVEL_ERROR, "send picture failed\r\n");
		}
		OSTaskResume(Receive_COMMAND_TASK_PRIO);
		OSQPost(Message_Q, &message);	
		OSTimeDlyHMSM(0, 0, 0, 500);
		OS_EXIT_CRITICAL();	
	}
}


/*RS232接受指令后然后执行任务*/
void receive_command(void *pdata)
{
	unsigned char fun_id  = 0;

	while(1)
	{	
		if(RS232_3_Recv_Address != 0) //判断RS232_3有数据
		{
			if(strstr((char *)RS232_3_Recv, "\x7E\x7E") != NULL)
			{
				fun_id = GetDownFrameFunId(RS232_3);
				sw_log("receive_command", SW_LOG_LEVEL_DEBUG, "rs232_3 receive fun_id is %02x\r\n", fun_id);
				if(fun_id == ENTER_RESPONSE_MODE)
				{
					sw_log("receive_command", SW_LOG_LEVEL_QUIET, "enter response mode\r\n");
					OSTaskSuspend(STOP_MODE_TASK_PRIO);
					OSTaskSuspend(BASIC_MESSAGE_TASK_PRIO);
				}
				else if(fun_id == ENTER_NORMAL_MODE)
				{
					sw_log("receive_command", SW_LOG_LEVEL_QUIET, "enter normal mode\r\n");
					OSTaskResume(STOP_MODE_TASK_PRIO);
				}
				ExecuteFunctionByFunId(fun_id, RS232_3);
				ResponseUpMessage(fun_id, RS232_3);	
				RS232_3_Recv_Address = 0;
			}	
		}
		else if(g_usart2_receive_stat == 1)  //同时这里可以解析4G模块对应串口接收到的数据
		{
			
			if(strstr((char *)DMA_usart2_Buf, "\x7E\x7E") != NULL)  //筛选出报文指令数据
			{
				#if 0
					DMA_usart2_Buf[g_usart2_receive_cnt] = '\0';
					printf("%s\r\n", DMA_usart2_Buf);
				#endif
				fun_id = GetDownFrameFunId(SERIAL_2);
				sw_log("receive_command", SW_LOG_LEVEL_DEBUG, "gprs receive fun_id is %02x\r\n", fun_id);
				ExecuteFunctionByFunId(fun_id, SERIAL_2);
				ResponseUpMessage(fun_id, SERIAL_2);
				g_usart2_receive_stat = 0;
			}
		}
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

void bluetooth_task(void *pdata)
{
	u32 length = 0;
	u8 buff[50] = {0};
	u8 fun_id = 0;
	u32 i = 0;
	u8 flag = 0;

	while(1)
	{
		length = bluetooth_getbuff(buff);
		if(length > 0)
		{
			#if 1
			sw_log("bluetooth_task", SW_LOG_LEVEL_QUIET, "bluetooth Receive data:");
			for(i = 0; i < length; i++)
			{
				printf("%02x\t", buff[i]);
			}
			printf("\r\n");
			#endif
			fun_id = ((bluetooth_message_header_t *)buff)->fun_id;
			sw_log("bluetooth_task",SW_LOG_LEVEL_DEBUG, "fun_id %02x\r\n", fun_id);
			flag =  Execute_commanad(buff, length, fun_id);
			if(!flag)
			{
				Response_upframe(buff, length, fun_id);
			}
			else
			{
				sw_log("bluetooth_task", SW_LOG_LEVEL_ERROR, "bluetooth execute command failed\r\n");
			}
		}

		OSTimeDlyHMSM(0, 0, 0, 500);
	}
}

/*进入待机模式与唤醒任务*/
void stopmode_task(void *pdata)
{
	unsigned char *message;
	INT8U _err;

	while(1)
	{
		message = OSQPend(Message_Q, 0, &_err);

		switch (*message)
		{
		case ENTER_STOP_MODE/* constant-expression */:
			sw_log("stopmode_task", SW_LOG_LEVEL_QUIET, "waiting 20s ,then enter stop mode...\r\n");
			OSTimeDlyHMSM(0, 0, 20, 0); 
			OSTaskSuspend(5);
			OSTaskSuspend(6);
			OSTaskSuspend(BASIC_MESSAGE_TASK_PRIO);
			OSTaskSuspend(Receive_COMMAND_TASK_PRIO);
			OSTaskSuspend(BLUETOOTH_TASK_PRIO);
			OSTaskSuspend(FTP_TASK_PRIO);
			gprs_disconnect();
		#if 0
			myfree(SRAMIN, Data_Buffer);
			myfree(SRAMIN, Bulk_Data_Buff);
		#endif
			sw_log("stopmode_task", SW_LOG_LEVEL_QUIET, "enter stop mode\r\n");
			enter_stop_mode();
			break;
		case WAKE_UP_MODE:
			if(g_respose_flag == 1)
			{
				g_respose_flag = 0;	
			}
			else
			{	
			}
			PWCTRL_GPIO_Init(); 
			DM9000_Init();
			GPRS_4G_Init();
		#if  0
			USB_Port_Set(0);  //USB 断开
			delay_ms(700);
			USB_Port_Set(1);
			sw_log("stopmode_task", SW_LOG_LEVEL_QUIET, "USB connecting..");
			// USB 配置
			Data_Buffer = mymalloc(SRAMIN, BULK_MAX_PACKET_SIZE*2*4);
			Bulk_Data_Buff = mymalloc(SRAMIN, BULK_MAX_PACKET_SIZE);
			USB_Interrupts_Config();    
			Set_USBClock();   
			USB_Init();	    
			delay_ms(1800);
#endif			
			OSTaskResume(5);
			OSTaskResume(6);
			OSTaskResume(BASIC_MESSAGE_TASK_PRIO);
			OSTaskResume(Receive_COMMAND_TASK_PRIO);
			OSTaskResume(BLUETOOTH_TASK_PRIO);	
			break;
		
		default:
			break;
		}
		OSTimeDlyHMSM(0, 0, 0, 500);
	}
}


void system_data_broadcast(void *pdata)
{
	OS_STK_DATA os_stk_info;
	float used_rate;
	
	while(1)
	{
		printf("enter system_data_broadcast ****17\r\n");
		OSTaskStkChk(BASIC_MESSAGE_TASK_PRIO, &os_stk_info); 
		used_rate = 100 * os_stk_info.OSUsed / (float)( os_stk_info.OSUsed + os_stk_info.OSFree);
		printf("BASIC_MESSAGE_TASK:free is %d, used is %d, used rate is %f%%\r\n", os_stk_info.OSFree, os_stk_info.OSUsed, used_rate); 
		OSTaskStkChk(Receive_COMMAND_TASK_PRIO, &os_stk_info);
		used_rate = 100 * os_stk_info.OSUsed / (float)( os_stk_info.OSUsed + os_stk_info.OSFree);
		printf("RECEIVE_COMMAND_TASK:free is %d, used is %d, used rate is %f%%\r\n", os_stk_info.OSFree, os_stk_info.OSUsed, used_rate);
		OSTaskStkChk(BLUETOOTH_TASK_PRIO, &os_stk_info);
		used_rate = 100 * os_stk_info.OSUsed / (float)( os_stk_info.OSUsed + os_stk_info.OSFree);
		printf("BLUETOOTH_TASK:free is %d, used is %d, used rate is %f%%\r\n", os_stk_info.OSFree, os_stk_info.OSUsed, used_rate); 
		OSTaskStkChk(GPRS_TASK_PRIO, &os_stk_info);
		used_rate = 100 * os_stk_info.OSUsed / (float)( os_stk_info.OSUsed + os_stk_info.OSFree);
		printf("GPRS_TASK:free is %d, used is %d, used rate is %f%%\r\n", os_stk_info.OSFree, os_stk_info.OSUsed, used_rate);
		OSTaskStkChk(FTP_TASK_PRIO, &os_stk_info);
		used_rate = 100 * os_stk_info.OSUsed / (float)( os_stk_info.OSUsed + os_stk_info.OSFree);
		printf("FTP_TASK:free is %d, used is %d, used rate is %f%%\r\n", os_stk_info.OSFree, os_stk_info.OSUsed, used_rate);
		OSTimeDlyHMSM(0,0,0,500);
	}

}


/*目前三种唤醒 1.通过串口1接收唤醒， 2.通过SD30078定时闹钟唤醒， 3.通过雨量脉冲采集唤醒(电源没有重新开，任务只有BASIC_MESSAGE执行)*/
/**/
void EXTI0_IRQHandler(void)   //sd3078 time IT
{

	unsigned char ctr2 = 0;
	static unsigned char message = WAKE_UP_MODE;

	OSIntEnter();
	SystemInit();
	if(g_stop_mode_flag  == 1)
	{
		g_stop_mode_flag = 0;
		sw_log("EXTI0_IRQHandler", SW_LOG_LEVEL_QUIET, "System ReInit after waking up\r\n");
		OSQPost(Message_Q, &message);
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
	I2CReadSerial(RTC_Address, CTR2, 1, &ctr2);
	switch (ctr2 & 0x30)
	{
	case ELECTRIC_INT_FLAG:
		printf("enter electric int flag\r\n");
		break;
	case ALARM_INT_FLAG:
		sw_log("EXTI0_IRQHandler", SW_LOG_LEVEL_QUIET,"enter alarm int flag,wating for sending message task..\r\n");
		break;
	case FREQUENCY_INT_FLAG:
		printf("enter frequency int flag\r\n");
		break;
	case COUNTDOWN_INT_FLAG:
		printf("enter countdown int flag\r\n");
		break;
	default:
		break;
	}
	OSIntExit();
}

void EXTI1_IRQHandler(void)   // 雨量采集脉冲中断
{
	OSIntEnter();
#if 1
	SystemInit();
	RS232_1_Init(115200); //电源开启
#endif
	g_rainfall_pluse_count++;
	sw_log("EXTI1_IRQHandler", SW_LOG_LEVEL_DEBUG, "total rainfall pluse is %d\r\n", g_rainfall_pluse_count);
	EXTI_ClearITPendingBit(EXTI_Line1);
	OSIntExit();
}

//DM9000, ST16C554 INT
void EXTI9_5_IRQHandler(void)
{
	u8 dat = 0;

	if(EXTI_GetITStatus(EXTI_Line6) != RESET)   //DM9000网络中断  
	{
		/* code */
		//printf("enter dm9000_int \r\n");
		OSIntEnter(); 
		EXTI->PR = 1 << 6;  			//清除LINE6上的中断标志位  
		while(DM9000_INT==0)
		{
			DMA9000_ISRHandler();
		}
		OSIntExit();  
	
	}
	else if(EXTI_GetITStatus(EXTI_Line7) != RESET)  //ST16C554D中断
	{
		OSIntEnter();
		EXTI_ClearITPendingBit(EXTI_Line7);
		if(ST16C554D_ReadReg(0, ST16C554D_LSR) & 0X01)         //RS485_1
		{		
			RS485_1_Address = 0;
			do
			{
				dat = ST16C554D_ReadReg(0, ST16C554D_RHR);
				delay_us(100);
				RS485_1_Receive[RS485_1_Address++] = dat;
			} while (ST16C554D_ReadReg(0, ST16C554D_LSR) & 0X01);
		}

		if(ST16C554D_ReadReg(1, ST16C554D_LSR) & 0x01)    	//RS232_2
		{
			/*ohter code*/
			RS232_2_Recv_Address = 0;
			do
			{
				dat = ST16C554D_ReadReg(1, ST16C554D_RHR);
				delay_us(100);
				RS232_2_Recv[RS232_2_Recv_Address++] = dat;
			} while (ST16C554D_ReadReg(1, ST16C554D_LSR) & 0x01);   
		}
		if(ST16C554D_ReadReg(2, ST16C554D_LSR) & 0X01)    //RS232_3
		{
			RS232_3_Recv_Address = 0;
			do
			{
				dat = ST16C554D_ReadReg(2,ST16C554D_RHR);
				delay_us(100);
				RS232_3_Recv[RS232_3_Recv_Address++] = dat;
			} while (ST16C554D_ReadReg(2, ST16C554D_ISR) & 0X04);		
		}

		if(ST16C554D_ReadReg(3, ST16C554D_LSR) & 0x01)		//RS232_4
		{
			/*ohter code*/
			RS232_4_Recv_Address = 0;
			do
			{
				dat = ST16C554D_ReadReg(3, ST16C554D_RHR);
				delay_us(100);
				RS232_4_Recv[RS232_4_Recv_Address++] = dat;
			} while (ST16C554D_ReadReg(3, ST16C554D_LSR) & 0x01);

		}
		OSIntExit();
	
	}	

} 

