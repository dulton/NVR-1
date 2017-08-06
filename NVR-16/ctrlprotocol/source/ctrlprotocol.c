#include <stdio.h>
#include <string.h>

typedef char s8;

#include "ctrlprotocol.h"
#include "common_basetypes.h"
#include "common.h"

#include "mod_config.h"
#include "mod_preview.h"

#ifdef WIN32
typedef HANDLE MutexHandle;
typedef HANDLE SemHandle;
#else
typedef pthread_mutex_t MutexHandle;
typedef sem_t SemHandle;
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#include <netinet/tcp.h>
#endif

//yaogang modify for server heart beat check
#define HEART_BEAT_INTERVAL (10*60) //10min


static char g_upnpid[16] = {0};

//csp modify 20130405
static char g_upnppw[16] = {0};

//csp modify 20130406
//#define P2P_SERVER
BOOL login_p2p_server();

static time_t get_time_sec_monotonic(void)
{
	//uint64 val;
	struct timespec time;
	
	clock_gettime(CLOCK_MONOTONIC, &time);

	//val = time.tv_sec;
	//val = val*1000 + time.tv_nsec/(1000*1000);

	return time.tv_sec;
} 

//pw 2010/9/10
int looprecv(SOCKET s, char * buf, unsigned int rcvsize)
{
	u32 remian = rcvsize;
	u32 recvlen = 0;
	int ret = 0;
	while(remian > 0)
	{
		ret=recv(s,buf+recvlen,remian,0);
		if(ret <= 0)
		{
			return ret;
		}
		recvlen += ret;
		remian -= ret;
	} 
	return recvlen;
}

//pw 2010/9/10
int loopsend(SOCKET s, char *buf, unsigned int sndsize)
{
	int remian = sndsize;
	int sendlen = 0;
	int ret = 0;
	while(remian > 0)
	{
		ret = send(s,buf+sendlen,remian,0);
		if(ret <= 0)
		{
			return ret;
		}
		sendlen += ret;
		remian -= ret;
	}
	return sndsize;
}



const u8 gc_protocolGUID[16] = {0x61,0x78,0xDA,0xB5,0xD3,0x8E,0x43,0xdb,0x9E,0xD7,0xF2,0x20,0x78,0x36,0x18,0x79};



//linux下接收标志应该设置成MSG_NOSIGNAL?

#define CTRL_PROTOCOL_TYPE			SOCK_STREAM//SOCK_STREAM TCP方式

// #ifdef WIN32
struct TCP_KEEPALIVE {
    u_long  onoff;
    u_long  keepalivetime;
    u_long  keepaliveinterval;
};

#define SIO_KEEPALIVE_VALS   _WSAIOW(IOC_VENDOR,4)
// #endif

static BOOL SockClose(SOCKHANDLE hSock)
{
	//printf("SockClose sockfd=%d\n",hSock);
	if(hSock != INVALID_SOCKET)
	{
	#ifdef WIN32
		closesocket(hSock);
	#else
		close(hSock);
	#endif
	}
	return TRUE;
}

u16 CleanCPHandle(CPHandle cph)
{
	if(cph->sockfd != INVALID_SOCKET)
	{
		SockClose(cph->sockfd);
		cph->sockfd = INVALID_SOCKET;
	}
	cph->ip = 0;
	cph->port = 0;
	cph->conntype = CTRL_CONNECTION_NULL;
	cph->newmsgcome = 0;
	cph->nolnkcount = 0;
	cph->last_msg_time = 0;
	memset(&cph->guid,0,sizeof(cph->guid));
	return CTRL_SUCCESS;
}

SOCKHANDLE ConnectWithTimeout(u32 dwHostIp, u16 wHostPort, u32 dwTimeOut, u16 *pwErrorCode)
{
	SOCKHANDLE hClient = INVALID_SOCKET;
	struct sockaddr_in remote;
	fd_set ConnectSet;
	struct timeval timeVal;
	int ret;
#ifdef WIN32
	LPVOID lpMsgBuf;
	unsigned long arg;
#else
	int arg;
	int nErrCode;
	socklen_t errorlen;
#endif
	if(pwErrorCode) *pwErrorCode = CTRL_SUCCESS;
	hClient = socket(AF_INET,CTRL_PROTOCOL_TYPE,0);
	if(hClient == INVALID_SOCKET)
	{
	#ifdef WIN32
		printf("ConnectWithTimeout:create socket failed,errcode=%d\n",WSAGetLastError());
	#else
		printf("ConnectWithTimeout:create socket failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
	#endif
		if(pwErrorCode) *pwErrorCode = CTRL_FAILED_CREATESOCKET;
		return INVALID_SOCKET;
	}
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = dwHostIp;
	remote.sin_port = htons(wHostPort);
	if(dwTimeOut == CTRL_PROTOCOL_CONNECT_BLOCK)
	{
		ret = connect(hClient,(struct sockaddr *)&remote,sizeof(remote));
		if(ret == SOCKET_ERROR)
		{
		#ifdef WIN32
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
				(LPTSTR) &lpMsgBuf, 0, NULL);
			printf("ConnectWithTimeout:connect failed,errcode=%d,errmsg=%s\n",WSAGetLastError(),(LPCTSTR)lpMsgBuf);
			closesocket(hClient);
		#else
			printf("ConnectWithTimeout:connect failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
			close(hClient);
		#endif
			if(pwErrorCode) *pwErrorCode = CTRL_FAILED_CONNECT;
			return INVALID_SOCKET;
		}
	}
	else
	{
		arg = 1;
		//设置为非阻塞模式, 防止connect阻塞
	#ifdef WIN32
		ret = ioctlsocket(hClient,FIONBIO,&arg);
	#else
		ret = ioctl(hClient,FIONBIO,(int)&arg);
	#endif //WIN32
		if(ret == SOCKET_ERROR)
		{
			SockClose(hClient);
			if(pwErrorCode) *pwErrorCode = CTRL_FAILED_CONNECT;
			return INVALID_SOCKET;
		}
		ret = connect(hClient, (struct sockaddr *)&remote, sizeof(remote));
		if(ret == SOCKET_ERROR)
		{
			FD_ZERO(&ConnectSet);
			timeVal.tv_sec  = (dwTimeOut/1000);
			timeVal.tv_usec = (dwTimeOut%1000)*1000;
			FD_SET(hClient, &ConnectSet);
			ret = select(hClient+1, NULL, &ConnectSet, NULL, &timeVal);
			//printf("%s sockfd: %d, ret: %d\n", __func__, hClient, ret);
			if(ret > 0 && FD_ISSET(hClient, &ConnectSet))
			{
				//printf("%s ret > 0 && FD_ISSET(hClient, &ConnectSet)\n", __func__);
			#ifndef WIN32
				nErrCode = 1;
				errorlen = sizeof(nErrCode);
				ret = getsockopt(hClient, SOL_SOCKET, SO_ERROR, &nErrCode, &errorlen);
				if(SOCKET_ERROR == ret || nErrCode != 0)
				{
					printf("%s SOCKET_ERROR == ret || nErrCode != 0\n", __func__);
					close(hClient);
					if(pwErrorCode) 
					{
						*pwErrorCode = CTRL_FAILED_CONNECT;
					}
					
					return INVALID_SOCKET;
				}
			#endif
			}
			else
			{
				SockClose(hClient);
				if(ret == 0)
				{
					if(pwErrorCode) *pwErrorCode = CTRL_FAILED_TIMEOUT;
					return INVALID_SOCKET;
				}
				else
				{
					if(pwErrorCode) *pwErrorCode = CTRL_FAILED_CONNECT;
					return INVALID_SOCKET;
				}
			}
		}
		//设置为阻塞模式
		arg = 0;
	#ifdef WIN32
		ret = ioctlsocket(hClient,FIONBIO,&arg);
	#else
		ret = ioctl(hClient,FIONBIO,(int)&arg);
	#endif //WIN32
		if(ret == SOCKET_ERROR)
		{
			SockClose(hClient);
			if(pwErrorCode) *pwErrorCode = CTRL_FAILED_CONNECT;
			return INVALID_SOCKET;
		}
	}
	return hClient;
}

static u16 SetSockLinkOpt(SOCKHANDLE hSock)
{
	int ret = 0;

	//设置NODELAY选项
#ifdef WIN32
	char optval = 1;
#else
	int optval = 1;
#endif
	ret = setsockopt( hSock, IPPROTO_TCP/*SOL_SOCKET*/, TCP_NODELAY, (char *)&optval, sizeof(optval) );
	if( SOCKET_ERROR == ret )
	{
		printf("SetSockLinkOpt:set socket nodelay error!\n");
		return CTRL_FAILED_UNKNOWN;
	}

	//设置SO_LINGER为零(亦即linger结构中的l_onoff域设为非零,但l_linger为0),便
	//不用担心closesocket调用进入“锁定”状态(等待完成),不论是否有排队数据未发
	//送或未被确认。这种关闭方式称为“强行关闭”，因为套接字的虚电路立即被复位，尚
	//未发出的所有数据都会丢失。在远端的recv()调用都会失败，并返回WSAECONNRESET错误。
	{
		struct linger m_sLinger;
		m_sLinger.l_onoff = 1;  //(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
		m_sLinger.l_linger = 1; //(容许逗留的时间为0秒)				//dong	100424		m_sLinger.l_linger = 0; 
		ret = setsockopt(hSock,SOL_SOCKET,SO_LINGER,(char*)&m_sLinger,sizeof(struct linger));
		if( SOCKET_ERROR == ret )
		{
			printf("SetSockLinkOpt:set socket linger error!\n");
			return CTRL_FAILED_UNKNOWN;
		}
	}

#ifdef WIN32
	{
		struct TCP_KEEPALIVE klive;
		struct TCP_KEEPALIVE outKeepAlive = {0,0,0};//输出参数
		unsigned long ulBytesReturn = 0;
		int ret2 = 0;

		klive.onoff = 1;//启用保活
		klive.keepalivetime = 5000;
		klive.keepaliveinterval = 1000 * 5;//重试间隔为5秒 Resend if No-Reply

		ret2 = WSAIoctl(
			hSock,
			SIO_KEEPALIVE_VALS,
			&klive,
			sizeof(klive),
			&outKeepAlive,
			sizeof(outKeepAlive),
			(unsigned long *)&ulBytesReturn,
			0,
			NULL
		);
	}
#endif

//设置超时2s
#ifdef WIN32
		int timeo = 2000;
#else
		struct timeval timeo = { 2, 0 };
#endif
	ret = setsockopt(hSock, SOL_SOCKET, SO_SNDTIMEO,(void *) &timeo, sizeof(timeo));
	if(ret < 0)
	{
    	printf("ERROR: %s set send timeout failed\n", __func__);
		return CTRL_FAILED_UNKNOWN;
	}
    //set receive timeout
    ret = setsockopt(hSock,SOL_SOCKET,SO_RCVTIMEO,(void *) &timeo, sizeof(timeo));
	if(ret < 0)
	{
    	printf("ERROR: %s set receive timeout failed\n", __func__);
		return CTRL_FAILED_UNKNOWN;
	}

	return CTRL_SUCCESS;
}

static pAddStreamTCPLink pAddStreamLinkCB = NULL;
static void* pAddStreamLinkCBContext = NULL;

static pFindDeivce_CB pFindCB = NULL;
static void* pFindCBContext = NULL;

static u16 g_wCliAckPort = ACKSEARCHPORT;

static pMESSAGE_CB pfuncMsgCB = NULL;
static void* pCallBackContext = NULL;

static BOOL bCPInit = FALSE;
static THREADHANDLE hCPTask = (THREADHANDLE)NULL;
static THREADHANDLE hCPCheckTask = (THREADHANDLE)NULL;
static THREADHANDLE	hCPSearchTask = (THREADHANDLE)NULL;

static SOCKHANDLE hServerSock = INVALID_SOCKET;//服务器套接字
static ifly_cp_t hCPLink[CTRL_PROTOCOL_MAXLINKNUM];

static SOCKHANDLE hInterSock = INVALID_SOCKET;//内部消息套接字
static u16 wInterPort = 0;//内部消息套接字绑定端口

static u8  g_byAckBuf[4096];
static int g_dwAckLen;

static u8  g_bySyncAckBuf[4096];
static int g_dwSyncAckLen;

static MutexHandle g_hSyncMutex;
static SemHandle   g_hSyncSem;
static u16		   g_wSyncNumber = 0;//等待同步的消息的流水号


static SemHandle   g_hFindSem;

static BOOL CreateMutexHandle(MutexHandle *pMutex)
{
	BOOL bRet = TRUE;
#ifdef WIN32
	*pMutex = CreateMutex(NULL, FALSE, NULL);
	if(*pMutex == NULL) bRet = FALSE;
#else
	bRet = ( 0 == pthread_mutex_init(pMutex,NULL));
#endif
	return bRet;
}

static BOOL CloseMutexHandle(MutexHandle hMutex)
{
#ifdef WIN32
	return CloseHandle(hMutex);
#else
	return ( 0 == pthread_mutex_destroy(&hMutex) );
#endif
}

static BOOL LockMutex(MutexHandle hMutex)
{
#ifdef WIN32
	return ( WAIT_FAILED != WaitForSingleObject(hMutex,INFINITE) );
#else
	return ( 0 == pthread_mutex_lock(&hMutex) );
#endif
}

static BOOL UnlockMutex(MutexHandle hMutex)
{
#ifdef WIN32
	return ReleaseMutex(hMutex);
#else
	return ( 0 == pthread_mutex_unlock(&hMutex) );
#endif
}

static BOOL SemBCraete(SemHandle *pSem)
{
	BOOL bRet = TRUE;
#ifdef WIN32
	*pSem = CreateSemaphore(NULL, 1, 1, NULL);
	if(*pSem == NULL) bRet = FALSE;
#else
	bRet = ( 0 == sem_init(pSem,0,1));
#endif
	return bRet;
}

static BOOL SemDelete(SemHandle hSem)
{
#ifdef WIN32
	return CloseHandle(hSem);
#else
	return ( 0 == sem_destroy(&hSem) );
#endif
}

static BOOL SemTake(SemHandle hSem)
{
#ifdef WIN32
	return ( WAIT_FAILED != WaitForSingleObject(hSem,INFINITE) );
#else
	return ( 0 == sem_wait(&hSem) );
#endif
}

static BOOL SemTakeByTime(SemHandle hSem,u32 dwTimeout)
{
#ifdef WIN32
	return ( WAIT_OBJECT_0 == WaitForSingleObject(hSem,dwTimeout) );
#else
	if(dwTimeout == 0)
	{
		return ( 0 == sem_trywait(&hSem) );
	}
	else
	{
		//csp modify 20140501
		//return ( 0 == sem_wait(&hSem) );
		u32 nloop = (dwTimeout + 9) / 10;
		while(nloop)
		{
			if(0 == sem_trywait(&hSem))
			{
				return TRUE;
			}
			nloop--;
		}
		return FALSE;
	}
#endif
}

static BOOL SemGive(SemHandle hSem)
{
#ifdef WIN32
	LONG previous;
	return ReleaseSemaphore(hSem, 1, (LPLONG)&previous);
#else
	return ( 0 == sem_post(&hSem) );
#endif
}

void SetMsgCallBack(pMESSAGE_CB pCB,void *pContext)
{
	pfuncMsgCB = pCB;
	pCallBackContext = pContext;
}

int SendTcpConnHead(SOCKHANDLE hSock, u8 type)
{
	ifly_ProtocolHead_t Protocolhdr;
	int ret = 0;
	memset(&Protocolhdr, 0, sizeof(Protocolhdr));
	memcpy(Protocolhdr.safeguid, gc_protocolGUID, 16);
	Protocolhdr.protocolver = 1;
	Protocolhdr.byConnType = type;
	ret = send(hSock,(char *)&Protocolhdr,sizeof(Protocolhdr),0);
	return ret;
}

int CheckTcpConnType(SOCKHANDLE hSock)
{
	ifly_ProtocolHead_t Protocolhdr;
	
	int remian = sizeof(ifly_ProtocolHead_t);
	int recvlen = 0;
	int ret = 0;
	u8 buf[256] = {0};
	while(remian > 0)
	{
		//ret = recv(hSock,buf+recvlen,remian,0);//yqluo 20110301
		
		struct timeval timeout = {3,0};
		struct timeval timeout0 = {0,0};
		
		socklen_t optlen = sizeof(struct timeval);
		getsockopt(hSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout0, &optlen);
		//printf("timeout0 = %d\n", timeout0.tv_sec);
		
		setsockopt(hSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
		
		ret = recv(hSock,buf+recvlen,remian,0);
		
		setsockopt(hSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout0, sizeof(struct timeval));
		
		if(ret < 0)
		{
			break;
		}
		if(ret == 0)
		{
			break;
		}
		recvlen += ret;
		remian -= ret;
		
		//pw 2010/9/10
		if(memcmp(buf, gc_protocolGUID, min(recvlen,16)))
		{
			return -1;
		}
	}
	
	if(recvlen != sizeof(ifly_ProtocolHead_t))
	{
		return -1;
	}
	
	memcpy(&Protocolhdr, buf, sizeof(ifly_ProtocolHead_t));
	
	if(memcmp(Protocolhdr.safeguid, gc_protocolGUID, 16))
	{
		return -1;
	}
	
	return Protocolhdr.byConnType;
}

void SetKeepAlive(SOCKHANDLE hSock) //zlb20111110 bugfix
{
	////KeepAlive实现，单位秒  //zlb201109
	int keepAlive = 1;//设定KeepAlive  
	int keepIdle = 5;//开始首次KeepAlive探测前的TCP空闭时间  
	int keepInterval = 5;//两次KeepAlive探测间的时间间隔  
	int keepCount = 3;//判定断开前的KeepAlive探测次数  
	
	if (setsockopt(hSock,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepAlive,sizeof(keepAlive)) == -1)
	{
		printf("hSock=%d, set SO_KEEPALIVE failed, errno=%d, errstr=%s\n", hSock, errno, strerror(errno));
		return;
	}
	if (setsockopt(hSock,SOL_TCP,TCP_KEEPIDLE,(void *)&keepIdle,sizeof(keepIdle)) == -1)
	{
		printf("hSock=%d, set TCP_KEEPIDLE failed\n", hSock);
		return; 
	}
	if (setsockopt(hSock,SOL_TCP,TCP_KEEPINTVL,(void *)&keepInterval,sizeof(keepInterval)) == -1)
	{
		printf("hSock=%d, set TCP_KEEPINTVL failed\n", hSock);
		return;
	}
	if (setsockopt(hSock,SOL_TCP,TCP_KEEPCNT,(void *)&keepCount,sizeof(keepCount)) == -1)
	{
		printf("hSock=%d, set TCP_KEEPCNT failed\n", hSock);
	 	return;
	}
}

CPHandle AddCPLink(SOCKHANDLE hSock,u8 conntype,u32 ip,u16 port,u16 *pwErrorCode)
{
	int i;
	struct in_addr in;
	
	for(i=0;i<CTRL_PROTOCOL_MAXLINKNUM;i++)
	{
		pthread_mutex_lock(&hCPLink[i].lock);
		if(hCPLink[i].sockfd == INVALID_SOCKET)
		{
 			SetKeepAlive(hSock); //zlb20111110 bugfix
			
			hCPLink[i].sockfd = hSock;
			hCPLink[i].conntype = conntype;
			hCPLink[i].ip = ip;
			hCPLink[i].port = port;
			hCPLink[i].newmsgcome = 0;
			hCPLink[i].nolnkcount = 0;
			hCPLink[i].last_msg_time = get_time_sec_monotonic();
			memset(&hCPLink[i].guid,0,sizeof(hCPLink[i].guid));

			in.s_addr = ip;
			printf("AddCPLink client ip %s, time: %d\n",
					inet_ntoa(in), (int)hCPLink[i].last_msg_time);

			pthread_mutex_unlock(&hCPLink[i].lock);
			
			if(pwErrorCode) *pwErrorCode = CTRL_SUCCESS;
			return &hCPLink[i];
		}
		pthread_mutex_unlock(&hCPLink[i].lock);
	}
	if(pwErrorCode) *pwErrorCode = CTRL_FAILED_LINKLIMIT;
	return NULL;
}

void *CheckConnTaskProc(void *pParam)
{
	int i = 0;
	//u8 count = 0;
	
#ifndef HISI_3515
#ifndef WIN32
	printf("$$$$$$$$$$$$$$$$$$CheckConnTaskProc id:%d\n",gettid());
	Dump_Thread_Info("CPCheck",gettid());
#endif
#endif
	
	printf("yg CheckConnTaskProc id\n");
	while(1)
	{
#ifndef WIN32
		pthread_testcancel();
		sleep(20);
#else
		Sleep(20000);
#endif
		//printf("CheckConnTaskProc running\n");
		//if(++count >= 3)
		{
			//count = 0;
			//检测断链:如果超过3次没有收到客户端的任何消息则认为断链
			for(i=0;i<CTRL_PROTOCOL_MAXLINKNUM;i++)
			{
				if(hCPLink[i].sockfd != INVALID_SOCKET)
				{
					if(hCPLink[i].newmsgcome)
					{
						hCPLink[i].newmsgcome = 0;
						hCPLink[i].nolnkcount = 0;
					}
					else
					{
						if(++hCPLink[i].nolnkcount >= 3)
						{

							if(pfuncMsgCB != NULL)
							{
								//MessageBox(NULL, "CNNLOST:CheckConnTaskProc",NULL,MB_OK);
								pfuncMsgCB(&hCPLink[i],CTRL_NOTIFY_CONNLOST,NULL,0,NULL,NULL,pCallBackContext);
							}

							CleanCPHandle(&hCPLink[i]);
							continue;
						}
					}
				}
			}
		}
		
		//发送链路检测心跳消息
		for(i=0;i<CTRL_PROTOCOL_MAXLINKNUM;i++)
		{
			if(hCPLink[i].sockfd != INVALID_SOCKET && hCPLink[i].conntype == CTRL_CONNECTION_TCPCLIENT)
			{
				ifly_cp_header_t cpsndhead;
				cpsndhead.length	= htonl(sizeof(ifly_cp_header_t));
				cpsndhead.type		= htons(CTRL_NOTIFY);
				cpsndhead.event		= htons(CTRL_NOTIFY_HEARTBEAT_REQ);
				cpsndhead.number	= htons(GetTransactionNum());
				cpsndhead.version	= htons(CTRL_VERSION);
				send(hCPLink[i].sockfd,(char *)&cpsndhead,ntohl(cpsndhead.length),0);
			}
		}
	}
	
	return 0;
}

void *CPTaskProc(void *pParam)
{
	fd_set set;
	struct timeval timeout;
	int ret;
	int i;
	SOCKHANDLE hTmp;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	u8 abyBuf[4096];
	ifly_cp_header_t cpsndhead;
	ifly_cp_header_t cprcvhead;
	u16 wRet;

	time_t cur_time_s = 0;//yaogang modify for server heart beat check
	time_t last_msg_time_s = 0;
	int sock_fd = INVALID_SOCKET;
	int sock_fd_max = INVALID_SOCKET;
	struct in_addr in;
	
#ifndef HISI_3515
	#ifndef WIN32
	printf("$$$$$$$$$$$$$$$$$$CPTaskProc id:%d\n",gettid());
	Dump_Thread_Info("CPTask",gettid());
	#endif
#else
	#ifndef WIN32
	printf("$$$$$$$$$$$$$$$$$$CPTaskProc id:%d\n",getpid());
	#endif
#endif
	
	while(1)
	{
#ifndef WIN32
		pthread_testcancel();
#endif
		cur_time_s = get_time_sec_monotonic();
		FD_ZERO(&set);
		
		if(hServerSock != INVALID_SOCKET)
		{
			FD_SET(hServerSock, &set);
			if (sock_fd_max < hServerSock)
			{
				sock_fd_max = hServerSock;
			}
		}
		if(hInterSock != INVALID_SOCKET) 
		{
			FD_SET(hInterSock, &set);
			if (sock_fd_max < hInterSock)
			{
				sock_fd_max = hInterSock;
			}
		}
		
		for(i=0;i<CTRL_PROTOCOL_MAXLINKNUM;i++)
		{
			pthread_mutex_lock(&hCPLink[i].lock);

			sock_fd = hCPLink[i].sockfd;
			last_msg_time_s = hCPLink[i].last_msg_time;
			in.s_addr = hCPLink[i].ip;
			
			pthread_mutex_unlock(&hCPLink[i].lock);
			
			if(sock_fd == INVALID_SOCKET)
			{
				continue;
			}

			if (abs(cur_time_s - last_msg_time_s) > HEART_BEAT_INTERVAL+5*60)// 15 min
			{
				printf("client %s connect lost\n", inet_ntoa(in));
				if(pfuncMsgCB != NULL)
				{
					//printf("CPTaskProc CTRL_NOTIFY_CONNLOST\n");
					pfuncMsgCB(&hCPLink[i],CTRL_NOTIFY_CONNLOST,NULL,0,NULL,NULL,pCallBackContext);
				}

				pthread_mutex_lock(&hCPLink[i].lock);
				CleanCPHandle(&hCPLink[i]);
				pthread_mutex_unlock(&hCPLink[i].lock);

				continue;
			}

			FD_SET(hCPLink[i].sockfd,&set);			
		}
		
		//linux平台下timeout会被修改以表示剩余时间,故每次都要重新赋值
		timeout.tv_sec = 20;
		timeout.tv_usec = 0;
		ret = select(FD_SETSIZE,&set,NULL,NULL,&timeout);
		if(ret == 0)
		{
			//printf("CPTaskProc:select sock time out\n");
			continue;
		}
		if(ret == SOCKET_ERROR)
		{
			printf("CPTaskProc:select sock error:[%s]!\n",strerror(errno));
			#ifndef WIN32
			//应延时一段时间
			sleep(1);
			#else
			Sleep(1000);
			#endif
			continue;
		}
		for(i=0;i<CTRL_PROTOCOL_MAXLINKNUM;i++)
		{
			//客户端消息:回调函数处理
			if(hCPLink[i].sockfd != INVALID_SOCKET && FD_ISSET(hCPLink[i].sockfd,&set))
			{
				//printf("recv msg start\n");
				//MessageBox(NULL, "RECV 1", NULL,MB_OK);
				
				{
					int remian = sizeof(ifly_cp_header_t);
					int recvlen = 0;
					while(remian > 0)
					{
						
						ret=recv(hCPLink[i].sockfd,(char *)abyBuf+recvlen,remian,0);
						if(ret<=0)
						{
							break;
						}
						recvlen += ret;
						remian -= ret;
					}
				}
				
				//MessageBox(NULL, "RECV 2", NULL,MB_OK);
				//printf("recv msg end\n");
				if(ret<=0)
				{
					in.s_addr = hCPLink[i].ip;
					printf("%s recv failed, ip: %s\n", __func__, inet_ntoa(in));

					if(pfuncMsgCB != NULL)
					{
						pfuncMsgCB(&hCPLink[i],CTRL_NOTIFY_CONNLOST,NULL,0,NULL,NULL,pCallBackContext);
					}

					pthread_mutex_lock(&hCPLink[i].lock);
					CleanCPHandle(&hCPLink[i]);
					pthread_mutex_unlock(&hCPLink[i].lock);
					continue;
				}
				
				hCPLink[i].newmsgcome = 1;
				hCPLink[i].last_msg_time = get_time_sec_monotonic();
				
				if(ret >= sizeof(ifly_cp_header_t))
				{
					int ret2 = 0;
					int msglen = 0;
					memcpy(&cprcvhead,abyBuf,sizeof(cprcvhead));
					cprcvhead.length	= ntohl(cprcvhead.length);
					cprcvhead.type		= ntohs(cprcvhead.type);
					cprcvhead.version	= ntohs(cprcvhead.version);
					cprcvhead.number	= ntohs(cprcvhead.number);
					cprcvhead.event		= ntohs(cprcvhead.event);


					if(cprcvhead.length > sizeof(ifly_cp_header_t))
					{
						msglen = cprcvhead.length - sizeof(ifly_cp_header_t);

						{
							int remian = msglen;
							int recvlen = 0;
							while(remian > 0)
							{
								ret2=recv(hCPLink[i].sockfd,(char *)abyBuf+sizeof(ifly_cp_header_t)+recvlen,remian,0);
								if(ret2<=0)
								{
									cprcvhead.event = CTRL_FAILED_NETRCV;
									memcpy(abyBuf, &cprcvhead, sizeof(ifly_cp_header_t));
									break;
								}
								recvlen += ret2;
								remian -= ret2;
							}	
						}
					}

					if(cprcvhead.length < sizeof(ifly_cp_header_t) || cprcvhead.type > CTRL_ACK || cprcvhead.version != CTRL_VERSION)
					{
						//MessageBox(NULL, "CONTINUE 2", NULL,MB_OK);
						continue;
					}

					printf("yg CPTaskProc recv: \n");
					printf("\ttype: %d\n", cprcvhead.type);
					printf("\tevent: %d\n", cprcvhead.event);
					printf("\tlength: %d\n", cprcvhead.length);

					
					if(cprcvhead.type == CTRL_ACK)
					{
						if(cprcvhead.number == g_wSyncNumber)
						{

							memcpy(g_bySyncAckBuf,abyBuf,cprcvhead.length);
							g_dwSyncAckLen = cprcvhead.length;

							SemGive(g_hSyncSem);
							//printf("CPSend ack msg%d\n",cprcvhead.number);
						}
						//MessageBox(NULL, "CONTINUE 3", NULL,MB_OK);
						continue;
					}
					if(cprcvhead.event == CTRL_NOTIFY_HEARTBEAT_REQ)
					{
						in.s_addr = hCPLink[i].ip;
						//printf("recv heartbeat req msg from (0%s,%d)\n", inet_ntoa(in), hCPLink[i].port);

						cpsndhead.length	= htonl(sizeof(ifly_cp_header_t));
						cpsndhead.type		= htons(CTRL_NOTIFY);
						cpsndhead.event		= htons(CTRL_NOTIFY_HEARTBEAT_RESP);
						cpsndhead.number	= htons(cprcvhead.number);
						cpsndhead.version	= htons(cprcvhead.version);
#if 1
						if (send(hCPLink[i].sockfd,(char *)&cpsndhead,ntohl(cpsndhead.length),0) < 0)
						{
							printf("send heart beat resp failed, ip: %s\n", inet_ntoa(in));
							
							if(pfuncMsgCB != NULL)
							{
								pfuncMsgCB(&hCPLink[i],CTRL_NOTIFY_CONNLOST,NULL,0,NULL,NULL,pCallBackContext);
							}

							pthread_mutex_lock(&hCPLink[i].lock);
							CleanCPHandle(&hCPLink[i]);
							pthread_mutex_unlock(&hCPLink[i].lock);
						}
#endif
						//MessageBox(NULL, "CONTINUE 4", NULL,MB_OK);
						continue;
					}
					if(cprcvhead.event == CTRL_NOTIFY_HEARTBEAT_RESP)
					{
						//printf("recv heartbeat resp msg from (0x%08x,%d)\n",hCPLink[i].ip,hCPLink[i].port);
						//MessageBox(NULL, "CONTINUE 5", NULL,MB_OK);
						continue;
					}
					g_dwAckLen = 0;
					wRet = CTRL_FAILED_UNKNOWN;

					if(pfuncMsgCB != NULL)
					{
						g_dwAckLen = sizeof(g_byAckBuf)-sizeof(ifly_cp_header_t);
						//printf("recv msg,call back start\n");
						
						//MessageBox(NULL, "recv msg,call back start\n", NULL,MB_OK);
						//printf("event %d \n", cprcvhead.event);
						
						wRet = pfuncMsgCB(&hCPLink[i],cprcvhead.event,abyBuf+sizeof(ifly_cp_header_t),cprcvhead.length-sizeof(ifly_cp_header_t),g_byAckBuf+sizeof(ifly_cp_header_t),&g_dwAckLen,pCallBackContext);
						
						//printf("ret %d event %d \n", wRet, cprcvhead.event);
						//printf("recv msg,call back end\n");
					}
					else
					{
						//MessageBox(NULL, "call back start null\n", NULL,MB_OK);
					}
					
					if(cprcvhead.type == CTRL_COMMAND)
					{
						//printf("cmd %d err %d \n", cprcvhead.type, wRet);
						cpsndhead.length	= ntohl(sizeof(ifly_cp_header_t)+g_dwAckLen);
						cpsndhead.type		= ntohs(CTRL_ACK);
						cpsndhead.event		= ntohs(wRet);
						cpsndhead.number	= ntohs(cprcvhead.number);
						cpsndhead.version	= ntohs(cprcvhead.version);
						memcpy(g_byAckBuf,&cpsndhead,sizeof(ifly_cp_header_t));

						//printf("send ack msg start, number: %d, event: %d\n", cprcvhead.number, cprcvhead.event);
						//pw 2010/9/10
						if (loopsend(hCPLink[i].sockfd,(char *)g_byAckBuf,ntohl(cpsndhead.length)) <= 0)
						{
							in.s_addr = hCPLink[i].ip;
							printf("send cmd ack failed, ip: %s\n", inet_ntoa(in));
							
							if(pfuncMsgCB != NULL)
							{
								pfuncMsgCB(&hCPLink[i],CTRL_NOTIFY_CONNLOST,NULL,0,NULL,NULL,pCallBackContext);
							}

							pthread_mutex_lock(&hCPLink[i].lock);
							CleanCPHandle(&hCPLink[i]);
							pthread_mutex_unlock(&hCPLink[i].lock);
						}
						//printf("send ack msg end, event: %d\n", cprcvhead.event);
					}
				}
			}
		}
		//连接消息:接受连接
		if(hServerSock != INVALID_SOCKET && FD_ISSET(hServerSock,&set))
		{
			addrlen = sizeof(struct sockaddr_in);
			hTmp = accept(hServerSock,(struct sockaddr *)&clientaddr,&addrlen);
			if(hTmp != INVALID_SOCKET)
			{
				SetSockLinkOpt(hTmp);
				
				int Conntype = 0;
				Conntype = CheckTcpConnType(hTmp);
				
				if(0x1 == Conntype)
				{
					//printf("cpconnect\n");
					if(NULL == AddCPLink(hTmp,CTRL_CONNECTION_TCPSERVER,clientaddr.sin_addr.s_addr,ntohs(clientaddr.sin_port),NULL))
					{
						SockClose(hTmp);
					}
					else
					{
						printf("accept cmdlink success,connectin from(%s,%d),sockfd=%d\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port),hTmp);
					}
				}
				else if(0x2 == Conntype)
				{
					printf("accept medialink success,connectin from(%s,%d),sockfd=%d\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port),hTmp);
					
					ifly_TCP_Stream_Req reqhdr;
					
					int remian = sizeof(ifly_TCP_Stream_Req);
					int recvlen = 0;
					int ret = 0;
					u8 buf[256] = {0};
					while(remian > 0)
					{
						ret=recv(hTmp,buf+recvlen,remian,0);
						if(ret < 0)
						{
							break;
						}
						if(ret == 0)
						{
							break;
						}
						recvlen += ret;
						remian -= ret;
					}
					
					if(recvlen != sizeof(ifly_TCP_Stream_Req))
					{
						SockClose(hTmp);
					}
					else
					{
						memcpy(&reqhdr, buf, sizeof(ifly_TCP_Stream_Req));
						if(pAddStreamLinkCB)
						{
							if(pAddStreamLinkCBContext)
							{
								struct sockaddr_in* pAddr = pAddStreamLinkCBContext;
								memcpy(pAddr, &clientaddr, sizeof(clientaddr));
							}

							printf("client ip: %s, req cmd: %d, chn: %d, type:%d\n", 
								inet_ntoa(clientaddr.sin_addr), 
								reqhdr.command, 
								reqhdr.Monitor_t.chn,
								reqhdr.Monitor_t.type);
							
							int ret = pAddStreamLinkCB(hTmp, reqhdr, pAddStreamLinkCBContext);
							if (ret < 0)
							{
								SockClose(hTmp);
							}
							else
							{
								SetKeepAlive(hTmp); //zlb20111110 bugfix
								//printf("accept success,connectin from(%s,%d)sockfd=%d\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port),hTmp);
							}
						}
						else
						{
							SockClose(hTmp); //zlb20111110 bugfix
						}
					}
				}
				else
				{
					SockClose(hTmp);
				}
			}
		}
		//内部消息:不作处理
		if(hInterSock != INVALID_SOCKET && FD_ISSET(hInterSock,&set))
		{
			ret = recvfrom(hInterSock, abyBuf, sizeof(abyBuf), 0, NULL, NULL);
			if(ret == SOCKET_ERROR)
			{
#ifdef WIN32
				LPVOID lpMsgBuf;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
					(LPTSTR) &lpMsgBuf, 0, NULL);				
				printf("CPTaskProc:Internal udp message error:[%s]!\n", (LPCTSTR)lpMsgBuf);
#else
				printf("CPTaskProc:Internal udp message error(%d):[%s]!\n", errno, strerror(errno));
#endif
			}
		}
	}
	
	return 0;
}

static ifly_DeviceInfo_t g_deviceInfo;

//csp modify 20130406
typedef struct DevInfo_t
{
	unsigned char	safeguid[7];		// PROTOGUID	   tl-tek\0                 
	unsigned char	device_id[32];		//设备ID
	unsigned char	pass[32];			//密码
	unsigned int	ip;					//IP
	unsigned short	port;				//端口
	unsigned char	reserved[32];		//
}DevInfo_s;
//返回信息
typedef struct ReDev_t
{
	unsigned short	returnVal;			//返回值
}ReDev_s;
BOOL login_p2p_server()
{
	DevInfo_s g_DevInfo;
	memset(&g_DevInfo,0,sizeof(DevInfo_s));
	memcpy(g_DevInfo.safeguid,"tl-tek",7);
	strcpy((char *)g_DevInfo.device_id,g_upnpid);
	strcpy((char *)g_DevInfo.pass,g_upnppw);
	g_DevInfo.ip = g_deviceInfo.deviceIP;
	g_DevInfo.port = g_deviceInfo.devicePort;
	
	printf("login_p2p_server start......\n");
	
	int g_SocketDevice = socket(AF_INET,SOCK_STREAM,0);
	if(g_SocketDevice == -1)
	{
		return FALSE;
	}
	
	struct sockaddr_in sockStruct;
	sockStruct.sin_family = AF_INET;
	sockStruct.sin_port = htons(50001);
	sockStruct.sin_addr.s_addr = inet_addr("50.22.77.7");//inet_addr("192.168.1.101");
	if(connect(g_SocketDevice,(struct sockaddr *)&sockStruct,sizeof(sockStruct)) == SOCKET_ERROR)
	{
		printf("无法连接服务器\n");
		close(g_SocketDevice);
		return FALSE;
	}
	
	if(loopsend(g_SocketDevice,(char*)&g_DevInfo,sizeof(g_DevInfo)) == SOCKET_ERROR)
	{
		printf("login_p2p_server send msg failed\n");
		close(g_SocketDevice);
		return FALSE;
	}
	
	fd_set Ctrlset;
	FD_ZERO(&Ctrlset);
	FD_SET(g_SocketDevice, &Ctrlset);
	
	struct timeval timeout;
	timeout.tv_sec  = 5;//1;
	timeout.tv_usec = 0;
	int ret = select(FD_SETSIZE, &Ctrlset, NULL, NULL, &timeout);
	if(ret == 0)
	{
		printf("login_p2p_server select timeout\n");
		close(g_SocketDevice);
		return FALSE;
	}
	else if(ret == -1)
	{
		printf("login_p2p_server select error\n");
		close(g_SocketDevice);
		return FALSE;
	}
	
	if(g_SocketDevice != INVALID_SOCKET && FD_ISSET(g_SocketDevice, &Ctrlset))
	{
		ReDev_s g_ReDev;
		memset(&g_ReDev,0,sizeof(ReDev_s));
		
		if(looprecv(g_SocketDevice,(char*)&g_ReDev,sizeof(ReDev_s)) == SOCKET_ERROR)//recv(ServerSocket,buf,sizeof(buf),0);
		{
			close(g_SocketDevice);
			return FALSE;
		}
		
		printf("login_p2p_server returnVal=%d\n",g_ReDev.returnVal);
		
		if(g_ReDev.returnVal == 100)
		{
			close(g_SocketDevice);
			return TRUE;
		}
	}
	
	printf("login_p2p_server unknown error\n");
	
	close(g_SocketDevice);
	return FALSE;
}
static unsigned char p2p_param_chg_flag = 0;
//如果路由器的公网IP地址变了不能及时更新
//应该像DDNS一样检查公网IP地址
void *CPP2PProc(void *pParam)
{
	BOOL bLoginP2PServ = login_p2p_server();
	time_t last_login_time = time(NULL);
	
	while(1)
	{
		sleep(20);
		
		time_t curr = time(NULL);
		if(!bLoginP2PServ)
		{
			bLoginP2PServ = login_p2p_server();
			if(bLoginP2PServ)
			{
				last_login_time = curr;
			}
			
			p2p_param_chg_flag = 0;
		}
		else if(p2p_param_chg_flag)
		{
			bLoginP2PServ = login_p2p_server();
			if(bLoginP2PServ)
			{
				last_login_time = curr;
			}
			
			p2p_param_chg_flag = 0;
		}
		else
		{
			if(abs(curr-last_login_time) > 2*3600)
			{
				bLoginP2PServ = login_p2p_server();
				if(bLoginP2PServ)
				{
					last_login_time = curr;
				}
				
				p2p_param_chg_flag = 0;
			}
		}
	}
	
	return 0;
}

u32 GetLocalIp();

void *CPAckSearchProc(void *pParam)
{
#ifndef WIN32
	printf("$$$$$$$$$$$$$$$$$$CPAckSearchProc id:%d\n",getpid());
#endif
	
	int type = 0;
	memcpy(&type, pParam, sizeof(int));
	
#if 0//def JIUAN_2_UPNP
	char lan_productnum[16] = {0};
//	strcpy(lan_productnum, g_upnpid);
#endif
	
	if(type == CTRL_DEVICESEARCH_ACKSERVER)//接收搜索组播，然后回应
	{
		struct sockaddr_in ser;
		SOCKHANDLE serSocket;
		memset(&ser,0,sizeof(ser));
		ser.sin_family = AF_INET;
		ser.sin_port = htons(SEARCHPORT);
		ser.sin_addr.s_addr = INADDR_ANY;
		serSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if(serSocket == SOCKET_ERROR)
		{
			//printf("sersocket Error! %d\n",WSAGetLastError());
			SockClose(serSocket);
			return 0;
		}
		
		/************************************************************************/
		/*绑定socket                                                            */
		/************************************************************************/
		if(bind(serSocket,(struct sockaddr*)&ser,sizeof(ser)) == SOCKET_ERROR)
		{
			//printf("bind error! %d\n",WSAGetLastError());
			SockClose(serSocket);
			return 0;
		}
		
		/************************************************************************/
		/*加入组播                                                */
		/************************************************************************/	
		{
			struct ip_mreq mreq;
			memset(&mreq,0,sizeof(struct ip_mreq));
			mreq.imr_multiaddr.s_addr = inet_addr(MULTICASTGROUP);
			mreq.imr_interface.s_addr = INADDR_ANY;
			if(setsockopt(serSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char*)&mreq,sizeof(mreq)) == SOCKET_ERROR)
			{
				printf("setsockopt error!\n");
				SockClose(serSocket);
				return 0;
			}
		}
		
		while(1)
		{
			struct sockaddr_in cli;
			socklen_t len = sizeof(struct sockaddr_in);
			int ret = 0;
			char buf[256] = {0};
			memset(&cli,0,sizeof(cli));
			
			if((ret = (recvfrom(serSocket,buf,sizeof(buf),0,(struct sockaddr*)&cli,&len))) == SOCKET_ERROR)
			{
#ifdef WIN32
				printf("recvfrom error! %d \n",WSAGetLastError());
#else
				printf("recvfrom error! %d \n",errno);
#endif
			}
			else
			{
				ifly_ProtocolHead_t prohdr;
				u16 ackport = 0;
				
				if(ret < sizeof(ifly_ProtocolHead_t))
				{
					continue;
				}
				
				memcpy(&prohdr, buf, sizeof(ifly_ProtocolHead_t));//csp modify 20130405
				
				#if 0//def JIUAN_2_UPNP
				printf("LAN search1 %s,lan_productnum = %s\n",prohdr.safeguid,lan_productnum);
				ret = strncmp((char *)prohdr.safeguid,lan_productnum,sizeof(prohdr.safeguid));	
				if(ret != 0)
				{
					if(memcmp(prohdr.safeguid, gc_protocolGUID, 16) != 0)
					{
						printf("unknown search2 %s\n",prohdr.safeguid);
						continue;
					}
					else
					{
						printf("LAN search3 %s\n",prohdr.safeguid);
					}
				}
				else
				{
					printf("LAN search4 %s for getinfo\n",prohdr.safeguid);
				}
				#else
				if(memcmp(prohdr.safeguid, gc_protocolGUID, 16))
				{
					continue;
				}
				#endif
				
				//memcpy(&prohdr, buf, sizeof(ifly_ProtocolHead_t));//csp modify 20130405
				
				/*
				if(memcmp(prohdr.safeguid, gc_protocolGUID, 16))
				{
					continue;
				}
				*/
				
				if(prohdr.byConnType == 0x3)	//0x3：广播搜索
				{
					//解析端口
					memcpy(&ackport, prohdr.reserved, sizeof(u16));
					ackport = ntohs(ackport);
					printf("get ackport = %d\n",ackport);
					
					if(0 == ackport)//兼容当前固定端口
					{
						ackport = ACKSEARCHPORT;
					}
					
					struct sockaddr_in rem;
					memset(&rem,0,sizeof(rem));
					rem.sin_family = AF_INET;
					rem.sin_port = htons(ackport);
					rem.sin_addr.s_addr = cli.sin_addr.s_addr;
					
					SOCKHANDLE remSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
					if(remSocket == SOCKET_ERROR)
					{
						//printf("remsocket Error! %d\n",WSAGetLastError());
						//SockClose(remSocket);//csp modify 20130405
						continue;
					}
					
					#if 0//csp modify 20130405
					//u32 localip = GetLocalIp(NULL);
					u32 localip = GetLocalIp();
					if(localip != g_deviceInfo.deviceIP)
					{
						printf("update device ip from 0x:%08x to 0x:%08x",g_deviceInfo.deviceIP,localip);
						g_deviceInfo.deviceIP = localip;
					}
					#endif
					printf("%s nNVROrDecoder: %d\n", __func__, g_deviceInfo.nNVROrDecoder);
					if((sendto(remSocket,(char *)&g_deviceInfo,sizeof(g_deviceInfo),0,(struct sockaddr*)&rem,sizeof(rem))) == SOCKET_ERROR)
					{
						//printf("sendto error! %d\n",WSAGetLastError());
						SockClose(remSocket);
						continue;
					}
					
					//printf("send : %s\nto : %s\n",buf,inet_ntoa(rem.sin_addr));
					
					SockClose(remSocket);
				}
				else if(prohdr.byConnType == 0x4)	//0x4  轮巡同步(只对已经使能轮巡的设备有效)
				{
					printf("patrol synchronous\n");
					int ret;
					
					SModConfigPreviewParam sConfig;

					ret = ModConfigGetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);					
					if (ret == 0)
					{
						if (sConfig.nIsPatrol == 1) //设备已经使能轮巡
						{
							sConfig.nIsPatrol = 0;	//先关闭
							
							ret = ModConfigSetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
							if (ret == 0)
							{
								SPreviewPatrolPara sPatrolPara;
								sPatrolPara.emPreviewMode = sConfig.nPatrolMode;
								sPatrolPara.nInterval = sConfig.nInterval;
								sPatrolPara.nIsPatrol = sConfig.nIsPatrol;
								sPatrolPara.nStops = sConfig.nStops;
								memcpy(sPatrolPara.pnStopModePara, sConfig.pnStopModePara, sConfig.nStops);
								ModPreviewSetPatrol(&sPatrolPara);
							}

							usleep(200*1000);
							sConfig.nIsPatrol = 1;	//再开启
							
							ret = ModConfigSetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
							if (ret == 0)
							{
								SPreviewPatrolPara sPatrolPara;
								sPatrolPara.emPreviewMode = sConfig.nPatrolMode;
								sPatrolPara.nInterval = sConfig.nInterval;
								sPatrolPara.nIsPatrol = sConfig.nIsPatrol;
								sPatrolPara.nStops = sConfig.nStops;
								memcpy(sPatrolPara.pnStopModePara, sConfig.pnStopModePara, sConfig.nStops);
								ModPreviewSetPatrol(&sPatrolPara);
							}
						}
					}
				}
				else
				{
					//do nothing
				}
			}
		}
		
		SockClose(serSocket);
		return 0;
	} 
	else if(type == CTRL_DEVICESEARCH_ACKCLIENT)
	{
		struct sockaddr_in remaddr;
		struct sockaddr_in cli;
		SOCKHANDLE remSocket;
		
		char buf[256] = {0};
		int reuseflag = 1;
		int ret = 0;
		socklen_t len = sizeof(struct sockaddr_in);
		memset(&remaddr,0,sizeof(remaddr));
		remaddr.sin_family = AF_INET;
		remaddr.sin_port = htons(g_wCliAckPort);
		remaddr.sin_addr.s_addr = INADDR_ANY;
		
		remSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if(remSocket == INVALID_SOCKET)
		{
			//printf("WSASocket Error! %d \n",WSAGetLastError());
			SockClose(remSocket);
		}
		if(setsockopt(remSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&reuseflag,sizeof(reuseflag)) == SOCKET_ERROR) 
		{
			//printf("setsockopt error! %d \n",WSAGetLastError());
			SockClose(remSocket);
		}
		if(bind(remSocket,(struct sockaddr*)&remaddr,sizeof(remaddr)) == SOCKET_ERROR)
		{
			//printf("bind error! %d \n",WSAGetLastError());
			SockClose(remSocket);
		}
		
		SemGive(g_hFindSem);
		while(1)
		{
			ifly_DeviceInfo_t deviceinfo;
			memset(&cli,0,sizeof(cli));
			if((ret = (recvfrom(remSocket,buf,sizeof(buf),0,(struct sockaddr*)&cli,&len))) == SOCKET_ERROR)
			{
				//printf("recvfrom error! %d \n",WSAGetLastError());
				continue;
			}
			
			//printf("recv : %s,come on: %s\n",buf,inet_ntoa(cli.sin_addr));
			if(ret<sizeof(ifly_DeviceInfo_t))
			{
				continue;
			}
			memcpy(&deviceinfo, buf, sizeof(deviceinfo));
			deviceinfo.devicePort = ntohs(deviceinfo.devicePort);
			if(pFindCB)
			{
				pFindCB(deviceinfo, pFindCBContext);
			}

		}
		SockClose(remSocket);
	}
	else
	{
		return 0;
	}

	return 0;
}






u16 CPLibInit(u16 wPort)
{
	//InitData();
	int i;
#ifdef WIN32
	DWORD dwTaskID = 0;
	LPVOID lpMsgBuf;
#else
	void *temp;
#endif
	
	//printf("hehe1\n");

	if(bCPInit) return CTRL_SUCCESS;
	
	//printf("hehe2\n");

	//初始化套结字库
	{
#ifdef WIN32
		WSADATA wsaData;
		u32 err;
		
		err = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if(err != 0)
		{
			return FALSE;
		}
#endif
	}
	
	//printf("hehe3\n");
	
	CreateMutexHandle(&g_hSyncMutex);
	SemBCraete(&g_hSyncSem);
	SemTake(g_hSyncSem);
	
	SemBCraete(&g_hFindSem);
	if(wPort != 0)
	{
		struct sockaddr_in host;
		
		hServerSock = socket(AF_INET,CTRL_PROTOCOL_TYPE,0);
		if(hServerSock == INVALID_SOCKET)
		{
			return CTRL_FAILED_CREATESOCKET;
		}
		
		host.sin_family = AF_INET;
		host.sin_addr.s_addr = 0;
		host.sin_port = htons(wPort);
		if(bind(hServerSock,(struct sockaddr *)&host,sizeof(host)) == SOCKET_ERROR)
		{
#ifdef WIN32
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
				(LPTSTR) &lpMsgBuf, 0, NULL);	
			printf("CPLibInit:bind failed,errcode=%d,errmsg=%s\n",WSAGetLastError(),(LPCTSTR)lpMsgBuf);				
#else
			printf("CPLibInit:bind failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));			
#endif
			SockClose(hServerSock);
			return CTRL_FAILED_BIND;
		}
		if(listen(hServerSock,CTRL_PROTOCOL_MAXLINKNUM+STREAM_LINK_MAXNUM) == SOCKET_ERROR)
		{
#ifdef WIN32
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
				(LPTSTR) &lpMsgBuf, 0, NULL);	
			printf("CPLibInit:listen failed,errcode=%d,errmsg=%s\n",WSAGetLastError(),(LPCTSTR)lpMsgBuf);
#else
			printf("CPLibInit:listen failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
#endif
			SockClose(hServerSock);
			return CTRL_FAILED_LISTEN;
		}
	}
	
	for(i=0;i<CTRL_PROTOCOL_MAXLINKNUM;i++)
	{
		hCPLink[i].sockfd = INVALID_SOCKET;
		pthread_mutex_init(&hCPLink[i].lock, NULL);
		CleanCPHandle(&hCPLink[i]);
	}

	{
		struct sockaddr_in tLocalAddr;
		int ret;
#ifdef WIN32
		int iNameLen = sizeof(struct sockaddr_in);
#else
		socklen_t iNameLen = sizeof(struct sockaddr_in);
#endif
		
		hInterSock = socket(AF_INET, SOCK_DGRAM, 0);
		if(hInterSock == INVALID_SOCKET)
		{
			SockClose(hServerSock);
			return CTRL_FAILED_CREATESOCKET;
		}
		
		//绑定到任一端口
		tLocalAddr.sin_family		= AF_INET;
		tLocalAddr.sin_addr.s_addr	= 0;
		tLocalAddr.sin_port			= 0;
		ret = bind(hInterSock, (struct sockaddr *)&tLocalAddr, sizeof(tLocalAddr));
		if(ret == SOCKET_ERROR)
		{
			printf("CPLibInit:Internal socket bind failed\n");
			SockClose(hServerSock);
			SockClose(hInterSock);
			return CTRL_FAILED_BIND;
		}
		
		//获取绑定的端口号
		ret = getsockname(hInterSock,(struct sockaddr *)&tLocalAddr,&iNameLen);
		if(ret == SOCKET_ERROR)
		{
			printf("CPLibInit:Internal socket getsockname error!\n");
			SockClose(hServerSock);
			SockClose(hInterSock);
			return CTRL_FAILED_BIND;
		}
		wInterPort = ntohs(tLocalAddr.sin_port);
	}
	
	//printf("hehe4\n");
	
#ifdef P2P_SERVER//csp modify 20130406
	//SetUpnpId("00000001");
	//SetUpnpPW("2AV12OCRMZ");
	SetUpnpId("00000002");
	SetUpnpPW("OOVPSERMHM");
#endif
	
#ifdef WIN32
	hCPTask = CreateThread(NULL,STKSIZE_CP,(LPTHREAD_START_ROUTINE)CPTaskProc,NULL,0,&dwTaskID);
#else
	hCPTask = IFly_CreateThread(CPTaskProc,NULL,PRI_CP,STKSIZE_CP,0,0,NULL);
#endif
	if(hCPTask == (THREADHANDLE)NULL)
	{
#ifdef WIN32
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 0, NULL);	
		printf("CPLibInit:CreateThread failed,errcode=%d,errmsg=%s\n",GetLastError(),(LPCTSTR)lpMsgBuf);		
#else
		printf("CPLibInit:IFly_CreateThread failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
#endif
		SockClose(hInterSock);
		SockClose(hServerSock);
		return CTRL_FAILED_RESOURCE;
	}
	
	//printf("hehe5\n");
	
#if 0
#ifdef WIN32
	hCPCheckTask = CreateThread(NULL,STKSIZE_CPCHECK,(LPTHREAD_START_ROUTINE)CheckConnTaskProc,NULL,0,&dwTaskID);
#else
	hCPCheckTask = IFly_CreateThread(CheckConnTaskProc,NULL,PRI_CPCHECK,STKSIZE_CPCHECK,0,0,NULL);
#endif
	if(hCPCheckTask == (THREADHANDLE)NULL)
	{
#ifdef WIN32
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 0, NULL);	
		printf("CPLibInit:CreateThread failed,errcode=%d,errmsg=%s\n",GetLastError(),(LPCTSTR)lpMsgBuf);		
#else
		printf("CPLibInit:IFly_CreateThread failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
#endif
		SockClose(hInterSock);
		SockClose(hServerSock);
#ifdef WIN32
		TerminateThread(hCPTask,0);
#else
		pthread_cancel(hCPTask);
		pthread_join(hCPTask,&temp);
#endif
		return CTRL_FAILED_RESOURCE;
	}
#endif
	
	{
		static int acktype;
		if (wPort !=0)
		{
			acktype = CTRL_DEVICESEARCH_ACKSERVER;
		} 
		else
		{
			acktype = CTRL_DEVICESEARCH_ACKCLIENT;
		}
#ifdef WIN32
		hCPSearchTask = CreateThread(NULL,STKSIZE_CPCHECK,(LPTHREAD_START_ROUTINE)CPAckSearchProc,&acktype,0,&dwTaskID);
#else
		hCPSearchTask = IFly_CreateThread(CPAckSearchProc,NULL,PRI_CPCHECK,STKSIZE_CPCHECK,(u32)&acktype,0,NULL);
#endif
		if(hCPSearchTask == (THREADHANDLE)NULL)
		{
#ifdef WIN32
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
				(LPTSTR) &lpMsgBuf, 0, NULL);	
			printf("CPLibInit:CreateThread failed,errcode=%d,errmsg=%s\n",GetLastError(),(LPCTSTR)lpMsgBuf);		
#else
			printf("CPLibInit:IFly_CreateThread failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
#endif
			SockClose(hInterSock);
			SockClose(hServerSock);
#ifdef WIN32
			TerminateThread(hCPTask,0);
			TerminateThread(hCPCheckTask,0);
#else
			pthread_cancel(hCPTask);
			pthread_join(hCPTask,&temp);
			pthread_cancel(hCPCheckTask);
			pthread_join(hCPCheckTask,&temp);
#endif
			return CTRL_FAILED_RESOURCE;
		}
		
#ifdef P2P_SERVER//csp modify 20130406
#ifdef WIN32
		CreateThread(NULL,STKSIZE_CPCHECK,(LPTHREAD_START_ROUTINE)CPP2PProc,&acktype,0,&dwTaskID);
#else
		IFly_CreateThread(CPP2PProc,NULL,PRI_CPCHECK,STKSIZE_CPCHECK,(u32)&acktype,0,NULL);
#endif
#endif
	}
	
	bCPInit = TRUE;
	
	//printf("hehe6\n");
	
	return CTRL_SUCCESS;
}

u16 CPLibCleanup()
{
	int i;
#ifndef WIN32
	void *temp;
#endif
	
	if(bCPInit)
	{
#ifdef WIN32
		TerminateThread(hCPTask,0);
#else
		pthread_cancel(hCPTask);
		pthread_join(hCPTask,&temp);
#endif
#ifdef WIN32
		TerminateThread(hCPCheckTask,0);
#else
		pthread_cancel(hCPCheckTask);
		pthread_join(hCPCheckTask,&temp);
#endif
		
		SockClose(hServerSock);
		for(i=0;i<CTRL_PROTOCOL_MAXLINKNUM;i++)
		{
			if(hCPLink[i].sockfd != INVALID_SOCKET)
			{
				CleanCPHandle(&hCPLink[i]);
			}
		}
		
		{
#ifdef WIN32
			u32 err;
			err = WSACleanup();
			if(err != 0) return FALSE;	
#endif
		}

		CloseMutexHandle(g_hSyncMutex);
		SemDelete(g_hSyncSem);
		SemDelete(g_hFindSem);
		bCPInit = FALSE;
	}
	
	return CTRL_SUCCESS;
}

CPHandle CPConnect(u32 dwServerIp, u16 wServerPort, u32 dwTimeOut, u16 *pwErrorCode)
{
	CPHandle cph;
	u16 wErrCode;
	SOCKHANDLE sockfd = ConnectWithTimeout(dwServerIp,wServerPort,dwTimeOut,&wErrCode);
	if(sockfd == INVALID_SOCKET)
	{
		if(pwErrorCode) *pwErrorCode = wErrCode;
		return NULL;
	}
	SetSockLinkOpt(sockfd);

	SendTcpConnHead(sockfd, 0x1);

	cph = AddCPLink(sockfd,CTRL_CONNECTION_TCPCLIENT,dwServerIp,wServerPort,&wErrCode);
	if(pwErrorCode) *pwErrorCode = wErrCode;
	if(cph != NULL)
	{
		char ch = 0;
		struct sockaddr_in tDestAddr;
		int ret;
		tDestAddr.sin_family		= AF_INET;
		tDestAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
		tDestAddr.sin_port			= htons(wInterPort);
		//发送数据至内部套接字,使消息接收线程更新socket集合
		ret = sendto(hInterSock, &ch, sizeof(ch), 0, (struct sockaddr *)&tDestAddr, sizeof(tDestAddr));
	}
	return cph;
}

u16 CPPost(CPHandle cph, u16 event, const void *content, int length)
{
	//csp modify
	if(cph == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	u8 abyBuf[1024];
	ifly_cp_header_t cphead;
	cphead.length = (sizeof(cphead)+length);
	u32 tmp = cphead.length;
	cphead.length = htonl(cphead.length);
	cphead.type = htons(CTRL_NOTIFY);
	cphead.event = htons(event);
	cphead.number = htons(GetTransactionNum());
	cphead.version = htons(CTRL_VERSION);
	if(tmp > sizeof(abyBuf))
	{
		return CTRL_FAILED_PARAM;
	}
	memcpy(abyBuf,&cphead,sizeof(cphead));
	memcpy(abyBuf+sizeof(cphead),content,length);
	if(cph->sockfd == INVALID_SOCKET)
	{
		return CTRL_FAILED_CONNECT;
	}
	//pw 2010/9/10
	if(SOCKET_ERROR == loopsend(cph->sockfd,(char *)abyBuf,ntohl(cphead.length)))
	{
		return CTRL_FAILED_NETSND;
	}
	return CTRL_SUCCESS;
}

u16 CPSend(CPHandle cph, u16 event, const void *content, int length, void* ackbuf, int ackbuflen, int *realacklen, u32 dwTimeOut)
{
	u8 abyBuf[2048];
	ifly_cp_header_t cphead;
	if(cph == NULL)
	{
		return CTRL_FAILED_PARAM;
	}
	cphead.length = htonl(sizeof(cphead)+length);
	cphead.type = htons(CTRL_COMMAND);
	cphead.event = htons(event);
	cphead.number = htons(GetTransactionNum());
	cphead.version = htons(CTRL_VERSION);
	if(ntohl(cphead.length) > sizeof(abyBuf))
	{
		return CTRL_FAILED_PARAM;
	}
	memcpy(abyBuf,&cphead,sizeof(cphead));

	if (content && length)
	{
		memcpy(abyBuf+sizeof(cphead),content,length);
	}

	if(cph->sockfd == INVALID_SOCKET)
	{
		return CTRL_FAILED_CONNECT;
	}
	//同时只允许一个线程发送全局同步消息
	LockMutex(g_hSyncMutex);
	g_wSyncNumber = ntohs(cphead.number);
	SemTakeByTime(g_hSyncSem,0);//防止上一条消息的应答
	//pw 2010/9/10
	if(SOCKET_ERROR == loopsend(cph->sockfd,(char *)abyBuf,ntohl(cphead.length)))
	{
		g_wSyncNumber = 0;
		UnlockMutex(g_hSyncMutex);
		return CTRL_FAILED_NETSND;
	}
	if(!SemTakeByTime(g_hSyncSem,dwTimeOut))
	{
		g_wSyncNumber = 0;
		UnlockMutex(g_hSyncMutex);
		return CTRL_FAILED_TIMEOUT;
	}
	if(realacklen != NULL) 
	{
		*realacklen = g_dwSyncAckLen;
	}
	if(g_dwSyncAckLen > 0)
	{
		if(ackbuflen < g_dwSyncAckLen || ackbuf == NULL)
		{
			g_wSyncNumber = 0;
			UnlockMutex(g_hSyncMutex);
			return CTRL_FAILED_OUTOFMEMORY;
		}
		memcpy(ackbuf,g_bySyncAckBuf,g_dwSyncAckLen);
	}
	g_wSyncNumber = 0;
	UnlockMutex(g_hSyncMutex);
	return CTRL_SUCCESS;
}

u16 GetTransactionNum()
{
	static u16 wTransactionNum = 0;
	if( ++wTransactionNum >= 0x8000 )
	{
		wTransactionNum = 1;
	}
	return wTransactionNum;
}



u16 SendToHost(u32 dwHostIp, u32 dwTimeOut, void *pBuf, u32 dwBufLen)
{
	int ret;
	u16 wErrCode;
	u16 server_port = 0;
	ifly_cp_header_t cphead;
	SOCKHANDLE hClient = INVALID_SOCKET;
	
	//zlb20090918 begin
	FILE *fp = NULL;
	char str_port[16];

	memset(str_port, 0, sizeof(str_port));

	fp = fopen("/root/DVR_CTRL_PORT.txt", "rb");
	if (fp)
	{
		if (fread(str_port, 1, sizeof(str_port), fp) > 0)
		{
			server_port = atoi(str_port);
			printf("SendToHost DVR_CTRL_PORT = %u\n", server_port);	
		}
		else
		{
			printf("SendToHost DVR_CTRL_PORT = NULL\n");	
		}

		fclose(fp);
	}

	if (0 == server_port)
	{
		server_port = CTRL_PROTOCOL_SERVERPORT;
	}
	//zlb20090918 end

	printf("server_port=%d\n",server_port);
	//hClient = ConnectWithTimeout(dwHostIp, CTRL_PROTOCOL_SERVERPORT, dwTimeOut, &wErrCode);
	hClient = ConnectWithTimeout(dwHostIp, server_port, dwTimeOut, &wErrCode);
	if(hClient == INVALID_SOCKET) return wErrCode;

	ret = SendTcpConnHead(hClient, 0x1);
	if(ret<0)
	{
#ifdef WIN32
		printf("SendToHost:send failed,errcode=%d\n",WSAGetLastError());
		closesocket(hClient);
#else
		printf("SendToHost:send failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
		close(hClient);
#endif
		return CTRL_FAILED_LINKLIMIT;
	}
	ret = send(hClient,pBuf,dwBufLen,0);
	if(ret<0)
	{
#ifdef WIN32
		printf("SendToHost:send failed,errcode=%d\n",WSAGetLastError());
		closesocket(hClient);
#else
		printf("SendToHost:send failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
		close(hClient);
#endif
		return CTRL_FAILED_LINKLIMIT;
	}	
	ret = recv(hClient,(char *)&cphead,sizeof(ifly_cp_header_t),0);
	if(ret<0)
	{
#ifdef WIN32
		printf("SendToHost:recv failed,errcode=%d\n",WSAGetLastError());
		closesocket(hClient);
#else
		printf("SendToHost:recv failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
		close(hClient);
#endif
		return CTRL_FAILED_LINKLIMIT;
	}
	SockClose(hClient);
	//printf("SendToHost:result:(%d,%d,%d)\n",cphead.length,cphead.type,cphead.event);
	return cphead.event;
}

u16 SendToHost2(u32 dwHostIp, u32 dwTimeOut, void *pinBuf, u32 dwinBufLen, void *poutBuf, u32 *dwoutBufLen)
{
	int nRemain;
	int nRcvCount;
	int ret;
	u16 wErrCode;
	ifly_cp_header_t cphead;
	SOCKHANDLE hClient = INVALID_SOCKET;
	hClient = ConnectWithTimeout(dwHostIp, CTRL_PROTOCOL_SERVERPORT, dwTimeOut, &wErrCode);
	if(hClient == INVALID_SOCKET) return wErrCode;
	ret = send(hClient,pinBuf,dwinBufLen,0);
	if(ret<0)
	{
#ifdef WIN32
		printf("SendToHost2:send failed,errcode=%d\n",WSAGetLastError());
		closesocket(hClient);
#else
		printf("SendToHost2:send failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
		close(hClient);
#endif
		return CTRL_FAILED_LINKLIMIT;
	}
	nRemain = sizeof(ifly_cp_header_t);
	nRcvCount = 0;
	while(nRemain>0)
	{
		ret = recv(hClient,(char *)poutBuf+nRcvCount,nRemain,0);
		if(ret<0)
		{
#ifdef WIN32
			printf("SendToHost2:recv failed,errcode=%d\n",WSAGetLastError());
			closesocket(hClient);
#else
			printf("SendToHost2:recv failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
			close(hClient);
#endif
			return CTRL_FAILED_LINKLIMIT;
		}
		nRemain -= ret;
		nRcvCount += ret;
	}
	memcpy(&cphead,poutBuf,sizeof(ifly_cp_header_t));
	if(cphead.length > *dwoutBufLen)
	{
		SockClose(hClient);
		return CTRL_FAILED_PARAM;
	}
	*dwoutBufLen = cphead.length;
	nRemain = cphead.length - sizeof(ifly_cp_header_t);
	nRcvCount = 0;
	while(nRemain>0)
	{
		ret = recv(hClient,(char *)poutBuf+sizeof(ifly_cp_header_t)+nRcvCount,nRemain,0);
		if(ret<0)
		{
#ifdef WIN32
			printf("SendToHost2:recv failed,errcode=%d\n",WSAGetLastError());
			closesocket(hClient);
#else
			printf("SendToHost2:recv failed,errcode=%d,errmsg=%s\n",errno,strerror(errno));
			close(hClient);
#endif
			return CTRL_FAILED_LINKLIMIT;
		}
		nRemain -= ret;
		nRcvCount += ret;
	}
	SockClose(hClient);
	//printf("SendToHost2:result:(%d,%d,%d)\n",cphead.length,cphead.type,cphead.event);
	return cphead.event;
}

BOOL GetCPHandleLocalAddr(CPHandle cph, struct sockaddr_in *pAddr)
{
	struct sockaddr_in tSockAddr;
	socklen_t nlength = sizeof(tSockAddr);
	
	if(cph == NULL || cph->sockfd == INVALID_SOCKET || cph->conntype == CTRL_CONNECTION_NULL)
	{
		return FALSE;
	}
	
	if(SOCKET_ERROR == getsockname(cph->sockfd,(struct sockaddr *)&tSockAddr,&nlength))
	{
		printf("getsockname failed\n");
		return FALSE;
	}
	else
	{
		printf("getsockname success,local addr:(%s,%d)\n",inet_ntoa(tSockAddr.sin_addr),ntohs(tSockAddr.sin_port));
	}
	
//	char buf[256] = {0};
//	sprintf(buf, "ip= %s, port = %d, sin_family = %d",inet_ntoa(tSockAddr.sin_addr),ntohs(tSockAddr.sin_port), tSockAddr.sin_family);
//	MessageBox(NULL, buf, "tSockAddr.sin_addr", MB_OK);
	
	//MessageBox(NULL, inet_ntoa(tSockAddr.sin_addr), "tSockAddr.sin_addr", MB_OK);
	*pAddr = tSockAddr;
	
	return TRUE;
}






typedef struct ip_hdr
{
    u8  ip_verlen;        // IP version & length
    u8  ip_tos;           // IP type of service
    u16 ip_totallength;   // Total length
    u16 ip_id;            // Unique identifier 
    u16 ip_offset;        // Fragment offset field
    u8  ip_ttl;           // Time to live
    u8  ip_protocol;      // Protocol(TCP,UDP etc)
    u16 ip_checksum;      // IP checksum
    u32 ip_srcaddr;       // Source address
    u32 ip_destaddr;      // Destination address
}IP_HDR, *PIP_HDR;

typedef struct udp_hdr
{
    u16 src_portno;       // Source port number
    u16 dst_portno;       // Destination port number
    u16 udp_length;       // UDP packet length
    u16 udp_checksum;     // UDP checksum (optional)
}UDP_HDR, *PUDP_HDR;


SOCKHANDLE rawsocket_create(u32 dwProtocol)
{
	BOOL bOpt = TRUE;
	int nRet = 0;
	
	SOCKHANDLE hSocket = socket(AF_INET, SOCK_RAW, dwProtocol);
	if( hSocket == INVALID_SOCKET )
	{
		return INVALID_SOCKET;
	}
	
	// Enable the IP header include option
	nRet = setsockopt(hSocket, IPPROTO_IP, IP_HDRINCL, (char *)&bOpt, sizeof(bOpt));
	if (nRet == SOCKET_ERROR)
	{
		SockClose( hSocket );
		return INVALID_SOCKET;
	}
	
	return hSocket;
}

u16 rawsocket_checksum(u16 *pwbuffer, u32 size)
{
    unsigned long cksum=0;
	
    while (size > 1)
    {
        cksum += *pwbuffer++;
        size  -= sizeof(u16);   
    }
    if (size)
    {
        cksum += *(u8*)pwbuffer;   
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16); 
	
    return (u16)(~cksum); 
}

u32 rawsocket_sendto(SOCKHANDLE hSocket, u32 dwLocalIpAddr, u16 wLocalPort, u32 dwRemoteIpAddr, u16 wRemotePort, u8 *pbyBuf, u32 dwBufLen)
{
	IP_HDR       tipHdr;
    UDP_HDR      tudpHdr;
	char		 RawBuf[4096];
	struct sockaddr_in  tAddrIn;
	
	int iIPVersion = 4;	
	int iIPSize = sizeof(tipHdr) / sizeof(unsigned long);
	int iUdpSize = sizeof(tudpHdr) + dwBufLen;
	int iTotalSize = sizeof(tipHdr) + sizeof(tudpHdr) + dwBufLen;
	
	char *pRawBuf = (char *)&RawBuf;
    int  iUdpChecksumSize = 0;
	
	unsigned short cksum = 0;
	
	int nRet = 0;
	
	// Initalize the IP header
    //
    // IP version goes in the high order 4 bits of ip_verlen. The
    // IP header length (in 32-bit words) goes in the lower 4 bits.
    //
    tipHdr.ip_verlen = (iIPVersion << 4) | iIPSize;
    tipHdr.ip_tos = 0;                         // IP type of service
    tipHdr.ip_totallength = htons((u16)iTotalSize); // Total packet len
    tipHdr.ip_id = 0;                 // Unique identifier: set to 0
    tipHdr.ip_offset = 0;             // Fragment offset field
    tipHdr.ip_ttl = 128;              // Time to live
    tipHdr.ip_protocol = 0x11;        // Protocol(UDP) 
    tipHdr.ip_checksum = 0 ;          // IP checksum
    tipHdr.ip_srcaddr = dwLocalIpAddr;     // Source address ????
    tipHdr.ip_destaddr = dwRemoteIpAddr;      // Destination address
	
    // Initalize the UDP header
    //
    tudpHdr.src_portno = htons(wLocalPort) ;
    tudpHdr.dst_portno = htons(wRemotePort) ;
    tudpHdr.udp_length = htons((u16)iUdpSize) ;
    tudpHdr.udp_checksum = 0 ;
    // 
    // Build the UDP pseudo-header for calculating the UDP checksum.
    // The pseudo-header consists of the 32-bit source IP address, 
    // the 32-bit destination IP address, a zero byte, the 8-bit
    // IP protocol field, the 16-bit UDP length, and the UDP
    // header itself along with its data (padded with a 0 if
    // the data is odd length).
    //
	
	memset(pRawBuf, 0 , sizeof(RawBuf));
	
    memcpy(pRawBuf, &tipHdr.ip_srcaddr,  sizeof(tipHdr.ip_srcaddr));  
    pRawBuf += sizeof(tipHdr.ip_srcaddr);
    iUdpChecksumSize += sizeof(tipHdr.ip_srcaddr);
	
    memcpy(pRawBuf, &tipHdr.ip_destaddr, sizeof(tipHdr.ip_destaddr)); 
    pRawBuf += sizeof(tipHdr.ip_destaddr);
    iUdpChecksumSize += sizeof(tipHdr.ip_destaddr);
	
    pRawBuf++;
    iUdpChecksumSize += 1;
	
    memcpy(pRawBuf, &tipHdr.ip_protocol, sizeof(tipHdr.ip_protocol)); 
    pRawBuf += sizeof(tipHdr.ip_protocol);
    iUdpChecksumSize += sizeof(tipHdr.ip_protocol);
	
    memcpy(pRawBuf, &tudpHdr.udp_length, sizeof(tudpHdr.udp_length)); 
    pRawBuf += sizeof(tudpHdr.udp_length);
    iUdpChecksumSize += sizeof(tudpHdr.udp_length);
    
    memcpy(pRawBuf, &tudpHdr, sizeof(tudpHdr)); 
    pRawBuf += sizeof(tudpHdr);
    iUdpChecksumSize += sizeof(tudpHdr);
	
	if(pbyBuf || dwBufLen > 0)
	{
		memcpy(pRawBuf, pbyBuf, dwBufLen);
		iUdpChecksumSize += dwBufLen;
	}
	
	/*
    for(i = 0; i < strlen(strMessage); i++, ptr++)
        *ptr = strMessage[i];
    iUdpChecksumSize += strlen(strMessage);
	*/
	
    cksum = rawsocket_checksum((unsigned short *)&RawBuf, iUdpChecksumSize);
    tudpHdr.udp_checksum = cksum;
	
    //
    // Now assemble the IP and UDP headers along with the data
    //  so we can send it
    //    
    
    pRawBuf = (char *)&RawBuf;
	memset(pRawBuf, 0 , sizeof(RawBuf));
	
    memcpy(pRawBuf, &tipHdr, sizeof(tipHdr));   
	pRawBuf += sizeof(tipHdr);
    memcpy(pRawBuf, &tudpHdr, sizeof(tudpHdr)); 
	pRawBuf += sizeof(tudpHdr);
	if(pbyBuf || dwBufLen > 0)
	{
		memcpy(pRawBuf, pbyBuf, dwBufLen);
	}
	
    // Apparently, this SOCKADDR_IN structure makes no difference.
    // Whatever we put as the destination IP addr in the IP header
    // is what goes. Specifying a different destination in remote
	// will be ignored.
    //
	
	memset( &tAddrIn, 0, sizeof(tAddrIn));
	tAddrIn.sin_family	  = AF_INET; 
	tAddrIn.sin_addr.s_addr = dwRemoteIpAddr;
	tAddrIn.sin_port		  = htons(wRemotePort);
	
	nRet = sendto( hSocket, (char*)&RawBuf, iTotalSize, 0, (struct sockaddr *)&tAddrIn, sizeof(tAddrIn) );
	if(nRet < 0)
	{
		printf("raw socket:send failed\n");
	}
	else
	{
		printf("raw socket:send success,nRet=%d\n",nRet);
	}
    
	return nRet;
}

BOOL SendSTUNPacket(u32 dwLocalIpAddr, u16 wLocalPort, u32 dwRemoteIpAddr, u16 wRemotePort, u8 *pbyBuf, u32 dwBufLen)
{
	int nRet = 0;
	SOCKHANDLE s = rawsocket_create(IPPROTO_UDP);
	if(s == INVALID_SOCKET)
	{
		return FALSE;
	}
	nRet = rawsocket_sendto(s,dwLocalIpAddr,wLocalPort,dwRemoteIpAddr,wRemotePort,pbyBuf,dwBufLen);
	SockClose(s);
	return (nRet>0);
}

void SetDeviceInfo( ifly_DeviceInfo_t deviceinfo )
{
	memcpy(&g_deviceInfo, &deviceinfo, sizeof(ifly_DeviceInfo_t));
	
	#ifdef P2P_SERVER//csp modify 20130406
	p2p_param_chg_flag = 1;
	#endif
}

void SetFindDeviceCB( pFindDeivce_CB pCB, void *pContext )
{
	pFindCB = pCB;
	pFindCBContext = pContext;
}

int SearchDevice()
{
	SOCKHANDLE serSocket;
	struct sockaddr_in ser;
	int reuseflag = 1;

#ifndef WIN32
#else
	if (g_hFindSem != INVALID_HANDLE_VALUE)
	{
		SemTake(g_hFindSem);
		SemDelete(g_hFindSem);
		g_hFindSem = INVALID_HANDLE_VALUE;
	}
#endif


#ifndef WIN32
	serSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
#else
	serSocket = WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0, WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF|WSA_FLAG_OVERLAPPED);
#endif
	if (serSocket == INVALID_SOCKET)
	{
		return -1;
	}

	//端口复用

	if(setsockopt(serSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&reuseflag,sizeof(reuseflag)) == SOCKET_ERROR) 
	{
		//printf("port mulit error!\n");
		SockClose(serSocket);
		return -1;
	}
	
	memset(&ser,0,sizeof(ser));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(SEARCHPORT);
	ser.sin_addr.s_addr = inet_addr(MULTICASTGROUP);
	//加入组播
#ifndef WIN32
	struct ip_mreq mreq;
	memset(&mreq,0,sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICASTGROUP);
	mreq.imr_interface.s_addr = INADDR_ANY;
	if (setsockopt(serSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char*)&mreq,sizeof(mreq)) == SOCKET_ERROR)
	{
		//printf("setsockopt error 111   %d!\n", WSAGetLastError());
		SockClose(serSocket);
		return -1;
	}

#else
	if (WSAJoinLeaf(serSocket,(SOCKADDR*)&ser,sizeof(ser),NULL,NULL,NULL,NULL,JL_BOTH) == INVALID_SOCKET)
	{
		//printf("joinleaf error! %d \n",WSAGetLastError());
		SockClose(serSocket);

		return -1;
	}
#endif
	
	{
		//csp modify
		//int len = sizeof(struct sockaddr_in);
		//int ret = 0;
		
		u16 port = 0;
		ifly_ProtocolHead_t protocol;
		memset(&protocol, 0, sizeof(protocol));
		memcpy(protocol.safeguid, gc_protocolGUID, 16);
		protocol.protocolver = 1;
		protocol.byConnType = 0x3;
		port = htons(g_wCliAckPort);
		memcpy(protocol.reserved, &port, sizeof(u16));
		if((sendto(serSocket,(char*)&protocol,sizeof(protocol),0,(struct sockaddr*)&ser,sizeof(ser))) == SOCKET_ERROR)
		{
			//printf("sendto error! %d\n",WSAGetLastError());
			return -1;
		}
#ifndef WIN32
		usleep(1000*1000);
#else
		Sleep(1000);
#endif
	}
	
	SockClose(serSocket);
	return 0;
}

void SetAddStreamLinkCB( pAddStreamTCPLink pCB, void *pContext )
{
	pAddStreamLinkCB = pCB;
	pAddStreamLinkCBContext = pContext;
}

void SetCliAckPort( u16 wPort )
{
	g_wCliAckPort = wPort;
}

/*
 *	Description:check any client to connect to board
 *
 *	Input:	None
 *
 *	output:	TRUE or FALSE
 *
 *	Author:	panwei
 */
 //pw 2010/6/22
int check_link_stat()
{
		//pw 2010/6/22
		int z = 0;
		for(z = 0; z < CTRL_PROTOCOL_MAXLINKNUM; z++)
		{
			if(hCPLink[z].sockfd > 0)
			{
				//printf("client to board,z:%d socket:%d\n", z, hCPLink[z].sockfd);
				return TRUE;	
			}
		}
		return FALSE;
}

void SetUpnpId(char* upnpid)
{
	if(NULL != upnpid)
	{
		strcpy(g_upnpid,upnpid);
		
		#ifdef P2P_SERVER//csp modify 20130406
		p2p_param_chg_flag = 1;
		#endif
	}
}

//csp modify 20130406
void SetUpnpPW(char* upnppw)
{
	if(NULL != upnppw)
	{
		strcpy(g_upnppw,upnppw);
		
		#ifdef P2P_SERVER//csp modify 20130406
		p2p_param_chg_flag = 1;
		#endif
	}
}

