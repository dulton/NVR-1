#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "river.h"

extern int IPC_Find(ipc_node* head, ipc_node *pNode);

#ifdef __cplusplus
extern "C" {
#endif

#include "param.h"
#include "Net.h"

#ifdef __cplusplus
}
#endif

INT LogonNotifyCallback(UINT dwMsgID,UINT ip,UINT port,HANDLE hNotify,void *pPar)
{
	printf("LogonNotifyCallback\n");
	return 0;
}

INT CheckUserPswCallback(const CHAR *pUserName,const CHAR *pPsw)
{
	printf("Check user:%s,psw:%s\n",pUserName,pPsw);
	return 3;
}

INT UpdateFileCallback(INT nOperation,INT hsock,ULONG ip,ULONG port,INT nUpdateType,CHAR *pFileName,CHAR *pFileData,INT nFileLen)
{
	printf("UpdateFileCallback\n");
	return 0;
}

INT ServerMsgCmdCallback(ULONG ip,ULONG port,CHAR *pMsgHd)
{
	printf("ServerMsgCmdCallback\n");	
	return 0;
}

INT StreamWriteCheckCallback(INT nOperation,const CHAR *pUserName,const CHAR *pPsw,ULONG ip,ULONG port,OPEN_VIEWPUSH_INFO viewPushInfo,HANDLE hOpen)
{
	//printf("StreamWriteCheckCallback\n");
	return 0;
}

INT PreviewStreamCallback(HANDLE hOpenChannel,void *pStreamData,UINT dwClientID,void *pContext,ENCODE_VIDEO_TYPE encodeVideoType)
{
	printf("PreviewStreamCallback\n");
	return 0;
}

static ipc_node *g_river_head = NULL;
static ipc_node *g_river_tail = NULL;

typedef struct river_unit
{
	unsigned char channel_no;
	unsigned char enable;
	unsigned char stream_type;
	unsigned char trans_type;
	unsigned int protocol_type;
	unsigned int dwIp;
	unsigned short wPort;
	unsigned char force_fps;
	unsigned char frame_rate;
	char user[32];
	char pwd[32];
	char name[32];
	char uuid[64];
	char address[64];//onvifÊ¹ÓÃ
	char ipc_type;
	
	//char reserved[47];
	char reserved[33];
	unsigned char macAddr[6];
	unsigned int ipMultiAddr;
	unsigned short wMultiPort;
	unsigned short wPortWeb;
	
	unsigned int net_mask;
	unsigned int net_gateway;
	unsigned int dns1;
	unsigned int dns2;
}river_unit;

INT RIVER_CallbackServerFind(SEARCH_SER_INFO *pSearchInfo)
{
	if(pSearchInfo == NULL)
	{
		printf("CallbackServerFind: param error\n");
		return -1;
	}
	
	//printf("ip = 0x%08lx, port = %d, user = %s, pwd = %s, szDeviceName = %s\n", pSearchInfo->ipLocal, pSearchInfo->wPortListen, pSearchInfo->userName, pSearchInfo->userPassword, pSearchInfo->szDeviceName);
	printf("ip = 0x%08lx, port = %d, szDeviceName = %s\n", pSearchInfo->ipLocal, pSearchInfo->wPortListen, pSearchInfo->szDeviceName);
	
	if(1)
	{
		ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
		if(pNode == NULL)
		{
			printf("Not enough space to save new ipc info.\n");
			return -1;
		}
		
		memset(pNode, 0, sizeof(*pNode));
		pNode->next = NULL;
		
		unsigned char *p = pSearchInfo->macAddr;
		printf("RIVER IPC mac:(%02x:%02x:%02x:%02x:%02x:%02x),MultiAddr:0x%08lx,port:(%d,%d,%d),\n",p[0],p[1],p[2],p[3],p[4],p[5],pSearchInfo->ipMultiAddr,pSearchInfo->wPortWeb,pSearchInfo->wPortListen,pSearchInfo->wMultiPort);
		
		//strncpy(pNode->ipcam.address, pSearchInfo->szDeviceName, sizeof(pNode->ipcam.address)-1);
		//memcpy(pNode->ipcam.address, pSearchInfo->macAddr, sizeof(pSearchInfo->macAddr));
		sprintf(pNode->ipcam.address, "%02x:%02x:%02x:%02x:%02x:%02x", p[0],p[1],p[2],p[3],p[4],p[5]);
		//strcpy(pNode->ipcam.user, pSearchInfo->userName);
		//strcpy(pNode->ipcam.pwd, pSearchInfo->userPassword);
		strcpy(pNode->ipcam.user, "admin");
		strcpy(pNode->ipcam.pwd, "admin");
		pNode->ipcam.channel_no = 0;
		pNode->ipcam.enable = 0;
		pNode->ipcam.ipc_type = IPC_TYPE_720P;
		pNode->ipcam.protocol_type = PRO_TYPE_RIVER;
		pNode->ipcam.stream_type = STREAM_TYPE_MAIN;
		pNode->ipcam.trans_type = TRANS_TYPE_TCP;
		pNode->ipcam.force_fps = 0;
		pNode->ipcam.frame_rate = 30;
		pNode->ipcam.dwIp = htonl(pSearchInfo->ipLocal);
		pNode->ipcam.wPort = pSearchInfo->wPortListen;
		//strcpy(pNode->ipcam.uuid, pSearchInfo->szDeviceName);
		sprintf(pNode->ipcam.uuid, "%02x:%02x:%02x:%02x:%02x:%02x", p[0],p[1],p[2],p[3],p[4],p[5]);
		strncpy(pNode->ipcam.name, pSearchInfo->szDeviceName, sizeof(pNode->ipcam.name)-1);
		pNode->ipcam.net_mask = htonl(pSearchInfo->ipSubMask);
		pNode->ipcam.net_gateway = htonl(pSearchInfo->ipGateway);
		pNode->ipcam.dns1 = htonl(pSearchInfo->ipDnsAddr);
		pNode->ipcam.dns2 = inet_addr("4.4.4.4");
		
		river_unit *pRiverUnit = (river_unit *)(&pNode->ipcam);
		pRiverUnit->ipMultiAddr = htonl(pSearchInfo->ipMultiAddr);
		pRiverUnit->wMultiPort = pSearchInfo->wMultiPort;
		pRiverUnit->wPortWeb = pSearchInfo->wPortWeb;
		memcpy(pRiverUnit->macAddr, pSearchInfo->macAddr, sizeof(pRiverUnit->macAddr));
		
		if(g_river_head == NULL)
		{
			g_river_head = pNode;
			g_river_tail = pNode;
		}
		else
		{
			g_river_tail->next = pNode;
			g_river_tail = pNode;
		}
	}
	
	return 0;
}

int RIVER_Search(ipc_node** head, ipc_node **tail, unsigned char check_conflict)
{
	if(head == NULL || tail == NULL)
	{
		printf("RIVER_Search: param error\n");
		return -1;
	}
	
	int count = 0;
	
	unsigned char conflict_flag = 0;
	
	g_river_head = g_river_tail = NULL;
	
	printf("search yellow river**************************\n");
	
	int ret = NET_SearchAllServer(2, RIVER_CallbackServerFind);
	if(ret != ERR_SUCCESS)
	{
		if(ret != ERR_TIME_OUT)
		{
			printf("NET_SearchAllServer Erro: %d\n", ret);
		}
		//return -1;
	}
	else
	{
		printf("NET_SearchAllServer finish.\n");
	}
	
	ipc_node *pNode = g_river_head;
	while(pNode)
	{
		int rtn = IPC_Find(*head, pNode);
		if(rtn == 1)
		{
			printf("ipc conflict : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
			conflict_flag = 1;
		}
		else if(rtn == 2)
		{
			printf("ipc repeat : [%s,0x%08x,%d,%s]\n", pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
			ipc_node *p = pNode;
			pNode = pNode->next;
			free(p);
			p = NULL;
			continue;
		}
		
		count++;
		printf("river-ipc%d : [%s,0x%08x,%d,%s]\n", count, pNode->ipcam.address, pNode->ipcam.dwIp, pNode->ipcam.wPort, pNode->ipcam.uuid);
		
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
		
		pNode = pNode->next;
		(*tail)->next = NULL;
	}
	
	g_river_head = g_river_tail = NULL;
	
	return count;
}

#define INVALID_HANDLE	NULL

typedef struct
{
	HANDLE hLogonServer;
	HANDLE hOpenChannel;
	RealStreamCB pStreamCB;
	unsigned int dwContext;
	pthread_mutex_t lock;
}river_client_info;

static river_client_info *g_riverc_info = NULL;

static unsigned int g_river_client_count = 0;

static unsigned char g_init_flag = 0;

static unsigned char g_sdk_inited = 0;

int RIVER_Init(unsigned int max_client_num)
{
	if(max_client_num <= 0)
	{
		return -1;
	}
	
	g_river_client_count = max_client_num;
	
	g_riverc_info = (river_client_info *)malloc(g_river_client_count*sizeof(river_client_info));
	if(g_riverc_info == NULL)
	{
		return -1;
	}
	memset(g_riverc_info,0,g_river_client_count*sizeof(river_client_info));
	
	int i = 0;
	for(i = 0; i < (int)g_river_client_count; i++)
	{
		g_riverc_info[i].hLogonServer = INVALID_HANDLE;
		g_riverc_info[i].hOpenChannel = INVALID_HANDLE;
		
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_riverc_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	
	g_init_flag = 1;
	
	return 0;
}

int RIVER_DeInit()
{
	return 0;
}

int RIVER_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
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
	
	ERR_CODE errCode = ERR_FAILURE;
	
	if(!g_sdk_inited)
	{
		errCode = NET_Startup(5000, LogonNotifyCallback, CheckUserPswCallback, UpdateFileCallback, ServerMsgCmdCallback, StreamWriteCheckCallback, (ChannelStreamCallback)PreviewStreamCallback);
		//printf("--->|\tNET_Startup: %s\n",errCode?"Failed!":"Successful!");
		if(errCode)
		{
			printf("NET_Startup failed\n");
			return -1;
		}
		g_sdk_inited = 1;
	}
	
	HANDLE hLogonServer = INVALID_HANDLE;
	
	if(hLogonServer == INVALID_HANDLE)
	{
		char devip[64];
		memset(devip, 0, sizeof(devip));
		
		struct in_addr serv;
		serv.s_addr = ipcam->dwIp;
		sprintf(devip, "%s", inet_ntoa(serv));
		
		errCode = NET_LogonServer(devip, ipcam->wPort, (CHAR *)"admin", ipcam->user, ipcam->pwd, 0, &hLogonServer);
		if(errCode)
		{
			printf("NET_LogonServer failed...\n");
			hLogonServer = INVALID_HANDLE;
			return -1;
		}
	}
	
	NET_CONFIG netconfig;
	UINT s32Size = sizeof(NET_CONFIG);
	memset(&netconfig, 0, s32Size);
	errCode = NET_GetServerConfig(hLogonServer, CMD_GET_NET_CONFIG, (CHAR *)&netconfig, &s32Size, NULL);
	
	NET_LogoffServer(hLogonServer);
	hLogonServer = INVALID_HANDLE;
	
	if(errCode)
	{
		printf("NET_GetServerConfig CMD_GET_NET_CONFIG failed:%d\n",errCode);
		return -1;
	}
	
	printf("NET_GetServerConfig CMD_GET_NET_CONFIG success\n");
	
	unsigned char *p = NULL;
	char address[32];
	
	p = (unsigned char *)&netconfig.IPAddr;
	sprintf(address,"%d.%d.%d.%d",p[3],p[2],p[1],p[0]);
	printf("CMD_GET_NET_CONFIG ip=%s\n",address);
	pnw->ip_address = inet_addr(address);
	
	p = (unsigned char *)&netconfig.SubNetMask;
	sprintf(address,"%d.%d.%d.%d",p[3],p[2],p[1],p[0]);
	printf("CMD_GET_NET_CONFIG netmask=%s\n",address);
	pnw->net_mask = inet_addr(address);
	
	p = (unsigned char *)&netconfig.GateWay;
	sprintf(address,"%d.%d.%d.%d",p[3],p[2],p[1],p[0]);
	printf("CMD_GET_NET_CONFIG gw=%s\n",address);
	pnw->net_gateway = inet_addr(address);
	
	p = (unsigned char *)&netconfig.DNSHostIP;
	sprintf(address,"%d.%d.%d.%d",p[3],p[2],p[1],p[0]);
	printf("CMD_GET_NET_CONFIG dns1=%s\n",address);
	pnw->dns1 = inet_addr(address);
	
	return 0;
}

int RIVER_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
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
	
#if 1
	river_unit *pRiverUnit = (river_unit *)(ipcam);
	char multiaddr[20];
	host.s_addr = pRiverUnit->ipMultiAddr;
	strcpy(multiaddr, inet_ntoa(host));
	
	unsigned char *p = pRiverUnit->macAddr;
	printf("RIVER_SetNetworkParam mac:(%02x:%02x:%02x:%02x:%02x:%02x),param:(%s,%s,%s,%s,%s,%s,%s,%s,%d,%d,%d)\n",p[0],p[1],p[2],p[3],p[4],p[5],ipcam->user,ipcam->pwd,ip,ipcam->name,netmask,gateway,multiaddr,fdnsip,pRiverUnit->wPortWeb,ipcam->wPort,pRiverUnit->wMultiPort);
	
	ERR_CODE errCode = ERR_FAILURE;
	
	/*if(!g_sdk_inited)
	{
		errCode = NET_Startup(5000, LogonNotifyCallback, CheckUserPswCallback, UpdateFileCallback, ServerMsgCmdCallback, StreamWriteCheckCallback, (ChannelStreamCallback)PreviewStreamCallback);
		//printf("--->|\tNET_Startup: %s\n",errCode?"Failed!":"Successful!");
		if(errCode)
		{
			printf("NET_Startup failed\n");
			return -1;
		}
		g_sdk_inited = 1;
	}*/
	
	errCode = NET_ConfigServer((UCHAR *)pRiverUnit->macAddr, ipcam->user, ipcam->pwd, ip, ipcam->name, netmask, gateway, multiaddr, fdnsip, pRiverUnit->wPortWeb, ipcam->wPort, pRiverUnit->wMultiPort);
	if(errCode)
	{
		printf("NET_ConfigServer failed:%d\n",errCode);
	}
	else
	{
		printf("NET_ConfigServer success\n");
	}
	
	return errCode?-1:0;
#else
	ERR_CODE errCode = ERR_FAILURE;
	
	if(!g_sdk_inited)
	{
		errCode = NET_Startup(5000, LogonNotifyCallback, CheckUserPswCallback, UpdateFileCallback, ServerMsgCmdCallback, StreamWriteCheckCallback, (ChannelStreamCallback)PreviewStreamCallback);
		//printf("--->|\tNET_Startup: %s\n",errCode?"Failed!":"Successful!");
		if(errCode)
		{
			printf("NET_Startup failed\n");
			return -1;
		}
		g_sdk_inited = 1;
	}
	
	HANDLE hLogonServer = INVALID_HANDLE;
	
	if(hLogonServer == INVALID_HANDLE)
	{
		char devip[64];
		memset(devip, 0, sizeof(devip));
		
		struct in_addr serv;
		serv.s_addr = ipcam->dwIp;
		sprintf(devip, "%s", inet_ntoa(serv));
		
		errCode = NET_LogonServer(devip, ipcam->wPort, (CHAR *)"admin", ipcam->user, ipcam->pwd, 0, &hLogonServer);
		if(errCode)
		{
			printf("NET_LogonServer failed...\n");
			hLogonServer = INVALID_HANDLE;
			return -1;
		}
	}
	
	NET_CONFIG netconfig;
	UINT s32Size = sizeof(NET_CONFIG);
	memset(&netconfig, 0, s32Size);
	errCode = NET_GetServerConfig(hLogonServer, CMD_GET_NET_CONFIG, (CHAR *)&netconfig, &s32Size, NULL);
	if(errCode)
	{
		printf("NET_GetServerConfig CMD_GET_NET_CONFIG failed:%d\n",errCode);
	}
	else
	{
		printf("NET_GetServerConfig CMD_GET_NET_CONFIG success\n");
		
		netconfig.IPAddr = ntohl(pnw->ip_address);
		netconfig.SubNetMask = ntohl(pnw->net_mask);
		netconfig.GateWay = ntohl(pnw->net_gateway);
		netconfig.DNSHostIP = ntohl(pnw->dns1);
		s32Size = sizeof(NET_CONFIG);
		errCode = NET_SetServerConfig(hLogonServer, CMD_SET_NET_CONFIG, (CHAR *)&netconfig, s32Size, 0);
		if(errCode)
		{
			printf("NET_SetServerConfig CMD_SET_NET_CONFIG failed:%d\n",errCode);
		}
		else
		{
			printf("NET_SetServerConfig CMD_SET_NET_CONFIG success\n");
		}
	}
	
	NET_LogoffServer(hLogonServer);
	hLogonServer = INVALID_HANDLE;
	
	return errCode?-1:0;
#endif
}

int myStreamCallBack(HANDLE hOpenChannel,void *pStreamData,UINT dwClientID,void *pContext,ENCODE_VIDEO_TYPE encodeVideoType,ULONG frameno)
{
	FRAME_HEAD *pFrameHead = (FRAME_HEAD *)pStreamData;
	EXT_FRAME_HEAD *exfream = (EXT_FRAME_HEAD *)((unsigned char *)pStreamData + sizeof(FRAME_HEAD));
	
	if(FRAME_FLAG_A == pFrameHead->streamFlag)
	{
		return -1;
	}
	
	int i;
	for(i=0; i<(int)g_river_client_count; i++)
	{
		int chn = i;
		if(g_riverc_info[chn].hOpenChannel == hOpenChannel)
		{
			if(g_riverc_info[chn].pStreamCB != NULL)
			{
				if(pFrameHead->streamFlag == FRAME_FLAG_VI || pFrameHead->streamFlag == FRAME_FLAG_VP)
				{
					real_stream_s stream;
					//memset(&stream, 0, sizeof(stream));
					stream.chn = chn;
					stream.data = (unsigned char *)pStreamData+sizeof(FRAME_HEAD)+sizeof(EXT_FRAME_HEAD);
					stream.len = pFrameHead->nByteNum-sizeof(EXT_FRAME_HEAD);
					stream.pts = exfream->nTimestamp;//pFrameHead->nTimestamp;
					stream.pts *= 1000;
					stream.media_type = MEDIA_PT_H264;
					if(pFrameHead->streamFlag == FRAME_FLAG_VI)
					{
						stream.frame_type = REAL_FRAME_TYPE_I;
					}
					else
					{
						stream.frame_type = REAL_FRAME_TYPE_P;
					}
					stream.rsv = 0;
					stream.mdevent = 0;
					stream.width = exfream->szFrameInfo.szFrameVideo.nVideoWidth;
					stream.height = exfream->szFrameInfo.szFrameVideo.nVideoHeight;
					//printf("chn%d pts:(%lu,%lu) w:%d h:%d\n",chn,pFrameHead->nTimestamp,exfream->nTimestamp,stream.width,stream.height);
					g_riverc_info[chn].pStreamCB(&stream, g_riverc_info[chn].dwContext);
				}
			}
		}
	}
	return 0;
}

int RIVER_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	if(!g_init_flag)
	{
		printf("RIVER_Start not inited\n");
		return -1;
	}
	
	if(chn < 0 || chn >= (int)(g_river_client_count))
	{
		printf("RIVER_Start chn%d error\n",chn);
		return -1;
	}
	
	ERR_CODE errCode = ERR_FAILURE;
	
	if(RIVER_GetLinkStatus(chn))
	{
		RIVER_Stop(chn);
	}
	
	if(!g_sdk_inited)
	{
		errCode = NET_Startup(5000, LogonNotifyCallback, CheckUserPswCallback, UpdateFileCallback, ServerMsgCmdCallback, StreamWriteCheckCallback, (ChannelStreamCallback)PreviewStreamCallback);
		//printf("--->|\tNET_Startup: %s\n",errCode?"Failed!":"Successful!");
		if(errCode)
		{
			printf("NET_Startup failed\n");
			return -1;
		}
		g_sdk_inited = 1;
	}
	
	char devip[64];
	memset(devip, 0, sizeof(devip));
	
	struct in_addr serv;
	serv.s_addr = dwIp;
	sprintf(devip, "%s", inet_ntoa(serv));
	
	if(g_riverc_info[chn].hLogonServer == INVALID_HANDLE)
	{
		errCode = NET_LogonServer(devip, wPort, (CHAR *)"admin", user, pwd, 0, &g_riverc_info[chn].hLogonServer);
		if(errCode)
		{
			printf("chn%d NET_LogonServer failed\n",chn);
			g_riverc_info[chn].hLogonServer = INVALID_HANDLE;
			return -1;
		}
	}
	
	unsigned int u32StreamFlag = 0;
	if(chn >= (int)(g_river_client_count/2))
	{
		u32StreamFlag = 1;
	}
	
	OPEN_CHANNEL_INFO_EX channelInfo;
	channelInfo.dwClientID = 1;
	channelInfo.nOpenChannel = 0;
	channelInfo.nSubChannel = u32StreamFlag;
	channelInfo.protocolType = (NET_PROTOCOL_TYPE)NET_PROTOCOL_TCP;//0;
	channelInfo.funcStreamCallback = (ChannelStreamCallback)myStreamCallBack;
	channelInfo.pCallbackContext = 0;
	errCode = NET_OpenChannel(devip, wPort, (CHAR *)"admin", user, pwd, (OPEN_CHANNEL_INFO_EX*)&channelInfo, &g_riverc_info[chn].hOpenChannel);
	if(errCode)
	{
		printf("chn%d NET_OpenChannel failed\n",chn);
		g_riverc_info[chn].hOpenChannel = INVALID_HANDLE;
		if(g_riverc_info[chn].hLogonServer != INVALID_HANDLE)
		{
			NET_LogoffServer(g_riverc_info[chn].hLogonServer);
			g_riverc_info[chn].hLogonServer = INVALID_HANDLE;
		}
		return -1;
	}
	g_riverc_info[chn].pStreamCB = pCB;
	g_riverc_info[chn].dwContext = dwContext;
	
	printf("chn%d NET_OpenChannel success\n",chn);
	
	return 0;
}

int RIVER_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	return -1;
}

int RIVER_Stop(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)(g_river_client_count))
	{
		return -1;
	}
	
	pthread_mutex_lock(&g_riverc_info[chn].lock);
	
	if(g_riverc_info[chn].hOpenChannel != INVALID_HANDLE)
	{
		NET_CloseChannel(g_riverc_info[chn].hOpenChannel);
		g_riverc_info[chn].hOpenChannel = INVALID_HANDLE;
	}
	
	if(g_riverc_info[chn].hLogonServer != INVALID_HANDLE)
	{
		NET_LogoffServer(g_riverc_info[chn].hLogonServer);
		g_riverc_info[chn].hLogonServer = INVALID_HANDLE;
	}
	
	pthread_mutex_unlock(&g_riverc_info[chn].lock);
	
	return 0;
}

//return value : 1 - Link; 0 - Lost
int RIVER_GetLinkStatus(int chn)
{
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)(g_river_client_count*2))
	{
		return 0;
	}
	
	if(chn >= (int)(g_river_client_count))
	{
		chn -= g_river_client_count;
	}
	
	int status = 0;
	
	pthread_mutex_lock(&g_riverc_info[chn].lock);
	
	status = (g_riverc_info[chn].hOpenChannel != INVALID_HANDLE);
	
	pthread_mutex_unlock(&g_riverc_info[chn].lock);
	
	return status;
}

int RIVER_CMD_Open(int chn)
{
	return 0;
}

int RIVER_CMD_Close(int chn)
{
	return 0;
}

int RIVER_CMD_SetImageParam(int chn, video_image_para_t *para)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	return 0;
}

int RIVER_CMD_GetImageParam(int chn, video_image_para_t *para)
{
	return -1;
}

int RIVER_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	ERR_CODE errCode = ERR_FAILURE;
	
	if(g_riverc_info[chn].hLogonServer == INVALID_HANDLE)
	{
		char devip[64];
		memset(devip, 0, sizeof(devip));
		
		struct in_addr serv;
		serv.s_addr = ipcam.dwIp;
		sprintf(devip, "%s", inet_ntoa(serv));
		
		errCode = NET_LogonServer(devip, ipcam.wPort, (CHAR *)"admin", ipcam.user, ipcam.pwd, 0, &g_riverc_info[chn].hLogonServer);
		if(errCode)
		{
			printf("chn%d NET_LogonServer failed\n",chn);
			g_riverc_info[chn].hLogonServer = INVALID_HANDLE;
			return -1;
		}
	}
	
	unsigned int PTZCmd = 0;
	BYTE Data1 = 0;
	BYTE Data2 = 0;
	
	if(cmd == EM_PTZ_CMD_START_TILEUP)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		PTZCmd = YT_UP;
		Data2 = tiltSpeed * 10;
	}
	else if(cmd == EM_PTZ_CMD_START_TILEDOWN)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		PTZCmd = YT_DOWN;
		Data2 = tiltSpeed * 10;
	}
	else if(cmd == EM_PTZ_CMD_START_PANLEFT)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		PTZCmd = YT_LEFT;
		Data1 = panSpeed * 10;
	}
	else if(cmd == EM_PTZ_CMD_START_PANRIGHT)
	{
		unsigned int panSpeed = *((unsigned int *)data);
		PTZCmd = YT_RIGHT;
		Data1 = panSpeed * 10;
	}
	else if(cmd == EM_PTZ_CMD_START_LEFTUP)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		PTZCmd = YT_UP;
		Data2 = tiltSpeed * 10;
	}
	else if(cmd == EM_PTZ_CMD_START_LEFTDOWN)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		PTZCmd = YT_DOWN;
		Data2 = tiltSpeed * 10;
	}
	else if(cmd == EM_PTZ_CMD_START_RIGHTUP)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		PTZCmd = YT_UP;
		Data2 = tiltSpeed * 10;
	}
	else if(cmd == EM_PTZ_CMD_START_RIGHTDOWN)
	{
		unsigned int tiltSpeed = *((unsigned int *)data);
		PTZCmd = YT_DOWN;
		Data2 = tiltSpeed * 10;
	}
	else if(cmd == EM_PTZ_CMD_START_ZOOMTELE)
	{
		PTZCmd = YT_ZOOMADD;
	}
	else if(cmd == EM_PTZ_CMD_START_ZOOMWIDE)
	{
		PTZCmd = YT_ZOOMSUB;
	}
	else if(cmd == EM_PTZ_CMD_START_FOCUSNEAR)
	{
		PTZCmd = YT_FOCUSADD;
	}
	else if(cmd == EM_PTZ_CMD_START_FOCUSFAR)
	{
		PTZCmd = YT_FOCUSSUB;
	}
	else if(cmd == EM_PTZ_CMD_START_IRISSMALL)
	{
		PTZCmd = YT_IRISSUB;
	}
	else if(cmd == EM_PTZ_CMD_START_IRISLARGE)
	{
		PTZCmd = YT_IRISADD;
	}
	else if(cmd >= EM_PTZ_CMD_STOP_TILEUP && cmd < EM_PTZ_CMD_PRESET_SET)
	{
		if(cmd == EM_PTZ_CMD_STOP_TILEUP)
		{
			PTZCmd = YT_UP_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_TILEDOWN)
		{
			PTZCmd = YT_DOWN_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_PANLEFT)
		{
			PTZCmd = YT_LEFT_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_PANRIGHT)
		{
			PTZCmd = YT_RIGHT_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_LEFTUP)
		{
			PTZCmd = YT_LEFT_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_LEFTDOWN)
		{
			PTZCmd = YT_LEFT_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_RIGHTUP)
		{
			PTZCmd = YT_RIGHT_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_RIGHTDOWN)
		{
			PTZCmd = YT_RIGHT_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_FOCUSNEAR)
		{
			PTZCmd = YT_FOCUSADD_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_FOCUSFAR)
		{
			PTZCmd = YT_FOCUSSUB_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_ZOOMTELE)
		{
			PTZCmd = YT_ZOOMADD_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_ZOOMWIDE)
		{
			PTZCmd = YT_ZOOMSUB_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_IRISLARGE)
		{
			PTZCmd = YT_IRISADD_STOP;
		}
		else if(cmd == EM_PTZ_CMD_STOP_IRISSMALL)
		{
			PTZCmd = YT_IRISSUB_STOP;
		}
		else
		{
			PTZCmd = YT_STOP;
		}
	}
	else if(cmd == EM_PTZ_CMD_PRESET_SET)
	{
		unsigned int preset = *((unsigned int *)data);
		PTZCmd = YT_PRESET;
		Data1 = preset;
	}
	else if(cmd == EM_PTZ_CMD_PRESET_GOTO)
	{
		unsigned int preset = *((unsigned int *)data);
		PTZCmd = YT_CALL;
		Data1 = preset;
	}
	else if(cmd == EM_PTZ_CMD_PRESET_CLEAR)
	{
		unsigned int preset = *((unsigned int *)data);
		PTZCmd = YT_PRESET_DEL;
		Data1 = preset;
	}
	else if(cmd == EM_PTZ_CMD_LIGHT_ON)
	{
		PTZCmd = YT_LAMPOPEN;
	}
	else if(cmd == EM_PTZ_CMD_LIGHT_OFF)
	{
		PTZCmd = YT_LAMPCLOSE;
	}
	else if(cmd == EM_PTZ_CMD_AUX_ON)
	{
		unsigned int id = *((unsigned int *)data);
		if(id == 1)//Ä¬ÈÏ1ºÅ¸¨ÖúÎªÓêË¢
		{
			PTZCmd = YT_BRUSHOPEN;
		}
	}
	else if(cmd == EM_PTZ_CMD_AUX_OFF)
	{
		unsigned int id = *((unsigned int *)data);
		if(id == 1)//Ä¬ÈÏ1ºÅ¸¨ÖúÎªÓêË¢
		{
			PTZCmd = YT_BRUSHCLOSE;
		}
	}
	else if(cmd == EM_PTZ_CMD_AUTOPAN_ON)
	{
		PTZCmd = YT_AUTOOPEN;
	}
	else if(cmd == EM_PTZ_CMD_AUTOPAN_OFF)
	{
		PTZCmd = YT_AUTOCLOSE;
	}
	
	PAN_CTRL nPANCtrl;
	nPANCtrl.ChannelNo = 0;
	nPANCtrl.COMMNo = 0;
	nPANCtrl.PanType = 0xff;
	nPANCtrl.PanCmd = PTZCmd;
	nPANCtrl.Data1 = Data1;
	nPANCtrl.Data2 = Data2;
	
	if(PTZCmd != 0)
	{
		errCode = NET_SetServerConfig(g_riverc_info[chn].hLogonServer, CMD_SET_PAN_CTRL, (CHAR *)&nPANCtrl, sizeof(nPANCtrl), 0);
		if(errCode)
		{
			printf("hehe1,chn%d NET_SetServerConfig CMD_SET_PAN_CTRL failed:%d\n",chn,errCode);
			return -1;
		}
		else
		{
			printf("hehe1,chn%d NET_SetServerConfig CMD_SET_PAN_CTRL success\n",chn);
		}
		
		unsigned int PTZCmd = 0;
		BYTE Data1 = 0;
		BYTE Data2 = 0;
		
		if(cmd == EM_PTZ_CMD_START_LEFTUP || cmd == EM_PTZ_CMD_START_LEFTDOWN)
		{
			unsigned int panSpeed = *((unsigned int *)data);
			PTZCmd = YT_LEFT;
			Data1 = panSpeed * 10;
		}
		else if(cmd == EM_PTZ_CMD_START_RIGHTUP|| cmd == EM_PTZ_CMD_START_RIGHTDOWN)
		{
			unsigned int panSpeed = *((unsigned int *)data);
			PTZCmd = YT_RIGHT;
			Data1 = panSpeed * 10;
		}
		
		nPANCtrl.PanCmd = PTZCmd;
		nPANCtrl.Data1 = Data1;
		nPANCtrl.Data2 = Data2;
		
		if(PTZCmd != 0)
		{
			errCode = NET_SetServerConfig(g_riverc_info[chn].hLogonServer, CMD_SET_PAN_CTRL, (CHAR *)&nPANCtrl, sizeof(nPANCtrl), 0);
			if(errCode)
			{
				printf("hehe2,chn%d NET_SetServerConfig CMD_SET_PAN_CTRL failed:%d\n",chn,errCode);
				return -1;
			}
			else
			{
				printf("hehe2,chn%d NET_SetServerConfig CMD_SET_PAN_CTRL success\n",chn);
			}
		}
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

int RIVER_CMD_SetTime(int chn, time_t t, int force)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!force)
	{
		if(!ipcam.enable || !RIVER_GetLinkStatus(chn))
		{
			//printf("RIVER_CMD_SetTime: chn%d vlost!!!\n",chn);
			return -1;
		}
	}
	
	ERR_CODE errCode = ERR_FAILURE;
	
	if(!g_sdk_inited)
	{
		errCode = NET_Startup(5000, LogonNotifyCallback, CheckUserPswCallback, UpdateFileCallback, ServerMsgCmdCallback, StreamWriteCheckCallback, (ChannelStreamCallback)PreviewStreamCallback);
		//printf("--->|\tNET_Startup: %s\n",errCode?"Failed!":"Successful!");
		if(errCode)
		{
			printf("NET_Startup failed\n");
			return -1;
		}
		g_sdk_inited = 1;
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
	
	DVS_DATE nDVSTime;
	memset(&nDVSTime, 0, sizeof(nDVSTime));
	nDVSTime.year = now.tm_year - 100;
	nDVSTime.month	= now.tm_mon + 1;
	nDVSTime.day   = now.tm_mday;
	nDVSTime.week  = now.tm_wday;
	nDVSTime.hour  = now.tm_hour;
	nDVSTime.minute = now.tm_min;
	nDVSTime.second = now.tm_sec;
	
	if(g_riverc_info[chn].hLogonServer == INVALID_HANDLE)
	{
		char devip[64];
		memset(devip, 0, sizeof(devip));
		
		struct in_addr serv;
		serv.s_addr = ipcam.dwIp;
		sprintf(devip, "%s", inet_ntoa(serv));
		
		errCode = NET_LogonServer(devip, ipcam.wPort, (CHAR *)"admin", ipcam.user, ipcam.pwd, 0, &g_riverc_info[chn].hLogonServer);
		if(errCode)
		{
			printf("chn%d NET_LogonServer failed\n",chn);
			g_riverc_info[chn].hLogonServer = INVALID_HANDLE;
			return -1;
		}
	}
	
	errCode = NET_SetServerConfig(g_riverc_info[chn].hLogonServer, CMD_SET_TIME, (CHAR *)&nDVSTime, sizeof(nDVSTime), 0);
	if(errCode)
	{
		printf("chn%d NET_SetServerConfig CMD_SET_TIME failed:%d\n",chn,errCode);
		return -1;
	}
	
	printf("chn%d NET_SetServerConfig CMD_SET_TIME success\n",chn);
	
	return 0;
}

int RIVER_CMD_Reboot(int chn)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	ERR_CODE errCode = ERR_FAILURE;
	
	if(g_riverc_info[chn].hLogonServer == INVALID_HANDLE)
	{
		char devip[64];
		memset(devip, 0, sizeof(devip));
		
		struct in_addr serv;
		serv.s_addr = ipcam.dwIp;
		sprintf(devip, "%s", inet_ntoa(serv));
		
		errCode = NET_LogonServer(devip, ipcam.wPort, (CHAR *)"admin", ipcam.user, ipcam.pwd, 0, &g_riverc_info[chn].hLogonServer);
		if(errCode)
		{
			printf("chn%d NET_LogonServer failed\n",chn);
			g_riverc_info[chn].hLogonServer = INVALID_HANDLE;
			return -1;
		}
	}
	
	errCode = NET_SetServerConfig(g_riverc_info[chn].hLogonServer, CMD_SET_RESTART_DVS, NULL, 0, 0);
	if(errCode)
	{
		printf("chn%d NET_SetServerConfig CMD_SET_RESTART_DVS failed:%d\n",chn,errCode);
		return -1;
	}
	
	printf("chn%d NET_SetServerConfig CMD_SET_RESTART_DVS success\n",chn);
	
	return 0;
}

int RIVER_CMD_RequestIFrame(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	return 0;
}

