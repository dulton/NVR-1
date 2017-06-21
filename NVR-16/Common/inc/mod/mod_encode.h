#ifndef _ENCODE_COMMON_H_
#define _ENCODE_COMMON_H_

#include "common_basetypes.h"
#include "common_geometric.h"
#include "common_mutex.h"
#include "common_semaphore.h"
#include "common_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	EM_PIC_LEVEL_BEST = 0,
	EM_PIC_LEVEL_BETTER,
	EM_PIC_LEVEL_GOOD,
	EM_PIC_LEVEL_NORMAL,
	EM_PIC_LEVEL_BAD,
	EM_PIC_LEVEL_WORSE,
	//EM_PIC_LEVEL_WORST,
} EMPICLEVEL;

typedef enum
{
	EM_CHN_VIDEO_SUB = 0, 		//视频主通道
	EM_CHN_VIDEO_MAIN, 			//视频副通道
	EM_CHN_AUDIO, 				//音频通道
	EM_CHN_TALK, 				//对讲通道
	EM_CHN_VIDEO_SNAP, 			//视频抓图
	//csp modify 20130423
	EM_CHN_VIDEO_THIRD, 		//视频第三路码流
} EMCHNTYPE;

typedef enum
{
	//视频编码类型
	EM_ENCODE_VIDEO_MJPEG = 26, 		//Motion JPEG
	EM_ENCODE_VIDEO_H261 = 31, 		//H.261
	EM_ENCODE_VIDEO_MP2 = 33, 			//MPEG2 video
	EM_ENCODE_VIDEO_H263 = 34, 		//H.263
	EM_ENCODE_VIDEO_MP4 = 97, 			//MPEG-4
	EM_ENCODE_VIDEO_H264 = 98, 		//H.264
	
	//语音编码类型
	EM_ENCODE_AUDIO_PCMU = 0, 			//G.711 ulaw
	EM_ENCODE_AUDIO_PCMA = 8, 			//G.711 Alaw
	EM_ENCODE_AUDIO_G722 = 9, 			//G.722
	EM_ENCODE_AUDIO_G7231 = 4, 		//G.7231
	EM_ENCODE_AUDIO_G728 = 15, 		//G.728
	EM_ENCODE_AUDIO_G729 = 18, 		//G.729
	EM_ENCODE_AUDIO_RAW = 19, 			//raw audio
	EM_ENCODE_AUDIO_ADPCM = 20, 		//adpcm
	EM_ENCODE_AUDIO_ADPCM_HS = 21, 	//海思adpcm
	EM_ENCODE_AUDIO_MP3 = 96,			//mp3
} EMENCODETYPE;

typedef enum
{
	EM_VIDEO_QCIF = 0,		//PAL:176x144, NTSC:176x112
	EM_VIDEO_CIF = 1,		//PAL:352x288, NTSC:352x240
	EM_VIDEO_HALF_D1 = 2,   //PAL:704*288, NTSC:704x240	
	EM_VIDEO_4CIF = 3,		
	EM_VIDEO_D1 = 4,		//PAL:704x576, NTSC:704x480
	EM_VIDEO_720P = 5,		//1280*720
	EM_VIDEO_1080P = 6,		//1920*1080
	EM_VIDEO_960H = 7,		//1920*1080
	EM_VIDEO_VGA = 8,		//640*480
} EMVIDEORESOLUTION;

typedef enum
{
	EM_FRAME_TYPE_P = 0, /*PSLICE types*/
	EM_FRAME_TYPE_I = 1, /*ISLICE types*/
} EMVIDEOFRAMETYPE;

typedef enum
{
	EM_BITRATE_VARIABLE = 0, 		//变码率
	EM_BITRATE_CONST, 				//定码率
} EMBITRATETYPE;

typedef enum
{
	EM_ENCODE_BOARD_COMMON 			= 0,	//通用
	EM_ENCODE_BOARD_HISI3520_16D1 	= 1,	//针对hisi3520芯片16路d1系列 (R9616S)
	//
} EMENCODEBOARDTYPE;

typedef struct
{
	EMENCODETYPE emEncodeType; 				//参考 EMENCODETYPE
	EMVIDEOFRAMETYPE emVideoFrameType; 		//参考 EMVIDEOFRAMETYPE
	u64 nTimeStamp; 						//时间戳 ms
	u32 nLen;
	char* pData;
	EMVIDEORESOLUTION emResol;
	u32 nBitRate;
	//u8 nReserve[8];
	u16 width;
	u16 height;
	u8 nReserve[4];
} SEncodeHead;

typedef void (* PFNEncodeRequest)(SEncodeHead* sEncodeHead, u32 dwContent);

//csp modify 20130106
typedef void (* PFNEncodeEXCEPTION)(u8 nType);

typedef struct
{
	u32 nMainNum; 			//视频主编码通道数
	u32 nMainMaxFrameSize;	//主码流最大单帧大小
	u32 nSubNum; 			//视频副编码通道数
	u32 nSubMaxFrameSize;	//子码流最大单帧大小
	u32 nSnapNum; 			//视频抓图编码通道数
	u32 nSnapMaxFrameSize;	//抓图最大单帧大小
	u32 nAudioNum; 			//音频编码通道数
	u32 nAudioMaxFrameSize;	//音频最大单帧大小
	
	u32 nTalkNum; 			//对讲编码通道数
	u32 nTalkMaxFrameSize;	//对讲最大单帧大小
	
	EMENCODEBOARDTYPE emBoardType;//主板类型
	
	u8 nReserve[4]; 		//保留字段
	
	//csp modify 20130106
	PFNEncodeEXCEPTION EncExceptCB;
	
	//csp modify 20130316
	u8 nVideoStandard;		//视频制式(EMBIZVIDSTANDARD)
} SEncodeCapbility;

typedef struct 
{
	u32 nBitRate; 							//编码位率 以KB为单位 即（字节数为nBitRate << 10）
	u32 nFrameRate; 						//编码帧率
	u32 nGop;								//关键帧间隔
	u32 nMinQP;								//最小QP值
	u32 nMaxQP;								//最大QP值
	EMVIDEORESOLUTION emVideoResolution;	//参考 EMVIDEORESOLUTION
	EMENCODETYPE emEncodeType; 				//参考 EMENCODETYPE
	EMBITRATETYPE emBitRateType; 			//参考 EMBITRATETYPE
	EMPICLEVEL emPicLevel;					//参考EMPICLEVEL
	u8 nReserve[8];
} SVideoParam;

typedef struct
{
	
} SVideoSnapParam;

typedef struct
{
	EMENCODETYPE emEncodeType; 			//参考 EMENCODETYPE
	u32 nSampleRate; 					//音频采样率
	u32 nSampleDuration; 				//音频采样时长
	u32 nSampleSize; 					//音频采样长度
	u8 nBitWidth; 						//音频位宽
	u8 nReserve[3];
} SAudioParam;

typedef	struct
{
	u8 nForce; //如果参数的改变必须停止编码是否强制停止 //默认要求置0
	union
	{
		SVideoParam sVideoMainParam; 		//参考SVideoMainParam
		SVideoParam sVideoSubParam; 		//参考SVideoSubParam
		SVideoSnapParam sVideoSnapParam; 	//参考SVideoSnapParam
		SAudioParam sAudioParam; 			//参考SAudioParam
	};
} SEncodeParam;

typedef enum
{
	EM_ENCODE_FONTSIZE_SMALL,
	EM_ENCODE_FONTSIZE_NORMAL,
	EM_ENCODE_FONTSIZE_ALL
} EMENCODEFONTSIZE;

typedef enum
{
    EM_ENCODE_OSD_CHNNAME = 0,
    EM_ENCODE_OSD_TIME,
    EM_ENCODE_OSD_EXT1,
    EM_ENCODE_OSD_NUM,
} EMENCODEOSDTYPE;

typedef s32 (* PFNEncodeTextToOsd)(PARAOUT u8* pBuffer, s32 w, s32 h, s8* strOsd, EMENCODEFONTSIZE emSize);

//csp modify 20130423
typedef struct
{
	u8	support;
	u8	wnd_num_threshold;
	EMVIDEORESOLUTION emResolution;
} SThirdStreamProperty;

s32 ModEncodeInit(SEncodeCapbility* psEncodeCap);
s32 ModEncodeDeinit(void);
s32 ModEncodeSetParam(u8 nChn, EMCHNTYPE emChnType, SEncodeParam* psEncodeParam);
s32 ModEncodeGetParam(u8 nChn, EMCHNTYPE emChnType, SEncodeParam* psEncodeParam);
s32 ModEncodeRequestStart(u8 nChn, EMCHNTYPE emChnType, PFNEncodeRequest pfnEncodeCB, u32 nContent);
s32 ModEncodeRequestStop(u8 nChn, EMCHNTYPE emChnType);

s32 ModEncodeOsdInit(u8 nChn, EMENCODEOSDTYPE emOsdType, SRect* pRect, SRgb* pBgColor);
s32 ModEncodeOsdDeinit();
s32 ModEncodeOsdRegistCB(PFNEncodeTextToOsd pfnGetOSDCB);
s32 ModEncodeOsdResolRef(u8 nChn, EMVIDEORESOLUTION emReSolRef);
s32 ModEncodeOsdSet(u8 nChn, EMENCODEOSDTYPE emOsdType, SRect* pRect, EMENCODEFONTSIZE emSize, s8* strOsd);
s32 ModEncodeOsdShow(u8 nChn, EMENCODEOSDTYPE emOsdType, u8 nShow);

s32 ModEncodeGetKeyFrameEx(u8 nChn, SVideoParam* psEncodeParam, u8 bSub);

//csp modify 20130423
s32 ModEncodeSupportThirdStream();

//csp modify 20130423
s32 ModEncodeSetThirdStreamProperty(SThirdStreamProperty *psProperty);
s32 ModEncodeGetThirdStreamProperty(SThirdStreamProperty *psProperty);

//yaogang modify 20150212
s32 ModEncSnapCreateChn(u8 EncChn, int Width, int Height);
s32 ModEncSnapDestroyChn(u8 EncChn);
//s32 ModEncGetSnapshot(u8 StreamChn, u8 EncChn, u8 *pdata, u32 *psize);
s32 ModEncGetSnapshot(u8 EncChn, u16 *pWidth, u16 *pHeight, u8 *pdata, u32 *psize);




#ifdef __cplusplus
}
#endif


#endif

