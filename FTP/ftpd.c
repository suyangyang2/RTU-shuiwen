#include <string.h>
#include <stdlib.h>

#include <lwip/sockets.h>
#include <time.h>
#include "malloc.h"
#include "exfuns.h"  
#include "4g.h"
#include "picture.h"
#include "dm9000.h"
#include "ftpd.h"

#define FTP_DEBUG 1
#define SELECT_PICTURE_COUNT 2

volatile char g_save_picture_flag = 0;               //GPRS 保存图片标志位
char filename[256];                                  //文件名字
int g_send_sockfd=0;
int m = 0;
static struct ftp_session* session_list = NULL;
unsigned char g_picture_count = 1;





struct ftp_session* ftp_new_session()
{
	struct ftp_session* session;

	session = (struct ftp_session*)mymalloc(SRAMIN, sizeof(struct ftp_session));
	if(session == NULL)
	{
		printf("ftp session malloc failed\r\n");
	}

	session->next = session_list;
	session_list = session;

	return session;
}


void ftp_close_session(struct ftp_session* session)
{
	struct ftp_session* list;

	if (session_list == session)
	{
		session_list = session_list->next;
		session->next = NULL;
	}
	else
	{
		list = session_list;
		while (list->next != session) list = list->next;

		list->next = session->next;
		session->next = NULL;
	}

	myfree(SRAMIN, session);
}

int ftp_get_filesize(char * filename)
{
	
	int end = 0;
#if 0
	fd = open(filename, O_RDONLY, 0);
	if (fd < 0) return -1;

	pos = lseek(fd, 0, SEEK_CUR);
	end = lseek(fd, 0, SEEK_END);
	lseek (fd, pos, SEEK_SET);
	close(fd);
#endif
	return end;
}

int is_absolute_path(char* path)
{
#ifdef _WIN32
	if (path[0] == '\\' ||
		(path[1] == ':' && path[2] == '\\'))
		return RT_TRUE;
#else
	if (path[0] == '/') return RT_TRUE;
#endif

	return RT_FALSE;
}

int build_full_path(struct ftp_session* session, char* path, char* new_path, size_t size)
{
	if (is_absolute_path(path) == RT_TRUE)
		strcpy(new_path, path);
	else
	{
		sprintf(new_path, "%s/%s", session->currentdir, path);
	}

	return 0;
}

void ftpd_thread_entry(void* parameter)
{
	int numbytes;
	int sockfd, maxfdp1;
	struct sockaddr_in local;
	fd_set readfds, tmpfds;
	struct ftp_session* session;
	unsigned int addr_len = sizeof(struct sockaddr);
	char * buffer = (char *) mymalloc(SRAMIN, FTP_BUFFER_SIZE);
	if(buffer == NULL)
	{
		printf("ftpd thread malloc failed\r\n");
	}

	
	local.sin_port=htons(FTP_PORT);
	local.sin_family=PF_INET;
	local.sin_addr.s_addr=INADDR_ANY;

	FD_ZERO(&readfds);
	FD_ZERO(&tmpfds);
	sockfd=socket(AF_INET, SOCK_STREAM, 0);    
	if(sockfd < 0)
	{
		printf("create socket failed\n");
		return ;
	}

	bind(sockfd, (struct sockaddr *)&local, addr_len);
	listen(sockfd, FTP_MAX_CONNECTION);

	FD_SET(sockfd, &readfds);
	for(;;)
	{
		
	    /* get maximum fd */
	    maxfdp1 = sockfd + 1;
        session = session_list;
	    while (session != RT_NULL)
	    {
	        if (maxfdp1 < session->sockfd + 1)
                maxfdp1 = session->sockfd + 1;

            FD_SET(session->sockfd, &readfds);
            session = session->next;
	    }

		tmpfds=readfds;
		//printf("tmpfds 0x%x\n", tmpfds.fd_bits[0]);
		if (select(maxfdp1, &tmpfds, 0, 0, 0) == 0) continue;
		//printf("%s:%d\n", __FUNCTION__, __LINE__);
		if(FD_ISSET(sockfd, &tmpfds))
		{
			int com_socket;
			struct sockaddr_in remote;

			com_socket = accept(sockfd, (struct sockaddr*)&remote, (socklen_t *)&addr_len);
			if(com_socket == -1)
			{
				printf("Error on accept()\nContinuing...\n");
				continue;
			}
			else
			{
				printf("Got connection from %s\n", inet_ntoa(remote.sin_addr));
				send(com_socket, FTP_WELCOME_MSG, strlen(FTP_WELCOME_MSG), 0);
				FD_SET(com_socket, &readfds);

				/* new session */
				session = ftp_new_session();
				if (session != NULL)
				{
					strcpy(session->currentdir, FTP_SRV_ROOT);
					session->sockfd = com_socket;
					session->remote = remote;
				}
			}
		}

		{
			struct ftp_session* next;

			session = session_list;
			while (session != NULL)
			{
				next = session->next;
				if (FD_ISSET(session->sockfd, &tmpfds))
				{
					numbytes=recv(session->sockfd, buffer, FTP_BUFFER_SIZE, 0);
					if(numbytes==0 || numbytes==-1)
					{
						printf("Client %s disconnected\n", inet_ntoa(session->remote.sin_addr));
						FD_CLR(session->sockfd, &readfds);
						printf("session socket id %d\n", session->sockfd);
						closesocket(session->sockfd);
						ftp_close_session(session);
						
					}
					else
					{
						buffer[numbytes]=0;
						m = ftp_process_request(session, buffer);
						if(m == -1)             ////接受数据处理部分
						{
							printf("Client %s disconnected\r\n", inet_ntoa(session->remote.sin_addr));
							closesocket(session->sockfd);
							FD_CLR(session->sockfd, &readfds);
							ftp_close_session(session);
						}
					
					}
				}
				session = next;
				//printf("session != NULL\r\n");
			}
		}
	}

	// myfree(SRAMIN, buffer);
}



int ftp_process_request(struct ftp_session* session, char *buf)
{
	int  fd;
	struct timeval tv;
	fd_set readfds;
	//int j = 0;
	int  numbytes;       
	char *sbuf;
	char *parameter_ptr, *ptr;
	unsigned int addr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in local, pasvremote;
	int picture_size = 0;
	int start_recv_one_pic=0;
	RESULT_MY res;
	
				
	sbuf =(char *)mymalloc(SRAMIN, FTP_BUFFER_SIZE);       //数据缓存
	if(sbuf == NULL)
	{
		printf("ftp_process_request malloc failed\r\n");
		return -1;
	}

	tv.tv_sec=3, tv.tv_usec=0;
	local.sin_family=PF_INET;
	local.sin_addr.s_addr=INADDR_ANY;

	/* remove \r\n */
	ptr = buf;
	while (*ptr)
	{
		if (*ptr == '\r' || *ptr == '\n') *ptr = 0;
		ptr ++;
	}

	/* get request parameter */
	parameter_ptr = strchr(buf, ' '); if (parameter_ptr != NULL) parameter_ptr ++;

	// debug:
	printf("%s requested: \"%s\"\n", inet_ntoa(session->remote.sin_addr), buf);

	//
	//-----------------------
	if(str_begin_with(buf, "USER")==0)		//向远程主机表明自己的身份，需要口令时，必须输入口令
	{
		printf("%s sent login \"%s\"\n", inet_ntoa(session->remote.sin_addr), parameter_ptr);
		// login correct
		if(strcmp(parameter_ptr, "anonymous") == 0)
		{
			session->is_anonymous = RT_TRUE;
			sprintf(sbuf, "331 Anonymous login OK send e-mail address for password.%s\r\n", parameter_ptr);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
		else if (strcmp(parameter_ptr, FTP_USER) == 0)
		{
			session->is_anonymous = RT_FALSE;		
			sprintf(sbuf, "331 Password required for %s\r\n", parameter_ptr);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
		else
		{
			// incorrect login
			sprintf(sbuf, "530 Login incorrect. Bye.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			myfree(SRAMIN, sbuf);
			return -1;
		}
		myfree(SRAMIN, sbuf);
		return 0;
	}
	else if(str_begin_with(buf, "PASS")==0)
	{
		printf("%s sent password \"%s\"\n", inet_ntoa(session->remote.sin_addr), parameter_ptr);
		if (strcmp(parameter_ptr, FTP_PASSWORD)==0 ||
			session->is_anonymous == RT_TRUE)
		{
			// password correct
			sprintf(sbuf, "230 User logged in\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			myfree(SRAMIN, sbuf);	
			return 0;
		}

		// incorrect password
		sprintf(sbuf, "530 Login or Password incorrect. Bye!\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		myfree(SRAMIN, sbuf);		
		return -1;
	}
	else if(str_begin_with(buf, "LIST")==0  )
	{
		memset(sbuf,0,FTP_BUFFER_SIZE);
		sprintf(sbuf, "150 Opening Binary mode connection for file list.\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		do_list(session->currentdir, session->pasv_sockfd);
		printf("list: socket id %d\n", session->pasv_sockfd);
		closesocket(session->pasv_sockfd);
		session->pasv_active = 0;
		sprintf(sbuf, "226 Transfert Complete.\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	else if(str_begin_with(buf, "NLST")==0 )
	{
		memset(sbuf, 0, FTP_BUFFER_SIZE);
		sprintf(sbuf, "150 Opening Binary mode connection for file list.\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		do_simple_list(session->currentdir, session->pasv_sockfd);
		closesocket(session->pasv_sockfd);
		session->pasv_active = 0;
		sprintf(sbuf, "226 Transfert Complete.\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	else if(str_begin_with(buf, "PWD")==0 || str_begin_with(buf, "XPWD")==0)
	{
		sprintf(sbuf, "257 \"%s\" is current directory.\r\n", session->currentdir);
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	else if(str_begin_with(buf, "TYPE")==0)
	{
		// Ignore it
		if(strcmp(parameter_ptr, "I")==0)
		{
			sprintf(sbuf, "200 Type set to binary.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
		else
		{
			sprintf(sbuf, "200 Type set to ascii.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
	}
	else if(str_begin_with(buf, "PASV")==0)
	{
		int dig1, dig2;
		int sockfd;
		char optval='1';
		
		session->pasv_port = 10000;
		session->pasv_active = 1;
		local.sin_port=htons(session->pasv_port);
		local.sin_addr.s_addr=INADDR_ANY;

		dig1 = (int)(session->pasv_port/256);
		dig2 = session->pasv_port % 256;

		FD_ZERO(&readfds);

		if((sockfd=socket(PF_INET, SOCK_STREAM, 0))==-1)
		{
			sprintf(sbuf, "425 Can't open data connection.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			goto err1;
		}

		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))==-1)
		{
			sprintf(sbuf, "425 Can't open data connection.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			goto err1;
		}
		printf("%s:%d\n", __FUNCTION__, __LINE__);	
		if(bind(sockfd, (struct sockaddr *)&local, addr_len)==-1)
		{
			sprintf(sbuf, "425 Can't open data connection.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			goto err1;
		}

		if(listen(sockfd, 1)==-1)
		{
			sprintf(sbuf, "425 Can't open data connection.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			goto err1;
		}
		printf("Listening %d seconds @ port %d\n", tv.tv_sec, session->pasv_port);
		sprintf(sbuf, "227 Entering passive mode (%d,%d,%d,%d,%d,%d)\r\n", 127, 0, 0, 1, dig1, dig2);
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		FD_SET(sockfd, &readfds);
		select(0, &readfds, 0, 0, &tv);
		if(FD_ISSET(sockfd, &readfds))
		{
			if((session->pasv_sockfd = accept(sockfd, (struct sockaddr*)&pasvremote, (socklen_t *)&addr_len))==-1)
			{
				printf("%s:%d sockfd %d\n", __FUNCTION__, __LINE__, session->pasv_sockfd);
				sprintf(sbuf, "425 Can't open data connection.\r\n");
				send(session->sockfd, sbuf, strlen(sbuf), 0);
				goto err1;
			}
			else
			{
				printf("Got Data(PASV) connection from %s(socket id %d)\n", inet_ntoa(pasvremote.sin_addr), session->pasv_sockfd);
				
				session->pasv_active = 1;
				closesocket(sockfd);
			}
		}
		else
		{
err1:
			if(FD_ISSET(sockfd, &readfds) && (session->pasv_sockfd == 0))
			{
				closesocket(session->pasv_sockfd);
			}
			closesocket(sockfd);
			FD_ZERO(&readfds);
			session->pasv_active = 0;
			myfree(SRAMIN, sbuf);
			return 0;
		}
	}
	else if (str_begin_with(buf, "RETR")==0)
	{
		int file_size;

		strcpy(filename, buf + 5);

		build_full_path(session, parameter_ptr, filename, 256);
		file_size = ftp_get_filesize(filename);
		if (file_size == -1)
		{
			sprintf(sbuf, "550 \"%s\" : not a regular file\r\n", filename);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			session->offset=0;
			myfree(SRAMIN, sbuf);			
			return 0;
		}
		
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		while((numbytes = read(fd, sbuf, FTP_BUFFER_SIZE))>0)
		{
			send(session->pasv_sockfd, sbuf, numbytes, 0);
		}
		sprintf(sbuf, "226 Finished.\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		//close(fd);
		closesocket(session->pasv_sockfd);
	}
	else if (str_begin_with(buf, "STOR")==0)
	{
		FIL fil;
		UINT bw;
		if(session->is_anonymous == RT_TRUE)
		{
			sprintf(sbuf, "550 Permission denied.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			myfree(SRAMIN, sbuf);
			return 0;
		}

		if(g_picture_count == 1)
		{
			printf("give up this ftp picture data\r\n");
		}
		if(g_picture_count == SELECT_PICTURE_COUNT)
		{
			res = create_directory((unsigned char *)filename);  //创建路径
			if(res != OK)
			{
				printf("mkdir failed\r\n");
			}
			if(f_open (&fil,(TCHAR *)filename,FA_CREATE_ALWAYS|FA_WRITE) != FR_OK)
			{
				printf("open picture failed\r\n");
			}
		}		
		sprintf(sbuf, "150 Opening binary mode data connection for \"%s\".\r\n", filename);
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		FD_ZERO(&readfds);
		FD_SET(session->pasv_sockfd, &readfds);
		printf("Waiting %d seconds for data...\n", tv.tv_sec);
		while(select(session->pasv_sockfd+1, &readfds, 0, 0, &tv)>0 )
		{
			if((numbytes=recv(session->pasv_sockfd, sbuf, FTP_BUFFER_SIZE, 0))>0)
			{			
				if ((unsigned char)sbuf[0] == 0xff && (unsigned char)sbuf[1] == 0xd8)
				{
					start_recv_one_pic = 1;
					if(g_picture_count == SELECT_PICTURE_COUNT)
					{
						if(f_write(&fil,sbuf,numbytes,&bw) != FR_OK)
						{
							printf("write picture data failed\r\n");
						}
					}
					
				}else if (start_recv_one_pic == 1)
				{
					if(g_picture_count == SELECT_PICTURE_COUNT)
					{
						if(f_write(&fil,sbuf,numbytes,&bw) != FR_OK)
						{
							printf("write picture data failed\r\n");
						}
					}			
					if( ((unsigned char)sbuf[numbytes - 2] == 0xff) && ((unsigned char)sbuf[numbytes-1] == 0xd9) )
					{
						break;
					}		
				}

			}
			else if(numbytes == 0)
			{
				sprintf(sbuf, "226 Finished.\r\n");
				send(session->sockfd, sbuf, strlen(sbuf), 0);
				break;
			}
			else if(numbytes == -1)
			{
				closesocket(session->pasv_sockfd);
				myfree(SRAMIN, sbuf);
				return -1;
			}
		}
		if(g_picture_count == SELECT_PICTURE_COUNT)
		{
			f_close(&fil);
			picture_size = f_size(&fil);
			printf("The %d picture is successfully stored ,picture size is %d KB\r\n", Figure_num++,(unsigned int)picture_size / 1024);
			g_picture_count = 1;
			g_save_picture_flag = 1;
			DM9000_Set_PhyPower(0); //关掉phy
			closesocket(session->pasv_sockfd);
			myfree(SRAMIN, sbuf);		
			//OSSemPost(gprs_task_en);		//释放信号量
			return -1; // 因为要关掉此次session，所以这里返回-1， 然后上级行数关掉session和socket	
		}
		else
		{
			g_picture_count++;
			closesocket(session->pasv_sockfd);
		}	
	}
	else if(str_begin_with(buf, "SIZE")==0)
	{
		int file_size;

		build_full_path(session, parameter_ptr, filename, 256);

		file_size = ftp_get_filesize(filename);
		if( file_size == -1)
		{
			sprintf(sbuf, "550 \"%s\" : not a regular file\r\n", filename);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
		else
		{
			sprintf(sbuf, "213 %d\r\n", file_size);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
	}
	else if(str_begin_with(buf, "MDTM")==0)
	{
		sprintf(sbuf, "550 \"/\" : not a regular file\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	else if(str_begin_with(buf, "SYST")==0)
	{
		sprintf(sbuf, "215 %s\r\n", "RT-Thread RTOS");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	else if(str_begin_with(buf, "CWD")==0)
	{
		build_full_path(session, parameter_ptr, filename, 256);

		sprintf(sbuf, "250 Changed to directory \"%s\"\r\n", filename);
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		strcpy(session->currentdir, filename);
		printf("Changed to directory %s", filename);
	}
	else if(str_begin_with(buf, "CDUP")==0)
	{
		sprintf(filename, "%s/%s", session->currentdir, "..");

		sprintf(sbuf, "250 Changed to directory \"%s\"\r\n", filename);
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		strcpy(session->currentdir, filename);
		printf("Changed to directory %s", filename);
	}
	else if(str_begin_with(buf, "PORT")==0)
	{
		int i;
		int portcom[6];
		char tmpip[100];

		i=0;
		portcom[i++]=atoi(strtok(parameter_ptr, ".,;()"));
		for(;i<6;i++)
			portcom[i]=atoi(strtok(0, ".,;()"));
		sprintf(tmpip, "%d.%d.%d.%d", portcom[0], portcom[1], portcom[2], portcom[3]);

		FD_ZERO(&readfds);
		if((session->pasv_sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1)
		{
			sprintf(sbuf, "425 Can't open data connection.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			closesocket(session->pasv_sockfd);
			session->pasv_active = 0;
			myfree(SRAMIN, sbuf);	
			return 0;
		}
		pasvremote.sin_addr.s_addr=inet_addr(tmpip);
		pasvremote.sin_port=htons(portcom[4] * 256 + portcom[5]);
		pasvremote.sin_family=PF_INET;
		if(connect(session->pasv_sockfd, (struct sockaddr *)&pasvremote, addr_len)==-1)
		{
			// is it only local address?try using gloal ip addr
			pasvremote.sin_addr=session->remote.sin_addr;
			if(connect(session->pasv_sockfd, (struct sockaddr *)&pasvremote, addr_len)==-1)
			{
				sprintf(sbuf, "425 Can't open data connection.\r\n");
				send(session->sockfd, sbuf, strlen(sbuf), 0);
				closesocket(session->pasv_sockfd);
				myfree(SRAMIN, sbuf);				
				return 0;
			}
		}
		session->pasv_active=1;
		session->pasv_port = portcom[4] * 256 + portcom[5];
		printf("Connected to Data(PORT) %s @ %d\n", tmpip, portcom[4] * 256 + portcom[5]);
		sprintf(sbuf, "200 Port Command Successful.\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	else if(str_begin_with(buf, "REST")==0)
	{
		if(atoi(parameter_ptr)>=0)
		{
			session->offset=atoi(parameter_ptr);
			sprintf(sbuf, "350 Send RETR or STOR to start transfert.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
	}
	else if(str_begin_with(buf, "MKD")==0)
	{
		if (session->is_anonymous == RT_TRUE)
		{
			sprintf(sbuf, "550 Permission denied.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			myfree(SRAMIN, sbuf);			
			return 0;
		}

		build_full_path(session, parameter_ptr, filename, 256);
		/*
		if(mkdir(filename, 0) == -1)
		{
			sprintf(sbuf, "550 File \"%s\" exists.\r\n", filename);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
		else
		{
			sprintf(sbuf, "257 directory \"%s\" successfully created.\r\n", filename);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}*/
		sprintf(sbuf, "257 directory \"%s\" successfully created.\r\n", filename);
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	else if(str_begin_with(buf, "DELE")==0)
	{
		if (session->is_anonymous == RT_TRUE)
		{
			sprintf(sbuf, "550 Permission denied.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			myfree(SRAMIN, sbuf);
			return 0;
		}

		build_full_path(session, parameter_ptr, filename, 256);
		/*
		if(unlink(filename)==0)
			sprintf(sbuf, "250 Successfully deleted file \"%s\".\r\n", filename);
		else
		{
			sprintf(sbuf, "550 Not such file or directory: %s.\r\n", filename);
		}*/
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	else if(str_begin_with(buf, "RMD")==0)
	{
		if (session->is_anonymous == RT_TRUE)
		{
			sprintf(sbuf, "550 Permission denied.\r\n");
			send(session->sockfd, sbuf, strlen(sbuf), 0);
			myfree(SRAMIN, sbuf);			
			return 0;
		}
		build_full_path(session, parameter_ptr, filename, 256);
		/*
		if(unlink(filename) == -1)
		{
			sprintf(sbuf, "550 Directory \"%s\" doesn't exist.\r\n", filename);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}
		else
		{
			sprintf(sbuf, "257 directory \"%s\" successfully deleted.\r\n", filename);
			send(session->sockfd, sbuf, strlen(sbuf), 0);
		}*/
	}
	
	else if(str_begin_with(buf, "QUIT")==0)
	{
		sprintf(sbuf, "221 Bye!\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
		myfree(SRAMIN, sbuf);		
		return -1;
	}
	else
	{
		sprintf(sbuf, "502 Not Implemented.\r\n");
		send(session->sockfd, sbuf, strlen(sbuf), 0);
	}
	myfree(SRAMIN, sbuf);	

	
	return 0;
}


void ftpd_start()
{
	struct sockaddr_in server_addr;
	
	g_send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (g_send_sockfd < 0)
	{
		printf("create send fd failed\n");
	}
	///初始化清空
	memset(&server_addr, 0, sizeof(server_addr));
	///初始化服务器，
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("192.168.1.1");    ///修改1了
	server_addr.sin_port = htons(4002);
	
	connect(g_send_sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	ftpd_thread_entry(NULL);     //TCP连接进程函数
}


int do_list(char* directory, int sockfd)
{
	
//	DIR* dirp;
//	struct dirent* entry;
	char line_buffer[256], line_length;
#ifdef _WIN32
	struct _stat s;
#else
//	struct stat s;
#endif
	line_length = sprintf(line_buffer, "-rw-r--r-- 1 admin admin %d Jan 1 2000 %s\r\n", 100, "enmpty_file");
	send(sockfd, line_buffer, line_length, 0);
	return 0;
}

int do_simple_list(char* directory, int sockfd)
{
	
	
	char line_buffer[256], line_length;
	line_length = sprintf(line_buffer, "%s\r\n", "enmpty_file");
	send(sockfd, line_buffer, line_length, 0);
	
	return 0;
}

int str_begin_with(char* src, char* match)
{
	while (*match)
	{
		/* check source */
		if (*src == 0) return -1;

		if (*match != *src) return -1;
		match ++; src ++;
	}

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(ftpd_start, start ftp server)
#endif
