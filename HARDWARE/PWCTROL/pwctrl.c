#include "pwctrl.h"
#include "delay.h"
#include "stm32f10x_gpio.h"
/*
********************************************************************************************
* 函数名：PWCTRL_GPIO_Init
* 返回值：无
* 参  数：无
* 描  述：电源控制GPIO初始化
*
*********************************************************************************************
*/
void PWCTRL_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOG, ENABLE);

	//+12_1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = POE_PWRCTRL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(POE_PWRCTRL_GPIO_PORT, &GPIO_InitStructure);
	POE_OFF();
	
	//+12_2
	GPIO_InitStructure.GPIO_Pin = GPRS_PWRCTRL_PIN;
	GPIO_Init(GPRS_PWRCTRL_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(GPRS_PWRCTRL_GPIO_PORT, GPRS_PWRCTRL_PIN);

	//+12_3
	GPIO_InitStructure.GPIO_Pin =	RS232_PWR_PIN;
	GPIO_Init(RS232_PWR_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(RS232_PWR_GPIO_PORT, RS232_PWR_PIN);

	//+12_4
	GPIO_InitStructure.GPIO_Pin = RS485_PWR_PIN;
	GPIO_Init(RS485_PWR_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(RS485_PWR_GPIO_PORT, RS485_PWR_PIN);

	//+12_5
	GPIO_InitStructure.GPIO_Pin = RAINFALL_PWR_PIN;
	GPIO_Init(RAINFALL_PWR_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(RAINFALL_PWR_GPIO_PORT, RAINFALL_PWR_PIN);

	//6 ATK-HC05  +3.3_1
	GPIO_InitStructure.GPIO_Pin = ATK_HC05_PWRCTRL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ATK_HCO5_PWRCTRL_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ATK_HCO5_PWRCTRL_GPIO_PORT, ATK_HC05_PWRCTRL_PIN);


	//7 ATK-S1218 +3.3_2
	GPIO_InitStructure.GPIO_Pin = ATK_1218_PWRCTRL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ATK_1218_PWRCTRL_PIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ATK_1218_PWRCTRL_PIO_PORT, ATK_1218_PWRCTRL_PIN);

	//FLASH,SRAM,TF +3.3_3
	GPIO_InitStructure.GPIO_Pin = FLASH_SRAM_TF_PWRCTRL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(FLASH_SRAM_TF_PWRCTRL_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(FLASH_SRAM_TF_PWRCTRL_GPIO_PORT, FLASH_SRAM_TF_PWRCTRL_PIN);

	//DM9000,RJ45 +3.3_4
	GPIO_InitStructure.GPIO_Pin = DM9000_RJ45_PWRCTRL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DM9000_RJ45_PWRCTRL_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(DM9000_RJ45_PWRCTRL_GPIO_PORT, DM9000_RJ45_PWRCTRL_PIN);

	//ST16C554 ,WK2124  +3.3_5
	GPIO_InitStructure.GPIO_Pin = ST16C_WK2124_PWRCTRL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ST16C_WK2124_PWRCTRL_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ST16C_WK2124_PWRCTRL_GPIO_PORT, ST16C_WK2124_PWRCTRL_PIN);

}

void PWR_ON(void)
{
	GPIO_SetBits(POE_PWRCTRL_GPIO_PORT, POE_PWRCTRL_PIN);
	delay_ms(1);
	GPIO_SetBits(GPRS_PWRCTRL_GPIO_PORT, GPRS_PWRCTRL_PIN); //GPRS pwrctrl 2
	delay_ms(1);
	GPIO_SetBits(RS232_PWR_GPIO_PORT, RS232_PWR_PIN);
	delay_ms(1);
	GPIO_SetBits(RS485_PWR_GPIO_PORT, RS485_PWR_PIN);
	delay_ms(1);
	GPIO_SetBits(RAINFALL_PWR_GPIO_PORT, RAINFALL_PWR_PIN);
	delay_ms(1);
	GPIO_SetBits(ATK_HCO5_PWRCTRL_GPIO_PORT, ATK_HC05_PWRCTRL_PIN); //3.3_1
	delay_ms(1);
	GPIO_SetBits(ATK_1218_PWRCTRL_PIO_PORT, ATK_1218_PWRCTRL_PIN); //3.3_2
	delay_ms(1);
	GPIO_SetBits(FLASH_SRAM_TF_PWRCTRL_GPIO_PORT, FLASH_SRAM_TF_PWRCTRL_PIN);   //3.3_3
	delay_ms(1);
	GPIO_SetBits(DM9000_RJ45_PWRCTRL_GPIO_PORT, DM9000_RJ45_PWRCTRL_PIN); //3.3_4
	delay_ms(1);
	GPIO_SetBits(DI_PWRCTRL_GPIO_PORT, DI_PWRCTRL_PIN);
	delay_ms(1);
	GPIO_SetBits(ST16C_WK2124_PWRCTRL_GPIO_PORT, ST16C_WK2124_PWRCTRL_PIN); //3.3_6
	delay_ms(1);
}

void PWR_OFF(void)
{
	GPIO_ResetBits(POE_PWRCTRL_GPIO_PORT, POE_PWRCTRL_PIN); 	//POE
	delay_ms(1);
	GPIO_ResetBits(GPRS_PWRCTRL_GPIO_PORT, GPRS_PWRCTRL_PIN);   	//4G
	delay_ms(1);
	GPIO_ResetBits(RS232_PWR_GPIO_PORT, RS232_PWR_PIN);
	delay_ms(1);
	GPIO_ResetBits(RS485_PWR_GPIO_PORT, RS485_PWR_PIN);
	delay_ms(1);
	//GPIO_ResetBits(RAINFALL_PWR_GPIO_PORT, RAINFALL_PWR_PIN);      //rainfall
	delay_ms(1);
	GPIO_ResetBits(ATK_HCO5_PWRCTRL_GPIO_PORT, ATK_HC05_PWRCTRL_PIN);  // +3.3_1
	delay_ms(1);
	GPIO_ResetBits(ATK_1218_PWRCTRL_PIO_PORT, ATK_1218_PWRCTRL_PIN);   //+3.3_2
	delay_ms(1);
	GPIO_ResetBits(FLASH_SRAM_TF_PWRCTRL_GPIO_PORT, FLASH_SRAM_TF_PWRCTRL_PIN); //+3.3_3
	delay_ms(1);
	GPIO_ResetBits(DM9000_RJ45_PWRCTRL_GPIO_PORT, DM9000_RJ45_PWRCTRL_PIN);   //+3.3_4 电源设计DM9000中断上拉，目前关闭，会报overflow conter overflow错误
	delay_ms(1);
	GPIO_ResetBits(DI_PWRCTRL_GPIO_PORT, DI_PWRCTRL_PIN);			//+3.3_5
	delay_ms(1);
	GPIO_ResetBits(ST16C_WK2124_PWRCTRL_GPIO_PORT, ST16C_WK2124_PWRCTRL_PIN);  //+3.3_6
	delay_ms(1);	
}


void POE_ON(void)
{
	GPIO_SetBits(POE_PWRCTRL_GPIO_PORT, POE_PWRCTRL_PIN);
}

void POE_OFF(void)
{
	GPIO_ResetBits(POE_PWRCTRL_GPIO_PORT,POE_PWRCTRL_PIN);
}


