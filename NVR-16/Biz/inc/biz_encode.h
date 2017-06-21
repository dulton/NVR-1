#ifndef _BIZ_ENCODE_H_
#define _BIZ_ENCODE_H_

#include "biz.h"
#include "mod_encode.h"
#include "biz_manager.h"

typedef enum
{
	EM_BIZ_ENCODE_BOARD_COMMON 			= 0,	//通用
	EM_BIZ_ENCODE_BOARD_HISI3520_16D1 	= 1,	//针对hisi3520芯片16路d1系列 (R9616S)
	//
}EMBIZENCODEBOARDTYPE;

typedef struct
{
	u32 nMainNum; 			//视频主编码通道数
	u32 nMainMaxFrameSize;		//主码流最大单帧大小
	u32 nSubNum; 			//视频副编码通道数
	u32 nSubMaxFrameSize;		//子码流最大单帧大小
	u32 nSnapNum; 			//视频抓图编码通道数
	u32 nSnapMaxFrameSize;		//抓图最大单帧大小
	u32 nAudioNum; 			//音频编码通道数
	u32 nAudioMaxFrameSize;		//音频最大单帧大小
	EMBIZENCODEBOARDTYPE emChipType;
} SBizEncode;

typedef struct
{
    u8 nChn;
    EMCHNTYPE emChnType;
    SEncodeHead* psEncodeHead;
} SBizEncodeData;

typedef enum
{
    EM_BIZ_ENCODE_OSD_CHNNAME = 0,
    EM_BIZ_ENCODE_OSD_TIME,
    EM_BIZ_ENCODE_OSD_EXT1,
    EM_BIZ_ENCODE_OSD_NUM,
} EMBizENCODEOSDTYPE;


s32 EncodeInit(u8 nEnable, SBizEncode* psInitPara);
s32 EncodeOsdInit();
s32 EncodeChangeOsdResolution(u8 nChn);
s32 EncodeChangeOsdPosition(u8 nChn, EMBizENCODEOSDTYPE emOsdType, u16 nX, u16 nY);
s32 EncodeOsdShow(u8 nChn, EMBizENCODEOSDTYPE emOsdType, u8 bShow);
s32 VEncMainSetParam(u8 nChn, SBizEncodePara* psEncodeParam);
s32 VEncSubSetParam(u8 nChn, SBizEncodePara* psEncodeParam);
s32 EncodeGetParam(u8 nChn, EMCHNTYPE emChnType, PARAOUT SBizEncodePara* psEncodeParam);
s32 EncodeRequestStart(u8 nChn, EMCHNTYPE emChnType);
s32 EncodeRequestStartWithoutRec(u8 nChn, EMCHNTYPE emChnType);
s32 EncodeRequestStop(u8 nChn, EMCHNTYPE emChnType);
s32 EncodeRequestStopWithoutRec(u8 nChn, EMCHNTYPE emChnType);
s32 EncodeTimeOsdSetAllChn(u8* strTime);
s32 EncodeStrOsdUpdate(u8 chn, u8* strOsd);

#endif

