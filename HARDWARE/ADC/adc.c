#include "adc.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"




/**
		AD0-PF6		AD1-PF7		AD2-PF8 	AD3-PF9		AD4-PF10		ADC3
		AD5-PC2 	AD6-PC1		AD7-PC3 	AD8-PC0 	 						ADC123

**/

const float CONVERSION_RATE = 0.199;

void Adc_total_Init(void)
{
	ADC_74HC_Init();
	adc_Init();
}


static void ADC_74HC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);

	GPIO_InitStructure.GPIO_Pin = HC4051_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HC4051_EN_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = AO_PIN | A1_PIN | A2_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(A0_PORT, &GPIO_InitStructure);
	
}


float Get_RealADC(u8 channel,const u8 times)
{
	u8 bit[3] = {0};
	int i = 0;
	u32 temp = 0;
	float adc_value = 0;
	GPIO_TypeDef *gpio_port[] = {A0_PORT, A1_PORT, A2_PORT};
	uint16_t pin[] = {AO_PIN, A1_PIN, A2_PIN};

	while(channel > 0)
	{
		bit[i] = channel % 2;
		//printf("bit[%d] = %d\r\n", i ,bit[i]);
		channel /= 2;
		i++;
	}
	HC4051_EN = 0;
	for(i = 0; i < 3; i++)
	{
		if(bit[i] == 0)
		{
			GPIO_ResetBits(gpio_port[i], pin[i]);
		}
		else
		{
			GPIO_SetBits(gpio_port[i], pin[i]);
		}
		
	}
	delay_ms(10);
	temp = Get_Adc_Average(ADC_Channel_4, times);
	adc_value = (((float)temp * 3.3)/4096) / CONVERSION_RATE;
	return adc_value;
}

/*
********************************************************************************************
* ��������adc3_Init
* ����ֵ����	
* ��  ������
* ��  ����һ·ADC���Զ˿ڳ�ʼ��
*
*********************************************************************************************
*/
static void adc_Init(void)
{
	
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF|RCC_APB2Periph_ADC3  ,ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	ADC_DeInit(ADC3);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC3, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ��� 
	
	ADC_Cmd(ADC3,ENABLE);
	ADC_ResetCalibration(ADC3);
	while(ADC_GetResetCalibrationStatus(ADC3));
	ADC_StartCalibration(ADC3);
	while(ADC_GetCalibrationStatus(ADC3));
}


/*
********************************************************************************************
* ��������Get_Adc3
* ����ֵ����õ�ADCֵ
* ��  ����ch��ͨ��ֵ0-3
* ��  ����һ·ADC���Զ˿ڳ�ʼ��
*
*********************************************************************************************
*/
static u16 Get_Adc(u8 ch)
{
	ADC_RegularChannelConfig(ADC3,ch,1,ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC3,ENABLE);
	while(!ADC_GetFlagStatus(ADC3,ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC3);
}

/*
********************************************************************************************
* ��������Get_Adc3_Average
* ����ֵ����õ�ADCһ��������ƽ��ֵ
* ��  ����ch��ͨ��ֵ0-3
*					times���ɼ����ݵĴ���
* ��  �����õ�һ�����������������ݵľ�ֵ
*
*********************************************************************************************
*/
static u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
}















#if 0

/*
********************************************************************************************
* ��������adc_Init
* ����ֵ����	
* ��  ������
* ��  ����һ·ADC���Զ˿ڳ�ʼ��
*
*********************************************************************************************
*/
void adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC| RCC_APB2Periph_ADC1 ,ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);   //����ܳ���14M
	
	
//	//PF6 AD8
//	GPIO_InitStructure.GPIO_Pin = ADC8_PIN;
//	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_AIN;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(ADC0_GPIO_PORT,&GPIO_InitStructure);
	
//AD7
	GPIO_InitStructure.GPIO_Pin = ADC7_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC7_GPIO_PORT,&GPIO_InitStructure);
	
	ADC_DeInit(ADC1);
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ��� 
	
	ADC_Cmd(ADC1,ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

/*
********************************************************************************************
* ��������Get_Adc
* ����ֵ����õ�ADCֵ
* ��  ����ch��ͨ��ֵ0-3
* ��  ����һ·ADC���Զ˿ڳ�ʼ��
*
*********************************************************************************************
*/
u16 Get_Adc(u8 ch)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
}

/*
********************************************************************************************
* ��������Get_Adc_Average
* ����ֵ����õ�ADCһ��������ƽ��ֵ
* ��  ����ch��ͨ��ֵ0-3
*					times���ɼ����ݵĴ���
* ��  �����õ�һ�����������������ݵľ�ֵ
*
*********************************************************************************************
*/
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
}

#endif


