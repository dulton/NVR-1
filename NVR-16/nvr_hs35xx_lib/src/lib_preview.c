/******************************************************************************

	TL hslib preview
	
	2008-12-16	created by kong
	2010-11-11	modify by andyrew

******************************************************************************/
#include "common.h"
#include "vio_common.h"
#include "lib_preview.h"
#include "lib_video_cover.h"
#include "lib_common.h"

extern unsigned char is_nvr_preview_vdec_open(void);
extern int nvr_preview_vdec_open(int vdec_first_chn, int vdec_chn_num);
extern int nvr_preview_vdec_close(void);
extern int nvr_preview_vdec_req_iframe(int vdec_first_chn, int vdec_chn_num);

extern void SampleWaitDestroyVdecChn(HI_S32 s32ChnID);
extern HI_S32 SAMPLE_VDEC_CreateVdecChn(HI_S32 s32ChnID, SIZE_S *pstSize, PAYLOAD_TYPE_E enType);
extern HI_S32 SAMLE_COMM_VDEC_BindVo(VDEC_CHN VdChn, VO_DEV VoDev, VO_CHN VoChn);
extern HI_S32 SAMLE_COMM_VDEC_UnBindVo(VDEC_CHN VdChn, VO_DEV VoDev, VO_CHN VoChn);
#ifndef FAST_SWITCH_PREVIEW
static u32 get_tick()
{
#if 1
	return 0;
#else
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	//return tv.tv_sec*HZ + (tv.tv_usec*HZ)/1000000;
	return tv.tv_sec*1000 + (tv.tv_usec)/1000;
	
	//return TickGet();
#endif
}
#endif

void tl_preview(pre_mode_e mode, int chn) 
{
	HI_S32 max_ch = ARG_VI_NUM_MAX;
	if(chn < 0 || chn >= max_ch)
	{
		printf("lib_preview.c@nvr_preview: err chn=%d\n", chn);
		return;
	}
	
	#ifndef FAST_SWITCH_PREVIEW
	u32 t1,t2;
	#endif
	
	#ifdef FAST_SWITCH_PREVIEW
	PREVIEW_LOCK();
	#endif
	
	//printf("lib_preview.c@nvr_preview: mode=%d, chn=%d\n", mode, chn);
	//printf("lib_preview.c@nvr_preview: last_pre_mode=%d, last_mode_arg=%d\n", lib_gbl_info.last_pre_mode, lib_gbl_info.last_mode_arg);
	
	if(mode != PREVIEW_CLOSE)
	{
		if((mode == lib_gbl_info.last_pre_mode) && (chn == lib_gbl_info.last_mode_arg))
		{
			#ifdef FAST_SWITCH_PREVIEW
			PREVIEW_UNLOCK();
			#endif
			
			return;
		}
		else
		{
			#ifdef FAST_SWITCH_PREVIEW
			//printf("%s 1\n", __func__);
			if(!is_nvr_preview_vdec_open())
			{
				//printf("%s 2\n", __func__);
				vio_set_vo_ch_attr_all(ARG_VO_NUM_MAX);
				vio_bind_vi2vo_all(0, ARG_VI_NUM_MAX, 0, 0, 0);
				nvr_preview_vdec_open(0, ARG_VI_NUM_MAX);
			}
			#else
			if(is_nvr_preview_vdec_open())
			{
				t1 = get_tick();
				
				nvr_preview_vdec_close();
				
				t2 = get_tick();
				//printf("close time:%d\n",t2-t1);
			}
			#endif
		}
	}
	else
	{
		#ifdef FAST_SWITCH_PREVIEW
		if(is_nvr_preview_vdec_open())
		{
			nvr_preview_vdec_close();
			vio_unbind_vi2vo_all(0, ARG_VI_NUM_MAX);
		}
		#else
		if(is_nvr_preview_vdec_open())
		{
			nvr_preview_vdec_close();
		}
		#endif
	}
	
	//printf("lib_preview.c@nvr_preview: last_pre_mode=%d, last_mode_arg=%d\n", lib_gbl_info.last_pre_mode, lib_gbl_info.last_mode_arg);
	
	#ifndef FAST_SWITCH_PREVIEW
	//unbind
	int last_pre_mode = lib_gbl_info.last_pre_mode;
	switch(last_pre_mode)
	{
		case PREVIEW_1D1:
			vio_unbind_vi2vo_all(0, 1);
			break;
		case PREVIEW_2PIC:
			vio_unbind_vi2vo_all(0, 2);
			break;
		case PREVIEW_4CIF:
			vio_unbind_vi2vo_all(0, 4);
			break;
		case PREVIEW_9CIF:
			vio_unbind_vi2vo_all(0, 9);
			break;
		case PREVIEW_10PIC:
			vio_unbind_vi2vo_all(0, 10);
			break;
		case PREVIEW_16CIF:
			vio_unbind_vi2vo_all(0, 16);
			break;
		case PREVIEW_25CIF:
			vio_unbind_vi2vo_all(0, 25);
			break;
		case PREVIEW_36CIF:
			vio_unbind_vi2vo_all(0, 36);
			break;
		case PREVIEW_CLOSE:
			break;
		default:
			break;
	}
	#endif
	
	//bind
	//printf("%s mode: %d\n", __func__, mode);
	switch(mode)
	{
		case PREVIEW_1D1:
		{
			//printf("PREVIEW_1D1 chn=%d\n",chn);
			
			#ifdef FAST_SWITCH_PREVIEW
			adjust_vo_ch_attr(VO_DEVICE_HD, VO_MODE_1MUX, chn, 1);
			//nvr_preview_vdec_req_iframe(chn, 1);
			#else
			t1 = get_tick();

			nvr_preview_vdec_open(chn, 1);
			
			t2 = get_tick();
			//printf("open time:%d\n",t2-t1);
			
			t1 = get_tick();
			
			vio_set_vo_ch_attr_all(1);
			
			t2 = get_tick();
			//printf("vo time:%d\n",t2-t1);
			
			t1 = get_tick();
			
			vio_bind_vi2vo_all(chn, 1, 0, chn, 0);
			
			t2 = get_tick();
			//printf("bind time:%d\n",t2-t1);
			
			nvr_preview_vdec_req_iframe(chn, 1);
			#endif
			
			//printf("PREVIEW_1D1 over\n");
		}break;
		case PREVIEW_4CIF:
		{
			#ifdef FAST_SWITCH_PREVIEW
			if((chn + 4 <= ARG_VI_NUM_MAX) || (chn == 0))
			{
				chn = chn;
			}
			else
			{
				chn = ARG_VI_NUM_MAX-4;
			}
			
			adjust_vo_ch_attr(VO_DEVICE_HD, VO_MODE_4MUX, chn, 4);
			#else
			nvr_preview_vdec_open(chn, 4);
			
			vio_set_vo_ch_attr_all(4);
			
			if(chn < 4)
				vio_bind_vi2vo_all(0, 4, 0, 0, 0);
			else if(chn < 8)
				vio_bind_vi2vo_all(4, 4, 0, 4, 0);
			else if(chn < 12)
				vio_bind_vi2vo_all(8, 4, 0, 8, 0);
			else if(chn < 16)
				vio_bind_vi2vo_all(12, 4, 0, 12, 0);
			else
				vio_bind_vi2vo_all(chn, 4, 0, chn, 0);
			
			nvr_preview_vdec_req_iframe(chn, 4);
			#endif
		}break;
		case PREVIEW_9CIF:
		{
			#ifdef FAST_SWITCH_PREVIEW
			if((chn + 9 <= ARG_VI_NUM_MAX) || (chn == 0))
			{
				chn = chn;
			}
			else
			{
				chn = ARG_VI_NUM_MAX-9;
			}
			
			adjust_vo_ch_attr(VO_DEVICE_HD, VO_MODE_9MUX, chn, 9);
			#else
			t1 = get_tick();
			
			if((chn + 9 <= ARG_VI_NUM_MAX) || (chn == 0))
				nvr_preview_vdec_open(chn, 9);
			else
				nvr_preview_vdec_open(ARG_VI_NUM_MAX-9, 9);
			
			t2 = get_tick();
			//printf("open time:%d\n",t2-t1);
			
			t1 = get_tick();
			
			vio_set_vo_ch_attr_all(9);
			
			t2 = get_tick();
			//printf("vo time:%d\n",t2-t1);
			
			t1 = get_tick();
			
			if((chn + 9 <= ARG_VI_NUM_MAX) || (chn == 0))
				vio_bind_vi2vo_all(chn, 9, 0, chn, 0);
			else
				vio_bind_vi2vo_all(ARG_VI_NUM_MAX-9, 9, 0, ARG_VI_NUM_MAX-9, 0);
			
			t2 = get_tick();
			//printf("bind time:%d\n",t2-t1);
			
			if((chn + 9 <= ARG_VI_NUM_MAX) || (chn == 0))
				chn = chn;
			else
				chn = ARG_VI_NUM_MAX-9;
			
			nvr_preview_vdec_req_iframe(chn, 9);
			#endif
		}break;
		case PREVIEW_16CIF:
		{
			#ifdef FAST_SWITCH_PREVIEW
			if((chn + 16 <= ARG_VI_NUM_MAX) || (chn == 0))
			{
				chn = chn;
			}
			else
			{
				chn = ARG_VI_NUM_MAX-16;
			}
			
			adjust_vo_ch_attr(VO_DEVICE_HD, VO_MODE_16MUX, chn, 16);
			#else
			if((chn + 16 <= ARG_VI_NUM_MAX) || (chn == 0))
				nvr_preview_vdec_open(chn, 16);
			else
				nvr_preview_vdec_open(ARG_VI_NUM_MAX-16, 16);
			
			vio_set_vo_ch_attr_all(16);
			
			if((chn + 16 <= ARG_VI_NUM_MAX) || (chn == 0))
				vio_bind_vi2vo_all(chn, 16, 0, chn, 0);
			else
				vio_bind_vi2vo_all(ARG_VI_NUM_MAX-16, 16, 0, ARG_VI_NUM_MAX-16, 0);
			
			if((chn + 16 <= ARG_VI_NUM_MAX) || (chn == 0))
				chn = chn;
			else
				chn = ARG_VI_NUM_MAX-16;
			
			nvr_preview_vdec_req_iframe(chn, 16);
			#endif
		}break;
		case PREVIEW_25CIF:
		{
			#ifdef FAST_SWITCH_PREVIEW
			if((chn + 25 <= ARG_VI_NUM_MAX) || (chn == 0))
			{
				chn = chn;
			}
			else
			{
				chn = ARG_VI_NUM_MAX-25;
			}
			
			adjust_vo_ch_attr(VO_DEVICE_HD, VO_MODE_25MUX, chn, 25);
			#else
			if((chn + 25 <= ARG_VI_NUM_MAX) || (chn == 0))
				nvr_preview_vdec_open(chn, 25);
			else
				nvr_preview_vdec_open(ARG_VI_NUM_MAX-25, 25);
			
			vio_set_vo_ch_attr_all(25);
			
			if((chn + 25 <= ARG_VI_NUM_MAX) || (chn == 0))
				vio_bind_vi2vo_all(chn, 25, 0, chn, 0);
			else
				vio_bind_vi2vo_all(ARG_VI_NUM_MAX-25, 25, 0, ARG_VI_NUM_MAX-25, 0);
			
			if((chn + 25 <= ARG_VI_NUM_MAX) || (chn == 0))
				chn = chn;
			else
				chn = ARG_VI_NUM_MAX-25;
			
			nvr_preview_vdec_req_iframe(chn, 25);
			#endif
		}break;
		case PREVIEW_36CIF:
		{
			#ifdef FAST_SWITCH_PREVIEW
			if((chn + 36 <= ARG_VI_NUM_MAX) || (chn == 0))
			{
				chn = chn;
			}
			else
			{
				chn = ARG_VI_NUM_MAX-36;
			}
			
			adjust_vo_ch_attr(VO_DEVICE_HD, VO_MODE_36MUX, chn, 36);
			#else
			nvr_preview_vdec_open(0, 36);
			
			vio_set_vo_ch_attr_all(36);
			
			vio_bind_vi2vo_all(0, 36, 0, 0, 0);
			
			nvr_preview_vdec_req_iframe(0, 36);
			#endif
		}break;
		case PREVIEW_CLOSE:
		{
			//printf("lib_preview.c@preview: close\n");
		}break;
		default:
		{
			printf("lib_preview.c@preview: not support mode=%d\n", mode);
		}break;
	}
	
	lib_gbl_info.last_pre_mode = mode;
	lib_gbl_info.last_mode_arg = chn;

	//printf("lib_preview.c@nvr_preview: last_pre_mode=%d, last_mode_arg=%d\n", lib_gbl_info.last_pre_mode, lib_gbl_info.last_mode_arg);
	#ifdef FAST_SWITCH_PREVIEW
	PREVIEW_UNLOCK();
	#endif
	
	return;
}

extern VO_INTF_SYNC_E g_enIntfSync;
static unsigned char g_elec_zoom_flag = 0;

int tl_zoom_windows(int eaFlag, int firstCh, pre_rect_s stCapRect)
{
	if(lib_gbl_info.last_pre_mode != PREVIEW_1D1 || lib_gbl_info.last_mode_arg != firstCh)
	{
		printf("zoom windows:param error-1\n");
		return -1;
	}
	
	VO_DEV VoDev = VO_DEVICE_HD;
	
	#ifdef FAST_SWITCH_PREVIEW
	HI_S32 s32VpssFirstGrpIdx = ARG_VO_NUM_MAX;
	HI_S32 s32VpssGrpCnt = 1;
	
	#ifdef HI3535
	VPSS_GRP VpssGrp = firstCh;//PIP
	#endif
	VPSS_GRP VpssGrp_Clip = ARG_VO_NUM_MAX;//ZOOM
	
	VO_CHN VoChn = firstCh;//PIP
	VO_CHN VoChn_Clip = ARG_VO_NUM_MAX;//ZOOM
	#else
	HI_S32 s32VpssFirstGrpIdx = ARG_VI_NUM_MAX;
	HI_S32 s32VpssGrpCnt = 1;
	
	VPSS_GRP VpssGrp = firstCh;//PIP
	VPSS_GRP VpssGrp_Clip = ARG_VI_NUM_MAX;//ZOOM
	
	VO_CHN VoChn = 0;//PIP
	VO_CHN VoChn_Clip = 1;//ZOOM
	#endif
	
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	
	//#ifdef HI3531
	//VPSS_CLIP_INFO_S stVpssClipInfo;
	//#else
	VPSS_CROP_INFO_S stVpssClipInfo;
	//#endif
	
	VO_VIDEO_LAYER_ATTR_S stPipLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
	
	HI_S32 s32Ret = HI_SUCCESS;
	
	//最新的SDK还需要这段代码吗???
	#if 0
	if(ARG_VI_NUM_MAX >= 32)
	{
		if(firstCh == 31)
		{
			s32VpssFirstGrpIdx = 30;
			VpssGrp_Clip = 30;
		}
		else
		{
			s32VpssFirstGrpIdx = 31;
			VpssGrp_Clip = 31;
		}
	}
	else if(ARG_VI_NUM_MAX >= 24)
	{
		s32VpssFirstGrpIdx = 32;
		VpssGrp_Clip = 32;
	}
	#endif
	
	if(eaFlag == PREVIEW_EA_CLOSE)
	{
		//printf("zoom windows close......\n");
		g_elec_zoom_flag = 0;
		goto END_1D1_CLIP_5;
	}
	
	#ifdef HI3535
	if(!IPC_GetLinkStatus(firstCh))
	{
		//printf("zoom windows:video lost\n");
		return -1;
	}

	//printf("%s yg 1\n", __func__);
	if(!g_elec_zoom_flag)
	{
		//printf("%s yg 2\n", __func__);
		HI_MPI_VO_GetVideoLayerAttr(SAMPLE_VO_LAYER_VPIP, &stPipLayerAttr);
	}
	#endif
	
	s32Ret = SAMPLE_COMM_VO_GetWH(g_enIntfSync,
				&stPipLayerAttr.stDispRect.u32Width,
				&stPipLayerAttr.stDispRect.u32Height,
				&stPipLayerAttr.u32DispFrmRt);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n", s32Ret);
		goto END_1D1_CLIP_5;
	}
	
	SIZE_S stSize = lib_gbl_info.main_stream_size[firstCh];
	
	if(stSize.u32Width == 704)
	{
		stSize.u32Width = 720;
	}
	
	//printf("origin rect:(%d,%d,%d,%d)\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height);
	
	if(stCapRect.u32Width < 32)
	{
		stCapRect.u32Width = 32;
	}
	if(stCapRect.u32Height < 16)
	{
		stCapRect.u32Height = 16;
	}
	
	stCapRect.s32X = stSize.u32Width*stCapRect.s32X/720;
	stCapRect.s32Y = stSize.u32Height*stCapRect.s32Y/(IS_PAL()?576:480);
	stCapRect.u32Width = stSize.u32Width*stCapRect.u32Width/720;
	stCapRect.u32Height = stSize.u32Height*stCapRect.u32Height/(IS_PAL()?576:480);
	
	unsigned int min_rect_w = (stPipLayerAttr.stDispRect.u32Width + 15) / 16;
	unsigned int min_rect_h = (stPipLayerAttr.stDispRect.u32Height + 15) / 16;
	min_rect_w = (min_rect_w + 15) / 16 * 16;
	min_rect_h = (min_rect_h + 15) / 16 * 16;
	if(stCapRect.u32Width < min_rect_w)
	{
		stCapRect.u32Width = min_rect_w;
	}
	if(stCapRect.u32Height < min_rect_h)
	{
		stCapRect.u32Height = min_rect_h;
	}
	if(stCapRect.s32X + stCapRect.u32Width > stSize.u32Width)
	{
		stCapRect.s32X = stSize.u32Width - stCapRect.u32Width;
	}
	if(stCapRect.s32Y + stCapRect.u32Height > stSize.u32Height)
	{
		stCapRect.s32Y = stSize.u32Height - stCapRect.u32Height;
	}
	
	if(g_elec_zoom_flag)
	{
		#ifdef HI3535
		s32Ret = HI_MPI_VPSS_GetGrpCrop(VpssGrp_Clip, &stVpssClipInfo);
		#else
		s32Ret = HI_MPI_VPSS_GetCropCfg(VpssGrp_Clip, &stVpssClipInfo);
		#endif
		
		/*** enable vpss group clip ***/
		stVpssClipInfo.bEnable = HI_TRUE;
		//#ifdef HI3531
		//stVpssClipInfo.enClipCoordinate = VPSS_CLIP_ABS_COOR;//VPSS_CLIP_RITIO_COOR;
		//stVpssClipInfo.stClipRect.s32X = ALIGN_BACK(stCapRect.s32X, 4);
		//stVpssClipInfo.stClipRect.s32Y = ALIGN_BACK(stCapRect.s32Y, 4);
		//stVpssClipInfo.stClipRect.u32Height = ALIGN_BACK(stCapRect.u32Height, 16);
		//stVpssClipInfo.stClipRect.u32Width = ALIGN_BACK(stCapRect.u32Width, 16);
		//#else
		stVpssClipInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;//VPSS_CLIP_RITIO_COOR;
		stVpssClipInfo.stCropRect.s32X = ALIGN_BACK(stCapRect.s32X, 4);
		stVpssClipInfo.stCropRect.s32Y = ALIGN_BACK(stCapRect.s32Y, 4);
		stVpssClipInfo.stCropRect.u32Height = ALIGN_BACK(stCapRect.u32Height, 16);
		stVpssClipInfo.stCropRect.u32Width = ALIGN_BACK(stCapRect.u32Width, 16);
		//#endif
		
		//printf("zoom rect:(%d,%d,%d,%d)\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height);
		
		#ifdef HI3535
		s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssClipInfo);
		#else
		s32Ret = HI_MPI_VPSS_SetCropCfg(VpssGrp_Clip, &stVpssClipInfo);
		#endif
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VPSS_SetCropCfg failed with %#x!\n", s32Ret);
			return -1;
		}
		
		return 0;
	}
	
	//HI_MPI_VDEC_StopRecvStream(firstCh);
	
	#ifndef FAST_SWITCH_PREVIEW
	vio_unbind_vi2vo_all(0, 1);
	//printf("vio_unbind_vi2vo_all [1] finish,vpss size:(%d,%d)\n",stSize.u32Width,stSize.u32Height);
	#endif
	
	HI_MPI_VO_SetAttrBegin(VoDev);
	
	#ifdef HI3535
	//printf("HI3535_zoom_windows-1\n");
	HI_MPI_VO_SetAttrBegin(SAMPLE_VO_LAYER_VPIP);
	SAMPLE_COMM_VO_UnBindVpss(VoDev, VoChn);
	HI_MPI_VO_DisableChn(VoDev, VoChn);
	//printf("HI3535_zoom_windows-2\n");
	#endif
	
	#ifdef FAST_SWITCH_PREVIEW
	HI_MPI_VO_DisableChn(VoDev, VoChn_Clip);
	#else
	HI_MPI_VO_DisableChn(VoDev, VoChn);
	HI_MPI_VO_DisableChn(VoDev, VoChn_Clip);
	#endif
	
	if(!g_elec_zoom_flag)
	{
		stSize.u32Width = VDEC_MAX_W;
		stSize.u32Height = VDEC_MAX_H;
		s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssFirstGrpIdx, s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, NULL);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("Start Vpss failed!\n");
			goto END_1D1_CLIP_2;
		}
		
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// bind vdec to vpss group
		stSrcChn.enModId = HI_ID_VDEC;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = firstCh;
		
		stDestChn.enModId = HI_ID_VPSS;
		stDestChn.s32DevId = VpssGrp_Clip;//ZOOM
		stDestChn.s32ChnId = 0;
		
		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if(s32Ret != HI_SUCCESS)
		{
			LIB_PRT("bind vi to vpss group failed with %#x!\n", s32Ret);
			goto END_1D1_CLIP_3;
		}
		// bind vdec to vpss group
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		
		#ifdef HI3535
		//printf("HI3535_zoom_windows-3\n");
		
		HI_MPI_VO_DisableVideoLayer(SAMPLE_VO_LAYER_VPIP);
		
		s32Ret = HI_MPI_VO_BindVideoLayer(SAMPLE_VO_LAYER_VPIP, VoDev);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_PipLayerBindDev failed with %#x!\n", s32Ret);
			goto END_1D1_CLIP_3;
		}
		
		stPipLayerAttr.stDispRect.s32X = 0;
		stPipLayerAttr.stDispRect.s32Y = 0;
		stPipLayerAttr.stImageSize.u32Height = stPipLayerAttr.stDispRect.u32Height;
		stPipLayerAttr.stImageSize.u32Width = stPipLayerAttr.stDispRect.u32Width;
		stPipLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

		printf("%s MenuRect.s32X: %d\n", __func__, stPipLayerAttr.stDispRect.s32X);
		printf("%s MenuRect.s32Y: %d\n", __func__, stPipLayerAttr.stDispRect.s32Y);
		printf("%s MenuRect.u32Width: %d\n", __func__, stPipLayerAttr.stDispRect.u32Width);
		printf("%s MenuRect.u32Height: %d\n", __func__, stPipLayerAttr.stDispRect.u32Height);
		
		//printf("SetPipLayerAttr:(%d,%d)\n",stPipLayerAttr.stDispRect.u32Width,stPipLayerAttr.stDispRect.u32Height);
		s32Ret = HI_MPI_VO_SetVideoLayerAttr(SAMPLE_VO_LAYER_VPIP, &stPipLayerAttr);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_SetPipLayerAttr failed with %#x!\n", s32Ret);
			goto END_1D1_CLIP_4;
		}
		
		s32Ret = HI_MPI_VO_EnableVideoLayer(SAMPLE_VO_LAYER_VPIP);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_EnablePipLayer failed with %#x!\n", s32Ret);
			goto END_1D1_CLIP_4;
		}
		//yaogang modify 20150922
		s32Ret = HI_MPI_VO_SetPlayToleration(SAMPLE_VO_LAYER_VPIP, 300);//300ms
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("%s HI_MPI_VO_SetPlayToleration() failed!\n", __func__);
			goto END_1D1_CLIP_4;
		}
		
		#else
		HI_MPI_VO_DisablePipLayer();
		
		s32Ret = HI_MPI_VO_PipLayerBindDev(VoDev);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_PipLayerBindDev failed with %#x!\n", s32Ret);
			goto END_1D1_CLIP_3;
		}
		
		stPipLayerAttr.stDispRect.s32X = 0;
		stPipLayerAttr.stDispRect.s32Y = 0;
		stPipLayerAttr.stImageSize.u32Height = stPipLayerAttr.stDispRect.u32Height;
		stPipLayerAttr.stImageSize.u32Width = stPipLayerAttr.stDispRect.u32Width;
		stPipLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		//printf("SetPipLayerAttr:(%d,%d)\n",stPipLayerAttr.stDispRect.u32Width,stPipLayerAttr.stDispRect.u32Height);
		s32Ret = HI_MPI_VO_SetPipLayerAttr(&stPipLayerAttr);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_SetPipLayerAttr failed with %#x!\n", s32Ret);
			goto END_1D1_CLIP_4;
		}
		
		s32Ret = HI_MPI_VO_EnablePipLayer();
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_EnablePipLayer failed with %#x!\n", s32Ret);
			goto END_1D1_CLIP_4;
		}
		#endif
	}
	
	g_elec_zoom_flag = 1;
	
#ifdef HI3535
	s32Ret = HI_MPI_VPSS_GetGrpCrop(VpssGrp_Clip, &stVpssClipInfo);
#else
	s32Ret = HI_MPI_VPSS_GetCropCfg(VpssGrp_Clip, &stVpssClipInfo);
#endif
	
	/*** enable vpss group clip ***/
	stVpssClipInfo.bEnable = HI_TRUE;
//#ifdef HI3531
//	stVpssClipInfo.enClipCoordinate = VPSS_CLIP_ABS_COOR;//VPSS_CLIP_RITIO_COOR;
//	stVpssClipInfo.stClipRect.s32X = ALIGN_BACK(stCapRect.s32X, 4);
//	stVpssClipInfo.stClipRect.s32Y = ALIGN_BACK(stCapRect.s32Y, 4);
//	stVpssClipInfo.stClipRect.u32Height = ALIGN_BACK(stCapRect.u32Height, 16);
//	stVpssClipInfo.stClipRect.u32Width = ALIGN_BACK(stCapRect.u32Width, 16);
//#else
	stVpssClipInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;//VPSS_CLIP_RITIO_COOR;
	stVpssClipInfo.stCropRect.s32X = ALIGN_BACK(stCapRect.s32X, 4);
	stVpssClipInfo.stCropRect.s32Y = ALIGN_BACK(stCapRect.s32Y, 4);
	stVpssClipInfo.stCropRect.u32Height = ALIGN_BACK(stCapRect.u32Height, 16);
	stVpssClipInfo.stCropRect.u32Width = ALIGN_BACK(stCapRect.u32Width, 16);
//#endif
	
	//printf("zoom rect:(%d,%d,%d,%d)\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height);
	
#ifdef HI3535
	s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssClipInfo);
#else
	s32Ret = HI_MPI_VPSS_SetCropCfg(VpssGrp_Clip, &stVpssClipInfo);
#endif
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VPSS_SetCropCfg failed with %#x!\n", s32Ret);
		goto END_1D1_CLIP_4;
	}
	
	/***vo chn 0: pip***/
	//占1/4个屏幕
	stChnAttr.stRect.u32Width	= ALIGN_BACK(stPipLayerAttr.stDispRect.u32Width/4, 4);
	stChnAttr.stRect.u32Height	= ALIGN_BACK(stPipLayerAttr.stDispRect.u32Height/4, 4);
	stChnAttr.stRect.s32X		= ALIGN_BACK(stPipLayerAttr.stDispRect.u32Width/4*3, 4);
	stChnAttr.stRect.s32Y		= ALIGN_BACK(stPipLayerAttr.stDispRect.u32Height/4*3, 4);
	stChnAttr.u32Priority		= 1;//PIP层
	stChnAttr.bDeflicker		= HI_TRUE;//HI_FALSE;
	//printf("PIP VOChnAttr:(%d,%d,%d,%d)\n",stChnAttr.stRect.s32X,stChnAttr.stRect.s32Y,stChnAttr.stRect.u32Width,stChnAttr.stRect.u32Height);
#ifdef HI3535
	s32Ret = HI_MPI_VO_SetChnAttr(SAMPLE_VO_LAYER_VPIP, VoChn, &stChnAttr);
#else
	s32Ret = HI_MPI_VO_SetChnAttr(VoDev, VoChn, &stChnAttr);
#endif
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VO_GetChnAttr failed with %#x!\n", s32Ret);
		goto END_1D1_CLIP_4;
	}
	
#ifdef HI3535
	s32Ret = HI_MPI_VO_EnableChn(SAMPLE_VO_LAYER_VPIP, VoChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n", s32Ret);
		goto END_1D1_CLIP_5;
	}
#else
	#ifndef FAST_SWITCH_PREVIEW
	s32Ret = HI_MPI_VO_EnableChn(VoDev, VoChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n", s32Ret);
		goto END_1D1_CLIP_5;
	}
	#endif
#endif
	
	/***vo chn 1: clip***/
	//占整个屏幕
	stChnAttr.stRect.u32Width	= ALIGN_BACK(stPipLayerAttr.stDispRect.u32Width, 4);
	stChnAttr.stRect.u32Height	= ALIGN_BACK(stPipLayerAttr.stDispRect.u32Height, 4);
	stChnAttr.stRect.s32X		= ALIGN_BACK(0, 4);
	stChnAttr.stRect.s32Y		= ALIGN_BACK(0, 4);
	stChnAttr.u32Priority		= 0;//原视频层
	stChnAttr.bDeflicker		= HI_TRUE;//HI_FALSE;
	//printf("ZOOM VOChnAttr:(%d,%d,%d,%d)\n",stChnAttr.stRect.s32X,stChnAttr.stRect.s32Y,stChnAttr.stRect.u32Width,stChnAttr.stRect.u32Height);
	s32Ret = HI_MPI_VO_SetChnAttr(VoDev, VoChn_Clip, &stChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n",  s32Ret);
		goto END_1D1_CLIP_5;
	}
	
	s32Ret = HI_MPI_VO_EnableChn(VoDev, VoChn_Clip);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("failed with %#x!\n", s32Ret);
		goto END_1D1_CLIP_5;
	}
	
	HI_MPI_VO_SetAttrEnd(VoDev);
	
#ifdef HI3535
	HI_MPI_VO_SetAttrEnd(SAMPLE_VO_LAYER_VPIP);
	
	s32Ret = SAMPLE_COMM_VO_BindVpss(SAMPLE_VO_LAYER_VPIP, VoChn, VpssGrp, VPSS_PRE0_CHN);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
		goto END_1D1_CLIP_5;
	}
#else
	#ifndef FAST_SWITCH_PREVIEW
	s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VPSS_PRE0_CHN);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
		goto END_1D1_CLIP_5;
	}
	#endif
#endif
	
	s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn_Clip, VpssGrp_Clip, VPSS_PRE0_CHN);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
		goto END_1D1_CLIP_5;
	}
	
	//HI_MPI_VDEC_StartRecvStream(firstCh);
	
	//printf("zoom over######\n");
	
	return 0;
	
	/******************************************
	exit process
	******************************************/
END_1D1_CLIP_5:
	//lib_gbl_info.last_pre_mode = PREVIEW_CLOSE;
	//HI_MPI_VDEC_StopRecvStream(firstCh);
	#ifdef FAST_SWITCH_PREVIEW
	SAMPLE_COMM_VO_UnBindVpss(VoDev, VoChn_Clip);
	HI_MPI_VO_DisableChn(VoDev, VoChn_Clip);
	#ifdef HI3535
	SAMPLE_COMM_VO_UnBindVpss(SAMPLE_VO_LAYER_VPIP, VoChn);
	HI_MPI_VO_DisableChn(SAMPLE_VO_LAYER_VPIP, VoChn);
	#endif
	#else
	SAMPLE_COMM_VO_UnBindVpss(VoDev, VoChn);
	SAMPLE_COMM_VO_UnBindVpss(VoDev, VoChn_Clip);
	HI_MPI_VO_DisableChn(VoDev, VoChn);
	HI_MPI_VO_DisableChn(VoDev, VoChn_Clip);
	#endif
END_1D1_CLIP_4:
	#ifdef HI3535
	HI_MPI_VO_DisableVideoLayer(SAMPLE_VO_LAYER_VPIP);
	HI_MPI_VO_UnBindVideoLayer(SAMPLE_VO_LAYER_VPIP, VoDev);
	#else
	HI_MPI_VO_DisablePipLayer();
	HI_MPI_VO_PipLayerUnBindDev(VoDev);
	#endif
END_1D1_CLIP_3:
	stSrcChn.enModId = HI_ID_VDEC;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = firstCh;
	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp_Clip;
	stDestChn.s32ChnId = 0;
	HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
END_1D1_CLIP_2:
	SAMPLE_COMM_VPSS_Stop(s32VpssFirstGrpIdx, s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
//END_1D1_CLIP_1:
	#ifdef FAST_SWITCH_PREVIEW
	#ifdef HI3535
	HI_MPI_VO_EnableChn(VoDev, VoChn);
	SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VPSS_PRE0_CHN);
	#endif
	adjust_vo_ch_attr(VoDev, VO_MODE_1MUX, firstCh, 1);
	#else
	vio_set_vo_ch_attr_all(1);
	vio_bind_vi2vo_all(firstCh, 1, 0, firstCh, 0);
	#endif
	
	//lib_gbl_info.byMainWaitIFrame[firstCh] = 1;
	//lib_gbl_info.last_pre_mode = PREVIEW_1D1;
	//HI_MPI_VDEC_StartRecvStream(firstCh);
	
	g_elec_zoom_flag = 0;
	
	if(s32Ret == HI_SUCCESS)
	{
		printf("preview elec zoom close\n");
	}
	else
	{
		printf("preview elec zoom failed\n");
	}
	
	return s32Ret;
}

//yaogang modify 20150110
/******************************
17-19通道号。0-15已经作为32路预览的高16通道
-----------------------------
					|		|
		17			|		|
					|		|
--------------------|		|
					|  		|
		16			|  19	|
		view			|  menu	|
					|		|
					|		|
--------------------|		|
					|		|
		18			|		|
					|		|
-----------------------------
图片分辨率704 * 576
********************************/
#define SNAP_WIDTH	960
#define SNAP_HEIGHT	576
//							0:open  1:close
int tl_snap_preview_init(int eaFlag, pre_rect_s * pRectMenu)
{
	VO_DEV VoDev = VO_DEVICE_HD;
	HI_S32 s32Ret = HI_SUCCESS;

	int i;
	VDEC_CHN VdChn;
	
	/************************************************
		step3:  start VDEC
	*************************************************/
	SIZE_S stSize;
	stSize.u32Width = SNAP_WIDTH;
	stSize.u32Height = SNAP_HEIGHT;

	if (eaFlag)
	{
		goto error7;
	}

	//for(i = 0; i < open_chn_num; i++)
	for (i = ARG_VO_NUM_MAX; i < 4+ARG_VO_NUM_MAX; i++)
	{
		/*** create vdec chn ***/
		VdChn = i;
		s32Ret = SAMPLE_VDEC_CreateVdecChn(VdChn, &stSize, PT_JPEG);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("create vdec chn%d failed!\n", VdChn);
			goto error3;
		}
		else
		{
			LIB_PRT("create vdec chn%d success!\n", VdChn);
		}
		#if 0
		/*** bind vdec to vpss ***/
		VpssGrp = i + s32VpssFirstGrpIdx;
		s32Ret = SAMLE_COMM_VDEC_BindVpss(VdChn, VpssGrp);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("vdec(vdch=%d) bind vpss(vpssg=%d) failed!\n", VdChn, VpssGrp);
			goto error3;
		}
		#endif 
	}
	
	/************************************************
		step4:  start VO
    	*************************************************/
	VO_VIDEO_LAYER_ATTR_S stPipLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
	VO_CHN VoChn = 0;//PIP

	HI_MPI_VO_GetVideoLayerAttr(SAMPLE_VO_LAYER_VPIP, &stPipLayerAttr);

	s32Ret = SAMPLE_COMM_VO_GetWH(g_enIntfSync,
				&stPipLayerAttr.stDispRect.u32Width,
				&stPipLayerAttr.stDispRect.u32Height,
				&stPipLayerAttr.u32DispFrmRt);
	if(s32Ret != HI_SUCCESS)
	{
		//printf("%s SAMPLE_COMM_VO_GetWH failed with %#x!\n", __func__, s32Ret);
		LIB_PRT("failed with %#x!\n", s32Ret);
		goto error3;
	}
	
	#ifdef HI3535
		//printf("HI3535_zoom_windows-3\n");
		
		HI_MPI_VO_DisableVideoLayer(SAMPLE_VO_LAYER_VPIP);
	
		HI_MPI_VO_SetVideoLayerPartitionMode(SAMPLE_VO_LAYER_VPIP, VO_PART_MODE_SINGLE); 
		
		s32Ret = HI_MPI_VO_BindVideoLayer(SAMPLE_VO_LAYER_VPIP, VoDev);
		if(HI_SUCCESS != s32Ret)
		{
			//printf("%s HI_MPI_VO_PipLayerBindDev failed with %#x!\n", __func__, s32Ret);
			LIB_PRT("HI_MPI_VO_PipLayerBindDev failed with %#x!\n", s32Ret);
			goto error3;
		}
		
		stPipLayerAttr.stDispRect.s32X = 0;
		stPipLayerAttr.stDispRect.s32Y = 0;
		stPipLayerAttr.stImageSize.u32Height = stPipLayerAttr.stDispRect.u32Height;
		stPipLayerAttr.stImageSize.u32Width = stPipLayerAttr.stDispRect.u32Width;
		stPipLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
/*
		printf("%s MenuRect.s32X: %d\n", __func__, stPipLayerAttr.stDispRect.s32X);
		printf("%s MenuRect.s32Y: %d\n", __func__, stPipLayerAttr.stDispRect.s32Y);
		printf("%s MenuRect.u32Width: %d\n", __func__, stPipLayerAttr.stDispRect.u32Width);
		printf("%s MenuRect.u32Height: %d\n", __func__, stPipLayerAttr.stDispRect.u32Height);
*/		
		//printf("SetPipLayerAttr:(%d,%d)\n",stPipLayerAttr.stDispRect.u32Width,stPipLayerAttr.stDispRect.u32Height);
		s32Ret = HI_MPI_VO_SetVideoLayerAttr(SAMPLE_VO_LAYER_VPIP, &stPipLayerAttr);
		if(HI_SUCCESS != s32Ret)
		{
			//printf("%s HI_MPI_VO_SetPipLayerAttr failed with %#x!\n", __func__, s32Ret);
			LIB_PRT("HI_MPI_VO_SetPipLayerAttr failed with %#x!\n", s32Ret);
			goto error4;
		}
				
		s32Ret = HI_MPI_VO_EnableVideoLayer(SAMPLE_VO_LAYER_VPIP);
		if(HI_SUCCESS != s32Ret)
		{
			//printf("%s HI_MPI_VO_EnableVideoLayer failed with %#x!\n", __func__, s32Ret);
			LIB_PRT("HI_MPI_VO_EnablePipLayer failed with %#x!\n", s32Ret);
			goto error4;
		}
		
		//printf("%s HI_MPI_VO_EnableVideoLayer !\n", __func__);
	#else
		HI_MPI_VO_DisablePipLayer();
		
		s32Ret = HI_MPI_VO_PipLayerBindDev(VoDev);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_PipLayerBindDev failed with %#x!\n", s32Ret);
			goto error3;
		}
		
		stPipLayerAttr.stDispRect.s32X = 0;
		stPipLayerAttr.stDispRect.s32Y = 0;
		stPipLayerAttr.stImageSize.u32Height = stPipLayerAttr.stDispRect.u32Height;
		stPipLayerAttr.stImageSize.u32Width = stPipLayerAttr.stDispRect.u32Width;
		stPipLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		//printf("SetPipLayerAttr:(%d,%d)\n",stPipLayerAttr.stDispRect.u32Width,stPipLayerAttr.stDispRect.u32Height);
		s32Ret = HI_MPI_VO_SetPipLayerAttr(&stPipLayerAttr);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_SetPipLayerAttr failed with %#x!\n", s32Ret);
			goto error4;
		}
		
		s32Ret = HI_MPI_VO_EnablePipLayer();
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("HI_MPI_VO_EnablePipLayer failed with %#x!\n", s32Ret);
			goto error4;
		}
	#endif

	//HI_MPI_VO_SetVideoLayerPartitionMode(SAMPLE_VO_LAYER_VPIP, VO_PART_MODE_SINGLE);
	// 4 chn  16
	//#define ARG_VI_NUM_MAX 				(lib_gbl_info.vi_num_max)
	//#define ARG_VO_NUM_MAX 				(lib_gbl_info.vo_num_max)
	RECT_S chn_rect[4];
	//16
	chn_rect[0].u32Width	= stPipLayerAttr.stDispRect.u32Width - pRectMenu->u32Width;
	chn_rect[0].u32Height	= pRectMenu->u32Height /4 *2;
	chn_rect[0].s32X		= 0;
	chn_rect[0].s32Y		= pRectMenu->u32Height /4;
	// 17
	chn_rect[1].u32Width	= stPipLayerAttr.stDispRect.u32Width - pRectMenu->u32Width;
	chn_rect[1].u32Height	= pRectMenu->u32Height / 4;
	chn_rect[1].s32X		= 0;
	chn_rect[1].s32Y		= 0;
	// 18
	chn_rect[2].u32Width	= stPipLayerAttr.stDispRect.u32Width - pRectMenu->u32Width;
	chn_rect[2].u32Height	= pRectMenu->u32Height / 4;
	chn_rect[2].s32X		= 0;
	chn_rect[2].s32Y		= pRectMenu->u32Height / 4 * 3;
	// 19
	chn_rect[3].u32Width	= pRectMenu->u32Width;
	chn_rect[3].u32Height	= pRectMenu->u32Height;
	chn_rect[3].s32X		= stPipLayerAttr.stDispRect.u32Width - pRectMenu->u32Width;
	chn_rect[3].s32Y		= 0;
	
	for (i=0; i<4; i++)
	{
		stChnAttr.stRect.u32Width		= ALIGN_BACK(chn_rect[i].u32Width, 4);
		stChnAttr.stRect.u32Height	= ALIGN_BACK(chn_rect[i].u32Height, 4);
		stChnAttr.stRect.s32X		= ALIGN_BACK(chn_rect[i].s32X, 4);
		stChnAttr.stRect.s32Y		= ALIGN_BACK(chn_rect[i].s32Y, 4);
		/*
		printf("%s stChnAttr.stRect.u32Width: %d\n", __func__, stChnAttr.stRect.u32Width);
		printf("%s stChnAttr.stRect.u32Height: %d\n", __func__, stChnAttr.stRect.u32Height);
		printf("%s stChnAttr.stRect.s32X: %d\n", __func__, stChnAttr.stRect.s32X);
		printf("%s stChnAttr.stRect.s32Y: %d\n", __func__, stChnAttr.stRect.s32Y);
		*/
		stChnAttr.u32Priority		= 1;//PIP层
		stChnAttr.bDeflicker		= HI_TRUE;//HI_FALSE;
		//printf("PIP VOChnAttr:(%d,%d,%d,%d)\n",stChnAttr.stRect.s32X,stChnAttr.stRect.s32Y,stChnAttr.stRect.u32Width,stChnAttr.stRect.u32Height);
		VoChn = i + ARG_VO_NUM_MAX; //PIP 16 开始

		#ifdef HI3535
			s32Ret = HI_MPI_VO_SetChnAttr(SAMPLE_VO_LAYER_VPIP, VoChn, &stChnAttr);
		#else
			s32Ret = HI_MPI_VO_SetChnAttr(VoDev, VoChn, &stChnAttr);
		#endif
		if(HI_SUCCESS != s32Ret)
		{
			//printf("%s HI_MPI_VO_GetChnAttr failed with %#x!\n", __func__, s32Ret);
			LIB_PRT("chn%d HI_MPI_VO_GetChnAttr failed with %#x!\n", VoChn, s32Ret);
			goto error5;
		}

		//enable chn
		#ifdef HI3535
		s32Ret = HI_MPI_VO_EnableChn(SAMPLE_VO_LAYER_VPIP, VoChn);
		if(s32Ret != HI_SUCCESS)
		{
			//printf("%s HI_MPI_VO_EnableChn failed with %#x!\n", __func__, s32Ret);
			LIB_PRT("chn%d HI_MPI_VO_EnableChn failed with %#x!\n", VoChn, s32Ret);
			goto error6;
		}
		#else
		#ifndef FAST_SWITCH_PREVIEW
		s32Ret = HI_MPI_VO_EnableChn(VoDev, VoChn);
		if(s32Ret != HI_SUCCESS)
		{
			//printf("%s HI_MPI_VO_EnableChn failed with %#x!\n", __func__, s32Ret);
			LIB_PRT("chn%d HI_MPI_VO_EnableChn failed with %#x!\n", VoChn, s32Ret);
			goto error6;
		}
		#endif
		#endif
		LIB_PRT("chn%d HI_MPI_VO_EnableChn with %#x!\n", VoChn, s32Ret);
	}

	/************************************************
		step5:  bind vou
    	*************************************************/
    	for (i = ARG_VO_NUM_MAX; i < 4+ARG_VO_NUM_MAX; i++)
	{
		VdChn = i;
		s32Ret = SAMLE_COMM_VDEC_BindVo(VdChn, SAMPLE_VO_LAYER_VPIP, VdChn);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("SAMLE_COMM_VDEC_BindVo chn%d failed!\n", VdChn);
			goto error7;
		}
		else
		{
			LIB_PRT("SAMLE_COMM_VDEC_BindVo chn%d success!\n", VdChn);
		}
    	}
	/*
	FILE *fpStrm=NULL;
	HI_U8 *pu8Buf = NULL;
	int len;
	
	fpStrm = fopen("/mnt/snap.jpg", "rb");
	if(fpStrm == NULL)
	{
		printf("%s: can't open file\n", __func__);
		perror("123");
		return 0;
	}
	pu8Buf = malloc(128*1024);
	len = fread(pu8Buf, 1, 128*1024, fpStrm);
	printf("%s: read file size: %d\n", __func__, len);
	
	VDEC_STREAM_S stStream;
	stStream.u64PTS  = 0;
	stStream.pu8Addr = pu8Buf;
	stStream.u32Len  = len; 
	stStream.bEndOfFrame  = HI_TRUE;
	stStream.bEndOfStream = HI_FALSE;       
	s32Ret=HI_MPI_VDEC_SendStream(ARG_VO_NUM_MAX, &stStream, 0);
        if (HI_SUCCESS != s32Ret)
        {
		printf("%s: HI_MPI_VDEC_SendStream failed, ret: 0x%x\n", __func__, s32Ret);	
        }
        */
	return 0;
	
error7:
	for (i = ARG_VO_NUM_MAX; i < 4+ARG_VO_NUM_MAX; i++)
	{
		SAMLE_COMM_VDEC_UnBindVo(i, SAMPLE_VO_LAYER_VPIP, i);
	}
error6:
	for (i = ARG_VO_NUM_MAX; i < 4+ARG_VO_NUM_MAX; i++)
	{
		#ifdef HI3535
		HI_MPI_VO_DisableChn(SAMPLE_VO_LAYER_VPIP, i);
		
		#else
		#ifndef FAST_SWITCH_PREVIEW
		HI_MPI_VO_DisableChn(VoDev, i);
		
		#endif
		#endif
	}
error5:
	#ifdef HI3535
	HI_MPI_VO_DisableVideoLayer(SAMPLE_VO_LAYER_VPIP);
	#else
	HI_MPI_VO_DisablePipLayer();
	#endif
	
error4:
	#ifdef HI3535
	HI_MPI_VO_UnBindVideoLayer(SAMPLE_VO_LAYER_VPIP, VoDev);
	#else
	HI_MPI_VO_PipLayerUnBindDev(VoDev);
	#endif
	
error3:
	for (i = ARG_VO_NUM_MAX; i < ARG_VO_NUM_MAX+4; i++)
	{
		SampleWaitDestroyVdecChn(i);
	}

out:
	return s32Ret;
}

int tl_snap_vdec_write(u8 *pbuf, u32 nSize)
{
	VDEC_STREAM_S stStream;
	HI_S32 s32Ret;
	
	stStream.u64PTS  = 0;
	stStream.pu8Addr = pbuf;
	stStream.u32Len  = nSize; 
	stStream.bEndOfFrame  = HI_TRUE;
	stStream.bEndOfStream = HI_FALSE;       
	s32Ret=HI_MPI_VDEC_SendStream(ARG_VO_NUM_MAX, &stStream, -1);
        if (HI_SUCCESS != s32Ret)
        {
		printf("%s: HI_MPI_VDEC_SendStream failed, ret: 0x%x\n", __func__, s32Ret);
        }
	return s32Ret;
}


