#ifndef _BIZ_PLAYBACK_H_
#define _BIZ_PLAYBACK_H_

#include "biz.h"
#include "mod_playback.h"
#include "biz_manager.h"

typedef struct
{
	u8 nMaxChn;
	HDDHDR hDisk;
	u64 nChnMaskOfD1;
} SBizPlayback;

s32 PlaybackInit(u8 nEnable, SBizPlayback* psInitPara);

//yaogang modify 20150112
s32 SnapDisplay(SBizRecSnapInfo *pSnapInfo);

//开始回放
s32 PlaybackStart(SBizPbStart *psPbStart);
//回放控制 
s32 PlayBackControl(SBizPbControl* psBizPbCtl);

/*回放时单通道放大/恢复 (说明:nKey<0,自动依次放大下一个通道; 
							  nKey>=最大通道数,恢复多通道回放;
							  nKey其它,放大第nKey个窗口)*/
s32 PlayBackZoom(s32 nKey);

s32 PlayBackGetRealPlayChn(u64* pChnMask);

s32 PlayBackSetModeVOIP(u8 nVoip);

s32 PlayBackGetVideoFormat(u8 nChn);

//注册回调函数,参数type: 0,关闭电子放大
s32 PlayBackRegistFunCB(u8 type, PBREGFUNCCB func); 

#endif

