/******************************************************************************

	TL hslib local common
	
	2008-12-16	created by kong
	2010-11-11	modify by andyrew

******************************************************************************/
#include "common.h"
#include "vio_common.h"
#include "lib_common.h"

//csp modify 20140423
//VO_INTF_SYNC_E g_enIntfSync = VO_OUTPUT_720P50;
VO_INTF_SYNC_E g_enIntfSync = VO_OUTPUT_720P60;

HI_S32 SAMPLE_COMM_VO_GetWH(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W,HI_U32 *pu32H, HI_U32 *pu32Frm)
{
	switch (enIntfSync)
	{
		case VO_OUTPUT_PAL       	:  *pu32W = 720;  *pu32H = 576;  *pu32Frm = 25; break;
		case VO_OUTPUT_NTSC      	:  *pu32W = 720;  *pu32H = 480;  *pu32Frm = 30; break;
		case VO_OUTPUT_800x600_60	:  *pu32W = 800;  *pu32H = 600;  *pu32Frm = 60; break;
		case VO_OUTPUT_720P50    	:  *pu32W = 1280; *pu32H = 720;  *pu32Frm = 50; break;
		case VO_OUTPUT_1080P24  	:  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 24; break;
		case VO_OUTPUT_720P60    	:  *pu32W = 1280; *pu32H = 720;  *pu32Frm = 60; break;
		case VO_OUTPUT_1080P30   	:  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 30; break;
		case VO_OUTPUT_1080P25   	:  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 25; break;
		case VO_OUTPUT_1080P50   	:  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 50; break;
		case VO_OUTPUT_1080P60   	:  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 60; break;
		case VO_OUTPUT_1024x768_60	:  *pu32W = 1024; *pu32H = 768;  *pu32Frm = 60; break;
		case VO_OUTPUT_1280x1024_60	:  *pu32W = 1280; *pu32H = 1024; *pu32Frm = 60; break;
		case VO_OUTPUT_1366x768_60	:  *pu32W = 1366; *pu32H = 768;  *pu32Frm = 60; break;
		case VO_OUTPUT_1440x900_60	:  *pu32W = 1440; *pu32H = 900;  *pu32Frm = 60; break;
		case VO_OUTPUT_1280x800_60	:  *pu32W = 1280; *pu32H = 800;  *pu32Frm = 60; break;
		default: 
			LIB_PRT("vo enIntfSync not support!\n");
			return HI_FAILURE;
	}
	return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VO_StartDevLayer(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr, HI_U32 u32SrcFrmRate)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_U32 u32Frm = 0;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	
	SAMPLE_COMM_VO_StopDevLayer(VoDev);//csp modify//否则N制会有bug
	
	if (0 == u32SrcFrmRate)
	{
	   LIB_PRT("vo u32SrcFrmRate invaild! %d!\n", u32SrcFrmRate);
	   return HI_FAILURE;
	}
	
	#ifdef HI3535//csp modify 20150110
	if(VoDev == VO_DEVICE_CVBS)
	{
		return HI_SUCCESS;
	}
	
	HI_U32 u32DispBufLen = 3;//10;//ddr down
	VO_LAYER VoLayer = VoDev;
	if(VoDev == VO_DEVICE_CVBS)
	{
		VoLayer = SAMPLE_VO_LAYER_VSD0;
	}
	s32Ret = HI_MPI_VO_SetDispBufLen(VoLayer, u32DispBufLen);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("Set display buf len failed with error code %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	#endif
	
	s32Ret = HI_MPI_VO_SetPubAttr(VoDev, pstPubAttr);
	if (s32Ret != HI_SUCCESS)
	{
	   LIB_PRT("failed with %#x!\n", s32Ret);
	   return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VO_Enable(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
	   LIB_PRT("failed with %#x!\n", s32Ret);
	   return HI_FAILURE;
	}
	
	s32Ret = SAMPLE_COMM_VO_GetWH(pstPubAttr->enIntfSync, &u32Width, &u32Height, &u32Frm);
	if (s32Ret != HI_SUCCESS)
	{
	   LIB_PRT("failed with %#x!\n", s32Ret);
	   return HI_FAILURE;
	}
	
	#ifdef HI3535
	if(VoDev == VO_DEVICE_CVBS)
	{
		VoDev = SAMPLE_VO_LAYER_VSD0;
	}
	
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoDev, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
	   LIB_PRT("failed with %#x!\n", s32Ret);
	   return HI_FAILURE;
	}
	#endif
	
	//csp modify 20140423
	if(u32SrcFrmRate > u32Frm)
	{
		u32SrcFrmRate = u32Frm;//视频显示帧率<=设备帧率，即刷新率，与时序相关
	}
	
	stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stLayerAttr.u32DispFrmRt = u32SrcFrmRate;
 	stLayerAttr.stDispRect.s32X		 = 0;
	stLayerAttr.stDispRect.s32Y		 = 0;
	stLayerAttr.stDispRect.u32Width	 = u32Width;
	stLayerAttr.stDispRect.u32Height  = u32Height;
	stLayerAttr.stImageSize.u32Width  = u32Width;
	stLayerAttr.stImageSize.u32Height = u32Height;
	
	printf("HI_MPI_VO_SetVideoLayerAttr: VoLayer=%d u32SrcFrmRate=%d\n",VoDev,u32SrcFrmRate);
 	
	s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoDev, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
	   LIB_PRT("failed with %#x!\n", s32Ret);
	   return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VO_EnableVideoLayer(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
	   LIB_PRT("failed with %#x!\n", s32Ret);
	   return HI_FAILURE;
	}
	//yaogang modify 20150922
	s32Ret = HI_MPI_VO_SetPlayToleration(VoDev, 300);//300ms
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("%s HI_MPI_VO_SetPlayToleration() failed!\n", __func__);
		return HI_FAILURE;
	}
	
	return s32Ret;
}


HI_S32 SAMPLE_COMM_VO_StopDevLayer(VO_DEV VoDev)
{
	HI_S32 s32Ret = HI_SUCCESS;
	
	#ifdef HI3535
	if(VoDev == VO_DEVICE_HD)
	{
		s32Ret = HI_MPI_VO_DisableVideoLayer(SAMPLE_VO_LAYER_VHD0);
	    if (s32Ret != HI_SUCCESS)
	    {
	        LIB_PRT("failed with %#x!\n", s32Ret);
	        return HI_FAILURE;
	    }
	}
	else if(VoDev == VO_DEVICE_CVBS)
	{
		s32Ret = HI_MPI_VO_DisableVideoLayer(SAMPLE_VO_LAYER_VSD0);
	    if (s32Ret != HI_SUCCESS)
	    {
	        LIB_PRT("failed with %#x!\n", s32Ret);
	        return HI_FAILURE;
	    }
	}
	#else
	s32Ret = HI_MPI_VO_DisableVideoLayer(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	#endif
	
	s32Ret = HI_MPI_VO_Disable(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	return s32Ret;
}

static HI_VOID SAMPLE_COMM_VO_HdmiConvertSync(VO_INTF_SYNC_E enIntfSync,
    HI_HDMI_VIDEO_FMT_E *penVideoFmt)
{
	switch (enIntfSync)
	{
		case VO_OUTPUT_PAL:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_PAL;
			break;
		case VO_OUTPUT_NTSC:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_NTSC;
			break;
		case VO_OUTPUT_1080P24:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_24;
			break;
		case VO_OUTPUT_1080P25:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_25;
			break;
		case VO_OUTPUT_1080P30:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
			break;
		case VO_OUTPUT_720P50:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_720P_50;
			break;
		case VO_OUTPUT_720P60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_720P_60;
			break;
		case VO_OUTPUT_1080I50:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_50;
			break;
		case VO_OUTPUT_1080I60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_60;
			break;
		case VO_OUTPUT_1080P50:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
			break;
		case VO_OUTPUT_1080P60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
			break;
		case VO_OUTPUT_576P50:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_576P_50;
			break;
		case VO_OUTPUT_480P60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_480P_60;
			break;
		case VO_OUTPUT_800x600_60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
			break;
		case VO_OUTPUT_1024x768_60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
			break;
		case VO_OUTPUT_1280x1024_60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
			break;
		case VO_OUTPUT_1366x768_60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
			break;
		case VO_OUTPUT_1440x900_60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60;
			break;
		case VO_OUTPUT_1280x800_60:
			*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
			break;
		default:
			LIB_PRT("Unkonw VO_INTF_SYNC_E value!\n");
			break;
	}
	
	return;
}

HI_S32 SAMPLE_COMM_VO_HdmiStart(VO_INTF_SYNC_E enIntfSync)
{
    HI_HDMI_INIT_PARA_S stHdmiPara;
    HI_HDMI_ATTR_S      stAttr;
    HI_HDMI_VIDEO_FMT_E enVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1920X1080_60;
	
    SAMPLE_COMM_VO_HdmiConvertSync(enIntfSync, &enVideoFmt);
	
    stHdmiPara.enForceMode = HI_HDMI_FORCE_HDMI;
    stHdmiPara.pCallBackArgs = NULL;
    stHdmiPara.pfnHdmiEventCallback = NULL;
    HI_MPI_HDMI_Init(&stHdmiPara);
	
    HI_MPI_HDMI_Open(HI_HDMI_ID_0);
	
    HI_MPI_HDMI_GetAttr(HI_HDMI_ID_0, &stAttr);
	
    stAttr.bEnableHdmi = HI_TRUE;
    
    stAttr.bEnableVideo = HI_TRUE;
    stAttr.enVideoFmt = enVideoFmt;
	
    stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
    stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
    stAttr.bxvYCCMode = HI_FALSE;
	
    stAttr.bEnableAudio = HI_FALSE;
    stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.bIsMultiChannel = HI_FALSE;
	
    stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;
	
    stAttr.bEnableAviInfoFrame = HI_TRUE;
    stAttr.bEnableAudInfoFrame = HI_TRUE;
    stAttr.bEnableSpdInfoFrame = HI_FALSE;
    stAttr.bEnableMpegInfoFrame = HI_FALSE;
	
    stAttr.bDebugFlag = HI_FALSE;          
    stAttr.bHDCPEnable = HI_FALSE;
	
    stAttr.b3DEnable = HI_FALSE;
    
    HI_MPI_HDMI_SetAttr(HI_HDMI_ID_0, &stAttr);
	
    HI_MPI_HDMI_Start(HI_HDMI_ID_0);
	
	#ifdef HDMI_HAS_AUDIO
	extern HI_S32 SAMPLE_COMM_AUDIO_StartHdmi();
	SAMPLE_COMM_AUDIO_StartHdmi();
	#endif
    
    printf("HDMI start success.\n");
	
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VO_HdmiStop(HI_VOID)
{
    HI_MPI_HDMI_Stop(HI_HDMI_ID_0);
    HI_MPI_HDMI_Close(HI_HDMI_ID_0);
    HI_MPI_HDMI_DeInit();
	
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VO_BindVoWbc(VO_DEV VoWbcDev, VO_DEV VoDev, VO_CHN VoChn)
{
#ifdef HI3535
	if(VoDev == VO_DEVICE_CVBS)
	{
		VoDev = SAMPLE_VO_LAYER_VSD0;
	}
#endif
	
    MPP_CHN_S stSrcChn, stDestChn;
	
    stSrcChn.enModId    = HI_ID_VOU;
    stSrcChn.s32DevId   = VoWbcDev;
    stSrcChn.s32ChnId   = 0;
	
    stDestChn.enModId   = HI_ID_VOU;
    stDestChn.s32ChnId  = VoChn;
    stDestChn.s32DevId  = VoDev;
	
    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

HI_S32 SAMPLE_COMM_VO_UnBindVoWbc(VO_DEV VoDev, VO_CHN VoChn)
{
    MPP_CHN_S stDestChn;
	
    stDestChn.enModId   = HI_ID_VOU;
    stDestChn.s32DevId  = VoDev;
    stDestChn.s32ChnId  = VoChn;
	
    return HI_MPI_SYS_UnBind(NULL, &stDestChn);
}

HI_S32 vio_enable_vo_all(LIB_VO_MODE_E enMode)
{
	VO_PUB_ATTR_S VoAttr;
	VO_DEV VoDev;
	VO_VIDEO_LAYER_ATTR_S stVideoLayerAttr;
	VO_PUB_ATTR_S stVoPubAttr; 
	HI_S32 gs_u32ViFrmRate;
	HI_S32 s32Ret = HI_SUCCESS;
	VO_WBC_ATTR_S stWbcAttr;
	
	memset(&VoAttr, 0, sizeof(VO_PUB_ATTR_S));
	memset(&stVideoLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
	
	gs_u32ViFrmRate = IS_PAL() ? 25 : 30;
	printf("gs_u32ViFrmRate: %d\n", gs_u32ViFrmRate);//25
	
	VoDev = VO_DEVICE_HD;
	stVoPubAttr.enIntfSync = g_enIntfSync;
	stVoPubAttr.enIntfType = VO_INTF_VGA | VO_INTF_HDMI; // VGA & HDMI output
	stVoPubAttr.u32BgColor = 0x00000000;
	
#ifdef HI3535
	//
#else
	#ifdef HI3531//csp modify
	stVoPubAttr.bDoubleFrame = SmallMemory_DoubleFrame_value; // In VI HD input case, this item should be set to HI_FALSE
	#else
	stVoPubAttr.bDoubleFrame = SmallMemory_DoubleFrame_value; // In VI HD input case, this item should be set to HI_FALSE
	#endif
#endif
	
	//csp modify 20130726
	//s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, gs_u32ViFrmRate);
	//s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, gs_u32ViFrmRate*2);
	s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, HD_FRAMERATE);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_StartDevLayer failed! vodev=%d\n",VoDev);
	}
	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, stVoPubAttr.enIntfSync, enMode);
	if (HI_SUCCESS != s32Ret)
	{
        LIB_PRT("SAMPLE_COMM_VO_StartChn failed! vodev=%d\n",VoDev);
	}
	
	// if it's displayed on HDMI, we should start HDMI 
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
		{
			LIB_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
		}
	}
	
	VoDev = VO_DEVICE_CVBS;
	stVoPubAttr.enIntfSync = IS_PAL()? VO_OUTPUT_PAL : VO_OUTPUT_NTSC;//csp modify
	stVoPubAttr.enIntfType = VO_INTF_CVBS;
	stVoPubAttr.u32BgColor = 0x00000000;
#ifdef HI3535
	//
#else
	stVoPubAttr.bDoubleFrame = HI_FALSE;
#endif
	//#ifndef DHD1_WBC_DSD0//csp modify 20121202
	s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, gs_u32ViFrmRate);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_StartDevLayer failed! vodev=%d\n",VoDev);
	}
	//#endif
	
#ifdef DHD1_WBC_DSD0
	#ifdef HI3535//csp modify 20150110
	return 0;
	#endif
	
	//printf("~~~~~~~~~~~~~~~~~~~use wbc1~~~~~~~~~~~~\n");
	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, stVoPubAttr.enIntfSync, VO_MODE_1MUX);
	if (HI_SUCCESS != s32Ret)
	{
        LIB_PRT("SAMPLE_COMM_VO_StartChn failed! vodev=%d\n",VoDev);
	}
	
	/******************************************
	start HD WBC 
	******************************************/	
	//csp modify
	#ifdef HI3535
	s32Ret = HI_MPI_VO_GetWbcAttr(SAMPLE_VO_WBC_BASE, &stWbcAttr);
	#else
	s32Ret = HI_MPI_VO_GetWbcAttr(VO_DEVICE_HD, &stWbcAttr);
	#endif
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_VO_GetWbcAttr failed!%#x\n", s32Ret);
	}
	printf("HI_MPI_VO_GetWbcAttr-1 w=%d h=%d\n",stWbcAttr.stTargetSize.u32Width,stWbcAttr.stTargetSize.u32Height);
	
	//csp modify
	s32Ret = SAMPLE_COMM_VO_GetWH(IS_PAL()? VO_OUTPUT_PAL : VO_OUTPUT_NTSC, \
	                  &stWbcAttr.stTargetSize.u32Width, \
	                  &stWbcAttr.stTargetSize.u32Height, \
	                  &stWbcAttr.u32FrameRate);
	stWbcAttr.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("SAMPLE_COMM_VO_GetWH failed!\n");
	}
	
	printf("HI_MPI_VO_SetWbcAttr-1 w=%d h=%d\n",stWbcAttr.stTargetSize.u32Width,stWbcAttr.stTargetSize.u32Height);
	
	#ifdef HI3535
	VO_WBC_SOURCE_S stWbcSource;
	stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
	stWbcSource.u32SourceId = VO_DEVICE_HD;
	s32Ret = HI_MPI_VO_SetWbcSource(SAMPLE_VO_WBC_BASE, &stWbcSource);
    if (s32Ret != HI_SUCCESS)
    {
        LIB_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
	
	s32Ret = HI_MPI_VO_SetWbcAttr(SAMPLE_VO_WBC_BASE, &stWbcAttr);
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_VO_SetWbcAttr failed!%#x\n", s32Ret);
		return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VO_EnableWbc(SAMPLE_VO_WBC_BASE);
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_VO_EnableWbc failed!%#x\n", s32Ret);
		return HI_FAILURE;
	}
	#else
	s32Ret = HI_MPI_VO_SetWbcAttr(VO_DEVICE_HD, &stWbcAttr);
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_VO_SetWbcAttr failed!%#x\n", s32Ret);
	}
	
	s32Ret = HI_MPI_VO_EnableWbc(VO_DEVICE_HD);
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_VO_EnableWbc failed!%#x\n", s32Ret);
	}
	#endif
	
	/******************************************
	Bind HD1 WBC to SD0 Chn0 
	******************************************/
	s32Ret = SAMPLE_COMM_VO_BindVoWbc(VO_DEVICE_HD, VO_DEVICE_CVBS, 0);
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("SAMPLE_COMM_VO_BindVoWbc failed!%#x\n", s32Ret);
	}
#else
	printf("~~~~~~~~~~~~~~~~~~~not use wbc~~~~~~~~~~~~\n");
	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, stVoPubAttr.enIntfSync, enMode);
	if (HI_SUCCESS != s32Ret)
	{
        LIB_PRT("SAMPLE_COMM_VO_StartChn failed! vodev=%d\n",VoDev);
	}
#endif
	
	return 0;
}


#ifdef FAST_SWITCH_PREVIEW
HI_S32 SAMPLE_COMM_VO_ShowChn(VO_DEV VoDev,VO_INTF_SYNC_E enIntfSync,LIB_VO_MODE_E enMode,int first_Chn,int ChnNum)
{
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_U32 u32Frm = 0;
	VO_CHN_ATTR_S stChnAttr;
	
	//printf("SAMPLE_COMM_VO_ShowChn start...\n");
	
	switch(enMode)
	{
		case VO_MODE_1MUX:
			u32WndNum = 1;
			u32Square = 1;
			break;
		case VO_MODE_4MUX:
			u32WndNum = 4;
			u32Square = 2;
			break;
		case VO_MODE_9MUX:
			if(ARG_VI_NUM_MAX == 8)
			{
				u32WndNum = 8;
			}
			else
			{
				u32WndNum = 9;
			}
			u32Square = 3;
			break;
		case VO_MODE_16MUX:
			u32WndNum = 16;
			u32Square = 4;
			break;
		case VO_MODE_25MUX:
			if(ARG_VI_NUM_MAX == 24)
			{
				u32WndNum = 24;
			}
			else
			{
				u32WndNum = 25;
			}
			u32Square = 5;
			break;
		case VO_MODE_36MUX:
			if(ARG_VI_NUM_MAX == 32)
			{
				u32WndNum = 32;
			}
			else
			{
				u32WndNum = 36;
			}
			u32Square = 6;
			break;
		default:
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
	}
	
	s32Ret = SAMPLE_COMM_VO_GetWH(enIntfSync, &u32Width, &u32Height, &u32Frm);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	int end_Chn = first_Chn+ChnNum;
	if (end_Chn > ARG_VI_NUM_MAX)
	{
		end_Chn = ARG_VI_NUM_MAX;
	}
	
	//printf("SAMPLE_COMM_VO_ShowChn:first_Chn=%d,end_Chn=%d\n",first_Chn,end_Chn);
	
	for (i=first_Chn; i<end_Chn; i++)
	{
		//printf("SAMPLE_COMM_VO_ShowChn:index=%d,u32Square=%d\n",i,u32Square);
		
		stChnAttr.stRect.s32X       = ALIGN_BACK((u32Width/u32Square) * ((i-first_Chn)%u32Square), 2);
		stChnAttr.stRect.s32Y       = ALIGN_BACK((u32Height/u32Square) * ((i-first_Chn)/u32Square), 2);
		stChnAttr.stRect.u32Width   = ALIGN_BACK(u32Width/u32Square, 2);
		stChnAttr.stRect.u32Height  = ALIGN_BACK(u32Height/u32Square, 2);
		stChnAttr.u32Priority       = 0;
		stChnAttr.bDeflicker        = HI_TRUE;
		
		//pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[i]);
		
		VO_CHN_ATTR_S stOldChnAttr;
		s32Ret = HI_MPI_VO_GetChnAttr(VoDev, i, &stOldChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			//pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[i]);
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__,  s32Ret);
			return HI_FAILURE;
		}
		
		if(stOldChnAttr.stRect.s32X == stChnAttr.stRect.s32X && 
			stOldChnAttr.stRect.s32Y == stChnAttr.stRect.s32Y && 
			stOldChnAttr.stRect.u32Width == stChnAttr.stRect.u32Width &&
			stOldChnAttr.stRect.u32Height == stChnAttr.stRect.u32Height)
		{
			//printf("VoChn[%02d] skip SetChnAttr\n",i);
		}
		else
		{
			s32Ret = HI_MPI_VO_SetChnAttr(VoDev, i, &stChnAttr);
			if (s32Ret != HI_SUCCESS)
			{
				//pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[i]);
				printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__,  s32Ret);
				return HI_FAILURE;
			}
		}
		
		//SAMPLE_COMM_VO_BindVpss(VoDev, i, i, VPSS_PRE0_CHN);//csp modify 20150110
		
		#ifdef HI3535
		//yaogang modify 20150609
		//是为了解决删除IPC通道后，
		//双击该通道区域会有一帧画面残留，逻辑上应该是黑屏

		VDEC_CHN_STAT_S stStat_vdec;
		s32Ret = HI_MPI_VDEC_Query(i, &stStat_vdec);
		if(HI_SUCCESS != s32Ret)
		{
			printf("preview chn%d function:%s,line:%d,HI_MPI_VDEC_Query ret: 0x%x\n", i, __FUNCTION__, __LINE__, s32Ret);
			return HI_FAILURE;
		}
		//如果VDEC通道启动接收，则显示
		if (stStat_vdec.bStartRecvStream)
		{
			s32Ret = HI_MPI_VO_ShowChn(VoDev, i);
		}
		//yaogang modify 20150609 end
		#else
		s32Ret = HI_MPI_VO_ChnShow(VoDev, i);
		#endif
		if (s32Ret != HI_SUCCESS)
		{
			//pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[i]);
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
		
		//SAMLE_COMM_VDEC_BindVpss(i, i);//csp modify 20150110
		
		//HI_MPI_VO_SetChnFrameRate(VoDev, i, 25);//csp modify 20140423
		
		//VPSS_GRP VpssGrp = i;
		//HI_MPI_VPSS_ResetGrp(VpssGrp);
		//VO_CHN VoChn = i;
		//HI_MPI_VO_ClearChnBuffer(VoDev, VoChn, HI_FALSE);//HI_TRUE
		
		//pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[i]);
	}
	
	return HI_SUCCESS;
}

HI_S32 adjust_vo_ch_attr(VO_DEV VoDev, LIB_VO_MODE_E enMode,int first_Chn,int ChnNum)
{
	HI_S32 i = 0;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32WndNum = 16;
	
	VO_INTF_SYNC_E enIntfSync;
	
	unsigned int t1 = getTimeStamp();
	
	switch(VoDev)
	{
		//case VO_DEVICE_HD:
		//	enIntfSync = g_enIntfSync;
		//	break;
		#ifdef HI3535
		case SAMPLE_VO_DEV_DSD0:
		#else
		case SAMPLE_VO_DEV_DSD0:
		case SAMPLE_VO_DEV_DSD1:
		#endif
		//printf("%s IS_PAL(): %d\n", __func__, IS_PAL());
			enIntfSync = IS_PAL()? VO_OUTPUT_PAL : VO_OUTPUT_NTSC;
			break;
		default:
			if(VoDev == VO_DEVICE_HD)
			{
				enIntfSync = g_enIntfSync;
			}
			else
			{
				enIntfSync = IS_PAL()? VO_OUTPUT_PAL : VO_OUTPUT_NTSC;
				LIB_PRT("VoDev index %d is error!\n", VoDev);
			}
			break;
	}
	
	if(TL_BOARD_TYPE_NR2116 == TL_HSLIB_TYPE || TL_BOARD_TYPE_NR3116 == TL_HSLIB_TYPE)
	{
		u32WndNum = 16;
	}
	else if(TL_BOARD_TYPE_NR3132 == TL_HSLIB_TYPE)
	{
		u32WndNum = 32;//36;
	}
	else if(TL_BOARD_TYPE_NR3124 == TL_HSLIB_TYPE)
	{
		u32WndNum = 24;//25;
	}
	else if(TL_BOARD_TYPE_NR1004 == TL_HSLIB_TYPE)
	{
		u32WndNum = 4;
	}
	else if(TL_BOARD_TYPE_NR1008 == TL_HSLIB_TYPE)
	{
		u32WndNum = 8;//9;
	}
	//printf("%s u32WndNum: %d\n", __func__, u32WndNum);
	#if 1
	s32Ret = HI_MPI_VO_SetAttrBegin(VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VO_SetAttrBegin failed!%d\n", s32Ret);
		return HI_FAILURE;
	}
	#endif
	
	for (i=0; i<u32WndNum; i++)
	{
	#ifdef HI3535
		//pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[i]);
		
		//SAMLE_COMM_VDEC_UnBindVpss(i, i);//csp modify 20150110

		//yaogang
		HI_MPI_VO_HideChn(VoDev, i);
		
		//SAMPLE_COMM_VO_UnBindVpss(VoDev, i);//csp modify 20150110
		
		//pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[i]);
	#else
		HI_MPI_VO_ChnHide(VoDev, i);
	#endif
	}
	
	//printf("SAMPLE_COMM_VO_ShowChn:first_Chn=%d,ChnNum=%d\n",first_Chn,ChnNum);

	//yaogang
	s32Ret = SAMPLE_COMM_VO_ShowChn(VoDev, enIntfSync, enMode, first_Chn, ChnNum);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
		return HI_FAILURE;
	}
	
	#if 1
	s32Ret = HI_MPI_VO_SetAttrEnd(VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VO_SetAttrEnd failed!ret:0x%x\n", s32Ret);
		return HI_FAILURE;
	}
	#endif
	
	unsigned int t2 = getTimeStamp();
	if(t2-t1>8) printf("adjust_vo_ch_attr span:%u###\n",t2-t1);
	
	return s32Ret;
}

#endif

HI_S32 vio_set_vo_ch_attr_one(VO_DEV VoDev, HI_U32 u32WndNum)
{
	LIB_VO_MODE_E enVoMode;
	VO_INTF_SYNC_E enIntfSync;
	HI_S32 s32Ret = HI_SUCCESS;
	
	LIB_VO_MODE_E previewMode = VO_MODE_16MUX;
	
	switch(u32WndNum)
	{
		case 1:
			enVoMode = VO_MODE_1MUX;
			break;
		case 4:
			enVoMode = VO_MODE_4MUX;
			break;
		case 9:
			enVoMode = VO_MODE_9MUX;
			break;
		case 16:
			enVoMode = VO_MODE_16MUX;
			break;
		case 25:
			enVoMode = VO_MODE_25MUX;
			break;
		case 36:
			enVoMode = VO_MODE_36MUX;
			break;
		default:
			enVoMode = VO_MODE_4MUX;
			LIB_PRT("the window number %d is error!\n", u32WndNum);
	}
	
	switch(VoDev)
	{
		//case VO_DEVICE_HD:
		//	enIntfSync = g_enIntfSync;
		//	break;
		#ifdef HI3535
		case SAMPLE_VO_DEV_DSD0:
		#else
		case SAMPLE_VO_DEV_DSD0:
		case SAMPLE_VO_DEV_DSD1:
		#endif
			enIntfSync = IS_PAL()? VO_OUTPUT_PAL : VO_OUTPUT_NTSC;
			break;
		default:
			if(VoDev == VO_DEVICE_HD)
			{
				enIntfSync = g_enIntfSync;
			}
			else
			{
				enIntfSync = IS_PAL()? VO_OUTPUT_PAL : VO_OUTPUT_NTSC;
				LIB_PRT("VoDev index %d is error!\n", VoDev);
			}
			break;
	}
	
	//LIB_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);
	
	s32Ret= HI_MPI_VO_SetAttrBegin(VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VO_SetAttrBegin failed!%d\n", s32Ret);
		return HI_FAILURE;
	}
	
	if(TL_BOARD_TYPE_NR2116 == TL_HSLIB_TYPE || TL_BOARD_TYPE_NR3116 == TL_HSLIB_TYPE)
	{
		previewMode = VO_MODE_16MUX;
	}
	else if(TL_BOARD_TYPE_NR3132 == TL_HSLIB_TYPE)
	{
		previewMode = VO_MODE_36MUX;
	}
	else if(TL_BOARD_TYPE_NR3124 == TL_HSLIB_TYPE)
	{
		previewMode = VO_MODE_25MUX;
	}
	else if(TL_BOARD_TYPE_NR1004 == TL_HSLIB_TYPE)
	{
		previewMode = VO_MODE_4MUX;
	}
	else if(TL_BOARD_TYPE_NR1008 == TL_HSLIB_TYPE)
	{
		previewMode = VO_MODE_9MUX;
	}
	
	s32Ret = SAMPLE_COMM_VO_StopChn(VoDev, previewMode);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_StopChn failed!\n");
		return HI_FAILURE;
	}
	
	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, enIntfSync, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
		return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VO_SetAttrEnd(VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VO_SetAttrEnd failed!%d\n", s32Ret);
		return HI_FAILURE;
	}
	
    return s32Ret;
}

HI_S32 vio_set_vo_ch_attr_all(HI_U32 u32ScreemDiv)
{
#ifdef DHD1_WBC_DSD0
	//printf("~~~~~~~~~~~~~~~~~~~use wbc2~~~~~~~~~~~~\n");
#else
	vio_set_vo_ch_attr_one(VO_DEVICE_CVBS, u32ScreemDiv);
#endif
	vio_set_vo_ch_attr_one(VO_DEVICE_HD, u32ScreemDiv);
	
	return 0;
}

HI_S32 SAMPLE_COMM_VO_BindVi(VO_DEV VoDev, VO_CHN VoChn, VI_CHN ViChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn, stDestChn;
	
	stSrcChn.enModId    = HI_ID_VIU;
	stSrcChn.s32DevId   = 0;
	stSrcChn.s32ChnId   = ViChn;
	
	stDestChn.enModId   = HI_ID_VOU;
	stDestChn.s32ChnId  = VoChn;
	stDestChn.s32DevId  = VoDev;
	
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	printf("~~~vi2vo~~~VoDev:%d, VoChn:%d, ViChn:%d\n", VoDev, VoChn, ViChn);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	return s32Ret;
}

HI_S32 SAMPLE_COMM_VO_UnBindVi(VO_DEV VoDev, VO_CHN VoChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stDestChn;
	
	stDestChn.enModId   = HI_ID_VOU;
	stDestChn.s32DevId  = VoDev;
	stDestChn.s32ChnId  = VoChn;
	
	s32Ret = HI_MPI_SYS_UnBind(NULL, &stDestChn);
	printf("~~~vi UnBind vo~~~VoDev:%d, VoChn:%d\n", VoDev, VoChn);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_SYS_UnBind failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	return s32Ret;
}

HI_S32 SAMPLE_COMM_VO_BindVpss(VO_DEV VoDev,VO_CHN VoChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;
	
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = VoDev;
	stDestChn.s32ChnId = VoChn;
	
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	//printf("~~~vpss2vo~~~VoDev:%d, VoChn:%d, VpssGrp:%d, VpssChn:%d\n", VoDev, VoChn, VpssGrp, VpssChn);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	return s32Ret;
}

HI_S32 SAMPLE_COMM_VO_UnBindVpss(VO_DEV VoDev,VO_CHN VoChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = VoDev;
	stDestChn.s32ChnId = VoChn;
	
	stSrcChn.enModId = 0;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;
	s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcChn);
	if(s32Ret != HI_SUCCESS)
	{
		//printf("HI_MPI_SYS_GetBindbyDest error!!!!!!!!!!%#x\n", s32Ret);
		return HI_SUCCESS;//csp modify
	}
	//printf("enModId:%d s32DevId:%d  s32ChnId:%d\n", stSrcChn.enModId, stSrcChn.s32DevId, stSrcChn.s32ChnId);
	
	s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	//printf("~~~un vpss2vo~~~VoDev:%d, VoChn:%d, VpssGrp:%d, VpssChn:%d\n", VoDev, VoChn, stSrcChn.s32DevId, stSrcChn.s32ChnId);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_SYS_UnBind failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	return s32Ret;
}

HI_S32 vio_bind_vi2vo_all(VI_CHN first_Chn, int ChnNum, VO_CHN first_VoChn, VPSS_GRP first_VpssGrp, int SubViChn_flag)
{
	HI_S32 s32Ret;
	HI_S32 i;
	VO_CHN VoChn = 0;
	VO_DEV VoDev;
	VPSS_GRP VpssGrp;
	
	VoChn = first_VoChn;
	VpssGrp = first_VpssGrp;
	
	if(ARG_VI_NUM_MAX == 8)
	{
		if(ChnNum == 9)
		{
			ChnNum = 8;
		}
	}
	
	for(i = first_Chn; i < first_Chn + ChnNum; i++, VoChn++, VpssGrp++)
	{		
		VoDev = VO_DEVICE_HD;
		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VPSS_PRE0_CHN);
		if (HI_SUCCESS != s32Ret)
		{
			LIB_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
			return HI_FAILURE;
		}
	}
	
	return HI_SUCCESS;
}

HI_S32 vio_unbind_vi2vo_all(VI_CHN first_VoChn, int ChnNum)
{
	HI_S32 s32Ret;
	HI_S32 i;
	VO_CHN VoChn;
	VO_DEV VoDev;
	
	VoChn = first_VoChn;
	
	if(ARG_VI_NUM_MAX == 8)
	{
		if(ChnNum == 9)
		{
			ChnNum = 8;
		}
	}
	else if(ARG_VI_NUM_MAX == 24)
	{
		if(ChnNum == 25)
		{
			ChnNum = 24;
		}
	}
	else if(ARG_VI_NUM_MAX == 32)
	{
		if(ChnNum == 36)
		{
			ChnNum = 32;
		}
	}
	
	for(i = 0; i < ChnNum; i++, VoChn++)
	{		
		VoDev = VO_DEVICE_HD;
		s32Ret = SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn);
		if (HI_SUCCESS != s32Ret)
		{
			LIB_PRT("SAMPLE_COMM_VO_UnBindVpss failed!\n");
			return HI_FAILURE;
		}
	}
	
	return HI_SUCCESS;
}

typedef struct
{
	int chn;
	int x;
	int y;
	unsigned int width;
	unsigned int height;
}vo_pos_t;

HI_S32 SAMPLE_COMM_VO_StopChn(VO_DEV VoDev,LIB_VO_MODE_E enMode)
{
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32WndNum = 0;
	
	#ifdef HI3535
	if(VoDev == VO_DEVICE_CVBS)
	{
		VoDev = SAMPLE_VO_LAYER_VSD0;
	}
	#endif
		
	switch(enMode)
	{
		case VO_MODE_1MUX:
		{
			u32WndNum = 1;
			break;
		}
		case VO_MODE_4MUX:
		{
			u32WndNum = 4;
			break;
		}
		case VO_MODE_9MUX:
		{
			if(ARG_VI_NUM_MAX == 8)
			{
				u32WndNum = 8;
			}
			else
			{
				u32WndNum = 9;
			}
			break;
		}
		case VO_MODE_16MUX:
		{
			u32WndNum = 16;
			break;
		}
		case VO_MODE_25MUX:
		{
			if(ARG_VI_NUM_MAX == 24)
			{
				u32WndNum = 24;
			}
			else
			{
				u32WndNum = 25;
			}
			break;
		}
		case VO_MODE_36MUX:
		{
			if(ARG_VI_NUM_MAX == 32)
			{
				u32WndNum = 32;
			}
			else
			{
				u32WndNum = 36;
			}
			break;
		}
		default:
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
	}
	
	for (i=0; i<u32WndNum; i++)
	{
		s32Ret = HI_MPI_VO_DisableChn(VoDev, i);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	}
	
	return s32Ret;
}

HI_S32 SAMPLE_COMM_VO_StartChn(VO_DEV VoDev,VO_INTF_SYNC_E enIntfSync,LIB_VO_MODE_E enMode)
{
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_U32 u32Frm = 0;
	VO_CHN_ATTR_S stChnAttr;
	
	#ifdef HI3535
	if(VoDev == VO_DEVICE_CVBS)
	{
		VoDev = SAMPLE_VO_LAYER_VSD0;
	}
	#endif
	
	switch(enMode)
	{
		case VO_MODE_1MUX:
			u32WndNum = 1;
			u32Square = 1;
			break;
		case VO_MODE_4MUX:
			u32WndNum = 4;
			u32Square = 2;
			break;
		case VO_MODE_9MUX:
			if(ARG_VI_NUM_MAX == 8)
			{
				u32WndNum = 8;
			}
			else
			{
				u32WndNum = 9;
			}
			u32Square = 3;
			break;
		case VO_MODE_16MUX:
			u32WndNum = 16;
			u32Square = 4;
			break;
		case VO_MODE_25MUX:
			if(ARG_VI_NUM_MAX == 24)
			{
				u32WndNum = 24;
			}
			else
			{
				u32WndNum = 25;
			}
			u32Square = 5;
			break;
		case VO_MODE_36MUX:
			if(ARG_VI_NUM_MAX == 32)
			{
				u32WndNum = 32;
			}
			else
			{
				u32WndNum = 36;
			}
			u32Square = 6;
			break;
		default:
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
	}
	
	s32Ret = SAMPLE_COMM_VO_GetWH(enIntfSync, &u32Width, &u32Height, &u32Frm);
	if (s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	for (i=0; i<u32WndNum; i++)
	{
		stChnAttr.stRect.s32X       = ALIGN_BACK((u32Width/u32Square) * (i%u32Square), 2);
		stChnAttr.stRect.s32Y       = ALIGN_BACK((u32Height/u32Square) * (i/u32Square), 2);
		stChnAttr.stRect.u32Width   = ALIGN_BACK(u32Width/u32Square, 2);
		stChnAttr.stRect.u32Height  = ALIGN_BACK(u32Height/u32Square, 2);
		stChnAttr.u32Priority       = 0;
		stChnAttr.bDeflicker        = HI_TRUE;
		
		s32Ret = HI_MPI_VO_SetChnAttr(VoDev, i, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__,  s32Ret);
			return HI_FAILURE;
		}
		
		s32Ret = HI_MPI_VO_EnableChn(VoDev, i);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
		
		//HI_MPI_VO_SetChnFrameRate(VoDev, i, 25);//csp modify 20140423
	}
	
	return HI_SUCCESS;
}

