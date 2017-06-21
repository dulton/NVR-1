#ifndef _PREVIEW_H_
#define _PREVIEW_H_

#include "common_basetypes.h"
#include "common_geometric.h"
#include "common_mutex.h"
#include "common_semaphore.h"
#include "common_thread.h"
#include "common_debug.h"

#define PreviewMode pre_mode_e

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	EM_PREVIEW_1SPLIT = 1,
	EM_PREVIEW_4SPLITS = 4,
	EM_PREVIEW_6SPLITS = 6,
	EM_PREVIEW_8SPLITS = 8,
	EM_PREVIEW_9SPLITS = 9,
	EM_PREVIEW_10SPLITS = 10,
	EM_PREVIEW_12SPLITS = 12,
	EM_PREVIEW_13SPLITS = 13,
	EM_PREVIEW_16SPLITS = 16,
	EM_PREVIEW_25SPLITS = 24,
	EM_PREVIEW_24SPLITS = 25,
	EM_PREVIEW_36SPLITS = 32,
	EM_PREVIEW_32SPLITS = 36,
	EM_PREVIEW_64SPLITS = 64,
	EM_PREVIEW_PIP = 254,
	EM_PREVIEW_CLOSE = 255,
} EMPREVIEWMODE;

//预览色彩参数
typedef struct
{
	u16 nHue;
	u16 nSaturation;
	u16 nContrast;
	u16 nBrightness;
} SPreviewImagePara;

typedef struct
{
	u8 nBrightness;
	u8 nContrast;
	u8 nSaturation;
	u8 nHue;
} SVoImagePara;

//轮巡参数
typedef struct
{
    EMPREVIEWMODE emPreviewMode; //轮巡预览模式
    u8 nIsPatrol; //默认是否开启轮巡
    u8 nStops; //轮巡站数(及一次循环总共有多少个监控画面);
    u16 nInterval; //轮巡时间间隔(s)
    u8 pnStopModePara[255]; //每站预览模式参数;参考SBizPreviewPara.nModePara；nStops==0xff时无效
} SPreviewPatrolPara;

//预览回调参数
typedef struct
{
    EMPREVIEWMODE emPreviewMode; //预溃窗口分割模式
    u8 nModePara; //extend parameter for EMPREVIEWMODE
    u8 nAdioChn;
    u8 nMute;
    u8 nVolume;
} SPreviewCbData;

//预览回调函数
typedef void (* FNPREVIEWCB)(SPreviewCbData* psPreviewCbData); 

//预览回调参数
typedef struct
{
    EMPREVIEWMODE emPreviewMode; //预溃窗口分割模式
    u8 nModePara; //extend parameter for EMPREVIEWMODE
} SPreviewPara;

//预览初始化参数
typedef struct
{
    u8 nVideoNum;
    u8 nAudioNum;
    u8 nMaxStrOsdNum;
    u8 nMaxImgOsdNum;
    u8 nMaxRectOsdNum;
    FNPREVIEWCB pfnPreviewCb;
} SPreviewInitPara;

//电子放大预览参数
typedef struct{
	unsigned int s32X;
	unsigned int s32Y;
	unsigned int u32Width;
	unsigned int u32Height;
}SPreviewElecZoom;

//预览初始化
s32 ModPreviewInit(SPreviewInitPara* psPreviewInitPara);

//启动/停止预览
s32 ModPreviewStart(SPreviewPara* psPreviewPara);
s32 ModPreviewStop(void);
//预览通道放大
s32 ModPreviewZoomChn(u8 nChn);
//电子放大预览
s32 ModPreviewElecZoomChn(int flag, u8 nChn, SPreviewElecZoom* stCapRect);
s32 ModPreviewPbElecZoomChn(int flag, u8 nChn, SPreviewElecZoom* stCapRect);

//图片预览
s32 ModPreviewSnap(int flag, SPreviewElecZoom* stCapRect);

//恢复预览到上个状态(可用于预览通道放大或回放停止后)
s32 ModPreviewResume(u8 nForce); //参数说明: 0,不强制恢复,若预览还没开启，则不作任何处理
								 //			 非0,强制恢复,若预览还没开启，则强制开启预览后再恢复预览模式，主要用于回放结束后恢复预览
//预览下页
s32 ModPreviewNext(void);

//预览静音控制
s32 ModPreviewMute(u8 nEnable);
//预览音量设置
s32 ModPreviewVolume(u8 nVlm);
//预览音频输出参数设置
s32 ModPreviewSetAudioOut(u8 nChn);

//预览图像色彩设置/获取
s32 ModPreviewSetImage(u8 nChn, SPreviewImagePara* psImagePara);
s32 ModPlayBackSetImage(SVoImagePara* psVoImagePara);
//s32 ModPreviewGetImage(u8 nChn, PARAOUT SPreviewImagePara* psImagePara);

//预览轮巡参数设置
s32 ModPreviewSetPatrol(SPreviewPatrolPara* psPatrolPara);

//预览轮巡开始/结束
s32 ModPreviewStartPatrol(void);
s32 ModPreviewStopPatrol(void);

s32 ModPreviewVoip(u8 nEnable);

//预览矩形遮盖显示与隐藏
s32 ModPreviewRectOsdShow(u8 nChn, u8 nIndex, u8 nShow, SPRect pSRect);

//csp modify
s32 ModPreviewIsOpened(void);
s32 ModPreviewGetMode(void);

#ifdef __cplusplus
}
#endif

#endif

