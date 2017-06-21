#include "biz_preview.h"
#include "mod_preview.h"
#include "public.h"

EMPREVIEWMODE PreviewModeBizToMod(EMBIZPREVIEWMODE emBizPreviewMode);
void PreviewEventDeal(SPreviewCbData* psPreviewPara);

s32 PreviewInit(u8 nEnable, SBizPreview* psInitPara)
{
    if (nEnable && psInitPara)
   {
        SPreviewInitPara sPreviewInitPara;
        
        sPreviewInitPara.nVideoNum = psInitPara->nVideoNum;
        sPreviewInitPara.nAudioNum = psInitPara->nAudioNum;
        sPreviewInitPara.nMaxImgOsdNum = psInitPara->nMaxImgOsdNum;
        sPreviewInitPara.nMaxStrOsdNum = psInitPara->nMaxStrOsdNum;
		sPreviewInitPara.nMaxRectOsdNum = psInitPara->nMaxRectOsdNum;
        sPreviewInitPara.pfnPreviewCb = PreviewEventDeal;
        if (0 != ModPreviewInit(&sPreviewInitPara))
        {
            return -1;
        }
        // set
    }

    return 0;
}

void PreviewEventDeal(SPreviewCbData* psPreviewPara)
{
    if (psPreviewPara)
    {
        SBizEventPara sBizEventPara;
		
        sBizEventPara.emType = EM_BIZ_EVENT_PREVIEW_REFRESH;
        sBizEventPara.sPreviewData.emBizPreviewMode = (EMBIZPREVIEWMODE)psPreviewPara->emPreviewMode;
        sBizEventPara.sPreviewData.nAdioChn = psPreviewPara->nAdioChn;
        sBizEventPara.sPreviewData.nModePara = psPreviewPara->nModePara;
        sBizEventPara.sPreviewData.nMute = psPreviewPara->nMute;
        sBizEventPara.sPreviewData.nVolume = psPreviewPara->nVolume;
        
        SendBizEvent(&sBizEventPara);
    }
}

EMPREVIEWMODE PreviewModeBizToMod(EMBIZPREVIEWMODE emBizPreviewMode)
{
    return (EMPREVIEWMODE)emBizPreviewMode;
}

s32 PreviewStart(SBizPreviewPara *psBizPreviewPara)
{
	s32 rtn = -1;
    if(psBizPreviewPara)
    {
    	//printf("PreviewStart - ModPreviewStart\n");
		
        SPreviewPara sPreviewPara;
		
        sPreviewPara.emPreviewMode = PreviewModeBizToMod(psBizPreviewPara->emBizPreviewMode);
        sPreviewPara.nModePara = psBizPreviewPara->nModePara;
        rtn = ModPreviewStart(&sPreviewPara);
    }
	
    return rtn;
}

s32 PreviewStop(void)
{
    return ModPreviewStop();
}

s32 PreviewZoomChn(u8 nChn)
{
    return ModPreviewZoomChn(nChn);
}

s32 PreviewElecZoomChn(int flag, u8 nChn, SBizPreviewElecZoom* stCapRect)
{
	return ModPreviewElecZoomChn(flag, nChn, stCapRect);
}

//Í¼Æ¬Ô¤ÀÀ
s32 PreviewSnap(int flag,  SBizPreviewElecZoom* stMenuRect)
{
	return ModPreviewSnap(flag, (SPreviewElecZoom *)stMenuRect);
}

s32 PreviewPbElecZoomChn(int flag, u8 nChn, SBizPreviewElecZoom* stCapRect)
{
	return ModPreviewPbElecZoomChn(flag, nChn, stCapRect);
}

s32 PreviewResume(u8 nForce)
{
    return ModPreviewResume(nForce);
}

s32 PreviewNext(void)
{
    return ModPreviewNext();
}

s32 PreviewLast(void)
{
	return ModPreviewLast();
}

s32 PreviewNextMod(void)
{
	return ModPreviewNextMod();
}

s32 PreviewLastMod(void)
{
	return ModPreviewLastMod();
}

s32 PreviewMute(u8 nEnable)
{
    return ModPreviewMute(nEnable);
}

s32 PreviewSetAudioOut(u8 nChn)
{
    return ModPreviewSetAudioOut(nChn);
}

s32 PreviewVolume(u8 nVolume)
{
    return ModPreviewVolume(nVolume);
}

s32 PreviewStartPatrol(void)
{
    return ModPreviewStartPatrol();
}

s32 PreviewStopPatrol(void)
{
    return ModPreviewStopPatrol();
}

s32 PreviewSetPatrol(SPreviewPatrolPara* psPatrolPara)
{
    return ModPreviewSetPatrol(psPatrolPara);
}

s32 PreviewGetPatrol(SPreviewPatrolPara* psPatrolPara)
{
    return 0;
}

s32 PreviewSetImage(u8 nChn, SPreviewImagePara* psImagePara)
{
#if 0
	int ret = 0;
	BOOL flag = FALSE;
	SBizDvrInfo  DvrInfo;
	ConfigGetDvrInfo(0, &DvrInfo);
	u16 tmp = psImagePara->nBrightness;
	
	if ((0 == strcasecmp(DvrInfo.sproductnumber, "R9504S"))
		|| (0 == strcasecmp(DvrInfo.sproductnumber, "R9508S")))
	{
		psImagePara->nBrightness >>= 1;
		flag = TRUE;
	}
	
    ret = ModPreviewSetImage(nChn, psImagePara);
	
	if(flag)
	{
		psImagePara->nBrightness = tmp;
	}
#endif
	
	return ModPreviewSetImage(nChn, psImagePara);
}

s32 PlayBackSetImage(SVoImagePara* psVoImagePara)
{
    return ModPlayBackSetImage(psVoImagePara);
}

s32 PreviewGetImage(u8 nChn, SPreviewImagePara* psImagePara)
{
    return 0;
}

s32 PreviewRectOsdShow(u8 nChn, u8 nIndex, u8 nShow, SPRect pSRect)
{
	return ModPreviewRectOsdShow(nChn, nIndex, nShow, pSRect);
}

