#ifndef _4G_H
#define _4G_H
#include "sys.h"





#define GPRS_RST  PBout(8) 
#define GPRS_RI    PFin(11)
#define GPRS_DTR    PGout(14)
#define GPRS_RECEIVE_LEN		200
//#define MAX_ERRO_TIME     100
#define MAX_CONTENT_LEN 	1000  //1600
#define TRANS_CONTENT_LEN  900  //1200 
#define NETERRO_TIME   10 //4           //δ�յ��±��ĵ����̴���   
#define DOWN_FRAME_LEN_THRESHOLD    50   //�жϽ��ܵ��±����ݵ���ֵ
#define WAIT_TIME_LONG   1  //100			//����������������ݴ���֮��ļ��
#define TOTAL_RETRANSMIT_NUM  20
#define SINGLE_RETRANSMIT_NUM  3

#define NO_DOWN_FRAME_ERRO     1
#define DOWN_FRAME_NO7E_ERRO   2
#define DOWN_FRAME_CRC_ERRO    3
#define DOWN_FRAME_NAK_ERRO    4


#define NETWORK_S0H 			0x7E7E
#define NETWORK_STX 			0x02 //����������ʼ
#define NETWORK_SYN 			0x16 //�������������ʼ
#define NETWORK_ETX				0x03 //���Ľ����������ޱ���
#define NETWORK_ETB				0x17 //���Ľ����������б���
#define NETWORK_ENQ				0x05 //ѯ��
#define NETWORK_EOT				0x04 //����������˳�
#define NETWORK_ACK 			0x06 //�϶�ȷ�ϣ���������
#define NETWORK_NAK				0x15 //��Ӧ�𣬷����ط�
#define NETWORK_ESC 			0x1B //��������������ն�����

#define CENTER_ADDR				0X01
#define PASSWORD				0X0000
#define REMOTE_ADDR_ID          0XF1F1
#define OBSERVE_TIME_ID         0XF0F0


#define REMOTE_ADDR_CLASS_ID_JIANGSHUI	0x50
#define REMOTE_ADDR_CLASS_ID_HEDAO		0x48
#define REMOTE_ADDR_CLASS_ID_SHUIKU		0x4B
#define REMOTE_ADDR_CLASS_ID_ZHABA		0x5A

#define REMOTE_ADDR_LEN  0x5
#define TOTAL_PKTS_AND_SN_LEN 0x3
#define GPRS_SEND_FIFO   250
#define  TCPSENDATE_ERRO   14



#define  GPRS_OK           0  
#define  AT_ERRO           1  
#define  ATE0_ERRO         2    //���Դ���   
#define  CPIN_ERRO         3	//��������
#define  CNMP_ERRO         4	//ѡ��4G�������
#define  CPSI_ERRO         5   //��ѯ4G����ʧ��
#define  CGATT_ERRO        6	//ע��4G����ʧ��
#define  NETCLOSE_ERRO     7
#define  CIPMODE_ERRO      8   //���ݴ���ʧ��
#define  CIPHEAD_ERRO      9	//����IPͷ�������˴���յ����ݻ���ʾ��������IP��ַ
#define  CSOCKSETPN_ERRO   10   //����PDP������
#define  NETOPEN_ERRO      11	
#define  IPADDR_ERRO       12    //IPADDR����
#define  TCPCONNE_ERRO     13
#define  TCPSENDATE_ERRO   14
#define  ATCIPSEND_ERRO    15     //TCP���÷����ֽ�������
#define  TCP_DISCONNECT_ERRO 16

//extern uint8_t s_gprs_init_flag;      // gprs��ʼ����ɱ�־������������ɱ�־�����ӳɹ���־
//extern uint8_t	s_gprs_net_flag;
//extern uint8_t s_gprs_connect_flag;




#pragma pack(1)

typedef struct pkt_time_s{
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
        
}pkt_time_t;

typedef struct pic_time_s{
    unsigned short id;
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
}pic_time_t;

////ͼƬ�ϱ����Ľṹ
typedef struct pic_info_s {
    unsigned short sn;
    pkt_time_t pkt_time;
    unsigned short remote_addr_id;
    unsigned char remote_addr[REMOTE_ADDR_LEN];
    unsigned char class_id;
   // unsigned short pic_time_id;
    pic_time_t pic_time;
    unsigned short pic_id;
    char data[TRANS_CONTENT_LEN];    
}pic_info_t;


typedef struct up_frame_s{
	unsigned short start_id;
	unsigned char center_addr;
	unsigned char remote_addr[REMOTE_ADDR_LEN];
	unsigned short password;
	unsigned char fun_id;
	unsigned short frame_id_len;          //���û��
	unsigned char content_start_id;
  unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
	union {
            pic_info_t pic_info;
            char data[TRANS_CONTENT_LEN];
	    }picture;
	unsigned char content_end_id;
	unsigned short crc;
}up_frame_t;

typedef struct pic_down_info_s{
    unsigned short sn;
    pkt_time_t pkt_time;
}pic_down_info_t;

typedef struct down_frame_s{
    unsigned short start_id;
    unsigned char remote_addr[REMOTE_ADDR_LEN];
    unsigned char center_addr;
    unsigned short password;
    unsigned char fun_id;
    unsigned short frame_id_len;
    unsigned char content_sitart_id;
    unsigned char total_pkts_and_cur_sn[TOTAL_PKTS_AND_SN_LEN];
    /*content*/
    pic_down_info_t pic_down_info;
    
    unsigned char content_end_id;
    unsigned short crc;
}down_frame_t;


#pragma pack()


extern const unsigned char remote_address[5];      //ң��վ��ַ
extern u8 g_gprs_receive_data_buff[GPRS_RECEIVE_LEN];
extern u8 g_gprs_receive_data_length;

u8 gprs_init(void);
u8 gprs_net_config(void);
u8 gprs_connect(void);
u8 gprs_disconnect(void);
void err_process(uint8_t err);

void GPRS_GPIO_Init(void);      //��λ���ų�ʼ��
void GPRS_Reset(void);        ////GPRS��λ
u8 gprs_enter_sleep_mode(void);
void gprs_exit_sleep_mode(void);
//char *str_delim(u8 num, char *temp, char *delim); //�ָ���ַ���  
 


void gprs_send_string(char* send_data);    ////4G�����ַ�������
int gprs_send_string_len(unsigned char* send_data,unsigned int len);  //4G����ָ�������ַ�������
u8 gprs_send_cmd(char *cmd,const char *ack,u32 timeout,u16 retime);
//u8 find_string(char* p);

int SendPictureToServer(void);
int GPRS_4G_Init(void);
int wp_send_data(up_frame_t *frame, int len);
void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf);
void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf);
unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen);
unsigned int GetIntData_FromString(unsigned char *buff);
unsigned int Analysis_Data_FromGPRS(unsigned char *src_buff, unsigned char *ip_information, unsigned char *port,unsigned char *recv_data);
 
#endif

