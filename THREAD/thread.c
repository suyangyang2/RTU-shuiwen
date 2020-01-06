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
static OS_STK FTP_TASK_STK[FTP_STK_SIZE] = {0};                                 //�ļ�������
static OS_STK GPRS_TASK_STK[GPRS_STK_SIZE] = {0};                               //4G����GPRS
static OS_STK BASIC_MESSAGE_TASK_STK[BASIC_MESSAGE_TASK_STK_SIZE] = {0};        //������Ϣ����
static OS_STK Receive_COMMAND_TASK_STK[Receive_COMMAND_TASK_STK_SIZE] = {0};		//����ָ��
static OS_STK BLUETOOTH_TASK_STK[BLUETOOTH_TASK_STK_SIZE] = {0};								//����
static OS_STK STOP_MODE_TASK_STK[STOP_MODE_TASK_SIZE] = {0};										//����	
static OS_STK  SYSTEM_DATA_BROADCAST_STK[SYSTEM_DATA_BROADCAST_STK_SIZE] = {0};		//ϵͳ����

OS_STK_DATA StackBytes;
__align(8) static OS_STK AppTaskXXXStk[200];

void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;

	OSStatInit();  //����ͳ������
	OS_ENTER_CRITICAL();  
	gprs_task_en = OSSemCreate(0);
	Message_Q = OSQCreate(&Message_Group[0], N_MESSAGE);
	//IWDG_Init(4, 1250); //���ʱ��2��

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
	unsigned char minutes = 0;                           //����
	unsigned char hours = 0;										         //Сʱ
	unsigned char last_minutes = 0xff;					         //��һ����
	unsigned char last_hours = 0xff;						         //����Сʱ	
	unsigned char rainfall_data[3] = {0};				         //��������
	unsigned char waterlevel_data[4] = {0};				       //ˮλ����
	float five_minute_rainfall = 0;								       //���������
	float rainfall_threshold = 0;										     //������ֵ
	RTU_operation_parameters_t rtu_paramters = {0};
	volatile int wait_time = 0;
	unsigned char message = 0;
	unsigned char ftp_time_interval = FTP_TIME_INTERVAL_MINUTES;   //FTP����ʱ����

	while(1)
	{
		RTC_ReadDate(&real_time);                            //��RTCʵʱ���ݼĴ��� ���� TRUE/FALSE
		minutes = BCDTimeTodata(real_time.minute);           //����
		hours = BCDTimeTodata(real_time.hour);               //Сʱ
		if ((minutes % 5 == 0) & (minutes != last_minutes))  //5 min �ӱ�  ���ݲɼ�,���ݴ洢
		{
			
			RTU_parameters_Readdata(&rtu_paramters);
			SaveAndGetData_FiveMinute(rainfall_data, waterlevel_data);  //�洢�ɼ����ݵ�FLASH������������ȡ����
			last_minutes = minutes;														          //��¼�ϴ�ʱ��		
			plus_message_init();															          //���ĳ�ʼ��
			if((minutes == 0) & (hours != last_hours))				          //Сʱ��
			{
				last_hours = hours;
				hour_message_init();                                                               //ʹ�÷���ʱ����۲�ʱ��
				RS232_3_Send_buf((unsigned char *)&g_hour_message, sizeof(g_hour_message));        //RS232����ʱ����Ϣ
				//gprs_send_string_len((unsigned char *)&g_hour_message, sizeof(g_hour_message));  //GPRS����ʱ������
				sw_log("basic_message_task", SW_LOG_LEVEL_QUIET, "send hour message\r\n");
			}
			five_minute_rainfall =  BCDSendataToFloat(FIVE_MIN_PRECIPIT_ID, g_plus_message.plus_message_content.five_min_precipit_data);
			rainfall_threshold = BCDSendataToFloat(0x0008, (unsigned char *)&rtu_paramters.threshold_rainfall_value_of_plusemessage);
			if(five_minute_rainfall >= rainfall_threshold)                      //����ӵ���������������ֵ
			{
				sw_log("basic_message_task", SW_LOG_LEVEL_QUIET, "send plus message\r\n");
				RS232_3_Send_buf((unsigned char *)&g_plus_message, sizeof(g_plus_message));
				//gprs_send_string_len((unsigned char *)&g_plus_message, sizeof(g_plus_message));	
			}	
			ftp_time_interval -= 5;     //FTPʱ������ÿ��5���ӣ�
			if(!ftp_time_interval)      //����Ȼ��ֱ�ӽ������ģʽ������Ӧ�ü�����Ӧ�±���ʱ�䣻
			{
				POE_ON();     //POE��PD3��
				ftp_time_interval = FTP_TIME_INTERVAL_MINUTES; //���¸�λ��ʼֵ��		
				OSTaskResume(FTP_TASK_PRIO);             //����FTP����
				sw_log("basic_message_task", SW_LOG_LEVEL_QUIET	, "wating to save picture,max time is 120s\r\n");
				wait_time = FTP_SAVE_MAX_WAIT_TIME;
				while(wait_time)                  //�ж��Ƿ�FTP�洢ͼƬ�ɹ���
				{
					if(g_save_picture_flag == 1)
					{
						g_save_picture_flag = 0;     //�����־λ
						break;
					}
					else
					{
						OSTimeDlyHMSM(0,0,1,0);
					}
					wait_time--;
				}
				if(wait_time == 0)    //�洢ʧ��ֱ�ӽ������ģʽ���ɹ���gprs_�����з�����Ϣ���У��������ģʽ
				{
					sw_log("basic_message_task", SW_LOG_LEVEL_QUIET, "ftp save picture failed\r\n");
					message = ENTER_STOP_MODE;
					OSQPost(Message_Q, &message);  //�����������ģʽ����Ϣ
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
				OSQPost(Message_Q, &message);  //�����������ģʽ����Ϣ
			}	
		}
		OSTimeDlyHMSM(0, 0 , 0, 800);
	}
}

//ͼƬ�洢����
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
		OSTaskSuspend(Receive_COMMAND_TASK_PRIO);   //��ֹ�·����ݣ�����Ϊ��ָ��
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


/*RS232����ָ���Ȼ��ִ������*/
void receive_command(void *pdata)
{
	unsigned char fun_id  = 0;

	while(1)
	{	
		if(RS232_3_Recv_Address != 0) //�ж�RS232_3������
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
		else if(g_usart2_receive_stat == 1)  //ͬʱ������Խ���4Gģ���Ӧ���ڽ��յ�������
		{
			
			if(strstr((char *)DMA_usart2_Buf, "\x7E\x7E") != NULL)  //ɸѡ������ָ������
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

/*�������ģʽ�뻽������*/
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
			USB_Port_Set(0);  //USB �Ͽ�
			delay_ms(700);
			USB_Port_Set(1);
			sw_log("stopmode_task", SW_LOG_LEVEL_QUIET, "USB connecting..");
			// USB ����
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


/*Ŀǰ���ֻ��� 1.ͨ������1���ջ��ѣ� 2.ͨ��SD30078��ʱ���ӻ��ѣ� 3.ͨ����������ɼ�����(��Դû�����¿�������ֻ��BASIC_MESSAGEִ��)*/
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

void EXTI1_IRQHandler(void)   // �����ɼ������ж�
{
	OSIntEnter();
#if 1
	SystemInit();
	RS232_1_Init(115200); //��Դ����
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

	if(EXTI_GetITStatus(EXTI_Line6) != RESET)   //DM9000�����ж�  
	{
		/* code */
		//printf("enter dm9000_int \r\n");
		OSIntEnter(); 
		EXTI->PR = 1 << 6;  			//���LINE6�ϵ��жϱ�־λ  
		while(DM9000_INT==0)
		{
			DMA9000_ISRHandler();
		}
		OSIntExit();  
	
	}
	else if(EXTI_GetITStatus(EXTI_Line7) != RESET)  //ST16C554D�ж�
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

