#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "DDNS_9508.h"

#define DOMAIN_PORT 1818
#define MAX_RETRY_COUNT 2
#define MAX_LEN 255
const char Copyright[255] = "Author: ShareBit, QQ: 82170290, Mobile: 13823142341";

#define SERVER_IP_1 "61.145.164.180"
#define SERVER_IP_2 "58.251.49.102"

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int

typedef int SOCKET;

WORD g_uid = 0;
static int g_nLastError = 0;
int g_sockClient = 0;
struct sockaddr_in g_servAddr[2];

#define WITHOUT_THREAD //zlb //zlb20111117 去掉malloc并优化WITHOUT_THREAD

#pragma pack( push, 1 )

#define PACK_NO_PADDING  __attribute__ ((__packed__))

typedef struct
{
	BYTE  ver; //版本
	BYTE  cmd; //命令
	BYTE  userlen; //用户名长度
	BYTE  passlen; //密码长度
	BYTE  seriallen; //序列号长度
	BYTE  keylen; //密匙长度
	WORD  id; //包序号
	BYTE  szTxt[MAX_LEN]; //
} PACK_NO_PADDING REQPACK_CMD;

typedef struct
{
	BYTE ver;
	BYTE cmd;
	WORD id;
	WORD retcode;
} PACK_NO_PADDING ACKPACK_CMD;

#pragma pack( pop )

typedef struct
{
	char userid[MAX_LEN];
	int userlen;
	char password[MAX_LEN];
	int passlen;
}REG_INFO;

#ifndef WITHOUT_THREAD
REG_INFO *l = NULL;
pthread_attr_t attr;
pthread_cond_t mycond;
pthread_mutex_t mymutex;
#endif
int g_iCount = 0;

int SHX_GetLastError(void)
{
	return g_nLastError;
}

int SHX_Init(void)
{
	g_sockClient = socket(AF_INET,SOCK_DGRAM,0);
	if (g_sockClient == -1)
	{
		g_nLastError = ERR_SOCKET;
		return 0;
	}

#ifndef WITHOUT_THREAD
	pthread_cond_init(&mycond,NULL);
	pthread_mutex_init(&mymutex,NULL);
#endif
	memset(&g_servAddr[0],0,sizeof(g_servAddr[0]));
	memset(&g_servAddr[1],0,sizeof(g_servAddr[0]));

	g_servAddr[0].sin_family = AF_INET;
	g_servAddr[0].sin_port = htons(DOMAIN_PORT);
	g_servAddr[0].sin_addr.s_addr = inet_addr(SERVER_IP_1);

	g_servAddr[1].sin_family = AF_INET;
	g_servAddr[1].sin_port = htons(DOMAIN_PORT);
	g_servAddr[1].sin_addr.s_addr = inet_addr(SERVER_IP_2);

	g_iCount = 0;
#ifndef WITHOUT_THREAD
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
#endif
	//(Copyright);

	debug_lanston_ddns("initialized!");

	return 1;
}

int SHX_ReqDomain(const char *userid, int userlen,const char *password ,int passlen,char * serialnumber,int seriallen,char * key,int keylen)
{
	if ((userid == NULL) || (userlen == 0) || (password == NULL) || (passlen == 0) || (serialnumber == NULL) ||	(seriallen == 0) || (key == NULL) || (keylen == 0))
	{
		return 0;
	}

	if ((userlen > 64) || (passlen > 64) || (seriallen > 64) || (keylen > 64))
	{
		return 0;
	}

	if ((userlen < 0) || (passlen < 0) || (seriallen < 0) || (keylen < 0))
	{
		return 0;
	}

	REQPACK_CMD pack = {0};
	g_uid ++;
	pack.ver = 0x01;
	pack.cmd = 0x02;
	pack.userlen = userlen;
	pack.passlen = passlen;
	pack.seriallen = seriallen;
	pack.keylen = keylen;
	pack.id = htons(g_uid);//20100621 zlb
	memcpy(pack.szTxt,userid,userlen);
	memcpy(pack.szTxt+userlen,password,passlen);
	memcpy(pack.szTxt+userlen+passlen,serialnumber,seriallen);
	memcpy(pack.szTxt+userlen+passlen+seriallen,key,keylen);


	int Result = 0;
	int tolen = sizeof(struct sockaddr);
	struct sockaddr_in from;
	socklen_t fromlen =sizeof(from);

	int ret;
	int i;
	fd_set readfds;
	struct timeval tv;
	for (i = 0; i < MAX_RETRY_COUNT * 2; i++)
	{
		if (sendto(g_sockClient,(char*)&pack,8+userlen+passlen+seriallen+keylen,0,(struct sockaddr *)&g_servAddr[i%2],tolen) != -1)
		{
		}else
		{
			g_nLastError = ERR_SOCKET;
			Result = 0;
			continue;
		}
		//printf("req sent g_uid=0x%08x\n", g_uid);
Label1:
		
		FD_ZERO(&readfds);
		FD_SET(g_sockClient, &readfds);
		
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		ret = select(g_sockClient+1,&readfds,NULL,NULL, &tv);
		if (ret == -1)
		{
			//严重错误
			g_nLastError = ERR_SOCKET;
			Result = 0;
			printf("req select err\n");
			continue;
		}else if (ret == 0)
		{
			//超时
			g_nLastError = ERR_SOCKET_TIMEOUT;
			Result = 0;
			printf("req select timeout\n");
			continue;
		}else
		{
			if (FD_ISSET(g_sockClient,&readfds))
			{
				int recvlen = MAX_LEN;
				char recvbuf[MAX_LEN] = {0};
				ret = recvfrom(g_sockClient,recvbuf,recvlen,0,(struct sockaddr*)&from,&fromlen);
				
				if (ret == -1)
				{
					printf("req recv err\n");
					g_nLastError = ERR_SOCKET;
					Result = 0;
					usleep(50);
					continue;
				}else if (ret == 0)
				{
					printf("req recv timeout\n");
					g_nLastError = ERR_SOCKET_TIMEOUT;
					Result = 0;
					usleep(50);
					continue;
				}else
				{
					//printf("req recv ret=%u sizeof(ACKPACK_CMD)=%u\n", ret, sizeof(ACKPACK_CMD));
					if (ret == sizeof(ACKPACK_CMD))
					{
						ACKPACK_CMD cmd;
						memcpy(&cmd,recvbuf,ret);
						
						//20100621 zlb
						cmd.id = ntohs(cmd.id);
						cmd.retcode = ntohs(cmd.retcode);
						//printf("req recv cmd.cmd=0x%02x cmd.id=0x%02x cmd.retcode=0x%02x\n", cmd.cmd, cmd.id, cmd.retcode);

						if ((cmd.cmd == 0x02) && (cmd.id == g_uid) && (cmd.ver == 0x01))
						{
							if (cmd.retcode == 0x00)
							{
								g_nLastError = ERR_OK;
								return 1;
							}else
							{
								g_nLastError = ERR_OK;
								return 0;
							}
						}else
						{
							if (cmd.id < g_uid)
							{
								i +=1;
								if (i < MAX_RETRY_COUNT * 2)
								{
									goto Label1;
								}else
								{
									g_nLastError = ERR_OK;
									return 0;
								}
							}
							else
							{
								g_nLastError = ERR_OK;
								return 0;
							}
						}
					}else
					{
						g_nLastError = ERR_OK;
						Result = 0;
						continue;
					}
				}
			}
		}
	}
	
	debug_lanston_ddns("req!");

	return Result;
}

//域名解析线程
void* ThreadFunc(void* ptr)
{
	printf("$$$$$$$$$$$$$$$$$$ThreadFunc id:%d\n",getpid());
	
	int ret;
	//struct timespec timeout;
#ifdef WITHOUT_THREAD
	REG_INFO *l = (REG_INFO *)ptr;
	if (NULL == l)
	{
		return NULL;
	}
#endif
	REQPACK_CMD pack = {0};
	pack.ver = 0x01;
	pack.cmd = 0x04;
	pack.userlen = l->userlen;
	pack.passlen = l->passlen;
	pack.seriallen = 0;
	pack.keylen = 0;
	memcpy(pack.szTxt,l->userid,l->userlen);
	memcpy(pack.szTxt+l->userlen,l->password,l->passlen);
	printf("~~~~~~~~user:%s, passwd:%s\n", l->userid, l->password);
#ifndef WITHOUT_THREAD
	free(l);
	l = NULL;
#endif
	
	int tolen = sizeof(struct sockaddr);
	
#ifndef WITHOUT_THREAD
	while (1)
#endif
	{
		g_uid ++;
		pack.id = htons(g_uid);//20100621 zlb

		//保证第一次直接通信
		if ((ret = sendto(g_sockClient,(char*)&pack,8+pack.userlen+pack.passlen,0,(struct sockaddr*)&g_servAddr[g_uid%2],tolen)) != -1)
		{
			g_nLastError = ERR_OK;
		}else
		{
			g_nLastError = ERR_SOCKET;
		}

		printf("~~~~~~~~ret:%d\n", ret);
#ifndef WITHOUT_THREAD
		timeout.tv_sec = time(NULL) + 15;
		timeout.tv_nsec = 0;

		pthread_mutex_lock(&mymutex);
		ret = pthread_cond_timedwait(&mycond, &mymutex,&timeout);
		pthread_mutex_unlock(&mymutex);
		if (ret == 0) break;
#endif
		
		debug_lanston_ddns("once intvl action!");
	}
	
//pw 2010/9/6
#ifndef WITHOUT_THREAD
	pthread_exit(0);
#endif
	
	return 0;
}

void SHX_ReqStartClient(const char *userid, int userlen,const char *password ,int passlen)
{
#ifndef WITHOUT_THREAD
	l = (REG_INFO*)malloc(sizeof(REG_INFO));
#else
	REG_INFO sRegInfo;
	REG_INFO *l = &sRegInfo;
#endif
	memset(l,0,sizeof(REG_INFO));
	memcpy(l->userid,userid,strlen(userid));
	l->userlen = strlen(userid);
	memcpy(l->password,password,strlen(password));
	l->passlen = strlen(password);

#ifdef WITHOUT_THREAD
	ThreadFunc(l);
#else
	pthread_t pid;
	pthread_create(&pid, &attr, ThreadFunc, NULL);
#endif

	debug_lanston_ddns("start req!");

	g_iCount++;
}

void SHX_ReqStopClient(const char *userid, int userlen,const char *password ,int passlen)
{
	REQPACK_CMD pack = {0};
	pack.ver = 0x01;
	pack.cmd = 0x05;
	pack.userlen = userlen;
	pack.passlen = passlen;
	pack.seriallen = 0;
	pack.keylen = 0;
	memcpy(pack.szTxt,userid,userlen);
	memcpy(pack.szTxt+userlen,password,passlen);

	int tolen = sizeof(struct sockaddr);

	if (g_iCount != 0)
	{
	#ifndef WITHOUT_THREAD
		pthread_cond_signal(&mycond);
	#endif
		usleep(100 * 1000);//sleep(300);//20100621 zlb
		g_iCount--;
	}
	
	int i;
	for (i=0; i<MAX_RETRY_COUNT * 2; i++)
	{
		g_uid ++;
		pack.id = htons(g_uid);//20100621 zlb

		if (sendto(g_sockClient,(char*)&pack,8+userlen+passlen,0,(struct sockaddr*)&g_servAddr[i%2],tolen) != -1)
		{
		}else
		{
		}
	}
	
	debug_lanston_ddns("stop req	!");
}

void SHX_UnInit(void)
{
	if (g_iCount != 0)
	{
		#ifndef WITHOUT_THREAD
		pthread_cond_signal(&mycond);
		#endif
		usleep(100 * 1000);//sleep(300);//20100621 zlb
		g_iCount--;
	}
	#ifndef WITHOUT_THREAD
	pthread_attr_destroy(&attr);
	pthread_cond_destroy(&mycond);
	pthread_mutex_destroy(&mymutex);

	if (l != NULL)
	{
		free(l);
		l = NULL;
	}
	#endif
	close(g_sockClient);
	
	debug_lanston_ddns("close ddns!");

	return;
}

