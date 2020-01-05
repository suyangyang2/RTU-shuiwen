#ifndef FTPD_H
#define FTPD_H

#include "sys.h"
#include <string.h>
#include <stdlib.h>

#include <lwip/sockets.h>
#include <time.h>
#include "malloc.h"
#include "exfuns.h"  
#include "picture.h"
#include "rtc.h"
#include "sd3078rtc.h"


#define FTP_PORT			21
#define FTP_SRV_ROOT		"/"
#define FTP_MAX_CONNECTION	2
#define FTP_USER			"rtt"
#define FTP_PASSWORD		"demo"
#define FTP_WELCOME_MSG		"220-= welcome on RT-Thread FTP server =-\r\n220 \r\n"
#define FTP_BUFFER_SIZE		1024

#define RT_TRUE                         1   /**< boolean true  */
#define RT_FALSE                        0   /**< boolean fails */
#define RT_NULL                         ((void *)0)





struct ftp_session
{
	char is_anonymous;

	int sockfd;
	struct sockaddr_in remote;

	/* pasv data */
	char pasv_active;
	int  pasv_sockfd;

	unsigned short pasv_port;
	size_t offset;

	/* current directory */
	char currentdir[256];

	struct ftp_session* next;
};


extern char filename[256];
extern Time_Def sysTime;
extern OS_EVENT *gprs_task_en;
extern volatile char g_save_picture_flag;
extern unsigned char g_picture_count;

struct ftp_session* ftp_new_session(void);
void ftp_close_session(struct ftp_session* session);
int ftp_get_filesize(char * filename);
int is_absolute_path(char* path);
int build_full_path(struct ftp_session* session, char* path, char* new_path, size_t size);
int do_list(char* directory, int sockfd);
int do_simple_list(char* directory, int sockfd);
int str_begin_with(char* src, char* match);

void ftpd_thread_entry(void* parameter);
int ftp_process_request(struct ftp_session* session, char * buf);
void ftpd_start(void);

















#endif // !FTPD_H
