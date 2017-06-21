#ifndef _MEDIA_STREAM_H_
#define _MEDIA_STREAM_H_

//#include "iflytype.h"
#include "common_basetypes.h"

#include <stdio.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define SOCKHANDLE SOCKET
#define SEMHANDLE  HANDLE
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
typedef struct sockaddr_in SOCKADDR_IN;
#define ADDR_ANY   0
#define SEMHANDLE  sem_t *
#define SOCKHANDLE int
#define INVALID_SOCKET		(-1)
#define SOCKET_ERROR		(-1)
#define HIWORD(l)           ((u16)(((u32)(l) >> 16) & 0xFFFF))  //08-03-08 chenjie
#define LOWORD(l)           ((u16)(l))							//08-03-08 chenjie
#define HIBYTE(w)           ((u8)(((u16)(w) >> 8) & 0xFF))		//08-03-08 chenjie
#define LOBYTE(w)           ((u8)(w))							//08-03-08 chenjie
#endif

#ifndef MEDIA_TYPE
#define MEDIA_TYPE
//图像编码类型
#define  MEDIA_TYPE_H264	 (u8)98//H.264//可能是109?
#define  MEDIA_TYPE_MP4	     (u8)97//MPEG-4
#define  MEDIA_TYPE_H261	 (u8)31//H.261
#define  MEDIA_TYPE_H263	 (u8)34//H.263
#define  MEDIA_TYPE_MJPEG	 (u8)26//Motion JPEG
#define  MEDIA_TYPE_MP2		 (u8)33//MPEG2 video

//语音编码类型
#define	 MEDIA_TYPE_MP3	     (u8)96//mp3
#define  MEDIA_TYPE_PCMU	 (u8)0//G.711 ulaw
#define  MEDIA_TYPE_PCMA	 (u8)8//G.711 Alaw
#define	 MEDIA_TYPE_G7231	 (u8)4//G.7231
#define	 MEDIA_TYPE_G722	 (u8)9//G.722
#define	 MEDIA_TYPE_G728	 (u8)15//G.728
#define	 MEDIA_TYPE_G729	 (u8)18//G.729
#define	 MEDIA_TYPE_RAWAUDIO (u8)19//raw audio
//#define  MEDIA_TYPE_ADPCM	 (u8)20//adpcm
#define  MEDIA_TYPE_ADPCM	 (u8)21//adpcm//wrchen 091117

#endif//MEDIA_TYPE

#define  MEDIASTREAM_NO_ERROR               (u16)0//成功返回值  
#define  ERROR_MEDIA_STREAM_BASE            (u16)16000
#define  ERROR_SND_PARAM					(ERROR_MEDIA_STREAM_BASE+1)//设置发送模块参数出错 
#define  ERROR_SND_NOCREATE					(ERROR_MEDIA_STREAM_BASE+2)//发送模块没有创建
#define  ERROR_SND_MEMORY					(ERROR_MEDIA_STREAM_BASE+3)//发送模块内存操作出错
#define  ERROR_SND_CREATE_SOCK				(ERROR_MEDIA_STREAM_BASE+4)//发送模块创建socket
#define  ERROR_RTP_SSRC                     (ERROR_MEDIA_STREAM_BASE+5)//RTP同步源错误.
#define  ERROR_LOOPBUF_CREATE               (ERROR_MEDIA_STREAM_BASE+6)//环状缓冲创建错误  
#define  ERROR_RTP_NO_INIT                  (ERROR_MEDIA_STREAM_BASE+7)//RTP类有些对象没创建
#define  ERROR_RTCP_NO_INIT                 (ERROR_MEDIA_STREAM_BASE+8)//RTP类有些对象没创建
#define  ERROR_RTCP_SET_TIMER               (ERROR_MEDIA_STREAM_BASE+9)//RTCP设置定时器出错
#define  ERROR_RTP_SSRC_COLLISION           (ERROR_MEDIA_STREAM_BASE+10)//RTP 同步源出错
#define  ERROR_SOCK_INIT                    (ERROR_MEDIA_STREAM_BASE+11)//socket 没有初始化
#define  ERROR_H261_PACK_NUM                (ERROR_MEDIA_STREAM_BASE+12)//H261的包数不合理
#define  ERROR_PACK_TOO_LEN                 (ERROR_MEDIA_STREAM_BASE+13)//G.711的数举包太长
#define  ERROR_H263_PACK_NUM                (ERROR_MEDIA_STREAM_BASE+14)//H263的包数不合理
#define  ERROR_H263_PACK_TOOMUCH            (ERROR_MEDIA_STREAM_BASE+15)//H263的数据包太长

#define  ERROR_NET_RCV_PARAM                (ERROR_MEDIA_STREAM_BASE+100)//设置接收模块参数出错
#define  ERROR_NET_RCV_NOCREATE             (ERROR_MEDIA_STREAM_BASE+101)//接收模块没有创建
#define  ERROR_NET_RCV_MEMORY               (ERROR_MEDIA_STREAM_BASE+102)//接收模块内存操作出错
#define  ERROR_RCV_RTP_CREATE               (ERROR_MEDIA_STREAM_BASE+103)//接收模块RTP创建失败
#define  ERROR_RCV_RTP_CALLBACK             (ERROR_MEDIA_STREAM_BASE+104)//接收模块设置RTP回调函数失败
#define  ERROR_RCV_RTP_SETREMOTEADDR        (ERROR_MEDIA_STREAM_BASE+105)//接收模块设置RTP远端地址失败
#define  ERROR_CREATE_LOOP_BUF              (ERROR_MEDIA_STREAM_BASE+106)//创建环状缓冲失败
#define  ERROR_RCV_NO_CREATE                (ERROR_MEDIA_STREAM_BASE+107)//接收模块接收对象没有创建

#define  ERROR_WSA_STARTUP                  (ERROR_MEDIA_STREAM_BASE+200)//wsastartup error
#define  ERROR_CREATE_SEMAPORE              (ERROR_MEDIA_STREAM_BASE+201)// create semapore error
#define  ERROR_SOCKET_CALL                  (ERROR_MEDIA_STREAM_BASE+202)//调用socket() 函数出错
#define  ERROR_BIND_SOCKET                  (ERROR_MEDIA_STREAM_BASE+203)//socket 绑定出错
#define  ERROR_CREATE_THREAD                (ERROR_MEDIA_STREAM_BASE+204)//创建线程出错 

#define  ERROR_LOOPBUF_FULL                 (ERROR_MEDIA_STREAM_BASE+205)//循环缓冲满

#define	 ERROR_SND_FRAME                    (INT32)-1;

#define  MIN_PRE_BUF_SIZE                   (INT32)28// G.711需再加一字节

#define  MAX_H261_HEADER_LEN				(INT32)292//
#define  MAX_H263_HEADER_LEN				(INT32)3076//

#ifndef  MAX_NETSND_DEST_NUM
#define  MAX_NETSND_DEST_NUM				5
#endif

#define SAFE_DELETE(p) {if(p){free(p);p=NULL;}}
#define IFLYFAILED(p) ((p)!=MEDIASTREAM_NO_ERROR)

//#ifndef MAX_FRAME_SIZE
#define MAX_FRAME_SIZE				(INT32)256*1024//zlb20100802
//#define MAX_FRAME_SIZE				(INT32)80*1024
//#endif

#define MAX_PACK_NUM				0x60// 对于最大128kbyte的数据帧而言，小包数 <  96
#define MAX_PACK_SIZE				(INT32)1450//
#define MAX_RCV_PACK_SIZE			(INT32)8192// 接收时最大的包长

#define LOOP_BUF_UINT_NUM			(INT32)200//400
#define REPEAT_LOOP_BUF_UINT_NUM	(INT32)600//
#define MAX_SESSION_NUM				(INT32)128//

#define MAX_EXTEND_PACK_SIZE		(INT32)(MAX_PACK_SIZE - MAX_PACK_EX_LEN - EX_HEADER_SIZE)
#define MAX_EXTEND_PACK_NUM			(INT32)((MAX_FRAME_SIZE+MAX_EXTEND_PACK_SIZE-1)/MAX_EXTEND_PACK_SIZE)// 

#define VIDEO_TIME_SPAN				(INT32)40

#define RTP_FIXEDHEADER_SIZE		(INT32)12//
#define EX_HEADER_SIZE				(INT32)4//
#define MAX_PACK_EX_LEN				(INT32)12//1 TOTALNUM 1Index,1mode,1rate,4FrameId,2width,2Height;
#define EX_TOTALNUM_POS				(INT32)0//小包所在的帧所含的总包数
#define EX_INDEX_POS				(INT32)1//小包在所在帧中的包序号
#define EX_FRAMEMODE_POS			(INT32)2//该帧为音频帧时的音频模式
#define EX_FRAMERATE_POS			(INT32)3//帧率
#define EX_FRAMEID_POS				(INT32)4//帧ID
#define EX_WIDTH_POS				(INT32)8 //该帧为视频帧时的宽
#define EX_HEIGHT_POS				(INT32)10//该帧为视频帧时的高

#define MAX_LOCAL_IP_NUM			16
#define MAX_SND_NUM					1024

#define MAX_AUDIO_PACK_NUM			0x08//音频乱序缓冲数
#define MAX_PACK_QUENE_NUM			0x32//
#define DEFAULT_PACK_QUENE_NUM		0x05//

#define VER_MEDIASTREAM				(const char *)"mediastream 30.01.01.04.040708"


typedef int BOOL32;
typedef	int	INT32;

//Frame Header Structure
typedef struct
{
    u8     m_byMediaType; //媒体类型
    u8    *m_pData;       //数据缓冲
	u32    m_dwPreBufSize;//m_pData缓冲前预留了多少空间，用于加
	// RTP option的时候偏移指针一般为12+4+12
	// (FIXED HEADER + Extence option + Extence bit)
    u32    m_dwDataSize;  //m_pData指向的实际缓冲大小
    u8     m_byFrameRate; //发送帧率,用于接收端
	u32    m_dwFrameID;   //帧标识，用于接收端
	u32    m_dwTimeStamp; //时间戳, 用于接收端
    union
    {
        struct{
			BOOL32    m_bKeyFrame;    //频帧类型(I or P)
			u16       m_wVideoWidth;  //视频帧宽
			u16       m_wVideoHeight; //视频帧宽
		}m_tVideoParam;
        u8    m_byAudioMode;//音频模式
    };
}FRAMEHDR,*PFRAMEHDR;

/*网络参数*/
typedef struct
{
	u32		m_dwRTPAddr;/*RTP地址(网络序)*/
	u16		m_wRTPPort;/*RTP端口(本机序)*/
	u32		m_dwRTCPAddr;/*RTCP地址(网络序)*/
	u16		m_wRTCPPort;/*RTCP端口(本机序)*/

	u32		m_dwContext1;
	
}TNetSession;

/*本地网络参数*/
typedef struct
{
	TNetSession  m_tLocalNet;
	u32			 m_dwRtcpBackAddr;/*RTCP回发地址(网络序)*/
	u16          m_wRtcpBackPort;/*RTCP回发端口(本机序)*/        
}TLocalNetParam;

/*网络发送参数*/
typedef struct
{
	u8	m_byNum;		   /*实际地址对数*/
	TNetSession m_tLocalNet;/*当地地址对*/
	TNetSession m_tRemoteNet[MAX_NETSND_DEST_NUM];/*远端地址对*/
}TNetSndParam;

/*发送模块状态信息*/
typedef struct
{
	u8				m_byMediaType;/*媒体类型*/
	u32				m_dwMaxFrameSize;/*最大的帧大小*/
	u32  			m_dwNetBand;/*发送带宽*/
    u32				m_dwFrameID; /*数据帧标识*/
	u8				m_byFrameRate;/*发送频率*/
    TNetSndParam    m_tSendAddr;/*发送地址*/
}TSndStatus;

/*发送模块统计信息*/
typedef struct
{
    u32		  m_dwPackSendNum;		/*已发送的包数*/
	u32		  m_dwFrameNum;			/*已发送的帧数*/
	u32	      m_dwFrameLoseNum;	    /*由于缓冲满等原因造成的发送的丢帧数*/
}TSndStatistics;

/*接收模块状态信息*/
typedef struct
{
	BOOL	          m_bRcvStart;/*是否开始接收*/
    u32               m_dwFrameID;/*数据帧ID*/
    TLocalNetParam    m_tRcvAddr;/*接收当地地址*/
}TRcvStatus;

/*解码器统计信息*/
typedef struct
{
	u32        m_dwPackNum;/*已接收的包数*/
    u32        m_dwPackLose;/*丟包数*/
    u32        m_dwPackIndexError;/*包乱序数*/
	u32        m_dwFrameNum;/*已接收的帧数*/
	//u32	   m_dwFrameLoseNum;/*由于数据源切换清空队列等原因造成的接收的丢帧数*/
}TRcvStatistics;

typedef struct
{
    u8	     m_byMark;					//是否帧边界1表示最后一包
    u8       m_byExtence;				//是否有扩展信息
    u8       m_byPayload;				//载荷
    u32		 m_dwSSRC;					//同步源
    u16      m_wSequence;				//序列号
    u32      m_dwTimeStamp;				//时间戳
    u8       *m_pExData;                //扩展数据
    INT32    m_nExSize;					//扩展大小:sizeof(u32)的倍数；
    u8		 *m_pRealData;				//媒体数据 
    INT32    m_nRealSize;				//数据大小 
    INT32    m_nPreBufSize;				//m_pRealData前预分配的空间;
}TRtpPack;

typedef struct
{
    u16  m_wFirstTimeSpan;	 //第一个重传检测点
	u16  m_wSecondTimeSpan;  //第二个重传检测点
	u16  m_wThirdTimeSpan;   //第三个重传检测点
	u16  m_wRejectTimeSpan;  //过期丢弃的时间跨度
}TRSParam;

//发送端高级设置参数
typedef struct
{
	s32      m_nMaxSendNum; //根据带块计算的最大发送次数;	
	BOOL32	 m_bRepeatSend; //对于 (mp4/H.264) 是否重发
	u16      m_wBufTimeSpan;
}TAdvancedSndInfo;

//接收端高级设置参数
typedef struct
{
	BOOL32	  m_bConfuedAdjust;  //对于 (mp3) 是否做乱序调整	
	BOOL32	  m_bRepeatSend;     //对于 (mp4/H.264) 是否重发
	TRSParam  m_tRSParam;
}TAdvancedRcvInfo;

struct ifly_netsnd_t;
struct ifly_netrcv_t;

typedef struct
{
	struct ifly_netsnd_t *m_pcNetSnd;
	SEMHANDLE m_hSndSynSem;  //用于对象的单线程操作的同步量
}ifly_mediasnd_t;

typedef struct
{
	struct ifly_netrcv_t *m_pcNetRcv;
	SEMHANDLE m_hRcvSynSem;  //用于对象的单线程操作的同步量
}ifly_mediarcv_t;

//全局MediaSnd列表结构
typedef struct
{
	s32 m_nMediaSndCount;               //发送对象总数 <= MAX_SND_NUM
	ifly_mediasnd_t *m_tMediaSndUnit[MAX_SND_NUM];
}TMediaSndList;

//全局MediaRcv列表结构
typedef struct
{
	s32 m_nMediaRcvCount;               //接收对象总数 <= FD_SETSIZE
	ifly_mediarcv_t *m_tMediaRcvUnit[FD_SETSIZE];
}TMediaRcvList;

//Frame Rcv CallBack Function
typedef void (*PFRAMEPROC)(PFRAMEHDR pFrmHdr, u32 dwContext);
//RTP PACK Call back
typedef void (*PRTPCALLBACK)(TRtpPack *pRtpPack, u32 dwContext);

#ifdef __cplusplus
extern "C" {
#endif

//模块版本信息和编译时间 及 依赖的库的版本和编译时间
void mediastreamver();

//模块帮助信息 及 依赖的库的帮助信息
void mediastreamhelp();

u16  mediastreamStartup();

BOOL mediastreamCleanup();

//创建发送模块
ifly_mediasnd_t* CreateMediaSnd(u32 dwMaxFrameSize, u32 dwNetBand, u8 byFrameRate, u8 byMediaType, u32 dwSSRC);

//删除发送模块
u16 DestroyMediaSnd(ifly_mediasnd_t* pMediaSnd);

//设置网络发送参数(进行底层套结字的创建，绑定端口,以及发送目标地址的设定等动作)
u16 SetMediaSndNetParam(ifly_mediasnd_t* pMediaSnd,TNetSndParam tNetSndParam);

//移除网络发送本地地址参数(进行底层套结字的删除，释放端口等动作)
u16 RemoveMediaSndLocalNetParam(ifly_mediasnd_t* pMediaSnd);

//重置帧ID
u16 ResetMediaSndFrameId(ifly_mediasnd_t* pMediaSnd);
//重置同步源SSRC
u16 ResetMediaSndSSRC(ifly_mediasnd_t* pMediaSnd,u32 dwSSRC);

//重置发送端对于mpeg4或者H.264采用的重传处理的开关,关闭后，将不对已经发送的数据包进行缓存
u16 ResetMediaSndRSFlag(ifly_mediasnd_t* pMediaSnd, BOOL32 bRepeatSnd);

//设置发送选项
u16 SetMediaSndInfo(ifly_mediasnd_t* pMediaSnd,u32 dwNetBand, u8 byFrameRate);
//发送数据包
u16 SendMediaFrame(ifly_mediasnd_t* pMediaSnd,PFRAMEHDR pFrmHdr);

//得到状态
u16 GetMediaSndStatus(ifly_mediasnd_t* pMediaSnd,TSndStatus *pSndStatus);
//得到统计
u16 GetMediaSndStatistics(ifly_mediasnd_t* pMediaSnd,TSndStatistics *pSndStatistics);
//得到发送端高级设置参数(重传等)
u16 GetMediaSndAdvancedInfo(ifly_mediasnd_t* pMediaSnd,TAdvancedSndInfo *pAdvancedSndInfo);


//创建接收模块
ifly_mediarcv_t* CreateMediaRcv(u32 dwMaxFrameSize, PFRAMEPROC pFrameCallBackProc, u32 dwContext, u32 dwSSRC);
ifly_mediarcv_t* CreateMediaRcvRtp(u32 dwMaxFrameSize, PRTPCALLBACK PRtpCallBackProc, u32 dwContext, u32 dwSSRC);

//设置接收地址参数(进行底层套结子的创建，绑定端口等动作)
u16 SetMediaRcvLocalParam(ifly_mediarcv_t* pMediaRcv,TLocalNetParam tLocalNetParam);

//移除接收地址参数(进行底层套结子的删除，释放端口等动作)
u16 RemoveMediaRcvLocalParam(ifly_mediarcv_t* pMediaRcv);

//重置接收端对于mpeg4或者H.264采用的重传处理的开关,关闭后，将不发送重传请求
u16 ResetMediaRcvRSFlag(ifly_mediarcv_t* pMediaRcv,TRSParam tRSParam, BOOL bRepeatSnd);

//开始接收
u16 StartMediaRcv(ifly_mediarcv_t* pMediaRcv);
//停止接收
u16 StopMediaRcv(ifly_mediarcv_t* pMediaRcv);
//得到状态
u16 GetMediaRcvStatus(ifly_mediarcv_t* pMediaRcv,TRcvStatus *pRcvStatus);
//得到统计
u16 GetMediaRcvStatistics(ifly_mediarcv_t* pMediaRcv,TRcvStatistics *pRcvStatistics);

//删除接收模块
u16 DestroyMediaRcv(ifly_mediarcv_t* pMediaRcv);

#ifdef __cplusplus
}
#endif

#endif
