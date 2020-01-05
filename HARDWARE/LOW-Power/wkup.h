#ifndef _WKUP_H
#define _WKUP_H

#include "sys.h"





void Sys_Standby(void);
void Sys_Enter_Standby(void);
void WKUP_Init(void);
void enter_stop_mode(void);

void EXTI_Disable(void);

void Peripheral_Disable(void);
void Peripheral_Enable(void);



/**
void PWR_EnterSleepMode(void);
void PWR_EnterStopMode(void);
void PWR_EnterStandbyMode(void);
**/




extern unsigned char g_stop_mode_flag;
















#endif
