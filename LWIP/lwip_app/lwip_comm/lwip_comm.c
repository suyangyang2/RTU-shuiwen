#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "malloc.h"
#include "delay.h"
#include "usart.h"  
#include <stdio.h>
#include "includes.h"
  

//��OS�汾��lwipʵ��,�����Ҫ3������: lwip�ں�����(������),dhcp����(��ѡ),DM9000��������(������)
//�������������񶼴���.
//����,����Ҫ�õ�2���ź���,���ں���ķ���DM9000.

//lwip DHCP����
//�����������ȼ�
#define LWIP_DHCP_TASK_PRIO       		7 
//���������ջ��С
#define LWIP_DHCP_STK_SIZE  		    128
//�����ջ�������ڴ����ķ�ʽ��������	
OS_STK * LWIP_DHCP_TASK_STK;	
//������
void lwip_dhcp_task(void *pdata); 

//lwip DM9000���ݽ��մ�������
//�����������ȼ�
#define LWIP_DM9000_INPUT_TASK_PRIO		6 
//���������ջ��С
#define LWIP_DM9000_INPUT_TASK_SIZE	    512
//�����ջ�������ڴ����ķ�ʽ��������	
OS_STK * LWIP_DM9000_INPUT_TASK_STK;	
//������
void lwip_dm9000_input_task(void *pdata); 


OS_STK * TCPIP_THREAD_TASK_STK;			//lwip�ں������ջ  

OS_EVENT* dm9000input;					//DM9000���������ź���
OS_EVENT* dm9000lock;					//DM9000��д���������ź���

//////////////////////////////////////////////////////////////////////////////////////////
__lwip_dev lwipdev;						//lwip���ƽṹ�� 
struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�
extern u32 memp_get_memorysize(void);	//��memp.c���涨��
extern u8_t *memp_memory;				//��memp.c���涨��.
extern u8_t *ram_heap;					//��mem.c���涨��.




//DM9000���ݽ��մ�������
void lwip_dm9000_input_task(void *pdata)
{
	//�����绺�����ж�ȡ���յ������ݰ������䷢�͸�LWIP���� 
	ethernetif_input(&lwip_netif);
}

//lwip�ں˲���,�ڴ�����
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 lwip_comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	mempsize=memp_get_memorysize();			//�õ�memp_memory�����С
	memp_memory=mymalloc(SRAMIN,mempsize);	//Ϊmemp_memory�����ڴ�
	sw_log("lwip_comm_mem_malloc", SW_LOG_LEVEL_DEBUG,"memp_memory:%d\r\n",mempsize);
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//�õ�ram heap��С
	ram_heap=mymalloc(SRAMIN,ramheapsize);	//Ϊram_heap�����ڴ� 
	sw_log("lwip_comm_mem_malloc", SW_LOG_LEVEL_DEBUG, "ram_heap:%d\r\n",ramheapsize);
	TCPIP_THREAD_TASK_STK=mymalloc(SRAMIN,TCPIP_THREAD_STACKSIZE*4);			//���ں����������ջ 
	LWIP_DHCP_TASK_STK=mymalloc(SRAMIN,LWIP_DHCP_STK_SIZE*4);					//��dhcp���������ջ 
	LWIP_DM9000_INPUT_TASK_STK=mymalloc(SRAMIN,LWIP_DM9000_INPUT_TASK_SIZE*4);	//��dm9000�������������ջ 
	if(!memp_memory||!ram_heap||!TCPIP_THREAD_TASK_STK||!LWIP_DHCP_TASK_STK||!LWIP_DM9000_INPUT_TASK_STK)//������ʧ�ܵ�
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip�ں˲���,�ڴ��ͷ�
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
	myfree(SRAMIN,TCPIP_THREAD_TASK_STK);
	myfree(SRAMIN,LWIP_DHCP_TASK_STK);
	myfree(SRAMIN,LWIP_DM9000_INPUT_TASK_STK);
}
//lwip Ĭ��IP����
//lwipx:lwip���ƽṹ��ָ��
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//Ĭ��Զ��IPΪ:192.168.1.100
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=100;
	//MAC��ַ����(�����ֽڹ̶�Ϊ:2.0.0,�����ֽ���STM32ΨһID)
	lwipx->mac[0]=dm9000cfg.mac_addr[0];
	lwipx->mac[1]=dm9000cfg.mac_addr[1];
	lwipx->mac[2]=dm9000cfg.mac_addr[2];
	lwipx->mac[3]=dm9000cfg.mac_addr[3];
	lwipx->mac[4]=dm9000cfg.mac_addr[4];
	lwipx->mac[5]=dm9000cfg.mac_addr[5]; 
	//Ĭ�ϱ���IPΪ:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=1;
	lwipx->ip[3]=30;
	//Ĭ����������:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//Ĭ������:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//û��DHCP	
} 

//LWIP��ʼ��(LWIP������ʱ��ʹ��)
//����ֵ:0,�ɹ�
//      1,�ڴ����
//      2,DM9000��ʼ��ʧ��
//      3,�������ʧ��.
u8 lwip_comm_init(void)
{
	OS_CPU_SR cpu_sr;
	u8 err;
	struct netif *Netif_Init_Flag;		//����netif_add()����ʱ�ķ���ֵ,�����ж������ʼ���Ƿ�ɹ�
	struct ip_addr ipaddr;  			//ip��ַ
	struct ip_addr netmask; 			//��������
	struct ip_addr gw;      			//Ĭ������ 
	if(lwip_comm_mem_malloc())return 1;	//�ڴ�����ʧ��
	
 	dm9000input=OSSemCreate(0);			//�������ݽ����ź���,������DM9000��ʼ��֮ǰ����
	dm9000lock=OSMutexCreate(2,&err);	//���������ź���,��ߵ����ȼ�2	
	
	if(DM9000_Init())return 2;			//��ʼ��DM9000
	tcpip_init(NULL,NULL);				//��ʼ��tcp ip�ں�,�ú�������ᴴ��tcpip_thread�ں�����
	lwip_comm_default_ip_set(&lwipdev);	//����Ĭ��IP����Ϣ
#if LWIP_DHCP		//ʹ�ö�̬IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	sw_log("lwip_comm_init", SW_LOG_LEVEL_DEBUG,"MAC Address:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	sw_log("lwip_comm_init", SW_LOG_LEVEL_DEBUG, "Static Ip Address........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	sw_log("lwip_comm_init", SW_LOG_LEVEL_DEBUG,"Subnet Mask..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	sw_log("lwip_comm_init", SW_LOG_LEVEL_DEBUG, "Default Gateway..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//�������б������һ������
	if(Netif_Init_Flag != NULL) 	//������ӳɹ���,����netifΪĬ��ֵ,���Ҵ�netif����
	{
		netif_set_default(&lwip_netif); //����netifΪĬ������
		netif_set_up(&lwip_netif);		//��netif����
	}
	
	OS_ENTER_CRITICAL();  		//�����ٽ���
	OSTaskCreate(lwip_dm9000_input_task,(void*)0,(OS_STK*)&LWIP_DM9000_INPUT_TASK_STK[LWIP_DM9000_INPUT_TASK_SIZE-1],LWIP_DM9000_INPUT_TASK_PRIO); 		 //��̫�����ݽ�������
	OS_EXIT_CRITICAL();			//�˳��ٽ���
#if	LWIP_DHCP
	lwip_comm_dhcp_creat();		//����DHCP����
#endif		
	return 0;//����OK.
}   
//���ʹ����DHCP
#if LWIP_DHCP
//����DHCP����
void lwip_comm_dhcp_creat(void)
{
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();  //�����ٽ���
	OSTaskCreate(lwip_dhcp_task,(void*)0,(OS_STK*)&LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE-1],LWIP_DHCP_TASK_PRIO);//����DHCP���� 
	OS_EXIT_CRITICAL();  //�˳��ٽ���
}
//ɾ��DHCP����
void lwip_comm_dhcp_delete(void)
{
	dhcp_stop(&lwip_netif); 		//�ر�DHCP
	OSTaskDel(LWIP_DHCP_TASK_PRIO);	//ɾ��DHCP����
}
//DHCP��������
void lwip_dhcp_task(void *pdata)
{
	u32 ip=0,netmask=0,gw=0;
	dhcp_start(&lwip_netif);//����DHCP 
	lwipdev.dhcpstatus=0;	//����DHCP
	printf("���ڲ���DHCP������,���Ե�...........\r\n");   
	while(1)
	{ 
		printf("���ڻ�ȡ��ַ...\r\n");
		ip=lwip_netif.ip_addr.addr;		//��ȡ��IP��ַ
		netmask=lwip_netif.netmask.addr;//��ȡ��������
		gw=lwip_netif.gw.addr;			//��ȡĬ������ 
		if(ip!=0)   					//����ȷ��ȡ��IP��ַ��ʱ��
		{
			lwipdev.dhcpstatus=2;	//DHCP�ɹ�
 			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			//������ͨ��DHCP��ȡ����IP��ַ
			lwipdev.ip[3]=(uint8_t)(ip>>24); 
			lwipdev.ip[2]=(uint8_t)(ip>>16);
			lwipdev.ip[1]=(uint8_t)(ip>>8);
			lwipdev.ip[0]=(uint8_t)(ip);
			printf("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			//����ͨ��DHCP��ȡ�������������ַ
			lwipdev.netmask[3]=(uint8_t)(netmask>>24);
			lwipdev.netmask[2]=(uint8_t)(netmask>>16);
			lwipdev.netmask[1]=(uint8_t)(netmask>>8);
			lwipdev.netmask[0]=(uint8_t)(netmask);
			printf("ͨ��DHCP��ȡ����������............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			//������ͨ��DHCP��ȡ����Ĭ������
			lwipdev.gateway[3]=(uint8_t)(gw>>24);
			lwipdev.gateway[2]=(uint8_t)(gw>>16);
			lwipdev.gateway[1]=(uint8_t)(gw>>8);
			lwipdev.gateway[0]=(uint8_t)(gw);
			printf("ͨ��DHCP��ȡ����Ĭ������..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
		{  
			lwipdev.dhcpstatus=0XFF;//DHCPʧ��.
			//ʹ�þ�̬IP��ַ
			IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			printf("DHCP����ʱ,ʹ�þ�̬IP��ַ!\r\n");
			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}  
		delay_ms(250); //��ʱ250ms
	}
	lwip_comm_dhcp_delete();//ɾ��DHCP���� 
}
#endif 





















































