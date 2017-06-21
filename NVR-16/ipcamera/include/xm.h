#ifndef __XM_H__ 
#define __XM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>

#include "ipcamera.h"

#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_PORT 34569

#define LOGIN_REQ		1000	//登录请求			0x3e8
#define LOGIN_RSP		1001	//登录响应
#define LOGOUT_REQ		1002	//登出请求
#define LOGOUT_RSP		1003	//等出响应
#define FORCELOGOUT_REQ	1004	//强制登出请求
#define FORCELOGOUT_RSP	1005	//强制登出响应
#define KEEPALIVE_REQ	1006	//保活请求			0x3ee
#define KEEPALIVE_RSP	1007	//保活响应			0x3ef
#define SYSINFO_REQ		1020	//获取系统信息请求	0x3fc
#define CONFIG_SET		1040	//设置配置请求
#define CONFIG_SET_RSP	1041	//设置配置响应
#define CONFIG_GET		1042	//获取配置请求
#define CONFIG_GET_RSP	1043	//获取配置响应

#define IPSEARCH_REQ	1530	//IP自动搜索请求
#define IPSEARCH_RSP	1531	//IP自动搜索请求响应

#define ABILITY_GET		1360	//能力级获取请求0x550
#define ABILITY_GET_RSP	1361	//能力级获取响应0x551

#define PTZ_REQ			1400	//云台控制请求
#define PTZ_RSP		1401	//云台控制响应
#define MONITOR_REQ		1410	//实时监视请求		0x582
#define MONITOR_RSP		1411	//实时监视请求响应	0x583
#define MONITOR_DATA	1412	//实时监视数据		0x584
#define MONITOR_CLAIM	1413	//监视认领请求		0x585
#define MONITOR_CLAIM_RSP	1414//监视认领请求响应
#define SYSMANAGER_REQ	1450	//系统管理请求
#define SYSMANAGER_RSP	1451	//系统管理请求响应
#define TIME_SET_REQ	1590	//时间设置请求		0x636
#define TIME_SET_RSP	1591	//时间设置请求响应	0x637

#define DATA_SIZE (16*1024)
#define FAILURE (-1)
#define SUCCESS (0)
#define NETSNDRCVFLAG	MSG_NOSIGNAL

#define VER 0

int XM_Init(unsigned int max_client_num);
int XM_DeInit();
int XM_Search(ipc_node** head, ipc_node **tail, unsigned char check_conflict);
int XM_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp);
int XM_Stop(int chn);

int XM_GetLinkStatus(int chn);
int XM_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
int XM_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);

int XM_CMD_Open(int chn);
int XM_CMD_Close(int chn);
int XM_CMD_SetImageParam(int chn, video_image_para_t *para);
int XM_CMD_GetImageParam(int chn, video_image_para_t *para);
int XM_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data);
int XM_CMD_SetTime(int chn, time_t t, int force);
int XM_CMD_Reboot(int chn);
int XM_CMD_RequestIFrame(int chn);

#endif

/*

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
	char	ip[16];
	unsigned short	port;
} net_info_t;

#define MAIN_STREAM		0
#define EXTRA_STREAM	1

typedef struct
{
	unsigned int	SessionID;
	int main_sock;
	int stream_sock;
	int stream_type;
	unsigned int dwStreamWidth;
	unsigned int dwStreamHeight;
	RealStreamCB pStreamCB;
	unsigned int dwContext;
	pthread_t capture_tid;
	pthread_mutex_t lock;
} Session_info_t;

typedef struct
{
	net_info_t	tcp_info;
	Session_info_t session[2];//0: main_stream  1:extra_stream
} xm_camera_info_t;	

*/


