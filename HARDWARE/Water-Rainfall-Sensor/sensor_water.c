/*
* @file  sensor_water.c
* @brief
       通过定时器输入捕获的方式，来得到每一次电平变化中断信号，（应该ETR外部时钟输入模式2，来脉冲计数对整个系统性能更好）
* @author
* @date 2019.9.17
*/

#include "sensor_water.h"
#include "4g.h"
#include "delay.h"


TIM_ICInitTypeDef TIM3_ICInitStructure;
volatile unsigned int g_rainfall_pluse_count = 0;


/*
********************************************************************************************
* 函数名：GraytoDecimal
* 返回值：二进制格式数据
* 参  数：x格雷码二进制表示的数据
* 描  述：将水位采集到的格雷码二进制数据转化为真实地二进制数据
*
*********************************************************************************************
*/
static unsigned int GraytoDecimal(unsigned int x);

void RainFall_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_Init(&NVIC_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	

}



#if 0
void RainFall_Pluse_Init(void)
{
	bsp_timer3_cap_init(0xffff,72-1); //如果是PWM电平捕获，精度为1us

}



void bsp_timer3_cap_init(u16 arr, u16 psc)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	
	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM3_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM3_ICInitStructure.TIM_ICFilter = 0x00;
	TIM_ICInit(TIM3, &TIM3_ICInitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(TIM3, TIM_IT_Update|TIM_IT_CC4, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET)
	{
		g_rainfall_pluse_count++;
		if(g_rainfall_pluse_count == 0xffffffff)
		{
			g_rainfall_pluse_count = 0;
		}
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_CC4|TIM_IT_Update);

}

#endif














static unsigned int GraytoDecimal(unsigned int x)
{
	unsigned int y = x;
	while (x >>= 1)
	{
		y ^= x;
	}
	return y;
}


void DI_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE, ENABLE);

	//初始化S0-4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = S0_PIN | S1_PIN | S2_PIN | S3_PIN | S4_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(S0_GPIO_PORT, &GPIO_InitStructure);

	//初始化S5-7
	GPIO_InitStructure.GPIO_Pin = S5_PIN | S6_PIN | S7_PIN;
	GPIO_Init(S5_GPIO_PORT, &GPIO_InitStructure);

	//0E1 , 0E2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = OE1_PIN | OE2_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(OE1_GPIO_PORT, &GPIO_InitStructure);

}

unsigned int GetWaterLevelData(void)
{
	unsigned int waterlevel_graydata = 0;
	unsigned int waterlevel_data = 0;

	waterlevel_graydata = GetGrayWaterLevelData();
	waterlevel_data = GraytoDecimal(waterlevel_graydata);
	return waterlevel_data;
}


int GetGrayWaterLevelData(void)
{
	int waterlevel_data = 0;
	int i = 0;
	
	GPIO_TypeDef *gpio_port[13] = {S0_GPIO_PORT, S1_GPIO_PORT, S2_GPIO_PORT, S3_GPIO_PORT, S4_GPIO_PORT, \
								   S5_GPIO_PORT, S6_GPIO_PORT, S7_GPIO_PORT, S0_GPIO_PORT, S1_GPIO_PORT, \
								   S2_GPIO_PORT, S3_GPIO_PORT, S4_GPIO_PORT};
	uint16_t gpio_pin[13] = {S0_PIN, S1_PIN, S2_PIN, S3_PIN, S4_PIN, S5_PIN, S6_PIN, S7_PIN , \
							 S0_PIN, S1_PIN, S2_PIN, S3_PIN, S4_PIN};

	OE1 = 0;
	OE2 = 1;
	for(i = 0; i < 14; i++)
	{
		if (i == 8)
		{
			delay_ms(10);
			OE1 = 1;
			OE2 = 0;
		}
		if(GPIO_ReadInputDataBit(gpio_port[i], gpio_pin[i]) == SET)
		{
			waterlevel_data |= 1 << i;
		}
		else
		{
			waterlevel_data &= ~(1 << i);
		}
		delay_ms(10);
	}
	
	OE1 = 1;
	OE2 = 1;
	return waterlevel_data;
}






/*通过ETR定时器计数的方式来计算脉冲个数*/
#if 0

void bsp_timer3_init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM_ETRClockMode2Config(TIM3, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);
	TIM_SetCounter(TIM3, 0);
	TIM_Cmd(TIM3, ENABLE);
	
}
__inline u16 GetRainFallCount(void)
{
	return TIM3->CNT;

}


__inline void SetRainFallCount(u16 count)
{
	TIM3->CNT = count;
}

#endif



#if 0


/**  
	SO - PE2 ; S1 - PE3 ; S2 - PE4 ; S3 - PE5 ; S4 - PE6
  S5 - PC13; S6 - PC14; S7 - PC15; S8 - PA0 ; S9 - PA1  
	S10 - PC4; S11 - PC5; S12 - PB0; S13 - PB1; 
**/



/*
********************************************************************************************
* 函数名:Sensor_Water_GPIO_Init
* 返回值：无
* 参  数：无
* 描  述：用于DP25接口GPIO初始化
*
*********************************************************************************************
*/
void Sensor_Water_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE,ENABLE);
	
	//初始化S0-4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = S0_PIN|S1_PIN|S2_PIN|S3_PIN|S4_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(S0_GPIO_PORT,&GPIO_InitStructure);
	
	//初始化S5-7
	GPIO_InitStructure.GPIO_Pin = S5_PIN|S6_PIN|S7_PIN;
	GPIO_Init(S5_GPIO_PORT,&GPIO_InitStructure);
	
	//初始化S8-9
	GPIO_InitStructure.GPIO_Pin = S8_PIN|S9_PIN;
	GPIO_Init(S8_GPIO_PORT,&GPIO_InitStructure);
	
	//初始化S10-11
	GPIO_InitStructure.GPIO_Pin = S10_PIN|S11_PIN;
	GPIO_Init(S10_GPIO_PORT,&GPIO_InitStructure);
	
	//初始化S12-13
	GPIO_InitStructure.GPIO_Pin = S12_PIN|S13_PIN;
	GPIO_Init(S12_GPIO_PORT,&GPIO_InitStructure);
	
}

#endif // 0
