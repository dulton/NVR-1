#ifndef _BIZ_PREVIEW_H_
#define _BIZ_PREVIEW_H_

#include "biz.h"
#include "mod_preview.h"
#include "biz_manager.h"

typedef struct
{
	u8 nVideoNum;
	u8 nAudioNum;
	u8 nMaxStrOsdNum;
	u8 nMaxImgOsdNum;
	u8 nMaxRectOsdNum;
} SBizPreview;

s32 PreviewInit(u8 nEnable, SBizPreview* psInitPara);
s32 PreviewStart(SBizPreviewPara *psBizPreviewPara);
s32 PreviewStop(void);
s32 PreviewZoomChn(u8 nChn);
s32 PreviewElecZoomChn(int flag, u8 nChn, SBizPreviewElecZoom* stCapRect);
//Õº∆¨‘§¿¿
s32 PreviewSnap(int flag,  SBizPreviewElecZoom* stMenuRect);

s32 PreviewPbElecZoomChn(int flag, u8 nChn, SBizPreviewElecZoom* stCapRect);
s32 PreviewResume(u8 nForce);
s32 PreviewNext(void);
s32 PreviewLast(void);
s32 PreviewNextMod(void);
s32 PreviewLastMod(void);
s32 PreviewMute(u8 nEnable);
s32 PreviewSetAudioOut(u8 nChn);
s32 PreviewVolume(u8 nVolume);
s32 PreviewStartPatrol(void);
s32 PreviewStopPatrol(void);

s32 PreviewSetPatrol(SPreviewPatrolPara* psPatrolPara);
s32 PreviewSetImage(u8 nChn, SPreviewImagePara* psImagePara);
s32 PlayBackSetImage(SVoImagePara* psVoImagePara);
s32 PreviewRectOsdShow(u8 nChn, u8 nIndex, u8 nShow, SPRect pSRect);

#endif

