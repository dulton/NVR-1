#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "fullhan.h"

#include "FHDEV_Net_Define.h"
#include "FHDEV_Net.h"

#define USE_FH_SDK//csp modify 20120425

#define INVALID_USERID		0
#define INVALID_REALPLAYHND	0

typedef struct
{
	#ifdef USE_FH_SDK
	DWORD dwUserID;
	DWORD dwRealPlayHandle;
	FHNP_Capacity_t capability;
	#else
	int sock_media;
	pthread_t pid;
	char eventLoopWatchVariable;
	char username[32];
	char password[32];
	#endif
	RealStreamCB pStreamCB;
	unsigned int dwContext;
	pthread_mutex_t lock;
}fh_client_info;

static fh_client_info *g_fhc_info = NULL;

static unsigned int g_fh_client_count = 0;

static unsigned char g_init_flag = 0;

#ifdef USE_FH_SDK
static unsigned char g_sdk_inited = 0;
#else
#include "common_basetypes.h"

#include "CryptAES.h"

#define FHNP_61_SEARCH_DType_Fh61Cam 0x01  //!< FH8610 CAM

#define FHNP_MAX_DATA_LEN 4096    //!< 数据最大长度

typedef struct tag_FHNP_NetData
{
	unsigned char ucDataVer; //!< 数据包版本号
	unsigned char ucData[FHNP_MAX_DATA_LEN]; //!< 数据内容
}PACK_NO_PADDING FHNP_NetData_t;

typedef struct tag_FHNP_NetMsg
{
	unsigned char ucMsgVer; //!< 通迅包版本号
	unsigned char ucHeadLen; //!< 通讯包头长
	unsigned char ucChannel; //!< 通道号
	unsigned char ucCmd; //!< 一级通讯命令
	unsigned char ucSubCmd;    //!< 二级通讯命令
	unsigned int uiErrorID; //!< 错误码
	unsigned char ucClientIndex; //!< 客户端索引号
	char chUser[32]; //!< 用户名
	char chPassword[32]; //!< 密码
	unsigned int uiReserver; //!< 保留字段
	unsigned char ucFlag;    //!< 多包模式下标识包属性
	unsigned short usDataLen; //!< 数据长度(FHNP_NetData_t的实际长度)
	FHNP_NetData_t stNetData; //!< 数据
}PACK_NO_PADDING FHNP_NetMsg_t;

/* 实时流请求 */
typedef struct 
{
	unsigned char encid; //!< must be 0
	int transmode; //!< FHEN_61_TransMode_e
	unsigned char needack; //!< 是否对请求进行反馈
}PACK_NO_PADDING FHNP_61_StreamRequest_t;

/*! 帧头结构 */
typedef struct 
{
	/* flag第四个字节表示帧类型，0xa1表示I帧，0xa2表示B帧，0xa0表示P帧，
	0xa4表示音频帧，0xa5表示jpeg。
	* 前三个字节目前默认填充0x00, 0x00, 0x01, */
	unsigned char flag[4]; //!< 0x00, 0x00, 0x01, 0xax
	unsigned char encid; //!< 当前帧视频主次码流标示
	unsigned char frame_num; //!< 帧号
	unsigned char block_num; //!< 帧数据块号
	unsigned char block_flag; //!< 数据块标识, bit0 为1 时表示帧开始, 
		//bit1为1时表示帧结束, bit0和bit1都为1时, 说明该帧只有一个block, 该block既
		//是开始也是结束. 帧结束时，紧跟帧头后面是 FHNP_61_FrameInfo_t
	unsigned int block_len; //!< 此块数据的大小
}PACK_NO_PADDING FHNP_61_BlockHead_t;

/*! 帧信息 */
typedef struct 
{ 
	unsigned long long timestamp;   //!< time stamp
	//unsigned __int64 timestamp;   //!< time stamp
	unsigned char avopt;  //!< 代表码流成分，00: 视频  01: 音频  10: 混合
	unsigned char vformat;  //!< 视频编码格式 00: h264
	unsigned char aformat; 
	//!< 音频编码格式 参照FHEN_61_EncAudioFormat_e
	unsigned char restart;  //!< 用来告知解码端，视频属性(如分辨率 b帧等)发生了变化
	unsigned short width; //!< 视频宽
	unsigned short height; //!< 视频高
	unsigned int    bitrate; //!< 码率, 单位kbps
	unsigned int frame_len; //!< 帧的总大小
	unsigned char frame_rate; //!< 帧率
	unsigned char audio_info;  //!< 低4bit:samplerateBits(0001:8000,0010:16000,0100:32000,0101:44100,0110:48000), 中间2bit:bitwBits(01:位宽8,10:位宽16,11:位宽24), 最高2bit:stereoBits（01：单声道，10：立体声）)
	unsigned char alarm_info; //!< 告警状态, bit 0: 移动侦测,   bit 1: 遮挡侦测
	unsigned char reserved;
}PACK_NO_PADDING FHNP_61_FrameInfo_t;

/*! 错误码 */
typedef enum
{
	FHEN_61_OK = 0,
	FHEN_61_ERR_SetConfig, //!< 设置/操作失败
	FHEN_61_ERR_GetConfig, //!< 获取失败
	FHEN_61_ERR_GetData, //!< 获取的数据有误
	FHEN_61_ERR_Parameter, //!< 参数有误
	FHEN_61_ERR_Authority, //!< 无权限
	FHEN_61_ERR_UserInexistence, //!< 用户不存在
	FHEN_61_ERR_Password, //!< 密码错误
	FHEN_61_ERR_ReLogin, //!< 重复登录
	FHEN_61_ERR_UserBeKicked, //!< 用户被踢除
	FHEN_61_ERR_Upgrading, //!< 升级中
	FHEN_61_ERR_OutOfMemory, //!< 内存不足
	FHEN_61_ERR_RecvOverTime, //!< 接收超时
	FHEN_61_ERR_Upgrade_DataError, //!< 升级包数据错误
	FHEN_61_ERR_Upgrade_SameVersion, //!< 升级包与设备版本一样
	FHEN_61_ERR_Upgrade_OldVersion, //!< 升级包为旧版本
	FHEN_61_ERR_UpgradeErr, //!< 升级失败
	FHEN_61_ERR_Upgrade_WriteFlashFail,//!< 升级过程中写flash失败 
	FHEN_61_ERR_Formating, //!< 格式化过程中
	FHEN_61_ERR_Recording, //!< 录像过程中
	FHEN_61_ERR_NotSupport, //!< 录像过程中
	FHEN_61_ERR_UNKNOW, //!< 未知错误
}FHEN_61_ERROR_e;

/*! 一级通讯命令 */
typedef enum
{
	FHEN_61_NC_UserManage = 0x01, //!< 用户管理
	FHEN_61_NC_WifiConfig, //!< WIFI配置
	FHEN_61_NC_NetConfig, //!< 网络配置
	FHEN_61_NC_EncConfig, //!< 编码配置
	FHEN_61_NC_IspConfig, //!< ISP配置
	FHEN_61_NC_RealStream, //!< 实时码流
	FHEN_61_NC_Playback, //!< 回放码流
	FHEN_61_NC_Record, //!< 录像
	FHEN_61_NC_Picture, //!< 截图
	FHEN_61_NC_Talk, //!< 对讲
	FHEN_61_NC_Storage, //!< 存储(SD卡)
	FHEN_61_NC_System, //!< 系统
	FHEN_61_NC_Log, //!< 日志
	FHEN_61_NC_Defence, //!< 布防
	FHEN_61_NC_Serial, //!< 串口
	FHEN_61_NC_General, //!< 其它配置
	FHEN_61_NC_Notify, //!< 消息上报
}FHEN_61_NetCmd_e;

/*! 二级通讯命令FHEN_61_NC_Stream */
typedef enum
{
	FHEN_61_NSC_RealStream_ServiceGet = 0x01,
	FHEN_61_NSC_RealStream_ServiceGet_ACK,
	FHEN_61_NSC_RealStream_ServiceSet,
	FHEN_61_NSC_RealStream_ServiceSet_ACK,
	FHEN_61_NSC_RealStream_Start,
	FHEN_61_NSC_RealStream_Start_ACK,
	FHEN_61_NSC_RealStream_Stop,
	FHEN_61_NSC_RealStream_Stop_ACK,
	FHEN_61_NSC_RealStream_UdpStart,
	FHEN_61_NSC_RealStream_UdpStart_ACK,
	FHEN_61_NSC_RealStream_UdpStop,
	FHEN_61_NSC_RealStream_UdpStop_ACK,
	FHEN_61_NSC_RealStream_AudioOn,
	FHEN_61_NSC_RealStream_AudioOn_ACK,
	FHEN_61_NSC_RealStream_AudioOff,
	FHEN_61_NSC_RealStream_AudioOff_ACK,
	FHEN_61_NSC_RealStream_StatusGet,
	FHEN_61_NSC_RealStream_StatusGet_ACK,
	FHEN_61_NSC_RealStream_AudioStatus,
	FHEN_61_NSC_RealStream_AudioStatus_ACK,
}FHEN_61_NetCmdStream_e;

/* 传输模式 */
typedef enum
{
	FHEN_61_TransMode_TCP = 0x00, //!< TCP
	FHEN_61_TransMode_UDP, //!< UDP（暂无效）
	FHEN_61_TransMode_RTP, //!< RTP（暂无效）
	FHEN_61_TransMode_UDP_TS, //!< UDP + TS（暂无效）
	FHEN_61_TransMode_RTP_TS, //!< RTP + TS（暂无效）
	FHEN_61_TransMode_MCAST_UDP_TS, //!< UDP + TS MultiCast（暂无效）
	FHEN_61_TransMode_RTSP_UDP_TS, //!< UDP + TS RTSP（暂无效）
	FHEN_61_TransMode_RTSP_RTP_TS, //!< UDP + TS MultiCast（暂无效）
}FHEN_61_TransMode_e;

typedef unsigned short	u16;
typedef unsigned int	u32;

extern "C" unsigned int GetLocalIp();
extern "C" int ConnectWithTimeout(u32 dwHostIp, u16 wHostPort, u32 dwTimeOut, u16 *pwErrorCode);

extern "C" int loopsend(int s, char *buf, unsigned int sndsize);
extern "C" int looprecv(int s, char *buf, unsigned int rcvsize);

extern int SkipData(int socket, int skip_len);
extern int DoStreamStateCallBack(int chn, real_stream_state_e msg);
#endif

int FH_Init(unsigned int max_client_num)
{
	if(max_client_num <= 0)
	{
		return -1;
	}
	
	g_fh_client_count = max_client_num / 2;
	
	g_fhc_info = (fh_client_info *)malloc(g_fh_client_count*sizeof(fh_client_info));
	if(g_fhc_info == NULL)
	{
		return -1;
	}
	memset(g_fhc_info,0,g_fh_client_count*sizeof(fh_client_info));
	
	int i = 0;
	for(i = 0; i < (int)g_fh_client_count; i++)
	{
		#ifdef USE_FH_SDK
		g_fhc_info[i].dwUserID = INVALID_USERID;
		g_fhc_info[i].dwRealPlayHandle = INVALID_REALPLAYHND;
		#else
		g_fhc_info[i].sock_media = -1;
		g_fhc_info[i].pid = 0;
		g_fhc_info[i].eventLoopWatchVariable = 0;
		#endif
		
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&g_fhc_info[i].lock, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	
	g_init_flag = 1;
	
	return 0;
}

int FH_DeInit()
{
	return 0;
}

int FH_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
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
	
	return -1;
}

int FH_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw)
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
	
	return -1;
}

#ifdef USE_FH_SDK
void FH_DataCB(DWORD dwPlayHandle, FHNPEN_StreamType_e eType, LPFHNP_FrameHead_t lpFrameHead, char* pBuf, DWORD dwBufLen, LPVOID lpUser)
{
	int i;
	for(i=0; i<(int)g_fh_client_count; i++)
	{
		int chn = i;
		if(g_fhc_info[chn].dwRealPlayHandle == dwPlayHandle)
		{
			if(g_fhc_info[chn].pStreamCB != NULL)
			{
				if(lpFrameHead->stVFrameHead.btFrameType < FHNPEN_FType_AFrame)
				{
					real_stream_s stream;
					//memset(&stream, 0, sizeof(stream));
					stream.chn = chn;
					stream.data = (unsigned char *)pBuf;
					stream.len = dwBufLen;
					stream.pts = lpFrameHead->stVFrameHead.ullTimeStamp;
					//stream.pts *= 1000;
					stream.media_type = MEDIA_PT_H264;
					if(lpFrameHead->stVFrameHead.btFrameType == FHNPEN_FType_IFrame)
					{
						stream.frame_type = REAL_FRAME_TYPE_I;
					}
					else
					{
						stream.frame_type = REAL_FRAME_TYPE_P;
					}
					stream.rsv = 0;
					stream.mdevent = 0;
					stream.width = lpFrameHead->stVFrameHead.wWidth;
					stream.height = lpFrameHead->stVFrameHead.wHeight;
					g_fhc_info[chn].pStreamCB(&stream, g_fhc_info[chn].dwContext);
					stream.chn = chn + g_fh_client_count;
					g_fhc_info[chn].pStreamCB(&stream, g_fhc_info[chn].dwContext + g_fh_client_count);
				}
			}
		}
	}
}
#else
int fh8610_StartRealPlay(int s, char *user, char *pwd)
{
	FHNP_NetMsg_t msg;
	FHNP_61_StreamRequest_t req;
	//printf("fh8610_StartRealPlay: start real play\n");
	memset(&msg, 0, sizeof(msg));
	msg.ucMsgVer = 0x61;//0x0;//0x61;
	msg.ucHeadLen = sizeof(FHNP_NetMsg_t) - sizeof(FHNP_NetData_t);
	msg.ucChannel = 0;
	msg.ucCmd = FHEN_61_NC_RealStream;
	msg.ucSubCmd = FHEN_61_NSC_RealStream_Start;
	msg.uiErrorID = 0;
	msg.ucClientIndex = 0;
	strcpy(msg.chUser, user);
	strcpy(msg.chPassword, pwd);
	msg.uiReserver = 0;
	msg.ucFlag = 0;
	msg.usDataLen = 1+sizeof(FHNP_61_StreamRequest_t);
	msg.stNetData.ucDataVer = 0;
	req.encid = 0;
	req.transmode = FHEN_61_TransMode_TCP;
	req.needack = 1;
	memcpy(msg.stNetData.ucData, &req, sizeof(req));
	int iDataLen = msg.ucHeadLen+msg.usDataLen;
	int ret = AESSocketSend(s, (char *)&msg, iDataLen, 0);
	if(ret <= 0)
	{
		printf("fh8610_StartRealPlay: AESSocketSend failed:%d\n",ret);
		return -1;
	}
	//printf("fh8610_StartRealPlay: start recv\n");
	memset(&msg, 0, sizeof(msg));
	ret = AESSocketRecv(s, (char *)&msg, sizeof(msg), 0);
	if(ret <= 0)
	{
		printf("fh8610_StartRealPlay: AESSocketRecv failed:%d\n",ret);
		return -1;
	}
	//printf("fh8610_StartRealPlay: recv ack\n");
	if(msg.ucCmd != FHEN_61_NC_RealStream)
	{
		printf("fh8610_StartRealPlay: error-1\n");
		return -1;
	}
	if(msg.ucSubCmd != FHEN_61_NSC_RealStream_Start_ACK)
	{
		printf("fh8610_StartRealPlay: error-2\n");
		return -1;
	}
	if(msg.uiErrorID != FHEN_61_OK)
	{
		printf("fh8610_StartRealPlay: error-3,uiErrorID=%d\n",msg.uiErrorID);
		return -1;
	}
	if(msg.ucChannel != 0)
	{
		printf("fh8610_StartRealPlay: error-4\n");
		return -1;
	}
	//if(msg.usDataLen != 1)
	//{
	//	printf("fh8610_StartRealPlay: error-5,usDataLen=%d\n",msg.usDataLen);
	//	return -1;
	//}
	printf("fh8610_StartRealPlay: success,ucDataVer=%d\n",msg.stNetData.ucDataVer);
	//fh8610_getstream(s);
	return 0;
}

int fh8610_StopRealPlay(int s, char *user, char *pwd)
{
	FHNP_NetMsg_t msg;
	//FHNP_61_StreamRequest_t req;
	//printf("fh8610_StopRealPlay: stop real play\n");
	memset(&msg, 0, sizeof(msg));
	msg.ucMsgVer = 0x61;//0x0;//0x61;
	msg.ucHeadLen = sizeof(FHNP_NetMsg_t) - sizeof(FHNP_NetData_t);
	msg.ucChannel = 0;
	msg.ucCmd = FHEN_61_NC_RealStream;
	msg.ucSubCmd = FHEN_61_NSC_RealStream_Stop;
	msg.uiErrorID = 0;
	msg.ucClientIndex = 0;
	strcpy(msg.chUser, user);
	strcpy(msg.chPassword, pwd);
	msg.uiReserver = 0;
	msg.ucFlag = 0;
	msg.usDataLen = 1+sizeof(int);//1+sizeof(FHNP_61_StreamRequest_t);
	msg.stNetData.ucDataVer = 0;
	//req.encid = 0;
	//req.transmode = FHEN_61_TransMode_TCP;
	//req.needack = 1;
	//memcpy(msg.stNetData.ucData, &req, sizeof(req));
	//memcpy(msg.stNetData.ucData, &req.encid, sizeof(req.encid));
	int *pEncID = (int *)msg.stNetData.ucData;
	*pEncID = 0;
	int iDataLen = msg.ucHeadLen+msg.usDataLen;
	int ret = AESSocketSend(s, (char *)&msg, iDataLen, 0);
	if(ret <= 0)
	{
		printf("fh8610_StopRealPlay: AESSocketSend failed:%d\n",ret);
		return -1;
	}
	//printf("fh8610_StopRealPlay: start recv\n");
	memset(&msg, 0, sizeof(msg));
	ret = AESSocketRecv(s, (char *)&msg, sizeof(msg), 0);
	if(ret <= 0)
	{
		printf("fh8610_StopRealPlay: AESSocketRecv failed:%d\n",ret);
		return -1;
	}
	//printf("fh8610_StopRealPlay: recv ack\n");
	if(msg.ucCmd != FHEN_61_NC_RealStream)
	{
		printf("fh8610_StopRealPlay: error-1\n");
		return -1;
	}
	if(msg.ucSubCmd != FHEN_61_NSC_RealStream_Stop_ACK)
	{
		printf("fh8610_StopRealPlay: error-2\n");
		return -1;
	}
	if(msg.uiErrorID != FHEN_61_OK)
	{
		printf("fh8610_StopRealPlay: error-3,uiErrorID=%d\n",msg.uiErrorID);
		return -1;
	}
	if(msg.ucChannel != 0)
	{
		printf("fh8610_StopRealPlay: error-4\n");
		return -1;
	}
	//if(msg.usDataLen != 1)
	//{
	//	printf("fh8610_StopRealPlay: error-5,usDataLen=%d\n",msg.usDataLen);
	//	return -1;
	//}
	printf("fh8610_StopRealPlay: success,ucDataVer=%d\n",msg.stNetData.ucDataVer);
	return 0;
}

void* ThreadPROCFH8610(void* pParam)
{
	unsigned int time_out_count = 0;
	
	char *enc_buf = NULL;
	unsigned int buf_len = 0;
	
	FHNP_61_BlockHead_t head;
	FHNP_61_FrameInfo_t info;
	
	unsigned char last_frame_no = 0;
	unsigned char last_block_no = 0;
	
	unsigned int frame_len = 0;
	
	int chn = (int)pParam;
	if(chn < 0 || chn >= (int)g_fh_client_count)
	{
		goto ProcQuit;
	}
	
	buf_len = 500*1024;//400*1024;
	enc_buf = (char *)malloc(buf_len);
	if(enc_buf == NULL)
	{
		goto ProcOver;
	}
	
	while(g_init_flag)
	{
		if(g_fhc_info[chn].eventLoopWatchVariable)
		{
			break;
		}
		
		if(g_fhc_info[chn].sock_media == -1)
		{
			break;
		}
		
		int max_fd_num = g_fhc_info[chn].sock_media;
		
		fd_set set;
		FD_ZERO(&set);
		FD_SET(g_fhc_info[chn].sock_media,&set);
		
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
				printf("ThreadPROCFH8610:chn%d select sock timeout & quit!!!\n",chn);
				time_out_count = 0;
				break;
			}
			else
			{
				printf("ThreadPROCFH8610:chn%d select sock timeout<%d> & continue!\n",chn,time_out_count);
				continue;
			}
		}
		if(ret < 0)
		{
			printf("ThreadPROCFH8610:chn%d select sock error:[%s]!\n",chn,strerror(errno));
			break;
		}
		if(g_fhc_info[chn].sock_media != -1 && FD_ISSET(g_fhc_info[chn].sock_media,&set))
		{
			time_out_count = 0;
			
			memset(&head, 0, sizeof(head));
			int s = g_fhc_info[chn].sock_media;
			int ret = looprecv(s, (char *)&head, sizeof(head));
			if(ret <= 0)
			{
				break;
			}
			if(head.flag[0] != 0x00 || head.flag[1] != 0x00 || head.flag[2] != 0x01)
			{
				printf("fh8610_getstream: frame head error-1");
				break;
			}
			if(head.flag[3] != 0xa1 && head.flag[3] != 0xa2 && head.flag[3] != 0xa0)
			{
				printf("fh8610_getstream: frame head error-2");
				break;
			}
			if(head.encid != 0)
			{
				printf("fh8610_getstream: frame head error-3");
				break;
			}
			//printf("recv frame data,block_len=%d,block_flag=%d\n",head.block_len,head.block_flag);
			if(head.block_flag & 0x1)
			{
				frame_len = 0;
				
				if(head.frame_num != last_frame_no+1)
				{
					//printf("new frame comming:(%d,%d,%d)\n",last_frame_no,head.frame_num,head.block_num);
				}
				last_frame_no = head.frame_num;
				
				last_block_no = head.block_num;
			}
			else
			{
				if(head.block_num != last_block_no+1)
				{
					printf("new block comming:(%d,%d,%d)\n",last_block_no,head.block_num,head.frame_num);
					break;
				}
				last_block_no = head.block_num;
			}
			
			if(head.block_flag & 0x2)
			{
				if(frame_len + head.block_len > buf_len)
				{
					ret = SkipData(s, head.block_len+sizeof(info));
					if(ret != 0)
					{
						break;
					}
					else
					{
						frame_len = 0;
						continue;
					}
				}
				else
				{
					ret = looprecv(s, (char *)&info, sizeof(info));
					if(ret <= 0)
					{
						break;
					}
					ret = looprecv(s, enc_buf+frame_len, head.block_len);
					if(ret <= 0)
					{
						break;
					}
					frame_len += (head.block_len);
					//printf("frame_len=%d\n",frame_len);
					if(g_fhc_info[chn].pStreamCB != NULL)
					{
						real_stream_s stream;
						//memset(&stream, 0, sizeof(stream));
						stream.chn = chn;
						stream.data = (unsigned char *)enc_buf;
						stream.len = frame_len;
						stream.pts = info.timestamp;
						//stream.pts *= 1000;
						stream.media_type = MEDIA_PT_H264;
						//if((enc_buf[4] & 0x1f) == 0x07)
						if(head.flag[3] == 0xa1)
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
						stream.width = info.width;
						stream.height = info.height;
						g_fhc_info[chn].pStreamCB(&stream, g_fhc_info[chn].dwContext);
						stream.chn = chn + g_fh_client_count;
						g_fhc_info[chn].pStreamCB(&stream, g_fhc_info[chn].dwContext + g_fh_client_count);
					}
					frame_len = 0;
				}
			}
			else
			{
				if(frame_len + head.block_len > buf_len)
				{
					ret = SkipData(s, head.block_len);
					if(ret != 0)
					{
						break;
					}
					else
					{
						continue;
					}
				}
				else
				{
					ret = looprecv(s, enc_buf+frame_len, head.block_len);
					if(ret <= 0)
					{
						break;
					}
					frame_len += head.block_len;
				}
			}
		}
		else
		{
			if(++time_out_count > 10)
			{
				printf("ThreadPROCFH8610:chn%d unknown sock error & quit!!!\n",chn);
				time_out_count = 0;
				break;
			}
		}
	}
	
	pthread_mutex_lock(&g_fhc_info[chn].lock);
	
	g_fhc_info[chn].pid = 0;
	g_fhc_info[chn].eventLoopWatchVariable = 0;
	
	if(g_fhc_info[chn].sock_media != -1)
	{
		fh8610_StopRealPlay(g_fhc_info[chn].sock_media, g_fhc_info[chn].username, g_fhc_info[chn].password);
		
		close(g_fhc_info[chn].sock_media);
		g_fhc_info[chn].sock_media = -1;
	}
	
	pthread_mutex_unlock(&g_fhc_info[chn].lock);
	
	free(enc_buf);
	enc_buf = NULL;
	
ProcOver:
	DoStreamStateCallBack(chn, REAL_STREAM_STATE_LOST);
ProcQuit:
	pthread_detach(pthread_self());
	return 0;
}
#endif

int FH_Start(int chn, RealStreamCB pCB, unsigned int dwContext, char* streamInfo, unsigned int dwIp, unsigned short wPort, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)(g_fh_client_count*2))
	{
		return -1;
	}
	
	if(chn >= (int)(g_fh_client_count))
	{
		return 0;
	}
	
	if(FH_GetLinkStatus(chn))
	{
		FH_Stop(chn);
	}
	
#ifdef USE_FH_SDK
	if(!g_sdk_inited)
	{
		if(!FHDEV_NET_Init())
		{
			return -1;
		}
		g_sdk_inited = 1;
	}
	
	char devip[64];
	memset(devip, 0, sizeof(devip));
	
	struct in_addr serv;
	serv.s_addr = dwIp;
	sprintf(devip, "%s", inet_ntoa(serv));
	
	if(g_fhc_info[chn].dwUserID == INVALID_USERID)
	{
		g_fhc_info[chn].dwUserID = FHDEV_NET_Login(devip, wPort, user, pwd, &g_fhc_info[chn].capability);
		if(g_fhc_info[chn].dwUserID == INVALID_USERID)
		{
			return -1;
		}
	}
	
	FHNP_Preview_t para;
	memset(&para, 0, sizeof(para));
	para.btTransMode = rtsp_over_tcp?FHNPEN_TransMode_TCP:FHNPEN_TransMode_UDP;
	para.btBlocked = 1;
	
	g_fhc_info[chn].dwRealPlayHandle = FHDEV_NET_StartRealPlay(g_fhc_info[chn].dwUserID, &para, FHNPEN_ST_FRAME, FH_DataCB, (LPVOID)chn);
	if(g_fhc_info[chn].dwRealPlayHandle == INVALID_REALPLAYHND)
	{
		if(g_fhc_info[chn].dwUserID != INVALID_USERID)
		{
			FHDEV_NET_Logout(g_fhc_info[chn].dwUserID);
			g_fhc_info[chn].dwUserID = INVALID_USERID;
		}
		return -1;
	}
	g_fhc_info[chn].pStreamCB = pCB;
	g_fhc_info[chn].dwContext = dwContext;
	
	return 0;
#else
	u16 wErrorCode = 0;
	int s = ConnectWithTimeout(dwIp, wPort, 5000, &wErrorCode);
	if(s < 0)
	{
		printf("FH_Start:chn%d connect error:(0x%08x,%d),err:(%d,%s)\n",chn,dwIp,wPort,errno,strerror(errno));
		return -1;
	}
	int ret = fh8610_StartRealPlay(s, user, pwd);
	if(ret < 0)
	{
		printf("FH_Start:chn%d fh8610_StartRealPlay failed\n",chn);
		close(s);
		return -1;
	}
	
	pthread_mutex_lock(&g_fhc_info[chn].lock);
	
	g_fhc_info[chn].sock_media = s;
	g_fhc_info[chn].pStreamCB = pCB;
	g_fhc_info[chn].dwContext = dwContext;
	g_fhc_info[chn].eventLoopWatchVariable = 0;
	strcpy(g_fhc_info[chn].username, user);
	strcpy(g_fhc_info[chn].password, pwd);
	if(pthread_create(&g_fhc_info[chn].pid, NULL, ThreadPROCFH8610, (void *)chn) != 0)
	{
		g_fhc_info[chn].pid = 0;
		FH_Stop(chn);
		
		pthread_mutex_unlock(&g_fhc_info[chn].lock);
		return -1;
	}
	
	pthread_mutex_unlock(&g_fhc_info[chn].lock);
	
	return 0;
#endif
}

int FH_Startbyurl(int chn, RealStreamCB pCB, unsigned int dwContext, char* rtspURL, char *user, char *pwd, unsigned char rtsp_over_tcp)
{
	return -1;
}

int FH_Stop(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	if(chn < 0 || chn >= (int)(g_fh_client_count*2))
	{
		return -1;
	}
	
	if(chn >= (int)(g_fh_client_count))
	{
		return 0;
	}
	
#ifdef USE_FH_SDK
	pthread_mutex_lock(&g_fhc_info[chn].lock);
	
	if(g_fhc_info[chn].dwRealPlayHandle != INVALID_REALPLAYHND)
	{
		FHDEV_NET_StopRealPlay(g_fhc_info[chn].dwRealPlayHandle);
		g_fhc_info[chn].dwRealPlayHandle = INVALID_REALPLAYHND;
	}
	
	if(g_fhc_info[chn].dwUserID != INVALID_USERID)
	{
		FHDEV_NET_Logout(g_fhc_info[chn].dwUserID);
		g_fhc_info[chn].dwUserID = INVALID_USERID;
	}
	
	pthread_mutex_unlock(&g_fhc_info[chn].lock);
#else
	if(g_fhc_info[chn].pid != 0)
	{
		#if 0
		pthread_mutex_lock(&g_fhc_info[chn].lock);
		
		if(g_fhc_info[chn].sock_media != -1)
		{
			fh8610_StopRealPlay(g_fhc_info[chn].sock_media, g_fhc_info[chn].username, g_fhc_info[chn].password);
			
			close(g_fhc_info[chn].sock_media);
			g_fhc_info[chn].sock_media = -1;
		}
		
		pthread_mutex_unlock(&g_fhc_info[chn].lock);
		#endif
		
		g_fhc_info[chn].eventLoopWatchVariable = 1;
		
		pthread_join(g_fhc_info[chn].pid, NULL);
	}
	
	pthread_mutex_lock(&g_fhc_info[chn].lock);
	
	g_fhc_info[chn].pid = 0;
	g_fhc_info[chn].eventLoopWatchVariable = 0;
	
	if(g_fhc_info[chn].sock_media != -1)
	{
		close(g_fhc_info[chn].sock_media);
		g_fhc_info[chn].sock_media = -1;
	}
	
	pthread_mutex_unlock(&g_fhc_info[chn].lock);
#endif
	
	return 0;
}

//return value : 1 - Link; 0 - Lost
int FH_GetLinkStatus(int chn)
{
	if(!g_init_flag)
	{
		return 0;
	}
	
	if(chn < 0 || chn >= (int)(g_fh_client_count*2))
	{
		return 0;
	}
	
	if(chn >= (int)(g_fh_client_count))
	{
		chn -= g_fh_client_count;
	}
	
	int status = 0;
	
	pthread_mutex_lock(&g_fhc_info[chn].lock);
	
	#ifdef USE_FH_SDK
	status = (g_fhc_info[chn].dwRealPlayHandle != INVALID_REALPLAYHND);
	#else
	status = (g_fhc_info[chn].sock_media != -1);
	#endif
	
	pthread_mutex_unlock(&g_fhc_info[chn].lock);
	
	return status;
}

int FH_CMD_Open(int chn)
{
	return 0;
}

int FH_CMD_Close(int chn)
{
	return 0;
}

int FH_CMD_SetImageParam(int chn, video_image_para_t *para)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	return 0;
}

int FH_CMD_GetImageParam(int chn, video_image_para_t *para)
{
	return -1;
}

int FH_CMD_PtzCtrl(int chn, EMPTZCMDTYPE cmd, void* data)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	return -1;
}

int FH_CMD_SetTime(int chn, time_t t, int force)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	if(!force)
	{
		if(!ipcam.enable || !FH_GetLinkStatus(chn))
		{
			//printf("FH_CMD_SetTime: chn%d vlost!!!\n",chn);
			return -1;
		}
	}
	
#ifdef USE_FH_SDK
	if(!g_sdk_inited)
	{
		if(!FHDEV_NET_Init())
		{
			return -1;
		}
		g_sdk_inited = 1;
	}
#endif
	
	return -1;
}

int FH_CMD_Reboot(int chn)
{
	ipc_unit ipcam;
	if(IPC_Get(chn, &ipcam))
	{
		return -1;
	}
	
	return -1;
}

int FH_CMD_RequestIFrame(int chn)
{
	if(!g_init_flag)
	{
		return -1;
	}
	
	return 0;
}

