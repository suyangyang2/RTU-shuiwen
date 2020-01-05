#include "common.h"


void USART_DISABLE()
{

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	

//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
 
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

	USART_Cmd(USART1,DISABLE);
	USART_Cmd(USART2,DISABLE);
	USART_Cmd(USART3,DISABLE);
	USART_Cmd(UART4,DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB1Periph_USART3|RCC_APB1Periph_UART4|
	RCC_APB1Periph_UART5,DISABLE);

}
void SPI1_DISABLE()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_Cmd(SPI1,DISABLE);
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTA.4时钟使能 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,DISABLE);
}

void SPI2_DISABLE()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_Cmd(SPI2,DISABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD, ENABLE);	 //使能PA,PD端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	//PB.12 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.12
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);						 //PB.12  输出高	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,DISABLE);
}
void ADC_DISABLE()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3,ENABLE);
	ADC_DeInit(ADC3);
 	ADC_Cmd(ADC3, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_ADC3,DISABLE);

}

void Reduce_Power(void)
{
	USART_DISABLE();
	SPI1_DISABLE();
	SPI2_DISABLE();
	ADC_DISABLE();	
}



void System_ReInit(void)
{
	
	#if 0
	u32 sd_total, sd_free;
	FRESULT res;
	INT8U task_res;
	int i = 0;
	#endif
	
	PWR_ON();
	FSMC_SRAM_Init();
	my_mem_init(SRAMIN);
	my_mem_init(SRAMEX);
#if 0
	delay_init();	   
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	PWCTRL_GPIO_Init();
	RS232_1_Init(115200);     //printf serial
	WKUP_Init();  //RTC SD3078中断IO初始化
	gprs_usart_init(115200);  	
	WK2124_Init(); // ATK-hc05, ATK-1218, RS485_2
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
	//bsp_timer3_cap_init(0xffff, 72-1);     //值不确定,以1HZ频率计数
	RainFall_Init();
	//RS485_1--ST16C554_A,  RS485_2--WK2124_3
	#if 0
	exfuns_init();							//为fatfs相关变量申请内存
	while (SD_Init())
	{
		printf("TF test failed!\r\n");
	}
	res = f_mount(fs[0], "0:", 1); 					//挂载SD卡
	if (res != FR_OK)
	{
		printf("SD mount failed\r\n");
	}
	while (exf_getfree((u8 *)"0", &sd_total, &sd_free))	//得到SD卡的总容量和剩余容量
	{
		printf("SD Fatfs Error!\r\n");
		delay_ms(500);
	}
	printf("sd free contain %d\r\n", sd_free >> 10);
	#endif
	printf("start os task lwip\r\n");
#if 0
	while (lwip_comm_init()) //lwip初始化
	{
		printf("Lwip Init failed!\r\n");
		delay_ms(500);
	}
	ST16C554D_Init(); 		//RS485_1, RS232_2, RS232_3, RS232_4
#endif
	
#endif
	DM9000_Init();
	printf("DM9000 Init success!\r\n");
	

	

}


