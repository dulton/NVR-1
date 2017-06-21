#ifndef __NETCOMM_H_
#define __NETCOMM_H_

#include "common_basetypes.h"
#include "hddcmd.h"
#include "diskmanage.h"
#include "Cmdcallback.h"


#ifdef __cplusplus
extern "C" {
#endif

// #define 
#define FILE_PATH_MAX				128

#define RTSP                             // RTSP ENABLE
#define MOBILE_SERVER2                   // 手机监控ENABLE
#define DDNS_USER_LEN_MAX			32

// typedef

typedef u32	IPDEF;

// 远程格式化/远程升级回调函数

//2.4.5
typedef enum
{
	EM_NETFORMAT,
	EM_NETUPDATE,
} EMNETOP;
typedef enum
{
	EM_NETUPGRADE_USB,
	EM_NETUPGRADE_NET,
} EMNETUPGRADEWAY;

// mainboard || panel
typedef enum
{
	EM_NETUPGRADE_MAINBOARD,
	EM_NETUPGRADE_PANEL,
	EM_NETUPGRADE_STARTLOGO,
	EM_NETUPGRADE_APPLOGO,
} EMNETUPGRADETARGET;

typedef struct _SFormat
{
	u8 nIdx;	
} SRFormat;

typedef struct _SUpdate
{
	EMNETUPGRADETARGET 		eTgt;
	EMNETUPGRADEWAY		 	eWay;
	char sfilepath[FILE_PATH_MAX];	
} SRUpdate;

typedef struct
{
	EMNETOP eOp; 	// 0 format, 1 update
	union
	{
		SRFormat Format;
		SRUpdate Update;
	};
} SRemoteOpCB;

typedef s32 (*PFRemoteOp)(SRemoteOpCB * para);

/*
 VOIP 数据流输出结构
*/
typedef struct _sVOIPStream
{
	s32 	 nChn;
	u8		 *pData;
	u32		 nLen; // in buffer length; out real data length
	u64		 nTimeStamp;
	BOOL     bRaw;
} SVOIPStream, *PSVOIPStream;

/*
 VOIP 读入音频流回调
*/
typedef void (*PNetCommVOIPIn)(PSVOIPStream* pStream);

/*
 VOIP 输出流输出回调
*/
typedef void (*PNetCommVOIPOut)(PSVOIPStream* pStream);

/* 
 网络流媒体帧类型
*/
typedef enum
{
	EM_VIDEO,
	EM_AUDIO,
	EM_TALK,
	EM_MAX
} EM_STREAM_FRAME;

/*
 预览音视频帧头信息结构
*/
typedef struct _sNetCommStreamHeader
{
	u8	byChnIndex;       // 
	u32	byMediaType;
	u8	byFrameType;
	u8  byLastSnd;
	u8	bSub; // 0 not sub, 3 - bit0 mb sub, bit1 net sub
	u32 dwlen;
	u32 timeStamp;
	u8  resolution;
	u16 nWidth;
	u16 nHeight;
	u8  nFrameRate;
	EM_STREAM_FRAME eFrameType;
	u32 nAudioMode;
	u32 stamp, stamp1;
} SNetComStmHead, *PSNetComStmHead;

/*
 远程回放文件操作类型
 
*/
typedef enum
{
	EM_RMTPLAY_OPEN,
	EM_RMTPLAY_SEEK,
	EM_RMTPLAY_GetTimeLength,
	EM_RMTPLAY_GetMediaFormat,
	EM_RMTPLAY_CLOSE,
	EM_RMTPLAY_GETFRAME,	
} EM_RMTPLAY_OP;

/*
 文件操作请求来源
 回放，备份
 
*/
typedef enum
{
	EM_FILEOP_DOWN,
	EM_FILEOP_REPLAY,
	EM_FILEOP_BACKUP,
	
} EM_FILEOP_SRC;

/*
 远程回放文件操作参数结构
*/
typedef struct _sRemotePlayFileOp
{
	// op type
	EM_RMTPLAY_OP eOp;

	// op req src
	EM_FILEOP_SRC eOpSrc;
	
	// operate result
	s32 nOpRslt;
	
	// op param
	union
	{
		struct {
			char szFilePath[FILE_PATH_MAX]; // open file path
			u32  nOffset;         			// seek offset
			void* pFile;
		} OpenSeek;
		
		struct {
			u32 nTotalTime; 
		} GetTimeLen;
		
		struct {
			u16 nWidth;
			u16 nHeight;
		} GetFormat;
		
		struct {
			int nFrameSize;
			u8* pFrameBuf;
			u32 nBufSize;
			u32 nStartTime;
			u8  bKey;
			u64 nPts;
			u8  nMediaType; // 1 audio, 0 video
		} GetFrame;			
	};
		
} SRmtPlayFileOp, *PSRmtPlayFileOp;

/*
 预览请求数据流类型
*/
typedef enum
{
	EM_PREQ_MAIN,
	EM_PREQ_SUB,
	EM_PREQ_SUB_MB,
	EM_PREQ_AUDIO,
	EM_PREQ_VOIP,
	EM_PREQ_THIRD,//csp modify 20130423
	EM_PREQ_MAX
} EM_PREVREQ;

/*
 流请求操作类型
*/
typedef enum
{
	EM_START,
	EM_STOP,
} EM_ACT;

/*
 PStreamRequestCB param struct 
*/
typedef struct _sTcpStreamReq
{
	u8 			nChn;
	EM_PREVREQ 	nType; // 0 main, 1 sub, 2 audio, 
	EM_ACT		nAct; // 0 start, 1 stop	
} STcpStreamReq;

/*
 在线用户信息
*/
typedef struct
{
	u8		bOnline; // 0 offline, 1 online
	u8		bUserDel;
	u8		resv[2];
	u32		id;
	u32 	ip;
	s32		sock;
	char	mac[18];	
	char 	name[32];
} PACK_NO_PADDING SNetUser;

/*
	动态IP, 
	静态IP,
	PPPoE拨号连接
	..
*/
typedef enum
{
	ECW_STATIC,
	ECW_DHCP,
	ECW_PPPOE,
	ECW_MAX
} E_CONN_WAY;

/*
	已连接，
	未连接
	..
*/
typedef enum
{
	ECS_ONLINE,
	ECS_OFFLINE,
	ECS_MAX
} E_CONN_STATE;

typedef struct _sNetConnSta
{
	E_CONN_WAY 		eWay;
	E_CONN_STATE	eState;
} SNetConnStatus;

/*
 远程回放文件操作回调类型
 
 参数:
  void* pFileHandle : 文件句柄
  EM_RMTPLAY_OP eOp : 文件操作
  PSRmtPlayFileOp pParam : 操作参数
  
 返回: 由pParam返回操作结果
*/
typedef void (*PRemotePlayCB)(void* pFileHandle, PSRmtPlayFileOp pParam);

/*
 提供网络模块调用方定义的回调类型,该回调负责申请音视频流
 
*/
typedef void (*PStreamRequestCB)(STcpStreamReq* param);

/*
 提供网络模块调用方定义的回调类型,该回调负责申请关键帧
 
*/
typedef void (*PRequestKeyFrameCB)();

// 网络内部支持日志类型
typedef enum
{
	EM_NETLOG_START_DWELL,
	EM_NETLOG_STOP_DWELL,
	EM_NETLOG_MUTE,
	EM_NETLOG_CHANGE_VOICE,
	EM_NETLOG_CONTROL_PTZ,
	EM_NETLOG_SNAP_SUCCESS,
	EM_NETLOG_START_MANUAL_REC,
	EM_NETLOG_STOP_MANUAL_REC,
	EM_NETLOG_MANUAL_ALARM,
	EM_NETLOG_CLEAR_ALARM,
	EM_NETLOG_START_MD,
	EM_NETLOG_STOP_MD,
	EM_NETLOG_POWER_OFF,
	EM_NETLOG_REBOOT,
	EM_NETLOG_LOGIN,
	EM_NETLOG_LOGOFF,
	EM_NETLOG_FORMAT_SUCCESS,
	EM_NETLOG_FORMAT_FAIL,
	EM_NETLOG_UPDATE,

	EM_NETLOG_ENABLE_PWDCHECK,
	EM_NETLOG_DISABLE_PWDCHECK,
	EM_NETLOG_CHANGE_VO,
	EM_NETLOG_CHANGE_MAX_NETUSER,
	EM_NETLOG_CHANGE_VIDEOFORMAT,
	EM_NETLOG_CHANGE_LANGUAGE,
	EM_NETLOG_CHANGE_ZONE,
	EM_NETLOG_ENABLE_AUTO_TIMESYNC,
	EM_NETLOG_DISABLE_AUTO_TIMESYNC,
	EM_NETLOG_CHANGE_NTP_SERVER,
	EM_NETLOG_DST_CONFIG,
	EM_NETLOG_CHANGE_CHN_NAME,
	EM_NETLOG_CHANGE_REC_ENABLE,
	EM_NETLOG_CHANGE_REC_AUDIO_ENABLE,
	EM_NETLOG_CHANGE_REC_CODESTREAM,
	EM_NETLOG_CHANGE_TIME_BEFORE_ALARM,
	EM_NETLOG_CHANGE_TIME_AFTER_ALARM,
	EM_NETLOG_CHANGE_REC_EXPIRETIME,
	EM_NETLOG_CHANGE_OSD,
	EM_NETLOG_CHANGE_TIMER_RECSCHE,
	EM_NETLOG_CHANGE_MD_RECSCHE,
	EM_NETLOG_CHANGE_SENSORALARM_RECSCHE,
	EM_NETLOG_CHANGE_SENSOR_ENABLE,
	EM_NETLOG_CHANGE_SENSOR_TYPE,
	EM_NETLOG_CHANGE_SENSOR_ALARMDEAL,
	EM_NETLOG_CHANGE_SENSOR_DETECTION_SCHE,
	EM_NETLOG_CHANGE_MD_ENABLE,
	EM_NETLOG_CHANGE_MD_AREA_SETUP,
	EM_NETLOG_CHANGE_MD_ALARMDEAL,
	EM_NETLOG_CHANGE_MD_DETECTION_SCHE,
	EM_NETLOG_CHANGE_VIDEOLOSS_ALARMDEAL,
	EM_NETLOG_CHANGE_ALARMOUT_SCHE,
	EM_NETLOG_ENABLE_SOUND_ALARM,
	EM_NETLOG_DISABLE_SOUND_ALARM,
	EM_NETLOG_CHANGE_IP,
	EM_NETLOG_CHANGE_HTTP_PORT,
	EM_NETLOG_CHANGE_SERVER_PORT,
	EM_NETLOG_CHANGE_NET_CODESTREAM,
	EM_NETLOG_CHANGE_OUTBOX_CONFIG,
	EM_NETLOG_CHANGE_INBOX_CONFIG,
	EM_NETLOG_CHANGE_DNS_CONFIG,
	EM_NETLOG_ADD_USER,
	EM_NETLOG_DELETE_USER,
	EM_NETLOG_CHANGE_USER_AUTH,
	EM_NETLOG_CHANGE_PWD,
	EM_NETLOG_CHANGE_SERIAL_CONFIG,
	EM_NETLOG_CHANGE_CRUISELINE,
	EM_NETLOG_FACTORY_RESET,
	EM_NETLOG_PB_STOP,
	EM_NETLOG_PB_PAUSE,
	EM_NETLOG_PB_RESUME,
	EM_NETLOG_PB_BACKWARD,
	EM_NETLOG_PB_FORWARD,
	EM_NETLOG_START_BACKUP,
	EM_NETLOG_END_BACKUP,
	EM_NETLOG_SEARCH_DATA_BYTIME,
	EM_NETLOG_SEARCH_RECTIME,
	EM_NETLOG_CHECKINFO
} EM_NETLOG;

// 网络命令日志记录回调参数
typedef struct _sNetLog
{
	int ip;
	int	sockfd;
	EM_NETLOG nLogId;	// 遵循ctrlprotocol定义命令ID
} SNetLog;

/*
 日志记录回调
 
*/
typedef void (*PNetLogWriteCB)(SNetLog* pLog);


/*
 配置接口存取网络参数结构
*/
typedef struct _sNetAdvPara
{
	char	szSmtpServer[64];
	
	// 发件人邮箱
	char	szSendMailBox[64];
	char	szSMailPasswd[64];

    u8		nSSLFlag;

	// 收件人邮箱
	char	szReceiveMailBox[64];
	char	szReceiveMailBox2[64];
	char	szReceiveMailBox3[64];

	// 附加图片
	char	szSnapPic[64];
	
	// 抓图间隔
	u32		nSnapIntvl;
	
} SNetAdvancePara;

#define MAX_ALARM_UPLOAD_NUM 5

typedef struct
{
	CPHandle g_cph;
	u8 g_state;
}ifly_AlarmUploadCenter_t;

typedef struct _sNetCfgPara
{
	// 基本参数
	u8     	Version[8];			    // 8字节的版本信息
    char   	HostName[16];           // 主机名
    u32  	HostIP;                 // IP 地址
    u32  	Submask;                // 子网掩码
    u32  	GateWayIP;              // 网关 IP
    u32  	DNSIP;                  // DNS IP
    u32  	DNSIPAlt;				// DNS IP
    u8 		MAC[18];				// MAC Address

    // 外部接口
    u32   	AlarmServerIP;          // 报警中心IP
    u16 	AlarmServerPort;        // 报警中心端口
    u32   	SMTPServerIP;           // SMTP server IP
    u16 	SMTPServerPort;         // SMTP server port
	
    u32   	LogServerIP;            // Log server IP
    u16 	LogServerPort;          // Log server port

    // 本机服务端口
    u16 	HttpPort;               	// HTTP服务端口号
    u16 	MobilePort;              	// 手机监控端口号
    
    u16 	TCPMaxConn;             	// TCP 最大连接数
    u16 	TCPPort;                	// TCP 侦听端口

	u8		nSubStreamMax;				// 子码流数上限
	u8		nSendBufNumPerChn;			// 每通道发送buff数
	u32		nFrameSizeMax;				// 帧buff大小上限
	
	// 网路子码流
	u8		nVideoMediaType;			// 视频编码类型
	u8		nAudioMediaType;			// 预览音频编码类型
	u8		nVoipMediaType;				// 对讲音频编码类型
	u8		nAudioSampleMode;			// 音频采样模式

    u16 	UDPPort;                	// UDP 侦听端口

    u8		McastFlag;
    u16 	McastPort;              	// 组播端口
    u32 	McastIP;                	// 组播IP
    
	u8		PPPOEFlag;					// pppoe 使能开关
	u8		PPPOEUser[64];
	u8		PPPOEPasswd[64];

	u8		DhcpFlag;					// Dhcp 使能开关
	
	u8		DDNSFlag;					// DDNS 控制位
	u8		DDNSDomain[64];
	u8		DDNSUser[64];
	u8		DDNSPasswd[64];
	u16		UpdateIntvl;				// DDNS	IP更新间隔
	
    // 其他
    u8    	MonMode;                	// 监视协议 TCP|UDP|MCAST
    u8    	PlayMode;               	// 回放协议 TCP|UDP|MCAST
    u8    	AlmSvrStat;             	// 报警中心状态 <开/关>
	
	// 高级配置
    SNetAdvancePara	sAdancePara;
	
	//csp modify 20130321
	u8		UPNPFlag;
} SNetPara;

// 网络状态及通知回调
typedef enum
{
	EM_NET_STATE_DHCP,
	EM_NET_STATE_PPPOE,
	EM_NET_STATE_MAIL,
	EM_NET_STATE_DDNS,
	EM_NET_STATE_CONN,
	EM_NET_STATE_UPDATEMAINBOARDSTART,
	EM_NET_STATE_UPDATEPANNELSTART,
	EM_NET_STATE_UPDATESTARTLOGOSTART,
	EM_NET_STATE_UPDATEAPPLOGOSTART,
	
	EM_NET_STATE_FORMAT_START,
	EM_NET_STATE_SG,//上传监管平台返回状态
	EM_NET_STATE_SGReqSnap,//监管平台请求上传
} EM_NET_ACT;

typedef struct _sNetStateRslt
{
	u8 state; // 0 start, 1 stop
	u8 nErr; // 0 succ, 1 fail
} SNetStateRslt;

typedef struct _sNetStateNotify
{
	EM_NET_ACT eAct;
	SNetStateRslt sRslt;
	unsigned int SGReqChnMask; //深广平台服务器请求通道掩码
} SNetStateNotify;

typedef void (*PNetCommStateNotify)(void* pNotify);

/*
 网络模块初始化参数结构(暂定，后续根据需要删减)
 
*/ 
typedef struct _sNetCommCfg
{
	SNetPara sParaIns;
	u16		nAllLinkMax;			  		// 码流数上限
	u8		nSubStreamMax;					// 子码流数上限
	u8		nSendBufNumPerChn;				// 每通道发送buff数
	u32		nFrameSizeMax;					// 帧buff大小上限
	u32		yFactor;						// 制式
	u8		nVideoMediaType;
	u8		nAudioMediaType;
	u8		nVideoFrameRate;
	u8		nAudioFrameRate;
	u8		nAudioMode;
	u8 		nVoipMode;
	u8		nChnMax;
	//yaogang modify 20150324 
	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	u8		nNVROrDecoder;
	
	//csp mpdofy
	u16		nVideoMaxWidth;
	u16		nVideoMaxHeight;
	void*	pDiskMgr;
	
	// Callback
	PStreamRequestCB 	pReqStreamCB;
	PRequestKeyFrameCB 	pReqKeyCB;
	PRemotePlayCB 		pRpCB;
	PNetCommVOIPOut 	pVOIPOut;
	PNetCommVOIPIn 		pVOIPIn;

	PNetLogWriteCB		pWriteLogCB;

	PNetCommStateNotify pNotify;
	PFRemoteOp			pRFomat, pRUpdate;
} SNetCommCfg, *PSNetCommCfg;

typedef struct _sNetMultiCastInfo
{
	u32		deviceIP; 						//设备IP  
	u16		devicePort;						//设备端口 
	char 	device_name[32];				//设备名称
	char 	device_mode[32];				//设备型号
	u8		maxChnNum;						//最大通道数
	u8		maxAduioNum;					//最大音频数
	u8		maxSubstreamNum;				//最大子码流数
	u8		maxPlaybackNum;					//最大回放数
	u8		maxAlarmInNum;					//最大报警输入数
	u8		maxAlarmOutNum;					//最大报警输出数
	u8		maxHddNum;						//最大硬盘数
	u8 nNVROrDecoder;	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
} PACK_NO_PADDING SNetMultiCastInfo;

// set net para cmd
typedef enum
{
	EM_NET_SET_IP,
	EM_NET_SET_GW,
	EM_NET_SET_MSK,
	EM_NET_SET_HW,
	EM_NET_SET_DNS,
	EM_NET_ADD_DNS
} EM_NET_SET;

typedef struct _sNetSet
{
	union
	{
		u32 nIP;
		u32 nGateway;
		u32 nSubmask;
		u8	szHW[18];
		u32 nDns;
	};
} SNetSet;

/*
 Network state
*/
typedef enum
{
	EM_NS_CONNECTED = 0,
	EM_NS_DISCONNECTED,
	
} EM_NET_CONN_STATE;

/*
 邮件参数
*/
typedef struct _sMailContent
{
	char* pSrcUsr;
	char* pSrcBox;
	char* pToUsr;
	char* pToBox;

	char* pUserName;
	char* pPasswd;

	char* pSmtp;
	char* pSubject;
	char* pBody;

	u8	  bAttach;	
} SMailContent;

// DDNS 
typedef enum
{
	EM_NET_DDNS_TL = 1,
	EM_NET_DDNS_3322,
	EM_NET_DDNS_DYDNS,
	EM_NET_DDNS_DVRNET,
	EM_NET_DDNS_NOIP,
	EM_NET_DDNS_JMDVR,
	EM_NET_DDNS_CHANGEIP,
	EM_NET_DDNS_JSJDVR,
	EM_NET_DDNS_CCTVSVR,
	EM_NET_DDNS_CCTVDVR,
	EM_NET_DDNS_KONLAN,
	EM_NET_DDNS_MYQSEE,
	EM_NET_DDNS_DYNDDNS,
	EM_NET_DDNS_PEOPLE,
	EM_NET_MAX,
} EM_NET_DDNS;

typedef struct _sNetDdnsPara
{
	char szUser[DDNS_USER_LEN_MAX];
	char szPasswd[DDNS_USER_LEN_MAX];
	char szDomain[DDNS_USER_LEN_MAX];
} SNetDdnsPara;

typedef enum {
	UPDATE_TYPE_MAINBOARD 	=  0,
	UPDATE_TYPE_PANEL 		=  1,
	UPDATE_TYPE_STARTLOGO 	=  2,
	UPDATE_TYPE_APPLOGO 	=  3
} E_REMOTE_UPDATE;

typedef struct _sMultiCastInfo
{
	u32		deviceIP; 						//设备IP  
	u16		devicePort;						//设备端口 
	char 	device_name[32];				//设备名称	
} SMultiCastInfo;

s32 NetComm_DDNS_Install(EM_NET_DDNS eProt);
s32 NetComm_DDNS_Start(SNetDdnsPara* pStart);
s32 NetComm_DDNS_Stop( void );
s32 NetComm_DDNS_Register(SNetDdnsPara* pReg);

s32 NetComm_SendMail(SMailContent* pMail);

// extern ..
/*
 网络模块初始化
 
 参数：
	pCfg: PSNetCommCfg
	pReqStreamCB: PStreamRequestCB
	pReqKeyCB : PRequestKeyFrameCB 
	
 返回:
  类型 s32
  说明 0 成功，非0 失败
 
*/
s32 NetCommInit(
	PSNetCommCfg pCfg
);

/*
 网络模块预览音视频帧发送函数,调用模块收到流请求后通过此函数发送流数据
 
 参数：
	pHead: PSNetComStmHead
	pBuf: u8*
	
 返回: 
  类型 s32
  说明 0 成功， 非0 失败

*/
s32 NetCommSendPreviewFrame(PSNetComStmHead pHead, u8* pBuf);

/*
 写入网络参数
 参数: 
 	pConfig: SNetSet
 	
 返回:
  类型 s32
  说明 0 成功， 非0 失败
 	
*/
s32 NetComm_WritePara( SNetPara* pConfig );


/*
 读出网络参数
 参数: 
 	pConfig: SNetPara
 	
 返回:
  类型 s32
  说明 0 成功， 非0 失败
 	
*/
s32 NetComm_ReadPara(SNetPara* pConfig);
s32 NetComm_ReadParaEx(SNetPara* pConfig);
s32 NetComm_StartDHCP();
s32 NetComm_StopDHCP();
s32 NetComm_PPPoE_Start(char *pszUser, char* pszPasswd);
s32 NetComm_PPPoE_Stop(u8 flag);
u8 NetComm_GetState();
s32 NetComm_WebRestart(u16 nPort);
s32 NetComm_NetUserCtrl(u8 op, SNetUser* pUser);
void NetComm_InitMultiCastInfo(SNetMultiCastInfo* pMultiCastInfo);
s32 NetComm_DDNSIpUpdate(void);
u8 NetComm_GetVoipState( void );
void NetComm_UpdateProgNotify(u8 nProgress);

void NetComm_GetMultiCastInfo(SMultiCastInfo* pInfo);
void NetComm_SetMultiCastInfo(SMultiCastInfo* pInfo);

void net_write_upload_alarm(u32 key);
void upload_alarm(u32 key);
void SetRefreshDiskStatue(u8 flag);
u8 GetCheckDiskStatue();

//yaogang modify 20141209 深广平台
//s32 net_sg_init();


#ifdef __cplusplus
}
#endif

#endif // __NETCOMM_H_

