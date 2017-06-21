#include "mod_preview.h"

#include "hi_comm_vo.h"

#include "lib_common.h"
#include "lib_venc.h"
#include "lib_audio.h"
#include "lib_preview.h"
#include "lib_video_cover.h"

#include "lib_vdec.h"
#include "lib_video.h"

#include "public.h"

#define VOLUME_SET_DEFAULT 50
#define VOLUME_SET_MAX 100

typedef struct
{
    u8 nOutChn; //音频输出通道;0xff表示自动切换
    u8 nActiveChn; //当前实际输出通道
    u8 nOutVolume; //输出音量
    u8 nMute; //默认音频输出状态;0 非静音输出; 1 静音输出
    u8 nVoip; //语音对讲是否启用状态
} SPreviewAudioPara;

typedef struct
{
	int nFd;
	SRect sRect;
} SPreviewRectOsd;

typedef struct
{
	u8 nIsPreview;
	u8 nIsPatrolParaChange;
	u8 nIsModeChange;
	SPreviewInitPara sInitPara;
	SPreviewPara sCurPreview;
	SPreviewPara sLastPreview;
	SPreviewAudioPara sPreviewAudio;
	audio_volume_atr_t sVolume;
	
	SPreviewPatrolPara sPatrolPara;
	SPreviewRectOsd *pSRectOsd;
	
	pthread_mutex_t lock;
} SPreviewManager;

SPreviewManager g_sPreviewManager;

EMPREVIEWMODE GetDefaultMode(u8 nMaxChn);
void* ModeCtrlFxn(void* arg);

#if 0
void InitViColor()
{
	u32 kk = 0;
	u8 ss[3] = {0x02, 0x03, 0xA};
	
	u32 tw_num = 1; // 9504 = 1, 9508 = 2
	for(kk = 0; kk < tw_num; kk++)
	{
		tw286x_wr(kk, 0xaf, 0x33);
	 	tw286x_wr(kk, 0xb0, 0x33);
		tw286x_wr(kk, 0x83, 0xdc); 

		u32 i=0;
		for(i=0; i<4; i++)
		{
			tw286x_wr(kk, ss[0]+i*0x10, 0x68);
			tw286x_wr(kk, ss[1]+i*0x10, 0x15);
			tw286x_wr(kk, ss[2]+i*0x10, 0x0D);
		}
	}
}
#endif

#if 0
void InitVoColor()
{
	u32 ret;						
	VO_CSC_S stpubcscparm;

	
	stpubcscparm.enCSCType = VO_CSC_LUMA;
	stpubcscparm.u32Value = 40;
	ret = HI_MPI_VO_SetDevCSC(0, &stpubcscparm);
	if(0 != ret)
	{
		PUBPRT("Set VO_CSC_LUMA fail\n");
	}
			
	stpubcscparm.enCSCType = VO_CSC_SATU;     //stpubcscparm.enCSCType=VO_CSC_LUMA;
	stpubcscparm.u32Value = 54;//58;		
	ret = HI_MPI_VO_SetDevCSC(0, &stpubcscparm);
	if(0 != ret)
	{
		PUBPRT("Set VO_CSC_LUMA fail\n");
	}
		
	stpubcscparm.enCSCType = VO_CSC_CONTR;
	stpubcscparm.u32Value = 49;//30;			
	ret = HI_MPI_VO_SetDevCSC(0, &stpubcscparm);
	if(0 != ret)
	{
		PUBPRT("Set VO_CSC_CONTR fail\n");
	}
	
	stpubcscparm.enCSCType = VO_CSC_HUE;
	stpubcscparm.u32Value = 50;
	ret = HI_MPI_VO_SetDevCSC(0, &stpubcscparm);
	if(0 != ret)
	{
		PUBPRT("Set VO_CSC_HUE fail\n");
	}
}
#endif

s32 ModPreviewInit(SPreviewInitPara* psPreviewInit)
{
    s32 i;
    pthread_t ret;
	
    if(NULL == psPreviewInit)
    {
        return -1;
    }
	
	//InitViColor();
	//InitVoColor();
	
    memset(&g_sPreviewManager, 0, sizeof(SPreviewManager));
	
    pthread_mutex_init(&g_sPreviewManager.lock, NULL);
    
    g_sPreviewManager.sInitPara = *psPreviewInit;
	
    g_sPreviewManager.nIsPreview = 0;
    g_sPreviewManager.nIsModeChange = 0;
    g_sPreviewManager.sCurPreview.nModePara = -1; //为0时刚启动时testUI里会传一个0过来,桌面不会画线
    g_sPreviewManager.sCurPreview.emPreviewMode = GetDefaultMode(psPreviewInit->nVideoNum);
    g_sPreviewManager.sLastPreview = g_sPreviewManager.sCurPreview;

    g_sPreviewManager.nIsPatrolParaChange = 1;
    g_sPreviewManager.sPatrolPara.emPreviewMode = EM_PREVIEW_1SPLIT;
    g_sPreviewManager.sPatrolPara.nInterval = 5;
    g_sPreviewManager.sPatrolPara.nIsPatrol = 0;
    g_sPreviewManager.sPatrolPara.nStops = psPreviewInit->nVideoNum;
    //g_sPreviewManager.sPatrolPara.pnStopModePara = malloc(psPreviewInit->nVideoNum);
    if(NULL == g_sPreviewManager.sPatrolPara.pnStopModePara)
    {
        return -1;
    }
    
    for(i = 0; i < psPreviewInit->nVideoNum; i++)
    {
        g_sPreviewManager.sPatrolPara.pnStopModePara[i] = i;
    }
	
	g_sPreviewManager.pSRectOsd = malloc(sizeof(SPreviewRectOsd) * psPreviewInit->nMaxRectOsdNum);
	memset(g_sPreviewManager.pSRectOsd, 0, sizeof(SPreviewRectOsd) * psPreviewInit->nMaxRectOsdNum);
	for(i = 0; i < psPreviewInit->nMaxRectOsdNum; i++)
	{
		g_sPreviewManager.pSRectOsd[i].nFd = -1;
	}
	
    g_sPreviewManager.sPreviewAudio.nMute = 0;
    g_sPreviewManager.sPreviewAudio.nOutChn = 0xff;
    g_sPreviewManager.sPreviewAudio.nActiveChn = 0;
    g_sPreviewManager.sPreviewAudio.nOutVolume = 0;
	
    //PUBPRT("before tl_audio_get_volume_atr");
    tl_audio_get_volume_atr(&g_sPreviewManager.sVolume);
    int max_volume = g_sPreviewManager.sVolume.max_val;
    int min_volume = g_sPreviewManager.sVolume.min_val;
    
    g_sPreviewManager.sPreviewAudio.nOutVolume = VOLUME_SET_DEFAULT;
    //PUBPRT("before tl_audio_set_volume");
    tl_audio_set_volume(min_volume + VOLUME_SET_DEFAULT * (max_volume - min_volume) / VOLUME_SET_MAX);
	//PUBPRT("after tl_audio_set_volume");
    //ModPreviewStart(&g_sPreviewManager.sCurPreview);
	
    //PUBPRT("before pthread_create PatrolFxn");
    pthread_create(&ret, NULL, ModeCtrlFxn, NULL);
	
    return 0;
}

s32 ModPreviewStart(SPreviewPara* psPreviewPara)
{
	EMPREVIEWMODE emMode;
	u32 nFirstChn;
	
	//printf("ModPreviewStart-1 mode=%d......\n",psPreviewPara->emPreviewMode);
    
	if(NULL == psPreviewPara)
	{
		return -1;
	}
	
#if 0
	while(g_sPreviewManager.nIsModeChange != 0)//cw_preview
	{
		printf("dead loop???\n");
		usleep(1000);
	}
#else
    if(g_sPreviewManager.nIsModeChange != 0)
	{
		return -1;
    }
#endif
	
	//emMode = psPreviewPara->emPreviewMode;
	//nFirstChn = psPreviewPara->nModePara;
	
	//printf("g_sPreviewManager.sCurPreview.emPreviewMode = %d,g_sPreviewManager.sLastPreview.emPreviewMode = %d\n",g_sPreviewManager.sCurPreview.emPreviewMode,g_sPreviewManager.sLastPreview.emPreviewMode);
	
	pthread_mutex_lock(&g_sPreviewManager.lock);
	
	if(g_sPreviewManager.nIsPreview &&
			(psPreviewPara->emPreviewMode==g_sPreviewManager.sCurPreview.emPreviewMode &&
			psPreviewPara->nModePara==g_sPreviewManager.sCurPreview.nModePara))
	{
		pthread_mutex_unlock(&g_sPreviewManager.lock);
		
		return 0;//-1;
	}
	
	//printf("ModPreviewStart-2 mode=%d......\n",psPreviewPara->emPreviewMode);
	
    g_sPreviewManager.nIsPreview = 1;
	
	if(EM_PREVIEW_1SPLIT != g_sPreviewManager.sCurPreview.emPreviewMode)
	{
		g_sPreviewManager.sLastPreview = g_sPreviewManager.sCurPreview;
	}
	
	//printf("ModPreviewStart-3 mode=%d......\n",psPreviewPara->emPreviewMode);
	
	emMode = psPreviewPara->emPreviewMode;
    nFirstChn = psPreviewPara->nModePara;
    memcpy(&g_sPreviewManager.sCurPreview, psPreviewPara, sizeof(SPreviewPara));

//printf("yg emMode: %d, nFirstChn: %d\n", emMode, nFirstChn);
    switch(emMode)
    {
    	case EM_PREVIEW_1SPLIT:
			
    		tl_preview(PREVIEW_1D1, nFirstChn);
    		break;
     	case EM_PREVIEW_4SPLITS:
    		tl_preview(PREVIEW_4CIF, nFirstChn);
    		break;
     	case EM_PREVIEW_8SPLITS:
    		tl_preview(PREVIEW_8PIC, nFirstChn);
    		break;
     	case EM_PREVIEW_9SPLITS:
			//printf("tl_preview 9CIF-1,FirstChn=%d\n",nFirstChn);
			//if(nFirstChn > 0)
			//{
			//	nFirstChn = (nFirstChn+9-1)/9*9;
			//}
			//printf("tl_preview 9CIF-2,FirstChn=%d\n",nFirstChn);
    		tl_preview(PREVIEW_9CIF, nFirstChn);
    		break;
    	case EM_PREVIEW_10SPLITS:
    		tl_preview(PREVIEW_10PIC, nFirstChn);
    		break;
		case EM_PREVIEW_6SPLITS:
    		tl_preview(PREVIEW_6PIC_1, nFirstChn);
    		break;
    	case EM_PREVIEW_12SPLITS:
    		break;
     	case EM_PREVIEW_13SPLITS:
    		break;
     	case EM_PREVIEW_16SPLITS:
    	    tl_preview(PREVIEW_16CIF, nFirstChn);
    		break;
     	case EM_PREVIEW_24SPLITS:
			//for nvr
			//tl_preview(PREVIEW_24CIF, nFirstChn);
			//printf("24SPLITS->25SPLITS\n");
			tl_preview(PREVIEW_25CIF, nFirstChn);
    		break;
		case EM_PREVIEW_25SPLITS:
			//printf("25SPLITS->25SPLITS\n");
			tl_preview(PREVIEW_25CIF, nFirstChn);
    		break;
     	case EM_PREVIEW_32SPLITS:
			//for nvr
			tl_preview(PREVIEW_36CIF, nFirstChn);
    		break;
     	case EM_PREVIEW_36SPLITS:
			tl_preview(PREVIEW_36CIF, nFirstChn);
    		break;
     	case EM_PREVIEW_64SPLITS:
    		break;
     	case EM_PREVIEW_PIP:
    		tl_preview(PREVIEW_PIP, nFirstChn);
    		break;
    	default:
    		break;
    }
	
#if 1
	if((0 == g_sPreviewManager.sPreviewAudio.nMute) && (0 == g_sPreviewManager.sPreviewAudio.nVoip) && (g_sPreviewManager.sPreviewAudio.nOutVolume))
	{
		if(0xff == g_sPreviewManager.sPreviewAudio.nOutChn)
		{
			if(nFirstChn < g_sPreviewManager.sInitPara.nAudioNum)
			{
				tl_audio_out_sel(nFirstChn);
				g_sPreviewManager.sPreviewAudio.nActiveChn = nFirstChn;
			}
			else//9624
			{
				tl_audio_out_sel(TL_AUDIO_CH_PLAY_BACK);
			}
		}
		else if(g_sPreviewManager.sPreviewAudio.nOutChn < g_sPreviewManager.sInitPara.nAudioNum)
		{
			tl_audio_out_sel(g_sPreviewManager.sPreviewAudio.nOutChn);
			g_sPreviewManager.sPreviewAudio.nActiveChn = g_sPreviewManager.sPreviewAudio.nOutChn;
		}
	}
	else//yzw add
	{
		tl_audio_out_sel(TL_AUDIO_CH_PLAY_BACK);
	}
#endif
	
    g_sPreviewManager.nIsModeChange = 1;
    
    pthread_mutex_unlock(&g_sPreviewManager.lock);
	
    return 0;
}

s32 ModPreviewStop(void)
{
    pthread_mutex_lock(&g_sPreviewManager.lock);
	
    g_sPreviewManager.nIsPreview = 0;
    
    tl_preview(PREVIEW_CLOSE, 0);
	
    tl_audio_out_sel(TL_AUDIO_CH_PLAY_BACK);
	
    g_sPreviewManager.nIsModeChange = 1;
    
    pthread_mutex_unlock(&g_sPreviewManager.lock);
    
    return 0;
}

//预览通道放大
s32 ModPreviewZoomChn(u8 nChn)
{
    if(0 == g_sPreviewManager.nIsPreview)
	{
		return  0;
	}
	
    SPreviewPara sPreviewPara;
    
    sPreviewPara.emPreviewMode = EM_PREVIEW_1SPLIT;
    sPreviewPara.nModePara = nChn;
	
	//printf("ModPreviewZoomChn - ModPreviewStart\n");
	
    return ModPreviewStart(&sPreviewPara);
}

s32 ModPreviewElecZoomChn(int flag, u8 nChn, SPreviewElecZoom* stCapRect)
{
	if(!flag)
	{
  	  	tl_zoom_windows(PREVIEW_EA_OPEN,nChn,*((pre_rect_s*)stCapRect));
	}
	else
	{
		tl_zoom_windows(PREVIEW_EA_CLOSE,nChn,*((pre_rect_s*)stCapRect));
	}
	
	return 0;//csp modify
}

s32 ModPreviewSnap(int flag, SPreviewElecZoom* stMenuRect)
{
	if(!flag)
	{
  	  	tl_snap_preview_init(0, (pre_rect_s*)stMenuRect);//open
	}
	else
	{
		tl_snap_preview_init(1, (pre_rect_s*)stMenuRect);//close
	}
	
	return 0;//csp modify
}


s32 ModPreviewPbElecZoomChn(int flag, u8 nChn, SPreviewElecZoom* stCapRect)
{
	if(!flag)
	{
		return tl_vdec_eletroinc_zoom(PREVIEW_EA_OPEN,nChn,*((pre_rect_s*)stCapRect));
	}
	else
	{
		return tl_vdec_eletroinc_zoom(PREVIEW_EA_CLOSE,nChn,*((pre_rect_s*)stCapRect));
	}
}

// 1 预览通道放大或停止后继续多画面预览
s32 ModPreviewResume(u8 nForce)
{
	if(0 == nForce)
	{
		if(0 == g_sPreviewManager.nIsPreview)
		{
			return 0;
		}
	}
	
	//printf("ModPreviewResume - ModPreviewStart\n");
	
	ModPreviewStart(&g_sPreviewManager.sLastPreview);
	
	return 0;//csp modify
}

s32 ModPreviewNext(void)
{
    if(0 == g_sPreviewManager.nIsPreview)
	{
		return 0;
	}
	
    SPreviewPara sPreviewPara;
    u8 nSplitNum = 1;
	
    sPreviewPara = g_sPreviewManager.sCurPreview;
    switch(sPreviewPara.emPreviewMode)
    {
    	case EM_PREVIEW_1SPLIT:
    		nSplitNum = 1;
    		break;
     	case EM_PREVIEW_4SPLITS:
    		nSplitNum = 4;
    		break;
     	case EM_PREVIEW_8SPLITS:
    		nSplitNum = 8;
    		break;
     	case EM_PREVIEW_9SPLITS:
    		nSplitNum = 9;
    		break;
		//csp modify 20130504
		case EM_PREVIEW_10SPLITS:
    		nSplitNum = 10;
    		break;
		case EM_PREVIEW_6SPLITS:
    		nSplitNum = 6;
    		break;
    	case EM_PREVIEW_12SPLITS:
            nSplitNum = 12;
    		break;
     	case EM_PREVIEW_13SPLITS:
            nSplitNum = 13;
    		break;
     	case EM_PREVIEW_16SPLITS:
            nSplitNum = 16;
    		break;
     	case EM_PREVIEW_24SPLITS:
			//for nvr
            //nSplitNum = 0;
			nSplitNum = 24;
    		break;
		case EM_PREVIEW_25SPLITS:
			nSplitNum = 25;
    		break;
     	case EM_PREVIEW_32SPLITS:
            nSplitNum = 32;
    		break;
     	case EM_PREVIEW_36SPLITS:
			//for nvr
            //nSplitNum = 0;
            nSplitNum = 36;
    		break;
     	case EM_PREVIEW_64SPLITS:
            nSplitNum = 64;
            break;
        default:
            return -1;
    }
    sPreviewPara.nModePara += nSplitNum;
					
	if((sPreviewPara.nModePara + nSplitNum) > g_sPreviewManager.sInitPara.nVideoNum)
	{
		if(g_sPreviewManager.sInitPara.nVideoNum != sPreviewPara.nModePara)
		{
			if(g_sPreviewManager.sInitPara.nVideoNum > nSplitNum)//cw_9508S
				sPreviewPara.nModePara = g_sPreviewManager.sInitPara.nVideoNum - nSplitNum;
			else
				sPreviewPara.nModePara = 0;
		}
	}
	
	sPreviewPara.nModePara %= g_sPreviewManager.sInitPara.nVideoNum;
	//printf("ModPreviewNext : (%d,%d)\n",sPreviewPara.emPreviewMode,sPreviewPara.nModePara);
	
	//printf("ModPreviewNext - ModPreviewStart\n");

	//csp modify 20150110
	//printf("yg sPreviewPara.nModePara: %d\n", sPreviewPara.nModePara);
	//printf("yg nSplitNum: %d\n", nSplitNum);
	
    return ModPreviewStart(&sPreviewPara);
}

s32 ModPatrolNext(void)
{
	//printf("yg 1\n");
	if(0 == g_sPreviewManager.nIsPreview)
	{
		return 0;
	}
	
	SPreviewPara sPreviewPara;
	SPreviewPatrolPara sPreviewPatrol;
	u8 nSplitNum = 1;
	//printf("yg 2\n");

	pthread_mutex_lock(&g_sPreviewManager.lock);
	sPreviewPara = g_sPreviewManager.sCurPreview;
	sPreviewPatrol = g_sPreviewManager.sPatrolPara;
	pthread_mutex_unlock(&g_sPreviewManager.lock);

	 if(0 == sPreviewPatrol.nIsPatrol)
	{
		return 0;
	}
	//printf("yg 3\n");

	sPreviewPara.emPreviewMode = sPreviewPatrol.emPreviewMode;
	
    switch(sPreviewPara.emPreviewMode)
    {
    	case EM_PREVIEW_1SPLIT:
    		nSplitNum = 1;
    		break;
     	case EM_PREVIEW_4SPLITS:
    		nSplitNum = 4;
    		break;
     	case EM_PREVIEW_8SPLITS:
    		nSplitNum = 8;
    		break;
     	case EM_PREVIEW_9SPLITS:
    		nSplitNum = 9;
    		break;
		//csp modify 20130504
		case EM_PREVIEW_10SPLITS:
    		nSplitNum = 10;
    		break;
		case EM_PREVIEW_6SPLITS:
    		nSplitNum = 6;
    		break;
    	case EM_PREVIEW_12SPLITS:
            nSplitNum = 12;
    		break;
     	case EM_PREVIEW_13SPLITS:
            nSplitNum = 13;
    		break;
     	case EM_PREVIEW_16SPLITS:
            nSplitNum = 16;
    		break;
     	case EM_PREVIEW_24SPLITS:
			//for nvr
            //nSplitNum = 0;
			nSplitNum = 24;
    		break;
		case EM_PREVIEW_25SPLITS:
			nSplitNum = 25;
    		break;
     	case EM_PREVIEW_32SPLITS:
            nSplitNum = 32;
    		break;
     	case EM_PREVIEW_36SPLITS:
			//for nvr
            //nSplitNum = 0;
            nSplitNum = 36;
    		break;
     	case EM_PREVIEW_64SPLITS:
            nSplitNum = 64;
            break;
        default:
            return -1;
    }
				
    sPreviewPara.nModePara += nSplitNum;
					
	if((sPreviewPara.nModePara + nSplitNum) > g_sPreviewManager.sInitPara.nVideoNum)
	{
		if(g_sPreviewManager.sInitPara.nVideoNum != sPreviewPara.nModePara)
		{
			if(g_sPreviewManager.sInitPara.nVideoNum > nSplitNum)//cw_9508S
				sPreviewPara.nModePara = g_sPreviewManager.sInitPara.nVideoNum - nSplitNum;
			else
				sPreviewPara.nModePara = 0;
		}
	}
	
	sPreviewPara.nModePara %= g_sPreviewManager.sInitPara.nVideoNum;
	//printf("ModPreviewNext : (%d,%d)\n",sPreviewPara.emPreviewMode,sPreviewPara.nModePara);
	
	//printf("ModPreviewNext - ModPreviewStart\n");
	
	//csp modify 20150110
	//printf("yg sPreviewPara.nModePara: %d\n", sPreviewPara.nModePara);
	//printf("yg nSplitNum: %d\n", nSplitNum);
	
    return ModPreviewStart(&sPreviewPara);
}


s32 ModPreviewLast(void)
{
	if(0 == g_sPreviewManager.nIsPreview)
	{
		return 0;
	}
	
    SPreviewPara sPreviewPara;
    u8 nSplitNum = 1;
	
    sPreviewPara = g_sPreviewManager.sCurPreview;
    switch(sPreviewPara.emPreviewMode)
    {
    	case EM_PREVIEW_1SPLIT:
    		nSplitNum = 1;
    		break;
     	case EM_PREVIEW_4SPLITS:
    		nSplitNum = 4;
    		break;
     	case EM_PREVIEW_8SPLITS:
    		nSplitNum = 8;
    		break;
     	case EM_PREVIEW_9SPLITS:
    		nSplitNum = 9;
    		break;
		//csp modify 20130504
		case EM_PREVIEW_10SPLITS:
    		nSplitNum = 10;
    		break;
		case EM_PREVIEW_6SPLITS:
    		nSplitNum = 6;
    		break;
    	case EM_PREVIEW_12SPLITS:
           	nSplitNum = 12;
    		break;
     	case EM_PREVIEW_13SPLITS:
            nSplitNum = 13;
    		break;
     	case EM_PREVIEW_16SPLITS:
            nSplitNum = 16;
    		break;
     	case EM_PREVIEW_24SPLITS:
			//for nvr
            //nSplitNum = 0;
			nSplitNum = 24;
    		break;
		case EM_PREVIEW_25SPLITS:
            nSplitNum = 25;
    		break;
     	case EM_PREVIEW_32SPLITS:
            nSplitNum = 32;
    		break;
     	case EM_PREVIEW_36SPLITS:
			//for nvr
            //nSplitNum = 0;
			nSplitNum = 36;
    		break;
     	case EM_PREVIEW_64SPLITS:
            nSplitNum = 64;
            break;
        default:
            return -1;
    }
    #if 1
    if(sPreviewPara.nModePara <= nSplitNum)
    {
    	if(0 == sPreviewPara.nModePara)
    	{
    		if(g_sPreviewManager.sInitPara.nVideoNum >= nSplitNum)
	    		sPreviewPara.nModePara = g_sPreviewManager.sInitPara.nVideoNum - nSplitNum;
		else
			sPreviewPara.nModePara = 0;
    	}
    	else
    	{
    		sPreviewPara.nModePara = 0;
    	}
    }
    else
    {
    	if(sPreviewPara.nModePara + nSplitNum >= g_sPreviewManager.sInitPara.nVideoNum)
    	{
    		u8 i = 0;
    		for(i = 1; ; i++)
    		{
    			if(i*nSplitNum >= g_sPreviewManager.sInitPara.nVideoNum)
    			{
    				break;
    			}
    		}
    		sPreviewPara.nModePara = (i-2)*nSplitNum;
    	}
    	else
    	{
    		sPreviewPara.nModePara -= nSplitNum;
    	}
    }
    #else
 	if ( sPreviewPara.nModePara <= nSplitNum )  //cw_panel
		sPreviewPara.nModePara = 0;
	else
		sPreviewPara.nModePara -= nSplitNum;
	//9624
	if((sPreviewPara.nModePara + nSplitNum) > g_sPreviewManager.sInitPara.nVideoNum)
	{
		if(g_sPreviewManager.sInitPara.nVideoNum != sPreviewPara.nModePara )
		{
			sPreviewPara.nModePara = g_sPreviewManager.sInitPara.nVideoNum - nSplitNum;
		}
	}
    sPreviewPara.nModePara %= g_sPreviewManager.sInitPara.nVideoNum;
    #endif
	
	//printf("ModPreviewLast - ModPreviewStart\n");
	
    return ModPreviewStart(&sPreviewPara);
}

//预览下个模式
s32 ModPreviewNextMod(void)
{
	if(0 == g_sPreviewManager.nIsPreview)
	{
		return 0;
	}
	
	SPreviewPara sPreviewPara;
	EMPREVIEWMODE emMod;
	
	sPreviewPara = g_sPreviewManager.sCurPreview;
	
  #if 1
  switch(sPreviewPara.emPreviewMode)
  {
  	case EM_PREVIEW_1SPLIT:
  		emMod = EM_PREVIEW_4SPLITS;
  		break;
   	case EM_PREVIEW_4SPLITS://csp modify
		if(g_sPreviewManager.sInitPara.nVideoNum == 10)
		{
			emMod = EM_PREVIEW_10SPLITS;
		}
		else if(g_sPreviewManager.sInitPara.nVideoNum == 6)
		{
			emMod = EM_PREVIEW_6SPLITS;
		}
		else
		{
  			emMod = EM_PREVIEW_9SPLITS;
  		}
		break;
   	case EM_PREVIEW_9SPLITS:
  		emMod = EM_PREVIEW_16SPLITS;
  		break;
   	case EM_PREVIEW_16SPLITS:
          emMod = EM_PREVIEW_25SPLITS; 
  		break;
	case EM_PREVIEW_24SPLITS:
          emMod = EM_PREVIEW_36SPLITS;
  		break;
   	case EM_PREVIEW_25SPLITS:
          emMod = EM_PREVIEW_36SPLITS;
  		break;
   	case EM_PREVIEW_36SPLITS:
          emMod = EM_PREVIEW_1SPLIT;
  		break;
	//csp modify 20130501
	case EM_PREVIEW_PIP:
		emMod = EM_PREVIEW_4SPLITS;
		break;
	//csp modify 20130504
	case EM_PREVIEW_10SPLITS:
		emMod = EM_PREVIEW_1SPLIT;
		break;
	case EM_PREVIEW_6SPLITS:
		emMod = EM_PREVIEW_1SPLIT;
		break;
      default:
          return -1;
  }
#else
  switch(sPreviewPara.emPreviewMode)
  {
  	case EM_PREVIEW_1SPLIT:
  		emMod = EM_PREVIEW_4SPLITS;
  		break;
   	case EM_PREVIEW_4SPLITS:
  		emMod = EM_PREVIEW_6SPLITS;
  		break;
   	case EM_PREVIEW_6SPLITS: 
      emMod = EM_PREVIEW_8SPLITS;
  		break;
   	case EM_PREVIEW_8SPLITS:
  		emMod = EM_PREVIEW_9SPLITS;
  		break;
   	case EM_PREVIEW_9SPLITS:
  		emMod = EM_PREVIEW_12SPLITS;
  		break;
  	case EM_PREVIEW_12SPLITS: 
          emMod = EM_PREVIEW_13SPLITS;
  		break;
   	case EM_PREVIEW_13SPLITS:
          emMod = EM_PREVIEW_16SPLITS;
  		break;
   	case EM_PREVIEW_16SPLITS:
          emMod = EM_PREVIEW_24SPLITS; 
  		break;
   	case EM_PREVIEW_24SPLITS:
          emMod = EM_PREVIEW_32SPLITS;
  		break;
   	case EM_PREVIEW_32SPLITS:
          emMod = EM_PREVIEW_36SPLITS;
  		break;
   	case EM_PREVIEW_36SPLITS:
          emMod = EM_PREVIEW_64SPLITS;
  		break;
   	case EM_PREVIEW_64SPLITS:
          emMod = EM_PREVIEW_1SPLIT;
          break;
      default:
          return -1;;
  }
#endif
	if(emMod > g_sPreviewManager.sInitPara.nVideoNum + 4)//yzw
	//if(emMod > g_sPreviewManager.sInitPara.nVideoNum)
	{
		emMod = EM_PREVIEW_1SPLIT;
	}
	sPreviewPara.nModePara = 0;
	sPreviewPara.emPreviewMode = emMod;
	
	//printf("ModPreviewNextMod - ModPreviewStart\n");
	
	return ModPreviewStart(&sPreviewPara);
}

//预览上个模式
s32 ModPreviewLastMod(void)
{
	if(0 == g_sPreviewManager.nIsPreview)
	{
		return 0;
	}
	
	SPreviewPara sPreviewPara;
	EMPREVIEWMODE emMod;
	
	sPreviewPara = g_sPreviewManager.sCurPreview;
	
  #if 1
  switch(sPreviewPara.emPreviewMode)
  {
  	case EM_PREVIEW_1SPLIT:
  		emMod = GetDefaultMode(g_sPreviewManager.sInitPara.nVideoNum);
  		break;
   	case EM_PREVIEW_4SPLITS:
  		emMod = EM_PREVIEW_1SPLIT;
  		break;
   	case EM_PREVIEW_9SPLITS:
  		emMod = EM_PREVIEW_4SPLITS;
  		break;
   	case EM_PREVIEW_16SPLITS:
          emMod = EM_PREVIEW_9SPLITS; 
  		break;
   	case EM_PREVIEW_24SPLITS:
          emMod = EM_PREVIEW_16SPLITS;
  		break;
	case EM_PREVIEW_25SPLITS:
          emMod = EM_PREVIEW_16SPLITS;
  		break;
	case EM_PREVIEW_36SPLITS:
          emMod = EM_PREVIEW_25SPLITS;
  		break;
	//csp modify 20130501
	case EM_PREVIEW_PIP:
		emMod = EM_PREVIEW_1SPLIT;
		break;
	//csp modify 20130504
	case EM_PREVIEW_10SPLITS:
		emMod = EM_PREVIEW_4SPLITS;
		break;
	case EM_PREVIEW_6SPLITS:
		emMod = EM_PREVIEW_4SPLITS;
		break;
      default:
          return -1;
  }
  #else
  switch(sPreviewPara.emPreviewMode)
  {
  	case EM_PREVIEW_1SPLIT:
  		emMod = GetDefaultMode(g_sPreviewManager.sInitPara.nVideoNum);
  		break;
   	case EM_PREVIEW_4SPLITS:
  		emMod = EM_PREVIEW_1SPLIT;
  		break;
   	case EM_PREVIEW_6SPLITS: 
      emMod = EM_PREVIEW_4SPLITS;
  		break;
   	case EM_PREVIEW_8SPLITS:
  		emMod = EM_PREVIEW_6SPLITS;
  		break;
   	case EM_PREVIEW_9SPLITS:
  		emMod = EM_PREVIEW_8SPLITS;
  		break;
  	case EM_PREVIEW_12SPLITS: 
          emMod = EM_PREVIEW_9SPLITS;
  		break;
   	case EM_PREVIEW_13SPLITS:
          emMod = EM_PREVIEW_12SPLITS;
  		break;
   	case EM_PREVIEW_16SPLITS:
          emMod = EM_PREVIEW_13SPLITS; 
  		break;
   	case EM_PREVIEW_24SPLITS:
          emMod = EM_PREVIEW_16SPLITS;
  		break;
   	case EM_PREVIEW_32SPLITS:
          emMod = EM_PREVIEW_24SPLITS;
  		break;
   	case EM_PREVIEW_36SPLITS:
          emMod = EM_PREVIEW_32SPLITS;
  		break;
   	case EM_PREVIEW_64SPLITS:
          emMod = EM_PREVIEW_36SPLITS;
          break;
      default:
          return -1;;
  }
 #endif
  sPreviewPara.nModePara = 0;
  sPreviewPara.emPreviewMode = emMod;

  //printf("ModPreviewLastMod - ModPreviewStart\n");

  return ModPreviewStart(&sPreviewPara);
}

EMPREVIEWMODE GetDefaultMode(u8 nMaxChn)
{
	#if 1
	switch(nMaxChn)
    {
		case 1:
			return EM_PREVIEW_1SPLIT;
        case 4:
        	return EM_PREVIEW_4SPLITS;
		case 8:
        case 9:
            return EM_PREVIEW_9SPLITS;
        case 12:
        	return EM_PREVIEW_12SPLITS;
		//csp modify 20130504
		case 10:
        	return EM_PREVIEW_10SPLITS;
		case 6:
        	return EM_PREVIEW_6SPLITS;
        case 16:
        	return EM_PREVIEW_16SPLITS;
        case 24:
        case 25:
            return EM_PREVIEW_25SPLITS;
        case 32:
        case 36:
        	return EM_PREVIEW_36SPLITS; 		
        case 64:
        	return EM_PREVIEW_64SPLITS;
        default:
        	return EM_PREVIEW_4SPLITS;
    }
	#else
    switch(((nMaxChn + 3) >> 2))
    {
        case 1:
        	return EM_PREVIEW_4SPLITS;
        case 2:
              return EM_PREVIEW_9SPLITS;
        case 3:
        	return EM_PREVIEW_12SPLITS; 
        case 4: 
        	return EM_PREVIEW_16SPLITS;
        case 6:
            return EM_PREVIEW_25SPLITS;
        case 8:
        case 9: 
        	return EM_PREVIEW_36SPLITS; 		
        case 16:
        	return EM_PREVIEW_64SPLITS;
        default:
        	return EM_PREVIEW_4SPLITS;
    }
	#endif
}

s32 ModPreviewMute(u8 nEnable)
{
	pthread_mutex_lock(&g_sPreviewManager.lock);
	
	u8 nFirstChn = g_sPreviewManager.sCurPreview.nModePara;
	
	if(g_sPreviewManager.sPreviewAudio.nMute == nEnable)
	{
		pthread_mutex_unlock(&g_sPreviewManager.lock);
		return 0;
	}
	
	if((0 == nEnable) && (0 == g_sPreviewManager.sPreviewAudio.nVoip) && (g_sPreviewManager.sPreviewAudio.nOutVolume))
	{
		if(0xff == g_sPreviewManager.sPreviewAudio.nOutChn)
		{
			if(nFirstChn < g_sPreviewManager.sInitPara.nAudioNum)
			{
				tl_audio_out_sel(nFirstChn);
			}
		}
		else if(g_sPreviewManager.sPreviewAudio.nOutChn < g_sPreviewManager.sInitPara.nAudioNum)
		{
			tl_audio_out_sel(g_sPreviewManager.sPreviewAudio.nOutChn);
		}
	}
	else
	{
		tl_audio_out_sel(TL_AUDIO_CH_PLAY_BACK);
	}
	
	g_sPreviewManager.sPreviewAudio.nMute = nEnable;
	
	//g_sPreviewManager.nIsModeChange = 1;
	
	pthread_mutex_unlock(&g_sPreviewManager.lock);
	
	return 0;
}

s32 ModPreviewVoip(u8 nEnable)
{
    pthread_mutex_lock(&g_sPreviewManager.lock);
    
    u8 nFirstChn = g_sPreviewManager.sCurPreview.nModePara;
    
    if (g_sPreviewManager.sPreviewAudio.nVoip == nEnable)
    {
        pthread_mutex_unlock(&g_sPreviewManager.lock);
        return 0;
    }

    printf("nMute %d nVoip %d\n", g_sPreviewManager.sPreviewAudio.nMute, g_sPreviewManager.sPreviewAudio.nVoip);

    if ((g_sPreviewManager.nIsPreview)
		&& (0 == g_sPreviewManager.sPreviewAudio.nMute) 
		&& (g_sPreviewManager.sPreviewAudio.nOutVolume)
		&& (0 == nEnable))
    {
        if (0xff == g_sPreviewManager.sPreviewAudio.nOutChn)
        {
            if (nFirstChn < g_sPreviewManager.sInitPara.nAudioNum)
            {
                tl_audio_out_sel(nFirstChn);
            }
        }
        else if (g_sPreviewManager.sPreviewAudio.nOutChn <  g_sPreviewManager.sInitPara.nAudioNum)
        {
            tl_audio_out_sel(g_sPreviewManager.sPreviewAudio.nOutChn);
        }
        printf("nFirstChn %d nOutChn %d\n", nFirstChn, g_sPreviewManager.sPreviewAudio.nOutChn);
    }
    else
    {
        tl_audio_out_sel(TL_AUDIO_CH_PLAY_BACK);
    }
    
    g_sPreviewManager.sPreviewAudio.nVoip = nEnable;
	
    //g_sPreviewManager.nIsModeChange = 1;
	
    pthread_mutex_unlock(&g_sPreviewManager.lock);
	
	//printf("ModPreviewVoip:nEnable=%d\n",nEnable);
	//fflush(stdout);
	
    return 0;
}

s32 ModPreviewVolume(u8 nVlm)
{
    pthread_mutex_lock(&g_sPreviewManager.lock);
   	//PUBPRT("before lock");
    if((nVlm != g_sPreviewManager.sPreviewAudio.nOutVolume) || (0 == nVlm))
    {
        int max_volume = g_sPreviewManager.sVolume.max_val;
        int min_volume = g_sPreviewManager.sVolume.min_val;
        
        if(nVlm == 0)
       	{
       		tl_audio_out_sel(TL_AUDIO_CH_PLAY_BACK);//上面接口在音量设置为0时仍有较小音量输出
        }
        else
       	{
        	if((g_sPreviewManager.sPreviewAudio.nOutVolume == 0)
        		&& (0 == g_sPreviewManager.sPreviewAudio.nVoip))
      		{
      			u8 nFirstChn = g_sPreviewManager.sCurPreview.nModePara;
      			if(0xff == g_sPreviewManager.sPreviewAudio.nOutChn)
		        {
		            if(nFirstChn < g_sPreviewManager.sInitPara.nAudioNum)
		            {
		                tl_audio_out_sel(nFirstChn);
		            }
		        }
		        else if(g_sPreviewManager.sPreviewAudio.nOutChn <  g_sPreviewManager.sInitPara.nAudioNum)
		        {
		            tl_audio_out_sel(g_sPreviewManager.sPreviewAudio.nOutChn);
		        }
   			}
   			PUBPRT("before tl_audio_set_volume");
	        tl_audio_set_volume(min_volume + nVlm * (max_volume - min_volume) / VOLUME_SET_MAX);
	        PUBPRT("after tl_audio_set_volume");
       	}
        
        g_sPreviewManager.sPreviewAudio.nOutVolume = nVlm;
        //g_sPreviewManager.nIsModeChange = 1;
    }
    //PUBPRT("after lock");
    pthread_mutex_unlock(&g_sPreviewManager.lock);
    
    return 0;
}

s32 ModPreviewSetAudioOut(u8 nChn)
{
    pthread_mutex_lock(&g_sPreviewManager.lock);
    
    u8 nFirstChn = g_sPreviewManager.sCurPreview.nModePara;
    
    if(g_sPreviewManager.sPreviewAudio.nOutChn == nChn || (nChn < 0xff && nChn >=  g_sPreviewManager.sInitPara.nAudioNum))
    {
        pthread_mutex_unlock(&g_sPreviewManager.lock);
        return 0;
    }
	
    if ((0 == g_sPreviewManager.sPreviewAudio.nMute)
    	&& (0 == g_sPreviewManager.sPreviewAudio.nVoip))
    {
        if (0xff == nChn)
        {
            if (nFirstChn < g_sPreviewManager.sInitPara.nAudioNum)
            {
                tl_audio_out_sel(nFirstChn);
            }
        }
        else if (nChn < g_sPreviewManager.sInitPara.nAudioNum)
        {
            tl_audio_out_sel(nChn);
        }
    }
	
    g_sPreviewManager.sPreviewAudio.nOutChn = nChn;
	
    //g_sPreviewManager.nIsModeChange = 1;
	
    pthread_mutex_unlock(&g_sPreviewManager.lock);
	
	return 0;//csp modify
}

s32 ModPreviewSetImage(u8 nChn, SPreviewImagePara* psImagePara)
{
	int ret = 0;
	
	pthread_mutex_lock(&g_sPreviewManager.lock);
	
	ret = tl_video_set_img_para(nChn, (video_image_para_t *)psImagePara);
	//printf("ModPreviewSetImage:chn=%d,color=(%d,%d,%d,%d)\n",nChn,psImagePara->nBrightness,psImagePara->nContrast,psImagePara->nSaturation,psImagePara->nHue);
	
	pthread_mutex_unlock(&g_sPreviewManager.lock);
	
	return ret;
}

s32 ModPlayBackSetImage(SVoImagePara* psVoImagePara)
{
	pthread_mutex_lock(&g_sPreviewManager.lock);
	//printf("1cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
	tl_vo_set_img_para_all((vo_image_para_t *)psVoImagePara);
	//printf("2cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
	pthread_mutex_unlock(&g_sPreviewManager.lock);
	
	return 0;
}

s32 ModPreviewSetPatrol(SPreviewPatrolPara* psPatrolPara)
{
    pthread_mutex_lock(&g_sPreviewManager.lock);

    if (0 == memcmp(psPatrolPara, &g_sPreviewManager.sPatrolPara, sizeof(SPreviewPatrolPara)))
    {
        pthread_mutex_unlock(&g_sPreviewManager.lock);
        return -1;
    }
//yaogang modify 20141025
    g_sPreviewManager.sPatrolPara.emPreviewMode = psPatrolPara->emPreviewMode;

    g_sPreviewManager.sPatrolPara.nInterval = psPatrolPara->nInterval;
    g_sPreviewManager.sPatrolPara.nIsPatrol = psPatrolPara->nIsPatrol;
    //g_sPreviewManager.sPatrolPara.nStops = psPatrolPara->nStops;
    //if (g_sPreviewManager.sPatrolPara.nStops >= g_sPreviewManager.sInitPara.nVideoNum)
    //{
    //    g_sPreviewManager.sPatrolPara.nStops = g_sPreviewManager.sInitPara.nVideoNum;
    //}
    //memcpy(g_sPreviewManager.sPatrolPara.pnStopModePara, psPatrolPara->pnStopModePara, g_sPreviewManager.sPatrolPara.nStops);
    g_sPreviewManager.nIsPatrolParaChange = 1;
    
    pthread_mutex_unlock(&g_sPreviewManager.lock);
	
    return 0;
}

s32 ModPreviewStartPatrol(void)
{
    pthread_mutex_lock(&g_sPreviewManager.lock);

    g_sPreviewManager.sPatrolPara.nIsPatrol = 1;
    g_sPreviewManager.nIsPatrolParaChange = 1;
    
    pthread_mutex_unlock(&g_sPreviewManager.lock);
    
    return 0;
}

s32 ModPreviewStopPatrol(void)
{
    pthread_mutex_lock(&g_sPreviewManager.lock);

    g_sPreviewManager.sPatrolPara.nIsPatrol = 0;    
    g_sPreviewManager.nIsPatrolParaChange = 1;
    
    pthread_mutex_unlock(&g_sPreviewManager.lock);

    return 0;
}

#if 1
void* ModeCtrlFxn(void* arg)
{
	printf("$$$$$$$$$$$$$$$$$$ModeCtrlFxn id:%d\n",getpid());
	
	//s32 i, nCurStop = 0;
	s32 nCurStop = 0;
	SPreviewPatrolPara sPreviewPatrol;
	SPreviewPara sPreviewPara;
	u32 nLastTimeStamp = 0, nCurTimeStamp = 0;
	
	memset(&sPreviewPatrol, 0, sizeof(SPreviewPatrolPara));
	u8 flag = 0;
	
    while(1)
    {
    #if 1
    	if (g_sPreviewManager.nIsModeChange)
        {
        	pthread_mutex_lock(&g_sPreviewManager.lock);
        	
            if (g_sPreviewManager.sInitPara.pfnPreviewCb)
            {
                SPreviewCbData sPreviewPara;

                if (g_sPreviewManager.nIsPreview)
                {
                    sPreviewPara.emPreviewMode = g_sPreviewManager.sCurPreview.emPreviewMode;
				}
                else
                {
                    sPreviewPara.emPreviewMode = EM_PREVIEW_CLOSE;
                }
                sPreviewPara.nModePara = g_sPreviewManager.sCurPreview.nModePara;
                sPreviewPara.nAdioChn = g_sPreviewManager.sPreviewAudio.nActiveChn;
                sPreviewPara.nMute = g_sPreviewManager.sPreviewAudio.nMute;
                sPreviewPara.nVolume = g_sPreviewManager.sPreviewAudio.nOutVolume;
                pthread_mutex_unlock(&g_sPreviewManager.lock);

                g_sPreviewManager.sInitPara.pfnPreviewCb(&sPreviewPara);
				
				pthread_mutex_lock(&g_sPreviewManager.lock);
            }
            g_sPreviewManager.nIsModeChange = 0;
            pthread_mutex_unlock(&g_sPreviewManager.lock);
        }
       #endif
		
        #if 1
        if(g_sPreviewManager.nIsPreview)
        {
        	//printf("nIsPatrolParaChange:%d,nIsPatrol:%d,nInterval:%d,nStops:%d\n",
			//	g_sPreviewManager.nIsPatrolParaChange,
			//	sPreviewPatrol.nIsPatrol,
			//	sPreviewPatrol.nInterval,
			//	sPreviewPatrol.nStops
			//	);
			
			if (g_sPreviewManager.nIsPatrolParaChange)
			{
				pthread_mutex_lock(&g_sPreviewManager.lock);
				sPreviewPatrol = g_sPreviewManager.sPatrolPara;
				g_sPreviewManager.nIsPatrolParaChange = 0;
				pthread_mutex_unlock(&g_sPreviewManager.lock);


				nCurStop = 0;
				flag = 1;
			}
			
	        nCurTimeStamp = PublicGetTimeStamp();
	//printf("nCurTimeStamp=%u,nLastTimeStamp=%u,span=%u\n",nCurTimeStamp,nLastTimeStamp,nCurTimeStamp-nLastTimeStamp);
	#if 0
		if(sPreviewPatrol.nIsPatrol && sPreviewPatrol.nInterval > 0 && sPreviewPatrol.nStops > 1 && ((nCurTimeStamp - nLastTimeStamp) >= sPreviewPatrol.nInterval * 1000 || flag))
	        {
				flag = 0;
	            sPreviewPara.emPreviewMode = sPreviewPatrol.emPreviewMode;
	            sPreviewPara.nModePara = sPreviewPatrol.pnStopModePara[nCurStop];
	            if(0 != ModPreviewStart(&sPreviewPara))
				{
					//printf("ModPreviewStart - failed\n");
					continue;
	            }
				//printf("ModPreviewStart - succ\n");
			printf("yg sPreviewPara.emPreviewMode: %d\n", sPreviewPara.emPreviewMode);
			printf("yg sPreviewPara.nModePara: %d\n", sPreviewPara.nModePara);
				
	            nLastTimeStamp = nCurTimeStamp;

			printf("yg nCurStop: %d\n", nCurStop);
			printf("yg sPreviewPatrol.nStops: %d\n", sPreviewPatrol.nStops);
			printf("yg g_sPreviewManager.sInitPara.nVideoNum: %d\n", g_sPreviewManager.sInitPara.nVideoNum);
	            nCurStop++;
	            if(nCurStop >= sPreviewPatrol.nStops)
	            {
	                nCurStop = 0;
	            }
	        }
	#endif
	/*
		printf("nIsPatrol: %d, nIsPatrol: %d, nStops: %d, nInterval: %d\n", \
			sPreviewPatrol.nIsPatrol, \
			sPreviewPatrol.nInterval, \
			sPreviewPatrol.nStops, \
			sPreviewPatrol.nInterval * 1000);
		*/
		if(sPreviewPatrol.nIsPatrol && sPreviewPatrol.nInterval > 0 && sPreviewPatrol.nStops > 1 && ((nCurTimeStamp - nLastTimeStamp) >= sPreviewPatrol.nInterval * 1000 || flag))
	        {
			if (flag == 1)//第一屏
			{
				flag = 0;
				sPreviewPara.emPreviewMode = sPreviewPatrol.emPreviewMode;
	           		sPreviewPara.nModePara = 0;
				ModPreviewStart(&sPreviewPara);
			}
			else
			{
				if(0 != ModPatrolNext())
				{
					//printf("ModPreviewStart - failed\n");
					continue;
				}
			}
			
			nLastTimeStamp = nCurTimeStamp;
	        }
	}
   #endif
		
        usleep(50);
    }
	
    return 0;
}
#else
void* ModeCtrlFxn(void* arg)
{
    s32 i, nCurStop = 0;
    SPreviewPatrolPara sPreviewPatrol;
    SPreviewPara sPreviewPara;
    u32 nLastTimeStamp = 0, nCurTimeStamp = 0;
    
    memset(&sPreviewPatrol, 0, sizeof(SPreviewPatrolPara));

    while(1)
    {
 
        if (g_sPreviewManager.nIsPatrolParaChange)
        {
            pthread_mutex_lock(&g_sPreviewManager.lock);
            sPreviewPatrol = g_sPreviewManager.sPatrolPara;
            g_sPreviewManager.nIsPatrolParaChange = 0;
            pthread_mutex_unlock(&g_sPreviewManager.lock);
            
            nCurStop = 0;
        }

        nCurTimeStamp = PublicGetTimeStamp();
        if (sPreviewPatrol.nIsPatrol && sPreviewPatrol.nInterval > 0 && sPreviewPatrol.nStops > 1 && (nCurTimeStamp - nLastTimeStamp) >= sPreviewPatrol.nInterval * 1000)
        {
            sPreviewPara.emPreviewMode = sPreviewPatrol.emPreviewMode;
            sPreviewPara.nModePara = sPreviewPatrol.pnStopModePara[nCurStop];
            ModPreviewStart(&sPreviewPara);
            
            nLastTimeStamp = nCurTimeStamp;

            nCurStop++;
            if (nCurStop >= sPreviewPatrol.nStops)
            {
                nCurStop = 0;
            }
        }

        if (g_sPreviewManager.nIsModeChange)
        {
        	//printf("g_sPreviewManager.nIsModeChange  \n");
            pthread_mutex_lock(&g_sPreviewManager.lock);
            if (g_sPreviewManager.sInitPara.pfnPreviewCb)
            {
                SPreviewCbData sPreviewPara;

                if (g_sPreviewManager.nIsPreview)
                {
                    sPreviewPara.emPreviewMode = g_sPreviewManager.sCurPreview.emPreviewMode;
					//printf("@@@@@@@@@@  callback  mode  = %d\n ",sPreviewPara.emPreviewMode);
				}
                else
                {
                    sPreviewPara.emPreviewMode = EM_PREVIEW_CLOSE;
                }
                sPreviewPara.nModePara = g_sPreviewManager.sCurPreview.nModePara;
                sPreviewPara.nAdioChn = g_sPreviewManager.sPreviewAudio.nActiveChn;
                sPreviewPara.nMute = g_sPreviewManager.sPreviewAudio.nMute;
                sPreviewPara.nVolume = g_sPreviewManager.sPreviewAudio.nOutVolume;
                pthread_mutex_unlock(&g_sPreviewManager.lock);
				
                g_sPreviewManager.sInitPara.pfnPreviewCb(&sPreviewPara);
				
				pthread_mutex_lock(&g_sPreviewManager.lock);
            }
            g_sPreviewManager.nIsModeChange = 0;
            pthread_mutex_unlock(&g_sPreviewManager.lock);
        }

        usleep(50);
    }

    return 0;
}
#endif

s32 ModPreviewRectOsdShow(u8 nChn, u8 nIndex, u8 nShow, SPRect pSRect)
{
	//return 0;
	
	u8 nVideoNum = g_sPreviewManager.sInitPara.nVideoNum;
	if (0 == nVideoNum)
	{
		return -1;
	}
	
	u8 nRectOsdNumPerChn = g_sPreviewManager.sInitPara.nMaxRectOsdNum / nVideoNum;
	if(0 == nRectOsdNumPerChn)
	{
		return -1;
	}
	if(nChn < nVideoNum && nIndex < nRectOsdNumPerChn)
	{
		SPreviewRectOsd* pSRectOsd = NULL;
		
		pSRectOsd = &g_sPreviewManager.pSRectOsd[nChn * nRectOsdNumPerChn + nIndex];
		if (pSRectOsd->nFd >= 0)
		{
			if (nShow != ((pSRectOsd->nFd >= 0) ? 1 : 0) || memcmp(pSRect, &pSRectOsd->sRect, sizeof(SRect)))
			{
				tl_video_cover_destroy(pSRectOsd->nFd);
				//printf("tl_video_cover_destroy pSRectOsd->nFd=%d\n", pSRectOsd->nFd);
				pSRectOsd->nFd = -1;
			}
		}
		
		memcpy(&pSRectOsd->sRect, pSRect, sizeof(SRect));
		
		if (nShow && (pSRectOsd->nFd < 0))
		{
			video_cover_atr_t atr;
			
			atr.color = 0x000000;//0x00E77D4A;
			atr.x = pSRect->x;
			atr.y = pSRect->y;
			atr.width = pSRect->w;
			atr.height = pSRect->h;
			pSRectOsd->nFd = tl_video_cover_create(nChn, &atr);
			//printf("tl_video_cover_create fd=%d,rect:(%d,%d,%d,%d)\n", pSRectOsd->nFd, pSRect->x, pSRect->y, pSRect->w, pSRect->h);
		}
		return 0;
	}
	return -1;
}

//csp modify
s32 ModPreviewIsOpened(void)
{
	return g_sPreviewManager.nIsPreview;
}

//csp modify
s32 ModPreviewGetMode(void)//多通道预览时指示几画面显示
{
	return g_sPreviewManager.sCurPreview.emPreviewMode;
}
//yaogang modify 20141202
s32 ModPreviewGetModePara(void)//多通道预览时从哪个通道开始显示
{
	return g_sPreviewManager.sCurPreview.nModePara;
}

