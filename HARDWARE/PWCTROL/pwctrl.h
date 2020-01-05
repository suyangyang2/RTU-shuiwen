#ifndef _PWCTRL_H
#define _PWCTRL_H
#include "sys.h"

#define POE_PWRCTRL_GPIO_PORT GPIOD         //+12_1
#define POE_PWRCTRL_PIN       GPIO_Pin_3
//pwrctrl 2
#define GPRS_PWRCTRL_GPIO_PORT GPIOG        //+12_2
#define GPRS_PWRCTRL_PIN       GPIO_Pin_11
#define RS232_PWR_GPIO_PORT    GPIOG        //+12_3
#define RS232_PWR_PIN          GPIO_Pin_12
#define RS485_PWR_GPIO_PORT    GPIOG        //+12_4
#define RS485_PWR_PIN          GPIO_Pin_13  
#define RAINFALL_PWR_GPIO_PORT GPIOG         //+12_5                //PG15                   
#define RAINFALL_PWR_PIN       GPIO_Pin_15




#define ATK_HCO5_PWRCTRL_GPIO_PORT		    GPIOB       //+3.3_1
#define ATK_HC05_PWRCTRL_PIN				GPIO_Pin_3
#define ATK_1218_PWRCTRL_PIO_PORT			GPIOB        //+3.3_2
#define ATK_1218_PWRCTRL_PIN				GPIO_Pin_4
#define FLASH_SRAM_TF_PWRCTRL_GPIO_PORT		GPIOB        //+3.3_3
#define FLASH_SRAM_TF_PWRCTRL_PIN			GPIO_Pin_5
#define DM9000_RJ45_PWRCTRL_GPIO_PORT		GPIOC        //+3.3_4
#define DM9000_RJ45_PWRCTRL_PIN		        GPIO_Pin_1
#define DI_PWRCTRL_GPIO_PORT		        GPIOC        //+3.3_5
#define DI_PWRCTRL_PIN		                GPIO_Pin_2
#define ST16C_WK2124_PWRCTRL_GPIO_PORT	    GPIOC        //+3.3_6
#define ST16C_WK2124_PWRCTRL_PIN		    GPIO_Pin_3












void PWCTRL_GPIO_Init(void);
void PWR_ON(void);
void PWR_OFF(void);
void POE_ON(void);
void POE_OFF(void);

















#endif
