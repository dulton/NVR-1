#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "I13.h"

typedef char			HI_CHAR;
typedef char*			HI_PCHAR;

typedef unsigned char	HI_UCHAR;

typedef signed char		HI_S8;
typedef short			HI_S16;
typedef int				HI_S32;

typedef unsigned char	HI_U8;
typedef unsigned short	HI_U16;
typedef unsigned int	HI_U32;

typedef int				SOCKET;

#define HI_SUCCESS		0
#define HI_FAILURE		(-1)

#define TIMEOUT			4000//5000//csp modify 20131111
#define BUF_LEN			2048

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

extern "C" unsigned int GetLocalIp();
extern "C" int ConnectWithTimeout(u32 dwHostIp, u16 wHostPort, u32 dwTimeOut, u16 *pwErrorCode);

typedef struct
{
	int cap_fd;
	unsigned int dwIp;
	unsigned short wPort;
	char username[32];
	char password[32];
	int channel_info;
	char stream_type[32];
	RealStreamCB pStreamCB;
	unsigned int dwContext;
	int video_width;
	int video_height;
	pthread_t pid;
	char eventLoopWatchVariable;
	unsigned int frame_count;//csp modify 20140423
	time_t last_t;//csp modify 20140423
	u32 last_pts;//csp modify 20140423
	pthread_mutex_t lock;
}i13_client_info;

static i13_client_info *g_i13c_info = NULL;

static unsigned int g_i13_client_count = 0;

static unsigned char g_init_flag = 0;

static HI_S32 SendData(SOCKET sock, HI_U8* pu8Buff, HI_U32 u32DataLen);

int ipnc_do_cgi_cmd(ipc_unit *ipcam, char* cgi);
int ipnc_exec_cgi_cmd(ipc_unit *ipcam, char* cgi, char *result);

/*------------------- define ---------------*/
#define PT_H264 96
#define PT_G726	97	
#define PT_G711 8
#define PT_DATA 100

#define CHANNEL1 11
#define CHANNEL2 12

#define STREAM_TYPE1 "video_audio"//"video_audio_data"
#define STREAM_TYPE2 "video_audio"
#define STREAM_TYPE3 "video_data"
#define STREAM_TYPE4 "audio_data"
#define STREAM_TYPE5 "video"
#define STREAM_TYPE6 "audio"

//#define COMMAND	 "netconf set -ipaddr 192.168.1.52 -netmask 255.255.255.0 -gateway 192.168.1.1 -dhcp off -fdnsip 192.168.1.1 -sdnsip 211.23.12.13 -dnsstat 0 -hwaddr 00:01:89:11:11:07"
//#define COMMAND	 "netconf set -ipaddr %s -netmask %s -gateway %s -fdnsip %s -sdnsip %s -dhcp off"
#define COMMAND	 "netconf set -ipaddr %s -netmask %s -gateway %s -fdnsip %s -dhcp off"

//#define BUF_SIZE (512*1024*2)

typedef struct sRTP_HDR_S
{
    /* byte 0 */
    unsigned short cc      :4;   /* CSRC count */
    unsigned short x       :1;   /* header extension flag */
    unsigned short p       :1;   /* padding flag */
    unsigned short version :2;   /* protocol version */
    /* byte 1 */
    unsigned short pt      :7;   /* payload type */
    unsigned short marker  :1;   /* marker bit */
    /* bytes 2, 3 */
    unsigned short seqno  :16;   /* sequence number */
    /* bytes 4-7 */
    unsigned int ts;            /* timestamp in ms */
    /* bytes 8-11 */
    unsigned int ssrc;          /* synchronization source */
} RTP_HDR_S;

typedef struct sRTSP_ITLEAVED_HDR_S
{
    unsigned char  daollar;      /*8, $:dollar sign(24 decimal)*/
    unsigned char  channelid;    /*8, channel id*/
    unsigned short resv;         /*16, reseved*/
    unsigned int payloadLen;   /*32, payload length*/
    RTP_HDR_S rtpHead;   /*rtp head*/
}RTSP_ITLEAVED_HDR_S;

/*--------------- main -------------------*/

#define HOST "192.168.1.88"//"192.168.1.105"
#define PORT 80

#define STRING_LEN 32
typedef struct sPARAM_S
{
	char Uname[STRING_LEN];		//username
	char Pword[STRING_LEN];		//password
	char Ip[STRING_LEN];		//IPCam IP address
	unsigned short Port;		//IPCam http port
	char* Buf;			//data buffer
	int State;			        //streaming thread state
	int Socket;					//socket handle
} PARAM_S;

/* send request packet of streaming to IPCam */
static int RequestStreamCommand(PARAM_S* pParam, int channel,char *stream_type)
{
	char out_buffer[1024];
	char content[256];
	int ret;
	
	memset(content, 0, 256);
	memset(out_buffer, 0, 1024);
	
	//sprintf(out_buffer, "GET http://%s:%d/livestream/%d?action=play&media=%s HTTP/1.1\r\n", 
	//	pParam->Ip, pParam->Port, CHANNEL1, STREAM_TYPE5);
	sprintf(out_buffer, "GET http://%s:%d/livestream/%d?action=play&media=%s HTTP/1.1\r\n", 
		pParam->Ip, pParam->Port, channel, stream_type);
	
	strcpy(out_buffer+strlen(out_buffer), "User-Agent: HiIpcam/V100R003 VodClient/1.0.0\r\n");
	strcpy(out_buffer+strlen(out_buffer), "Connection: Keep-Alive\r\n");
	strcpy(out_buffer+strlen(out_buffer), "Cache-Control: no-cache\r\n");
	sprintf(out_buffer+strlen(out_buffer), "Authorization: %s %s\r\n",pParam->Uname, pParam->Pword);	
	strcpy(content, "Cseq: 1\r\n");
	strcpy(content+strlen(content), "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n");
	sprintf(out_buffer+strlen(out_buffer), "Content-Length: %d\r\n", strlen(content));
	strcpy(out_buffer+strlen(out_buffer), "\r\n");
	strcpy(out_buffer+strlen(out_buffer), content);
	
	//csp modify 20131111
	//ret = send(pParam->Socket, out_buffer, strlen(out_buffer), 0);
	//if(ret < 0)
	//{
	//	printf("RequestStreamCommand send error \n");	
	//	return -1;
	//}
	ret = SendData(pParam->Socket, (HI_U8 *)out_buffer, strlen(out_buffer));
	if(HI_SUCCESS != ret)
	{
		printf("RequestStreamCommand SendData error \n");	
		return -1;
	}
	
	return 0;
}

/* get video and audio information */
static int GetVideoAndAudioAttr(char* buf,int chn)
{
	char* tmp;
	char str[2][16];
	int video_format, video_width, video_height;
	int audio_format;
	
	tmp = strstr(buf, "m=video");
	if(tmp != NULL)
	{
		sscanf(tmp,"%*s %d %*[^/]/%*[^/]/%32[^/]/%32[^\r]", &video_format,str[0],str[1]);
		video_width = atoi(str[0]);
		video_height = atoi(str[1]);
		printf("#################chn%d width:%d,video height:%d#################\n", chn, video_width, video_height);
		g_i13c_info[chn].video_width = video_width;
		g_i13c_info[chn].video_height = video_height;
		//IPC_SetStreamResolution(chn, video_width, video_height);
	}
	tmp = strstr(buf, "m=audio");
	if(tmp != NULL)
	{
		sscanf(tmp,"%*s %d %*[^/]/%[^/]/%32[^\r]",&audio_format,str[0],str[1]);
		if(audio_format == PT_G726)
		{
			//printf("audio format: G726 \n");
		}
		else if(audio_format == PT_G711)
		{
			//printf("audio format: G711a \n");
		}
	}
	
	return 0;	
}

/* receive response packet of streaming from IPCam */
static int ResponseStreamCommand(PARAM_S* pParam,int chn)
{
	char buf[512];
	int i, ret, n;
	char* cur;
	
	memset(buf, 0, 512);
	
	i = 0;
	n = 0;
	cur = &buf[i];
	
	ret = -1;//csp modify 20131111
	
	while(1)
	{
		//csp modify 20131111
		if(pParam->Socket == -1)
		{
			printf("ResponseStreamCommand socket error\n");
			return -1;
		}
		int max_fd_num = pParam->Socket;
		fd_set set;
		FD_ZERO(&set);
		FD_SET(pParam->Socket,&set);
		//linux平台下timeout会被修改以表示剩余时间,故每次都要重新赋值
		struct timeval timeout;
		timeout.tv_sec = 6;//4;//csp modify for juan
		timeout.tv_usec = 0;
		int rtn = select(max_fd_num+1,&set,NULL,NULL,&timeout);
		if(rtn <= 0)
		{
			printf("ResponseStreamCommand chn%d select error,result:%d,reason:%s\n",chn,rtn,buf);
			return -1;
		}
		
    	ret = recv(pParam->Socket, cur, 1, 0);
    	if(ret <= 0)
    	{
    		printf("ResponseStreamCommand chn%d recv error\n",chn);
    		return -1;
    	}
		
		//csp modify 20131111
		//if((buf[i] == '\n') && (buf[i-1] == '\r') && (i > 1))
		if((i > 1) && (buf[i] == '\n') && (buf[i-1] == '\r'))
		{
    		if(n == 0)
    		{
    			if(strstr(buf, "200 OK") == NULL)
    			{
    				printf("Response error\n");
    				ret = -1;
    				break;
    			}
    			n++;
    		}
    		else if(n == 1)
    		{
    			//csp modify 20131111
    			//if((buf[i-2] == '\n') && (buf[i-3] == '\r'))
    			if((i >= 3) && (buf[i-2] == '\n') && (buf[i-3] == '\r'))
    			{
    				n++;
    			}
    		}
    		else
    		{
    			//csp modify 20131111
    			//if((buf[i-2] == '\n') && (buf[i-3] == '\r'))
				if((i >= 3) && (buf[i-2] == '\n') && (buf[i-3] == '\r'))
    			{
    				ret = 0;
    				//printf("ResponseStreamCommand result:%s\n", buf);
    				GetVideoAndAudioAttr(buf,chn);
    				break;
    			}
    		}
    	}
    	i++;
		//csp modify 20131111
		if(i >= (int)(sizeof(buf)-1))
		{
			printf("Response error\n");
			ret = -1;
			break;
		}
    	cur = &buf[i];
	}
	
	return ret;
}

/* receive streaming data */
static int ReceiveData(int socket, char* buf, int recv_len)
{
	int pack_len = 0;
	int ret = -1;
	char* cur_buf = NULL;
	
	if(buf == NULL)
	{
		return -1;
	}
	
	cur_buf = buf;	
	
	while(pack_len < recv_len)
	{
		ret = recv(socket, cur_buf, recv_len-pack_len, 0);
		if(ret <= 0)
		{
			printf("ReceiveData error,ret=%d,errno=%d,errstr=%s\n",ret,errno,strerror(errno));
			return -1;
		}
		//pack_len = pack_len + ret;
		//cur_buf = cur_buf + pack_len;
		pack_len += ret;
		cur_buf = buf+pack_len;
	}
	
	return 0;
}

/* skip streaming data */
int SkipData(int socket, int skip_len)
{
	int remain_len = skip_len;
	int ret = -1;
	char buffer[4096];
	
	while(remain_len > 0)
	{
		int size = (remain_len < ((int)sizeof(buffer))) ? remain_len : ((int)sizeof(buffer));
		ret = recv(socket, buffer, size, 0);
		if(ret <= 0)
		{
			printf("SkipData error,ret=%d,errno=%d,errstr=%s\n",ret,errno,strerror(errno));
			return -1;
		}
		remain_len -= ret;
	}
	
	return 0;
}
/*------------------- define ---------------*/

int I13_Init(unsigned int max_client_num)
{
	if(max_client_num <= 0)
	{
		return -1;
	}
	
	g_i13_client_count = max_client_num;
	
	g_i13c_info = (i13_client_info *)malloc(g_i13_client_count*sizeof(i13_client_info));
	if(g_i13c_info == NULL)
	{
		return -1;
	}
	memset(g_i13c_info,0,g_i13_client_count*sizeof(i13_client_info));
	
	int i = 0;
	for(i = 0; i < (int)g_i13_client_count; i++)
	{
		g_i13c_info[i].cap_fd = -1;
		g_i13c_info[i].eventLoopWatchVariable = 0;
		
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_i13c_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	
	g_init_flag = 1;
	
	return 0;
}

int I13_DeInit()
{
	return 0;
}

int I13_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	if(pnw == NULL)
	{
		return -1;
	}
	
	if(ipcam->net_mask)
	{
		pnw->ip_address = ipcam->dwIp;
		pnw->net_mask = ipcam->net_mask;
		pnw->net_gateway = ipcam->net_gateway;
		pnw->dns1 = ipcam->dns1;
		pnw->dns2 = ipcam->dns2;
		
		return 0;
	}
	
	char cgi[1024];
	sprintf(cgi, "/web/cgi-bin/hi3510/param.cgi?cmd=getnetattr");
	
	char result[1024];
	memset(result,0,sizeof(result));
	if(ipnc_exec_cgi_cmd(ipcam, cgi, result) != 0)
	{
		printf("I13_GetNetworkParam failed\n");
		return -1;
	}
	
	char *ip = strstr(result, " ip=\"");
	if(ip == NULL)
	{
		printf("ip is null\n");
		return -1;
	}
	
	char *netmask = strstr(result, "netmask=\"");
	if(netmask == NULL)
	{
		printf("netmask is null\n");
		return -1;
	}
	
	char *gateway = strstr(result, "gateway=\"");
	if(gateway == NULL)
	{
		printf("gateway is null\n");
		return -1;
	}
	
	char *fdnsip = strstr(result, "fdnsip=\"");
	if(fdnsip == NULL)
	{
		printf("fdnsip is null\n");
		//return -1;
	}
	
	char *sdnsip = strstr(result, "sdnsip=\"");
	if(sdnsip == NULL)
	{
		printf("sdnsip is null\n");
		//return -1;
	}
	
	ip += strlen(" ip=\"");
	*strstr(ip, "\"") = 0;
	printf("I13 ip : %s\n", ip);
	
	netmask += strlen("netmask=\"");
	*strstr(netmask, "\"") = 0;
	printf("I13 netmask : %s\n", netmask);
	
	gateway += strlen("gateway=\"");
	*strstr(gateway, "\"") = 0;
	printf("I13 gateway : %s\n", gateway);
	
	if(fdnsip != NULL)
	{
		fdnsip += strlen("fdnsip=\"");
		*strstr(fdnsip, "\"") = 0;
		printf("I13 fdnsip : %s\n", fdnsip);
	}
	
	if(sdnsip != NULL)
	{
		sdnsip += strlen("sdnsip=\"");
		*strstr(sdnsip, "\"") = 0;
		printf("I13 sdnsip : %s\n", sdnsip);
	}
	
	pnw->ip_address = inet_addr(ip);
	pnw->net_mask = inet_addr(netmask);
	pnw->net_gateway = inet_addr(gateway);
	if(fdnsip != NULL)
	{
		pnw->dns1 = inet_addr(fdnsip);
	}
	else
	{
		pnw->dns1 = 0;
	}
	if(sdnsip != NULL)
	{
		pnw->dns2 = inet_addr(sdnsip);
	}
	else
	{
		pnw->dns2 = 0;
	}
	
	return 0;
}

int I13_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	if(pnw == NULL)
	{
		return -1;
	}
	
	struct sockaddr_in local, remote, from;
	
	int sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sock == -1)
	{
		printf("Set IPNC:socket error\n");
		return -1;
	}
	
	int reuseflag = 1;
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *)&reuseflag,sizeof(reuseflag)) == SOCKET_ERROR) 
	{
		printf("Set IPNC:port mulit error\n");
		close(sock);
		return -1;
	}
	
	unsigned int dwInterface = INADDR_ANY;
	unsigned int dwMulticastGroup = inet_addr(MCASTADDR);
	unsigned short iPort = MCASTPORT;
	
	local.sin_family = AF_INET;
	local.sin_port   = htons(iPort);
	local.sin_addr.s_addr = dwInterface;
	
	if(bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("Set IPNC:bind error\n");
		close(sock);
		return -1;
	}
	
	struct ip_mreq mreq;
	memset(&mreq,0,sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(MCASTADDR);
	mreq.imr_interface.s_addr = GetLocalIp();
	if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char*)&mreq,sizeof(mreq)) == SOCKET_ERROR)
	{
		printf("Set IPNC:join mulit error\n");
		close(sock);
		return -1;
	}
	
	//csp modify for klw
	int optval = 0;
	if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		printf("setsockopt(IP_MULTICAST_LOOP) failed\n");
		//close(sock);
		//return 0;
	}
	
	remote.sin_family      = AF_INET;
	remote.sin_port        = htons(iPort);
	remote.sin_addr.s_addr = dwMulticastGroup;
	
	char recvbuf[BUFSIZE], sendbuf[BUFSIZE];
	
	char ip[20];
	char netmask[20];
	char gateway[20];
	char fdnsip[20];
	char sdnsip[20];
	
	struct in_addr host;
	
	host.s_addr = pnw->ip_address;
	strcpy(ip, inet_ntoa(host));
	
	host.s_addr = pnw->net_mask;
	strcpy(netmask, inet_ntoa(host));
	
	host.s_addr = pnw->net_gateway;
	strcpy(gateway, inet_ntoa(host));
	
	host.s_addr = pnw->dns1;
	strcpy(fdnsip, inet_ntoa(host));
	
	host.s_addr = pnw->dns2;
	strcpy(sdnsip, inet_ntoa(host));
	
	char cmd[320];
	//sprintf(cmd,COMMAND,ip,netmask,gateway,fdnsip,sdnsip);
	sprintf(cmd,COMMAND,ip,netmask,gateway,fdnsip);
	
	printf("设置新的IP地址\n");
	snprintf(sendbuf, BUFSIZE, "CMD * HDS/1.0\r\n"
								"CSeq:3\r\n"
								"Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC\r\n"
								"Accept-Type:text/HDP\r\n"
								"Authorization:Basic %s:%s\r\n"
								"Device-ID:%s\r\n"
								"Content-Length:%d\r\n"
								"\r\n%s", 
								ipcam->user, ipcam->pwd, ipcam->uuid, strlen(cmd), cmd);
	//printf("hello:%s\n", sendbuf);
	
	if(sendto(sock, (char *)sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&remote, sizeof(remote)) == SOCKET_ERROR)
	{
		printf("Set IPNC:sendto failed with: %d\n", errno);
		close(sock);
		return -1;
	}
	
	char ack[256];
	sprintf(ack,"Device-ID:%s",ipcam->uuid);
	
	unsigned char byAckFlag = 0;
	
	int i;
	for(i = 0; i < 100; i++)
	{
		fd_set r;
		struct timeval t;
		
		t.tv_sec = 0;
		t.tv_usec = 4*1000;//100;//csp modify for klw
		
		FD_ZERO(&r);
		FD_SET(sock, &r);
		
		int len = sizeof(struct sockaddr_in);
		
		int ret = select(sock + 1, &r, NULL, NULL, &t);
		if(ret > 0)
		{
			memset(recvbuf, 0, sizeof(recvbuf));//csp modify for klw
			
			ret = recvfrom(sock, recvbuf, BUFSIZE, 0, (struct sockaddr *)&from, (socklen_t *)&len);
			if(ret < 0)
			{
				usleep(1);
				continue;
			}
			
			recvbuf[ret] = 0;
			
			if((strstr(recvbuf, "MCTP/1.0 200 OK") && strstr(recvbuf, "Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC") && strstr(recvbuf, ack)) || 
				(strstr(recvbuf, "HDS/1.0 200 OK") && strstr(recvbuf, "Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC") && strstr(recvbuf, ack)))//csp modify for klw
			{
				printf("i13 set netrork param ok\n");
				byAckFlag = 1;
				break;
			}
			else
			{
				printf("I13_SetNetworkParam recv error response from(%s,%d),msg:%s\n",inet_ntoa(from.sin_addr),ntohs(from.sin_port),recvbuf);
			}
		}
	}
	
	close(sock);
	
	if(!byAckFlag)
	{
		printf("I13_SetNetworkParam no ack\n");
		
		//csp modify for juan
		if(ipcam->protocol_type == PRO_TYPE_JUAN)
		{
			return 0;
		}
		
		return -1;
	}
	
	return 0;
}

extern int DoStreamStateCallBack(int chn, real_stream_state_e msg);

void* ThreadPROCI13(void* pParam)
{
	unsigned int time_out_count = 0;
	
	char *enc_buf = NULL;
	int enc_len = 0;
	
	int chn = (int)pParam;
	if(chn < 0 || chn >= (int)g_i13_client_count)
	{
		goto ProcQuit;
	}
	
	if(chn < (int)(g_i13_client_count/2))
	{
		enc_len = 500*1024;//400*1024;
	}
	else
	{
		enc_len = 200*1024;
	}
	enc_buf = (char *)malloc(enc_len);
	if(enc_buf == NULL)
	{
		goto ProcOver;
	}
	
	while(g_init_flag)
	{
		if(g_i13c_info[chn].eventLoopWatchVariable)
		{
			break;
		}
		
		int max_fd_num = g_i13c_info[chn].cap_fd;
		
		fd_set set;
		FD_ZERO(&set);
		FD_SET(g_i13c_info[chn].cap_fd,&set);
		
		//linux平台下timeout会被修改以表示剩余时间,故每次都要重新赋值
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		int ret = select(max_fd_num+1,&set,NULL,NULL,&timeout);
		if(ret == 0)
		{
			//csp modify 20131111
			//if(++time_out_count > 10)
			if(++time_out_count > 5)
			{
				printf("ThreadPROCI13:chn%d select sock timeout & quit!!!\n",chn);
				time_out_count = 0;
				break;
			}
			else
			{
				printf("ThreadPROCI13:chn%d select sock timeout<%d> & continue!\n",chn,time_out_count);
				continue;
			}
		}
		if(ret < 0)
		{
			printf("ThreadPROCI13:chn%d select sock error:[%s]!\n",chn,strerror(errno));
			break;
		}
		if(g_i13c_info[chn].cap_fd != INVALID_SOCKET && FD_ISSET(g_i13c_info[chn].cap_fd,&set))
		{
			time_out_count = 0;
			
			RTSP_ITLEAVED_HDR_S rtsp_header;
			ret = ReceiveData(g_i13c_info[chn].cap_fd, (char*)&rtsp_header, sizeof(rtsp_header));
			if(ret != 0)
			{
				printf("chn%d recv header error\n",chn);
				break;
			}
			
			unsigned int packet_len = ntohl(rtsp_header.payloadLen) - sizeof(RTP_HDR_S);
			unsigned int timestamp = ntohl(rtsp_header.rtpHead.ts);
			
			if(packet_len > (unsigned int)enc_len)
			{
				printf("chn%d skip data:framelen(%d) > bufferlen(%d)\n",chn,packet_len,enc_len);
				ret = SkipData(g_i13c_info[chn].cap_fd, packet_len);
				if(ret != 0)
				{
					printf("chn%d skip data error\n",chn);
					break;
				}
				else
				{
					printf("chn%d skip data succ\n",chn);
					continue;
				}
			}
			
			char *recv_buf = (char*)enc_buf;
			ret = ReceiveData(g_i13c_info[chn].cap_fd, (char*)recv_buf, packet_len);
			if(ret != 0)
			{
				printf("chn%d recv data error\n",chn);
				break;
			}
			
			//printf("chn%d recv data type:<%d> data len:<%d>\n",chn,rtsp_header.rtpHead.pt,packet_len);
			
			if(rtsp_header.rtpHead.pt == PT_DATA)
			{
				continue;
			}
			
			if(rtsp_header.rtpHead.pt == PT_G711 || rtsp_header.rtpHead.pt == PT_G726)
			{
				if(g_i13c_info[chn].pStreamCB != NULL)
				{
					real_stream_s stream;
					memset(&stream, 0, sizeof(stream));
					stream.chn = chn;
					stream.data = (unsigned char *)recv_buf + 4;
					stream.len = packet_len - 4;
					stream.pts = timestamp;
					stream.pts *= 1000;
					stream.media_type = (rtsp_header.rtpHead.pt == PT_G711) ? MEDIA_PT_G711 : MEDIA_PT_G726;
					g_i13c_info[chn].pStreamCB(&stream, g_i13c_info[chn].dwContext);
				}
				
				continue;
			}
			
			if(rtsp_header.rtpHead.pt != PT_H264)
			{
				//printf("recv data NOT PT_H264:<%d>\n",rtsp_header.rtpHead.pt);
				continue;
			}
			
			if(g_i13c_info[chn].pStreamCB != NULL)
			{
				//csp modify 20140423
				if(timestamp != g_i13c_info[chn].last_pts)
				{
					g_i13c_info[chn].frame_count++;
					if((g_i13c_info[chn].frame_count % 250) == 0)
					{
						//printf("i13 chn%d span %lds\n",chn,time(NULL)-g_i13c_info[chn].last_t);
						g_i13c_info[chn].last_t = time(NULL);
					}
					g_i13c_info[chn].last_pts = timestamp;
				}
				
				real_stream_s stream;
				//memset(&stream, 0, sizeof(stream));
				stream.chn = chn;
				stream.data = (unsigned char *)recv_buf;
				stream.len = packet_len;
				stream.pts = timestamp;
				stream.pts *= 1000;
				stream.media_type = MEDIA_PT_H264;
				if((recv_buf[4] & 0x1f) == 0x07)
				{
					stream.frame_type = REAL_FRAME_TYPE_I;
					//if(chn == 0)
					//{
					//	printf("chn%d frame type:I\n",chn);
					//}
				}
				else
				{
					stream.frame_type = REAL_FRAME_TYPE_P;
					//if(chn == 0)
					//{
					//	printf("chn%d frame type:P\n",chn);
					//}
				}
				stream.rsv = 0;
				stream.mdevent = 0;
				stream.width = g_i13c_info[chn].video_width;
				stream.height = g_i13c_info[chn].video_height;
				g_i13c_info[chn].pStreamCB(&stream, g_i13c_info[chn].dwContext);
			}
		}
		else
		{
			if(++time_out_count > 10)
			{
				printf("ThreadPROCI13:chn%d unknown sock error & quit!!!\n",chn);
				time_out_count = 0;
				break;
			}
		}
	}
	
	pthread_mutex_lock(&g_i13c_info[chn].lock);
	
	g_i13c_info[chn].pid = 0;
	g_i13c_info[chn].eventLoopWatchVariable = 0;
	
	if(g_i13c_info[chn].cap_fd != -1)
	{
		close(g_i13c_info[chn].cap_fd);
		g_i13c_info[chn].cap_fd = -1;
	}
	
	pthread_mutex_unlock(&g_i13c_info[chn].lock);
	
	free(enc_buf);
	enc_buf = NULL;
	
ProcOver:
	DoStreamStateCallBack(chn, REAL_STREAM_STATE_LOST);
ProcQuit:
	pthread_detach(pthread_self());
	return 0;
}

int I13_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_i13_client_count)
	{
		return -1;
	}
	
	//printf("I13_Start - 1\n");
	
	if(I13_GetLinkStatus(chn))
	{
		I13_Stop(chn);
	}
	
	//printf("I13_Start - 2\n");
	
	//csp modify 20140613
	unsigned char juan_ipc = 0;
	if(streamInfo)
	{
		if(strcmp(streamInfo, "JUAN") == 0)
		{
			juan_ipc = 1;
		}
	}
	
	PARAM_S params;
	memset(&params, 0, sizeof(PARAM_S));
	
	params.Buf = NULL;
	params.State = 1;
	params.Socket = -1;
	
	strcpy(params.Uname, user);
	strcpy(params.Pword, pwd);
	
	struct in_addr serv;
	serv.s_addr = dwIp;
	strcpy(params.Ip,inet_ntoa(serv));
	params.Port = wPort;
	
	PARAM_S* pParam = (PARAM_S*)&params;
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pParam->Port);
	addr.sin_addr.s_addr = inet_addr(pParam->Ip);
	if(addr.sin_addr.s_addr == 0 || params.Port == 0 || addr.sin_addr.s_addr == (unsigned int)(-1))
	{
		//printf("ip:port error\n");
		return -1;
	}
	
	//printf("I13_Start - 3\n");
	
	#if 1
	u16 wErrorCode = 0;
	pParam->Socket = ConnectWithTimeout(inet_addr(pParam->Ip), pParam->Port, 5000, &wErrorCode);
	if(pParam->Socket < 0)
	{
		printf("chn%d connect error:(%s,%d,0x%08x,0x%08x),err:(%d,%s)\n",
			chn,pParam->Ip,pParam->Port,addr.sin_addr.s_addr,inet_addr(pParam->Ip),errno,strerror(errno));
		return -1;
	}
	int ret = 0;
	#else
	/* create TCP/IP socket and connect IPCam */
	if((pParam->Socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket error\n");
		return -1;
	}
	
	//printf("I13_Start - 3.1, connecting...\n");
	
	int ret = connect(pParam->Socket, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0)
	{
		printf("chn%d connect error:(%s,%d,0x%08x,0x%08x),err:(%d,%s)\n",
			chn,pParam->Ip,pParam->Port,addr.sin_addr.s_addr,inet_addr(pParam->Ip),errno,strerror(errno));
		close(pParam->Socket);
		pParam->Socket = -1;
		return -1;
	}
	#endif
	
	//printf("I13_Start - 4\n");
	
	if(chn < (int)(g_i13_client_count/2))
	{
		//csp modify 20140613
		if(juan_ipc)
		{
			//九安的音频数据量太大了，会有问题
			ret = RequestStreamCommand(pParam, CHANNEL1, (char *)STREAM_TYPE5);
		}
		else
		{
			ret = RequestStreamCommand(pParam, CHANNEL1, (char *)STREAM_TYPE1);
		}
	}
	else
	{
		ret = RequestStreamCommand(pParam, CHANNEL2, (char *)STREAM_TYPE5);
	}
	if(ret < 0)
	{
		printf("chn%d stream request cmd error\n",chn);
		close(pParam->Socket);
		pParam->Socket = -1;
		return -1;
	}
	
	//printf("I13_Start - 5\n");
	
	/* receive response packet of streaming from IPCam */
	ret = ResponseStreamCommand(pParam,chn);
	if(ret < 0)
	{
		printf("chn%d stream recv cmd error\n",chn);
		close(pParam->Socket);
		pParam->Socket = -1;
		return -1;
	}
	
	//printf("I13_Start - 6\n");
	
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	setsockopt(pParam->Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	
	pthread_mutex_lock(&g_i13c_info[chn].lock);
	
	g_i13c_info[chn].cap_fd = pParam->Socket;
	g_i13c_info[chn].dwIp = dwIp;
	g_i13c_info[chn].wPort = wPort;
	strcpy(g_i13c_info[chn].username, user);
	strcpy(g_i13c_info[chn].password, pwd);
	if(chn < (int)(g_i13_client_count/2))
	{
		g_i13c_info[chn].channel_info = CHANNEL1;
		
		//csp modify 20140613
		if(juan_ipc)
		{
			strcpy(g_i13c_info[chn].stream_type, STREAM_TYPE5);
		}
		else
		{
			strcpy(g_i13c_info[chn].stream_type, STREAM_TYPE1);
		}
	}
	else
	{
		g_i13c_info[chn].channel_info = CHANNEL2;
		strcpy(g_i13c_info[chn].stream_type, STREAM_TYPE5);
	}
	g_i13c_info[chn].pStreamCB = pCB;
	g_i13c_info[chn].dwContext = dwContext;
	g_i13c_info[chn].eventLoopWatchVariable = 0;
	
	//printf("I13_Start - 7\n");
	
	if(pthread_create(&g_i13c_info[chn].pid, NULL, ThreadPROCI13, (void *)chn) != 0)
	{
		g_i13c_info[chn].pid = 0;
		I13_Stop(chn);
		
		//csp modify 20140425
		pthread_mutex_unlock(&g_i13c_info[chn].lock);
		return -1;
	}
	
	//printf("I13_Start - 8\n");
	
	pthread_mutex_unlock(&g_i13c_info[chn].lock);
	
	return 0;
}

int I13_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	return -1;
}

int I13_Stop(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_i13_client_count)
	{
		return -1;
	}
	
	if(g_i13c_info[chn].pid != 0)
	{
		//printf("I13_Stop::Stop-1\n");
		
		g_i13c_info[chn].eventLoopWatchVariable = 1;
		
		//printf("I13_Stop::Stop-2\n");
		
		pthread_join(g_i13c_info[chn].pid, NULL);
		
		//printf("I13_Stop::Stop-3\n");
	}
	
	pthread_mutex_lock(&g_i13c_info[chn].lock);
	
	g_i13c_info[chn].pid = 0;
	g_i13c_info[chn].eventLoopWatchVariable = 0;
	
	if(g_i13c_info[chn].cap_fd != -1)
	{
		close(g_i13c_info[chn].cap_fd);
		g_i13c_info[chn].cap_fd = -1;
	}
	
	pthread_mutex_unlock(&g_i13c_info[chn].lock);
	
	return 0;
}

//return value : 1 - Link; 0 - Lost
int I13_GetLinkStatus(int chn)
{
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)g_i13_client_count)
	{
		return 0;
	}
	
	int status = 0;
	
	pthread_mutex_lock(&g_i13c_info[chn].lock);
	
	status = (g_i13c_info[chn].cap_fd != -1);
	
	pthread_mutex_unlock(&g_i13c_info[chn].lock);
	
	return status;
}

static void base64Encode(char *intext, char *output)
{
	char table64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/@";
	unsigned char ibuf[3];
	unsigned char obuf[4];
	int i;
	int inputparts;
	
	while(*intext) {
		for (i = inputparts = 0; i < 3; i++) { 
			if(*intext) {
				inputparts++;
				ibuf[i] = *intext;
				intext++;
			}
			else
				ibuf[i] = 0;
		}
		
		obuf [0] = (ibuf [0] & 0xFC) >> 2;
		obuf [1] = ((ibuf [0] & 0x03) << 4) | ((ibuf [1] & 0xF0) >> 4);
		obuf [2] = ((ibuf [1] & 0x0F) << 2) | ((ibuf [2] & 0xC0) >> 6);
		obuf [3] = ibuf [2] & 0x3F;
		
		switch(inputparts) {
		case 1: /* only one byte read */
			sprintf(output, "%c%c==", 
				table64[obuf[0]],
				table64[obuf[1]]);
			break;
		case 2: /* two bytes read */
			sprintf(output, "%c%c%c=", 
				table64[obuf[0]],
				table64[obuf[1]],
				table64[obuf[2]]);
			break;
		default:
			sprintf(output, "%c%c%c%c", 
				table64[obuf[0]],
				table64[obuf[1]],
				table64[obuf[2]],
				table64[obuf[3]] );
			break;
		}
		output += 4;
	}
	*output=0;
}

int open_cmd_link(ipc_unit *ipcam)
{
	if(ipcam != NULL)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(ipcam->wPort);
		addr.sin_addr.s_addr = ipcam->dwIp;
		if(addr.sin_addr.s_addr == 0 || ipcam->wPort == 0 || addr.sin_addr.s_addr == (unsigned int)(-1))
		{
			printf("ip:port error\n");
			return -1;
		}
		
		//printf("open_cmd_link:connecting...\n");
		
		#if 1
		u16 wErrorCode = 0;
		int sock = ConnectWithTimeout(ipcam->dwIp, ipcam->wPort, 5000, &wErrorCode);
		if(sock < 0)
		{
			printf("open_cmd_link:Connect Error!\n");
			return -1;
		}
		#else
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if(sock < 0)
		{
			perror("Socket Error!");
			return -1;
		}
		
		int ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
		if(ret < 0)
		{
			perror("open_cmd_link:Connect Error!");
			close(sock);
			return -1;
		}
		#endif
		
		//printf("open_cmd_link:connect success\n");
		
		//csp modify 20131111
		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
		
		return sock;
	}
	
	return -1;
}

int close_cmd_link(int sock)
{
	if(sock != -1)
	{
		close(sock);
		sock = -1;
	}
	
	return 0;
}

static int send_loop(int fd, unsigned char *buffer, unsigned int len)
{
	unsigned int remain = len;
	unsigned int sendlen = 0;
	while(remain > 0)
	{
		int ret = send(fd, (char *)(buffer+sendlen), remain, NETSNDRCVFLAG);
		if(ret <= 0)
		{
			printf("send_loop failed:(%d,%s)\n",errno,strerror(errno));
			printf("send_loop failed,ret=%d,remain=%d,sendlen=%d,len=%d\n",ret,remain,sendlen,len);
			//fflush(stdout);
			return -1;
		}
		remain -= ret;
		sendlen += ret;
	}
	return len;
}

static HI_S32 SendData(SOCKET sock, HI_U8* pu8Buff, HI_U32 u32DataLen)
{
	if(send_loop(sock, pu8Buff, u32DataLen) <= 0)
	{
		printf("SendData failed\n");
		return HI_FAILURE;
	}
	
	return HI_SUCCESS;
}

HI_S32 ReadCmdInfo(SOCKET Sock, HI_CHAR* Ipaddr, HI_CHAR* Cgi, HI_CHAR* Buf, HI_S32* Buf_len, HI_CHAR* Auth)
{
	HI_S32 s32Ret;
	HI_S32 len = 0;
	struct timeval struTimeout;
	fd_set struReadSet;
	
	memset(Buf, 0, *Buf_len);
	sprintf(Buf, "GET %s HTTP/1.1\r\n", Cgi);
	strcpy(Buf+strlen(Buf), "Accept: */*\r\n");
	strcpy(Buf+strlen(Buf), "Accept-Language: zh-cn\r\n");
	strcpy(Buf+strlen(Buf), "Accept-Encoding: gzip, deflate\r\n");
	strcpy(Buf+strlen(Buf), "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727)\r\n");
	sprintf(Buf+strlen(Buf), "Host: %s\r\n", Ipaddr);
	strcpy(Buf+strlen(Buf), "Connection: Keep-Alive\r\n");
	sprintf(Buf+strlen(Buf), "Authorization: Basic %s\r\n", Auth);
	strcpy(Buf+strlen(Buf), "\r\n");
	len = strlen(Buf);
	
	//len += 1;//csp add
	
	//if(strstr(Cgi,"requestifrm")) printf("%s\n", Buf);
	
	s32Ret = SendData(Sock, (HI_U8 *)Buf, len);
	if(HI_SUCCESS != s32Ret)
	{
		printf("Send Error!\n");
       	return HI_FAILURE;
	}
	
	//如果CGI没有返回值，下面代码就不用，直接返回 HI_SUCCESS
	memset(Buf, 0, *Buf_len);
	len = 0;
	
	do
	{
		struTimeout.tv_sec = TIMEOUT/1000;
		struTimeout.tv_usec = TIMEOUT%1000;
		
		FD_ZERO(&struReadSet);
		FD_SET((HI_U32)Sock, &struReadSet);
		
		s32Ret = select(Sock + 1, &struReadSet, NULL, NULL, &struTimeout);
		if(s32Ret <= 0)
		{
			printf("select Error!\n");
			return HI_FAILURE;
		}
		
		//memset(Buf, 0, *Buf_len);
		len = recv(Sock, Buf, *Buf_len, 0);
		if(len <= 0)
		{
			printf("Recv Error:%d!\n",len);
			return HI_FAILURE;
		}
		
		//Buf[len] = 0;
	}while(0);
	
	//*Buf_len = len;
	
	//if(strstr(Cgi,"requestifrm")) printf("[[[ReadCmdInfo=%s]]]\n", Buf);
	
	return HI_SUCCESS;
}

int ipnc_do_cgi_cmd(ipc_unit *ipcam, char* cgi)
{
	int cmd_fd = open_cmd_link(ipcam);
	
	if(cmd_fd != -1)
	{
		char sAuth[128] = {0};
		char sTemp[128] = {0};
		
		sprintf(sTemp, "%s:%s", ipcam->user, ipcam->pwd);
		base64Encode(sTemp, sAuth);
		
		HI_CHAR sBuf[BUF_LEN] = {0};
		HI_S32 s32Len = BUF_LEN;
		
		struct in_addr host;
		host.s_addr = ipcam->dwIp;
		char ip[20];
		strcpy(ip,inet_ntoa(host));
		
		if(ReadCmdInfo(cmd_fd, ip, cgi, sBuf, &s32Len, sAuth) == HI_FAILURE)
		{
			printf("ReadCmdInfo Error!\n");
			close_cmd_link(cmd_fd);
			return -1;
		}
		
		close_cmd_link(cmd_fd);
		return 0;
	}
	
	return -1;
}

int ipnc_exec_cgi_cmd(ipc_unit *ipcam, char* cgi, char *result)
{
	int cmd_fd = open_cmd_link(ipcam);
	
	if(cmd_fd != -1)
	{
		char sAuth[128] = {0};
		char sTemp[128] = {0};
		
		sprintf(sTemp, "%s:%s", ipcam->user, ipcam->pwd);
		base64Encode(sTemp, sAuth);
		
		HI_CHAR sBuf[BUF_LEN] = {0};
		HI_S32 s32Len = BUF_LEN;
		
		struct in_addr host;
		host.s_addr = ipcam->dwIp;
		char ip[20];
		strcpy(ip,inet_ntoa(host));
		
		if(ReadCmdInfo(cmd_fd, ip, cgi, sBuf, &s32Len, sAuth) == HI_FAILURE)
		{
			printf("ReadCmdInfo Error!\n");
			close_cmd_link(cmd_fd);
			return -1;
		}
		
		strcpy(result,sBuf);
		
		close_cmd_link(cmd_fd);
		return 0;
	}
	
	return -1;
}

int ipnc_get_devtype(ipc_unit *ipcam,char *devtype)
{
	char cgi[1024];
	sprintf(cgi,"/cgi-bin/hi3510/param.cgi?cmd=getdevtype");
	
	char result[1024];
	memset(result,0,sizeof(result));
	
	if(ipnc_exec_cgi_cmd(ipcam,cgi,result) != 0)
	{
		printf("ipnc_get_devtype failed\n");
		return -1;
	}
	
	//printf("ipnc_get_devtype : %s\n",result);
	
	if(devtype)
	{
		char *p = strstr(result, "devtype=\"");
		if(p == NULL)
		{
			printf("devtype is null\n");
			return -1;
		}
		
		p += strlen("devtype=\"");
		*strstr(p, "\"") = 0;
		
		strcpy(devtype, p);
		printf("devtype : %s\n", devtype);
	}
	
	return 0;
}

int I13_CMD_Open(int chn)
{
	return 0;
}

int I13_CMD_Close(int chn)
{
	return 0;
}

int I13_CMD_SetImageParam(int chn, video_image_para_t *para)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	char devtype[32];
	if(ipnc_get_devtype(&ipcam, devtype) < 0)
	{
		return -1;
	}
	
	//720P相机不支持色度设置
	//brightness:[0,255]
	//contrast:[0,7]
	//saturation:[0,255]
	int brightness = para->brightness;
	int contrast = para->contrast/32;
	int saturation = para->saturation;
	
#if 1
	brightness = para->brightness;
	contrast = para->contrast;
	saturation = para->saturation;
	if(strstr(devtype,"S7"))
	{
		contrast = para->contrast/32;//[1,7]
		if(contrast == 0)
		{
			contrast = 1;
		}
	}
	else if(strstr(devtype,"S8"))
	{
		brightness = para->brightness/42;//[0,6]
		contrast = para->contrast/31;//[0,8]
		saturation = para->saturation/42;//[0,6]
	}
	else if(strstr(devtype,"S9"))
	{
		contrast = para->contrast/32;//[1,7]
		if(contrast == 0)
		{
			contrast = 1;
		}
	}
	else// if(strstr(devtype,"Sc"))
	{
		brightness = para->brightness*100/255;//[1,100]
		contrast = para->contrast*100/255;//[1,100]
		saturation = para->saturation*100/255;//[1,100]
		if(brightness == 0)
		{
			brightness = 1;
		}
		if(contrast == 0)
		{
			contrast = 1;
		}
		if(saturation == 0)
		{
			saturation = 1;
		}
	}
#endif
	
	//setvdisplayattr
	char cgi[1024];
	if(para->brightness == 128 && para->contrast == 128 && para->saturation == 128)
	{
		sprintf(cgi,
			"/web/cgi-bin/hi3510/param.cgi?cmd=setimageattr&-default=on");
	}
	else
	{
		if(strstr(devtype,"S7") || strstr(devtype,"S8") || strstr(devtype,"S9"))
		{
			//sprintf(cgi,
			//	"/cgi-bin/hi3510/param.cgi?cmd=setvdisplayattr&-brightness=%d&-saturation=%d&-contrast=%d",
			//	brightness,
			//	saturation,
			//	contrast);
			sprintf(cgi,
				"/web/cgi-bin/hi3510/param.cgi?cmd=setimageattr&-brightness=%d&-saturation=%d&-contrast=%d",
				brightness,
				saturation,
				contrast);
		}
		else
		{
			//URI: /web/cgi-bin/hi3510/param.cgi?cmd=setimageattr&-brightness=87
			//URI: /web/cgi-bin/hi3510/param.cgi?cmd=setimageattr&-default=on
			sprintf(cgi,
				"/web/cgi-bin/hi3510/param.cgi?cmd=setimageattr&-brightness=%d&-saturation=%d&-contrast=%d",
				brightness,
				saturation,
				contrast);
		}
	}
	
	return -1;//???
	
	printf("I13_CMD_SetImageParam cgi=%s\n",cgi);
	
	int rtn = ipnc_do_cgi_cmd(&ipcam,cgi);
	
	return rtn;
}

int I13_CMD_GetImageParam(int chn, video_image_para_t *para)
{
	return -1;
}

int I13_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	char cgi[384];
	char cgi2[384];
	memset(cgi,0,sizeof(cgi));
	memset(cgi2,0,sizeof(cgi2));
	
	if(cmd == EM_PTZ_CMD_START_TILEUP)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=up&-speed=%d",tiltSpeed);
		//printf("I13_CMD_PtzCtrl TILEUP cgi:%s\n",cgi);
	}
	else if(cmd == EM_PTZ_CMD_START_TILEDOWN)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=down&-speed=%d",tiltSpeed);
		//printf("I13_CMD_PtzCtrl TILEDOWN cgi:%s\n",cgi);
	}
	else if(cmd == EM_PTZ_CMD_START_PANLEFT)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=left&-speed=%d",panSpeed);
		//printf("I13_CMD_PtzCtrl PANLEFT cgi:%s\n",cgi);
	}
	else if(cmd == EM_PTZ_CMD_START_PANRIGHT)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=right&-speed=%d",panSpeed);
		//printf("I13_CMD_PtzCtrl PANRIGHT cgi:%s\n",cgi);
	}
	else if(cmd == EM_PTZ_CMD_START_LEFTUP)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=left&-speed=%d",panSpeed);
		unsigned int tiltSpeed = *((unsigned int *)data);
		sprintf(cgi2,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=up&-speed=%d",tiltSpeed);
	}
	else if(cmd == EM_PTZ_CMD_START_LEFTDOWN)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=left&-speed=%d",panSpeed);
		unsigned int tiltSpeed = *((unsigned int *)data);
		sprintf(cgi2,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=down&-speed=%d",tiltSpeed);
	}
	else if(cmd == EM_PTZ_CMD_START_RIGHTUP)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=right&-speed=%d",panSpeed);
		unsigned int tiltSpeed = *((unsigned int *)data);
		sprintf(cgi2,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=up&-speed=%d",tiltSpeed);
	}
	else if(cmd == EM_PTZ_CMD_START_RIGHTDOWN)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=right&-speed=%d",panSpeed);
		unsigned int tiltSpeed = *((unsigned int *)data);
		sprintf(cgi2,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=down&-speed=%d",tiltSpeed);
	}
	else if(cmd == EM_PTZ_CMD_START_ZOOMTELE)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=zoomin&-speed=%d",34);
	}
	else if(cmd == EM_PTZ_CMD_START_ZOOMWIDE)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=zoomout&-speed=%d",34);
	}
	else if(cmd == EM_PTZ_CMD_START_FOCUSNEAR)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=focusin&-speed=%d",34);
	}
	else if(cmd == EM_PTZ_CMD_START_FOCUSFAR)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=focusout&-speed=%d",34);
	}
	else if(cmd == EM_PTZ_CMD_START_IRISSMALL)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=aperturein&-speed=%d",34);
	}
	else if(cmd == EM_PTZ_CMD_START_IRISLARGE)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=apertureout&-speed=%d",34);
	}
	else if(cmd >= EM_PTZ_CMD_STOP_TILEUP && cmd < EM_PTZ_CMD_PRESET_SET)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=stop&-speed=%d",0);
	}
	else if(cmd == EM_PTZ_CMD_PRESET_SET)
	{
		unsigned int preset = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/preset.cgi?-act=set&-status=1&-number=%d",preset);
	}
	else if(cmd == EM_PTZ_CMD_PRESET_GOTO)
	{
		unsigned int preset = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/preset.cgi?-act=goto&-number=%d",preset);
	}
	else if(cmd == EM_PTZ_CMD_PRESET_CLEAR)
	{
		unsigned int preset = *((unsigned int *)data);
		sprintf(cgi,"/cgi-bin/hi3510/preset.cgi?-act=set&-status=0&-number=%d",preset);
	}
	else if(cmd == EM_PTZ_CMD_LIGHT_ON)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=light&-switch=%d",1);
	}
	else if(cmd == EM_PTZ_CMD_LIGHT_OFF)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=light&-switch=%d",0);
	}
	else if(cmd == EM_PTZ_CMD_AUX_ON)
	{
		unsigned int id = *((unsigned int *)data);
		if(id == 1)//默认1号辅助为雨刷
		{
			sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=brush&-switch=%d",1);
		}
	}
	else if(cmd == EM_PTZ_CMD_AUX_OFF)
	{
		unsigned int id = *((unsigned int *)data);
		if(id == 1)//默认1号辅助为雨刷
		{
			sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=brush&-switch=%d",0);
		}
	}
	else if(cmd == EM_PTZ_CMD_AUTOPAN_ON)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=auto");
	}
	else if(cmd == EM_PTZ_CMD_AUTOPAN_OFF)
	{
		sprintf(cgi,"/cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=stop&-speed=%d",0);
	}
	
	if(strlen(cgi))
	{
		int rtn = ipnc_do_cgi_cmd(&ipcam,cgi);
		if(rtn)
		{
			return -1;
		}
		
		if(strlen(cgi2))
		{
			rtn = ipnc_do_cgi_cmd(&ipcam,cgi2);
			if(rtn)
			{
				return -1;
			}
		}
	}
	
	return 0;
}

static const char* szTimeZoneInfo[] = 
{
	"Etc/GMT-12",//"GMT-12:00",
	"Pacific/Apia",//"GMT-11:00",
	"Pacific/Honolulu",//"GMT-10:00",
	"America/Anchorage",//"GMT-09:00",
	"America/Los_Angeles",//"GMT-08:00",
	"America/Phoenix",//"GMT-07:00",
	"America/Chicago",//"GMT-06:00",
	"America/New_York",//"GMT-05:00",
	"America/Caracas",//"GMT-04:30",//加拉加斯（委内瑞拉首都）
	"America/Santiago",//"GMT-04:00",
	"America/St_Johns",//"GMT-03:30",//纽芬兰
	"America/Buenos_Aires",//"GMT-03:00",
	"Atlantic/South_Georgia",//"GMT-02:00",
	"Atlantic/Cape_Verde",//"GMT-01:00",
	"Europe/Dublin",//"GMT",//"GMT+00:00",
	"Europe/Amsterdam",//"GMT+01:00",
	"Europe/Athens",//"GMT+02:00",
	"Asia/Baghdad",//"GMT+03:00",
	"Asia/Tehran",//"GMT+03:30",//伊朗首都德黑兰
	"Asia/Dubai",//"GMT+04:00",
	"Asia/Kabul",//"GMT+04:30",//喀布尔
	"Asia/Karachi",//"GMT+05:00",
	"Asia/Calcutta",//"GMT+05:30",//马德拉斯，加尔各答，孟买，新德里
	"Asia/Katmandu",//"GMT+05:45",//加德满都
	"Asia/Almaty",//"GMT+06:00",
	"Asia/Rangoon",//"GMT+06:30",//仰光
	"Asia/Bangkok",//"GMT+07:00",
	"Asia/Hong_Kong",//"GMT+08:00",
	"Asia/Tokyo",//"GMT+09:00",
	"Australia/Adelaide",//"GMT+09:30",//阿德莱德
	"Australia/Brisbane",//"GMT+10:00",
	"Asia/Magadan",//"GMT+11:00",
	"Pacific/Auckland",//"GMT+12:00",
	"Pacific/Tongatapu",//"GMT+13:00",
};

static const char* GetTZInfo(int index)
{
	if(index < 0 || index >= (int)(sizeof(szTimeZoneInfo)/sizeof(szTimeZoneInfo[0])))
	{
		return "Asia/Hong_Kong";
	}
	
	return szTimeZoneInfo[index];
}

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

int I13_CMD_SetTime(int chn, time_t t, int force)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!force)
	{
		if(!ipcam.enable || !I13_GetLinkStatus(chn))
		{
			//printf("I13_CMD_SetTime: chn%d vlost!!!\n",chn);
			return -1;
		}
	}
	
	char date[64];
	struct tm now;
	
	if(t == 0)
	{
		t = time(NULL);// + 1;//???
	}
	
	int nTimeZone = IPC_GetTimeZone();
	t += GetTZOffset(nTimeZone);
	
	localtime_r(&t,&now);
	sprintf(date,"%04d.%02d.%02d.%02d.%02d.%02d",
		now.tm_year+1900,
		now.tm_mon+1,
		now.tm_mday,
		now.tm_hour,
		now.tm_min,
		now.tm_sec);
	
	char cgi[384];
	//sprintf(cgi,"/cgi-bin/hi3510/param.cgi?cmd=setservertime&&-time=%s",date);
	sprintf(cgi,"/cgi-bin/hi3510/param.cgi?cmd=setservertime&-time=%s&-timezone=%s&-dstmode=off",date,GetTZInfo(nTimeZone));
	
	//csp modify for juan
	/*if(ipcam.protocol_type == PRO_TYPE_JUAN)
	{
		sprintf(cgi, "/NetSDK/System/deviceInfo");
		
		char result[1024];
		memset(result,0,sizeof(result));
		if(ipnc_exec_cgi_cmd(&ipcam, cgi, result) != 0)
		{
			printf("exec %s failed\n",cgi);
			return -1;
		}
		
		printf("exec %s result:%s\n",cgi,result);
		return 0;
	}*/
	
	int rtn = ipnc_do_cgi_cmd(&ipcam,cgi);
	if(rtn)
	{
		return -1;
	}
	
	return 0;
}

int I13_CMD_Reboot(int chn)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	char cgi[384];
	sprintf(cgi,"/cgi-bin/hi3510/param.cgi?cmd=sysreboot");
	
	int rtn = ipnc_do_cgi_cmd(&ipcam,cgi);
	if(rtn)
	{
		return -1;
	}
	
	return 0;
}

int I13_CMD_RequestIFrame(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_i13_client_count)
	{
		return -1;
	}
	
	int idx = chn;
	
	int subflag = 0;
	if(chn >= (int)(g_i13_client_count/2))
	{
		subflag = 1;
		chn -= (g_i13_client_count/2);
	}
	
	if(subflag)
	{
		return 0;
	}
	
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!ipcam.enable || !I13_GetLinkStatus(idx))
	{
		return -1;
	}
	
	char cgi[384];
	sprintf(cgi,"/cgi-bin/hi3510/param.cgi?cmd=requestifrm&-chn=%s",subflag?"12":"11");
	//sprintf(cgi,"/cgi-bin/hi3510/param.cgi?cmd=requestifrm&-chn=%s",subflag?"11":"11");
	//printf("chn%d request iframe:%s\n",chn,cgi);
	
	int rtn = ipnc_do_cgi_cmd(&ipcam,cgi);
	if(rtn)
	{
		return -1;
	}
	
	return 0;
}

