#include "short_letter.h"
#include "dmausart.h"
#include "4g.h"
#include "delay.h"
#include "usart.h"
#include <string.h>



void GetTeleCmd(char *tele_number, char *cmd)
{
	char buff[30] = { 0 };
	int offset = 0;
	char front[] = "AT+CMGS=\"";
	char rear[] = "\"\r\n";

	strncpy(buff, front, strlen(front));
	offset = strlen(front);
	strcat(buff, tele_number);
	offset += strlen(tele_number);
	memcpy(buff + offset, rear, strlen(rear));
	offset += strlen(rear);
	memcpy(cmd, buff, offset);
}

int Send_short_letter(char *tele_number, char *buff, int length)
{
	char tele_cmd[30] = {0};


	GetTeleCmd(tele_number, tele_cmd);
	if(gprs_send_cmd("AT+CPMS?\r\n","OK",200,1))
	{
			printf("AT+CPMS? failed\r\n");
			return -1;
	}
	if(gprs_send_cmd("AT+CMGF=1\r\n","OK",200,1))
	{	  
			printf("AT+CMGF=1 Failed\r\n");
			return -1;
	}
	if(gprs_send_cmd("AT+CSMP?\r\n","+CSMP:17,11,0,0",200, 1))
	{
			if(gprs_send_cmd("AT+CSMP=17,11,0,0\r\n", "OK",200,1))
			{
					printf("AT+CSMP Failed\r\n");
					return -1;
			}
	}
	if(gprs_send_cmd("AT+CSCS?\r\n","+CSCS:\"IRA\"",200, 1))
	{
			if(gprs_send_cmd("AT+CSCS=\"IRA\"\r\n","OK", 200,1))
			{
					printf("AT+CSCS Failed\r\n");
					return -1;
			}
	}
	if(gprs_send_cmd(tele_cmd,">", 400, 1))
	{
			printf("AT+CMGS Failed\r\n");
			return -1;
	}
	printf("start send short letter\r\n");
	Usart2_Send((u8 *)buff, length);
	gprs_send_string("\x1A");
	return 0;
}

//GPS
int GetGPS_Inforation(unsigned char *buff, int *length)
{
	int time = 2;

	while(time--)
	{
		if(gprs_init() != 0)
		{
			printf("4G module Init Failed\r\n");
			GPRS_Reset();
		}
		else
		{
				break;
		}
		delay_ms(500);
	}
	if(time == -1)   //初始化
	{
		return -1;
	}
	if(gprs_send_cmd("AT+CGPS=1\r\n","OK",200,2) != 0)
	{
		printf("OPEN CGPS failed\r\n");
		return -1;
	}
	gprs_send_string("AT+CGPSINFO\r\n");
	time = 0;
	while(1)
	{
		if(g_usart2_receive_stat == 1)
		{
			g_usart2_receive_stat = 0;
			if(time == 0)
			{
				memcpy(buff, DMA_usart2_Buf,g_usart2_receive_cnt);
				*length = g_usart2_receive_cnt;
			}
			if(time++ == 1) break;
		}
		delay_ms(1);
	}
	if(gprs_send_cmd("AT+CGPS=0\r\n","OK",200,2))
	{
		printf("CLOSE GPS failed\r\n");
		return -1;
	}
	return 0;
}
