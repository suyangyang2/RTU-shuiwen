#include "thread.h"

int main()
{
	u32 sd_total, sd_free;
	FRESULT res;
	INT8U task_res;

	delay_init();	  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	PWCTRL_GPIO_Init();
	delay_ms(100);
	RS232_1_Init(115200);     //printf serial
	sw_log_set_level(SW_LOG_LEVEL_PANIC);
	gprs_usart_init(115200); 
	//usmart_dev.init(SystemCoreClock/1000000);	//初始化USMART	
	WK2124_Init(); // ATK-hc05, ATK-1218, RS485_2 RS485_1--ST16C554_A,  RS485_2--WK2124_3
	WKUP_Init();  //RTC SD3078中断IO初始化
	RS485_GPIO_Init();
	W25QXX_Init();
	FSMC_SRAM_Init();
	my_mem_init(SRAMIN);
	my_mem_init(SRAMEX);
	DI_GPIO_Init();
	Adc_total_Init();
	SD3078_Init();
	delay_ms(10);
	GPRS_GPIO_Init();
	RainFall_Init();
	//RTU_Operation_parameter_init();   //RTU配置参数初始化
	exfuns_init();							//为fatfs相关变量申请内存
	while (SD_Init())
	{
		sw_log("main", SW_LOG_LEVEL_PANIC,"TF test failed!\r\n");
	}
	res = f_mount(fs[0], "0:", 1); 					//挂载SD卡
	if (res != FR_OK)
	{
		sw_log("main", SW_LOG_LEVEL_PANIC,"SD mount failed\r\n");
	}
	while (exf_getfree((u8 *)"0", &sd_total, &sd_free))	//得到SD卡的总容量和剩余容量
	{
		sw_log("main", SW_LOG_LEVEL_PANIC, "SD Fatfs Error!\r\n");
		delay_ms(500);
	}
	sw_log("main", SW_LOG_LEVEL_DEBUG, "sd free contain %d\r\n", sd_free >> 10);
#if 0
	//USB
	Mass_Memory_Size[1] = SDCardInfo.CardCapacity;
	sw_log("main", SW_LOG_LEVEL_DEBUG, "SDCarInfo.CardCapacity is %lld\r\n",SDCardInfo.CardCapacity);
	Mass_Block_Size[1] = 512;
	Mass_Block_Count[1] = Mass_Memory_Size[1] / Mass_Block_Size[1];

	Mass_Memory_Size[0]=1024*1024*12;	//前12M字节
	Mass_Block_Size[0] =512;			//设置SPI FLASH的操作扇区大小为512
	Mass_Block_Count[0]=Mass_Memory_Size[0]/Mass_Block_Size[0]; 
	delay_ms(1800);
	USB_Port_Set(0);  //USB 断开
	delay_ms(700);
	USB_Port_Set(1);
	sw_log("main", SW_LOG_LEVEL_QUIET, "USB connecting..");
	Data_Buffer = mymalloc(SRAMIN, BULK_MAX_PACKET_SIZE*2*4);
	Bulk_Data_Buff = mymalloc(SRAMIN, BULK_MAX_PACKET_SIZE);
	
	USB_Interrupts_Config();   // USB 配置 
 	Set_USBClock();   
 	USB_Init();	    
	delay_ms(1800);	
#endif
	OSInit();
	while (lwip_comm_init()) //lwip初始化
	{
		printf("Lwip Init failed!\r\n");
		delay_ms(500);
	}
	ST16C554D_Init(); 		//RS485_1, RS232_2, RS232_3, RS232_4
	task_res = OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //创建开始任务
	if(task_res != OS_ERR_NONE)
	{
		printf("start task create failed!\r\n");
	}
	//other code./

	#if 1
	GPRS_4G_Init();
	#endif
	
	sw_log("main",SW_LOG_LEVEL_QUIET,"System Init over, wating ...\r\n\r\n");
	OSStart(); 				//开始任务
	
}





