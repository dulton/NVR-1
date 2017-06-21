#ifdef _VXWORKS_
#include "vxworks.h"
#include "semLib.h"
#include "taskLib.h"
#include "sysLib.h"
#include "usrLib.h"
#include "msgQLib.h"
#include "tickLib.h"
#include "string.h"
#include "sockLib.h"
#include "inetLib.h"
#include "routeLib.h"
#include "ioLib.h"
#include "stdio.h"
#define SEMHANDLE					 SEM_ID
#define TASKHANDLE					 u32
#elif defined WIN32
#include <wtypes.h>
#define SEMHANDLE					 HANDLE
#else
#include <semaphore.h>
#define SEMHANDLE					 sem_t *
#endif

#pragma pack(push, 1)

#define BANK1_OFFSET	8
#define BANK2_OFFSET	0
#define BANK3_OFFSET	0

#define AD_SIZE         20
#define VCODEC_SIZE		108
#define SERIAL_SIZE		16
#define SERIAL_IP_SIZE	40
#define NET_SIZE		16
#define MANAGE_SIZE		32

#define ControlIP1_Offset 		0
#define ControlIP2_Offset 		4
#define ControlIP3_Offset 		8
#define ControlIP4_Offset 		12
#define ContrlPort_Offset 		16
#define ControlHBeat_Offset		20
#define ControlAlarm_Offset     24
#define ControlAlarmPort_Offset 28

typedef struct _VSWC_MANAGE_CONTEXT
{
	unsigned int 	m_ControlCenterIP1;//控制中心1的IP
	unsigned int 	m_ControlCenterIP2;	
	unsigned int 	m_ControlCenterIP3;	
	unsigned int 	m_ControlCenterIP4;	
	
	unsigned int 	m_ControlPort;//控制中心的控制端口----默认对等端口
	unsigned int 	m_HeartBeatFre;	//向控制中心汇报的发报频率（毫秒）
	
	unsigned int    m_AlarmCenter;
	unsigned int  	m_AlarmPort;
}VSWC_MANAGE_CONTEXT;////网络字节；

#define VCodecResolution_Offset 		0
#define VCodecRateControl_Offset 		4
#define VCodecBitRate_Offset			8
#define VCodecFrameRate_Offset			12
#define VCodecMinQuant_Offset           16
#define VCodecMaxQuant_Offset           20
#define VCodecIFrameInter_Offset		24
#define VCoddecBFrameInter_Offset       28

#define VCodecDestinationIP_Offset      52
#define VCodecDestinationPort_Offset    56
#define VCodecTransProtocal_Offset      60
#define VCodecGUID_Offset				64
#define VCodecPacketLen_Offset			80
#define VCodecInitStream_Offset			84
#define VCodecCurrentStream_Offset      88
#define VCodecIFrameNeed_Offset         92
#define VCodecFrameNum_Offset			96
#define VCodecSequenceNum_Offset		100
#define VCodecMotionD_Offset			104

/////////该结构描述视频编码模块的工作上下文
struct VSWC_VCODEC_ENC_CONTEXT
{
//	//该成员描述前端输入的分辨率： QCIF  CIF  2CIF 4CIF    这里关于分辨率的描述 留意一下
	unsigned int	m_VideoResolution;
	unsigned int   	m_RateControlMode;///////////////////////////////////
	unsigned int 	m_BitRate;///->码流控制模型的建立，是个有意义的工作
	unsigned int 	m_FrameRate;////可以几秒一帧最好-----考虑到某些应用的需要
	unsigned int  	m_MinQuantizer;
	unsigned int  	m_MaxQuantizer;
	unsigned int  	m_IFrameInterval;
	unsigned int  	m_BFrameEveryPFrame;////////目前暂时不用
	
	unsigned int	m_Reserved1;/////预留
	unsigned int	m_Reserved2;	
	unsigned int	m_Reserved3;	
	unsigned int	m_Reserved4;	
	unsigned int	m_Reserved5;	
	
	//////////传输
	unsigned int   	m_DestinationIP;
	unsigned int 	m_DestinationPort;
	unsigned int	m_TransProtocal;
	GUID            m_ServerGUID;//////用于TCP连接的建立
	unsigned int	m_PacketLen;//////指定每个发送包的长度
	
	/////////工作状态
	unsigned int	m_bInitialStream;////是否一上电就发送视频流
	unsigned int	m_bCurrentStream;////表明当前是否发送该视频流，该流是否被打开
	unsigned int	m_bNeedIFrame;
	unsigned int	m_EncodeFrameNum; //一秒钟内帧的序号（最大值NTSC(1~30),PAL(1~25)）
	unsigned int	m_VideoSequenceNum;
	unsigned int	m_bMotionDetection;
	
};

struct VSWC_VCODEC_ENC_NOTIFY
{
	int   m_FrameRateNotify;//帧率改变通知
	int   m_BitRateNofity;//码率改变通知
	int   m_ResolutionNotify;//分辨率改变通知
	int   m_MaxQuanyNotify;//最大量化参数改变通知
	int   m_MinQuantNotify;//最小量化参数改变通知
	int   m_AnalogNotify;//输入制式改变通知
	
};

#define VideoStandard_Offset 	0
#define Brightness_Offset       4
#define Contrast_Offset			8
#define Saturate_Offset			12
#define Hue_Offset				16
//////////该结构描述服务器AD转换模块的工作上下文
struct VSWC_AD_CONTEXT
{
	//该成员描述前端输入的制式： PAL    NTSC
	unsigned int		m_VideoStandard;
	//该成员描述前端输入的亮度
	unsigned int		m_Brightness;
	//该成员描述前端输入的对比度
	unsigned int		m_Contrast;
	//该成员描述前端输入的饱和度	
	unsigned int		m_Saturate;
	//该成员描述前端输入的色彩	
	unsigned int		m_Hue;
	
};

#define BaudRate_Offset 0
#define StopBit_Offset  4
#define CheckNum_Offset	8
#define Databits_Offset 12

///////////该结构描述串口属性
struct VSWC_SERIAL_CONTEXT
{
	//该成员描述串口波特率
	unsigned int  m_BaudRate;
	//该成员描述串口停止位
	unsigned int   m_StopBit;
	//该成员描述串口校验位
	unsigned int   m_CheckNum;
	//该成员描述串口数据位
	unsigned int   m_DataBits;
	
};

#define UnitIP_Offset 	0
#define UnitMask_Offset	4
#define UnitGate_Offset 8
#define UnitDNS_Offset	12

///////////该结构用于描述服务器的网络接口
struct VSWC_NETINTERFACE_CONTEXT
{
	unsigned int m_UnitIP;
	unsigned int m_UnitMask;
	unsigned int m_UnitGate;
	unsigned int m_UnitDNSServerIP;
};

struct VSWC_VCODEC_CONTROL_DS
{
	SEMHANDLE    	m_CountMutex;
	SEMHANDLE    	m_QueueSem;
	// --------------------------------------------------------------------------
	// video buffer 逻辑上可以看成一个队列（fifo）,划分为固定长度，考虑codec的
	//特性，每块buf目前设计为64KB
	// --------------------------------------------------------------------------
	unsigned char   *m_pVideoBuf;
	unsigned char   m_FrameBufNum;
	unsigned int    m_PerBufLen;
	
	unsigned char	m_ReadyBufCount;
	
	unsigned char   m_CodecPointer;
	unsigned char   m_SendPointer;	
	
};

struct SYSYTEM_ALARM_MSG
{
	unsigned short	m_AlarmCapaID;
	unsigned short  m_AlarmAppID;
	unsigned short	m_EventType;
	unsigned int	m_EventArg;
};

struct SYSTEM_SERAIL_DATA_MSG
{
	unsigned int   m_MsgLen;
	void           *m_MsgPointer;
};

#define SerialRemoteIP_Offset 		0
#define SerialRemotePort_Offset		4
#define SerialLocalPort_Offset		8
#define SerialInitialStream_Offset	12
#define SerialCurrentStream_Offset	16
#define SerialGUID_Offset			20
#define SerialConn_Type				36

struct VSWC_SERIAL_IP_CONTROL
{
	unsigned int 	m_RemoteIP;
	unsigned int 	m_RemotePort;
	unsigned int	m_LocalPort;
	unsigned int  	m_InitialStream;
	unsigned int  	m_CurrentStream;
	GUID            m_TargetGUID;
	unsigned int    m_ConnType;
};

/////以下的代码放在vswcdefines.h中
#define AUDIO_FORMAT_PCM                1
#define AUDIO_FORMAT_G723				4
#define AUDIO_LISTEN_PORT				59998

#define AENC_SIZE						20
#define ADEC_SIZE                       20

#define AEncInitStream_Offset 			0
#define AEncCurrentStream_Offset 		4 
#define AEncCodeType_Offset				8
#define AEncRemoteIP_Offset				12
#define AEncRemotePort_Offset			16

/////////////////////该上下文保存在BANK3中,20字节,对于pcm编码：默认8K  16位   单声道
struct VSWC_AUDIO_ENCODE_CONTEXT
{
	unsigned int m_bInitStream;	
	unsigned int m_bCurrentStream;
	
	unsigned int m_CodecType;///支持两种模式：PCM和G.723
	unsigned int m_DestinationIP;
	unsigned int m_DestinationPort;
};

#define ADecInitStream_Offset 		0
#define ADecCurrentStream_Offset 	4 
#define ADecCodeType_Offset			8
#define ADecRemoteIP_Offset			12
#define ADecRemotePort_Offset		16
/////////////////////该上下文保存在BANK3中,20字节,对于PCM解码,要还原成44K  16 双声道播放
struct VSWC_AUDIO_DECODE_CONTEXT
{
	unsigned int m_bInitStream;	
	unsigned int m_bCurrentStream;
	
	unsigned int m_CodecType;///支持两种模式：PCM和G.723
	unsigned int m_RemoteIP;
	unsigned int m_LocalPort;///////LocalPort :可以固定下来。
};

#define MDETECT_CONTEXT_SIZE		254
#define MD_ENABLE_Offset			0
#define MD_MASKSTRLEN_OFFSET		4
#define MD_CONSECUTIVEFRAME_Offset 	8
#define MD_UPPERTHRESHHOLD_Offset	12
#define MD_LOWERTHRESHHOLD_Offset	16
#define MD_VECTORTHRESHHOLD_Offset	20
#define MD_FROMR2L_Offset			24
#define MD_FROML2R_Offset			28
#define MD_FROMT2B_Offset			32
#define MD_FROMB2T_Offset			36
#define MD_MASKENABLE_Offset		40
#define MD_MASKSTR_Offset			44

struct MOTION_DETECTION_CONTEXT
{
	unsigned int m_bMotionDetection;
	unsigned int m_MaskStrLen;

	unsigned int m_ConsecutiveFrame;	
	unsigned int m_UpperThreshhold;
	unsigned int m_LowerThreshhold;

	unsigned int m_VectorThreshhold;
	unsigned int m_FromR2L;
	unsigned int m_FromL2R;
	unsigned int m_FromT2B;
	unsigned int m_FromB2T;

	unsigned int m_bMAlert;
	unsigned int m_SFrameCount;
	unsigned int m_MFrameCount;

	unsigned int m_bMaskEnable;///////////////////////////////////////40字节
	char         m_MotionMaskStr[210];
};

struct VSWC_VCODEC_CONTROL_DS  		m_VCodecControlDS;

/*
struct VSWC_VCODEC_ENC_CONTEXT 		m_VCodecEncContext;
struct VSWC_VCODEC_ENC_CONTEXT 		m_VCodecEncContext2;
struct VSWC_VCODEC_ENC_CONTEXT 		m_VCodecEncContext3;
struct VSWC_VCODEC_ENC_CONTEXT 		m_VCodecEncContext4;
*/

struct VSWC_VCODEC_ENC_NOTIFY		m_EncNotify;
struct VSWC_VCODEC_ENC_NOTIFY		m_EncNotify2;
struct VSWC_VCODEC_ENC_NOTIFY		m_EncNotify3;
struct VSWC_VCODEC_ENC_NOTIFY		m_EncNotify4;

struct VSWC_AD_CONTEXT              m_ADContext1;
struct VSWC_AD_CONTEXT              m_ADContext2;
//////////////以下两个ad上下文保存在bank3中
struct VSWC_AD_CONTEXT              m_ADContext3;
struct VSWC_AD_CONTEXT              m_ADContext4;

VSWC_MANAGE_CONTEXT            		m_CenterContext;

struct VSWC_SERIAL_CONTEXT          m_485Context;
struct VSWC_SERIAL_CONTEXT          m_232Context;
struct VSWC_NETINTERFACE_CONTEXT    m_NetContext;
struct VSWC_SERIAL_IP_CONTROL		m_485IPControl;
struct VSWC_SERIAL_IP_CONTROL		m_232IPControl;

struct VSWC_AUDIO_ENCODE_CONTEXT 	m_ACodecEncContext;
struct VSWC_AUDIO_DECODE_CONTEXT 	m_ACodecDecContext;

struct MOTION_DETECTION_CONTEXT  	m_MotionContext;//默认motion detection 只对第一路视频输入的第一路流有效
struct MOTION_DETECTION_CONTEXT  	m_MotionContext3;//2
struct MOTION_DETECTION_CONTEXT  	m_MotionContext4;//3
struct MOTION_DETECTION_CONTEXT  	m_MotionContext5;//4

#ifdef _VXWORKS_
MSG_Q_ID							m_AMsgID;
MSG_Q_ID							m_485MsgID;
MSG_Q_ID							m_232MsgID;

unsigned int                        m_bInterrupt;
unsigned char 						readromcontence;

///////IIC MUTEX;
SEMHANDLE    	m_IICMutex;
SEMHANDLE    	m_MaskStrMutex;
int 			Advideo1;
int				Advideo2;
int 			Advideo3;
int				Advideo4;
int				m_bConfigMotionD;
#endif

#pragma pack( pop )
