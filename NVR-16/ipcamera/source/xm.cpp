#include "xm.h"

#undef DEBUG
//#define DEBUG

#define err(fmt, args...)	\
do{							\
	printf("Error: function(%s): "fmt, __func__, ##args);	\
}while(0)

#define warning(fmt, args...)	\
do{							\
	printf("Warning: function(%s): "fmt, __func__, ##args);	\
}while(0)



#ifndef DEBUG

#define dbg(fmt, args...)

#else

//yaogang
#define dbg(fmt, args...)	\
do{							\
	printf("%s: "fmt, __func__, ##args);	\
}while(0)

#endif


#ifdef WIN32
#define NETSNDRCVFLAG	0
#else
#include <netinet/tcp.h>
#define NETSNDRCVFLAG	MSG_NOSIGNAL
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#endif

typedef unsigned short	u16;
typedef unsigned int	u32;

typedef struct
{
	unsigned char	Head_Flag;		//协议头标识, 1字节，固定为0xFF
	unsigned char	VERSION;		//版本号，当前为1
	unsigned char	RESERVED01;		//保留位
	unsigned char	RESERVED02;		//保留位
	//会话ID，每个CU与PU建立连接后，PU会给CU分配一个唯一的会话ID，
	//PU请求登录时，该字段添0，登录成功后PU在响应消息中填充该字段，
	//此后双方在消息头部，都要填充会话ID
	unsigned int	SessionID;	
	unsigned int	SEQUENCE_NUMBER;//包序号，启动后从0开始递增，到达最大后，在从0开始
	unsigned char	Total_Packet;	//总包数，为0和1时，表示只有一个数据包，大于1表示有多个数据包，每个数据包大小不超过16K
	unsigned char	CurPacket;	//当前包序号，只有在Total Packet大于1时才有意义，表示当前是第几个数据包，下标从0开始
	unsigned short	Message_Id;	//消息码
	unsigned int	Data_Length;	//数据区长度
	//char	*pData;		//数据区，最大不超过16K
} ctrl_head_t;

typedef struct
{
	//char	ip[16];
	u32 ipaddr;
	u16	port;
} net_info_t;

#define MAIN_STREAM		0
#define EXTRA_STREAM	1

typedef struct
{
	unsigned int	SessionID;
	int chn;
	int main_sock;
	int stream_sock;
	//int stream_type;
	unsigned int dwStreamWidth;
	unsigned int dwStreamHeight;
	RealStreamCB pStreamCB;
	unsigned int dwContext;
	pthread_t capture_tid;
	//pthread_t keepalive_tid;//公用保活线程

	int connected;//是否连接码流
	int eventLoopWatchVariable;//码流接收线程退出条件
	pthread_mutex_t lock;
} Session_info_t;

typedef struct
{
	net_info_t	tcp_info;
	Session_info_t session[2];//0: main_stream  1:extra_stream
} xm_camera_info_t;	

extern "C" int loopsend(int s, char *buf, unsigned int sndsize);
extern "C" int ConnectWithTimeout(u32 dwHostIp, u16 wHostPort, u32 dwTimeOut, u16 *pwErrorCode);

extern int IPC_Find(ipc_node* head, ipc_node *pNode);
extern int DoStreamStateCallBack(int chn, real_stream_state_e msg);

//static volatile unsigned char eventLoopWatchVariable[64] = {0};//32通道64路码流//csp modify 20150421

static xm_camera_info_t *g_xmcam_info = NULL;
static unsigned char g_init_flag = 0;
static int g_xm_client_count = 0;
static pthread_t keep_alive_pid = 0;
static unsigned char g_sdk_inited = 0;
static int alive_Interval = 0;
const char Iframe[] = {0, 0, 0x01, 0xfc};
const char Pframe[] = {0, 0, 0x01, 0xfd};
const char Aframe[] = {0, 0, 0x01, 0xfa};//音频帧


//static char *pbuf = NULL;//公用buffer

#undef BUF_SIZE
#define BUF_SIZE (2*1024)//(1*1024)

static int TimeZoneOffset[] = 
{
	-12*3600,
	-11*3600,
	-10*3600,
	-9*3600,
	-8*3600,
	-7*3600,
	-6*3600,
	-5*3600,
	-4*3600-1800,
	-4*3600,
	-3*3600-1800,
	-3*3600,
	-2*3600,
	-1*3600,
	0,
	1*3600,
	2*3600,
	3*3600,
	3*3600+1800,
	4*3600,
	4*3600+1800,
	5*3600,
	5*3600+1800,
	5*3600+2700,
	6*3600,
	6*3600+1800,
	7*3600,
	8*3600,
	9*3600,
	9*3600+1800,
	10*3600,
	11*3600,
	12*3600,
	13*3600,
};

static int GetTZOffset(int index)
{
	if(index < 0 || index >= (int)(sizeof(TimeZoneOffset)/sizeof(TimeZoneOffset[0])))
	{
		return 0;
	}
	
	return TimeZoneOffset[index];
}

static Session_info_t *get_session(int chn)
{
	int ipc_index, session_index;

	if (chn < 0 || chn > g_xm_client_count-1)
	{
		dbg("%s chn%d param invaild\n", __func__, chn);
		return NULL;;
	}
	
	if (chn < g_xm_client_count/2)
	{
		ipc_index = chn;
		session_index = MAIN_STREAM;
	}
	else
	{
		ipc_index = chn - g_xm_client_count/2;
		session_index = EXTRA_STREAM;
	}
	return (&g_xmcam_info[ipc_index].session[session_index]);
}

#if 0
static int recv_timeout(int sock, char *pbuf, int size, int chn)
{
	u32 remian = size;
	u32 recvlen = 0;
	int ret = 0;
	u32 timeout_cnt = 0;
	struct timeval struTimeout;
	fd_set struReadSet;
	
	unsigned int err_cnt = 0;
	
	while(remian > 0)
	{
		struTimeout.tv_sec = 0;//500ms
		struTimeout.tv_usec = 100*1000;
		
		FD_ZERO(&struReadSet);
		FD_SET(sock, &struReadSet);
		
		ret = select(sock + 1, &struReadSet, NULL, NULL, &struTimeout);
		if (ret < 0)
		{
			err("select failed");
			return FAILURE;
		}
		else if (ret == 0)
		{
			if (eventLoopWatchVariable[chn])//退出标志
			{
				dbg("%s quit because stream stop\n", __func__);
				return FAILURE;
			}
			
			if (++timeout_cnt > 30)//总时长timeout_cnt * struTimeout
			{
				dbg("%s timeout-1\n", __func__);
				return FAILURE;
			}
			
			continue;
		}
		
		timeout_cnt = 0;
		ret = recv(sock, pbuf+recvlen, remian, 0);
		if(ret < 0)
		{
			err("recv failed");
			return FAILURE;
		}
		else if (ret == 0)
		{
			if(err_cnt++ >= 400)
			{
				err("recv 0 byte");
				return FAILURE;
			}
		}
		else
		{
			err_cnt = 0;
		}
		
		recvlen += ret;
		remian -= ret;
	}
	return recvlen;
}
#endif

static int recv_timeout(int sock, char *pbuf, int size)
{
	u32 remian = size;
	u32 recvlen = 0;
	int ret = 0;
	u32 timeout_cnt = 0;
	struct timeval struTimeout;
	fd_set struReadSet;
	
	unsigned int err_cnt = 0;
	
	while(remian > 0)
	{
		struTimeout.tv_sec = 0;//500ms
		struTimeout.tv_usec = 500*1000;
		
		FD_ZERO(&struReadSet);
		FD_SET(sock, &struReadSet);
		
		ret = select(sock + 1, &struReadSet, NULL, NULL, &struTimeout);
		if (ret < 0)
		{
			err("select failed\n");
			return FAILURE;
		}
		else if (ret == 0)
		{
			if (++timeout_cnt > 4)//总时长timeout_cnt * struTimeout
			{
				dbg("%s timeout-2\n", __func__);
				return FAILURE;
			}
			continue;
		}
		
		timeout_cnt = 0;
		ret = recv(sock, pbuf+recvlen, remian,0);
		if(ret < 0)
		{
			err("recv failed\n");
			return FAILURE;
		}
		else if (ret == 0)
		{
			//if(err_cnt++ >= 400)
			{
				err("recv 0 byte\n");
				return FAILURE;
			}
		}
		else
		{
			err_cnt = 0;
		}
		
		recvlen += ret;
		remian -= ret;
	}
	return recvlen;
}

//pbuf: value-resule
static int cmd_session(int sock, char *pbuf, int size)
{
	ctrl_head_t *pchead = (ctrl_head_t *)pbuf;
	char *pmsg = pbuf + sizeof(ctrl_head_t);
	u32 SessionID = pchead->SessionID;
	u16 Message_Id = pchead->Message_Id;
	int ret;
	
	ret = loopsend(sock, pbuf, size);
	if (ret != size)
	{
		err("loopsend failed\n");
		return FAILURE;
	}
	
	memset(pchead, 0, sizeof(ctrl_head_t));
	ret = recv_timeout(sock, pbuf, sizeof(ctrl_head_t));
	if (ret != int(sizeof(ctrl_head_t)))
	{
		//dbg("recv head err\n");
		err("recv head err\n");
		return FAILURE;
	}
#if 0
	dbg("Head_Flag: 0x%x\n", pchead->Head_Flag);
    dbg("VERSION: %d\n", pchead->VERSION);
    dbg("SESSION_ID: %d\n", pchead->SessionID);
    dbg("SEQUENCE_NUMBER: %d\n", pchead->SEQUENCE_NUMBER);
    dbg("Total_Packet: %d\n", pchead->Total_Packet);
    dbg("CurPacket: %d\n", pchead->CurPacket);
    dbg("Message_Id: %d\n", pchead->Message_Id);
    dbg("Data_Length: %d\n", pchead->Data_Length);
#endif
	
	ret = recv_timeout(sock, pmsg, pchead->Data_Length);
	if (ret != int(pchead->Data_Length))
	{
		dbg("recv data err\n");
		return FAILURE;
	}
//检测返回值
	char *tmp = strstr(pmsg, "Ret");
	if ((tmp == NULL) \
		|| (sscanf(tmp+strlen("Ret")+4, "%d", &ret) != 1) \
		|| ret != 100)
	{
		dbg("respond data err, %s\n", pmsg);
		return FAILURE;
	}
//检测会话ID和消息ID
	if ((SessionID != pchead->SessionID) || (Message_Id != pchead->Message_Id-1))
	{
		dbg("respond data1 err\n");
		return FAILURE;
	}
	
	return SUCCESS;
}

int XM_Init(unsigned int max_client_num)
{
	//dbg("XM_Init");
	if(max_client_num <= 0)
	{
		return FAILURE;
	}
	
	if(g_init_flag)
	{
		return SUCCESS;
	}
	
	g_xm_client_count = (int)max_client_num;
	
	g_xmcam_info = (xm_camera_info_t *)malloc(g_xm_client_count / 2 * sizeof(xm_camera_info_t));
	if (g_xmcam_info == NULL)
	{
		err("malloc g_xmcam_info failed\n");
		return FAILURE;
	}
	memset(g_xmcam_info, 0, g_xm_client_count / 2 * sizeof(xm_camera_info_t));//csp modify 20150421
	
	//memset((void *)eventLoopWatchVariable, 0, sizeof(eventLoopWatchVariable));
	
	int i , j;
	for (i = 0; i < g_xm_client_count/2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			g_xmcam_info[i].session[j].capture_tid = -1;
			g_xmcam_info[i].session[j].main_sock = -1;
			g_xmcam_info[i].session[j].stream_sock = -1;
			pthread_mutex_init(&g_xmcam_info[i].session[j].lock, NULL);
		}
	}
	/*
	pbuf = (char *)malloc(COMMON_BUF_SIZE);
	if (NULL == pbuf)
	{
		err("malloc common buf failed\n");
		return FAILURE;
	}
	*/
	g_init_flag = 1;
	
	return SUCCESS;
}

int XM_DeInit(void)
{
	return 0;
}

int XM_Search(ipc_node** head, ipc_node **tail, unsigned char check_conflict)
{
	int ret;
	int sock;
	char buf[BUF_SIZE];
	unsigned char conflict_flag = 0;
	
	if(head == NULL || tail == NULL)
	{
		printf("XM_Search: param error\n");
		return -1;
	}
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		err("socket error\n");
		return FAILURE;
	}
	dbg("%s -- create socket ok fd: %d\n", __func__, sock);
	
	const int opt = 1;  
    //设置该套接字为广播类型 
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));  
    if(ret == FAILURE)  
    {
		err("set socket broadcast error\n");
		close(sock);//csp modify
		return FAILURE;
    }
    dbg("set socket broadcast ok\n");
    
    struct sockaddr_in addrto;  
    memset(&addrto, 0, sizeof(struct sockaddr_in));  
    addrto.sin_family = AF_INET;  
    addrto.sin_addr.s_addr = htonl(INADDR_ANY);
    addrto.sin_port = htons(BROADCAST_PORT);
    
    ret = bind(sock,(struct sockaddr *)&(addrto), sizeof(struct sockaddr_in));
    if (ret < 0)
    {
		err("bind error\n");
		close(sock);//csp modify
		return FAILURE; 
    }
    dbg("bind ok\n");
    
    ctrl_head_t *pchead = (ctrl_head_t *)buf;
    memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
    pchead->Message_Id = IPSEARCH_REQ;
    pchead->Data_Length = 0;
    addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);//广播地址
    
    ret = sendto(sock, pchead, sizeof(ctrl_head_t), 0, (struct sockaddr*)&addrto, sizeof(struct sockaddr_in));  
    if(ret < 0)  
    {  
		err("IPSEARCH_REQ sendto error\n");
		close(sock);//csp modify
		return FAILURE;
    }
    dbg("IPSEARCH_REQ send to ok\n");
    
    struct sockaddr_in from;
    socklen_t len;
    int count = 0;
	unsigned int cc = 0;
	char *pmsg = buf+sizeof(ctrl_head_t);
	char *ptmp = NULL;
	int i;
	
	for(i = 0; i < 100; i++)
	{
		fd_set r;
		struct timeval t;
		
		t.tv_sec = 0;
		t.tv_usec = 30*1000;//10*1000;//100;
		
		FD_ZERO(&r);
		FD_SET(sock, &r);
		
		ret = select(sock + 1, &r, NULL, NULL, &t);
		
		if(ret <= 0)
		{
			//if(cc++ >= 20)
			if(cc++ >= 40)
			{
				break;
			}
			continue;
		}
		
		cc = 0;
		
		if(ret > 0)
		{
			memset(&from, 0, sizeof(struct sockaddr_in));
			len = sizeof(struct sockaddr_in);
			memset(pchead, 0, sizeof(ctrl_head_t));
			ret = recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr*)&from, &len);  
			if(ret <= 0)
			{
				err("broadcast recvfrom error\n");
				close(sock);//csp modify
				return FAILURE;
			}
			dbg("broadcast respond from: %s, recvchars: %d\n", \
	    		inet_ntoa(from.sin_addr), ret);
	    	
	    	if (pchead->Message_Id != IPSEARCH_RSP)//同时过滤自己发送的广播包
	    	{
				continue;
			}
	    	/*
	    	ipaddr = GetLocalIp();
	    	if (0 == ipaddr)//failed
	    	{
	    		err("GetLocalIp error");
		        return FAILURE;
	    	}
			if (ipaddr == from.sin_addr.s_addr)//过滤自己发送的广播包
			{
				continue;
			}
			*/
			
			//if(1)
			{
				ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
				if(pNode == NULL)
				{
					err("Not enough space to save new ipc info\n");
					close(sock);//csp modify
					return count;//return -1;
				}
				
				memset(pNode, 0, sizeof(*pNode));
				pNode->next = NULL;
			//HostIP	
				ptmp = strstr(pmsg,"HostIP");
			    if (NULL == ptmp)
			    {
			    	err("strstr HostIP err\n");
					close(sock);//csp modify
			    	return FAILURE;
			    }
			    sscanf(ptmp+strlen("HostIP")+5, "0x%x", &pNode->ipcam.dwIp);
			    dbg("xmipc ipaddr: %s\n", inet_ntoa(*(struct in_addr *)&pNode->ipcam.dwIp));
			//GateWay    
			    ptmp = strstr(pmsg,"GateWay");
			    if (NULL == ptmp)
			    {
			    	err("strstr GateWay err\n");
					close(sock);//csp modify
			    	return FAILURE;
			    }
			    sscanf(ptmp+strlen("GateWay")+5, "0x%x", &pNode->ipcam.net_gateway);
			    dbg("xmipc GateWay: %s\n", inet_ntoa(*(struct in_addr *)&pNode->ipcam.net_gateway));
			//Submask    
			    ptmp = strstr(pmsg,"Submask");
			    if (NULL == ptmp)
			    {
			    	err("strstr Submask err\n");
					close(sock);//csp modify
			    	return FAILURE;
			    }
			    sscanf(ptmp+strlen("Submask")+5, "0x%x", &pNode->ipcam.net_mask);
			    dbg("xmipc Submask: %s\n", inet_ntoa(*(struct in_addr *)&pNode->ipcam.net_mask));
			//TCPPort    
			    ptmp = strstr(pmsg, "TCPPort");
			    if (NULL == ptmp)
			    {
			    	err("strstr TCPPort err\n");
					close(sock);//csp modify
			    	return FAILURE;
			    }
			    sscanf(ptmp+strlen("TCPPort")+4, "%hu", &pNode->ipcam.wPort);
			    dbg("xmipc tcp port: %d\n", pNode->ipcam.wPort);
			//MAC
			    ptmp = strstr(pmsg, "MAC");
			    if (NULL == ptmp)
			    {
			    	err("strstr MAC err\n");
					close(sock);//csp modify
			    	return FAILURE;
			    }
			    strncpy(pNode->ipcam.address, ptmp+strlen("MAC")+5, 18-1);
			    pNode->ipcam.address[17] = '\0';
			    dbg("mac: %s\n", pNode->ipcam.address);
			    strcpy(pNode->ipcam.uuid, pNode->ipcam.address);//MAC
				
				strcpy(pNode->ipcam.user, "admin");
				strcpy(pNode->ipcam.pwd, "tlJwpbo6");
				pNode->ipcam.channel_no = 0;
				pNode->ipcam.enable = 0;
				pNode->ipcam.ipc_type = IPC_TYPE_720P;
				pNode->ipcam.protocol_type = PRO_TYPE_XM;
				pNode->ipcam.stream_type = STREAM_TYPE_MAIN;
				pNode->ipcam.trans_type = TRANS_TYPE_TCP;
				pNode->ipcam.force_fps = 0;
				pNode->ipcam.frame_rate = 30;
				//pNode->ipcam.dns1
				//pNode->ipcam.dns2
				
				strcpy(pNode->ipcam.name, "XMIPC");				
				
				int rtn = IPC_Find(*head, pNode);
				//printf("xm IPC_Find ret: %d\n", rtn);
				if(rtn == 1)
				{
					printf("ipc conflict : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
					conflict_flag = 1;
				}
				else if(rtn == 2)
				{
					printf("ipc repeat : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
					free(pNode);
					pNode = NULL;
					continue;
				}
				
				count++;
				printf("xm-ipc%d : [%s,0x%08x,%d,%s]\n", count, pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
				
				if(*head == NULL)
				{
					(*head) = pNode;
					(*tail) = pNode;
				}
				else
				{
					(*tail)->next = pNode;
					(*tail) = pNode;
				}
			}
		}
	}
	close(sock);
	
	return count;
}

//ipc_login and ipc_capture 函数中的各个控制命令必须按顺序发送和接收，
//否则获取不到主码流
static int xm_login(int chn, unsigned int dwIp, unsigned short wPort, char *user, char *pwd)
{
	int main_sock, ret;
	char buf[BUF_SIZE];
	
	Session_info_t *psession = NULL;
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	
	//main_sock
	u16 wErrorCode = 0;
	main_sock = ConnectWithTimeout(dwIp, wPort, 5000, &wErrorCode);
	if(main_sock < 0)
	{
		dbg("chn%d xm_login main_sock connect (0x%08x:%d) failed\n", chn, dwIp, wPort);
		return FAILURE;
	}
	//dbg("%s -- create socket ok, fd: %d\n", __func__, main_sock);
	
//login
	char *pmsg = buf + sizeof(ctrl_head_t);
	//(None | 3DES | MD5)???
	sprintf(pmsg, "{ \"EncryptType\" : \"MD5\","
				" \"LoginType\" : \"DVRIP-Web\","
				" \"PassWord\" : \"tlJwpbo6\","
				" \"UserName\" : \"admin\" }\n");
	
	dbg("login msg: %s\n", pmsg);
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
    memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
    pchead->Message_Id = LOGIN_REQ;
    pchead->Data_Length = strlen(pmsg);// + 1;
    
    ret = loopsend(main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if(ret != (int)(sizeof(ctrl_head_t)+pchead->Data_Length))
	{
		err("send_data err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("send login data ok\n");
	
	memset(pchead, 0, sizeof(ctrl_head_t));
	ret = recv_timeout(main_sock, buf, sizeof(ctrl_head_t));
	if (ret != int(sizeof(ctrl_head_t)))
	{
		err("recv login respond head err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	/*
	dbg("Head_Flag: 0x%x\n", pchead->Head_Flag);
    dbg("VERSION: %d\n", pchead->VERSION);
    dbg("SESSION_ID: %d\n", pchead->SessionID);
    dbg("SEQUENCE_NUMBER: %d\n", pchead->SEQUENCE_NUMBER);
    dbg("Total_Packet: %d\n", pchead->Total_Packet);
    dbg("CurPacket: %d\n", pchead->CurPacket);
    dbg("Message_Id: %d\n", pchead->Message_Id);
    dbg("Data_Length: %d\n", pchead->Data_Length);
	*/
	ret = recv_timeout(main_sock, pmsg, pchead->Data_Length);
	if (ret != int(pchead->Data_Length))
	{
		err("recv login respond data err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("respon data: %s\n", pmsg);
	
//AliveInterval
	char *p = NULL;
	
	if (alive_Interval == 0)
	{
	    p = strstr(buf+sizeof(ctrl_head_t), "AliveInterval");
	    if (NULL == p)
	    {
	    	err("strstr AliveInterval err\n");
			close(main_sock);//csp modify
	    	return FAILURE;
	    }
	    ret = sscanf(p+strlen("AliveInterval")+4, "%d", &alive_Interval);
	    dbg("ret = %d\n", ret);
	    dbg("alive_Interval: %d\n", alive_Interval);
	}
    /*
    //channelnum
    int channel;
    p = strstr(buf+sizeof(ctrl_head_t), "ChannelNum");
    if (NULL == p)
    {
    	err("strstr ChannelNum err");
    	close(main_sock);//csp modify
    	return FAILURE;
    }
    ret = sscanf(p+strlen("ChannelNum")+4, "%d", &channel);
    dbg("ret = %d\n", ret);
    psession->channel = (unsigned char)channel;
    dbg("ChannelNum: %d\n", psession->channel);
    */
//session id
	int SessionID;
    p = strstr(buf+sizeof(ctrl_head_t), "SessionID");
    if (NULL == p)
    {
    	err("strstr SessionID err\n");
		close(main_sock);//csp modify
    	return FAILURE;
    }
    ret = sscanf(p+strlen("SessionID")+5, "0x%x", &SessionID);
    dbg("ret = %d\n", ret);
    dbg("SessionID: %d\n", SessionID);
	
//get systeminfo
	sprintf(pmsg, "{ \"Name\" : \"SystemInfo\","
		" \"SessionID\" : \"0x%08x\" }\n", SessionID);
	dbg("get SystemInfo: %s\n", pmsg);
	
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = SessionID;
    pchead->Message_Id = SYSINFO_REQ;
    pchead->Data_Length = strlen(pmsg);// + 1;
    
    ret = loopsend(main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if(ret != int(sizeof(ctrl_head_t)+pchead->Data_Length))
	{
		dbg("send SYSINFO_REQ head err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("send SYSINFO_REQ head ok\n");
	
	memset(pchead, 0, sizeof(ctrl_head_t));
	ret = recv_timeout(main_sock, buf, sizeof(ctrl_head_t));
	if (ret != int(sizeof(ctrl_head_t)))
	{
		dbg("recv SYSINFO_REQ respond head err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("Head_Flag: 0x%x\n", pchead->Head_Flag);
    dbg("VERSION: %d\n", pchead->VERSION);
    dbg("SESSION_ID: %d\n", pchead->SessionID);
    dbg("SEQUENCE_NUMBER: %d\n", pchead->SEQUENCE_NUMBER);
    dbg("Total_Packet: %d\n", pchead->Total_Packet);
    dbg("CurPacket: %d\n", pchead->CurPacket);
    dbg("Message_Id: %d\n", pchead->Message_Id);
    dbg("Data_Length: %d\n", pchead->Data_Length);
    
	ret = recv_timeout(main_sock, pmsg, pchead->Data_Length);
	if (ret != int(pchead->Data_Length))
	{
		err("recv SYSINFO_REQ respond msg err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("SYSINFO_REQ: respond msg: %s\n", pmsg);
	
//get VideoEnc info
	sprintf(pmsg, "{ \"Name\" : \"Simplify.Encode\","
		" \"SessionID\" : \"0x%08x\" }\n", SessionID);
	dbg("get VideoEnc: %s\n", pmsg);
	
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = SessionID;
    pchead->Message_Id = CONFIG_GET;//0x410
    pchead->Data_Length = strlen(pmsg);// + 1;
    
    ret = loopsend(main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if(ret != int(sizeof(ctrl_head_t)+pchead->Data_Length))
	{
		dbg("send CONFIG_GET head err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("send CONFIG_GET head ok\n");
	
	memset(pchead, 0, sizeof(ctrl_head_t));
	ret = recv_timeout(main_sock, buf, sizeof(ctrl_head_t));
	if (ret != int(sizeof(ctrl_head_t)))
	{
		dbg("recv CONFIG_GET respond head err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("Head_Flag: 0x%x\n", pchead->Head_Flag);
    dbg("VERSION: %d\n", pchead->VERSION);
    dbg("SESSION_ID: %d\n", pchead->SessionID);
    dbg("SEQUENCE_NUMBER: %d\n", pchead->SEQUENCE_NUMBER);
    dbg("Total_Packet: %d\n", pchead->Total_Packet);
    dbg("CurPacket: %d\n", pchead->CurPacket);
    dbg("Message_Id: %d\n", pchead->Message_Id);
    dbg("Data_Length: %d\n", pchead->Data_Length);
    
	ret = recv_timeout(main_sock, pmsg, pchead->Data_Length);
	if (ret != int(pchead->Data_Length))
	{
		err("recv SYSINFO_REQ respond data err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("CONFIG_GET respon msg: %s\n", pmsg);
//视频宽高//1080P怎么办???
#define WIDTH_720P	1280//1080//csp modify
#define HEIGHT_720P 720
#define WIDTH_CIF	352
#define HEIGHT_CIF	288
#define WIDTH_D1	704
#define HEIGHT_D1	576
	int main_width, main_height;
	int extra_width, extra_height;
	
	main_width = main_height = 0;
	extra_width = extra_height = 0;
	
	p = strstr(pmsg, "ExtraFormat");
	p = strstr(p, "Resolution");
	if (strncmp(p+strlen("Resolution")+5, "CIF", strlen("CIF")) == 0)
	{
		extra_width = WIDTH_CIF;
		extra_height = HEIGHT_CIF;
	}
	else if (strncmp(p+strlen("Resolution")+5, "D1", strlen("D1")) == 0)
	{
		extra_width = WIDTH_D1;
		extra_height = HEIGHT_D1;
	}
	else
	{
		dbg("extra stream Resolution undown\n");
		
		extra_width = WIDTH_CIF;
		extra_height = HEIGHT_CIF;
	}
	
	p = strstr(pmsg, "MainFormat");
	p = strstr(p, "Resolution");
	if (strncmp(p+strlen("Resolution")+5, "720P", strlen("720P")) == 0)
	{
		main_width = WIDTH_720P;
		main_height = HEIGHT_720P;
	}
	else if (strncmp(p+strlen("Resolution")+5, "1080P", strlen("1080P")) == 0)//csp modify
	{
		main_width = 1920;
		main_height = 1080;
	}
	else if (strncmp(p+strlen("Resolution")+5, "960P", strlen("960P")) == 0)//csp modify
	{
		main_width = 1280;
		main_height = 960;
	}
	else if (strncmp(p+strlen("Resolution")+5, "D1", strlen("D1")) == 0)//csp modify
	{
		main_width = WIDTH_D1;
		main_height = HEIGHT_D1;
	}
	else
	{
		dbg("main stream Resolution undown\n");
		
		main_width = WIDTH_720P;
		main_height = HEIGHT_720P;
	}
	
//create stream socket
	int stream_sock;
	stream_sock = ConnectWithTimeout(dwIp, wPort, 5000, &wErrorCode);
	if(stream_sock < 0)
	{
		printf("chn%d xm_login stream_sock connect (0x%08x:%d) failed\n", chn, dwIp, wPort);
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("stream_sock connect success\n");
	
//赋值
	pthread_mutex_lock(&psession->lock);
	psession->main_sock = main_sock;
	psession->stream_sock = stream_sock;
	psession->SessionID = SessionID;
	if (chn < g_xm_client_count/2)
	{
		psession->dwStreamWidth = main_width;
		psession->dwStreamHeight = main_height;
	}
	else
	{
		psession->dwStreamWidth = extra_width;
		psession->dwStreamHeight = extra_height;
	}
	pthread_mutex_unlock(&psession->lock);
	
	return SUCCESS;
}
//外部加锁
static int xm_logout(int chn)
{
	char buf[BUF_SIZE];
	Session_info_t *psession = NULL;
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	int ret = 0;
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	
	if(psession->main_sock != -1)
	{
		sprintf(pmsg, "{ \"SessionID\" : \"0x%x\" }\n", pchead->SessionID);
		memset(pchead, 0, sizeof(ctrl_head_t));
	    pchead->Head_Flag = 0xff;
		pchead->VERSION = VER;
	    pchead->Message_Id = LOGOUT_REQ;
	    pchead->Data_Length = strlen(pmsg);
	    pchead->SessionID = psession->SessionID;
		ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
		if (ret != SUCCESS)
		{
			//pthread_mutex_unlock(&psession->lock);//csp modify
			dbg("%s LOGOUT_REQ failed\n", __func__);
			return FAILURE;//csp modify
		}
	}

	return SUCCESS;
}

static int xm_capture_start(int chn)
{
	int ret;
	char buf[BUF_SIZE];
	Session_info_t *psession = NULL;
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
//gettime
	time_t t = time(NULL);	
	int nTimeZone = IPC_GetTimeZone();
	t += GetTZOffset(nTimeZone);
	
	struct tm now;
	localtime_r(&t,&now);
	
//settime \"2014-06-26 20:32:10\"
	sprintf(pmsg, "{ \"Name\" : \"OPTimeSettingNoRTC\","
		" \"OPTimeSettingNoRTC\" : \"%d-%d-%d %d:%d:%d\","
		" \"SessionID\" : \"0x%x\" }\n", \
		now.tm_year+1900, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, \
		psession->SessionID);
	dbg("TimeSetting: %s\n", pmsg);
	
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
    pchead->SessionID = psession->SessionID;
    pchead->SEQUENCE_NUMBER = 1;
    pchead->Message_Id = TIME_SET_REQ;//time setting 0x636
    pchead->Data_Length = strlen(pmsg);// + 1;
    
    pthread_mutex_lock(&psession->lock);
    ret = loopsend(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if(ret != int(sizeof(ctrl_head_t)+pchead->Data_Length))
	{
		pthread_mutex_unlock(&psession->lock);
		dbg("send time setting head err\n");
		return FAILURE;
	}
	pthread_mutex_unlock(&psession->lock);
	dbg("send time setting head ok\n");
	
	/*
	//暂时不收
	memset(pchead, 0, sizeof(ctrl_head_t));
	ret = recv_data(psession->main_sock, pbuf, sizeof(ctrl_head_t));
	if (ret < 0)
	{
		err("recv time setting:stop respond head err");
		return FAILURE;
	}	
	dbg("Head_Flag: 0x%x\n", pchead->Head_Flag);
    dbg("VERSION: %d\n", pchead->VERSION);
    dbg("SESSION_ID: %d\n", pchead->SessionID);
    dbg("SEQUENCE_NUMBER: %d\n", pchead->SEQUENCE_NUMBER);
    dbg("Total_Packet: %d\n", pchead->Total_Packet);
    dbg("CurPacket: %d\n", pchead->CurPacket);
    dbg("Message_Id: %d\n", pchead->Message_Id);
    dbg("Data_Length: %d\n", pchead->Data_Length);
    
	ret = recv_data(psession->main_sock, pmsg, pchead->Data_Length);
	if (ret < 0)
	{
		err("recv time setting respond data err");
		return FAILURE;
	}
	dbg("time setting respon data: %s\n", pmsg);
	*/
	
//stream_sock MONITOR_CLAIM
	sprintf(pmsg, "{ \"Name\" : \"OPMonitor\","
				" \"OPMonitor\" : {"
				" \"Action\" : \"Claim\","
				" \"Parameter\" : {"
				" \"Channel\" : 0,"
				" \"CombinMode\" : \"NONE\","
				" \"StreamType\" : \"%s\","
				" \"TransMode\" : \"TCP\" } }," 
				" \"SessionID\" : \"0x%x\" }\n", \
				chn < g_xm_client_count/2? "Main":"Extra1", psession->SessionID);
	dbg("stream msg: %s\n", pmsg);
	
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
    pchead->Message_Id = MONITOR_CLAIM;
    pchead->Data_Length = strlen(pmsg);// + 1;
    pchead->SessionID = psession->SessionID;
    
    ret = cmd_session(psession->stream_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if(ret != SUCCESS)
	{
		dbg("stream_sock send MONITOR_CLAIM data err\n");
		return FAILURE;
	}
	dbg("stream_sock send MONITOR_CLAIM data ok\n");
/*
    ret = send_data(psession->stream_sock, pbuf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if(ret < 0)
	{
		err("send MONITOR_CLAIM data err");
		return FAILURE;
	}
	dbg("send MONITOR_CLAIM data ok\n");
	
	memset(pchead, 0, sizeof(ctrl_head_t));
	ret = recv_data(psession->stream_sock, pbuf, sizeof(ctrl_head_t));
	if (ret < 0)
	{
		err("recv MONITOR_CLAIM respond head err");
		return FAILURE;
	}
	dbg("Head_Flag: 0x%x\n", pchead->Head_Flag);
    dbg("VERSION: %d\n", pchead->VERSION);
    dbg("SESSION_ID: %d\n", pchead->SessionID);
    dbg("SEQUENCE_NUMBER: %d\n", pchead->SEQUENCE_NUMBER);
    dbg("Total_Packet: %d\n", pchead->Total_Packet);
    dbg("CurPacket: %d\n", pchead->CurPacket);
    dbg("Message_Id: %d\n", pchead->Message_Id);
    dbg("Data_Length: %d\n", pchead->Data_Length);
    
	ret = recv_data(psession->stream_sock, pmsg, pchead->Data_Length);
	if (ret < 0)
	{
		err("recv MONITOR_CLAIM respond data err");
		return FAILURE;
	}
	dbg("MONITOR_CLAIM respon data: %s\n", pmsg);
*/
//main sock MONITOR_REQ
	sprintf(pmsg, "{ \"Name\" : \"OPMonitor\","
				" \"OPMonitor\" : {"
				" \"Action\" : \"Start\","
				" \"Parameter\" : {"
				" \"Channel\" : 0,"
				" \"CombinMode\" : \"NONE\","
				" \"StreamType\" : \"%s\","
				" \"TransMode\" : \"TCP\" } }," 
				" \"SessionID\" : \"0x%x\" }\n", \
				chn < g_xm_client_count/2? "Main":"Extra1", psession->SessionID);
	dbg("main msg: %s\n", pmsg);
	
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
    pchead->Message_Id = MONITOR_REQ;
    pchead->Data_Length = strlen(pmsg);// + 1;
    pchead->SessionID = psession->SessionID;
    
    pthread_mutex_lock(&psession->lock);
    ret = loopsend(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if(ret < 0)
	{
		pthread_mutex_unlock(&psession->lock);
		dbg("main_sock send MONITOR_REQ data err\n");
		return FAILURE;
	}
	dbg("main_sock send MONITOR_REQ data ok\n");
	
	int i;
	for (i=0; i<2; i++)
	{
		memset(pchead, 0, sizeof(ctrl_head_t));
		ret = recv_timeout(psession->main_sock, buf, sizeof(ctrl_head_t));
		if (ret != int(sizeof(ctrl_head_t)))
		{
			pthread_mutex_unlock(&psession->lock);
			err("recv MONITOR_REQ respond head err\n");
			return FAILURE;
		}
		dbg("Head_Flag: 0x%x\n", pchead->Head_Flag);
	    dbg("VERSION: %d\n", pchead->VERSION);
	    dbg("SESSION_ID: %d\n", pchead->SessionID);
	    dbg("SEQUENCE_NUMBER: %d\n", pchead->SEQUENCE_NUMBER);
	    dbg("Total_Packet: %d\n", pchead->Total_Packet);
	    dbg("CurPacket: %d\n", pchead->CurPacket);
	    dbg("Message_Id: %d\n", pchead->Message_Id);
	    dbg("Data_Length: %d\n", pchead->Data_Length);
	    
		ret = recv_timeout(psession->main_sock, pmsg, pchead->Data_Length);
		if (ret != int(pchead->Data_Length))
		{
			pthread_mutex_unlock(&psession->lock);
			err("recv MONITOR_REQ respond data err\n");
			return FAILURE;
		}
		dbg("MONITOR_REQ respon data: %s\n", pmsg);
	}
	pthread_mutex_unlock(&psession->lock);
	
	return SUCCESS;
}

//外部加锁
static int xm_capture_stop(int chn)
{
	char buf[BUF_SIZE];
	Session_info_t *psession = NULL;
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	int ret = SUCCESS;
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	
	if (psession->stream_sock != -1)
	{
	//关闭流传输
		sprintf(pmsg, "{ \"Name\" : \"OPMonitor\","
				" \"OPMonitor\" : {"
				" \"Action\" : \"Stop\","
				" \"Parameter\" : {"
				" \"Channel\" : 0,"
				" \"CombinMode\" : \"NONE\","
				" \"StreamType\" : \"%s\","
				" \"TransMode\" : \"TCP\" } }," 
				" \"SessionID\" : \"0x%x\" }\n", \
				chn < g_xm_client_count/2? "Main":"Extra1", psession->SessionID);
		
		dbg("chn%d stop stream: %s\n", chn, pmsg);
		
		memset(pchead, 0, sizeof(ctrl_head_t));
	    pchead->Head_Flag = 0xff;
		pchead->VERSION = VER;
	    pchead->Message_Id = MONITOR_REQ;
	    pchead->Data_Length = strlen(pmsg);
	    pchead->SessionID = psession->SessionID;
	    
	    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
	    if (ret !=  SUCCESS)
		{
			dbg("%s MONITOR_REQ:stop failed\n", __func__);
		}
		else
		{
			dbg("send MONITOR_REQ:stop head ok\n");
		}
	}

	return ret;
}

//return value : 1 - Link; 0 - Lost
int XM_GetLinkStatus(int chn)
{
	int status = 0;
	Session_info_t *psession = NULL;
	
	if(!g_init_flag)
	{
		return 0;
	}
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return 0;
	}
	
	pthread_mutex_lock(&psession->lock);
	
	status = psession->connected;
	
	pthread_mutex_unlock(&psession->lock);
	
	return status;
}

#define TIMEOUT_CNT 3//2//csp modify
static void* Thread_keep_alive(void* pParam)
{
	int chn;
	int ret;
	char buf[BUF_SIZE];
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	Session_info_t *psession = NULL;
	
	unsigned int timeout[64] = {0};//bug//32通道64路码流//csp modify 20150421
	
	ret = pthread_detach(pthread_self());
	if(ret < 0)
	{
		err("pthread_detach\n");
		pthread_exit((void *)0);
	}
	else
	{
		printf("Thread_keep_alive detach ok\n");
	}
	
	memset(timeout, 0, sizeof(timeout));
	
	while(g_init_flag)
	{
		//printf("Thread_keep_alive: loop\n");
		for(chn = 0; chn < g_xm_client_count; chn++)
		{
			psession = get_session(chn);
			
			pthread_mutex_lock(&psession->lock);
			if (psession->main_sock == -1)
			{
				pthread_mutex_unlock(&psession->lock);
				//dbg("chn%d main_sock invalid\n", chn);
				continue;
			}
			
			sprintf(pmsg, "{ \"Name\" : \"KeepAlive\","
					" \"SessionID\" : \"0x%08x\" }\n", psession->SessionID);
			//dbg("keepalive msg: %s\n", pmsg);
			memset(pchead, 0, sizeof(ctrl_head_t));
			pchead->Head_Flag = 0xff;
			pchead->VERSION = VER;
			pchead->SessionID = psession->SessionID;
		    pchead->Message_Id = KEEPALIVE_REQ;	    
		    pchead->Data_Length = strlen(pmsg);
		    
		    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
		    pthread_mutex_unlock(&psession->lock);
		    
		    if (ret !=  SUCCESS)
			{
				dbg("chn%d - send keepalive err\n", chn);
				if (++timeout[chn] > TIMEOUT_CNT)
				{
					dbg("chn%d - send keepalive err and stop\n", chn);
					XM_Stop(chn);
					
					timeout[chn] = 0;//csp modify
				}
			}
			else
			{
				//dbg("chn%d - send keepalive ok\n", chn);
				timeout[chn] = 0;
			}
		}
		
		//csp modify
		#if 1
		int delay = alive_Interval;
		if(delay < 3)
		{
			delay = 1;
		}
		else
		{
			delay = delay/3;
			if(delay > 60)
			{
				delay = 60;
			}
		}
		sleep(delay);
		#else
		int delay = alive_Interval;
		if(delay < 3)
		{
			delay = 1;
		}
		else
		{
			delay = (delay-1)/2;
			if(delay > 60)
			{
				delay = 60;
			}
		}
		sleep(delay);
		#endif
	}
	
	pthread_exit((void *)SUCCESS);
}

#if 1
/* 
在IPC的位置描述:
音/视频数据以包为单位依次传输
注意:
	1、一帧数据分多个包
	2、I帧和P帧并没有分开，在一个包中可能包含I帧(结束部分)和P帧(起始部分)；
	3、一个包中也可能包含几个P帧
	4、雄迈协议已经将sps pps sei 组合I帧
*/
void* Thread_capture(void* pParam)
{
	Session_info_t *psession = (Session_info_t *)pParam;
	int chn = psession->chn;
	
	int ret = 0;
	char *pbuf = NULL;
	char *pbuf1 = NULL;
	unsigned int enc_len = 0;
	
	ctrl_head_t *pchead = NULL;//(ctrl_head_t *)pbuf;
	char *pdata = NULL;//pbuf + sizeof(ctrl_head_t);
	//real_frame_type_e frame_type = REAL_FRAME_TYPE_NONE;
	unsigned int discern_frame_type = 0;//识别帧类型。0 未识别帧；1 视频I帧；2 视频P帧；3 音频帧
	unsigned int frame_len = 0;	//一帧数据长度
	unsigned short frame_len_s = 0;//音频帧头中长度占两字节
	unsigned int frame_pos = 0;	//一帧中已经收到的数据长度
	unsigned int b_ignore_one_frame = 0;//当帧长度> enc_len 时，忽略

	//一包数据中当前处理位置，(后面)已经处理的数据，(前面)待处理的数据
	unsigned int pkg_pos = 0;
	unsigned int pkg_len = 0;
	int flag_err = 0;	//出错后置1
	struct timeval tm;
	unsigned long long frame_pts_us = 0;//收到Iframe 时重新赋值

	//frame cnt
	int frame_cnt = 0;

	if(NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		goto ProcQuit;
	}
	
	if(chn >= g_xm_client_count)
	{
		dbg("%s param error\n", __func__);
		goto ProcQuit;
	}
	
	pbuf = (char *)malloc(17*1024);//(char *)malloc(16*1024);//文档说一包数据不会超过16K
	if(NULL == pbuf)
	{
		dbg("%s -- chn%d malloc buf failed\n", __func__, chn);
		goto ProcOver;
	}
	
	if(chn < (int)(g_xm_client_count/2))
	{
		enc_len = 500*1024;//400*1024;
	}
	else
	{
		enc_len = 200*1024;
	}
	
	//csp note
	// 1.内存分配过大
	// 2.子码流的内存可以相应分配小一些
	//pbuf1 = (char *)malloc(1024*1024);
	pbuf1 = (char *)malloc(enc_len);
	if(NULL == pbuf1)
	{
		dbg("%s -- chn%d malloc buf1 failed\n", __func__, chn);
		goto Procbuf;
	}
	
	dbg("chn%d main sock: %d\n", chn, psession->main_sock);
	dbg("chn%d stream sock: %d\n", chn, psession->stream_sock);
	
	real_stream_s stream;
	memset(&stream, 0, sizeof(stream));
	stream.chn = chn;
	stream.data = (unsigned char *)pbuf1;	
	stream.rsv = 0;
	stream.mdevent = 0;
	stream.width = psession->dwStreamWidth;
	stream.height = psession->dwStreamHeight;
	stream.media_type = MEDIA_PT_H264;

	pchead = (ctrl_head_t *)pbuf;
	pdata = pbuf + sizeof(ctrl_head_t);
	
	while (g_init_flag)
	{
		pthread_mutex_lock(&psession->lock);
		if(psession->eventLoopWatchVariable)
		{
			pthread_mutex_unlock(&psession->lock);
			
			dbg("chn: %d eventLoopWatchVariable == 1, will be exit\n", chn);
			break;
		}
		pthread_mutex_unlock(&psession->lock);

		//接收一包数据
		memset(pchead, 0, sizeof(ctrl_head_t));
		ret = recv_timeout(psession->stream_sock, (char *)pchead, sizeof(ctrl_head_t));
		if (ret != int(sizeof(ctrl_head_t)))
		{
			err("error: chn%d - recv head\n", chn);

			flag_err = 1;
			break;
		}

		if(pchead->Data_Length > 16 * 1024)//文档说一包数据不会超过16K
		{
			err("error: chn%d pkg_len(%u) > 16 * 1024\n", chn, pchead->Data_Length);

			flag_err = 1;
			break;
		}

		#if 0
		if (chn == 16)
		{
			dbg("recv pkg: SessionID: %u, Message_Id: %u, SEQUENCE_NUMBER: %u, CurPacket(%u/%u), Data_Length: %u\n",
				pchead->SessionID, pchead->Message_Id, pchead->SEQUENCE_NUMBER, pchead->CurPacket, pchead->Total_Packet, pchead->Data_Length);
		}
		#endif

		ret = recv_timeout(psession->stream_sock, pdata, pchead->Data_Length);
		if (ret != int(pchead->Data_Length))
		{
			dbg("error: chn%d recv data\n", chn);

			flag_err = 1;
			break;
		}
		
		pkg_pos = 0;
		pkg_len = pchead->Data_Length;
		//处理数据
		while (pkg_pos < pkg_len)	//包中所有数据都处理完成
		{
			#if 0
			if (chn == 16)
			{
				dbg("frame_type: %d, frame_pos: %u, frame_len: %u, process_data_len: %u, Data_Length: %u\n",
					discern_frame_type, frame_pos, frame_len,
					pkg_pos, pkg_len);
			}
			#endif

			if (discern_frame_type == 0)	//还未识别帧类型
			{
				frame_len = 0;
				frame_pos = 0;
				b_ignore_one_frame = 0;
				
				if (memcmp(Iframe, pdata+pkg_pos, 4) == 0)
				{
					discern_frame_type = 1;
					memcpy(&frame_len, pdata+pkg_pos+12, 4);//实际帧大小，不包括pchead和I/P帧头部
					pkg_pos += 16;	//I帧头长					
					
					stream.media_type = MEDIA_PT_H264;
					stream.frame_type = REAL_FRAME_TYPE_I;

					gettimeofday(&tm, NULL);
					frame_pts_us = (unsigned long long)1000000*tm.tv_sec + tm.tv_usec;//tm.tv_usec;//csp modify
					frame_cnt = 0;
				}
				else if (memcmp(Pframe, pdata+pkg_pos, 4) == 0)
				{
					discern_frame_type = 2;
					memcpy(&frame_len, pdata+pkg_pos+4, 4);
					pkg_pos += 8;	//P帧头长

					stream.media_type = MEDIA_PT_H264;
					stream.frame_type = REAL_FRAME_TYPE_P;
					frame_cnt++;
				}
				else if (memcmp(Aframe, pdata+pkg_pos, 4) == 0)
				{
					discern_frame_type = 3;
					if (pdata[pkg_pos+4] == 0xe)
					{
						stream.media_type = MEDIA_PT_G711;
					}
					else
					{
						err("unknow audio format, header: %02x %02x %02x %02x %02x\n", 
						*(pdata+pkg_pos), *(pdata+pkg_pos+1), *(pdata+pkg_pos+2), *(pdata+pkg_pos+3), *(pdata+pkg_pos+4));

						flag_err = 1;
						break;
					}
					
					memcpy(&frame_len_s, pdata+pkg_pos+6, 2);//byte[6-7]
					frame_len = frame_len_s;
					
					pkg_pos += 8;	//音频帧头长
				}
				else
				{
					err("unknow FRAME_TYPE, header: %02x %02x %02x %02x\n", 
						*(pdata+pkg_pos), *(pdata+pkg_pos+1), *(pdata+pkg_pos+2), *(pdata+pkg_pos+3));

					flag_err = 1;
					break;
				}

				stream.len = frame_len;
				stream.pts = frame_pts_us;
				
				frame_pts_us += 40*1000;

				if (frame_len > enc_len)
				{
					if (frame_len > (1<<20))
					{
						err("chn%d frame_type: %d, len(%u) > 1M bytes\n", chn, 
							discern_frame_type,
							frame_len);
						
						flag_err = 1;
						break;
					}
					
					b_ignore_one_frame = 1;
					
					warning("chn%d frame_type: %d, len(%u) > bufsize(%u)\n", chn, 
						discern_frame_type,
						frame_len, enc_len);
				}
				
				#if 0
				if (chn == 16)
				{
					dbg("chn%d recv one frame, type: %d, len: %d, time: %llu, frame_cnt: %d\n",
 						chn, discern_frame_type, frame_len, stream.pts/1000, frame_cnt);
				}
				#endif
			}

			//当前包内有效数据不够填充该帧
			if (pkg_len - pkg_pos < frame_len - frame_pos)
			{
				if (!b_ignore_one_frame)
				{
					memcpy(pbuf1+frame_pos, pdata+pkg_pos, pkg_len-pkg_pos);
				}
				
				frame_pos += pkg_len-pkg_pos;
				pkg_pos = pkg_len;

				#if 0
				if (chn == 16)
				{
					dbg("1 frame_type: %d, frame_pos: %u, frame_len: %u, process_data_len: %u, Data_Length: %u\n",
						discern_frame_type, frame_pos, frame_len,
						pkg_pos, pkg_len);
				}
				#endif
			}
			else //当前包内有效数据足够填充该帧，一帧接收完成
			{
				if (!b_ignore_one_frame)
				{
					memcpy(pbuf1+frame_pos, pdata+pkg_pos, frame_len-frame_pos);
					
					//回调帧数据
					psession->pStreamCB(&stream, psession->dwContext);
				}				
				
				pkg_pos += frame_len-frame_pos;
				frame_pos = frame_len;

				#if 0
				if (chn == 16)
				{
					dbg("2 frame_type: %d, frame_cnt: %d, process_data_len: %u, Data_Length: %u\n",
						discern_frame_type, frame_cnt,
						pkg_pos, pkg_len);
				}
				#endif

				//重置
				discern_frame_type = 0;
			}
		}

		if (flag_err)
			break;		
	}

	pthread_mutex_lock(&psession->lock);
	if(psession->eventLoopWatchVariable)	
	{
		flag_err = 0;
	}
	pthread_mutex_unlock(&psession->lock);
	
	if(pbuf1 != NULL)
	{
		free(pbuf1);
		pbuf1 = NULL;
	}
	
Procbuf:
	if(pbuf != NULL)
	{
		free(pbuf);
		pbuf = NULL;
	}
	
ProcOver:
	if (flag_err)
		DoStreamStateCallBack(chn, REAL_STREAM_STATE_LOST);
	
ProcQuit:
	
	return 0;
}

#else
void* Thread_capture(void* pParam)
{
	int chn = 0;
	
	int ret = 0;
	char *pbuf = NULL;
	char *pbuf1 = NULL;
	Session_info_t *psession = NULL;
	ctrl_head_t *pchead = NULL;//(ctrl_head_t *)pbuf;
	char *pdata = NULL;//pbuf + sizeof(ctrl_head_t);
	real_frame_type_e frame_type = REAL_FRAME_TYPE_NONE;
	unsigned int frame_len = 0;
	unsigned int cur_len = 0;
	unsigned int pkg_len = 0;
	struct timeval tm;
	
	unsigned int enc_len = 0;
	
	chn = (int)pParam;
	if(chn >= g_xm_client_count)
	{
		dbg("%s param error\n", __func__);
		goto ProcQuit;
	}
	
	psession = get_session(chn);
	if(NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		goto ProcQuit;
	}
	
	pbuf = (char *)malloc(17*1024);//(char *)malloc(16*1024);//文档说一包数据不会超过16K
	if(NULL == pbuf)
	{
		dbg("%s -- chn%d malloc buf failed\n", __func__, chn);
		goto ProcOver;
	}
	
	if(chn < (int)(g_xm_client_count/2))
	{
		enc_len = 500*1024;//400*1024;
	}
	else
	{
		enc_len = 200*1024;
	}
	
	//csp note
	//1.内存分配过大
	//2.子码流的内存可以相应分配小一些
	//pbuf1 = (char *)malloc(1024*1024);
	pbuf1 = (char *)malloc(enc_len);
	if(NULL == pbuf1)
	{
		dbg("%s -- chn%d malloc buf1 failed\n", __func__, chn);
		goto Procbuf;
	}
	
	pchead = (ctrl_head_t *)pbuf;
	pdata = pbuf + sizeof(ctrl_head_t);
	
	dbg("chn%d main sock: %d\n", chn, psession->main_sock);
	dbg("chn%d stream sock: %d\n", chn, psession->stream_sock);
	
	real_stream_s stream;
	memset(&stream, 0, sizeof(stream));
	stream.chn = chn;
	//stream.data = (unsigned char *)recv_buf;
	//stream.len = packet_len;
	//stream.pts = timestamp;
	//stream.pts *= 1000;//微秒
	stream.rsv = 0;
	stream.mdevent = 0;
	stream.width = psession->dwStreamWidth;
	stream.height = psession->dwStreamHeight;
	stream.media_type = MEDIA_PT_H264;
	
	while(g_init_flag)
	{
		if(eventLoopWatchVariable[chn])
		{
			break;
		}
		
		memset(pchead, 0, sizeof(ctrl_head_t));
		ret = recv_timeout(psession->stream_sock, (char *)pchead, sizeof(ctrl_head_t), chn);
		if (ret != int(sizeof(ctrl_head_t)))
		{
			dbg("chn%d - recv media head respond head err\n", chn);
			break;
		}
		
		if(pchead->Data_Length > 16 * 1024)
		{
			dbg("chn%d Thread_capture error-0\n",chn);
			break;
		}
		
		ret = recv_timeout(psession->stream_sock, pdata, pchead->Data_Length, chn);
		if (ret != int(pchead->Data_Length))
		{
			dbg("chn%d recv media data respond data err\n", chn);
			break;
		}
		
		#if 0
		//dbg("%s - recv Data_Length: %d\n", psession->stream_type==MAIN_STREAM? "Main":"Extra1", pchead->Data_Length);
		#endif
		
		//dbg("%s - recv Data_Length: %d\n", (chn < (int)(g_xm_client_count/2))? "Main":"Extra1", pchead->Data_Length);
		
		/*
		dbg("recv data: \n");
		int i;
		for(i=0; i<32; i++)
		{
			dbg("\t 0x%02hhx", pdata[i]);
			if ((i+1)%8 == 0)
				dbg("\n");
		}
		*/
		#if 1
		if (chn == 16)
		{
			dbg("recv pkg: SessionID: %u, Message_Id: %u, SEQUENCE_NUMBER: %u, CurPacket(%u/%u), Data_Length: %u\n",
				pchead->SessionID, pchead->Message_Id, pchead->SEQUENCE_NUMBER, pchead->CurPacket, pchead->Total_Packet, pchead->Data_Length);
		}
		#endif

		
		if (frame_type == REAL_FRAME_TYPE_NONE)//一帧开始
		{
			if(cur_len != 0)
			{
				printf("%s warning: cur_len = %d\n", __func__, cur_len);
				cur_len = 0;
			}
			
			/*
			int i;
			for(i=0; i<32; i++)
			{
				dbg("\t 0x%02hhx", pdata[i]);
				if ((i+1)%8 == 0)
					dbg("\n");
			}
			*/
			if (memcmp(Iframe, pdata, 4) == 0)
			{
				frame_type = REAL_FRAME_TYPE_I;
			}
			else if (memcmp(Pframe, pdata, 4) == 0)
			{
				frame_type = REAL_FRAME_TYPE_P;
			}
			else
			{
				dbg("unknow FRAME_TYPE:(%02x %02x %02x %02x)\n",pdata[0],pdata[1],pdata[2],pdata[3]);
				continue;
			}
			
			if (frame_type == REAL_FRAME_TYPE_I)
			{
				memcpy(&frame_len, pdata+12, 4);//实际帧大小，不包括pchead和I/P帧头部
				pkg_len = pchead->Data_Length - 16;//当前包大小，包括I/P帧头部，所以要减去
				
				if(pkg_len > 16 * 1024)
				{
					dbg("chn%d Thread_capture error-1\n",chn);
					break;
				}
				
				memcpy(pbuf1, pdata+16, pkg_len);
				//fwrite(pdata+16, 1, pkg_len, pf);
				//fflush(pf);
				gettimeofday(&tm, NULL);
				if (chn == 16)
				dbg("chn%d %s - get Iframe size: %d, time: %ds-%03ldus\n", \
					chn, chn < g_xm_client_count/2? "Main":"Extra1", \
					frame_len, (int)tm.tv_sec, tm.tv_usec/1000);
				
				//stream.data = (unsigned char *)pdata+16;
			}
			else
			{
				memcpy(&frame_len, pdata+4, 4);
				pkg_len = pchead->Data_Length - 8;
				
				if(pkg_len > 16 * 1024)
				{
					dbg("chn%d Thread_capture error-2\n",chn);
					break;
				}
				
				memcpy(pbuf1, pdata+8, pkg_len);
				//fwrite(pdata+8, 1, pkg_len, pf);
				//fflush(pf);
				gettimeofday(&tm, NULL);
				if (chn == 16)
				dbg("chn%d %s - get Pframe size: %d, time: %ds-%03ldms\n", \
					chn, chn < g_xm_client_count/2? "Main":"Extra1", \
					frame_len, (int)tm.tv_sec, tm.tv_usec/1000);
				
				//stream.data = (unsigned char *)pdata+8;
			}

			#if 1
			if (chn == 16)
			dbg("1 frame_len: %u, cur_len: %u, pkg_len: %u\n",
				frame_len, cur_len, pkg_len);
			#endif
			
			cur_len += pkg_len;
			//dbg("cur frame len: %d\n", cur_len);
			if (cur_len == frame_len)
			{
				gettimeofday(&tm, NULL);
				
				stream.data = (unsigned char *)pbuf1;
				stream.frame_type = frame_type;
				stream.len = pkg_len;
				stream.pts = (unsigned long long)1000000*tm.tv_sec + tm.tv_usec;//tm.tv_usec;//csp modify
				psession->pStreamCB(&stream, psession->dwContext);
				
				//dbg("%s - recv frame over1\n", chn < g_xm_client_count/2? "Main":"Extra1");
				frame_type = REAL_FRAME_TYPE_NONE;
				cur_len = 0;
			}
			/*
			stream.frame_type = frame_type;
			stream.len = pkg_len;
			stream.pts = tm.tv_usec;
			psession->pStreamCB(&stream, psession->dwContext);
			*/
		}
		else
		{
			pkg_len = pchead->Data_Length;
			
			//memcpy(pbuf1+cur_len, pdata, pkg_len);
			if((frame_len <= enc_len) && ((cur_len + pkg_len) <= enc_len) && (cur_len <= enc_len))
			{
				memcpy(pbuf1+cur_len, pdata, pkg_len);
			}
			else
			{
				dbg("chn%d Thread_capture error-3\n",chn);
				break;
			}
			
			//fwrite(pdata, 1, pkg_len, pf);
			//fflush(pf);
			//dbg("next pkg len: %d\n", pkg_len);
			#if 1
			if (chn == 16)
			dbg("2 frame_len: %u, cur_len: %u, pkg_len: %u\n",
				frame_len, cur_len, pkg_len);
			#endif
			
			cur_len += pkg_len;
			//dbg("cur frame len: %d\n", cur_len);
			//if (cur_len == frame_len)
			if (cur_len >= frame_len)
			{
				if(frame_len <= enc_len)
				{
					gettimeofday(&tm, NULL);
					
					stream.data = (unsigned char *)pbuf1;
					stream.frame_type = frame_type;
					stream.len = cur_len;//frame_len;
					stream.pts = (unsigned long long)1000000*tm.tv_sec + tm.tv_usec;//tm.tv_usec;//csp modify
					psession->pStreamCB(&stream, psession->dwContext);
				}
				
				//dbg("%s - recv frame over2\n", chn < g_xm_client_count/2? "Main":"Extra1");
				frame_type = REAL_FRAME_TYPE_NONE;
				cur_len = 0;
			}
			#if 0
			else if(cur_len > frame_len)
			{
				dbg("%s - recv frame over3, cur_len(%u) > frame_len(%u)\n", \
					chn < g_xm_client_count/2? "Main":"Extra1", cur_len, frame_len);
				
				frame_type = REAL_FRAME_TYPE_NONE;
				cur_len = 0;
				
				dbg("chn%d Thread_capture error-4\n",chn);
				break;
			}
			#endif
			/*
			stream.frame_type = frame_type;
			stream.data = (unsigned char *)pdata;
			stream.len = pkg_len;
			//stream.pts = tm.tv_usec;//帧中间数据和帧头的时间相同
			psession->pStreamCB(&stream, psession->dwContext);
			*/
		}
	}
	
	if(pbuf1 != NULL)
	{
		free(pbuf1);
		pbuf1 = NULL;
	}
Procbuf:
	if(pbuf != NULL)
	{
		free(pbuf);
		pbuf = NULL;
	}
ProcOver:
	DoStreamStateCallBack(chn, REAL_STREAM_STATE_LOST);
ProcQuit:
	if(psession != NULL)
	{
		psession->capture_tid = 0;
		eventLoopWatchVariable[chn] = 0;
	}
	
	pthread_detach(pthread_self());
	
	return 0;
}
#endif

int XM_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	int ret;
	Session_info_t *psession = NULL;
	
	if(!g_init_flag)
	{
		return FAILURE;
	}
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	
	psession->chn = chn;
	if(!g_sdk_inited)
	{
		if(pthread_create(&keep_alive_pid, NULL, Thread_keep_alive, NULL) < 0)
		{
			perror("create xm keep alive thread failure");
			return FAILURE;
		}
		
		g_sdk_inited = 1;
	}
	
	if(XM_GetLinkStatus(chn))
	{
		XM_Stop(chn);
	}
	
	if(xm_login(chn, dwIp, wPort, user, pwd))
	{
		dbg("chn%d xm_login failed\n", chn);
		return FAILURE;
	}
	
	if(xm_capture_start(chn))
	{
		dbg("chn%d xm_capture failed\n",chn);
		
		goto fail_logout;
	}
	
	pthread_mutex_lock(&psession->lock);
	
	psession->pStreamCB = pCB;
	psession->dwContext = dwContext;

	ret = pthread_create(&psession->capture_tid, NULL, Thread_capture, (void *)psession);
	if (ret != 0)
	{
		pthread_mutex_unlock(&psession->lock);
		
		dbg("chn%d pthread_create capture failed\n", chn);
		goto fail_capture_stop;
	}

	psession->connected = 1;
	pthread_mutex_unlock(&psession->lock);
	
	dbg("chn%d success\n", chn);
	
	return SUCCESS;


fail_capture_stop:
	xm_capture_stop(chn);

fail_logout:
	xm_logout(chn);
	
	pthread_mutex_lock(&psession->lock);
	if(psession->stream_sock != -1)
	{
		close(psession->stream_sock);
		psession->stream_sock = -1;
	}
	if(psession->main_sock != -1)
	{
		close(psession->main_sock);
		psession->main_sock = -1;
	}

	psession->SessionID = -1;
	psession->capture_tid = -1;
	psession->eventLoopWatchVariable = 0;
	psession->connected = 0;
	
	pthread_mutex_unlock(&psession->lock);

	return FAILURE;
}

int XM_Stop(int chn)//
{
	Session_info_t *psession = NULL;
 	
	if (!g_init_flag)
	{
		dbg("%s g_init_flag invaild\n", __func__);
		return FAILURE;
	}
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	
	dbg("chn%d\n", chn);

	pthread_mutex_lock(&psession->lock);
	if (!psession->connected)
	{
		dbg("chn%d already stop\n", chn);
		pthread_mutex_unlock(&psession->lock);
		
		return SUCCESS;
	}

	pthread_mutex_unlock(&psession->lock);

	xm_capture_stop(chn);

	if (psession->capture_tid != -1)
	{
		dbg("chn%d stop thread\n", chn);
		psession->eventLoopWatchVariable = 1;
		//pthread_mutex_unlock(&psession->lock);

		pthread_join(psession->capture_tid, NULL);
		dbg("chn%d stop thread success\n", chn);
		//pthread_mutex_lock(&psession->lock);
	}

	//登出
	xm_logout(chn);

	pthread_mutex_lock(&psession->lock);
	
	if (psession->stream_sock != -1)
	{
		close(psession->stream_sock);
		psession->stream_sock = -1;
	}

	if (psession->main_sock!= -1)
	{
		close(psession->main_sock);
		psession->main_sock = -1;
	}

	psession->SessionID = -1;
	psession->capture_tid = -1;
	psession->eventLoopWatchVariable = 0;
	psession->connected = 0;
	
	pthread_mutex_unlock(&psession->lock);
	
	return SUCCESS;
}


int XM_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
#if 1//csp modify 20150421
	printf("%s do not support\n", __func__);
	return FAILURE;
#else
	int chn;
	Session_info_t *psession = NULL;
	int ret;
	char buf[BUF_SIZE];
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	char *tmp = NULL;
	
	if(!g_init_flag)
	{
		return FAILURE;
	}
	
	if(ipcam == NULL)
	{
		return FAILURE;
	}
	
	if(pnw == NULL)
	{
		return FAILURE;
	}
	
	chn = (int)ipcam->channel_no;
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
//get DNS
	u32 dns, spare_dns;
	sprintf(pmsg, "{ \"Name\" : \"NetWork.NetDNS\","
		" \"SessionID\" : \"0x%08x\" }\n", psession->SessionID);
	dbg("get NetDNS: %s\n", pmsg);
	
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = psession->SessionID;
    pchead->Message_Id = CONFIG_GET;//0x412
    pchead->Data_Length = strlen(pmsg);// + 1;
    
	pthread_mutex_lock(&psession->lock);
    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if (ret !=  SUCCESS)
	{
		pthread_mutex_unlock(&psession->lock);
		dbg("%s CONFIG_GET failed\n", __func__);
		return FAILURE;
	}
	pthread_mutex_unlock(&psession->lock);	
	dbg("%s send CONFIG_GET ok\n", __func__);
	
	tmp = strstr(pmsg, "Address");
	if ((tmp == NULL) || (sscanf(tmp+strlen("Address")+5, "0x%x", &dns) != 1))
	{
		dbg("%s CONFIG_GET respond DNS failed\n", __func__);
		return FAILURE;
	}
	tmp = strstr(pmsg, "SpareAddress");
	if ((tmp == NULL) || (sscanf(tmp+strlen("SpareAddress")+5, "0x%x", &spare_dns) != 1))
	{
		dbg("%s CONFIG_GET respond Spare_DNS failed\n", __func__);
		return FAILURE;
	}
	dbg("DNS: %s\n", inet_ntoa(*(struct in_addr *)&dns));
	dbg("Spare_DNS: %s\n", inet_ntoa(*(struct in_addr *)&spare_dns));
	ipcam->dns1 = dns;
	ipcam->dns2 = spare_dns;
	
	pnw->ip_address = ipcam->dwIp;
	pnw->net_mask = ipcam->net_mask;
	pnw->net_gateway = ipcam->net_gateway;
	pnw->dns1 = ipcam->dns1;
	pnw->dns2 = ipcam->dns2;	
	
	return SUCCESS;
#endif
}

int XM_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	Session_info_t *psession = NULL;
	
	int ret;
	char buf[BUF_SIZE];
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	char *tmp = NULL;
	
	if(!g_init_flag)
	{
		return FAILURE;
	}
	
	if(ipcam == NULL)
	{
		return FAILURE;
	}
	
	if(pnw == NULL)
	{
		return FAILURE;
	}
	
	#if 1//csp modify 20150421
	Session_info_t sinfo;
	psession = &sinfo;
	
	unsigned int dwIp = ipcam->dwIp;
	unsigned short wPort = ipcam->wPort;
	
	//main_sock
	u16 wErrorCode = 0;
	int main_sock = ConnectWithTimeout(dwIp, wPort, 5000, &wErrorCode);
	if(main_sock < 0)
	{
		dbg("XM_ConnectWithTimeout main_sock connect (0x%08x:%d) failed\n", dwIp, wPort);
		return FAILURE;
	}
	//dbg("%s -- create socket ok, fd: %d\n", __func__, main_sock);
	
//login
	//char *pmsg = buf + sizeof(ctrl_head_t);
	//(None | 3DES | MD5)???
	sprintf(pmsg, "{ \"EncryptType\" : \"MD5\","
				" \"LoginType\" : \"DVRIP-Web\","
				" \"PassWord\" : \"tlJwpbo6\","
				" \"UserName\" : \"admin\" }\n");
	
	dbg("login msg: %s\n", pmsg);
	//ctrl_head_t *pchead = (ctrl_head_t *)buf;
    memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
    pchead->Message_Id = LOGIN_REQ;
    pchead->Data_Length = strlen(pmsg);// + 1;
    
    ret = loopsend(main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if(ret != (int)(sizeof(ctrl_head_t)+pchead->Data_Length))
	{
		err("send_data err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("send login data ok\n");
	
	memset(pchead, 0, sizeof(ctrl_head_t));
	ret = recv_timeout(main_sock, buf, sizeof(ctrl_head_t));
	if (ret != int(sizeof(ctrl_head_t)))
	{
		err("recv login respond head err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	/*
	dbg("Head_Flag: 0x%x\n", pchead->Head_Flag);
    dbg("VERSION: %d\n", pchead->VERSION);
    dbg("SESSION_ID: %d\n", pchead->SessionID);
    dbg("SEQUENCE_NUMBER: %d\n", pchead->SEQUENCE_NUMBER);
    dbg("Total_Packet: %d\n", pchead->Total_Packet);
    dbg("CurPacket: %d\n", pchead->CurPacket);
    dbg("Message_Id: %d\n", pchead->Message_Id);
    dbg("Data_Length: %d\n", pchead->Data_Length);
	*/
	ret = recv_timeout(main_sock, pmsg, pchead->Data_Length);
	if (ret != int(pchead->Data_Length))
	{
		err("recv login respond data err\n");
		close(main_sock);//csp modify
		return FAILURE;
	}
	dbg("respon data: %s\n", pmsg);
	
	char *p = NULL;
	
	#if 0//AliveInterval
	if (alive_Interval == 0)
	{
	    p = strstr(buf+sizeof(ctrl_head_t), "AliveInterval");
	    if (NULL == p)
	    {
	    	err("strstr AliveInterval err");
			close(main_sock);//csp modify
	    	return FAILURE;
	    }
	    ret = sscanf(p+strlen("AliveInterval")+4, "%d", &alive_Interval);
	    dbg("ret = %d\n", ret);
	    dbg("alive_Interval: %d\n", alive_Interval);
	}
	#endif
	
    /*
    //channelnum
    int channel;
    p = strstr(buf+sizeof(ctrl_head_t), "ChannelNum");
    if (NULL == p)
    {
    	err("strstr ChannelNum err");
    	close(main_sock);//csp modify
    	return FAILURE;
    }
    ret = sscanf(p+strlen("ChannelNum")+4, "%d", &channel);
    dbg("ret = %d\n", ret);
    psession->channel = (unsigned char)channel;
    dbg("ChannelNum: %d\n", psession->channel);
    */
//session id
	int SessionID;
    p = strstr(buf+sizeof(ctrl_head_t), "SessionID");
    if (NULL == p)
    {
    	err("strstr SessionID err\n");
		close(main_sock);//csp modify
    	return FAILURE;
    }
    ret = sscanf(p+strlen("SessionID")+5, "0x%x", &SessionID);
    dbg("ret = %d\n", ret);
    dbg("SessionID: %d\n", SessionID);
	
	psession->main_sock = main_sock;
	psession->SessionID = SessionID;
	#else
	int chn;
	chn = (int)ipcam->channel_no;
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	#endif
	
//set dns
	sprintf(pmsg, "{ \"Name\" : \"NetWork.NetDNS\","
	" \"NetWork.NetDNS\" : {"
	" \"Address\" : \"0x%08x\","
	" \"SpareAddress\" : \"0x%08x\" },"
	" \"SessionID\" : \"0x%08x\" }\n", pnw->dns1, pnw->dns2, psession->SessionID);
	dbg("set NetDNS: %s\n", pmsg);
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = psession->SessionID;
    pchead->Message_Id = CONFIG_SET;//0x410
    pchead->Data_Length = strlen(pmsg);// + 1;
    
	//pthread_mutex_lock(&psession->lock);//csp modify 20150421
    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if (ret !=  SUCCESS)
	{
		//pthread_mutex_unlock(&psession->lock);//csp modify 20150421
		close(psession->main_sock);//csp modify 20150421
		dbg("%s CONFIG_SET NetDNS failed\n", __func__);
		return FAILURE;
	}
	//pthread_mutex_unlock(&psession->lock);//csp modify 20150421
	dbg("%s send CONFIG_SET NetDNS ok\n", __func__);
	#if 0//不对//set dhcp//csp modify 20150421
	sprintf(pmsg, "{ \"Name\" : \"NetWork.NetDHCP\","
	" \"NetWork.NetDHCP\" : [{"
	" \"Enable\" : \"false\","
	" \"Interface\" : \"eth0\" },"
	"{"
	" \"Enable\" : \"false\","
	" \"Interface\" : \"eth1\" }],"
	" \"SessionID\" : \"0x%08x\" }\n", psession->SessionID);
	dbg("set NetDHCP: %s\n", pmsg);
	memset(pchead, 0, sizeof(ctrl_head_t));
	pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = psession->SessionID;
	pchead->Message_Id = CONFIG_SET;//0x410
	pchead->Data_Length = strlen(pmsg);// + 1;
	
	//pthread_mutex_lock(&psession->lock);//csp modify 20150421
	ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
	if (ret !=	SUCCESS)
	{
		//pthread_mutex_unlock(&psession->lock);//csp modify 20150421
		close(psession->main_sock);//csp modify 20150421
		dbg("%s CONFIG_SET NetDHCP failed\n", __func__);
		return FAILURE;
	}
	//pthread_mutex_unlock(&psession->lock);//csp modify 20150421
	dbg("%s send CONFIG_SET NetDHCP ok\n", __func__);
	#endif
//先获取网络设置 修改 再设置
//get net info
	sprintf(pmsg, "{ \"Name\" : \"NetWork.NetCommon\","
		" \"SessionID\" : \"0x%08x\" }\n", psession->SessionID);
	dbg("get NetWork.NetCommon: %s\n", pmsg);
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = psession->SessionID;
    pchead->Message_Id = CONFIG_GET;//0x410
    pchead->Data_Length = strlen(pmsg);// + 1;
    
	//pthread_mutex_lock(&psession->lock);//csp modify 20150421
    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if (ret !=  SUCCESS)
	{
		//pthread_mutex_unlock(&psession->lock);//csp modify 20150421
		close(psession->main_sock);//csp modify 20150421
		dbg("%s CONFIG_GET failed\n", __func__);
		return FAILURE;
	}
	//pthread_mutex_unlock(&psession->lock);//csp modify 20150421
	dbg("%s send CONFIG_GET ok\n", __func__);
//modify ip
	tmp = strstr(pmsg, "HostIP");
	if (tmp == NULL)
	{
		dbg("%s CONFIG_GET respond HostIP failed\n", __func__);
		close(psession->main_sock);//csp modify 20150421
		return FAILURE;
	}
	sprintf(tmp+strlen("HostIP")+5, "0x%08x", pnw->ip_address);
	tmp[strlen(tmp)] = '"';//因为sprintf会在此写'\0'
//modify netmask
	tmp = strstr(pmsg, "Submask");
	if (tmp == NULL)
	{
		dbg("%s CONFIG_GET respond Submask failed\n", __func__);
		close(psession->main_sock);//csp modify 20150421
		return FAILURE;
	}
	sprintf(tmp+strlen("Submask")+5, "0x%08x", pnw->net_mask);
	tmp[strlen(tmp)] = '"';
//modify gateway
	tmp = strstr(pmsg, "GateWay");
	if (tmp == NULL)
	{
		dbg("%s CONFIG_GET respond GateWay failed\n", __func__);
		close(psession->main_sock);//csp modify 20150421
		return FAILURE;
	}
	sprintf(tmp+strlen("GateWay")+5, "0x%08x", pnw->net_gateway);
	tmp[strlen(tmp)] = '"';
//del "Ret" : 100 ,把后面的字符移过来覆盖
	char *tmp1 = strstr(pmsg, "Ret");
	if (tmp1 == NULL)
	{
		dbg("%s CONFIG_GET respond Ret failed\n", __func__);
		close(psession->main_sock);//csp modify 20150421
		return FAILURE;
	}
	tmp = strstr(pmsg, "SessionID");
	if (tmp == NULL)
	{
		dbg("%s CONFIG_GET respond SessionID failed\n", __func__);
		close(psession->main_sock);//csp modify 20150421
		return FAILURE;
	}
	int overlen = strlen(tmp);
//目标区域和源区域有重叠，使用memmove
	memmove(tmp1, tmp, overlen+1);//+ '\0'
//Set net info
	dbg("set NetWork.NetCommon: %s\n", pmsg);
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = psession->SessionID;
    pchead->Message_Id = CONFIG_SET;//0x410
    pchead->Data_Length = strlen(pmsg);// + 1;
    
	//pthread_mutex_lock(&psession->lock);//csp modify 20150421
    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if (ret !=  SUCCESS)
	{
		//pthread_mutex_unlock(&psession->lock);//csp modify 20150421
		close(psession->main_sock);//csp modify 20150421
		dbg("%s CONFIG_SET NetCommon failed as success\n", __func__);
		return SUCCESS;//return FAILURE;//csp modify 20150421
	}
	//pthread_mutex_unlock(&psession->lock);//csp modify 20150421
	
	dbg("%s send CONFIG_SET NetCommon ok\n", __func__);
	
	close(psession->main_sock);//csp modify 20150421
	
	return SUCCESS;
}

int XM_CMD_SetTime(int chn, time_t t, int force)
{
	Session_info_t *psession = NULL;
	int ret;
	char buf[BUF_SIZE];
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return FAILURE;
	}
	
	if(!force)
	{
		if(!ipcam.enable || !XM_GetLinkStatus(chn))
		{
			//printf("XM_CMD_SetTime: chn%d vlost!!!\n",chn);
			return FAILURE;
		}
	}
	
	if(t == 0)
	{
		t = time(NULL);// + 1;//???
	}
	
	int nTimeZone = IPC_GetTimeZone();
	t += GetTZOffset(nTimeZone);
	
	struct tm now;
	localtime_r(&t,&now);
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	//settime
	sprintf(pmsg, "{ \"Name\" : \"OPTimeSettingNoRTC\","
		" \"OPTimeSettingNoRTC\" : \"%d-%d-%d %d:%d:%d\","
		" \"SessionID\" : \"0x%x\" }\n", \
		now.tm_year+1900, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, \
		psession->SessionID);
	dbg("TimeSetting: %s\n", pmsg);
	
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
    pchead->SessionID = psession->SessionID;
    pchead->SEQUENCE_NUMBER = 1;
    pchead->Message_Id = TIME_SET_REQ;//time setting 0x636
    pchead->Data_Length = strlen(pmsg);// + 1;
    
    pthread_mutex_lock(&psession->lock);
    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if (ret !=  SUCCESS)
	{
		pthread_mutex_unlock(&psession->lock);
		dbg("%s TIME_SET_REQ failed\n", __func__);
		return FAILURE;
	}
	pthread_mutex_unlock(&psession->lock);
	dbg("send TIME_SET_REQ ok\n");
	
	return SUCCESS;
}

int XM_CMD_Reboot(int chn)
{
	Session_info_t *psession = NULL;
	int ret;
	char buf[BUF_SIZE];
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	ipc_unit ipcam;
	
	if(IPC_Get(chn, &ipcam))
	{
		return FAILURE;
	}
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	
	sprintf(pmsg, "{ \"Name\" : \"OPMachine\","
		" \"OPMachine\" : { \"Action\" : \"Reboot\" },"
		" \"SessionID\" : \"0x%08x\" }\n", psession->SessionID);
	dbg("set Reboot: %s\n", pmsg);
	
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = psession->SessionID;
    pchead->Message_Id = SYSMANAGER_REQ;
    pchead->Data_Length = strlen(pmsg);// + 1;
    
    pthread_mutex_lock(&psession->lock);
    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if (ret !=  SUCCESS)
	{
		pthread_mutex_unlock(&psession->lock);
		dbg("%s Reboot failed\n", __func__);
		return FAILURE;
	}
	pthread_mutex_unlock(&psession->lock);
	dbg("send Reboot ok\n");
	
	return SUCCESS;
}

int XM_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data)
{
	Session_info_t *psession = NULL;
	int ret;
	char buf[BUF_SIZE];
	ctrl_head_t *pchead = (ctrl_head_t *)buf;
	char *pmsg = buf + sizeof(ctrl_head_t);
	ipc_unit ipcam;
	
	
	if(IPC_Get(chn, &ipcam))
	{
		return FAILURE;
	}
		
	if(!ipcam.enable || !XM_GetLinkStatus(chn))
	{
		dbg("XM_CMD_SetTime: chn%d vlost!!!\n",chn);
		return FAILURE;
	}
	
	psession = get_session(chn);
	if (NULL == psession)
	{
		dbg("%s get_session failed\n", __func__);
		return FAILURE;
	}
	
	u32 step = *(u32 *)data;
	if (cmd >= EM_PTZ_CMD_START_TILEUP && cmd <= EM_PTZ_CMD_START_IRISLARGE)
	{
		if (step < 1)
			step = 1;
		if (step > 8)
			step = 8;
	}
	
	if(cmd == EM_PTZ_CMD_START_TILEUP)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"DirectionUp\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_TILEDOWN)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"DirectionDown\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_PANLEFT)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"DirectionLeft\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_PANRIGHT)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"DirectionRight\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_LEFTUP)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"DirectionLeftUp\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_LEFTDOWN)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"DirectionLeftDown\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_RIGHTUP)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"DirectionRightUp\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_RIGHTDOWN)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"DirectionRightDown\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_ZOOMTELE)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"ZoomTile\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_ZOOMWIDE)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"ZoomWide\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_FOCUSNEAR)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"FocusNear\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_FOCUSFAR)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"FocusFar\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_IRISSMALL)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"IrisSmall\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_IRISLARGE)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"IrisLarge\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : -1,"
			" \"Step\" : \"%d\","
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", step, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd >= EM_PTZ_CMD_STOP_TILEUP && cmd < EM_PTZ_CMD_PRESET_SET)
	{
		dbg("unknow ptz cmd\n");
	}
	else if(cmd == EM_PTZ_CMD_PRESET_SET)
	{
		u32 preset = *(u32 *)data;
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"SetPreset\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : \"%d\","
			" \"Step\" : 0,"
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", preset, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_PRESET_GOTO)
	{
		u32 preset = *(u32 *)data;
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"GotoPreset\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : \"%d\","
			" \"Step\" : 0,"
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", preset, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_PRESET_CLEAR)
	{
		u32 preset = *(u32 *)data;
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"ClearPreset\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : \"%d\","
			" \"Step\" : 0,"
			" \"Tour\" : 0 } },"
			" \"SessionID\" : \"0x%x\" }\n", preset, psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_START_TOUR)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"StartTour\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : 0,"
			" \"Step\" : 0,"
			" \"Tour\" : 1 } },"
			" \"SessionID\" : \"0x%x\" }\n", psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else if(cmd == EM_PTZ_CMD_STOP_TOUR)
	{
		sprintf(pmsg, "{ \"Name\" : \"OPPTZControl\","
			" \"OPPTZControl\" : {"
			" \"Command\" : \"StopTour\","
			" \"Parameter\" : {"
			" \"AUX\" : {"
			" \"Number\" : 0,"
			" \"Status\" : \"On\" },"
			" \"Channel\" : 0,"
			" \"MenuOpts\" : \"Enter\","
			" \"Pattern\" : \"Start\","
			" \"Preset\" : 0,"
			" \"Step\" : 0,"
			" \"Tour\" : 1 } },"
			" \"SessionID\" : \"0x%x\" }\n", psession->SessionID);
		dbg("set ptz: %s\n", pmsg);
	}
	else
	{
		dbg("unknow ptz cmd %d\n", cmd);
	}
	/*
	else if(cmd == EM_PTZ_CMD_LIGHT_ON)
	{
		dbg("unknow ptz cmd EM_PTZ_CMD_LIGHT_ON\n");
	}
	else if(cmd == EM_PTZ_CMD_LIGHT_OFF)
	{
		dbg("unknow ptz cmd EM_PTZ_CMD_LIGHT_OFF\n");
	}
	else if(cmd == EM_PTZ_CMD_AUX_ON)
	{
		unsigned int id = *((unsigned int *)data);
		if(id == 1)//默认1号辅助为雨刷
		{
			sprintf(pmsg,"/pmsg-bin/hi3510/ptzctrl.pmsg?-step=0&-act=brush&-switch=%d",1);
		}
	}
	else if(cmd == EM_PTZ_CMD_AUX_OFF)
	{
		unsigned int id = *((unsigned int *)data);
		if(id == 1)//默认1号辅助为雨刷
		{
			sprintf(pmsg,"/pmsg-bin/hi3510/ptzctrl.pmsg?-step=0&-act=brush&-switch=%d",0);
		}
	}
	else if(cmd == EM_PTZ_CMD_AUTOPAN_ON)
	{
		sprintf(pmsg,"/pmsg-bin/hi3510/ptzctrl.pmsg?-step=0&-act=auto");
	}
	else if(cmd == EM_PTZ_CMD_AUTOPAN_OFF)
	{
		sprintf(pmsg,"/pmsg-bin/hi3510/ptzctrl.pmsg?-step=0&-act=stop&-speed=%d",0);
	}
	*/
	memset(pchead, 0, sizeof(ctrl_head_t));
    pchead->Head_Flag = 0xff;
	pchead->VERSION = VER;
	pchead->SessionID = psession->SessionID;
    pchead->Message_Id = PTZ_REQ;
    pchead->Data_Length = strlen(pmsg);// + 1;
    
    pthread_mutex_lock(&psession->lock);
    ret = cmd_session(psession->main_sock, buf, sizeof(ctrl_head_t)+pchead->Data_Length);
    if (ret !=  SUCCESS)
	{
		pthread_mutex_unlock(&psession->lock);
		dbg("%s ptz cmd failed\n", __func__);
		return FAILURE;
	}
	pthread_mutex_unlock(&psession->lock);
	dbg("send ptz cmd ok\n");
	
	return SUCCESS;
}

int XM_CMD_Open(int chn)
{
	return SUCCESS;
}

int XM_CMD_Close(int chn)
{
	return SUCCESS;
}

int XM_CMD_SetImageParam(int chn, video_image_para_t *para)
{
	return SUCCESS;
}

int XM_CMD_GetImageParam(int chn, video_image_para_t *para)
{
	return FAILURE;
	//return SUCCESS;
}

int XM_CMD_RequestIFrame(int chn)
{
	//return FAILURE;
	return SUCCESS;
}

