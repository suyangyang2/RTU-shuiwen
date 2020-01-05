#ifndef _ADC_H
#define _ADC_H

#include "sys.h"


/*

	AD8 - PC0 ; AD7 - PC3 ; AD6 - PC1 ; AD5 - PC2
	AD4 - PF10; AD3 - PF9 ; AD2 - PF8 ; AD1 - PF7
	AD0 - PF6

*/



#define HC4051_EN_GPIO  GPIOF
#define HC4051_PIN  	GPIO_Pin_10
#define HC4051_EN   	PFout(10)
#define A0_PORT   		GPIOF
#define AO_PIN			GPIO_Pin_7
#define A1_PORT			GPIOF
#define A1_PIN			GPIO_Pin_8
#define	A2_PORT			GPIOF
#define A2_PIN			GPIO_Pin_9



#define ADC8_GPIO_PORT GPIOC
#define ADC8_PIN       GPIO_Pin_0
#define ADC7_GPIO_PORT GPIOC
#define ADC7_PIN       GPIO_Pin_3
#define ADC6_GPIO_PORT GPIOC
#define ADC6_PIN       GPIO_Pin_1
#define ADC5_GPIO_PORT GPIOC
#define ADC5_PIN       GPIO_Pin_2
#define ADC4_GPIO_PORT GPIOF
#define ADC4_PIN       GPIO_Pin_10
#define ADC3_GPIO_PORT GPIOF
#define ADC3_PIN       GPIO_Pin_9
#define ADC2_GPIO_PORT GPIOF
#define ADC2_PIN 			 GPIO_Pin_8
#define ADC1_GPIO_PORT GPIOF
#define ADC1_PIN       GPIO_Pin_7
#define ADC0_GPIO_PORT GPIOF
#define ADC0_PIN       GPIO_Pin_6

#define ADC7_CHANNEL ADC_Channel_13




void Adc_total_Init(void);
static void ADC_74HC_Init(void);

/*
********************************************************************************************
* 函数名：Get_RealADC
* 返回值：adc值
* 参  数：channel：8路通道的某一路； times:求adc平均值的累积次数
* 描  述：得到8路ADC的某一路的电压大小
*
*********************************************************************************************
*/
float Get_RealADC(u8 channel, const u8 times);

static void adc_Init(void);
static u16 Get_Adc(u8 ch);
static u16 Get_Adc_Average(u8 ch,u8 times);





















#endif
