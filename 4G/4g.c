/*
* @file  4g.c
* @brief connect server,and send picture data;
				cannot send correctly picture data now;(no downframe data process )
				
* @version 1.1
* @author 
* @date 2019.8.23
*/

#include "dmausart.h"
#include "memory_my.h"
#include "4g.h"
#include "ff.h"
#include "delay.h"
#include "malloc.h"
#include "string.h"
#include "usart.h"
#include "sd3078rtc.h"
#include "ftpd.h"
#include "wk2124.h"
#include <stdlib.h>
#include <string.h>

#define		TRUE            1
#define		FALSE           0
#define SEND_PICTURE_DEBUG  0
#define TEST_SINGLE_PICTURE 1
#define MAX_AT_CIPSEND_ERRO_TIMES 5


const unsigned char *tcp_ip_shuiwen = (unsigned  char *)"AT+CIPOPEN=0,\"TCP\",\"218.94.97.35\",5009\r\n";
const unsigned char *tcp_ip_debug = (unsigned char *)"AT+CIPOPEN=1,\"TCP\",\"106.15.234.181\",5009\r\n";
const char *filename_test = "0:/recv_8.jpg";   //"0:/recv_8.jpg"  "0:/PICTURE/Mon/0/1.jpg"

static u8 s_gprs_init_flag = 0, s_gprs_net_flag = 0, s_gprs_connect_flag = 0;        // gprs初始化完成标志，网络配置完成标志，连接成功标志
static u8 s_error_cnt;
static u8 s_error_flag;
static  Time_Def s_sysTime = {0};

u8 g_gprs_receive_data_buff[GPRS_RECEIVE_LEN] = {0};  
u8 g_gprs_receive_data_length = 0;

//static unsigned int g_total_retransmit_count = 0;
static unsigned int g_single_retransmit_count = 0;  



/*
********************************************************************************************
* 函数名：GPRS_4G_Init
* 返回值：0：初始化成功   其它值：初始化失败
* 参  数：无
* 描  述：4G模块初始化配置，TCP连接服务器
*
*********************************************************************************************
*/
int GPRS_4G_Init(void)
{
	int time = 3;

	s_gprs_init_flag = 0; 
	s_gprs_net_flag = 0;
	s_gprs_connect_flag = 0;
	s_error_flag = 0;

	while(time--)
	{
		if(!s_gprs_init_flag && (!s_gprs_connect_flag)&& (!s_gprs_net_flag))              // 如果核心板未初始化
		{ 
			s_error_flag = gprs_init();            // 核心板初始化，并将初始化结果保存在 s_error_flag
		}
		if(!s_gprs_net_flag && (!s_gprs_connect_flag) && s_gprs_init_flag)
		{
			s_error_flag = gprs_net_config();      // 核心板网络配置 
		}	
		if(s_gprs_init_flag && s_gprs_net_flag && (!s_gprs_connect_flag)) // 如果核心板初始化和网络配置都完成
		{
			s_error_flag = gprs_connect();         // 开始 TCP 连接
			if(s_error_flag == GPRS_OK) return 0;    //退出4G模块初始化	
		} 
		if((!s_gprs_init_flag)|(!s_gprs_net_flag)|(!s_gprs_connect_flag) && s_error_flag)  // 配置未通过
		{
			err_process(s_error_flag);                // AT命令错误处理
		}
	}
	return -1;
}

/*
********************************************************************************************
* 函数名：
* 返回值：0：成功   其它值：失败
* 参  数：无
* 描  述：4G模块实现图片发送
*
*********************************************************************************************
*/

static up_frame_t frame = { 0 };
static int pic_size = 0;
static FIL fil;
static void FrameConstantInit(void);
static int OpenAndGetPictureSize(int *calculate_total_pkts);
static int UpdateFrameContentData(unsigned char *buf, unsigned int *len, unsigned char open_new_picture_flag);
static int CheckReceive(unsigned int cur_sn);


static void FrameConstantInit(void)
{
	RTU_operation_parameters_t rtu_parameters;
	
	RTU_parameters_Readdata(&rtu_parameters);
	frame.start_id = NETWORK_S0H;
	frame.center_addr = CENTER_ADDR;
	memcpy(frame.remote_addr, rtu_parameters.rtu_remote_addr,sizeof(rtu_parameters.rtu_remote_addr));
	frame.password = PASSWORD;
	frame.fun_id = 0x36;
	frame.content_start_id = NETWORK_SYN;
}

static int OpenAndGetPictureSize(int *calculate_total_pkts)
{
	int total_pkts = 0;
	
	if (f_open(&fil,(const TCHAR *)filename_test, FA_READ) != FR_OK)
	{
		printf("Open Picture Fail\r\n");
		return -1;
	}
	pic_size = f_size(&fil);
	sw_log("OpenAndGetPictureSize", SW_LOG_LEVEL_DEBUG, "send picture size is %d Byte\r\n", pic_size);
	total_pkts = (pic_size / TRANS_CONTENT_LEN) + 1;
	*calculate_total_pkts = total_pkts;
	frame.total_pkts_and_cur_sn[2] = (unsigned char)0x01;
	frame.total_pkts_and_cur_sn[1] = ((unsigned char)total_pkts & 0xf) << 4;
	frame.total_pkts_and_cur_sn[0] = (unsigned char)(total_pkts >> 4);
	return 0;

}

static int UpdateFrameContentData(unsigned char *buf, unsigned int *len, unsigned char open_new_picture_flag)
{
	static volatile unsigned short cur_sn = 1;        
	RTU_operation_parameters_t rtu_operation_parameter;

	RTU_parameters_Readdata(&rtu_operation_parameter);
	
	if(open_new_picture_flag == 1)    //new picture first sn
	{
		cur_sn = 1;
	}
	pic_size -= *len;
	if ((cur_sn&0xfff) == 0x1)
	{
		if (RTC_ReadDate(&s_sysTime) != TRUE)
		{
			printf("Read time failed\r\n");	
			
			//return -1;
		}
		frame.picture.pic_info.sn++;
		frame.picture.pic_info.sn = (frame.picture.pic_info.sn >> 8) | (frame.picture.pic_info.sn << 8);
		frame.picture.pic_info.pkt_time.year = 0x19;    
		frame.picture.pic_info.pkt_time.month = 0x07;
		frame.picture.pic_info.pkt_time.day = 0x02;
		frame.picture.pic_info.pkt_time.hour = 0x10;
		frame.picture.pic_info.pkt_time.minute = 0x18;
		frame.picture.pic_info.pkt_time.second = 0x38;
		frame.picture.pic_info.remote_addr_id = 0xf1f1;
		memcpy(frame.picture.pic_info.remote_addr, rtu_operation_parameter.rtu_remote_addr, sizeof(rtu_operation_parameter.rtu_remote_addr));
		frame.picture.pic_info.class_id = REMOTE_ADDR_CLASS_ID_HEDAO;
		frame.picture.pic_info.pic_time.id = 0xF0F0;
		frame.picture.pic_info.pic_time.year = s_sysTime.year;
		frame.picture.pic_info.pic_time.month = s_sysTime.month;
		frame.picture.pic_info.pic_time.day = s_sysTime.day;
		frame.picture.pic_info.pic_time.hour = s_sysTime.hour;
		frame.picture.pic_info.pic_time.minute = s_sysTime.minute;
		frame.picture.pic_info.pic_id = 0xF3F3;
		memcpy(frame.picture.pic_info.data, buf, *len);
	}
	else
	{
		memcpy(frame.picture.data, buf, *len);       ///
	}
	if (pic_size <= 0)
	{
		frame.content_end_id = NETWORK_ETX;
		
	}
	else
	{
		frame.content_end_id = NETWORK_ETB;
	}
	*len += 3; // add total packets and current serial number
	if ((cur_sn & 0xFFF) == 0x1) //indicat it's first packet
	{
		*len += sizeof(frame.picture) - sizeof(frame.picture.pic_info.data);
	}
	frame.frame_id_len = ((*len & 0xff) << 8) | ((*len >> 8) & 0xff);
	frame.total_pkts_and_cur_sn[2] = cur_sn & 0xFF;
	frame.total_pkts_and_cur_sn[1] &= ~0xF;
	frame.total_pkts_and_cur_sn[1] |= (cur_sn >> 8) & 0xF;
	printf("cur_sn is %d\r\n", cur_sn);
	cur_sn++;
	return (cur_sn - 1);
}


/*
	0-代表成功读到返回
	其它值-代表失败
*/
static int CheckReceive(unsigned int cur_sn)
{
	int i = 0;
	u8 *buff = NULL;
	down_frame_t *down_frame = NULL;
	u16 calculate_crc = 0;
	u16 receive_crc = 0;

	for (i = 0; i < 3; i++)
	{
		delay_ms(WAIT_TIME_LONG);
		if (g_usart2_receive_stat == 1)
		{
			g_usart2_receive_stat = 0;
			DMA_usart2_Buf[g_usart2_receive_cnt] = '\0';
			printf("%s\r\n", DMA_usart2_Buf);
			buff = (u8 *)strstr((const char *)DMA_usart2_Buf, "\x7E\x7E");
			if (buff == NULL)
			{
				printf("don't receive 7E7E\r\n");
				return -1;
			}
			else
			{
				down_frame = (down_frame_t *)(&DMA_usart2_Buf[buff - DMA_usart2_Buf]);
				#if SEND_PICTURE_DEBUG
				for (j = 0; j < sizeof(down_frame_t); j++)
				{
					printf("%02x\t", ((u8 *)down_frame)[m]);
				}
				printf("\r\ndown_frame ack is %02x\r\n", down_frame->content_end_id);
				#endif
				calculate_crc = CRC16_MODBUS((u8 *)down_frame, sizeof(down_frame_t) - 2);
				calculate_crc = ((calculate_crc&0x00ff) << 8)|(calculate_crc >> 8);
				receive_crc = down_frame->crc;
				if(receive_crc != calculate_crc)
				{
					printf("crc is not right\r\n");
					return -1;
				}
			}
			return 0;
			
		}
	}
	printf("don't receive buff\r\n");
	return -1;
}

int SendPictureToServer(void)
{

	FRESULT fil_res;
	UINT br;
	int check_receive_status = 0;
	int calculate_total_pkts = 0;
	unsigned char open_new_picture_flag = 0;
	int cur_sn = 0;
	unsigned char buf[MAX_CONTENT_LEN] = {0};
	FrameConstantInit();
	if (OpenAndGetPictureSize(&calculate_total_pkts) != 0)
	{
		return -1;
	}
	open_new_picture_flag = 1;
	sw_log("SendPictureToServer", SW_LOG_LEVEL_DEBUG, "Start to send picture\r\n");
	while (1)
	{
		memset(buf, 0, MAX_CONTENT_LEN);
		fil_res = f_read(&fil, buf, TRANS_CONTENT_LEN, &br);  ///br代表此次发送的数据包读取字节数大小,后面也代表的报文上下长度
		if (fil_res != FR_OK)
		{
			printf("file read failed\r\n");
			return -1;
		}
		else if (br == 0)
		{
			sw_log("SendPictureToServer", SW_LOG_LEVEL_DEBUG, "send one picture success\r\n");
			break;
		}
		cur_sn = UpdateFrameContentData(buf, &br, open_new_picture_flag);
		open_new_picture_flag = 0;
		if(wp_send_data(&frame, br) != 0)
		{
			return -1;
		}
		//delay_ms(WAIT_TIME_LONG);  //在CheckReceive中也同样有delay，
		if(1)   //if(cur_sn == 1 || cur_sn == calculate_total_pkts)
		{
			delay_ms(10);
			check_receive_status = CheckReceive(cur_sn);
			if (check_receive_status != 0)
			{
				g_single_retransmit_count++;
			}
		}
	
	}
	fil_res = f_close(&fil);
	sw_log("SendPictureToServer",SW_LOG_LEVEL_DEBUG,"g_single_retransmit is %d\r\n", g_single_retransmit_count);
	if (fil_res != FR_OK)
	{
		printf("close picture files failed\r\n");
	}
	return 0;
}

/*
********************************************************************************************
* 函数名：wp_send_data
* 返回值：0：成功   其它值：失败
* 参  数：*frame:上报数据帧 
*					len:数据长度
* 描  述：整合上CRC校验数据，然后进行发送数据包
*
*********************************************************************************************
*/
int wp_send_data(up_frame_t *frame, int len)
{
	unsigned char buf[MAX_CONTENT_LEN] = {0};
	unsigned char *ptr = buf;

	
	memcpy(ptr, frame, (sizeof(up_frame_t) - 3 - sizeof(pic_info_t) - TOTAL_PKTS_AND_SN_LEN));
	ptr += (sizeof(up_frame_t) - 3 - sizeof(pic_info_t) - TOTAL_PKTS_AND_SN_LEN);
	
	memcpy(ptr, frame->total_pkts_and_cur_sn, len);
	ptr += len;
	*ptr++ = frame->content_end_id;
	frame->crc=CRC16_MODBUS(buf,ptr-buf);     ////    将从开始起始帧到结束码之间
	*ptr++ = (frame->crc & 0xff00) >> 8;
	*ptr++ = frame->crc & 0xff;
	if(gprs_send_string_len(buf, ptr-buf) != 0)
	{
		return -1;
	}

	return 0;
}

/*
********************************************************************************************
* 函数名：GPRS_GPIO_Init
* 返回值：无
* 参  数：无
* 描  述：4G模块RST、RI、DTR、PEN初始化
*
*********************************************************************************************
*/
void GPRS_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOF|RCC_APB2Periph_GPIOG , ENABLE);	  // 使能GPIO

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	              // 4G_RST     PB8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	      // 设置引脚工作模式为通用推挽输出 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	      // 设置引脚输出最大速率为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPRS_RST = 0;  //RST高电平复位

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  //4G_RI
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;  //4G_DTR
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //4G_PEN
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);         //PEN低电平，不会关机
}

/*
********************************************************************************************
* 函数名：GPRS_Reset
* 返回值：无
* 参  数：无
* 描  述：4G模块复位
*
*********************************************************************************************
*/
void GPRS_Reset(void)
{
	GPRS_RST = 1;
	delay_ms(10);
	GPRS_RST = 0;
}

u8 gprs_enter_sleep_mode(void)
{
	if(gprs_send_cmd((char *)"AT+CSCLK=1\r\n",(const char *)"OK",200,2))
	{
		printf("AT+CSCLK failed\r\n");
		return 1;
	}
	GPRS_DTR = 1;
	return 0;
}

void gprs_exit_sleep_mode(void)
{
	GPRS_DTR = 0;
}

/*
********************************************************************************************
* 函数名：gprs_init
* 返回值：0:成功    其它值:失败
* 参  数：无
* 描  述：4G模块核心板初始化
*
*********************************************************************************************
*/
u8 gprs_init(void)
{
	sw_log("gprs_init", SW_LOG_LEVEL_QUIET, "Starting 4G Init\r\n");
	if(gprs_send_cmd("AT\r\n","OK",500, 5))
	{
		return AT_ERRO;	
	}
	if(gprs_send_cmd("AT+CPIN?\r\n","OK",200,2))
	{
		return CPIN_ERRO;
	}
	if(gprs_send_cmd("AT+CPSI?\r\n","OK",200,2))
	{
		return CPSI_ERRO;
	}
	if(gprs_send_cmd("AT+CGATT?\r\n","OK",200,2)) //是否依附着网络
	{
		return CGATT_ERRO;
	}
	if(gprs_send_cmd("ATE0\r\n","OK",200,5))
	{
		return ATE0_ERRO;	
	}
	
	sw_log("gprs_init", SW_LOG_LEVEL_QUIET, "SIM7600CE Inits Success\r\n");
  s_gprs_init_flag = 1;                                  // 初始化完成，标志位置1 
	

  return 0;
}

/*
********************************************************************************************
* 函数名：gprs_net_config
* 返回值：0:成功    其它值:失败
* 参  数：无
* 描  述：4G模块配置初始化
*
*********************************************************************************************
*/
u8 gprs_net_config(void)
{
	if (gprs_send_cmd("AT+NETCLOSE\r\n", "+NETCLOSE:", 500, 2))
	{
		return NETCLOSE_ERRO;
	}

	if(gprs_send_cmd("AT+CIPMODE=0\r\n", "OK", 200, 2))
	{
		return CIPMODE_ERRO;
	}
	if (gprs_send_cmd("AT+CIPHEAD=1\r\n", "OK", 200, 2))
	{
		return CIPHEAD_ERRO;
	}
	if (gprs_send_cmd("AT+CSOCKSETPN=1\r\n", "OK", 200, 2))
	{
		return CSOCKSETPN_ERRO;
	}
	if (gprs_send_cmd("AT+NETOPEN\r\n", "+NETOPEN: 0", 500, 1))
	{
		return NETOPEN_ERRO;
	}	
	sw_log("gprs_net_config", SW_LOG_LEVEL_QUIET, "gprs netconfig success\r\n");
	s_gprs_net_flag = 1;                                 // 配置完成，标志位置1
	return 0;
}

/*
********************************************************************************************
* 函数名：gprs_connect
* 返回值：0:成功    其它值:失败
* 参  数：无
* 描  述：4G模块网络TCP连接
*
*********************************************************************************************
*/
u8 gprs_connect(void)
{
	delay_ms(20);
	#if 1
	if (gprs_send_cmd((char*)tcp_ip_shuiwen, "+CIPOPEN:", 500, 2))       // 建立TCP连接 218.94.97.35
	{
		sw_log("gprs_connect", SW_LOG_LEVEL_ERROR, "*** TCP Connect Fail ***\r\n");
		return TCPCONNE_ERRO;
	}
	else
	{
		sw_log("gprs_connect", SW_LOG_LEVEL_QUIET, "TCP Connect Success,%s\r\n",tcp_ip_shuiwen);
	}
	#endif
	if (gprs_send_cmd((char*)tcp_ip_debug, "+CIPOPEN:", 500, 2))       // 建立TCP连接 106.15.234.181
	{
		sw_log("gprs_connect", SW_LOG_LEVEL_ERROR, "*** TCP Connect Fail ***\r\n");
		return TCPCONNE_ERRO;
	}
	else
	{
		sw_log("gprs_connect", SW_LOG_LEVEL_QUIET, "TCP Connect Success,%s\r\n",tcp_ip_debug);
	}

	delay_ms(500);
	s_gprs_connect_flag = 1;
	delay_ms(20);                                      // 清除缓存数据
	return 0;
}


u8 gprs_disconnect(void)
{
	if(gprs_send_cmd((char *)"AT+CIPCLOSE=0\r\n", (const char *)"OK",500, 2))
	{
		sw_log("gprs_disconnect", SW_LOG_LEVEL_ERROR,"***TCP disconnect 0 channel failed\r\n");
		return TCP_DISCONNECT_ERRO;
	}
	if(gprs_send_cmd((char *)"AT+CIPCLOSE=1\r\n", (const char *)"OK",500, 2))
	{
		sw_log("gprs_disconnect", SW_LOG_LEVEL_ERROR,"***TCP disconnect 1 channel failed\r\n");
		return TCP_DISCONNECT_ERRO;
	}
	sw_log("gprs_disconnect", SW_LOG_LEVEL_QUIET, "gprs disconnect success\r\n");
	return GPRS_OK;
}

/*
********************************************************************************************
* 函数名：err_process
* 返回值：无
* 参  数：err：错误处理值
* 描  述：4G模块根据错误值进行相应地打印、处理
*
*********************************************************************************************
*/
void err_process(uint8_t err)
{
	sw_log("err_process", SW_LOG_LEVEL_QUIET, "Starting Erro process\r\n");
	switch(err)
	{
		case AT_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR, "AT ERROR\r\n");
			s_error_cnt = 4;               // 如果发送AT命令，核心板没有反应，则直接重启。
			break;
		case ATE0_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR, "ATE0_ERRO\r\n");
			break;
		case CPIN_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"CPIN_ERRO\r\n");
			break;
		case CNMP_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"CNMP_ERRO\r\n");
			break;
		case CPSI_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"No query to 4G network\r\n");
			break;
		case CGATT_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"Network Registration Failure\r\n");
			break;
		case NETCLOSE_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"Failed to shut down the network\r\n");
			break;
		case CIPMODE_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"Failed to set to non-passband mode\r\n");	
			break;
		case IPADDR_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"Failed to open IP header\r\n");	
			break;
		case CSOCKSETPN_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"Failed to activate mobile scenario\r\n");
			break;
		case NETOPEN_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"Failed to open the network\r\n");
			break;
		case TCPCONNE_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"The server connection failed and will be reconnected\r\n");
			break;
		case TCPSENDATE_ERRO:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"Data transmission error, may disconnect from the server, will be reconnected\r\n");
			break;
		default:
			sw_log("err_process", SW_LOG_LEVEL_ERROR,"Init Success\r\n");
	}
	s_gprs_init_flag = 0;	               // 标志位置0
	s_gprs_net_flag = 0;
	s_gprs_connect_flag = 0;
	s_error_cnt++;
	if(s_error_cnt >= 3)                   // 出错大于等于三次
	{
		sw_log("err_process", SW_LOG_LEVEL_QUIET,"s_error_cnt beyond 3,gprs reset\r\n");
		s_error_cnt = 0;
		GPRS_Reset();            // 重启核心板
	}	
}

/*
********************************************************************************************
* 函数名：gprs_send_string
* 返回值：无
* 参  数：*send_data:发送的字符串
* 描  述：4G模块发送字符串
*
*********************************************************************************************
*/
void gprs_send_string(char* send_data)
{
	Usart2_Send((u8 *)send_data, strlen(send_data));
}

/*
********************************************************************************************
* 函数名：gprs_send_string_len
* 返回值：0：成功     其它值：失败
* 参  数：*send_data:发送的数据   
*					len：数据长度
* 描  述：4G模块发送一定长度的数据
*
*********************************************************************************************
*/
int gprs_send_string_len(unsigned char* send_data,unsigned int len)  
{
	char cmd[30] = {0};
	int offset = 0;
	static int send_atcipsend_erro_count = 0;
	

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "AT+CIPSEND=0,%d\r\n",len);  //0 水文所IP通道
	if(gprs_send_cmd((char *)cmd, (const char *)">",400, 1))
	{
		// DMA_usart2_Buf[g_usart2_receive_cnt] = '\0';
		// printf("%s\r\n", DMA_usart2_Buf);
		send_atcipsend_erro_count++;
		printf("Send AT+CIPSEND failed\r\n");
	}
	else
	{
		send_atcipsend_erro_count = 0;
	}
	if(send_atcipsend_erro_count == MAX_AT_CIPSEND_ERRO_TIMES)
	{
		return -1;
	}
	delay_ms(10);
	Usart2_Send(send_data, len);
	delay_ms(10);  //10ms
	offset = (unsigned char *)&frame.total_pkts_and_cur_sn - (unsigned char *)&frame + 2;
	if((strstr((const char *)DMA_usart2_Buf, (const char *)"OK") == NULL) && (send_data[offset] == 0x02)) //表明第二包没有发送成功，如果对其他包也判断的话，容易出问题
	{
		if(gprs_send_cmd((char *)cmd, (const char *)"+CIPSEND:",400, 1))
		{
			printf("reSend AT+CIPSEND failed\r\n");
		}
		printf("resend AT+CIPSEND and data\r\n");
		Usart2_Send(send_data, len);
	}
	return 0;
}



/*
********************************************************************************************
* 函数名：gprs_send_cmd
* 返回值：0：得到回复成功     其它值：没有收到回复
* 参  数：*cmd:命令
*					*ack:需要回复的数据
					timeout:超时次数
					retime:重发次数
*					
*					
* 描  述：4G模块发送指令后，在5ms检查回复是否正确,
*
*********************************************************************************************
*/
u8 gprs_send_cmd(char *cmd,const char *ack,u32 timeout,u16 retime)
{
	u16 i = 0;
	u32 timeout_value = 0;
	

	for(i = 0; i < retime; i++)
	{
		gprs_send_string(cmd);
		timeout_value = timeout; // avoid infinite loop
		while(timeout_value--)
		{
			if(g_usart2_receive_stat == 1)
			{
				#if 0
				printf("receive ack:\r\n");
				for(i = 0; i < g_usart2_receive_cnt; i++)
				{
					printf("%c", DMA_usart2_Buf[i]);
				}
				printf("\r\n");
				#endif
				g_usart2_receive_stat = 0;
				if(strstr((const char*)DMA_usart2_Buf, ack) != NULL)
				{
					memset(DMA_usart2_Buf, 0, sizeof(DMA_usart2_Buf)); 
					return 0;
				}	
			}	
			delay_ms(5);
		}
	}
	return 1;	
}


unsigned int GetIntData_FromString(unsigned char *buff)
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned char data_buff[5] = { 0 };
	unsigned char *end_addr = NULL;

	while (1)
	{
		if (buff[i] != '\0')
		{
			if ((buff[i] >= 0x30) & (buff[i] <= 0x39))
			{
				data_buff[j++] = buff[i];
			}	
		}
		else
		{
			break;
		}
		i++;
	}
	return strtol((char *)data_buff, (char **)&end_addr, 10);
}

unsigned int Analysis_Data_FromGPRS(unsigned char *src_buff, unsigned char *ip_information, unsigned char *port,unsigned char *recv_data)
{
	char *result = NULL;
	unsigned char *src_buff_temp = src_buff;
	unsigned int data_length = 0;

	if (strstr((char *)src_buff_temp, "RECV FROM") == NULL)
	{
		sw_log("Analysis_Data_FromGPRS",SW_LOG_LEVEL_INFO, "No Recv From GPRS\r\n");
		return 0;
	}
	result = strtok((char *)src_buff_temp, ":");
	result = strtok(NULL, ":");    //IP
	strcpy((char *)ip_information, result);
	result = strtok(NULL, "\r\n");  //PORT
	strcpy((char *)port, result);
	result = strtok(NULL, "\r\n");  //IPD
	data_length = GetIntData_FromString((unsigned char *)result);
	result = strtok(NULL, "\r\n");
	memcpy((char *)recv_data, result, data_length);       //最好不能用strcpy,可能数据中有Ox00，导致赋值数据丢失
	return data_length;

}

void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf)
{
    int i;
    unsigned char tmp[4];
	
    tmp[0] = 0;
    for(i=0;i< 8;i++)
    {
      if(srcBuf[0]& (1 << i))
        tmp[0]|=1<<(7-i);
    }
    dBuf[0] = tmp[0];   
}

void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf)
{
    int i;
    unsigned short tmp[4];
	
    tmp[0] = 0;
    for(i=0;i< 16;i++)
    {
      if(srcBuf[0]& (1 << i))
        tmp[0]|=1<<(15 - i);
    }
    dBuf[0] = tmp[0];
}

 unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen)
{
  unsigned short wCRCin = 0xFFFF;
  unsigned short wCPoly = 0x8005;
  unsigned char wChar = 0;
  int i;
  
  while (usDataLen--) 	
  {
        wChar = *(puchMsg++);
        InvertUint8(&wChar,&wChar);
        wCRCin ^= (wChar << 8);
        for(i = 0;i < 8;i++)
        {
          if(wCRCin & 0x8000)
            wCRCin = (wCRCin << 1) ^ wCPoly;
          else
            wCRCin = wCRCin << 1;
        }
  }
  InvertUint16(&wCRCin,&wCRCin);
  return (wCRCin) ;
}








