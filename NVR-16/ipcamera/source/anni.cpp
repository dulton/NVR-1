#include "Anni.h"

#define SUCCESS		0
#define FAILURE		(-1)

#define CHANNEL1 11
#define CHANNEL2 12

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

extern int IPC_Find(ipc_node* head, ipc_node *pNode);

#define BUFSIZE			1024

typedef struct anni_node
{
	JBNV_SERVER_PACK_EX_V2 jbServerPack;
	struct anni_node *next;
}anni_node;

static anni_node *g_anni_head = NULL;

int ANNI_Delete(anni_node* head)
{
	if(head == NULL)
	{
		return 0;
	}
	
	anni_node *p1 = head;
	
	while(p1)
	{
		anni_node *p2 = p1->next;
		
		free(p1);
		
		p1 = p2;
	}
	
	return 0;
}

int ANNI_Free()
{
	if(g_anni_head)
	{
		ANNI_Delete(g_anni_head);
		g_anni_head = NULL;
	}
	
	return 0;
}

int ANNI_Search(ipc_node** head, ipc_node **tail, unsigned char check_conflict)
{
	if(head == NULL || tail == NULL)
	{
		printf("ANNI_Search: param error\n");
		return -1;
	}
	
	//ANNI_Free();
	
	printf("search anni**************************\n");
	
	unsigned char conflict_flag = 0;
	
	struct sockaddr_in local, remote, from;
	
	int sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sock == -1)
	{
		printf("Search IPNC:socket error\n");
		return -1;
	}
	
	int reuseflag = 1;
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *)&reuseflag,sizeof(reuseflag)) == SOCKET_ERROR) 
	{
		printf("Search IPNC:port mulit error\n");
		close(sock);
		return -1;
	}
	
	u32 dwInterface = INADDR_ANY;
	u32 dwMulticastGroup = inet_addr(MULTI_BROADCAST_SREACH_IPADDR);
	u16 iPort = MULTI_BROADCAST_SREACH_RECV_PORT;
	
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = dwInterface;
	local.sin_port = htons(iPort);
	if(bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("Search IPNC:bind error\n");
		close(sock);
		return -1;
	}
	
	struct ip_mreq mreq;
	memset(&mreq,0,sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = dwMulticastGroup;
	mreq.imr_interface.s_addr = GetLocalIp();
	if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char*)&mreq,sizeof(mreq)) == SOCKET_ERROR)
	{
		printf("Search IPNC:join mulit error\n");
		close(sock);
		return -1;
	}
	
	int optval = 0;
	if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		printf("setsockopt(IP_MULTICAST_LOOP) failed\n");
		//close(sock);
		//return 0;
	}
	
	char recvbuf[BUFSIZE], sendbuf[BUFSIZE];
	
	remote.sin_family	   = AF_INET;
	remote.sin_addr.s_addr = dwMulticastGroup;
	remote.sin_port 	   = htons(MULTI_BROADCAST_SREACH_SEND_PORT);
	
	int i = 0;
	for(i = 0; i < 1; i++)
	{
		strcpy(sendbuf, "SearchServer");
		
		if(sendto(sock, (char *)sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&remote, sizeof(remote)) == SOCKET_ERROR)
		{
			printf("Search IPNC:sendto failed with: %d\n", errno);
			close(sock);
			return -1;
		}
	}
	
	int count = 0;
	unsigned int cc = 0;
	
	socklen_t len = sizeof(struct sockaddr_in);
	
	for(i = 0; i < 100; i++)
	{
		fd_set r;
		struct timeval t;
		
		t.tv_sec = 0;
		t.tv_usec = 10*1000;//100;
		
		FD_ZERO(&r);
		FD_SET(sock, &r);
		
		int ret = select(sock + 1, &r, NULL, NULL, &t);
		
		if(ret <= 0)
		{
			//if(cc++ >= 10)
			if(cc++ >= 20)
			{
				break;
			}
			continue;
		}
		
		cc = 0;
		
		if(ret > 0)
		{
			ret = recvfrom(sock, recvbuf, BUFSIZE, 0, (struct sockaddr *)&from, &len);
			if(ret != sizeof(JBNV_SERVER_MSG_DATA_EX_V2))
			{
				printf("anni ipc MULTI_BROADCAST response error\n");
				continue;//break;
			}
			
			JBNV_SERVER_MSG_DATA_EX_V2 *psev_msg = (JBNV_SERVER_MSG_DATA_EX_V2 *)recvbuf;
			
			#if 0
			dbg("IPC response: ");
			dbg("\tdwPackFlag: 0x%lx", psev_msg->dwPackFlag);
			dbg("\tcamera ip: %s", psev_msg->jbServerPack.jspack.szIp);
			dbg("\twMediaPort: %d", psev_msg->jbServerPack.jspack.wMediaPort);
			dbg("\twWebPort: %d", psev_msg->jbServerPack.jspack.wWebPort);
			dbg("\twChannelCount: %d", psev_msg->jbServerPack.jspack.wChannelCount);
			dbg("\tszServerName: %s", psev_msg->jbServerPack.jspack.szServerName);
			dbg("\tdwDeviceType: %ld", psev_msg->jbServerPack.jspack.dwDeviceType);
			//MAC ADDR
			dbg("\tMAC ADDR: ");
			int i = 0;
			for(i = 0; i < 6; i++)
			{
				if(i == 5)
				{
					printf("%02x\n", psev_msg->jbServerPack.bMac[i] & 0xff);
				}
				else
				{
					printf("%02x:", psev_msg->jbServerPack.bMac[i] & 0xff);
				}
			}
			
			struct in_addr in;
			in.s_addr = (in_addr_t)psev_msg->jbServerPack.dwNetMask;
			dbg("\tdwNetMask: %s", inet_ntoa(in));
			
			in.s_addr = (in_addr_t)psev_msg->jbServerPack.dwGateway;
			dbg("\tdwGateway: %s", inet_ntoa(in));
			
			in.s_addr = (in_addr_t)psev_msg->jbServerPack.dwDNS;
			dbg("\tdwDNS: %s", inet_ntoa(in));
			#endif
			
			//if(1)
			{
				anni_node *pAnni = (anni_node *)malloc(sizeof(anni_node));
				if(pAnni == NULL)
				{
					printf("Not enough space to save new anni ipc info.\n");
					close(sock);
					return count;//return -1;
				}
				
				ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
				if(pNode == NULL)
				{
					printf("Not enough space to save new ipc info.\n");
					close(sock);
					free(pAnni);
					pAnni = NULL;
					return count;//return -1;
				}
				
				pAnni->jbServerPack = psev_msg->jbServerPack;
				pAnni->next = g_anni_head;
				g_anni_head = pAnni;
				
				memset(pNode, 0, sizeof(*pNode));
				pNode->next = NULL;
				
				
				
				int i = 0;
				for(i = 0; i < 6; i++)
				{
					sprintf(pNode->ipcam.address+i*3, "%02x:", psev_msg->jbServerPack.bMac[i] & 0xff);
				}
				pNode->ipcam.address[17] = 0;//trunc last ':'
				//printf("anni mac: %s\n", pNode->ipcam.address);
				
				strcpy(pNode->ipcam.user, "admin");
				strcpy(pNode->ipcam.pwd, "admin");
				pNode->ipcam.channel_no = 0;
				pNode->ipcam.enable = 0;
				pNode->ipcam.ipc_type = IPC_TYPE_720P;
				pNode->ipcam.protocol_type = PRO_TYPE_ANNI;
				pNode->ipcam.stream_type = STREAM_TYPE_MAIN;
				pNode->ipcam.trans_type = TRANS_TYPE_TCP;
				pNode->ipcam.force_fps = 0;
				pNode->ipcam.frame_rate = 30;
				pNode->ipcam.dwIp = inet_addr(psev_msg->jbServerPack.jspack.szIp);
				pNode->ipcam.wPort = psev_msg->jbServerPack.jspack.wMediaPort;
				//strcpy(pNode->ipcam.uuid, psev_msg->jbServerPack.csServerNo);//************
				strcpy(pNode->ipcam.uuid, pNode->ipcam.address);//************
				strcpy(pNode->ipcam.name, psev_msg->jbServerPack.jspack.szServerName);
				pNode->ipcam.net_mask = psev_msg->jbServerPack.dwNetMask;
				pNode->ipcam.net_gateway =psev_msg->jbServerPack.dwGateway;
				pNode->ipcam.dns1 = psev_msg->jbServerPack.dwDNS;
				
				
				
				int rtn = IPC_Find(*head, pNode);
				//printf("anni IPC_Find ret: %d\n", rtn);
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
				printf("ANNI-ipc%d : [%s,0x%08x,%d,%s]\n", count, pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
				
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

typedef struct
{
	volatile int cmd_fd;//命令通道SOCKET
	DWORD dwServerIndex;//用户 ID, 在用户登录的时候由前端返回
	int kp_alv_lost_cnt;//keep alive timeout count
	unsigned int dwIp;
	unsigned short wPort;
	char username[32];
	char password[32];
	DWORD dwStream1Width;
	DWORD dwStream1Height;
	DWORD dwStream2Width;
	DWORD dwStream2Height;
	pthread_mutex_t lock;
}anni_camera_info;

typedef struct
{
	volatile int cap_fd;//数据通道SOCKET
	RealStreamCB pStreamCB;
	unsigned int dwContext;
	int video_width;
	int video_height;
	volatile char eventLoopWatchVariable;
	pthread_t pid;
	pthread_mutex_t lock;
}anni_client_info;

static anni_camera_info *g_annicamera_info = NULL;

static anni_client_info *g_annic_info = NULL;

static unsigned int g_anni_client_count = 0;

static pthread_t keep_alive_pid = 0;

static unsigned char g_init_flag = 0;

static unsigned char g_sdk_inited = 0;

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

static int send_data(int sock, BYTE* pu8Buff, UINT u32DataLen)
{
	if(send_loop(sock, pu8Buff, u32DataLen) <= 0)
	{
		printf("SendData failed\n");
		return FAILURE;
	}
	
	return SUCCESS;
}

static int recv_data(int socket, BYTE *buf, int recv_len)
{
	int pack_len = 0;
	int ret = -1;
	BYTE * cur_buf = NULL;
	
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

int wait_fd_event(int fd, unsigned int timeout)
{
	if(fd < 0)
	{
		return -1;
	}
	
	struct timeval struTimeout;
	fd_set struReadSet;
	
	struTimeout.tv_sec = timeout/1000;
	struTimeout.tv_usec = timeout%1000;
	
	FD_ZERO(&struReadSet);
	FD_SET(fd, &struReadSet);
	
	int s32Ret = select(fd + 1, &struReadSet, NULL, NULL, &struTimeout);
	if(s32Ret <= 0)
	{
		printf("wait_fd_event:select Error,s32Ret=%d\n",s32Ret);
	}
	
	return s32Ret;
}

int anni_login2(ipc_unit *ipcam)
{
	if(ipcam == NULL)
	{
		return -1;
	}
	
	WORD wErrorCode = 0;
	int cmd_sock = ConnectWithTimeout(ipcam->dwIp, ipcam->wPort, 5000, &wErrorCode);
	if(cmd_sock < 0)
	{
		printf("anni_login2 connect (0x%08x:%d) failed\n",ipcam->dwIp,ipcam->wPort);
		return -1;
	}
	
	//命令socket 登录，获取USERID，用于之后获取视频数据							
	ACCEPT_HEAD accepthead;
	accepthead.nFlag = 9000;
	accepthead.nSockType = 0;//新命令通道网络连接
	int ret = send_data(cmd_sock, (BYTE *)&accepthead, sizeof(accepthead));
	if(ret == FAILURE)
	{
		dbg("ACCEPT_HEAD");
		close(cmd_sock);
		cmd_sock = -1;
		return -1;
	}
	printf("anni_login2 send ACCEPT_HEAD ok\n");
	
	USER_INFO u_info;//登陆IPC
	memset(&u_info, 0, sizeof(u_info));
	strcpy(u_info.szUserName, ipcam->user);//"admin";
	strcpy(u_info.szUserPsw, ipcam->pwd);//"admin";
	
	//send user info
	ret = send_data(cmd_sock, (BYTE*)&u_info, sizeof(u_info));
	if(ret == FAILURE)
	{
		dbg("TCP socket send USER_INFO");
		close(cmd_sock);
		cmd_sock = -1;
		return -1;
	}
	printf("anni_login2 send USER_INFO ok\n");
	
	//recv comm head
	COMM_HEAD commhead;
	ret = recv_data(cmd_sock, (BYTE *)&commhead, sizeof(commhead));
	if(ret < 0)
	{
		dbg("TCP socket recv COMM_HEAD");
		close(cmd_sock);
		cmd_sock = -1;
		return -1;
	}
	//printf("TCP socket recv COMM_HEAD: \n");
	//printf("\t commhead.nFlag: %ld\n", commhead.nFlag);
	//printf("\t commhead.nCommand: %ld\n", commhead.nCommand);
	//printf("\t commhead.nChannel: %ld\n", commhead.nChannel);
	//printf("\t commhead.nErrorCode: %ld\n", commhead.nErrorCode);
	//printf("\t commhead.nBufSize: %ld\n", commhead.nBufSize);
	if((commhead.nCommand != NETCMD_CHANNEL_CHECK_OK) || (commhead.nBufSize <= 0))
	{
		dbg("login anni IPC failed");
		close(cmd_sock);
		cmd_sock = -1;
		return -1;
	}
	
	BYTE *pbuf = (BYTE *)malloc(commhead.nBufSize);
	if(NULL == pbuf)
	{
		dbg("malloc pbuf failed");
		close(cmd_sock);
		cmd_sock = -1;
		return -1;
	}
	
	ret = recv_data(cmd_sock, (BYTE *)pbuf, commhead.nBufSize);
	if(ret < 0)
	{
		dbg("TCP socket recv response for COMM_HEAD");
		close(cmd_sock);
		cmd_sock = -1;
		free(pbuf);
		return -1;
	}
	
	//buf: CMD_SERVER_INFO + 视频通道数*JBNV_CHANNEL_INFO + 探头数量*JBNV_SENSOR_INFO 
	CMD_SERVER_INFO *pcsi = NULL;
	JBNV_CHANNEL_INFO *pjci = NULL;
	//JBNV_SENSOR_INFO *pjsi = NULL;
	
	pcsi = (CMD_SERVER_INFO *)pbuf;
	//printf("dwServerIndex(nID): %ld\n", pcsi->dwServerIndex);
	//printf("pcsi->ServerInfo.wChannelNum: %d\n", pcsi->ServerInfo.wChannelNum);
	//printf("pcsi->ServerInfo.AlarmInNum: %ld\n", pcsi->ServerInfo.AlarmInNum);
	//printf("pcsi->ServerInfo.AlarmOutNum: %ld\n", pcsi->ServerInfo.AlarmOutNum);
	//printf("pcsi->ServerInfo.szServerIp: %s\n", pcsi->ServerInfo.szServerIp);
	//printf("pcsi->ServerInfo.wServerPort: %d\n", pcsi->ServerInfo.wServerPort);
	//printf("pcsi->ServerInfo.dwServerCPUType: %ld\n", pcsi->ServerInfo.dwServerCPUType);
	
	pjci = (JBNV_CHANNEL_INFO *)(pbuf + sizeof(CMD_SERVER_INFO));
	//printf("pjci->dwStream1Height: %ld\n",pjci->dwStream1Height);
	//printf("pjci->dwStream1Width: %ld\n",pjci->dwStream1Width);
	//printf("pjci->dwStream1CodecID: %ld\n",pjci->dwStream1CodecID);
	//printf("pjci->dwStream2Height: %ld\n",pjci->dwStream2Height);
	//printf("pjci->dwStream2Width: %ld\n",pjci->dwStream2Width);
	//printf("pjci->dwStream2CodecID: %ld\n",pjci->dwStream2CodecID);
	//printf("pjci->csChannelName: %s\n",pjci->csChannelName);
	
	//pjsi = (JBNV_SENSOR_INFO *)(pbuf + sizeof(CMD_SERVER_INFO) + pcsi->ServerInfo.wChannelNum * sizeof(JBNV_CHANNEL_INFO));
	//printf("pjsi->dwIndex: %ld\n", pjsi->dwIndex);
	//printf("pjsi->dwSensorType: %ld\n", pjsi->dwSensorType);
	//printf("pjsi->csSensorName: %s\n", pjsi->csSensorName);
	
	//pjsi = (JBNV_SENSOR_INFO *)(pbuf + sizeof(CMD_SERVER_INFO) + pcsi->ServerInfo.wChannelNum * sizeof(JBNV_CHANNEL_INFO) + sizeof(JBNV_SENSOR_INFO));
	//printf("pjsi->dwIndex: %ld\n", pjsi->dwIndex);
	//printf("pjsi->dwSensorType: %ld\n", pjsi->dwSensorType);
	//printf("pjsi->csSensorName: %s\n", pjsi->csSensorName);
	
	free(pbuf);
	
	printf("anni_login2 success\n");
	
	return cmd_sock;
}

int anni_login(int chn, unsigned int dwIp, unsigned short wPort, char *user, char *pwd)
{
	if(chn < 0 || chn >= (int)(g_anni_client_count))
	{
		printf("anni_login: param error,chn=%d\n",chn);
		return -1;
	}
	
	if(dwIp == 0 || wPort == 0 || dwIp == (unsigned int)(-1))
	{
		printf("anni_login: ip:port error\n");
		return -1;
	}
	
	int real_chn = (chn < (int)(g_anni_client_count/2)) ? chn : (chn - (int)(g_anni_client_count/2));
	//printf("anni_login: real_chn=%d\n", real_chn);
	
	pthread_mutex_lock(&g_annicamera_info[real_chn].lock);
	
	if(g_annicamera_info[real_chn].cmd_fd < 0)
	{
		printf("anni_login: chn%d login\n", real_chn);
		
		WORD wErrorCode = 0;
		int cmd_sock = ConnectWithTimeout(dwIp, wPort, 5000, &wErrorCode);
		if(cmd_sock < 0)
		{
			printf("chn%d anni_login connect (0x%08x:%d) failed\n",chn,dwIp,wPort);
			pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
			return -1;
		}
		
		//命令socket 登录，获取USERID，用于之后获取视频数据							
		ACCEPT_HEAD accepthead;
		accepthead.nFlag = 9000;
		accepthead.nSockType = 0;//新命令通道网络连接
		int ret = send_data(cmd_sock, (BYTE *)&accepthead, sizeof(accepthead));
		if(ret == FAILURE)
		{
			dbg("ACCEPT_HEAD");
			close(cmd_sock);
			cmd_sock = -1;
			pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
			return -1;
		}
		printf("anni_login send ACCEPT_HEAD ok\n");
		
		#if 0
		int i = 0;
		//get local mac
		struct ifreq ifreq;
		strcpy(ifreq.ifr_name, "eth0");
		ret = ioctl(cmd_sock, SIOCGIFHWADDR, &ifreq);
		if(ret < 0)
		{
			dbg("get mac addr ioctl error");
			close(cmd_sock);
			cmd_sock = -1;
			pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
			return -1;
		}
		printf("local ethx mac addr: ");
		for(i = 0; i < 6; i++)
		{
			if(i == 5)
			{
				printf("%02x\n", ifreq.ifr_hwaddr.sa_data[i] & 0xff);
			}
			else
			{
				printf("%02x:", ifreq.ifr_hwaddr.sa_data[i] & 0xff);
			}
		}
		#endif
		
		USER_INFO u_info;//登陆IPC
		memset(&u_info, 0, sizeof(u_info));
		strcpy(u_info.szUserName, user);//"admin";
		strcpy(u_info.szUserPsw, pwd);//"admin";
		#if 0
		//memcpy(u_info.MacCheck, ifreq.ifr_hwaddr.sa_data, 6);
		for(i = 0; i < 6; i++)
		{
			if(i == 5)
			{
				sprintf(u_info.MacCheck+strlen(u_info.MacCheck), "%02x\n", ifreq.ifr_hwaddr.sa_data[i] & 0xff);
			}
			else
			{
				sprintf(u_info.MacCheck+strlen(u_info.MacCheck), "%02x:", ifreq.ifr_hwaddr.sa_data[i] & 0xff);
			}
		}
		#endif
		
		//send user info
		ret = send_data(cmd_sock, (BYTE*)&u_info, sizeof(u_info));
		if(ret == FAILURE)
		{
			dbg("TCP socket send USER_INFO");
			close(cmd_sock);
			cmd_sock = -1;
			pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
			return -1;
		}
		printf("anni_login send USER_INFO ok\n");
		
		//recv comm head
		COMM_HEAD commhead;
		ret = recv_data(cmd_sock, (BYTE *)&commhead, sizeof(commhead));
		if(ret < 0)
		{
			dbg("TCP socket recv COMM_HEAD");
			close(cmd_sock);
			cmd_sock = -1;
			pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
			return -1;
		}
		//printf("TCP socket recv COMM_HEAD: \n");
		//printf("\t commhead.nFlag: %ld\n", commhead.nFlag);
		//printf("\t commhead.nCommand: %ld\n", commhead.nCommand);
		//printf("\t commhead.nChannel: %ld\n", commhead.nChannel);
		//printf("\t commhead.nErrorCode: %ld\n", commhead.nErrorCode);
		//printf("\t commhead.nBufSize: %ld\n", commhead.nBufSize);
		if((commhead.nCommand != NETCMD_CHANNEL_CHECK_OK) || (commhead.nBufSize <= 0))
		{
			dbg("login anni IPC failed");
			close(cmd_sock);
			cmd_sock = -1;
			pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
			return -1;
		}
		
		BYTE *pbuf = (BYTE *)malloc(commhead.nBufSize);
		if(NULL == pbuf)
		{
			dbg("malloc pbuf failed");
			close(cmd_sock);
			cmd_sock = -1;
			pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
			return -1;
		}
		
		ret = recv_data(cmd_sock, (BYTE *)pbuf, commhead.nBufSize);
		if(ret < 0)
		{
			dbg("TCP socket recv response for COMM_HEAD");
			close(cmd_sock);
			cmd_sock = -1;
			free(pbuf);
			pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
			return -1;
		}
		
		//buf: CMD_SERVER_INFO + 视频通道数*JBNV_CHANNEL_INFO + 探头数量*JBNV_SENSOR_INFO 
		CMD_SERVER_INFO *pcsi = NULL;
		JBNV_CHANNEL_INFO *pjci = NULL;
		//JBNV_SENSOR_INFO *pjsi = NULL;
		
		pcsi = (CMD_SERVER_INFO *)pbuf;
		//printf("dwServerIndex(nID): %ld\n", pcsi->dwServerIndex);
		//printf("pcsi->ServerInfo.wChannelNum: %d\n", pcsi->ServerInfo.wChannelNum);
		//printf("pcsi->ServerInfo.AlarmInNum: %ld\n", pcsi->ServerInfo.AlarmInNum);
		//printf("pcsi->ServerInfo.AlarmOutNum: %ld\n", pcsi->ServerInfo.AlarmOutNum);
		//printf("pcsi->ServerInfo.szServerIp: %s\n", pcsi->ServerInfo.szServerIp);
		//printf("pcsi->ServerInfo.wServerPort: %d\n", pcsi->ServerInfo.wServerPort);
		//printf("pcsi->ServerInfo.dwServerCPUType: %ld\n", pcsi->ServerInfo.dwServerCPUType);
		
		pjci = (JBNV_CHANNEL_INFO *)(pbuf + sizeof(CMD_SERVER_INFO));
		//printf("pjci->dwStream1Height: %ld\n",pjci->dwStream1Height);
		//printf("pjci->dwStream1Width: %ld\n",pjci->dwStream1Width);
		//printf("pjci->dwStream1CodecID: %ld\n",pjci->dwStream1CodecID);
		//printf("pjci->dwStream2Height: %ld\n",pjci->dwStream2Height);
		//printf("pjci->dwStream2Width: %ld\n",pjci->dwStream2Width);
		//printf("pjci->dwStream2CodecID: %ld\n",pjci->dwStream2CodecID);
		//printf("pjci->csChannelName: %s\n",pjci->csChannelName);
		
		//pjsi = (JBNV_SENSOR_INFO *)(pbuf + sizeof(CMD_SERVER_INFO) + pcsi->ServerInfo.wChannelNum * sizeof(JBNV_CHANNEL_INFO));
		//printf("pjsi->dwIndex: %ld\n", pjsi->dwIndex);
		//printf("pjsi->dwSensorType: %ld\n", pjsi->dwSensorType);
		//printf("pjsi->csSensorName: %s\n", pjsi->csSensorName);
		
		//pjsi = (JBNV_SENSOR_INFO *)(pbuf + sizeof(CMD_SERVER_INFO) + pcsi->ServerInfo.wChannelNum * sizeof(JBNV_CHANNEL_INFO) + sizeof(JBNV_SENSOR_INFO));
		//printf("pjsi->dwIndex: %ld\n", pjsi->dwIndex);
		//printf("pjsi->dwSensorType: %ld\n", pjsi->dwSensorType);
		//printf("pjsi->csSensorName: %s\n", pjsi->csSensorName);
		
		g_annicamera_info[real_chn].dwIp = dwIp;
		g_annicamera_info[real_chn].wPort = wPort;
		strcpy(g_annicamera_info[real_chn].username, user);
		strcpy(g_annicamera_info[real_chn].password, pwd);
		
		g_annicamera_info[real_chn].dwStream1Width = pjci->dwStream1Width;
		g_annicamera_info[real_chn].dwStream1Height = pjci->dwStream1Height;
		g_annicamera_info[real_chn].dwStream2Width = pjci->dwStream2Width;
		g_annicamera_info[real_chn].dwStream2Height = pjci->dwStream2Height;
		
		g_annicamera_info[real_chn].kp_alv_lost_cnt = 0;
		g_annicamera_info[real_chn].dwServerIndex = pcsi->dwServerIndex;
		g_annicamera_info[real_chn].cmd_fd = cmd_sock;//主子码流共用
		
		free(pbuf);
	}
	
	pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
	
	return 0;
}

int anni_logout(int chn)
{
	if(chn < 0 || chn >= (int)(g_anni_client_count))
	{
		printf("anni_logout: param error,chn=%d\n",chn);
		return -1;
	}
	
	int real_chn = (chn < (int)(g_anni_client_count/2)) ? chn : (chn - (int)(g_anni_client_count/2));
	//printf("anni_logout: real_chn=%d\n", real_chn);
	
	pthread_mutex_lock(&g_annicamera_info[real_chn].lock);
	
	if(g_annicamera_info[real_chn].cmd_fd != -1)
	{
		printf("anni_logout: chn%d logout\n", real_chn);
		
		close(g_annicamera_info[real_chn].cmd_fd);
		g_annicamera_info[real_chn].cmd_fd = -1;
		g_annicamera_info[real_chn].dwServerIndex = 0;
		g_annicamera_info[real_chn].kp_alv_lost_cnt = 0;
	}
	
	pthread_mutex_unlock(&g_annicamera_info[real_chn].lock);
	
	pthread_mutex_lock(&g_annic_info[real_chn].lock);
	
	if(g_annic_info[real_chn].cap_fd != -1)
	{
		close(g_annic_info[real_chn].cap_fd);
		g_annic_info[real_chn].cap_fd = -1;
	}
	
	pthread_mutex_unlock(&g_annic_info[real_chn].lock);
	
	real_chn += (g_anni_client_count/2);
	
	pthread_mutex_lock(&g_annic_info[real_chn].lock);
	
	if(g_annic_info[real_chn].cap_fd != -1)
	{
		close(g_annic_info[real_chn].cap_fd);
		g_annic_info[real_chn].cap_fd = -1;
	}
	
	pthread_mutex_unlock(&g_annic_info[real_chn].lock);
	
	return 0;
}

int Anni_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
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
	
	int sock = anni_login2(ipcam);
	if(sock < 0)
	{
		return -1;
	}
	
	COMM_HEAD commhead;
	commhead.nFlag = 9000;
	commhead.nCommand = CMD_GET_NETWORK;
	commhead.nChannel = 0;
	commhead.nErrorCode = 0;
	commhead.nBufSize = 0;
	int ret = send_data(sock, (BYTE *)&commhead, sizeof(commhead));
	if(ret == FAILURE)
	{
		dbg("Anni_GetNetworkParam send COMM_HEAD failed");
		close(sock);
		sock = -1;
		return -1;
	}
	else
	{
		printf("Anni_GetNetworkParam send COMM_HEAD ok\n");
	}
	
	if(wait_fd_event(sock, 4000) <= 0)
	{
		dbg("wait_fd_event failed");
		close(sock);
		sock = -1;
		return -1;
	}
	else
	{
		dbg("wait_fd_event success");
	}
	
	//recv comm head
	memset(&commhead, 0, sizeof(commhead));
	ret = recv_data(sock, (BYTE *)&commhead, sizeof(commhead));
	if(ret < 0)
	{
		dbg("TCP socket recv COMM_HEAD");
		close(sock);
		sock = -1;
		return -1;
	}
	//printf("TCP socket recv COMM_HEAD: \n");
	//printf("\t commhead.nFlag: %ld\n", commhead.nFlag);
	//printf("\t commhead.nCommand: %ld\n", commhead.nCommand);
	//printf("\t commhead.nChannel: %ld\n", commhead.nChannel);
	//printf("\t commhead.nErrorCode: %ld\n", commhead.nErrorCode);
	//printf("\t commhead.nBufSize: %ld\n", commhead.nBufSize);
	//printf("\t DMS_SERVER_NETWORK: %d\n", sizeof(DMS_SERVER_NETWORK));
	if((commhead.nCommand != CMD_GET_NETWORK) || (commhead.nBufSize != sizeof(DMS_SERVER_NETWORK)))
	{
		dbg("anni IPC response failed");
		close(sock);
		sock = -1;
		return -1;
	}
	
	DMS_SERVER_NETWORK network;
	ret = recv_data(sock, (BYTE *)&network, sizeof(network));
	if(ret < 0)
	{
		dbg("TCP socket recv DMS_SERVER_NETWORK");
		close(sock);
		sock = -1;
		return -1;
	}
	
	close(sock);
	sock = -1;
	
	unsigned char *p = NULL;
	char address[32];
	
	p = (unsigned char *)&network.dwNetIpAddr;
	sprintf(address,"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
	printf("Anni_GetNetworkParam ip=%s\n",address);
	pnw->ip_address = inet_addr(address);
	
	p = (unsigned char *)&network.dwNetMask;
	sprintf(address,"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
	printf("Anni_GetNetworkParam netmask=%s\n",address);
	pnw->net_mask = inet_addr(address);
	
	p = (unsigned char *)&network.dwGateway;
	sprintf(address,"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
	printf("Anni_GetNetworkParam gw=%s\n",address);
	pnw->net_gateway = inet_addr(address);
	
	p = (unsigned char *)&network.dwDNSServer;
	sprintf(address,"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
	printf("Anni_GetNetworkParam dns1=%s\n",address);
	pnw->dns1 = inet_addr(address);
	
	pnw->ip_address = network.dwNetIpAddr;
	pnw->net_mask = network.dwNetMask;
	pnw->net_gateway = network.dwGateway;
	pnw->dns1 = network.dwDNSServer;
	
	printf("Anni_GetNetworkParam:get success\n");
	
	return 0;
}

int Anni_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
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
	
#if 0
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
#endif
	
#if 1
	int sock = anni_login2(ipcam);
	if(sock < 0)
	{
		return -1;
	}
	
	COMM_HEAD commhead;
	commhead.nFlag = 9000;
	commhead.nCommand = CMD_GET_NETWORK;
	commhead.nChannel = 0;
	commhead.nErrorCode = 0;
	commhead.nBufSize = 0;
	int ret = send_data(sock, (BYTE *)&commhead, sizeof(commhead));
	if(ret == FAILURE)
	{
		dbg("CMD_GET_NETWORK send COMM_HEAD failed");
		close(sock);
		sock = -1;
		return -1;
	}
	else
	{
		printf("CMD_GET_NETWORK send COMM_HEAD ok\n");
	}
	
	if(wait_fd_event(sock, 4000) <= 0)
	{
		dbg("wait_fd_event failed");
		close(sock);
		sock = -1;
		return -1;
	}
	else
	{
		dbg("wait_fd_event success");
	}
	
	//recv comm head
	memset(&commhead, 0, sizeof(commhead));
	ret = recv_data(sock, (BYTE *)&commhead, sizeof(commhead));
	if(ret < 0)
	{
		dbg("TCP socket recv COMM_HEAD");
		close(sock);
		sock = -1;
		return -1;
	}
	//printf("TCP socket recv COMM_HEAD: \n");
	//printf("\t commhead.nFlag: %ld\n", commhead.nFlag);
	//printf("\t commhead.nCommand: %ld\n", commhead.nCommand);
	//printf("\t commhead.nChannel: %ld\n", commhead.nChannel);
	//printf("\t commhead.nErrorCode: %ld\n", commhead.nErrorCode);
	//printf("\t commhead.nBufSize: %ld\n", commhead.nBufSize);
	//printf("\t DMS_SERVER_NETWORK: %d\n", sizeof(DMS_SERVER_NETWORK));
	if((commhead.nCommand != CMD_GET_NETWORK) || (commhead.nBufSize != sizeof(DMS_SERVER_NETWORK)))
	{
		dbg("anni IPC response failed");
		close(sock);
		sock = -1;
		return -1;
	}
	
	DMS_SERVER_NETWORK network;
	ret = recv_data(sock, (BYTE *)&network, sizeof(network));
	if(ret < 0)
	{
		dbg("TCP socket recv DMS_SERVER_NETWORK");
		close(sock);
		sock = -1;
		return -1;
	}
	
	commhead.nFlag = 9000;
	commhead.nCommand = CMD_SET_NETWORK;
	commhead.nChannel = 0;
	commhead.nErrorCode = 0;
	commhead.nBufSize = sizeof(DMS_SERVER_NETWORK);
	ret = send_data(sock, (BYTE *)&commhead, sizeof(commhead));
	if(ret == FAILURE)
	{
		dbg("CMD_SET_NETWORK send COMM_HEAD failed");
		close(sock);
		sock = -1;
		return -1;
	}
	else
	{
		printf("CMD_SET_NETWORK send COMM_HEAD ok\n");
	}
	
	network.dwNetIpAddr = pnw->ip_address;
	network.dwNetMask = pnw->net_mask;
	network.dwGateway = pnw->net_gateway;
	network.dwDNSServer = pnw->dns1;
	ret = send_data(sock, (BYTE *)&network, sizeof(network));
	if(ret == FAILURE)
	{
		dbg("CMD_SET_NETWORK send param failed");
		close(sock);
		sock = -1;
		return -1;
	}
	else
	{
		printf("CMD_SET_NETWORK send param ok\n");
	}
	
	close(sock);
	sock = -1;
#else
	anni_node *p1 = g_anni_head;
	while(p1)
	{
		char address[64];
		
		int i = 0;
		for(i = 0; i < 6; i++)
		{
			sprintf(address+i*3, "%02x:", p1->jbServerPack.bMac[i] & 0xff);
		}
		address[17] = 0;//trunc last ':'
		printf("anni mac: %s\n", address);
		
		if(strcmp(address, ipcam->address) == 0)
		{
			break;
		}
		
		p1 = p1->next;
	}
	
	if(p1 == NULL)
	{
		return -1;
	}
	
	JBNV_SET_SERVER_INFO_BROADCAST_V2 servinfo;
	servinfo.nBufSize = sizeof(JBNV_SET_SERVER_INFO_BROADCAST_V2);// - sizeof(servinfo.nBufSize);
	servinfo.hdr.dwSize = sizeof(DMS_BROADCAST_HEADER);
	servinfo.hdr.nCmd = 0x30000001;
	servinfo.hdr.dwPackFlag = 0x03404325;
	servinfo.hdr.nErrorCode = 0;
	servinfo.setInfo.dwSize = sizeof(JBNV_SET_SIGHT_SERVER_INFO);
	servinfo.setInfo.dwIp = ipcam->dwIp;//inet_addr(p1->jbServerPack.jspack.szIp);
	servinfo.setInfo.dwMediaPort = ipcam->wPort;//p1->jbServerPack.jspack.wMediaPort;
	servinfo.setInfo.dwWebPort = p1->jbServerPack.jspack.wWebPort;
	servinfo.setInfo.dwNetMask = ipcam->net_mask;
	servinfo.setInfo.dwGateway = ipcam->net_gateway;
	servinfo.setInfo.dwDNS = ipcam->dns1;
	servinfo.setInfo.dwComputerIP = GetLocalIp();//p1->jbServerPack.dwComputerIP;
	servinfo.setInfo.bEnableDHCP = FALSE;
	servinfo.setInfo.bEnableAutoDNS = FALSE;
	servinfo.setInfo.bEncodeAudio = p1->jbServerPack.bEncodeAudio;
	memcpy(servinfo.setInfo.szoldMac, p1->jbServerPack.bMac, 6);
	memcpy(servinfo.setInfo.szMac, p1->jbServerPack.bMac, 6);
	strcpy(servinfo.setInfo.szServerName, p1->jbServerPack.jspack.szServerName);
	servinfo.nxServer.dwSize = sizeof(JBNV_NXSIGHT_SERVER_ADDR_V2);
	servinfo.nxServer.bEnable = p1->jbServerPack.bEnableCenter;
	servinfo.nxServer.dwCenterIp = p1->jbServerPack.dwCenterIpAddress;
	servinfo.nxServer.wCenterPort = p1->jbServerPack.dwCenterPort;
	strcpy(servinfo.nxServer.csServerNo, p1->jbServerPack.csServerNo);
	strcpy(servinfo.nxServer.csCenterIP, p1->jbServerPack.csCenterIpAddress);
	
	struct sockaddr_in local, remote;
	
	int sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(sock == -1)
	{
		printf("Anni_SetNetworkParam:socket error\n");
		return -1;
	}
	
	int reuseflag = 1;
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *)&reuseflag,sizeof(reuseflag)) == SOCKET_ERROR) 
	{
		printf("Anni_SetNetworkParam:port mulit error\n");
		close(sock);
		return -1;
	}
	
	u32 dwInterface = INADDR_ANY;
	u32 dwMulticastGroup = inet_addr(MULTI_BROADCAST_SREACH_IPADDR);
	u16 iPort = MULTI_BROADCAST_SREACH_RECV_PORT;
	
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = dwInterface;
	local.sin_port = htons(iPort);
	if(bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("Anni_SetNetworkParam:bind error\n");
		close(sock);
		return -1;
	}
	
	struct ip_mreq mreq;
	memset(&mreq,0,sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = dwMulticastGroup;
	mreq.imr_interface.s_addr = GetLocalIp();
	if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char*)&mreq,sizeof(mreq)) == SOCKET_ERROR)
	{
		printf("Anni_SetNetworkParam:join mulit error\n");
		close(sock);
		return -1;
	}
	
	int optval = 0;
	if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		printf("setsockopt(IP_MULTICAST_LOOP) failed\n");
		//close(sock);
		//return 0;
	}
	
	remote.sin_family	   = AF_INET;
	remote.sin_addr.s_addr = dwMulticastGroup;
	remote.sin_port 	   = htons(MULTI_BROADCAST_SREACH_SEND_PORT);
	
	int i = 0;
	for(i = 0; i < 1; i++)
	{
		if(sendto(sock, (char *)&servinfo, sizeof(servinfo), 0, (struct sockaddr *)&remote, sizeof(remote)) == SOCKET_ERROR)
		{
			printf("Anni_SetNetworkParam:sendto failed with: %d\n", errno);
			close(sock);
			return -1;
		}
	}
	
	close(sock);
#endif
	
	printf("Anni_SetNetworkParam:set success\n");
	
	return 0;
}

int anni_sendheartbeat(int chn)
{
	if(chn < 0 || chn >= (int)(g_anni_client_count/2))
	{
		printf("anni_sendheartbeat: param error,chn=%d\n",chn);
		return -1;
	}
	
	COMM_HEAD send_h;
	memset(&send_h, 0, sizeof(send_h));
	send_h.nFlag = 9000;
	send_h.nCommand = NETCMD_KEEP_ALIVE;
	
	//printf("anni_sendheartbeat: chn%d before lock\n",chn);
	
	pthread_mutex_lock(&g_annicamera_info[chn].lock);
	
	//printf("anni_sendheartbeat: chn%d after lock\n",chn);
	
	if(g_annicamera_info[chn].cmd_fd < 0)
	{
		//printf("anni_sendheartbeat: chn%d sock error\n",chn);
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
		return -1;
	}
	
	int ret = send_data(g_annicamera_info[chn].cmd_fd, (BYTE *)&send_h, sizeof(send_h));
	if(ret == FAILURE)
	{
		printf("anni_sendheartbeat: chn%d send failed\n",chn);
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
		return -1;
	}
	//printf("anni_sendheartbeat: chn%d send success\n",chn);
	
	if(wait_fd_event(g_annicamera_info[chn].cmd_fd, 4000) <= 0)
	{
		printf("anni_sendheartbeat: chn%d wait response failed\n",chn);
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
		return -1;
	}
	//printf("anni_sendheartbeat: chn%d wait response success\n",chn);
	
	COMM_HEAD recv_h;
	ret = recv_data(g_annicamera_info[chn].cmd_fd, (BYTE *)&recv_h, sizeof(recv_h));
	if(ret < 0)
	{
		printf("anni_sendheartbeat: chn%d recv failed\n",chn);
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
		return -1;
	}
	//printf("anni_sendheartbeat: chn%d recv success\n",chn);
	
	if((recv_h.nFlag == 9000) && (recv_h.nCommand == NETCMD_KEEP_ALIVE))
	{
		//printf("anni_sendheartbeat: chn%d heartbeat success\n",chn);
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
		return 0;
	}
	
	pthread_mutex_unlock(&g_annicamera_info[chn].lock);
	
	printf("anni_sendheartbeat: chn%d heartbeat failed\n",chn);
	
	return -1;
}

void* Thread_keep_alive(void* pParam)
{
	int count = g_anni_client_count/2;
	
	int ret = pthread_detach(pthread_self());
	if(ret < 0)
	{
		perror("pthread_detach");
		//pthread_exit((void *)0);
	}
	else
	{
		printf("Thread_keep_alive detach ok\n");
	}
	
	while(g_init_flag)
	{
		//printf("Thread_keep_alive: loop\n");
		
		int i = 0;
		for(i = 0; i < count; i++)
		{
			//printf("chn%d sendheartbeat...\n",i);
			ret = anni_sendheartbeat(i);
			if(ret < 0)
			{
				anni_logout(i);
			}
		}
		
		sleep(5);
	}
	
	return 0;
}

int Anni_Init(unsigned int max_client_num)//chn*2  主/子码流
{
	//dbg("Anni_Init");
	
	if(max_client_num <= 0)
	{
		return -1;
	}
	
	if(g_init_flag)
	{
		return 0;
	}
	
	g_anni_client_count = max_client_num;
	
	g_annic_info = (anni_client_info *)malloc(g_anni_client_count*sizeof(anni_client_info));
	if(g_annic_info == NULL)
	{
		return -1;
	}
	memset(g_annic_info,0,g_anni_client_count*sizeof(anni_client_info));
	
	int i = 0;
	for(i = 0; i < (int)g_anni_client_count; i++)
	{
		g_annic_info[i].cap_fd = -1;
		g_annic_info[i].eventLoopWatchVariable = 0;
		
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_annic_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	
	g_annicamera_info = (anni_camera_info *)malloc(g_anni_client_count/2*sizeof(anni_camera_info));
	if(g_annicamera_info == NULL)
	{
		free(g_annic_info);
		g_annic_info = NULL;
		return -1;
	}
	memset(g_annicamera_info,0,g_anni_client_count/2*sizeof(anni_camera_info));
	
	for(i = 0; i < (int)g_anni_client_count/2; i++)
	{
		g_annicamera_info[i].cmd_fd = -1;
		g_annicamera_info[i].dwServerIndex = 0;
		g_annicamera_info[i].kp_alv_lost_cnt = 0;
		
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_annicamera_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	
	g_init_flag = 1;
	
	return 0;
}

int Anni_DeInit()
{
	return 0;
}

//return value : 1 - Link; 0 - Lost
int Anni_GetLinkStatus(int chn)
{
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return 0;
	}
	
	int status = 0;
	
	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	status = (g_annic_info[chn].cap_fd != -1);
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return status;
}

int Anni_Stop(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}
	
	if(g_annic_info[chn].pid != 0)
	{
		//printf("anni_Stop::Stop-1\n");
		
		g_annic_info[chn].eventLoopWatchVariable = 1;
		
		//printf("anni_Stop::Stop-2\n");
		
		pthread_join(g_annic_info[chn].pid, NULL);
		
		//printf("anni_Stop::Stop-3\n");
	}
	
	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	g_annic_info[chn].pid = 0;
	g_annic_info[chn].eventLoopWatchVariable = 0;
	
	if(g_annic_info[chn].cap_fd != -1)
	{
		close(g_annic_info[chn].cap_fd);
		g_annic_info[chn].cap_fd = -1;
	}
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return 0;
}

typedef struct
{
	BYTE *frame_buf;
	DWORD frame_size;//一帧图像大小，如果一帧图像大于８K，安尼IPC会拆包发送。组装
	BYTE *pkg_buf;//用于接收一包数据，现在分配8K大小，以后根据安尼调整
	DWORD pkg_size;//当前收到数据包大小
	DWORD cur_frame_offset;//标示当前帧已收到的数据大小  等于frame_size，表示一帧接收完成
	int frame_start;//一帧数据接收开始
	//int frame_end;//一帧数据接收结束
	WORD frame_index;//帧索引
	DWORD frame_timestamp;//时间戳
	BYTE is_I_frame;
}frame_info;

extern int DoStreamStateCallBack(int chn, real_stream_state_e msg);

void* ThreadPROCAnni(void* pParam)
{
	unsigned int time_out_count = 0;
	
	struct timeval timeout;
	real_stream_s stream;
	
	NET_DATA_HEAD ndh;
	DATA_PACKET *pdp = NULL;
	BYTE *enc_buf = NULL;
	int enc_len = 0;
	int max_fd_num = 0;
	int ret = 0;
	
	unsigned char byFirstRecv = 1;
	
	frame_info fi;
	memset(&fi, 0, sizeof(fi));//清零
	
	int chn = (int)pParam;
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		goto ProcQuit;
	}
	
	if(chn < (int)(g_anni_client_count/2))
	{
		enc_len = 500*1024;//400*1024;
	}
	else
	{
		enc_len = 200*1024;
	}
	
	enc_buf = (BYTE *)malloc(enc_len);
	if(enc_buf == NULL)
	{
		goto ProcOver;
	}
	
	fi.frame_buf = enc_buf;
	
	pdp = (DATA_PACKET *)malloc(sizeof(DATA_PACKET));
	if(pdp == NULL)
	{
		free(enc_buf);
		enc_buf = NULL;
		goto ProcOver;
	}
	
	while(g_init_flag)
	{
		if(g_annic_info[chn].eventLoopWatchVariable)
		{
			break;
		}
		
		if(g_annic_info[chn].cap_fd == -1)
		{
			break;
		}
		
		max_fd_num = g_annic_info[chn].cap_fd;
		
		fd_set set;
		FD_ZERO(&set);
		FD_SET(g_annic_info[chn].cap_fd, &set);
		
		//linux平台下timeout会被修改以表示剩余时间,故每次都要重新赋值
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		ret = select(max_fd_num+1,&set,NULL,NULL,&timeout);
		if(ret == 0)
		{
			//csp modify 20131111
			//if(++time_out_count > 10)
			if(++time_out_count > 5)
			{
				//printf("ThreadPROCAnni:chn%d select sock timeout & quit!!!\n",chn);
				time_out_count = 0;
				break;
			}
			else
			{
				//printf("ThreadPROCAnni:chn%d select sock timeout & continue!\n",chn);
				continue;
			}
		}
		if(ret < 0)
		{
			//printf("ThreadPROCAnni:chn%d select sock error:[%s]!\n",chn,strerror(errno));
			break;
		}
		if(g_annic_info[chn].cap_fd != INVALID_SOCKET && FD_ISSET(g_annic_info[chn].cap_fd,&set))
		{
			time_out_count = 0;
			
			ret = recv_data(g_annic_info[chn].cap_fd, (BYTE *)&ndh, sizeof(ndh));
			if(ret != 0)
			{
				//printf("chn%d recv NET_DATA_HEAD error\n",chn);
				break;
			}
			
			if(ndh.nFlag != 9000)
			{
				printf("###chn%d: ndh.nFlag != 9000###\n", chn);
				memset(&fi, 0, sizeof(fi));//清零
				fi.frame_buf = enc_buf;
				break;//continue;
			}
			
			if(byFirstRecv)
			{
				if(ndh.nSize == 5004 || ndh.nSize == 5005 || ndh.nSize == 5006 || ndh.nSize == 5018 || ndh.nSize == 5019)
				{
					printf("chn%d anni video request failed??????\n",chn);
				}
				byFirstRecv = 0;
			}
			
			ret = recv_data(g_annic_info[chn].cap_fd, (BYTE *)pdp, ndh.nSize);
			if(ret != 0)
			{
				//printf("chn%d recv DATA_PACKET error\n",chn);
				break;
			}
			
			/*printf("chn%d: nFlag: %ld nSize:%ld dwVideoSize: %ld wBufSize: %d wFrameIndex: %d dwTimeTick: %lu wMotionDetect: %d byKeyFrame: %d wIsSampleHead: %d\n", 
				chn, 
				ndh.nFlag, 
				ndh.nSize, 
				pdp->stFrameHeader.dwVideoSize, 
				pdp->wBufSize, 
				pdp->stFrameHeader.wFrameIndex, 
				pdp->stFrameHeader.dwTimeTick, 
				pdp->stFrameHeader.wMotionDetect, 
				pdp->stFrameHeader.byKeyFrame, 
				pdp->wIsSampleHead);*/
			
			if(pdp->wIsSampleHead)//为数据包头，1：包头
			{
				if(fi.cur_frame_offset == fi.frame_size && fi.frame_size != 0)//一帧 ok
				{
					//printf("chn%d: all right\n", chn);
					
					if(g_annic_info[chn].pStreamCB != NULL)
					{
						memset(&stream, 0, sizeof(stream));
						stream.chn = chn;
						stream.data = fi.frame_buf;
						stream.len = fi.frame_size;
						stream.pts = fi.frame_timestamp;
						stream.pts *= 1000;
						stream.media_type = MEDIA_PT_H264;
						if(fi.is_I_frame)
						{
							stream.frame_type = REAL_FRAME_TYPE_I;
						}
						else
						{
							stream.frame_type = REAL_FRAME_TYPE_P;
						}
						stream.rsv = 0;
						stream.mdevent = 0;
						stream.width = g_annic_info[chn].video_width;
						stream.height = g_annic_info[chn].video_height;
						//printf("chn%d is_I_frame: %d frame_index: %d frame_timestamp: %lu width * height: %d * %d\n", chn, fi.is_I_frame, fi.frame_index, fi.frame_timestamp, stream.width, stream.height);
						g_annic_info[chn].pStreamCB(&stream, g_annic_info[chn].dwContext);
					}
				}
				
				memset(&fi, 0, sizeof(fi));//清零
				fi.frame_buf = enc_buf;
				
				fi.pkg_buf = pdp->byPackData;
				fi.pkg_size = pdp->wBufSize;
				
				//为新一帧赋值
				//if(pdp->stFrameHeader.dwVideoSize <= (DWORD)enc_len)
				if(pdp->wBufSize <= enc_len)
				{
					fi.frame_start = 1;
					fi.frame_size = pdp->wBufSize;//pdp->stFrameHeader.dwVideoSize;
					fi.cur_frame_offset = 0;
					fi.frame_index = pdp->stFrameHeader.wFrameIndex;
					fi.frame_timestamp = pdp->stFrameHeader.dwTimeTick;
					fi.is_I_frame = pdp->stFrameHeader.byKeyFrame;//1: I帧 0: P帧
					memcpy(fi.frame_buf+fi.cur_frame_offset, pdp->byPackData, pdp->wBufSize);//填充
					fi.cur_frame_offset += pdp->wBufSize;
					//printf("chn%d: frame_size: %ld pkg_size: %ld frame_index: %d frame_timestamp: %lu\n", chn, fi.frame_size, fi.pkg_size, fi.frame_index, fi.frame_timestamp);
					//unsigned char *p = pdp->byPackData;
					//printf("chn%d wIsSampleHead: %d data: (%02x %02x %02x %02x %02x)\n",chn,pdp->wIsSampleHead,p[0],p[1],p[2],p[3],p[4]);
				}
				else
				{
					printf("chn%d: frame is too large\n", chn);
					memset(&fi, 0, sizeof(fi));//清零
					fi.frame_buf = enc_buf;
				}
			}
			else//pdp->wIsSampleHead 0：中间包
			{
				fi.pkg_buf = pdp->byPackData;
				fi.pkg_size = pdp->wBufSize;
				
				//查看时间戳//查看索引//查看数据大小
				if(fi.frame_start && 
					fi.frame_timestamp == pdp->stFrameHeader.dwTimeTick && 
					fi.frame_index == pdp->stFrameHeader.wFrameIndex && 
					fi.is_I_frame == pdp->stFrameHeader.byKeyFrame && 
					/*fi.frame_size == pdp->stFrameHeader.dwVideoSize && */
					/*fi.cur_frame_offset + pdp->wBufSize <= fi.frame_size && */
					fi.cur_frame_offset + pdp->wBufSize <= (DWORD)enc_len)
				{
					memcpy(fi.frame_buf+fi.cur_frame_offset, pdp->byPackData, pdp->wBufSize);//填充
					fi.cur_frame_offset += pdp->wBufSize;
					fi.frame_size += pdp->wBufSize;
					//printf("chn%d: add_size: %ld cur_frame_offset: %ld\n", chn, fi.pkg_size, fi.cur_frame_offset);
				}
				else
				{
					printf("chn%d: packet error\n", chn);
					memset(&fi, 0, sizeof(fi));//清零
					fi.frame_buf = enc_buf;
				}
			}
		}
		else
		{
			if(++time_out_count > 10)
			{
				printf("ThreadPROCAnni:chn%d unknown sock error & quit!!!\n",chn);
				time_out_count = 0;
				break;
			}
		}
	}
	
	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	g_annic_info[chn].pid = 0;
	g_annic_info[chn].eventLoopWatchVariable = 0;
	
	if(g_annic_info[chn].cap_fd != -1)
	{
		close(g_annic_info[chn].cap_fd);
		g_annic_info[chn].cap_fd = -1;
	}
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	free(pdp);
	pdp = NULL;
	
	fi.frame_buf = NULL;
	
	free(enc_buf);
	enc_buf = NULL;
	
ProcOver:
	DoStreamStateCallBack(chn, REAL_STREAM_STATE_LOST);
ProcQuit:
	pthread_detach(pthread_self());
	return 0;
}

int Anni_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	int ret = 0;
	
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)g_anni_client_count)
	{
		return -1;
	}
	
	if(!g_sdk_inited)
	{
		if(pthread_create(&keep_alive_pid, NULL, Thread_keep_alive, NULL) < 0)
		{
			perror("create anni keep alive thread failure");
			return -1;
		}
		
		g_sdk_inited = 1;
	}
	
	int real_chn = (chn < (int)(g_anni_client_count/2)) ? chn : (chn - (int)(g_anni_client_count/2));
	printf("Anni_Start: chn=%d real_chn=%d\n", chn, real_chn);
	
	if(Anni_GetLinkStatus(chn))
	{
		Anni_Stop(chn);
	}
	
	if(anni_login(chn, dwIp, wPort, user, pwd) < 0)
	{
		printf("chn%d Anni_Start failed:unlogin\n",chn);
		return -1;
	}
	
	WORD wErrorCode = 0;
	int sock = ConnectWithTimeout(dwIp, wPort, 5000, &wErrorCode);
	if(sock < 0)
	{
		printf("chn%d connect error:(0x%08x,%d),err:(%d,%s)\n",chn,dwIp,wPort,errno,strerror(errno));
		return -1;
	}
	
	printf("chn%d Anni_Start connect vedio success\n",chn);
	
	//请求视频码流
	ACCEPT_HEAD accepthead;
	accepthead.nFlag = 9000;
	accepthead.nSockType = 1;// 1: 视频实时网络连接
	ret = send_data(sock, (BYTE *)&accepthead, sizeof(accepthead));
	if(ret == FAILURE)
	{
		dbg("chn%d Anni_Start vedio socket TCP socket send head",chn);
		close(sock);
		sock = -1;
		return -1;
	}
	printf("chn%d Anni_Start send ACCEPT_HEAD ok\n",chn);
	
	OPEN_HEAD open_req;
	memset(&open_req, 0, sizeof(open_req));
	open_req.nFlag = 9000;
	open_req.nID = g_annicamera_info[real_chn].dwServerIndex;
	//printf("nID: %ld\n", open_req.nID);
	open_req.nProtocolType = PROTOCOL_TCP;
	open_req.nSerChannel = 0;
	if(chn < (int)(g_anni_client_count/2))
	{
		open_req.nStreamType = 0;//主码流
	}
	else
	{
		open_req.nStreamType = 1;//子码流
	}
	ret = send_data(sock, (BYTE *)&open_req, sizeof(open_req));
	if(ret == FAILURE)
	{
		printf("chn%d stream request cmd error\n",chn);
		close(sock);
		sock = -1;
		return -1;
	}
	printf("chn%d Anni_Start send OPEN_HEAD ok\n",chn);
	
	struct timeval tv;
	tv.tv_sec = 6;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	
	pthread_mutex_lock(&g_annic_info[chn].lock);
	
	g_annic_info[chn].cap_fd = sock;
	g_annic_info[chn].pStreamCB = pCB;
	g_annic_info[chn].dwContext = dwContext;
	if(chn < (int)(g_anni_client_count/2))
	{
		g_annic_info[chn].video_width = g_annicamera_info[real_chn].dwStream1Width;
		g_annic_info[chn].video_height = g_annicamera_info[real_chn].dwStream1Height;
	}
	else
	{
		g_annic_info[chn].video_width = g_annicamera_info[real_chn].dwStream2Width;
		g_annic_info[chn].video_height = g_annicamera_info[real_chn].dwStream2Height;
	}
	g_annic_info[chn].eventLoopWatchVariable = 0;
	
	if(pthread_create(&g_annic_info[chn].pid, NULL, ThreadPROCAnni, (void *)chn) != 0)
	{
		g_annic_info[chn].pid = 0;
		Anni_Stop(chn);
		pthread_mutex_unlock(&g_annic_info[chn].lock);
		return -1;
	}
	
	printf("chn%d Anni_Start success\n",chn);
	
	pthread_mutex_unlock(&g_annic_info[chn].lock);
	
	return 0;
}

int Anni_CMD_Open(int chn)
{
	return 0;
}

int Anni_CMD_Close(int chn)
{
	return 0;
}

int Anni_CMD_SetImageParam(int chn, video_image_para_t *para)
{
	return 0;
}

int Anni_CMD_GetImageParam(int chn, video_image_para_t *para)
{
	return -1;
}

int Anni_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	DMS_NET_PTZ_CONTROL_V0 ptzctrl;
	memset(&ptzctrl, 0, sizeof(ptzctrl));
	
	DWORD dwSize = sizeof(DMS_NET_PTZ_CONTROL_V0);
	DWORD dwChannel = 0;
	DWORD dwPTZCmd = 0;
	unsigned int dwValue = 0;
	DWORD dwPTZCmd2 = 0;
	unsigned int dwValue2 = 0;
	
	if(cmd == EM_PTZ_CMD_START_TILEUP)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_UP;
		dwValue = (tiltSpeed - 1) / 2;
	}
	else if(cmd == EM_PTZ_CMD_START_TILEDOWN)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_DOWN;
		dwValue = (tiltSpeed - 1) / 2;
	}
	else if(cmd == EM_PTZ_CMD_START_PANLEFT)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_LEFT;
		dwValue = (panSpeed - 1) / 2;
	}
	else if(cmd == EM_PTZ_CMD_START_PANRIGHT)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_RIGHT;
		dwValue = (panSpeed - 1) / 2;
	}
	else if(cmd == EM_PTZ_CMD_START_LEFTUP)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_UP;
		dwValue = (tiltSpeed - 1) / 2;
		dwPTZCmd2 = DMS_PTZ_CMD_LEFT;
		dwValue2 = dwValue;
	}
	else if(cmd == EM_PTZ_CMD_START_LEFTDOWN)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_DOWN;
		dwValue = (tiltSpeed - 1) / 2;
		dwPTZCmd2 = DMS_PTZ_CMD_LEFT;
		dwValue2 = dwValue;
	}
	else if(cmd == EM_PTZ_CMD_START_RIGHTUP)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_UP;
		dwValue = (tiltSpeed - 1) / 2;
		dwPTZCmd2 = DMS_PTZ_CMD_RIGHT;
		dwValue2 = dwValue;
	}
	else if(cmd == EM_PTZ_CMD_START_RIGHTDOWN)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_DOWN;
		dwValue = (tiltSpeed - 1) / 2;
		dwPTZCmd2 = DMS_PTZ_CMD_RIGHT;
		dwValue2 = dwValue;
	}
	else if(cmd == EM_PTZ_CMD_START_ZOOMTELE)
	{
		dwPTZCmd = DMS_PTZ_CMD_ZOOM_ADD;
	}
	else if(cmd == EM_PTZ_CMD_START_ZOOMWIDE)
	{
		dwPTZCmd = DMS_PTZ_CMD_ZOOM_SUB;
	}
	else if(cmd == EM_PTZ_CMD_START_FOCUSNEAR)
	{
		dwPTZCmd = DMS_PTZ_CMD_FOCUS_ADD;
	}
	else if(cmd == EM_PTZ_CMD_START_FOCUSFAR)
	{
		dwPTZCmd = DMS_PTZ_CMD_FOCUS_SUB;
	}
	else if(cmd == EM_PTZ_CMD_START_IRISSMALL)
	{
		dwPTZCmd = DMS_PTZ_CMD_IRIS_SUB;
	}
	else if(cmd == EM_PTZ_CMD_START_IRISLARGE)
	{
		dwPTZCmd = DMS_PTZ_CMD_IRIS_ADD;
	}
	else if(cmd >= EM_PTZ_CMD_STOP_TILEUP && cmd < EM_PTZ_CMD_PRESET_SET)
	{
		dwPTZCmd = DMS_PTZ_CMD_STOP;
	}
	else if(cmd == EM_PTZ_CMD_PRESET_SET)
	{
		unsigned int preset = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_PRESET;
		dwValue = preset;
	}
	else if(cmd == EM_PTZ_CMD_PRESET_GOTO)
	{
		unsigned int preset = *((unsigned int *)data);
		dwPTZCmd = DMS_PTZ_CMD_CALL;
		dwValue = preset;
	}
	else if(cmd == EM_PTZ_CMD_PRESET_CLEAR)
	{
		//unsigned int preset = *((unsigned int *)data);
	}
	else if(cmd == EM_PTZ_CMD_LIGHT_ON)
	{
		dwPTZCmd = DMS_PTZ_CMD_LIGHT_OPEN;
	}
	else if(cmd == EM_PTZ_CMD_LIGHT_OFF)
	{
		dwPTZCmd = DMS_PTZ_CMD_LIGHT_CLOSE;
	}
	else if(cmd == EM_PTZ_CMD_AUX_ON)
	{
		unsigned int id = *((unsigned int *)data);
		if(id == 1)//默认1号辅助为雨刷
		{
			dwPTZCmd = DMS_PTZ_CMD_BRUSH_START;
		}
	}
	else if(cmd == EM_PTZ_CMD_AUX_OFF)
	{
		unsigned int id = *((unsigned int *)data);
		if(id == 1)//默认1号辅助为雨刷
		{
			dwPTZCmd = DMS_PTZ_CMD_BRUSH_STOP;
		}
	}
	else if(cmd == EM_PTZ_CMD_AUTOPAN_ON)
	{
		dwPTZCmd = DMS_PTZ_CMD_AUTO_STRAT;
	}
	else if(cmd == EM_PTZ_CMD_AUTOPAN_OFF)
	{
		dwPTZCmd = DMS_PTZ_CMD_AUTO_STOP;
	}
	
	if(dwPTZCmd != 0)
	{
		if(anni_login(chn, ipcam.dwIp, ipcam.wPort, ipcam.user, ipcam.pwd) < 0)
		{
			printf("chn%d Anni_CMD_PtzCtrl failed:unlogin\n",chn);
			return -1;
		}
		
		pthread_mutex_lock(&g_annicamera_info[chn].lock);
		
		int sock = g_annicamera_info[chn].cmd_fd;
		
		DWORD ptzcmd[2] = {dwPTZCmd, dwPTZCmd2};
		unsigned int ptzparam[2] = {dwValue, dwValue2};
		
		int i = 0;
		for(i = 0; i < 2; i++)
		{
			if(ptzcmd[i] == 0)
			{
				break;
			}
			
			COMM_HEAD commhead;
			commhead.nFlag = 9000;
			commhead.nCommand = 0x00000050;
			commhead.nChannel = 0;
			commhead.nErrorCode = 0;
			commhead.nBufSize = dwSize;
			int ret = send_data(sock, (BYTE *)&commhead, sizeof(commhead));
			if(ret == FAILURE)
			{
				dbg("chn%d Anni_CMD_PtzCtrl send COMM_HEAD failed",chn);
				pthread_mutex_unlock(&g_annicamera_info[chn].lock);
				anni_logout(chn);
				return -1;
			}
			else
			{
				printf("chn%d Anni_CMD_PtzCtrl send COMM_HEAD ok\n",chn);
			}
			
			ptzctrl.dwSize = dwSize;
			ptzctrl.dwChannel = dwChannel;
			ptzctrl.dwPTZCommand = ptzcmd[i];
			ptzctrl.nPTZParam = ptzparam[i];
			ret = send_data(sock, (BYTE *)&ptzctrl, sizeof(ptzctrl));
			if(ret == FAILURE)
			{
				dbg("chn%d Anni_CMD_PtzCtrl send cmd failed",chn);
				pthread_mutex_unlock(&g_annicamera_info[chn].lock);
				anni_logout(chn);
				return -1;
			}
			else
			{
				printf("chn%d Anni_CMD_PtzCtrl send cmd ok\n",chn);
			}
			
			/*if(wait_fd_event(sock, 4000) <= 0)
			{
				dbg("chn%d wait_fd_event failed",chn);
			}
			else
			{
				dbg("chn%d wait_fd_event success",chn);
			}*/
			
			printf("chn%d Anni_CMD_PtzCtrl success,PTZCmd=0x%ld & PTZParam=%d\n",chn,ptzctrl.dwPTZCommand,ptzctrl.nPTZParam);
		}
		
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
	}
	
	return 0;
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

int Anni_CMD_SetTime(int chn, time_t t, int force)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!force)
	{
		if(!ipcam.enable || !Anni_GetLinkStatus(chn))
		{
			//printf("Anni_CMD_SetTime: chn%d vlost!!!\n",chn);
			return -1;
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
	
	/*char date[64];
	sprintf(date,"%04d.%02d.%02d.%02d.%02d.%02d",
		now.tm_year+1900,
		now.tm_mon+1,
		now.tm_mday,
		now.tm_hour,
		now.tm_min,
		now.tm_sec);*/
	
	if(anni_login(chn, ipcam.dwIp, ipcam.wPort, ipcam.user, ipcam.pwd) < 0)
	{
		printf("chn%d Anni_CMD_SetTime failed:unlogin\n",chn);
		return -1;
	}
	
	pthread_mutex_lock(&g_annicamera_info[chn].lock);
	
	int sock = g_annicamera_info[chn].cmd_fd;
	
	COMM_HEAD commhead;
	commhead.nFlag = 9000;
	commhead.nCommand = 0x00000005;
	commhead.nChannel = 0;
	commhead.nErrorCode = 0;
	commhead.nBufSize = sizeof(DMS_SYSTEMTIME);
	int ret = send_data(sock, (BYTE *)&commhead, sizeof(commhead));
	if(ret == FAILURE)
	{
		dbg("chn%d Anni_CMD_SetTime send COMM_HEAD failed",chn);
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
		anni_logout(chn);
		return -1;
	}
	else
	{
		printf("chn%d Anni_CMD_SetTime send COMM_HEAD ok\n",chn);
	}
	
	DMS_SYSTEMTIME annitime;
	memset(&annitime, 0, sizeof(annitime));
	annitime.wYear = now.tm_year+1900;
	annitime.wMonth = now.tm_mon+1;
	annitime.wDayOfWeek = now.tm_wday;
	annitime.wDay = now.tm_mday;
	annitime.wHour = now.tm_hour;
	annitime.wMinute = now.tm_min;
	annitime.wSecond = now.tm_sec;
	annitime.wMilliseconds = 0;
	ret = send_data(sock, (BYTE *)&annitime, sizeof(annitime));
	if(ret == FAILURE)
	{
		dbg("chn%d Anni_CMD_SetTime send DMS_SYSTEMTIME failed",chn);
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
		anni_logout(chn);
		return -1;
	}
	else
	{
		printf("chn%d Anni_CMD_SetTime send DMS_SYSTEMTIME ok\n",chn);
	}
	
	/*if(wait_fd_event(sock, 4000) <= 0)
	{
		dbg("chn%d wait_fd_event failed",chn);
	}
	else
	{
		dbg("chn%d wait_fd_event success",chn);
	}*/
	
	pthread_mutex_unlock(&g_annicamera_info[chn].lock);
	
	return 0;
}

int Anni_CMD_Reboot(int chn)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(anni_login(chn, ipcam.dwIp, ipcam.wPort, ipcam.user, ipcam.pwd) < 0)
	{
		printf("chn%d Anni_CMD_Reboot failed:unlogin\n",chn);
		return -1;
	}
	
	pthread_mutex_lock(&g_annicamera_info[chn].lock);
	
	int sock = g_annicamera_info[chn].cmd_fd;
	
	COMM_HEAD commhead;
	commhead.nFlag = 9000;
	commhead.nCommand = 0x00000001;
	commhead.nChannel = 0;
	commhead.nErrorCode = 0;
	commhead.nBufSize = 0;
	int ret = send_data(sock, (BYTE *)&commhead, sizeof(commhead));
	if(ret == FAILURE)
	{
		dbg("chn%d Anni_CMD_Reboot send COMM_HEAD failed",chn);
		pthread_mutex_unlock(&g_annicamera_info[chn].lock);
		anni_logout(chn);
		return -1;
	}
	else
	{
		printf("chn%d Anni_CMD_Reboot send COMM_HEAD ok\n",chn);
	}
	
	/*if(wait_fd_event(sock, 4000) <= 0)
	{
		dbg("chn%d wait_fd_event failed",chn);
	}
	else
	{
		dbg("chn%d wait_fd_event success",chn);
	}*/
	
	pthread_mutex_unlock(&g_annicamera_info[chn].lock);
	
	anni_logout(chn);
	
	printf("Anni_CMD_Reboot success\n");
	
	return 0;
}

int Anni_CMD_RequestIFrame(int chn)
{
	return 0;
}

