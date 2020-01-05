/*
    实现功能：
        1.设置时钟，
        2.查询时钟

    上报upframe
    执行

*/
#include "memory_my.h"
#include "bluetooth.h"
#include "hc05.h"
#include "sd3078rtc.h"
#include "usart.h"
#include <string.h>


#define BLUETOOTH_DEBUG 0

/*从报文中得到正文部分数据，并且返回正文数据长度*/
static unsigned int get_bluetooth_content_data(const unsigned char *src_buff, unsigned char *des_buff)
{
	unsigned int data_length = src_buff[7] >> 3;
	unsigned int i = 0;

	for ( i = 0; i < data_length; i++)
	{
		des_buff[i] = src_buff[8 + i];
	}
	return	data_length;
}


static int set_real_time(set_time_frame_t *frame)
{
    Time_Def real_time;

    real_time.year = frame->content.time.year;
    real_time.month = frame->content.time.month;
    real_time.day = frame->content.time.day;
    real_time.hour = frame->content.time.hour;
    real_time.minute = frame->content.time.minute;
    real_time.second = frame->content.time.second;

    printf(" set real time current time: %02x-%02x-%02x : %02x:%02x:%02x\r\n", real_time.year, real_time.month,\
			real_time.day, real_time.hour, real_time.minute, real_time.second);
    RTC_WriteDate(&real_time);
    return 0;

}

static void set_time_upframe(set_time_frame_t *frame)
{
    set_time_frame_t set_time_upframe_message;
    
    set_time_upframe_message.header.start_id = BT_START_ID;
    set_time_upframe_message.header.fun_id = BT_SETTIME_FUNID;
    set_time_upframe_message.header.updownid_contentlength = (BT_SETTIME_UPDOWN_UP_ID >> 8) | (BT_SETTIME_UPDOWN_UP_ID << 8);
    set_time_upframe_message.header.content_start_id = BT_CONTENT_START_ID;
    memcpy((u8 *)(&set_time_upframe_message.content.guide_id), (u8 *)(&frame->content.guide_id), sizeof(set_time_upframe_message.content) + 2);
    BlueTooth_Sendbuff((u8 *)&set_time_upframe_message, sizeof(set_time_upframe_message));
  
}

static void read_time_upframe(void)
{
    querry_time_upframe_t querry_time_upframe;
    Time_Def real_time;
    
    RTC_ReadDate(&real_time);

    querry_time_upframe.header.start_id = BT_START_ID;
    querry_time_upframe.header.fun_id  = BT_QUERY_TIME_FUNID;
    querry_time_upframe.header.updownid_contentlength = (0x0008 >> 8) | (0x0008 << 8);
    querry_time_upframe.header.content_start_id = BT_CONTENT_START_ID;
    querry_time_upframe.content.guide_id  = TIME_GUIDE_ID;
    querry_time_upframe.content.data_length = 0x30;
    querry_time_upframe.content.time.year = real_time.year;
    querry_time_upframe.content.time.month = real_time.month;
    querry_time_upframe.content.time.day = real_time.day;
    querry_time_upframe.content.time.hour = real_time.hour;
    querry_time_upframe.content.time.minute = real_time.minute;
    querry_time_upframe.content.time.second = real_time.second;
    querry_time_upframe.endid = (unsigned short)((0x0D0A >> 8) | (0X0D0A << 8));
    BlueTooth_Sendbuff((u8 *)&querry_time_upframe, sizeof(querry_time_upframe));

}
/* 此处待补充*/
#if 0

static void read_rtu_upframe(void)
{

}

#endif
static void modify_4g_parameters(const unsigned char *buff)
{
    #if 0
    unsigned char data[20] = {0};
    unsigned int  data_length = 0;
    bluetooth_frame_t *frame = (bluetooth_frame_t)*buff;

    data_length = (frame->data_length >> 3);
    memcpy(data, buff + sizeof(frame->header + 2), data_length);
    #endif

    unsigned char content_data[20];
    unsigned int content_data_length = 0;
    unsigned int i = 0;

    content_data_length =  get_bluetooth_content_data(buff, content_data);  //得到4g正文数据，接下来解析
    printf("modify 4g parameters :");
    for(i = 0; i < content_data_length; i++)
    {
        printf("%02x\t", content_data[i]);
    }
    printf("\r\n");
}

static void modify_rtu_parameters(const unsigned char *buff, const unsigned int length)
{
    unsigned char guide_id;
    bluetooth_frame_t *frame = (bluetooth_frame_t *)buff;
    unsigned char remote_addr[5] = {0};
		//RTU_operation_parameters_t rtu_parameters = {0};
		#if BLUETOOTH_DEBUG
			int i = 0;
		#endif

    guide_id = frame->guide_id;
    printf("guide_id is %02x\r\n", guide_id);
    switch (guide_id)
    {
    case REMOTE_ADDR_GUIDE_ID:                  //7E 7E 4A 80 07 02 F1 28 00 12 34 56 78 0D 0A
        memcpy(remote_addr, buff+8, BT_REMOTE_ADDR_LEN);
        memcpy(g_rtu_parameters.rtu_remote_addr,remote_addr, BT_REMOTE_ADDR_LEN);
        RTU_parameters_Writedata(&g_rtu_parameters);
        #if BLUETOOTH_DEBUG
        RTU_parameters_Readdata(&rtu_parameters);
				printf("read parameters is \r\n");
				for(i = 0; i < sizeof(rtu_parameters); i++)
				{
					printf("%02x\t", ((unsigned char *)&rtu_parameters)[i]);
				}
				printf("\r\n");
        #endif
        break;
    
    default:
        break;
    }

}

static void qerry_realdata_upframe(void)
{
    querry_realdata_upfram_t frame;

    frame.header.start_id = BT_START_ID;
    frame.header.fun_id = BT_QUERY_REALDATA_FUNID;
    frame.header.updownid_contentlength = (0x000D >> 8) | ( 0X000D << 8);
    frame.header.content_start_id = BT_CONTENT_START_ID;
    frame.content.waterlevel_guide_id = 0x0E;
    frame.content.waterlevel_data_length = 0x1A;
    //水位数据
    frame.content.waterlevel_data[0] = 0x00;
    frame.content.waterlevel_data[1] = 0x00;
    frame.content.waterlevel_data[2] = 0x12;

    //温度数据
    frame.content.temperature_guide_id = 0x03;
    frame.content.temperature_data_length = 0x11;
    frame.content.temperature_data[0] = 0x02;
    frame.content.temperature_data[1] = 0x24;

    //电压数据
    frame.content.voltage_guide_id = 0x38;
    frame.content.voltage_data_length = 0x12;
    frame.content.voltage_data[0] = 0x07;
    frame.content.voltage_data[1] = 0x33;

    frame.endid = (unsigned short)((0x0D0A >> 8) | (0X0D0A << 8));
    BlueTooth_Sendbuff((u8 *)&frame, sizeof(frame));

}



/*仅仅改变上下行标识，其它数据与下报格式相同*/
static void chang_upid_upframe(const unsigned char *buff, const u32 length)
{
    unsigned char send_buff[50] = {0};

    memcpy(send_buff, buff, length);
    send_buff[3] &= 0x7f;
    BlueTooth_Sendbuff(send_buff, length);
}


int Response_upframe(const u8 *buff,const u32 length,const u8 fun_id)
{
    switch (fun_id)
    {
	case BT_QUIT_FUNID:
        chang_upid_upframe(buff, length);     //1
        return 0;			
    case BT_SETTIME_FUNID:
        set_time_upframe((set_time_frame_t *)buff);  //2
        return  0;
    case BT_QUERY_TIME_FUNID:
        read_time_upframe();
        return 0;
    case BT_READ_RTU_FUNID:
        chang_upid_upframe(buff, length);
        return 0;
    case BT_MODIFY_RTU_PARAMETERS_FUNID:    //3
		chang_upid_upframe(buff, length);
        return 0;		
    case BT_MODIFY_4G_PARAMETERS_FUNID:    //4
         chang_upid_upframe(buff, length);
        return 0;		
    case BT_QUERY_REALDATA_FUNID:    //5
        qerry_realdata_upframe();
        return 0;		
    case BT_SENDATA_MANUALLY_FUNID:	//6
        chang_upid_upframe(buff, length);
        return 0;	
        
    default:
		return -1;
    }
}


int Execute_commanad(const u8 *buff ,const u32 length, const u8 fun_id)
{
    u8 flag = 0;

    switch (fun_id)
    {
    case BT_SETTIME_FUNID:
       flag =  set_real_time((set_time_frame_t *)buff);
       if(!flag)
       {
            return 0;
       }
       else
       {
           return -1;
       } 
    case BT_MODIFY_RTU_PARAMETERS_FUNID:
        modify_rtu_parameters(buff, length);
        return 0;
    case BT_MODIFY_4G_PARAMETERS_FUNID:
        modify_4g_parameters(buff);
        return 0;
    default:
        break;
    }
    return 0;  //调试
}

int Get_IP_Information(const unsigned char *src_buff, const unsigned int length, unsigned char *des_buff)
{
	unsigned int i = 0;
	unsigned char buff[20] = { 0 };
	unsigned char temp;
	unsigned char offset = 0;
	
	if (length != 6)
	{
		return -1;
	}
	for ( i = 0; i < length; i++)
	{
		temp = (src_buff[i] >> 4) + '0';
		buff[2 * i + 0] = temp;
		temp = (src_buff[i] & 0x0f) + '0';
		buff[2 * i + 1] = temp;
	}
	for (i = 0; i < 15; i++)
	{
		offset = (i + 1) / 4;

		if ((i + 1) % 4 == 0)
		{
			des_buff[i] = '.';
		}
		else
		{
			des_buff[i] = buff[i - offset];
		}
		
	}
    des_buff[15] = '\0';
	return 0;
}

