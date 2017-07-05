/******************************************************************************

	TL hslib common
	
	2008-12-16	created by kong
	2010-11-11	modify by andyrew

******************************************************************************/
#include "common.h"
#include "vio_common.h"
#include "lib_vdec.h"
#include "circlebuf.h"


extern int venc_init(void);	//come form lib_venc.c
extern int unsigned long long venc_get_sys_pts(int chn, unsigned long long pts);
extern void venc_update_time(int chn);

extern unsigned char GetPlayingStatus();//csp modify 20140406

#ifdef FAST_SWITCH_PREVIEW
extern int nvr_preview_vdec_open(int vdec_first_chn, int vdec_chn_num);
#endif

unsigned char is_decoder_techwell = 0;
unsigned char is_decoder_nvp = 0;
unsigned char is_decoder_rn631x = 0;

//csp modify 20140525
unsigned char no_audio_chip = 0;

struct lib_global_info lib_gbl_info;
struct lib_global_info *plib_gbl_info = NULL;

int g_client_id = 0;

void rebootSlaveByCmd(void)
{
	return;
}

/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/
#if 0
HI_S32 SAMPLE_COMM_VPSS_Start(HI_S32 s32FirstGrpIdx, HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt,VPSS_GRP_ATTR_S *pstVpssGrpAttr)

{
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN_ATTR_S stChnAttr;
	VPSS_GRP_PARAM_S stVpssParam;
	HI_S32 s32Ret;
	HI_S32 i, j;
	
	/*** Set Vpss Grp Attr ***/
	if(NULL == pstVpssGrpAttr)
	{
		stGrpAttr.u32MaxW = pstSize->u32Width;
		stGrpAttr.u32MaxH = pstSize->u32Height;
		#ifdef HI3535
		stGrpAttr.bDciEn = HI_FALSE;
		#else
		stGrpAttr.bDrEn = HI_FALSE;
		stGrpAttr.bDbEn = HI_FALSE;
		#endif
		stGrpAttr.bIeEn = HI_FALSE;
		stGrpAttr.bNrEn = HI_FALSE;//HI_TRUE : 800*600时HDMI闪烁
		stGrpAttr.bHistEn = HI_FALSE;
		stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;//VPSS_DIE_MODE_AUTO;
		stGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		//printf("SAMPLE_COMM_VPSS_Start u32MaxW=%d,u32MaxH=%d\n",stGrpAttr.u32MaxW,stGrpAttr.u32MaxH);
	}
	else
	{
		memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
	}
	
	for(i = s32FirstGrpIdx; i < s32FirstGrpIdx + s32GrpCnt; i++)
	{
		VpssGrp = i;
		
		/*** create vpss group ***/
		//#ifdef HI3531
		//s32Ret = HI_MPI_VPSS_CreatGrp(VpssGrp, &stGrpAttr);
		//#else
		s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
		//#endif
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("HI_MPI_VPSS_CreateGrp%d failed with %#x!\n", VpssGrp, s32Ret);
			return HI_FAILURE;
		}
		else
		{
			//LIB_PRT("HI_MPI_VPSS_CreateGrp%d success with %#x!\n", VpssGrp, s32Ret);
		}

		#if 0
		//csp modify 20150110
		s32Ret = HI_MPI_VPSS_DisableBackupFrame(VpssGrp);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("HI_MPI_VPSS_DisableBackupFrame%d failed with %#x!\n", VpssGrp, s32Ret);
			return HI_FAILURE;
		}
		else
		{
			//LIB_PRT("HI_MPI_VPSS_DisableBackupFrame%d success with %#x!\n", VpssGrp, s32Ret);
		}
		#endif
		
		/*** get vpss param ***/
		//#ifdef HI3531
		//s32Ret = HI_MPI_VPSS_GetParam(VpssGrp, 0, &stVpssParam);
		//#else
		s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
		//#endif
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			//return HI_FAILURE;
		}
		
		#ifdef HI3535
		#else
		stVpssParam.u32MotionThresh = 0;
		//stVpssParam.u32NrWforTsr = 4;
		stVpssParam.u32DiStrength = 0;
		#endif
		
		/*** set vpss param ***/
		//#ifdef HI3531
		//s32Ret = HI_MPI_VPSS_SetParam(VpssGrp, 0, &stVpssParam);
		//#else
		s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
		//#endif
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
		
		//csp modify
		int start_chn = 0;
		if(VpssGrp >= ARG_VI_NUM_MAX)
		{
			start_chn = VPSS_PRE0_CHN;
			s32ChnCnt = start_chn + 1;
		}
		else
		{
			//csp modify 20140406
			if(GetPlayingStatus())
			{
				//start_chn = 0;
				//s32ChnCnt = VPSS_PRE0_CHN + 1;
				start_chn = VPSS_PRE0_CHN;
				s32ChnCnt = start_chn + 1;
			}
			else
			{
				//csp modify 20140318
				start_chn = VPSS_LSTR_CHN;
				s32ChnCnt = VPSS_PRE0_CHN + 1;
			}
		}
		
		/*** enable vpss chn, with frame ***/
		for(j=start_chn; j<s32ChnCnt; j++)
		{
			VpssChn = j;
			
			s32Ret = HI_MPI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stChnAttr);
			
			/* Set Vpss Chn attr */
			if((j == 0) || (j == 4))
				stChnAttr.bSpEn = HI_FALSE;
			else
				stChnAttr.bSpEn = HI_TRUE;
			
			#ifdef HI3535
			stChnAttr.bBorderEn = HI_FALSE;//HI_TRUE;
			stChnAttr.stBorder.u32Color = 0xff00;
			stChnAttr.stBorder.u32LeftWidth = 2;
			stChnAttr.stBorder.u32RightWidth = 2;
			stChnAttr.stBorder.u32TopWidth = 2;
			stChnAttr.stBorder.u32BottomWidth = 2;
			#else
			stChnAttr.bFrameEn = HI_FALSE;//HI_TRUE;
			stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_LEFT] = 0xff00;
			stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_RIGHT] = 0xff00;
			stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_BOTTOM] = 0xff00;
			stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_TOP] = 0xff00;
			stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_LEFT] = 2;
			stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_RIGHT] = 2;
			stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_TOP] = 2;
			stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_BOTTOM] = 2;
			#endif
			
			s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
			if (s32Ret != HI_SUCCESS)
			{
				LIB_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				return HI_FAILURE;
			}
			
			#if 0
			VPSS_CHN_MODE_S stVpssMode;
			s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp,VpssChn,&stVpssMode);
			if(s32Ret != HI_SUCCESS)
			{
				printf("VPSS[%d,%d] HI_MPI_VPSS_GetChnMode failed\n",VpssGrp,VpssChn);
				//return s32Ret;
			}
			
			printf("VPSS[%d,%d] stVpssMode:(%d,%d,%d,%d)\n",VpssGrp,VpssChn,stVpssMode.u32Width,stVpssMode.u32Height,stVpssMode.enChnMode,stVpssMode.bDouble);
			
			stVpssMode.enChnMode = VPSS_CHN_MODE_AUTO;
			stVpssMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
			stVpssMode.u32Width = pstSize->u32Width;
			stVpssMode.u32Height = pstSize->u32Height;
			stVpssMode.bDouble = HI_TRUE;
			s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp,VpssChn,&stVpssMode);
			if(s32Ret != HI_SUCCESS)
			{
				printf("HI_MPI_VPSS_SetChnMode failed:0x%08x\n",s32Ret);
				return s32Ret;
			}
			#endif
			
			s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
			if (s32Ret != HI_SUCCESS)
			{
				LIB_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
				return HI_FAILURE;
			}
		}
		
		/*** start vpss group ***/
		s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}
	}
	
	return HI_SUCCESS;
}

#else
HI_S32 SAMPLE_COMM_VPSS_Start(HI_S32 s32FirstGrpIdx, HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt,VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN_ATTR_S stChnAttr;
	VPSS_GRP_PARAM_S stVpssParam;
	HI_S32 s32Ret;
	HI_S32 i, j;
	
	//pstSize->u32Width = VDEC_MAX_W;
	//pstSize->u32Height = VDEC_MAX_H;
	
	/*** Set Vpss Grp Attr ***/
	if(NULL == pstVpssGrpAttr)
	{
		stGrpAttr.u32MaxW = pstSize->u32Width;
		stGrpAttr.u32MaxH = pstSize->u32Height;
		#ifdef HI3535
		stGrpAttr.bDciEn = HI_FALSE;
		#else
		stGrpAttr.bDrEn = HI_FALSE;
		stGrpAttr.bDbEn = HI_FALSE;
		#endif
		stGrpAttr.bIeEn = HI_FALSE;
		stGrpAttr.bNrEn = HI_FALSE;//HI_TRUE;//HI_FALSE;//很关键,可以解决Hi3535主板VGA分辨率800x600下HDMI显示闪烁的问题//csp modify 20150110
		stGrpAttr.bHistEn = HI_FALSE;
		stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;//VPSS_DIE_MODE_AUTO;
		stGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		//printf("SAMPLE_COMM_VPSS_Start u32MaxW=%d,u32MaxH=%d\n",stGrpAttr.u32MaxW,stGrpAttr.u32MaxH);
	}
	else
	{
		memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
	}
	
	for(i = s32FirstGrpIdx; i < s32FirstGrpIdx + s32GrpCnt; i++)
	{
		VpssGrp = i;
		
		/*** create vpss group ***/
		//#ifdef HI3531
		//s32Ret = HI_MPI_VPSS_CreatGrp(VpssGrp, &stGrpAttr);
		//#else
		s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
		//#endif
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("HI_MPI_VPSS_CreateGrp%d failed with %#x!\n", VpssGrp, s32Ret);
			return HI_FAILURE;
		}
		else
		{
			//LIB_PRT("HI_MPI_VPSS_CreateGrp%d success with %#x!\n", VpssGrp, s32Ret);
		}
		
		#if 0
		//csp modify 20150110
		s32Ret = HI_MPI_VPSS_DisableBackupFrame(VpssGrp);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("HI_MPI_VPSS_DisableBackupFrame%d failed with %#x!\n", VpssGrp, s32Ret);
			return HI_FAILURE;
		}
		else
		{
			//LIB_PRT("HI_MPI_VPSS_DisableBackupFrame%d success with %#x!\n", VpssGrp, s32Ret);
		}
		#endif
		
		/*** get vpss param ***/
		//#ifdef HI3531
		//s32Ret = HI_MPI_VPSS_GetParam(VpssGrp, 0, &stVpssParam);
		//#else
		s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
		//#endif
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			//return HI_FAILURE;
		}
		
		#ifdef HI3535
		#else
		stVpssParam.u32MotionThresh = 0;
		//stVpssParam.u32NrWforTsr = 4;
		stVpssParam.u32DiStrength = 0;
		#endif
		
		/*** set vpss param ***/
		//#ifdef HI3531
		//s32Ret = HI_MPI_VPSS_SetParam(VpssGrp, 0, &stVpssParam);
		//#else
		s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
		//#endif
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
		
		//csp modify
		int start_chn = 0;
		if(VpssGrp >= ARG_VI_NUM_MAX)
		{
			start_chn = VPSS_PRE0_CHN;
			s32ChnCnt = start_chn + 1;
		}
		else
		{
			//csp modify 20140406
			if(GetPlayingStatus())
			{
				//start_chn = 0;
				//s32ChnCnt = VPSS_PRE0_CHN + 1;
				start_chn = VPSS_PRE0_CHN;
				s32ChnCnt = start_chn + 1;
			}
			else
			{
				//csp modify 20140318
				start_chn = VPSS_LSTR_CHN;
				s32ChnCnt = VPSS_PRE0_CHN + 1;
			}
		}
		
		/*** enable vpss chn, with frame ***/
		for(j=start_chn; j<s32ChnCnt; j++)
		{
			VpssChn = j;
			
			s32Ret = HI_MPI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stChnAttr);
			
			/* Set Vpss Chn attr */
			if((j == 0) || (j == 4))
				stChnAttr.bSpEn = HI_FALSE;
			else
				stChnAttr.bSpEn = HI_FALSE;//HI_TRUE;
			
			#ifdef HI3535
			stChnAttr.bSpEn = HI_FALSE;//csp modify 20150110
			stChnAttr.bBorderEn = HI_FALSE;//HI_TRUE;
			stChnAttr.stBorder.u32Color = 0xff00;
			stChnAttr.stBorder.u32LeftWidth = 2;
			stChnAttr.stBorder.u32RightWidth = 2;
			stChnAttr.stBorder.u32TopWidth = 2;
			stChnAttr.stBorder.u32BottomWidth = 2;
			#else
			stChnAttr.bFrameEn = HI_FALSE;//HI_TRUE;
			stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_LEFT] = 0xff00;
			stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_RIGHT] = 0xff00;
			stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_BOTTOM] = 0xff00;
			stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_TOP] = 0xff00;
			stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_LEFT] = 2;
			stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_RIGHT] = 2;
			stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_TOP] = 2;
			stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_BOTTOM] = 2;
			#endif
			
			s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
			if (s32Ret != HI_SUCCESS)
			{
				LIB_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				return HI_FAILURE;
			}
			
			#if 1//csp modify 20150110
			VPSS_CHN_MODE_S stVpssMode;
			s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp,VpssChn,&stVpssMode);
			if(s32Ret != HI_SUCCESS)
			{
				printf("VPSS[%d,%d] HI_MPI_VPSS_GetChnMode failed\n",VpssGrp,VpssChn);
				//return s32Ret;
			}
			
			//csp modify 20150110
			//printf("VPSS[%d,%d] stVpssMode:(%d,%d,%d,%d)\n",VpssGrp,VpssChn,stVpssMode.u32Width,stVpssMode.u32Height,stVpssMode.enChnMode,stVpssMode.bDouble);
			
			stVpssMode.enChnMode = VPSS_CHN_MODE_AUTO;
			stVpssMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
			stVpssMode.u32Width = pstSize->u32Width;
			stVpssMode.u32Height = pstSize->u32Height;
			stVpssMode.bDouble = HI_FALSE;//HI_TRUE;//ddr down//csp modify 20150110
			s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp,VpssChn,&stVpssMode);
			if(s32Ret != HI_SUCCESS)
			{
				printf("HI_MPI_VPSS_SetChnMode failed:0x%08x\n",s32Ret);
				return s32Ret;
			}
			#endif
			
			s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
			if (s32Ret != HI_SUCCESS)
			{
				LIB_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
				return HI_FAILURE;
			}
		}
		
		/*** start vpss group ***/
		s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}
	}
	
	return HI_SUCCESS;
}
#endif

/*****************************************************************************
* function : disable vpss dev
*****************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_Stop(HI_S32 s32FirstGrpIdx, HI_S32 s32GrpCnt, HI_S32 s32ChnCnt)
{
	HI_S32 i, j;
	HI_S32 s32Ret = HI_SUCCESS;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	
	for(i = s32FirstGrpIdx; i < s32FirstGrpIdx + s32GrpCnt; i++)
	{
		VpssGrp = i;
		
		#if 0
		s32Ret = HI_MPI_VPSS_ResetGrp(VpssGrp);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			//return HI_FAILURE;
		}
		#endif
		
		s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
		
		for(j=0; j<s32ChnCnt; j++)
		{
			VpssChn = j;
			s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
			if (s32Ret != HI_SUCCESS)
			{
				LIB_PRT("failed with %#x!\n", s32Ret);
				return HI_FAILURE;
			}
		}
		
		s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	}
	
	return HI_SUCCESS;
}

HI_S32 SAMPLE_EnableVpss(int firstVpssGrp, int VpssGrpcnt)
{
	VPSS_GRP VpssGrp;
	HI_S32 s32Ret;
	
	for(VpssGrp = firstVpssGrp; VpssGrp < VpssGrpcnt + firstVpssGrp; VpssGrp++)
	{
		s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
		if (s32Ret != HI_SUCCESS)
		{
			LIB_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}
	}
	
	return HI_SUCCESS;
}

HI_S32 SAMPLE_StartVpss(SIZE_S size, int chncnt)
{
	SIZE_S stSize;
	HI_S32 s32Ret;
	
	int i;
	for(i = 0; i < chncnt; i++)
	{
		stSize.u32Width = size.u32Width;
		stSize.u32Height = size.u32Height;
		
		s32Ret = SAMPLE_COMM_VPSS_Start(i, 1, &stSize, 5, NULL);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("Start Vpss failed!\n");
			return HI_FAILURE;
		}
	}
	
	return HI_SUCCESS;
}

static HI_S32 SAMPLE_InitMPP(void)
{
	VB_CONF_S stVbConf;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32AlignWidth = 16;
	MPP_SYS_CONF_S stSysConf = {0};
	
	/******************************************
	step  1: init global  variable 
	******************************************/
	memset(&stVbConf,0,sizeof(VB_CONF_S));
	
	if(TL_BOARD_TYPE_NR2116 == TL_HSLIB_TYPE)
	{
	#ifdef HI3535//csp modify 20150110
		printf("NR3516 config$$$$$$$$$$$$$$$$$$$$$$$$\n");
		//stVbConf.u32MaxPoolCnt = 2;
		//stVbConf.astCommPool[0].u32BlkSize = 1920*1200*3/2;
		//stVbConf.astCommPool[0].u32BlkCnt = 8;
		//stVbConf.astCommPool[1].u32BlkSize = 768*576*3/2;
		//stVbConf.astCommPool[1].u32BlkCnt = 64;
		//for venc
		stVbConf.u32MaxPoolCnt = 1;
		stVbConf.astCommPool[0].u32BlkSize = 704*576*3/2;//352*288*3/2;
		stVbConf.astCommPool[0].u32BlkCnt = 8;//12;
		//stVbConf.u32MaxPoolCnt = 2;
		//stVbConf.astCommPool[0].u32BlkSize = 1920*1200*3/2;
		//stVbConf.astCommPool[0].u32BlkCnt = 12;
		//stVbConf.astCommPool[1].u32BlkSize = 768*576*3/2;
		//stVbConf.astCommPool[1].u32BlkCnt = 16;
	#else
		printf("NR2116 config$$$$$$$$$$$$$$$$$$$$$$$$\n");
		stVbConf.u32MaxPoolCnt = 4;
		stVbConf.astCommPool[0].u32BlkSize = 720*576*3/2;
		stVbConf.astCommPool[0].u32BlkCnt = 4;
	#endif
	}
	else if(TL_BOARD_TYPE_NR3116 == TL_HSLIB_TYPE)
	{
		printf("NR3116 config$$$$$$$$$$$$$$$$$$$$$$$$\n");
		stVbConf.u32MaxPoolCnt = 4;
		stVbConf.astCommPool[0].u32BlkSize = 720*576*3/2;
		stVbConf.astCommPool[0].u32BlkCnt = 4;
	}
	else if(TL_BOARD_TYPE_NR3132 == TL_HSLIB_TYPE)
	{
		printf("NR3132 config$$$$$$$$$$$$$$$$$$$$$$$$\n");
		stVbConf.u32MaxPoolCnt = 4;
		stVbConf.astCommPool[0].u32BlkSize = 720*576*3/2;
		stVbConf.astCommPool[0].u32BlkCnt = 4;
	}
	else if(TL_BOARD_TYPE_NR3124 == TL_HSLIB_TYPE)
	{
		printf("NR3124 config$$$$$$$$$$$$$$$$$$$$$$$$\n");
		stVbConf.u32MaxPoolCnt = 4;
		stVbConf.astCommPool[0].u32BlkSize = 720*576*3/2;
		stVbConf.astCommPool[0].u32BlkCnt = 4;
	}
	else if(TL_BOARD_TYPE_NR1004 == TL_HSLIB_TYPE)
	{
		printf("NR1004 config$$$$$$$$$$$$$$$$$$$$$$$$\n");
		stVbConf.u32MaxPoolCnt = 4;
		stVbConf.astCommPool[0].u32BlkSize = 720*576*3/2;
		stVbConf.astCommPool[0].u32BlkCnt = 4;
	}
	else if(TL_BOARD_TYPE_NR1008 == TL_HSLIB_TYPE)
	{
		printf("NR1008 config$$$$$$$$$$$$$$$$$$$$$$$$\n");
		stVbConf.u32MaxPoolCnt = 4;
		stVbConf.astCommPool[0].u32BlkSize = 720*576*3/2;
		stVbConf.astCommPool[0].u32BlkCnt = 4;
	}
	else
	{
		printf("unknown nvr board type\n");
		stVbConf.u32MaxPoolCnt = 4;
		stVbConf.astCommPool[0].u32BlkSize = 720*576*3/2;
		stVbConf.astCommPool[0].u32BlkCnt = 4;
	}
	
	/******************************************
	step 2: mpp system init. 
	******************************************/
	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();
	
	s32Ret = HI_MPI_VB_SetConf(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_VB_SetConf failed!\n");
	    return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VB_Init();
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_VB_Init failed!\n");
	    return HI_FAILURE;
	}
	
	stSysConf.u32AlignWidth = u32AlignWidth;
	s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_SYS_SetConf failed\n");
	    return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_SYS_Init();
	if (HI_SUCCESS != s32Ret)
	{
	    LIB_PRT("HI_MPI_SYS_Init failed!\n");
	    return HI_FAILURE;
	}
	
	return HI_SUCCESS;
}


#include "lib_vdec.h"

typedef struct
{
	int chn;
	venc_frame_type_e type;
	unsigned long long pts;
	unsigned int len;
	unsigned short width;
	unsigned short height;
	unsigned long long local_pts;
}SVencFrameHeader;

extern unsigned int getTimeStamp();


//#define IPC_PROTOCOL_TEST //test record disk error by yaogang 20170222

//#define	UsePreviewQueue
#undef	UsePreviewQueue

#ifdef	UsePreviewQueue

//每个预览通道一个线程，原先一个线程处理多个预览通道会有相互影响
//每通道线程处理主/次两路码流，同时接收处理两路的消息
#define	PreviewFxnPerChn

//yaogang modify 20170218 in shanghai
//add TypeChnBufException

typedef enum  {
	TypeStartChn,
    TypeStopChn,
    TypeChnBufException, //预览缓冲区异常，表现为写缓冲失败
} PreviewFxnMsgType;

typedef struct
{
	int chn;
	PreviewFxnMsgType type;
}SPreviewFxnMsg;//8 byte

typedef enum  {
	StatusChnStart,
	StatusChnWaitIframe,	//等待I帧，结束后队头就是I帧
	StatusChnWaitBuf,		//等待通道队列缓冲完成
	StatusChnPreview,		//通道正常刷新帧到预览
	StatusChnWillBeStop,
    	StatusChnStop,
    	StatusChnRst,			//通道出错重置	
} PreviewChnStatus;

//通道队列中最新写入的一帧数据的IPC时间戳(IPC时间) us
static volatile unsigned long long QueueLastIPCPtsChn[ARG_CHN_MAX];

s32 ResetPreviewQueue(int chn)
{
	s32 ret = 0;
	
	pthread_mutex_lock(&plib_gbl_info->preview_queue_lock[chn]);

	QueueLastIPCPtsChn[chn] = 0;
	ret = ResetCircleBuf(&plib_gbl_info->preview_frame_queue[chn]);
		
	pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
	
	return ret;
}

s32 ReadFrameFromPreviewQueuePts(int chn, unsigned char *in_buf, unsigned int in_len, real_stream_s *stream, unsigned long long *pLocalPts)
{
	if((chn >= ARG_VI_NUM_MAX*2) || (in_buf == NULL) || (in_len <= 0) || (stream == NULL))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return -1;
	}
	
	SVencFrameHeader header;
	memset(&header, 0, sizeof(header));
	
	pthread_mutex_lock(&plib_gbl_info->preview_queue_lock[chn]);
	
	s32 ret = 0;
	ret = ReadDataFromBuf(&plib_gbl_info->preview_frame_queue[chn], (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)
	{
		//fprintf(stderr, "function: %s 1 chn%d\n", __FUNCTION__, chn);
		pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
		return -1;
	}
	
	//printf("chn%d frame len:%u\n", header.chn, header.len);
	
	if(header.len > in_len)
	{
		printf("%s header.len > in_len error\n", __func__); //yaogang modify 20150304
		ret = SkipCircleBuf(&plib_gbl_info->preview_frame_queue[chn], 0, 1, header.len);
		if(0 != ret)
		{
			ResumeCircleBufToPast(&plib_gbl_info->preview_frame_queue[chn], 0, 1);
			pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
			return -1;
		}
		pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
		return -1;
	}
	
	ret = ReadDataFromBuf(&plib_gbl_info->preview_frame_queue[chn], in_buf, header.len);
	if(0 != ret)
	{
		fprintf(stderr, "function: %s 2 chn%d\n", __FUNCTION__, chn);
		ResumeCircleBufToPast(&plib_gbl_info->preview_frame_queue[chn], 0, 1);
		pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
		return -1;
	}
	
	pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);

	if (chn != header.chn)
	{
		printf("function: %s chn%d != header.chn%d\n", __FUNCTION__, chn ,header.chn);
	}
	
	stream->chn = header.chn;
	stream->data = in_buf;
	stream->len = header.len;
	stream->pts = header.pts;//IpcPts
	stream->frame_type = header.type;

	*pLocalPts = header.local_pts;
	
	return 0;
}

//注意检测返回值  尤其是1004(EM_REC_BUF_NO_DATA)
s32 ReadFrameFromPreviewQueue(int chn, unsigned char *in_buf, unsigned int in_len, real_stream_s *stream)
{
	if((chn >= ARG_VI_NUM_MAX*2) || (in_buf == NULL) || (in_len <= 0) || (stream == NULL))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return -1;
	}
	
	SVencFrameHeader header;
	memset(&header, 0, sizeof(header));
	
	pthread_mutex_lock(&plib_gbl_info->preview_queue_lock[chn]);
	
	s32 ret = 0;
	ret = ReadDataFromBuf(&plib_gbl_info->preview_frame_queue[chn], (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)
	{
		if (EM_REC_BUF_NO_DATA != ret)
			fprintf(stderr, "function: %s 1 chn%d, ret: %d\n", __FUNCTION__, chn, ret);
		pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
		return ret;
	}
	
	//printf("chn%d frame len:%u\n", header.chn, header.len);
	
	if(header.len > in_len)
	{
		printf("%s header.len > in_len error\n", __func__); //yaogang modify 20150304
		ret = SkipCircleBuf(&plib_gbl_info->preview_frame_queue[chn], 0, 1, header.len);
		if(0 != ret)
		{
			ResumeCircleBufToPast(&plib_gbl_info->preview_frame_queue[chn], 0, 1);
			pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
			return -1;
		}
		pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
		return -1;
	}
	
	ret = ReadDataFromBuf(&plib_gbl_info->preview_frame_queue[chn], in_buf, header.len);
	if(0 != ret)
	{
		fprintf(stderr, "function: %s 2 chn%d\n", __FUNCTION__, chn);
		ResumeCircleBufToPast(&plib_gbl_info->preview_frame_queue[chn], 0, 1);
		pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
		return ret;
	}
	
	pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);

	if (chn != header.chn)
	{
		printf("function: %s chn%d != header.chn%d\n", __FUNCTION__, chn ,header.chn);
	}
	
	stream->chn = header.chn;
	stream->data = in_buf;
	stream->len = header.len;
	stream->pts = header.pts;
	stream->frame_type = header.type;
	
	return 0;
}

//yaogang modify 20170218 in shanghai
//add TypeChnBufException
int WriteFrameToPreviewQueue(real_stream_s *stream)
{
	SVencFrameHeader header;
	unsigned int t;
	int chn = stream->chn;
	s32 ret = 0;

	t = getTimeStamp(); //ms
	/*
	if (stream->chn < 16)
	{
		printf("%s pts: %u\n", __func__, t);

	}
	*/
	header.local_pts = t;
	header.local_pts *= 1000; //us
	header.chn = stream->chn;
	header.type = stream->frame_type;
	header.pts = stream->pts;
	header.len = stream->len;
	header.width = stream->width;
	header.height = stream->height;
	
	pthread_mutex_lock(&plib_gbl_info->preview_queue_lock[chn]);
	
	ret = WriteDataToBuf(&plib_gbl_info->preview_frame_queue[chn], (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)//failed
	{
		printf("error: %s chn: %d write header to buf failed, ret: %d\n", __func__, chn, ret);
	}
	else //success
	{
		ret = WriteDataToBuf(&plib_gbl_info->preview_frame_queue[chn], stream->data, stream->len);
		if(0 != ret) //failed
		{
			printf("error: %s chn: %d write frame to buf failed, ret: %d\n", __func__, chn, ret);
			ResumeCircleBufToPast(&plib_gbl_info->preview_frame_queue[chn], 1, 0);
		}
		else //success, update last frame time
		{
			//通道队列中最新写入的一帧数据的IPC时间戳(IPC时间) 
			QueueLastIPCPtsChn[stream->chn] = stream->pts;
		}
	}
	
	pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);

	if (0 != ret)//failed
	{
	#ifdef	UsePreviewQueue
		int real_chn = chn;
		if(real_chn >= ARG_VI_NUM_MAX)
		{
			real_chn -= ARG_VI_NUM_MAX;
		}
		
		//send msg to preview thread
		SPreviewFxnMsg PreviewMsg;
		memset(&PreviewMsg, 0, sizeof(PreviewMsg));
		
		PreviewMsg.chn = chn;
		PreviewMsg.type = TypeChnBufException;
		printf("%s send MsgQueue[%d] msg_chn: %d, msg_type: %d\n", 
			__func__, real_chn, PreviewMsg.chn, PreviewMsg.type);

		#ifdef	PreviewFxnPerChn
		ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueueChn[real_chn], (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
		if (ret)
		{
			printf("error: %s send MsgQueue[%d] falied, ret: %d, msg_chn: %d, msg_type: %d\n", 
				__func__, real_chn, ret, PreviewMsg.chn, PreviewMsg.type);
		}
		#else
		ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueue, (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
		if (ret)
		{
			printf("%s 2 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
		}
		#endif
	#endif	
	
		return -1;
	}
	
	return 0;
}

#endif


int WriteFrameToVencMainQueue(real_stream_s *stream)
{
	#if 1
	venc_stream_s out_stream;
	out_stream.chn = stream->chn;
	out_stream.data = stream->data;
	out_stream.len = stream->len;
	out_stream.pts = venc_get_sys_pts(stream->chn, stream->pts);
	out_stream.type = stream->frame_type;
	out_stream.rsv = 0;//stream->rsv;
	out_stream.width = stream->width;
	out_stream.height = stream->height;
	
	if(plib_gbl_info->pMainStreamCB != NULL)
	{
		plib_gbl_info->pMainStreamCB(&out_stream);
	}
	#else
	SVencFrameHeader header;
	header.chn = stream->chn;
	header.type = stream->type;
	header.pts = stream->pts;
	header.len = stream->len;
	header.width = stream->width;
	header.height = stream->height;
	
	pthread_mutex_lock(&plib_gbl_info->venc_main_queue_lock);
	
	s32 ret = WriteDataToBuf(&plib_gbl_info->venc_main_frame_queue, (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)
	{
		if(stream->chn == 0) fprintf(stderr, "%s: chn%d write header failed\n", __FUNCTION__, stream->chn);
		pthread_mutex_unlock(&plib_gbl_info->venc_main_queue_lock);
		return -1;
	}
	ret = WriteDataToBuf(&plib_gbl_info->venc_main_frame_queue, stream->data, stream->len);
	if(0 != ret)
	{
		if(stream->chn == 0) fprintf(stderr, "%s: chn%d write data failed\n", __FUNCTION__, stream->chn);
		ResumeCircleBufToPast(&plib_gbl_info->venc_main_frame_queue, 1, 0);
		pthread_mutex_unlock(&plib_gbl_info->venc_main_queue_lock);
		return -1;
	}
	
	pthread_mutex_unlock(&plib_gbl_info->venc_main_queue_lock);
	#endif
	
	return 0;
}

int WriteFrameToVencSubQueue(real_stream_s *stream)
{
	#if 1
	venc_stream_s out_stream;
	out_stream.chn = stream->chn - ARG_VI_NUM_MAX;
	out_stream.data = stream->data;
	out_stream.len = stream->len;
	out_stream.pts = stream->pts;
	out_stream.type = stream->frame_type;
	out_stream.rsv = 0;//stream->rsv;
	out_stream.width = stream->width;
	out_stream.height = stream->height;
	
	//csp modify 20140318
	#ifdef P2P_SUB_STREAM
	if(plib_gbl_info->venc[out_stream.chn].venc_min_start)
	{
		if(plib_gbl_info->pSubStreamCB != NULL)
		{
			plib_gbl_info->pSubStreamCB(&out_stream);
		}
	}
	//csp modify 20140406
	//printf("WriteFrameToVencSubQueue chn=%d venc_third_start=%d,last_pre_mode=%d,frametype=%d\n",out_stream.chn,plib_gbl_info->venc[out_stream.chn].venc_third_start,plib_gbl_info->last_pre_mode,out_stream.type);
	if(plib_gbl_info->venc[out_stream.chn].venc_third_start && plib_gbl_info->last_pre_mode == PREVIEW_CLOSE/* && out_stream.type == FRAME_TYPE_I*/)
	{
		//printf("chn%d third stream here...\n",out_stream.chn);
		if(plib_gbl_info->pThirdStreamCB != NULL)
		{
			plib_gbl_info->pThirdStreamCB(&out_stream);
		}
	}
	#else
	if(plib_gbl_info->pSubStreamCB != NULL)
	{
		plib_gbl_info->pSubStreamCB(&out_stream);
	}
	#endif
	#else
	SVencFrameHeader header;
	header.chn = stream->chn - ARG_VI_NUM_MAX;
	header.type = stream->type;
	header.pts = stream->pts;
	header.len = stream->len;
	header.width = stream->width;
	header.height = stream->height;
	
	pthread_mutex_lock(&plib_gbl_info->venc_sub_queue_lock);
	
	s32 ret = WriteDataToBuf(&plib_gbl_info->venc_sub_frame_queue, (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s [1]\n", __FUNCTION__);
		pthread_mutex_unlock(&plib_gbl_info->venc_sub_queue_lock);
		return -1;
	}
	ret = WriteDataToBuf(&plib_gbl_info->venc_sub_frame_queue, stream->data, stream->len);
	if(0 != ret)
	{
		fprintf(stderr, "function: %s [2]\n", __FUNCTION__);
		ResumeCircleBufToPast(&plib_gbl_info->venc_sub_frame_queue, 1, 0);
		pthread_mutex_unlock(&plib_gbl_info->venc_sub_queue_lock);
		return -1;
	}
	
	pthread_mutex_unlock(&plib_gbl_info->venc_sub_queue_lock);
	#endif
	
	return 0;
}

s32 ReadFrameFromVencMainQueue(unsigned char *in_buf, unsigned int in_len, real_stream_s *stream)
{
	#if 1
	return -1;
	#else
	if((in_buf == NULL) || (in_len <= 0) || (stream == NULL))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return -1;
	}
	
	SVencFrameHeader header;
	memset(&header, 0, sizeof(header));
	
	pthread_mutex_lock(&plib_gbl_info->venc_main_queue_lock);
	
	s32 ret = 0;
	ret = ReadDataFromBuf(&plib_gbl_info->venc_main_frame_queue, (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s [1]\n", __FUNCTION__);
		pthread_mutex_unlock(&plib_gbl_info->venc_main_queue_lock);
		return -1;
	}
	
	//printf("chn%d frame len:%u\n", header.chn, header.len);
	
	if(header.len > in_len)
	{
		fprintf(stderr, "%s: buffer overflow,skip...\n", __FUNCTION__);
		ret = SkipCircleBuf(&plib_gbl_info->venc_main_frame_queue, 0, 1, header.len);
		if(0 != ret)
		{
			fprintf(stderr, "%s: skip buffer failed\n", __FUNCTION__);
			ResumeCircleBufToPast(&plib_gbl_info->venc_main_frame_queue, 0, 1);
			pthread_mutex_unlock(&plib_gbl_info->venc_main_queue_lock);
			return -1;
		}
		pthread_mutex_unlock(&plib_gbl_info->venc_main_queue_lock);
		return -1;
	}
	
	ret = ReadDataFromBuf(&plib_gbl_info->venc_main_frame_queue, in_buf, header.len);
	if(0 != ret)
	{
		fprintf(stderr, "function: %s [2]\n", __FUNCTION__);
		ResumeCircleBufToPast(&plib_gbl_info->venc_main_frame_queue, 0, 1);
		pthread_mutex_unlock(&plib_gbl_info->venc_main_queue_lock);
		return -1;
	}
	
	pthread_mutex_unlock(&plib_gbl_info->venc_main_queue_lock);
	
	stream->chn = header.chn;
	stream->data = in_buf;
	stream->len = header.len;
	stream->pts = header.pts;
	stream->type = header.type;
	
	return 0;
	#endif
}

s32 ReadFrameFromVencSubQueue(unsigned char *in_buf, unsigned int in_len, real_stream_s *stream)
{
	#if 1
	return -1;
	#else
	if((in_buf == NULL) || (in_len <= 0) || (stream == NULL))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return -1;
	}
	
	SVencFrameHeader header;
	memset(&header, 0, sizeof(header));
	
	pthread_mutex_lock(&plib_gbl_info->venc_sub_queue_lock);
	
	s32 ret = 0;
	ret = ReadDataFromBuf(&plib_gbl_info->venc_sub_frame_queue, (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s [1]\n", __FUNCTION__);
		pthread_mutex_unlock(&plib_gbl_info->venc_sub_queue_lock);
		return -1;
	}
	
	//printf("chn%d frame len:%u\n", header.chn, header.len);
	
	if(header.len > in_len)
	{
		ret = SkipCircleBuf(&plib_gbl_info->venc_sub_frame_queue, 0, 1, header.len);
		if(0 != ret)
		{
			ResumeCircleBufToPast(&plib_gbl_info->venc_sub_frame_queue, 0, 1);
			pthread_mutex_unlock(&plib_gbl_info->venc_sub_queue_lock);
			return -1;
		}
		pthread_mutex_unlock(&plib_gbl_info->venc_sub_queue_lock);
		return -1;
	}
	
	ret = ReadDataFromBuf(&plib_gbl_info->venc_sub_frame_queue, in_buf, header.len);
	if(0 != ret)
	{
		fprintf(stderr, "function: %s [2]\n", __FUNCTION__);
		ResumeCircleBufToPast(&plib_gbl_info->venc_sub_frame_queue, 0, 1);
		pthread_mutex_unlock(&plib_gbl_info->venc_sub_queue_lock);
		return -1;
	}
	
	pthread_mutex_unlock(&plib_gbl_info->venc_sub_queue_lock);
	
	stream->chn = header.chn;
	stream->data = in_buf;
	stream->len = header.len;
	stream->pts = header.pts;
	stream->type = header.type;
	
	return 0;
	#endif
}

extern unsigned char is_nvr_preview_chn_open(int chn);
extern int nvr_preview_vdec_write(int chn, vdec_stream_s *pin_stream);

int GetPreviewQueueFirstFrameType(int chn, venc_frame_type_e *pFrameType)//得到对列头帧类型--I帧
{
	s32 ret = 0;
	SVencFrameHeader header;
	memset(&header, 0, sizeof(header));
	
	pthread_mutex_lock(&plib_gbl_info->preview_queue_lock[chn]);
	
	ret = ReadDataFromBuf(&plib_gbl_info->preview_frame_queue[chn], (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)
	{
		//if (EM_REC_BUF_NO_DATA != ret)
		if (ret != EM_REC_BUF_NO_DATA) //EM_REC_BUF_NO_DATA
			fprintf(stderr, "function: %s [1]\n", __FUNCTION__);
		
		pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
		return ret;
	}

	*pFrameType = header.type;
	
	ResumeCircleBufToPast(&plib_gbl_info->preview_frame_queue[chn], 0, 1);

	pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
	return 0;
	
}
int GetPreviewQueueFirstFramePTS(int chn, unsigned long long *pIpcPts, unsigned long long *pLocalPts) // us
{
	s32 ret = 0;
	SVencFrameHeader header;
	memset(&header, 0, sizeof(header));
	
	pthread_mutex_lock(&plib_gbl_info->preview_queue_lock[chn]);
	
	ret = ReadDataFromBuf(&plib_gbl_info->preview_frame_queue[chn], (u8 *)&header, sizeof(SVencFrameHeader));
	if(0 != ret)
	{
		if (ret != EM_REC_BUF_NO_DATA) //EM_REC_BUF_NO_DATA
			fprintf(stderr, "function: %s [1]\n", __FUNCTION__);
		
		pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
		return ret;
	}

	*pIpcPts = header.pts;
	*pLocalPts = header.local_pts;
	
	ResumeCircleBufToPast(&plib_gbl_info->preview_frame_queue[chn], 0, 1);

	pthread_mutex_unlock(&plib_gbl_info->preview_queue_lock[chn]);
	return 0;
}

//判断给定通道当前是否在前台显示
//调用时要先PREVIEW_LOCK()
int is_chn_current_show(int vo_chn)
{
	int ret;
	pre_mode_e last_pre_mode;
	int start_chn, end_chn;
		
	//PREVIEW_LOCK();
	
	last_pre_mode = lib_gbl_info.last_pre_mode;
	start_chn = lib_gbl_info.last_mode_arg;
	
	//PREVIEW_UNLOCK();

	//printf("%s vo_chn%d, last_pre_mode: %d, start_chn: %d\n", __func__, vo_chn, last_pre_mode, start_chn);
	
	ret = 0;
	switch(last_pre_mode)
	{
		case PREVIEW_1D1:
			if (vo_chn == start_chn)
			{
				ret = 1;
			}
			break;
		case PREVIEW_4CIF:
			end_chn = start_chn+4-1;//0-3
			if (end_chn >= ARG_VI_NUM_MAX)
			{
				printf("%s last_pre_mode: %d, start_chn: %d, end_chn >= ARG_VI_NUM_MAX\n", __func__, last_pre_mode, start_chn);
			}
				
			if (vo_chn >= start_chn && vo_chn <= end_chn)
			{
				ret = 1;
			}
			break;
		case PREVIEW_9CIF:
			end_chn = start_chn+9-1;//0-8
			if (end_chn >= ARG_VI_NUM_MAX)
			{
				printf("%s last_pre_mode: %d, start_chn: %d, end_chn >= ARG_VI_NUM_MAX\n", __func__, last_pre_mode, start_chn);
			}
			
			if (vo_chn >= start_chn && vo_chn <= end_chn)
			{
				ret = 1;
			}
			break;
		case PREVIEW_16CIF:
			end_chn = start_chn+16-1;//0-15
			if (end_chn >= ARG_VI_NUM_MAX)
			{
				printf("%s last_pre_mode: %d, start_chn: %d, end_chn >= ARG_VI_NUM_MAX\n", __func__, last_pre_mode, start_chn);
			}
			
			if (vo_chn >= start_chn && vo_chn <= end_chn)
			{
				ret = 1;
			}
			break;
		default:
			printf("%s last_pre_mode: %d unsupported\n", __func__, last_pre_mode);
	}

	return ret;
}

#ifdef	UsePreviewQueue
/*********************for test*********************/
static unsigned int cnt, yg_pts;
#define FRAME_BUF	(768*1024)

#ifdef	PreviewFxnPerChn
void *NVRPreviewFxnChn(void *arg)
{
	int FxnChn = (int)arg;
	printf("NVRPreviewFxnChn%d, pid:%u, ARG_VI_NUM_MAX: %d\n", FxnChn, (u32)pthread_self(), ARG_VI_NUM_MAX);

	unsigned char *FrameBuf = NULL;
	real_stream_s stream;
	vdec_stream_s in_stream;
	unsigned long long IpcPts, LocalPts, tmp;
	
	SPreviewFxnMsg msg;

	//通道队列中最近一次刷新到预览的帧时间(本地时间) ms
	u32 cur_pts;
	u32 adderrcnt;
	//u32 PrePtsMin;
	u32 IpcPtl[2]; //ipc protocol_type; PRO_TYPE_ONVIF  PRO_TYPE_KLW
	ipc_unit ipcam;
	volatile u32 PrePtsChn[2];
	volatile PreviewChnStatus ChnStatus[2];
	unsigned char VoBufClrCnt[2];
	//0/1:  主/子码流通道重置时，关闭VDEC接收，StatusChnWaitBuf时由主/子码流开启
	//-1 无效
	int VdecChnOnOffFlag;
	
	int chn, ChnIdx;
	int ret, delayms;
	unsigned char vdec_type, bprocess, sendflag;
	venc_frame_type_e FrameType;

	for (ChnIdx = 0; ChnIdx < 2; ChnIdx++)
	{
		PrePtsChn[ChnIdx] = 0;
		ChnStatus[ChnIdx] = StatusChnStop;
		VoBufClrCnt[ChnIdx] = 0;
		IpcPtl[ChnIdx] = 0;
	}
	VdecChnOnOffFlag = -1;//置无效值
	
	QueueLastIPCPtsChn[FxnChn] = 0;
	QueueLastIPCPtsChn[FxnChn+ARG_VI_NUM_MAX] = 0;
	
	/*********************for test*********************/
	int dowhilesuccess, i;
	unsigned char header[5];
	/*********************for test*********************/

	FrameBuf = (unsigned char *)malloc(FRAME_BUF);
	if (FrameBuf == NULL)
	{
		printf("NVRPreviewFxnChn%d FrameBuf malloc failed\n", FxnChn);
		while(1);
	}

	
	while(1)
	{
		ret = 0;
		//yaogang modify 20150306
		do
		{
			ret = ReadDataFromBuf(&plib_gbl_info->PreviewFxnMsgQueueChn[FxnChn], (u8 *)&msg, sizeof(SPreviewFxnMsg));
			if (ret == 0)
			{	
				if ((msg.chn == FxnChn) || (msg.chn == FxnChn+ARG_VI_NUM_MAX))
				{
					printf("%s get msg chn: %d, msg_type: %d\n", __func__, msg.chn, msg.type);					
				}
				else
				{
					printf("%s get msg chn: %d not match FxnChn: %d\n", __func__, msg.chn, FxnChn);
				}

				ChnIdx = msg.chn == FxnChn ? 0:1;
				switch (msg.type)
				{
					case TypeStartChn:
					{
						ChnStatus[ChnIdx] = StatusChnStart;
					} break;
					
					case TypeStopChn:
					{
						ChnStatus[ChnIdx] = StatusChnWillBeStop;
					} break;
					//yaogang modify 20170218 in shanghai
					case TypeChnBufException: //预览缓冲区异常，表现为写缓冲失败
					{
						ChnStatus[ChnIdx] = StatusChnRst;
					} break;
					
					default:
					{
						printf("%s read PreviewFxnMsg msg.type: %d invalid\n", __func__, msg.type);
					}
				}
			}
			else
			{
				if (ret != EM_REC_BUF_NO_DATA) //EM_REC_BUF_NO_DATA
				{
					printf("%s ReadDataFromBuf failed, PreviewFxnMsgQueueChn%d, ret: %d\n", __func__, FxnChn, ret);
				}
				break;
			}
		}while (1);
		//end
		
		//while (1)//缓冲中第一帧的时间戳+ 200ms < 当前时间
		for (ChnIdx = 0; ChnIdx < 2; ChnIdx++)
		{
			cur_pts = getTimeStamp(); //ms
			sendflag = 0;
			chn = ChnIdx == 0 ? FxnChn : FxnChn+ARG_VI_NUM_MAX;
			vdec_type = is_nvr_preview_chn_open(FxnChn);// 1 使用主码流预览2子码流预览
			
			switch (ChnStatus[ChnIdx])
			{
				case StatusChnStart:
				case StatusChnRst:
				{
					PrePtsChn[ChnIdx] = 0;
					ResetPreviewQueue(chn);
					
					//printf("%s StatusChnStart chn%d, vdec_type: %d, time: %u\n", __func__, chn, vdec_type, getTimeStamp());
					//IPC protocol
					ret = IPC_Get(FxnChn, &ipcam);
					if (ret == 0)
					{
						IpcPtl[ChnIdx] = ipcam.protocol_type;
					}
					else
					{
						printf("%s error chn%d, IPC_Get() failed %d, time: %u\n", __func__, chn, ret, getTimeStamp());
						continue;
					}
					//printf("%s chn%d protocol: %d\n", __func__, chn, IpcPtl[ChnIdx]);

					bprocess = 0;

					if ((vdec_type == 2) && (chn >= ARG_VI_NUM_MAX))//通道预览使用子码流
					{
						bprocess = 1;
					}
					
					if ((vdec_type == 1) && (chn < ARG_VI_NUM_MAX))//通道预览使用主码流
					{
						bprocess = 1;
					}

					if (bprocess)
					{
						//printf("%s HI_MPI_VDEC_StartRecvStream vdec_type: %d, chn%d\n", __func__, vdec_type, chn);
						
						pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[FxnChn]);

						VdecChnOnOffFlag = ChnIdx;//标记由主/子码流重置引起的关闭VDEC						
						
						ret = HI_MPI_VDEC_StopRecvStream(FxnChn);
						if(HI_SUCCESS != ret)
						{
							printf("%s error, line:%d, chn%d, HI_MPI_VDEC_StopRecvStream ret: 0x%x\n", __func__, __LINE__, chn, ret);
							pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
							continue;
						}
						
						ret = HI_MPI_VDEC_ResetChn(FxnChn);
						if(HI_SUCCESS != ret)
						{
							printf("%s error, line:%d, chn%d, HI_MPI_VDEC_ResetChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
							pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
							continue;
						}
						//HI_MPI_VDEC_StartRecvStream(vdec_chn);//yaogang modify 20150306
						
						pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
					}

					ChnStatus[ChnIdx] = StatusChnWaitIframe;
					
				} break;

				case StatusChnWaitIframe:
				{
					#if 0
					if(plib_gbl_info->preview_frame_queue[chn].nLenUsed <= 0)
					{
						//usleep(1);
						break;
					}
					#endif
					
					FrameType = 0;

					ret = GetPreviewQueueFirstFrameType(chn, &FrameType);
					if (ret == 0)
					{						
						if (FrameType == FRAME_TYPE_I)
						{	
						//#ifdef IPC_PROTOCOL_TEST
						#if 1
							if (IpcPtl[ChnIdx] != PRO_TYPE_KLW) //不缓冲直接写VDEC							
						#else
							if (IpcPtl[ChnIdx] == PRO_TYPE_ONVIF) //不缓冲直接写VDEC							
						#endif
							{
							#if 0
								bprocess = 0;
							
								if ((vdec_type == 2) && (chn >= ARG_VI_NUM_MAX))//通道预览使用子码流
								{
									bprocess = 1;
								}
								
								if ((vdec_type == 1) && (chn < ARG_VI_NUM_MAX))//通道预览使用主码流
								{
									bprocess = 1;
								}

								if (bprocess)
								{
									//printf("%s HI_MPI_VDEC_StartRecvStream vdec_type: %d, chn%d\n", __func__, vdec_type, vdec_chn);
									//yaogang modify 20150323
									//HI_MPI_VO_DisableChn + HI_MPI_VO_EnableChn是为了解决删除IPC通道后，
									//双击该通道区域会有一帧画面残留，逻辑上应该是黑屏
									#if 0
									PREVIEW_LOCK();
									ret = HI_MPI_VO_EnableChn(VO_DEVICE_HD, FxnChn);
									if(HI_SUCCESS != ret)
									{
										printf("%s error, line:%d, chn%d, HI_MPI_VO_EnableChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
										ChnStatus[ChnIdx] = StatusChnRst;
										continue;
									}
									PREVIEW_UNLOCK();
									#endif
									
									pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[FxnChn]);
									
									ret = HI_MPI_VDEC_StartRecvStream(FxnChn);//yaogang modify 20150306
									if(HI_SUCCESS != ret)
									{
										printf("%s error, line:%d, chn%d, HI_MPI_VDEC_StartRecvStream ret: 0x%x\n", __func__, __LINE__, chn, ret);
										ChnStatus[ChnIdx] = StatusChnRst;
										pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
										continue;
									}
									
									pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
								}
							#else
								pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[FxnChn]);
								//由谁关闭VDEC，由谁打开
								if (VdecChnOnOffFlag == ChnIdx)
								{
									VdecChnOnOffFlag = -1;//置无效值
									
									ret = HI_MPI_VDEC_StartRecvStream(FxnChn);//yaogang modify 20150306
									if(HI_SUCCESS != ret)
									{
										//结束录像回放到预览状态时，出现过此错误
										printf("%s error, line:%d, chn%d, HI_MPI_VDEC_StartRecvStream ret: 0x%x\n", __func__, __LINE__, chn, ret);
										ChnStatus[ChnIdx] = StatusChnRst;
										pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
										continue;
									}
								}
								pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);

								PREVIEW_LOCK();
								//如果当前通道需要在前台显示，则显示
								ret = is_chn_current_show(FxnChn);
								//printf("%s StatusChnWaitIframe chn%d vo show: %d, onvif\n", __func__, chn, ret);
								if (ret)
								{
									ret = HI_MPI_VO_ShowChn(VO_DEVICE_HD, FxnChn);
									if(HI_SUCCESS != ret)
									{
										printf("%s error, line:%d, chn%d, HI_MPI_VO_ShowChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
									}
								}
								PREVIEW_UNLOCK();
							#endif
								sendflag = 1;
								ChnStatus[ChnIdx] = StatusChnPreview;
								
								//printf("%s StatusChnWaitIframe ok chn%d, time: %u -- onvif\n", __func__, chn, getTimeStamp());
							}
							else if (IpcPtl[ChnIdx] == PRO_TYPE_KLW)
							{
								ChnStatus[ChnIdx] = StatusChnWaitBuf;
							}
							else
							{
								printf("%s error, chn%d protocol: %d, ChnStatus: %d\n", __func__, chn, IpcPtl[ChnIdx], ChnStatus[ChnIdx]);
								ChnStatus[ChnIdx] = StatusChnRst;
								continue;
							}
							
							//printf("%s StatusChnWaitBuf chn%d, vdec_type: %d, time: %u\n", __func__, chn, vdec_type, getTimeStamp());
						}
						else	//非I帧丢弃
						{
							if (ReadFrameFromPreviewQueue(chn, FrameBuf, FRAME_BUF, &stream) != 0)
							{
								//出错重置队列
								printf("%s error, chn%d ReadFrameFromPreviewQueue faild, StatusChnWaitIframe\n", __func__, chn);
								ChnStatus[ChnIdx] = StatusChnRst;
								continue;
							}
						}
					}
					else
					{
						//printf("%s chn%d GetPreviewQueueFirstFrameType failed %d, StatusChnWaitIframe\n", __func__, chn, ret);
						if (ret != EM_REC_BUF_NO_DATA)//EM_REC_BUF_NO_DATA
						{
							//出错重置队列
							printf("%s error, chn%d GetPreviewQueueFirstFrameType failed %d, StatusChnWaitIframe\n", __func__, chn, ret);
							ChnStatus[ChnIdx] = StatusChnRst;
							continue;
						}
					}
										
				} break;
				
				case StatusChnWaitBuf:
				{
					#if 0
					if(plib_gbl_info->preview_frame_queue[chn].nLenUsed <= 0)
					{
						//usleep(1);
						break;
					}
					#endif
					
					tmp =IpcPts = LocalPts =0;
					
					ret = GetPreviewQueueFirstFramePTS(chn, &IpcPts, &LocalPts);
					if (ret == 0)
					{
						//IpcPts = IpcPts/1000;
						//LocalPts = LocalPts/1000;
						tmp = QueueLastIPCPtsChn[chn];
						if ( IpcPts && (IpcPts + 400*1000  <= tmp) )//缓冲10帧 OK
						{	
							sendflag = 1;
							PrePtsChn[ChnIdx] = cur_pts;//本地预览播放时间控制
							ChnStatus[ChnIdx] = StatusChnPreview;

							#if 1
							if (chn == 0)
							{
								yg_pts = cur_pts;
								cnt = 0;
							}
							#endif

							#if 0
								bprocess = 0;
								
								if ((vdec_type == 2) && (chn >= ARG_VI_NUM_MAX))//通道预览使用子码流
								{
									bprocess = 1;
								}
								
								if ((vdec_type == 1) && (chn < ARG_VI_NUM_MAX))//通道预览使用主码流
								{
									bprocess = 1;
								}

								if (bprocess)
								{
									//printf("%s HI_MPI_VDEC_StartRecvStream vdec_type: %d, chn%d\n", __func__, vdec_type, vdec_chn);
									//yaogang modify 20150323
									//HI_MPI_VO_DisableChn + HI_MPI_VO_EnableChn是为了解决删除IPC通道后，
									//双击该通道区域会有一帧画面残留，逻辑上应该是黑屏
									#if 0
									PREVIEW_LOCK();
									ret = HI_MPI_VO_EnableChn(VO_DEVICE_HD, FxnChn);
									if(HI_SUCCESS != ret)
									{
										printf("%s, line:%d, chn%d, HI_MPI_VO_EnableChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
									}
									PREVIEW_UNLOCK();
									#endif
									
									pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[FxnChn]);
									
									ret = HI_MPI_VDEC_StartRecvStream(FxnChn);//yaogang modify 20150306
									if(HI_SUCCESS != ret)
									{
										printf("%s error, line:%d, chn%d, HI_MPI_VDEC_StartRecvStream ret: 0x%x\n", __func__, __LINE__, chn, ret);
										ChnStatus[ChnIdx] = StatusChnRst;
										pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
										continue;
									}
									
									pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
								}
							#else
								pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[FxnChn]);
								//由谁关闭VDEC，由谁打开
								if (VdecChnOnOffFlag == ChnIdx)
								{
									VdecChnOnOffFlag = -1;//置无效值
									
									ret = HI_MPI_VDEC_StartRecvStream(FxnChn);//yaogang modify 20150306
									if(HI_SUCCESS != ret)
									{
										//结束录像回放到预览状态时，出现过此错误
										printf("%s error, line:%d, chn%d, HI_MPI_VDEC_StartRecvStream ret: 0x%x\n", __func__, __LINE__, chn, ret);
										ChnStatus[ChnIdx] = StatusChnRst;
										pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);
										continue;
									}
								}
								pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);

								PREVIEW_LOCK();
								//如果当前通道需要在前台显示，则显示
								ret = is_chn_current_show(FxnChn);
								//printf("%s StatusChnWaitBuf chn%d vo show: %d\n", __func__, chn, ret);
								if (ret)
								{
									ret = HI_MPI_VO_ShowChn(VO_DEVICE_HD, FxnChn);
									if(HI_SUCCESS != ret)
									{
										printf("%s error, line:%d, chn%d, HI_MPI_VO_ShowChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
									}
								}
								PREVIEW_UNLOCK();
							#endif
							
							//printf("%s StatusChnPreview chn%d, vdec_type: %d, bprocess: %d, time: %u, IpcPts: %llu, QueueLastIPCPtsChn[%d]: %llu\n", 
							//	__func__, chn, vdec_type, bprocess, getTimeStamp(), IpcPts, chn, tmp);//此打印会造成预览卡顿
							
							//printf("%s StatusChnPreview chn%d, vdec_type: %d, bprocess: %d\n",  
							//	__func__, chn, vdec_type, bprocess);
							
							//printf("\t IpcPts: %llu, QueueLastIPCPtsChn[%d]: %llu, time: %u\n", 
							//	IpcPts, chn, tmp, getTimeStamp());
						}
					}	
					else
					{
						//printf("%s chn%d GetPreviewQueueFirstFramePTS failed %d, StatusChnWaitBuf\n", __func__, chn, ret);
						//if (ret != EM_REC_BUF_NO_DATA)//EM_REC_BUF_NO_DATA
						{
							//出错重置队列
							printf("%s error, chn%d GetPreviewQueueFirstFramePTS failed %d, StatusChnWaitBuf\n", __func__, chn, ret);
							ChnStatus[ChnIdx] = StatusChnRst;
							continue;
						}
					}
				} break;
												
				case StatusChnPreview:
				{
				//#ifdef IPC_PROTOCOL_TEST
				#if 1
					if (IpcPtl[ChnIdx] != PRO_TYPE_KLW) //不缓冲直接写VDEC							
				#else
					if (IpcPtl[ChnIdx] == PRO_TYPE_ONVIF) //不缓冲直接写VDEC							
				#endif
					{
						sendflag = 1;
					}
					else if (IpcPtl[ChnIdx] == PRO_TYPE_KLW)
					{
						if (cur_pts >= (PrePtsChn[ChnIdx] + 40))
						{
							PrePtsChn[ChnIdx] += 40;
							sendflag = 1;
							
							#if 0
							if (chn == 0)		
							{
								//printf("yg chn0 sendflag == 1\n");
								cnt++;
							}
							#endif
						}

						#if 0
						if ( (chn == 0) && (cur_pts >= (yg_pts+1000)) )
						{
							printf("%s chn0 send preview frame count: %d\n", __func__, cnt);
							yg_pts = cur_pts;
							cnt = 0;
						}
						#endif
					}
					else
					{
						printf("%s error, chn%d protocol: %d, ChnStatus: %d\n", __func__, chn, IpcPtl[ChnIdx], ChnStatus[ChnIdx]);
						ChnStatus[ChnIdx] = StatusChnRst;
						continue;
					}
				} break;
				
				case StatusChnWillBeStop:
				{
					printf("%s StatusChnWillBeStop chn%d\n", __func__, chn);
					#if 0
					bprocess = 0;
					
					if ((vdec_type == 2) && (chn >= ARG_VI_NUM_MAX))//通道预览使用子码流
					{
						bprocess = 1;
					}
					
					if ((vdec_type == 1) && (chn < ARG_VI_NUM_MAX))//通道预览使用主码流
					{
						bprocess = 1;
					}

					if (bprocess)
					#endif
					{
						//printf("%s HI_MPI_VDEC_StopRecvStream vdec_type: %d, chn%d\n", __func__, vdec_type, vdec_chn);
						#if 1
						//u32 interval = getTimeStamp();
						
						pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[FxnChn]);
						
						ret = HI_MPI_VDEC_StopRecvStream(FxnChn);
						if(HI_SUCCESS != ret)
						{
							printf("%s error, line:%d, chn%d, HI_MPI_VDEC_StopRecvStream ret: 0x%x\n", __func__, __LINE__, chn, ret);
							
						}
						
						ret = HI_MPI_VDEC_ResetChn(FxnChn);
						if(HI_SUCCESS != ret)
						{
							printf("%s error, line:%d, chn%d, HI_MPI_VDEC_ResetChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
						}
						
						pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[FxnChn]);

						PREVIEW_LOCK();
						ret = HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, FxnChn, HI_TRUE);
						if(HI_SUCCESS != ret)
						{
							printf("%s error, line:%d, chn%d, HI_MPI_VO_ClearChnBuffer ret: 0x%x\n", __func__, __LINE__, chn, ret);
						}

						VoBufClrCnt[ChnIdx] = 0;
						
						//yaogang modify 20150323
						//HI_MPI_VO_DisableChn + HI_MPI_VO_EnableChn是为了解决删除IPC通道后，
						//双击该通道区域会有一帧画面残留，逻辑上应该是黑屏
						/*
							#if 0
							ret = HI_MPI_VO_DisableChn(VO_DEVICE_HD, FxnChn);
							if(HI_SUCCESS != ret)
							{
								printf("%s error, line:%d, chn%d, HI_MPI_VO_DisableChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
							}
							#else
							
							//判断该通道当前是否在前台显示?
							//是-- 进入后续操作
							//否-- 不操作
							
							ret = is_chn_current_show(FxnChn);
							printf("%s StatusChnWillBeStop chn%d vo show: %d\n", __func__, chn, ret);
							if (ret)
							{
								ret = HI_MPI_VO_HideChn(VO_DEVICE_HD, FxnChn);
								if(HI_SUCCESS != ret)
								{
									printf("%s, line:%d, chn%d, HI_MPI_VO_HideChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
								}

								ret = HI_MPI_VO_ShowChn(VO_DEVICE_HD, FxnChn);
								if(HI_SUCCESS != ret)
								{
									printf("%s, line:%d, chn%d, HI_MPI_VO_ShowChn ret: 0x%x\n", __func__, __LINE__, chn, ret);
								}
							}
							#endif
						*/
						PREVIEW_UNLOCK();
						
						//printf("%s HI_MPI_VDEC_StopRecvStream vdec_type: %d, chn%d, interval: %d\n", __func__, vdec_type, vdec_chn, getTimeStamp()-interval);
						#endif
					}

					//printf("%s StatusChnWillBeStop chn%d, vdec_type: %d, bprocess: %d, cur_pts: %u, yg_pts: %u\n", 
					//	__func__, chn, vdec_type, bprocess, cur_pts, getTimeStamp());
					//printf("%s StatusChnStop chn%d\n", __func__, chn);

					PrePtsChn[ChnIdx] = 0;
					ResetPreviewQueue(chn);

					ChnStatus[ChnIdx] = StatusChnStop;
										
				} break;

				case StatusChnStop:
				{
					//do nothing
					//continue;
					#if 0
					if (VoBufClrCnt[ChnIdx] < 10)
					{
						VoBufClrCnt[ChnIdx]++;
						
						PREVIEW_LOCK();
						ret = HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, FxnChn, HI_TRUE);
						if(HI_SUCCESS != ret)
						{
							printf("%s error, line:%d, chn%d, HI_MPI_VO_ClearChnBuffer ret: 0x%x\n", __func__, __LINE__, chn, ret);
						}
						PREVIEW_UNLOCK();
					}
					#endif
				}break;
			
				default :
				{
					printf("%s chn%d status %d invalid\n", __func__, chn, ChnStatus[ChnIdx]);
				};
			}

		//#ifdef IPC_PROTOCOL_TEST
		#if 1
			if (IpcPtl[ChnIdx] != PRO_TYPE_KLW) //不缓冲直接写VDEC							
		#else
			if (IpcPtl[ChnIdx] == PRO_TYPE_ONVIF) //不缓冲直接写VDEC							
		#endif
			{
				ret = 0;
				//if (sendflag && (ChnStatus[ChnIdx] == StatusChnPreview) )
				if (sendflag)
				{
					/********************for test**********************/
					dowhilesuccess = 0;
					/******************************************/
					
					do{
						memset(&stream, 0, sizeof(stream));

						ret = ReadFrameFromPreviewQueue(chn, FrameBuf, FRAME_BUF, &stream);
						if(0 != ret)
						{
							if (EM_REC_BUF_NO_DATA != ret)//EM_REC_BUF_NO_DATA 1004
							{
								//出错重置队列
								printf("%s error, chn%d ReadFrameFromPreviewQueue failed %d -- onvif\n", __func__, chn, ret);
								ChnStatus[ChnIdx] = StatusChnRst;
							}
							/********************for test**********************/
							#if 0
							printf("%s chn%d 1 read queue failure, cur_pts: %u, success times: %d, pre frame header:\n", __func__, chn, cur_pts, dowhilesuccess);
							if (dowhilesuccess)
							{
								for(i=0; i<5; i++)
								{
									printf("\t 0x%x", header[i]);
								}
								printf("\n");
							}
							#endif
							/********************for test**********************/					
							
							break;
						}

						/********************for test**********************/
						dowhilesuccess++;
						memcpy(header, stream.data, 5);
						/********************for test**********************/

						memset(&in_stream, 0, sizeof(in_stream));
						in_stream.rsv = 0;
						in_stream.pts = stream.pts;
						in_stream.data = stream.data;
						in_stream.len = stream.len;
						nvr_preview_vdec_write(stream.chn, &in_stream);
					} while (1);
				}
			}
			else if (IpcPtl[ChnIdx] == PRO_TYPE_KLW)
			{
				//从队列取一帧写入预览
				ret = 0;			
				if (sendflag)
				{					
					/********************for test**********************/
					dowhilesuccess = 0;
					memset(header, 0, 5);
					/******************************************/
					
					do{
						memset(&stream, 0, sizeof(stream));

						ret = ReadFrameFromPreviewQueuePts(chn, FrameBuf, FRAME_BUF, &stream, &LocalPts);
						if(0 != ret)
						{
							/********************for test**********************/
							#if 0
							printf("%s chn%d 1 read queue failure, cur_pts: %u, success times: %d, pre frame header:\n", __func__, chn, cur_pts, dowhilesuccess);
							if (dowhilesuccess)
							{
								for(i=0; i<5; i++)
								{
									printf("\t 0x%x", header[i]);
								}
								printf("\n");
							}
							#else
							//printf("%s chn%d 1 read queue failure\n", __func__, chn);
							#endif
							/********************for test**********************/
							//出错重置
							ChnStatus[ChnIdx] = StatusChnRst;
							
							break;
						}
						
						if (stream.pts + 800*1000 < QueueLastIPCPtsChn[chn]) //缓冲的帧过多
						{
							#if 0
							printf("%s chn%d too mang frames in the buffer\n"
								"\t cur_pts: %u\n"
								"\t PrePtsChn: %u\n"
								"\t IpcPts: %llu\n"
								"\t QueueLastIPCPtsChn[chn]: %llu\n", 
								__func__, chn, cur_pts, PrePtsChn[ChnIdx], stream.pts, QueueLastIPCPtsChn[chn]);
							#else
							//printf("%s chn%d too mang frames\n", __func__, chn);
							#endif
							//出错重置
							ChnStatus[ChnIdx] = StatusChnRst;
							
							break;
						}

						/********************for test**********************/
						dowhilesuccess++;
						memcpy(header, stream.data, 5);
						/********************for test**********************/

						memset(&in_stream, 0, sizeof(in_stream));
						in_stream.rsv = 0;
						in_stream.pts = stream.pts;
						in_stream.data = stream.data;
						in_stream.len = stream.len;
						nvr_preview_vdec_write(stream.chn, &in_stream);

						#if 0
						if (chn == 0)
						{
							printf("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x, %u\n", 
								stream.data[0],
								stream.data[1],
								stream.data[2],
								stream.data[3],
								stream.data[4],
								getTimeStamp());
						}
						#endif
					} while (stream.len > 5 && ((stream.data[4] & 0x1f) >= 6) && ((stream.data[4] & 0x1f) <= 8) );//while (stream.len < 50);//一次刷新I帧的4个部分帧
				}

				//出错重置
				if (ChnStatus[ChnIdx] == StatusChnRst)
				{
					continue;
				}
				
				ret = 0;
				if (sendflag && ChnStatus[ChnIdx] == StatusChnPreview) //正常刷新之后，还需要刷??
				{
					#if 0//yaogang modify 20150323 for test

					cur_pts = getTimeStamp();
					if (cur_pts >= (PrePtsChn[ChnIdx] + 40))
					{	/*				
						printf("%s chn%d preview slow...\n"
							"\t cur_pts: %u\n"
							"\t PrePtsChn: %u\n"
							"\t QueueLastIPCPtsChn[chn]: %llu\n", 
							__func__, chn, cur_pts, PrePtsChn[ChnIdx], QueueLastIPCPtsChn[chn]);
						*/
						IpcPts = 0;
						if (GetPreviewQueueFirstFramePTS(chn, &IpcPts, &LocalPts) == 0)
						{
							printf("\t QueueFirstFramePTS: %llu\n", IpcPts);
						}
					}
					#endif

					adderrcnt = 0;
					while (cur_pts = getTimeStamp(), cur_pts >= (PrePtsChn[ChnIdx] + 40))
					{
						//printf("%s chn%d cur_pts1: %u, cur_pts: %u, PrePtsChn: %u, add up interval error > 40,  force write frame\n", 
							//__func__, chn, cur_pts1, cur_pts, PrePtsChn[ChnIdx]);
						
						ret = 0;
						PrePtsChn[ChnIdx] += 40;
						
						//printf("%s chn%d add up interval error > 40,  force write frame\n", __func__, chn);
						
						/********************for test**********************/
						dowhilesuccess = 0;
						memset(header, 0, 5);
						/******************************************/
						
						do{
							memset(&stream, 0, sizeof(stream));

							ret = ReadFrameFromPreviewQueue(chn, FrameBuf, FRAME_BUF, &stream);
							if(0 != ret)
							{
								/********************for test**********************/
								#if 1
								printf("%s chn%d 2 read queue failure, cur_pts: %u, adderrcnt: %d, success times: %d, pre frame header:\n", __func__, chn, cur_pts, adderrcnt, dowhilesuccess);
								if (dowhilesuccess)
								{
									for(i=0; i<5; i++)
									{
										printf("\t 0x%x", header[i]);
									}
									printf("\n");
								}
								#endif
								/********************for test**********************/
								
								break;
							}

							/********************for test**********************/
							dowhilesuccess++;
							memcpy(header, stream.data, 5);
							/********************for test**********************/

							memset(&in_stream, 0, sizeof(in_stream));
							in_stream.rsv = 0;
							in_stream.pts = stream.pts;
							in_stream.data = stream.data;
							in_stream.len = stream.len;
							nvr_preview_vdec_write(stream.chn, &in_stream);
						} while (stream.len > 5 && ((stream.data[4] & 0x1f) >= 6) && ((stream.data[4] & 0x1f) <= 8) );//while (stream.len < 50);//一次刷新I帧的4个部分帧

						
						if (adderrcnt++ > 2)
						{
							printf("%s chn%d adderrcnt++ > 2, cur_pts: %u\n", __func__, chn, cur_pts);
							ret = 1;
						}
						
						if(0 != ret)
						{
							//出错重置
							ChnStatus[ChnIdx] = StatusChnRst;
							
							break;
						}

						if(plib_gbl_info->PreviewFxnMsgQueueChn[FxnChn].nLenUsed > 0)
						{
							printf("%s FxnChn%d recv msg break\n", __func__, FxnChn);
							break;
						}
					}
				}

				//出错重置
				if (ChnStatus[ChnIdx] == StatusChnRst)
				{
					continue;
				}

				ret = 0;
				if((ChnStatus[ChnIdx] == StatusChnPreview || ChnStatus[ChnIdx] == StatusChnWaitBuf) && (plib_gbl_info->preview_frame_queue[chn].nLenUsed*100/plib_gbl_info->preview_frame_queue[chn].nLength > 80))
				{
					//printf("%s chn%d queue rate > 80%%,  force write frame, cur_pts: %u, ChnStatus: %d\n", __func__, chn, cur_pts, ChnStatus[ChnIdx]);

					ChnStatus[ChnIdx] = StatusChnRst;
				}
			}
		}
		
		//延时处理
		delayms = 5;
		
		#if 0
		if (IpcPtl[ChnIdx] == PRO_TYPE_ONVIF) //不缓冲直接写VDEC
		{
			delayms = 10;
		}
		else if (IpcPtl[ChnIdx] == PRO_TYPE_KLW)
		{			
			if (ChnStatus[0] == StatusChnStop && ChnStatus[1] == StatusChnStop)
			{
				delayms = 100;	//关闭状态下延时加长
			}
			
			if (ChnStatus[0] == StatusChnPreview && ChnStatus[1] == StatusChnPreview)
			{
				PrePtsMin = PrePtsChn[0] < PrePtsChn[1] ? PrePtsChn[0]: PrePtsChn[1];

				cur_pts = getTimeStamp();
				if (PrePtsMin + 40 > cur_pts)
				{
					delayms = PrePtsMin + 40 - cur_pts;
				}
				else
				{
					delayms = 1;
					//printf("%s chn%d delay err, cur_pts: %u, PrePtsChn: %u-%u\n", 
						//__func__, FxnChn, cur_pts, PrePtsChn[0], PrePtsChn[1]);
				}
			}
		}
		#endif
		
		usleep(delayms * 1000);
		
	}
	
	return 0;
}

#else
void *NVRPreviewFxn(void *arg)
{
	printf("NVRPreviewFxn, pid:%u, ARG_VI_NUM_MAX: %d\n", pthread_self(), ARG_VI_NUM_MAX);
	
	unsigned char *FrameBuf = NULL;
	real_stream_s stream;
	vdec_stream_s in_stream;
	unsigned long long IpcPts, LocalPts;
	
	SPreviewFxnMsg msg;
	//volatile unsigned char ChnRecvEnable[ARG_CHN_MAX * 2];

	//通道队列中最近一次刷新到预览的帧时间(本地时间) ms
	u32 cur_pts, cur_pts1;
	u32 adderrcnt;
	volatile u32 PrePtsChn[ARG_CHN_MAX*2];
	volatile PreviewChnStatus ChnStatus[ARG_CHN_MAX*2];
	unsigned char ChnStopStatusCnt[ARG_CHN_MAX*2];
	
	int chn, vdec_chn;
	int ret, sendflag;
	unsigned char vdec_type, bprocess;
	venc_frame_type_e FrameType;

	for (chn = 0; chn < ARG_VI_NUM_MAX*2; chn++)
	{
		//ChnRecvEnable[chn] = 1;
		PrePtsChn[chn] = 0;
		QueueLastIPCPtsChn[chn] = 0;
		ChnStopStatusCnt[chn] = 0;
		ChnStatus[chn] = StatusChnStop;
	}

	/*********************for test*********************/
	int dowhilesuccess, i;
	unsigned char header[5];
	/*********************for test*********************/

	FrameBuf = (unsigned char *)malloc(FRAME_BUF);
	if (FrameBuf == NULL)
	{
		printf("FrameBuf malloc failed\n");
		while(1);
	}
	
	while(1)
	{
		//yaogang modify 20150306
		do
		{
			ret = ReadDataFromBuf(&plib_gbl_info->PreviewFxnMsgQueue, (u8 *)&msg, sizeof(SPreviewFxnMsg));
			if (ret == 0)
			{	
				printf("%s get msg chn%d, msgtype: %d\n", __func__, msg.chn, msg.type);
				switch (msg.type)
				{
					case TypeStartChn:
					{
						ChnStatus[msg.chn] = StatusChnStart;
					} break;
					
					case TypeStopChn:
					{
						ChnStatus[msg.chn] = StatusChnWillBeStop;
					} break;
						
					default:
					{
						printf("%s read PreviewFxnMsg msg.type: %d invalid\n", __func__, msg.type);
					}
				}
			}
			else
			{
				break;
			}
		}while (1);
		//end
		
		cur_pts1 = getTimeStamp(); //ms
		
		//while (1)//缓冲中第一帧的时间戳+ 200ms < 当前时间
		for (chn = 0; chn < ARG_VI_NUM_MAX*2; chn++)
		{
			cur_pts = getTimeStamp(); //ms
			sendflag = 0;
			bprocess = 0;
			vdec_chn = chn < ARG_VI_NUM_MAX ? chn : (chn - ARG_VI_NUM_MAX);
			vdec_type = is_nvr_preview_chn_open(vdec_chn);// 1 使用主码流预览2子码流预览
			
			switch ( ChnStatus[chn] )
			{
				case StatusChnStart:
				case StatusChnRst:
				{
					printf("%s StatusChnStart chn%d, time: %u\n", __func__, chn, getTimeStamp());

					if ((vdec_type == 2) && (chn >= ARG_VI_NUM_MAX))//通道预览使用子码流
					{
						bprocess = 1;
					}
					
					if ((vdec_type == 1) && (chn < ARG_VI_NUM_MAX))//通道预览使用主码流
					{
						bprocess = 1;
					}

					if (bprocess)
					{
						//printf("%s HI_MPI_VDEC_StartRecvStream vdec_type: %d, chn%d\n", __func__, vdec_type, vdec_chn);
						
						pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[vdec_chn]);
						
						HI_MPI_VDEC_StopRecvStream(vdec_chn);
						HI_MPI_VDEC_ResetChn(vdec_chn);
						//HI_MPI_VDEC_StartRecvStream(vdec_chn);//yaogang modify 20150306
						
						pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[vdec_chn]);
					}

					PrePtsChn[chn] = 0;
					ResetPreviewQueue(chn);
					ChnStatus[chn] = StatusChnWaitIframe;
					
					printf("%s StatusChnWaitIframe chn%d, time: %u\n", __func__, chn, getTimeStamp());
				} break;

				case StatusChnWaitIframe:
				{
					#if 0
					if(plib_gbl_info->preview_frame_queue[chn].nLenUsed <= 0)
					{
						//usleep(1);
						break;
					}
					#endif
					
					FrameType = 0;
					if (GetPreviewQueueFirstFrameType(chn, &FrameType) == 0)
					{						
						if (FrameType == FRAME_TYPE_I)
						{	
							ChnStatus[chn] = StatusChnWaitBuf;
							
							printf("%s StatusChnWaitBuf chn%d, time: %u\n", __func__, chn, getTimeStamp());
						}
						else //非I帧丢弃
						{
							ReadFrameFromPreviewQueue(chn, FrameBuf, FRAME_BUF, &stream);
						}
					}
										
				} break;
				
				case StatusChnWaitBuf:
				{
					#if 0
					if(plib_gbl_info->preview_frame_queue[chn].nLenUsed <= 0)
					{
						//usleep(1);
						break;
					}
					#endif
					unsigned long long tmp;
					tmp =IpcPts = LocalPts =0;
					
					if (GetPreviewQueueFirstFramePTS(chn, &IpcPts, &LocalPts) == 0)
					{
						//IpcPts = IpcPts/1000;
						//LocalPts = LocalPts/1000;
						tmp = QueueLastIPCPtsChn[chn];
						if ( IpcPts && (IpcPts + 240*1000  <= tmp) )//缓冲4帧 OK
						{	
							sendflag = 1;
							PrePtsChn[chn] = cur_pts;//本地预览播放时间控制
							ChnStatus[chn] = StatusChnPreview;

							#if 1
							if (chn == 0)
							{
								yg_pts = cur_pts;
								cnt = 0;
							}
							#endif
							
							if ((vdec_type == 2) && (chn >= ARG_VI_NUM_MAX))//通道预览使用子码流
							{
								bprocess = 1;
							}
							
							if ((vdec_type == 1) && (chn < ARG_VI_NUM_MAX))//通道预览使用主码流
							{
								bprocess = 1;
							}

							if (bprocess)
							{
								//printf("%s HI_MPI_VDEC_StartRecvStream vdec_type: %d, chn%d\n", __func__, vdec_type, vdec_chn);
								//yaogang modify 20150323
								//HI_MPI_VO_DisableChn + HI_MPI_VO_EnableChn是为了解决删除IPC通道后，
								//双击该通道区域会有一帧画面残留，逻辑上应该是黑屏

								HI_MPI_VO_EnableChn(VO_DEVICE_HD, vdec_chn);
								
								pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[vdec_chn]);
								
								HI_MPI_VDEC_StartRecvStream(vdec_chn);//yaogang modify 20150306
								
								pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[vdec_chn]);
							}
							
							printf("%s StatusChnPreview chn%d, time: %u, IpcPts: %llu, QueueLastIPCPtsChn[%d]: %llu\n", \
								__func__, chn, getTimeStamp(), IpcPts, chn, tmp);							
						}
					}					
				} break;
												
				case StatusChnPreview:
				{
					if (cur_pts >= (PrePtsChn[chn] + 40))
					{
						PrePtsChn[chn] += 40;
						sendflag = 1;

						#if 1
						if (chn == 0)		
						{
							cnt++;
						}
						#endif
					}

					#if 1
					if ( (chn == 0) && (cur_pts >= (yg_pts+1000)) )
					{
						//printf("%s chn0 send preview frame count: %d\n", __func__, cnt);
						yg_pts = cur_pts;
						cnt = 0;
					}
					#endif
				} break;
				
				case StatusChnWillBeStop:
				{
					//printf("%s StatusChnWillBeStop chn%d\n", __func__, chn);
					
					if ((vdec_type == 2) && (chn >= ARG_VI_NUM_MAX))//通道预览使用子码流
					{
						bprocess = 1;
					}
					
					if ((vdec_type == 1) && (chn < ARG_VI_NUM_MAX))//通道预览使用主码流
					{
						bprocess = 1;
					}

					if (bprocess)
					{
						//printf("%s HI_MPI_VDEC_StopRecvStream vdec_type: %d, chn%d\n", __func__, vdec_type, vdec_chn);
						#if 1
						u32 interval = getTimeStamp();
						
						pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[vdec_chn]);
						
						HI_MPI_VDEC_StopRecvStream(vdec_chn);
						HI_MPI_VDEC_ResetChn(vdec_chn);
						//HI_MPI_VDEC_StartRecvStream(vdec_chn);//yaogang modify 20150306
						
						pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[vdec_chn]);

						HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, vdec_chn, HI_TRUE);
						//yaogang modify 20150323
						//HI_MPI_VO_DisableChn + HI_MPI_VO_EnableChn是为了解决删除IPC通道后，
						//双击该通道区域会有一帧画面残留，逻辑上应该是黑屏
						HI_MPI_VO_DisableChn(VO_DEVICE_HD, vdec_chn);

						//printf("%s HI_MPI_VDEC_StopRecvStream vdec_type: %d, chn%d, interval: %d\n", __func__, vdec_type, vdec_chn, getTimeStamp()-interval);
						#endif
					}

					printf("%s StatusChnWillBeStop chn%d, cur_pts1: %u, cur_pts: %u, yg_pts: %u\n", 
						__func__, chn, cur_pts1, cur_pts, getTimeStamp());
					//printf("%s StatusChnStop chn%d\n", __func__, chn);

					PrePtsChn[chn] = 0;
					ResetPreviewQueue(chn);

					ChnStatus[chn] = StatusChnStop;
					ChnStopStatusCnt[chn] = 0;
										
				} break;

				case StatusChnStop:
				{
					#if 0
					PrePtsChn[chn] = 0;
					//QueueLastIPCPtsChn[chn] = 0;
					ResetPreviewQueue(chn);

					if (ChnStopStatusCnt[chn]++ > 10)
					{
						ChnStopStatusCnt[chn] = 0;
						
						if ((vdec_type == 2) && (chn >= ARG_VI_NUM_MAX))//通道预览使用子码流
						{
							bprocess = 1;
						}
						
						if ((vdec_type == 1) && (chn < ARG_VI_NUM_MAX))//通道预览使用主码流
						{
							bprocess = 1;
						}
						
						if (bprocess)
						{
							HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, vdec_chn, HI_TRUE);
						}
					}
					#endif
				} break;
			
				default :
				{
					printf("%s chn%d status %d invalid\n", __func__, chn, ChnStatus[chn]);
				};
			}

			//从队列取一帧写入预览
			ret = 0;			
			if (sendflag)
			{
				//sendflag = 0;
				
				/********************for test**********************/
				dowhilesuccess = 0;
				memset(header, 0, 5);
				/******************************************/
				
				do{
					memset(&stream, 0, sizeof(stream));

					ret = ReadFrameFromPreviewQueue(chn, FrameBuf, FRAME_BUF, &stream);
					if(0 != ret)
					{
						/********************for test**********************/
						#if 1
						printf("%s chn%d 1 read queue failure, success times: %d, pre frame header:\n", __func__, chn, dowhilesuccess);
						if (dowhilesuccess)
						{
							for(i=0; i<5; i++)
							{
								printf("\t 0x%x", header[i]);
							}
							printf("\n");
						}
						#endif
						/********************for test**********************/
						//出错重置
						ChnStatus[chn] = StatusChnRst;
						
						break;
					}

					if (stream.pts + 520*1000 < QueueLastIPCPtsChn[chn]) //缓冲的帧过多
					{
						printf("%s chn%d too mang frames in the buffer\n"
							"\t cur_pts: %u\n"
							"\t PrePtsChn: %u\n"
							"\t IpcPts: %llu\n"
							"\t QueueLastIPCPtsChn[chn]: %llu\n", 
							__func__, chn, cur_pts, PrePtsChn[chn], stream.pts, QueueLastIPCPtsChn[chn]);

						//出错重置
						ChnStatus[chn] = StatusChnRst;
						
						break;
					}

					/********************for test**********************/
					dowhilesuccess++;
					memcpy(header, stream.data, 5);
					/********************for test**********************/

					memset(&in_stream, 0, sizeof(in_stream));
					in_stream.rsv = 0;
					in_stream.pts = stream.pts;
					in_stream.data = stream.data;
					in_stream.len = stream.len;
					nvr_preview_vdec_write(stream.chn, &in_stream);
				} while (stream.len > 5 && ((stream.data[4] & 0x1f) >= 6) && ((stream.data[4] & 0x1f) <= 8) );//while (stream.len < 50);//一次刷新I帧的4个部分帧
			}

			ret = 0;
			if (sendflag && ChnStatus[chn] == StatusChnPreview)
			{
				#if 1//yaogang modify 20150323 for test
				cur_pts = getTimeStamp();
				if (cur_pts >= (PrePtsChn[chn] + 40))
				{					
					printf("%s chn%d preview slow...\n"
						"\t cur_pts1: %u\n"
						"\t cur_pts: %u\n"
						"\t PrePtsChn: %u\n"
						"\t QueueLastIPCPtsChn[chn]: %llu\n", 
						__func__, chn, cur_pts1, cur_pts, PrePtsChn[chn], QueueLastIPCPtsChn[chn]);

					IpcPts = 0;
					if (GetPreviewQueueFirstFramePTS(chn, &IpcPts, &LocalPts) == 0)
					{
						printf("\t QueueFirstFramePTS: %llu\n", IpcPts);
					}
				}
				#endif

				adderrcnt = 0;
				while (cur_pts = getTimeStamp(), cur_pts >= (PrePtsChn[chn] + 40))
				{
					//printf("%s chn%d cur_pts1: %u, cur_pts: %u, PrePtsChn: %u, add up interval error > 40,  force write frame\n", 
						//__func__, chn, cur_pts1, cur_pts, PrePtsChn[chn]);
					
					ret = 0;
					PrePtsChn[chn] += 40;
					
					//printf("%s chn%d add up interval error > 40,  force write frame\n", __func__, chn);
					
					/********************for test**********************/
					dowhilesuccess = 0;
					memset(header, 0, 5);
					/******************************************/
					
					do{
						memset(&stream, 0, sizeof(stream));

						ret = ReadFrameFromPreviewQueue(chn, FrameBuf, FRAME_BUF, &stream);
						if(0 != ret)
						{
							/********************for test**********************/
							#if 1
							printf("%s chn%d 2 read queue failure, success times: %d, pre frame header:\n", __func__, chn, dowhilesuccess);
							if (dowhilesuccess)
							{
								for(i=0; i<5; i++)
								{
									printf("\t 0x%x", header[i]);
								}
								printf("\n");
							}
							#endif
							/********************for test**********************/
							
							break;
						}

						/********************for test**********************/
						dowhilesuccess++;
						memcpy(header, stream.data, 5);
						/********************for test**********************/

						memset(&in_stream, 0, sizeof(in_stream));
						in_stream.rsv = 0;
						in_stream.pts = stream.pts;
						in_stream.data = stream.data;
						in_stream.len = stream.len;
						nvr_preview_vdec_write(stream.chn, &in_stream);
					} while (stream.len > 5 && ((stream.data[4] & 0x1f) >= 6) && ((stream.data[4] & 0x1f) <= 8) );//while (stream.len < 50);//一次刷新I帧的4个部分帧

					
					if (adderrcnt++ > 4)
					{
						printf("%s chn%d adderrcnt++ > 4\n", __func__, chn);
						//PrePtsChn[chn] = getTimeStamp(); //ms
						//break;
						ret = 1;
					}
					
					if(0 != ret)
					{
						//出错重置
						//PrePtsChn[chn] = 0;
						//ResetPreviewQueue(chn);
						//QueueLastIPCPtsChn[chn] = 0;
						ChnStatus[chn] = StatusChnRst;
						
						break;
					}

					if(plib_gbl_info->PreviewFxnMsgQueue.nLenUsed > 0)
					{
						printf("%s recv msg break\n", __func__);
						break;
					}
				}
			}

			ret = 0;
			if((ChnStatus[chn] == StatusChnPreview || ChnStatus[chn] == StatusChnWaitBuf) && (plib_gbl_info->preview_frame_queue[chn].nLenUsed*100/plib_gbl_info->preview_frame_queue[chn].nLength > 80) )
			{
				printf("%s chn%d queue rate > 80%%,  force write frame\n", __func__, chn);

				ChnStatus[chn] = StatusChnRst;
				#if 0
				/********************for test**********************/
				dowhilesuccess = 0;
				memset(header, 0, 5);
				/******************************************/
				do{
					memset(&stream, 0, sizeof(stream));

					ret = ReadFrameFromPreviewQueue(chn, FrameBuf, FRAME_BUF, &stream);
					if(0 != ret)
					{
						/********************for test**********************/
						
						printf("%s chn%d add up interval error force write frame 3\n", __func__, chn);
						#if 0
						printf("dowhile force write success times: %d, pre frame header: \n", dowhilesuccess);
						if (dowhilesuccess)
						{
							
							for(i=0; i<5; i++)
							{
								printf("\t 0x%x", header[i]);
							}
							printf("\n");
						}
						#endif
						/********************for test**********************/
						//出错重置
						//PrePtsChn[chn] = 0;
						//ResetPreviewQueue(chn);
						//QueueLastIPCPtsChn[chn] = 0;
						ChnStatus[chn] = StatusChnRst;
						break;
						//continue;
					}

					/********************for test**********************/
					dowhilesuccess++;
					memcpy(header, stream.data, 5);
					/********************for test**********************/

					memset(&in_stream, 0, sizeof(in_stream));
					in_stream.rsv = 0;
					in_stream.pts = stream.pts;
					in_stream.data = stream.data;
					in_stream.len = stream.len;
					nvr_preview_vdec_write(stream.chn, &in_stream);
				} while (stream.len > 5 && ((stream.data[4] & 0x1f) >= 6) && ((stream.data[4] & 0x1f) <= 8) );//while (stream.len < 50);//一次刷新I帧的4个部分帧
				#endif
			}

		}
		usleep(5*1000);

	}
	
	return 0;
}
#endif
#endif


int DealStreamState(int chn, real_stream_state_e msg)
{
	#ifdef	UsePreviewQueue
	SPreviewFxnMsg PreviewMsg;
	#endif

	printf("%s chn%d, msg: %d\n", __func__, chn, msg);
	
	if(chn < 0 || chn >= (ARG_VI_NUM_MAX*2))
	{
		return -1;
	}
	
	if(chn >= ARG_VI_NUM_MAX)
	{
		chn -= ARG_VI_NUM_MAX;

		unsigned char ret = is_nvr_preview_chn_open(chn);
		
		switch(msg)
		{
		case REAL_STREAM_STATE_STOP:
		case REAL_STREAM_STATE_LOST:
			
			#ifdef	UsePreviewQueue
			//printf("chn%d sub stream lost, bySubStreamOpen: %d\n",chn+ARG_VI_NUM_MAX, plib_gbl_info->bySubStreamOpen[chn]);
			//yaogang modify 20150306
			if (plib_gbl_info->bySubStreamOpen[chn])
			{
				memset(&PreviewMsg, 0, sizeof(PreviewMsg));
				PreviewMsg.chn = chn + ARG_VI_NUM_MAX;
				PreviewMsg.type = TypeStopChn;
				printf("%s send msg[%d] chn%d, type: %d\n", __func__, chn, PreviewMsg.chn, PreviewMsg.type);

				#ifdef	PreviewFxnPerChn
				ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueueChn[chn], (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
				if (ret)
				{
					printf("%s 1 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
				}
				#else
				ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueue, (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
				if (ret)
				{
					printf("%s 2 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
				}
				#endif
				
			}
			//end
			
			#else
			
			if(ret == 2)//通道预览使用子码流
			{
				//yaogang modify 20140918
				pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[chn]);
				HI_MPI_VDEC_StopRecvStream(chn);
				HI_MPI_VDEC_ResetChn(chn);
				printf("%s HI_MPI_VDEC_StopRecvStream 1 chn%d\n", __func__, chn);
				//HI_MPI_VDEC_StartRecvStream(chn);//yaogang modify 20150306
				pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[chn]);
				
				#ifdef FAST_SWITCH_PREVIEW
				VO_CHN VoChn = chn;
				#else
				VO_CHN VoChn = chn-plib_gbl_info->last_mode_arg;
				#endif
				//HI_MPI_VO_ChnRefresh(VO_DEVICE_HD, VoChn);
				//HI_MPI_VO_ChnHide(VO_DEVICE_HD, VoChn);
				//HI_MPI_VO_ChnShow(VO_DEVICE_HD, VoChn);
				int i = 0;
				for(i=0;i<1;i++)
				{
					//HI_S32 s32Ret = HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
					//printf("VoChn:%d clear result:%d\n",VoChn,s32Ret);
					//printf("VoChn: %d, chn: %d msg:%d\n",VoChn, chn, msg);
					HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
					//if(i == 0) usleep(40*1000);
				}
		
			}
			
			#endif

			plib_gbl_info->bySubWaitIFrame[chn] = 1;
			plib_gbl_info->bySubStreamOpen[chn] = 0;
			
			break;
		case REAL_STREAM_STATE_START:
		#ifdef	UsePreviewQueue
			//yaogang modify 20150306
			//printf("%s start chn%d\n", __func__, chn+ ARG_VI_NUM_MAX);
			memset(&PreviewMsg, 0, sizeof(PreviewMsg));
			PreviewMsg.chn = chn + ARG_VI_NUM_MAX;
			PreviewMsg.type = TypeStartChn;
			printf("%s send msg[%d] chn%d, type: %d\n", __func__, chn, PreviewMsg.chn, PreviewMsg.type);

			#ifdef	PreviewFxnPerChn
			ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueueChn[chn], (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
			if (ret)
			{
				printf("%s 3 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
			}
			#else
			ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueue, (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
			if (ret)
			{
				printf("%s 4 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
			}
			#endif
			//end
		#else
			if(ret == 2)//通道预览使用子码流
			{
				pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[chn]);

				printf("%s HI_MPI_VDEC_StartRecvStream 1 chn%d\n", __func__, chn);
				HI_MPI_VDEC_StopRecvStream(chn);
				HI_MPI_VDEC_ResetChn(chn);
				HI_MPI_VDEC_StartRecvStream(chn);//yaogang modify 20150306
				
				pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[chn]);
			}
		#endif
		case REAL_STREAM_STATE_LINK:
			//printf("chn%d sub stream link\n",chn);
			if(IPC_GetLinkStatus(chn+ARG_VI_NUM_MAX))
			{
				plib_gbl_info->bySubStreamOpen[chn] = 1;
			}
			break;
		default:
			break;
		}
	}
	else //chn < ARG_VI_NUM_MAX
	{
		unsigned char ret = is_nvr_preview_chn_open(chn);
		switch(msg)
		{
		case REAL_STREAM_STATE_STOP:
		case REAL_STREAM_STATE_LOST:
					
		#ifdef	UsePreviewQueue
			//printf("chn%d main stream lost, byMainStreamOpen: %d\n",chn, plib_gbl_info->byMainStreamOpen[chn]);
			//yaogang modify 20150306
			if (plib_gbl_info->byMainStreamOpen[chn])
			{
				memset(&PreviewMsg, 0, sizeof(PreviewMsg));
				PreviewMsg.chn = chn;
				PreviewMsg.type = TypeStopChn;
				printf("%s send msg[%d] chn%d, type: %d\n", __func__, chn, PreviewMsg.chn, PreviewMsg.type);

				#ifdef	PreviewFxnPerChn
				ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueueChn[chn], (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
				if (ret)
				{
					printf("%s 5 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
				}
				#else
				ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueue, (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
				if (ret)
				{
					printf("%s 6 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
				}
				#endif
			}
			//end
			
			
		#else		
			//unsigned char ret = is_nvr_preview_chn_open(chn);
			if(ret == 1)//通道预览使用主码流
			{
				pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[chn]);
				HI_MPI_VDEC_StopRecvStream(chn);
				HI_MPI_VDEC_ResetChn(chn);
				printf("%s HI_MPI_VDEC_StopRecvStream 2 chn%d\n", __func__, chn);
				//HI_MPI_VDEC_StartRecvStream(chn);//yaogang modify 20150306
				pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[chn]);
				
				#ifdef FAST_SWITCH_PREVIEW
				VO_CHN VoChn = chn;
				#else
				VO_CHN VoChn = chn-plib_gbl_info->last_mode_arg;
				#endif
				//HI_MPI_VO_ChnRefresh(VO_DEVICE_HD, VoChn);
				//HI_MPI_VO_ChnHide(VO_DEVICE_HD, VoChn);
				//HI_MPI_VO_ChnShow(VO_DEVICE_HD, VoChn);
				int i = 0;
				for(i=0;i<1;i++)
				{
					//HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
					//if(i == 0) usleep(40*1000);
					//printf("VoChn: %d, chn: %d msg:%d\n",VoChn, chn, msg);
					HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
				}
				
			}
		#endif

			plib_gbl_info->byMainWaitIFrame[chn] = 1;
			plib_gbl_info->byMainStreamOpen[chn] = 0;
			
			venc_update_time(chn);
			
			break;
		case REAL_STREAM_STATE_START:
		#ifdef	UsePreviewQueue
			//yaogang modify 20150306
			//printf("%s start chn%d\n", __func__, chn);
			
			memset(&PreviewMsg, 0, sizeof(PreviewMsg));
			PreviewMsg.chn = chn;
			PreviewMsg.type = TypeStartChn;
			printf("%s send msg[%d] chn%d, type: %d\n", __func__, chn, PreviewMsg.chn, PreviewMsg.type);

			#ifdef	PreviewFxnPerChn
			ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueueChn[chn], (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
			if (ret)
			{
				printf("%s 7 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
			}
			#else
			ret = WriteDataToBuf(&plib_gbl_info->PreviewFxnMsgQueue, (u8 *)&PreviewMsg, sizeof(SPreviewFxnMsg));
			if (ret)
			{
				printf("%s 8 ret: %d, chn: %d, msgtype: %d\n", __func__, ret, PreviewMsg.chn, PreviewMsg.type);
			}
			#endif
			//end
		#else	
			if(ret == 1)//通道预览使用主码流
			{
				pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[chn]);
				
				printf("%s HI_MPI_VDEC_StartRecvStream 2 chn%d\n", __func__, chn);
				HI_MPI_VDEC_StopRecvStream(chn);
				HI_MPI_VDEC_ResetChn(chn);
				HI_MPI_VDEC_StartRecvStream(chn);//yaogang modify 20150306
				
				pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[chn]);
			}
		#endif
		case REAL_STREAM_STATE_LINK:
			//printf("chn%d main stream link\n",chn);
			venc_update_time(chn);
			if(IPC_GetLinkStatus(chn))
			{
				plib_gbl_info->byMainStreamOpen[chn] = 1;
			}
			break;
		default:
			break;
		}
	}
	
	return 0;
}


extern int DealRealAudio(real_stream_s *stream, unsigned int dwContext);

int DealRealStream(real_stream_s *stream, unsigned int dwContext)
{
	if(stream == NULL || stream->chn != dwContext)
	{
		printf("DealRealStream: param error\n");
		return -1;
	}
	
	//printf("###chn%d real data...\n", stream->chn);
	
	if(stream->media_type != MEDIA_PT_H264)
	{
		#ifdef HI3535
			return DealRealAudio(stream, dwContext);
			//return 0;
		#else
			return 0;//yg modify 20140913
		#endif
	}
	
	//printf("###chn%d real video data...\n", stream->chn);
	#if 0 //display frame header
		if (stream->chn == 0 || stream->chn == 16)
		{			
			printf("%s chn: %d, len: %d, 0x%x 0x%x 0x%x 0x%x 0x%x\n",
				__func__, stream->chn, stream->len,
				stream->data[0], stream->data[1], stream->data[2], stream->data[3], stream->data[4]);
		
		}
	#endif

	#if 1 //display max frame size
		static unsigned int max_frame_size = 0;
		static unsigned int display_cnt = 0;

		if (max_frame_size < stream->len)
			max_frame_size = stream->len;
		
		if (stream->chn == 0 && stream->frame_type == REAL_FRAME_TYPE_I)
		{	
			if (++display_cnt > 30)
			{
				display_cnt = 0;
				
				printf("%s max_frame_size: %u\n",
					__func__, max_frame_size);
			}
		}
	#endif
	
	#if 1
	int channel = stream->chn;
	
	/*if(channel == 8 || channel == 24)
	{
		if(channel == 8)
		{
			plib_gbl_info->main_frame_count[8]++;
			printf("chn%d frames:%d size:%d\n",channel,plib_gbl_info->main_frame_count[8],stream->len);
		}
		else
		{
			plib_gbl_info->sub_frame_count[8]++;
			printf("chn%d frames:%d size:%d\n",channel,plib_gbl_info->sub_frame_count[8],stream->len);
		}
		//printf("chn%d size:%d\n",channel,stream->len);
		return 0;
	}*/

	//子码流
	if(channel >= ARG_VI_NUM_MAX)
	{
		plib_gbl_info->sub_frame_count[channel-ARG_VI_NUM_MAX]++;
		
		if(plib_gbl_info->sub_stream_size[channel-ARG_VI_NUM_MAX].u32Width != stream->width)
		{
			plib_gbl_info->sub_stream_size[channel-ARG_VI_NUM_MAX].u32Width = stream->width;
		}
		if(plib_gbl_info->sub_stream_size[channel-ARG_VI_NUM_MAX].u32Height != stream->height)
		{
			plib_gbl_info->sub_stream_size[channel-ARG_VI_NUM_MAX].u32Height = stream->height;
		}
		
		//csp modify 20140406
		//printf("DealRealStream chn=%d venc_third_start=%d,last_pre_mode=%d,frametype=%d\n",channel-ARG_VI_NUM_MAX,plib_gbl_info->venc[channel-ARG_VI_NUM_MAX].venc_third_start,plib_gbl_info->last_pre_mode,stream->frame_type);
		if(plib_gbl_info->venc[channel-ARG_VI_NUM_MAX].venc_min_start || (plib_gbl_info->venc[channel-ARG_VI_NUM_MAX].venc_third_start && plib_gbl_info->last_pre_mode == PREVIEW_CLOSE))
		{
			//if(channel == 19) printf("DealRealStream:chn%d sub stream write to queue\n",channel-ARG_VI_NUM_MAX);
			if(WriteFrameToVencSubQueue(stream) < 0)
			{
				//do nothing
			}
		}
		
		/*if(channel == 16)
		{
			printf("chn%d frames:%d size:%d\n",channel,plib_gbl_info->sub_frame_count[channel-ARG_VI_NUM_MAX],stream->len);
			
			static int saved = 0;
			if(!saved)
			{
				static FILE *fp = NULL;
				if(fp == NULL)
				{
					//fp = fopen("/root/myusb/longse.h264","wb");
					fp = fopen("/mnt/longse.h264","wb");
				}
				if(fp != NULL)
				{
					fwrite(stream->data, stream->len, 1, fp);
					if(ftell(fp) > (2<<20))
					{
						printf("save stream over\n");
						
						fclose(fp);
						fp = NULL;
						
						saved = 1;
					}
				}
			}
		}*/
	}
	else //主码流
	{
		plib_gbl_info->main_frame_count[channel]++;
		
		if(plib_gbl_info->main_stream_size[channel].u32Width != stream->width)
		{
			plib_gbl_info->main_stream_size[channel].u32Width = stream->width;
		}
		if(plib_gbl_info->main_stream_size[channel].u32Height != stream->height)
		{
			plib_gbl_info->main_stream_size[channel].u32Height = stream->height;
		}
		
		plib_gbl_info->mdevent[channel] = stream->mdevent;
		if(stream->mdevent)
		{
			//printf("chn%d get md event\n",channel);
		}
		
		if(plib_gbl_info->venc[channel].start)
		{
			if(WriteFrameToVencMainQueue(stream) < 0)
			{
				//usleep(1);
			}
		}
		
		//if((channel == 0) && (plib_gbl_info->main_frame_count[channel] % 1500) == 0)
		//{
		//	printf("chn%d frames:%d time:%ld\n",channel,plib_gbl_info->main_frame_count[channel],time(NULL));
		//}
		
		/*if(channel == 3)
		{
			printf("chn%d frames:%d size:%d\n",channel,plib_gbl_info->main_frame_count[channel],stream->len);
			
			static int saved = 0;
			if(!saved)
			{
				static FILE *fp = NULL;
				if(fp == NULL)
				{
					fp = fopen("/mnt/ipc.h264","wb");
				}
				if(fp != NULL)
				{
					fwrite(stream->data, stream->len, 1, fp);
					if(ftell(fp) > (2<<20))
					{
						printf("save stream over\n");
						
						fclose(fp);
						fp = NULL;
						
						saved = 1;
					}
				}
			}
		}*/
	}

	
	int real_chn = stream->chn;
	if(real_chn >= ARG_VI_NUM_MAX)
	{
		real_chn -= ARG_VI_NUM_MAX;
	}
	//判断当前通道预览用的码流(主、子)
	unsigned char ret = is_nvr_preview_chn_open(real_chn);
	if(!ret) //非预览模式
	{
		//csp modify 20140318
		#ifdef P2P_SUB_STREAM
		if(plib_gbl_info->venc[real_chn].venc_third_start)
		{
			if(plib_gbl_info->last_pre_mode != PREVIEW_CLOSE)
			{
				
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
		#else
		return 0;
		#endif
	}
	else if(ret == 1 && stream->chn >= ARG_VI_NUM_MAX)//当前用主码流，但该帧是子码流
	{
		#ifdef FAST_SWITCH_PREVIEW
		//return 0;
		#else
		return 0;
		#endif
	}
	else if(ret == 2 && stream->chn < ARG_VI_NUM_MAX)//当前用子码流，但该帧是主码流
	{
		#ifdef FAST_SWITCH_PREVIEW
		//return 0;
		#else
		return 0;
		#endif
	}

#ifdef UsePreviewQueue	

	return WriteFrameToPreviewQueue(stream);

#endif	
	vdec_stream_s in_stream;
	in_stream.rsv = 0;
	in_stream.pts = stream->pts;
	in_stream.data = stream->data;
	in_stream.len = stream->len;
	
	pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[real_chn]);
	
	int rtn = nvr_preview_vdec_write(stream->chn, &in_stream);
	
	pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[real_chn]);
	
	return rtn;
	
	/*nvr_preview_vdec_write(stream->chn, &in_stream);
	if(stream->chn == ARG_VI_NUM_MAX)
	{
		int i;
		for(i=1;i<ARG_VI_NUM_MAX;i++)
		{
			nvr_preview_vdec_write(ARG_VI_NUM_MAX+i, &in_stream);
		}
	}
	return 0;*/
	#else
	vdec_stream_s in_stream;
	in_stream.rsv = 0;
	in_stream.pts = stream->pts;
	in_stream.data = stream->data;
	in_stream.len = stream->len;
	return nvr_preview_vdec_write(stream->chn, &in_stream);
	#endif
}

#ifdef HI3531
static HI_S32 SAMPLE_COMM_VO_MemConfig(VO_DEV VoDev, HI_CHAR *pcMmzName)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stMppChnVO;
	
    /* config vo dev */
    stMppChnVO.enModId  = HI_ID_VOU;
    stMppChnVO.s32DevId = VoDev;
    stMppChnVO.s32ChnId = 0;
    s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVO, pcMmzName);
    if(s32Ret)
    {
        LIB_PRT("HI_MPI_SYS_SetMemConf ERR!\n");
        return HI_FAILURE;
    } 
    
    return HI_SUCCESS;
}
#endif

HI_S32 VPSS_VO_Memconfig(void)
{
#ifdef HI3531
	HI_S32 i, s32Ret;
	MPP_CHN_S stMppChnVpss;
	HI_CHAR * pcMmzName;
	
	// init vpss mem
	for(i=0;i<64;i++)
	{
		stMppChnVpss.enModId  = HI_ID_VPSS;
		stMppChnVpss.s32DevId = i;
		stMppChnVpss.s32ChnId = 0;
		
		if(0 == (i%2))
		{
			pcMmzName = "ddr1";
		}
		else
		{
			pcMmzName = "ddr1";
		}
		
		/*vpss*/
		s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVpss, pcMmzName);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("Vpss HI_MPI_SYS_SetMemConf ERR!\n");
			return HI_FAILURE;
		}
	}
	
	// init vo mem
	s32Ret = SAMPLE_COMM_VO_MemConfig(SAMPLE_VO_DEV_DHD0, "ddr1");
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_MemConfig failed with %d!\n", s32Ret);
		return HI_FAILURE;
	}
	s32Ret = SAMPLE_COMM_VO_MemConfig(SAMPLE_VO_DEV_DHD1, "ddr1");
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_MemConfig failed with %d!\n", s32Ret);
		return HI_FAILURE;
	}
	s32Ret = SAMPLE_COMM_VO_MemConfig(SAMPLE_VO_DEV_DSD0, "ddr1");
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_MemConfig failed with %d!\n", s32Ret);
		return HI_FAILURE;
	}
#endif
	
	return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VDEC_MemConfig(HI_VOID)
{
	HI_S32 i = 0;
	HI_S32 s32Ret = HI_SUCCESS;
	
	HI_CHAR * pcMmzName;
	MPP_CHN_S stMppChnVDEC;
	
	/* VDEC chn max is 32*/
	for(i=0;i<32;i++)
	{
		stMppChnVDEC.enModId = HI_ID_VDEC;
		stMppChnVDEC.s32DevId = 0;
		stMppChnVDEC.s32ChnId = i;
		
		if(0 == (i%2))
		{
			pcMmzName = NULL;
		}
		else
		{
			pcMmzName = "ddr1";
		}
		
		s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVDEC,pcMmzName);
		if(s32Ret)
		{
			LIB_PRT("VDEC%d HI_MPI_SYS_SetMemConf failed with %#x!\n", i, s32Ret);
			return HI_FAILURE;
		}
	}
	
	return HI_SUCCESS;
}

void tl_hslib_init(tl_board_type_e type)
{
	HI_S32 s32Ret;
	int chn;
	//SIZE_S stSize;
	
	printf("hslib build in %s\n\n", __DATE__" "__TIME__);
	
	char *b = NULL;
	if(type == TL_BOARD_TYPE_NR2116)
	{
		b = "NR2116";
	}
	else if(type == TL_BOARD_TYPE_NR3116)
	{
		b = "NR3116";
	}
	else if(type == TL_BOARD_TYPE_NR3132)
	{
		b = "NR3132";
	}
	else if(type == TL_BOARD_TYPE_NR3124)
	{
		b = "NR3124";
	}
	else if(type == TL_BOARD_TYPE_NR1004)
	{
		b = "NR1004";
	}
	else if(type == TL_BOARD_TYPE_NR1008)
	{
		b = "NR1008";
	}
	else
	{
		printf("\n<ERROR>hslib_init fail!!!!!!!!!!!!! system will not work, not support board type=%d\n", type);
		
		while(1)
		{
			sleep(1);
		}
	}
	printf("running in hs35xx board(%s)\n", b);
	
	memset(&lib_gbl_info, 0x00, sizeof(struct lib_global_info));
	plib_gbl_info = &lib_gbl_info;
	lib_gbl_info.is_pal = 1;//default is PAL
	TL_HSLIB_TYPE = type;
	
	//csp modify
	lib_gbl_info.last_pre_mode = PREVIEW_CLOSE;
	lib_gbl_info.last_mode_arg = 0;

#if 0	
	//csp modify
	s32 ret;
	pthread_t pid;
	
	//csp modify
	for (chn=0; chn < ARG_VI_NUM_MAX; chn++)
	{
		pthread_mutex_init(&lib_gbl_info.preview_queue_lock[chn], NULL);
		lib_gbl_info.preview_frame_queue[chn].nBufId = 0;
		lib_gbl_info.preview_frame_queue[chn].nLength = 2*1024*1024;
		ret = CreateCircleBuf(&lib_gbl_info.preview_frame_queue[chn]);
		if(0 != ret)
		{
			//fprintf(stderr, "<ERROR-1>hslib_init CreateCircleBuf fail!!!!!!!!!!!!!\n");
			fprintf(stderr, "<ERROR-1>hslib_init CreateCircleBuf chn%d fail!!!!!!!!!!!!!\n", chn);
			
			while(1)
			{
				sleep(1);
			}
		}
	}
	
	for (chn=ARG_VI_NUM_MAX; chn < ARG_VI_NUM_MAX*2; chn++)
	{
		pthread_mutex_init(&lib_gbl_info.preview_queue_lock[chn], NULL);
		lib_gbl_info.preview_frame_queue[chn].nBufId = 0;
		lib_gbl_info.preview_frame_queue[chn].nLength = 512*1024;
		ret = CreateCircleBuf(&lib_gbl_info.preview_frame_queue[chn]);
		if(0 != ret)
		{
			//fprintf(stderr, "<ERROR-1>hslib_init CreateCircleBuf fail!!!!!!!!!!!!!\n");
			fprintf(stderr, "<ERROR-1>hslib_init CreateCircleBuf chn%d fail!!!!!!!!!!!!!\n", chn);
			
			while(1)
			{
				sleep(1);
			}
		}
	}
	//csp modify
	pthread_mutex_init(&lib_gbl_info.venc_main_queue_lock, NULL);
	pthread_mutex_init(&lib_gbl_info.venc_sub_queue_lock, NULL);
	//lib_gbl_info.venc_main_frame_queue.nBufId = 0;
	//lib_gbl_info.venc_main_frame_queue.nLength = 3*1024*1024;
	//ret = CreateCircleBuf(&lib_gbl_info.venc_main_frame_queue);
	//if(0 != ret)
	//{
	//	fprintf(stderr, "<ERROR-2>hslib_init CreateCircleBuf fail!!!!!!!!!!!!!\n");
	//	
	//	while(1)
	//	{
	//		sleep(1);
	//	}
	//}
	//lib_gbl_info.venc_sub_frame_queue.nBufId = 0;
	//lib_gbl_info.venc_sub_frame_queue.nLength = (1*1024*1024)/2;
	//ret = CreateCircleBuf(&lib_gbl_info.venc_sub_frame_queue);
	//if(0 != ret)
	//{
	//	fprintf(stderr, "<ERROR-3>hslib_init CreateCircleBuf fail!!!!!!!!!!!!!\n");
	//	
	//	while(1)
	//	{
	//		sleep(1);
	//	}
	//}
	
	//csp modify
	pthread_create(&pid, NULL, NVRPreviewFxn, NULL);
#endif	
	//csp modify
	if(TL_BOARD_TYPE_NR2116 == TL_HSLIB_TYPE)
	{
		lib_gbl_info.preview_chn_threshold = 1;//大于1个窗口则使用子码流预览
	}
	else if(TL_BOARD_TYPE_NR3116 == TL_HSLIB_TYPE)
	{
		lib_gbl_info.preview_chn_threshold = 4;//大于4个窗口则使用子码流预览
	}
	else if(TL_BOARD_TYPE_NR3132 == TL_HSLIB_TYPE)
	{
		lib_gbl_info.preview_chn_threshold = 4;//大于4个窗口则使用子码流预览
	}
	else if(TL_BOARD_TYPE_NR3124 == TL_HSLIB_TYPE)
	{
		lib_gbl_info.preview_chn_threshold = 4;//大于4个窗口则使用子码流预览
	}
	else if(TL_BOARD_TYPE_NR1004 == TL_HSLIB_TYPE)
	{
		lib_gbl_info.preview_chn_threshold = 1;//大于1个窗口则使用子码流预览
	}
	else if(TL_BOARD_TYPE_NR1008 == TL_HSLIB_TYPE)
	{
		lib_gbl_info.preview_chn_threshold = 1;//大于1个窗口则使用子码流预览
	}
	
	#ifdef FAST_SWITCH_PREVIEW
	if(lib_gbl_info.preview_chn_threshold > 1)
	{
		lib_gbl_info.preview_chn_threshold = 1;
	}
	#endif

	#ifdef HI3535
	
	#ifdef DDR4G
	//yaogang modify 20140918
	//四画面用主码流
	lib_gbl_info.preview_chn_threshold = 4;// 4 大于4个窗口则使用子码流预览
	#else
	lib_gbl_info.preview_chn_threshold = 1;
	#endif
	
	#endif
	
	//lib_gbl_info.sCmdHdr = CreateMsgCmd(sizeof(ipc_unit));
	//if(lib_gbl_info.sCmdHdr == NULL)
	//{
	//	fprintf(stderr, "<ERROR>hslib_init CreateMsgCmd fail!!!!!!!!!!!!!\n");
	//	
	//	while(1)
	//	{
	//		sleep(1);
	//	}
	//}
	
	plib_gbl_info->fd_tw286x = open("/dev/tw_286x", O_RDWR);
	if(plib_gbl_info->fd_tw286x < 0)
	{
		perror("open /dev/tw_286x");
		printf("<ERROR>hslib_init fail!!!!!!!!!!!!!please Check whether loaded the tw_286x.ko driver!\n");
		return; 
	}
	
	//不同的平台设备名不一样，要做区别处理
	#ifdef HI3531
	plib_gbl_info->fd_tl = open("/dev/tl_R9800", O_RDWR);
	#else
	plib_gbl_info->fd_tl = open("/dev/tl_R9700", O_RDWR);
	#endif
	if(plib_gbl_info->fd_tl < 0)
	{
		perror("open /dev/tl_Rxxx");
		printf("<ERROR>hslib_init fail!!!!!!!!!!!!! please Check GPIO driver!\n");
		return;
	}
	
	//csp modify for audio
	{
		unsigned int buf[5];
		if(ioctl(plib_gbl_info->fd_tw286x, TL_AUTO_CHIP_TYPE, buf) < 0)
		{
			printf("<ERROR>tl_hslib_init fail!!!!!!!!!!!!! TL_AUTO_CHIP_TYPE: system will not work!\n");
		}
		
		if(buf[0] == CHIP_TW2968_1 || buf[0] == CHIP_TW2968_2 || buf[0] == CHIP_TW2964_1)
		{
			is_decoder_techwell = 1;
			
			//csp modify 20140525
			if(buf[0] == CHIP_TW2968_1 || buf[0] == CHIP_TW2968_2 || buf[0] == CHIP_TW2964_1)
			{
				tw286x_wr(0, 0x40, 0x00);
				
				unsigned char chip_id = 0xff;
				tw286x_rd(0, 0xFF, &chip_id);//csp modify 20140612
				
				if(chip_id != 0xF0 && chip_id != 0xE8)
				{
					no_audio_chip = 1;
				}
			}
		}
		else if(buf[0] == CHIP_TW2868_1 || buf[0] == CHIP_TW2868_2 || buf[0] == CHIP_TW2866_1)
		{
			is_decoder_techwell = 1;
			
			//csp modify 20140525
			if(buf[0] == CHIP_TW2868_1|| buf[0] == CHIP_TW2868_2)
			{
				tw286x_wr(0, 0x40, 0x00);
				
				unsigned char chip_id = 0xff;
				tw286x_rd(0, 0xFF, &chip_id);//csp modify 20140612
				
				if(chip_id != 0xB0)
				{
					no_audio_chip = 1;
				}
			}
		}
		else if(buf[0] == CHIP_NVP1914_1 || buf[0] == CHIP_NVP1918_1 || buf[0] == CHIP_NVP1114B_1 || buf[0] == CHIP_NVP1118B_1)
		{
			is_decoder_nvp = 1;
		}
		else if(buf[0] == CHIP_RN6314A_1 || buf[0] == CHIP_RN6318A_1)
		{
			is_decoder_rn631x = 1;
		}
		else//csp modify 20140525
		{
			#ifndef HI3535
			no_audio_chip = 1;
			#endif
		}
		
		printf("is_decoder_techwell=%d,is_decoder_nvp=%d,is_decoder_rn631x=%d,decoder_chip=%d,no_audio_chip=%d\n",
			is_decoder_techwell,is_decoder_nvp,is_decoder_rn631x,buf[0],no_audio_chip);
	}
	
	if(0 != (g_client_id & TL_PAL))
	{
		lib_gbl_info.is_pal = 1;
	}
	else if(0 != (g_client_id & TL_NTSC))
	{
		lib_gbl_info.is_pal = 0;
	}
	printf("Hi352x hslib config for %s\n", IS_PAL() ? "PAL" : "NTSC");
	
	if(is_decoder_nvp)
	{
		nvp_wr(0, 0, 0x78, 0x88);
		nvp_wr(0, 0, 0x79, 0x88);
		nvp_wr(0, 0, 0xf8, 0x88);
		nvp_wr(0, 0, 0xf9, 0x88);
	}
	
	s32Ret = SAMPLE_InitMPP();
	if(0 != s32Ret)
	{
		printf("<ERROR>hslib_init fail, SAMPLE_InitMPP!!!!!!!!!!!!! system will not work!\n");
		return;
	}
	
	//MPP version
	MPP_VERSION_S stMppVersion;
	HI_MPI_SYS_GetVersion(&stMppVersion);
	printf("Mpp version: %s\n", stMppVersion.aVersion);
	
	#if defined(HI3531)
	VO_DEVICE_CVBS = SAMPLE_VO_DEV_DSD0;
	VO_DEVICE_HD   = SAMPLE_VO_DEV_DHD1;
	#elif defined(HI3535)
	VO_DEVICE_CVBS = SAMPLE_VO_DEV_DSD0;
	VO_DEVICE_HD   = SAMPLE_VO_DEV_DHD0;
	#else
	VO_DEVICE_CVBS = SAMPLE_VO_DEV_DSD1;
	VO_DEVICE_HD   = SAMPLE_VO_DEV_DHD0;
	#endif
	
	#ifdef HI3531
	s32Ret = VPSS_VO_Memconfig();
	if(0 != s32Ret)
	{
		printf("<ERROR>hslib_init fail, VPSS_VO_Memconfig!!!!!!!!!!!!! system will not work!\n");
		return;
	}
	
	s32Ret = SAMPLE_COMM_VDEC_MemConfig();
	if(0 != s32Ret)
	{
		LIB_PRT("<ERROR>hslib_init fail, SAMPLE_COMM_VDEC_MemConfig failed with %d!!! system will not work!\n", s32Ret);
		return;
	}
	#endif
	
	if(TL_BOARD_TYPE_NR2116 == TL_HSLIB_TYPE || TL_BOARD_TYPE_NR3116 == TL_HSLIB_TYPE)
	{
		ARG_VI_NUM_MAX = 16;
		ARG_VO_NUM_MAX = 16;
		
		//stSize.u32Width = 1280;
		//stSize.u32Height = 720;
		
		//s32Ret = SAMPLE_StartVpss(stSize, ARG_VI_NUM_MAX);
		//if(0 != s32Ret)
		//{
		//	printf("<ERROR>hslib_init fail, SAMPLE_StartVpss!!!!!!!!!!!!! system will not work!\n");
		//	return;
		//}
		
		s32Ret = vio_enable_vo_all(VO_MODE_16MUX);
		if(0 != s32Ret)
		{
			printf("<ERROR>hslib_init fail, vio_enable_vo_all!!!!!!!!!!!!! system will not work!\n");
			return;
		}
	}
	else if(TL_BOARD_TYPE_NR3132 == TL_HSLIB_TYPE)
	{
		ARG_VI_NUM_MAX = 32;
		ARG_VO_NUM_MAX = 36;
		
		s32Ret = vio_enable_vo_all(VO_MODE_36MUX);
		if(0 != s32Ret)
		{
			printf("<ERROR>hslib_init fail, vio_enable_vo_all!!!!!!!!!!!!! system will not work!\n");
			return;
		}
	}
	else if(TL_BOARD_TYPE_NR3124 == TL_HSLIB_TYPE)
	{
		ARG_VI_NUM_MAX = 24;
		ARG_VO_NUM_MAX = 25;
		
		s32Ret = vio_enable_vo_all(VO_MODE_25MUX);
		if(0 != s32Ret)
		{
			printf("<ERROR>hslib_init fail, vio_enable_vo_all!!!!!!!!!!!!! system will not work!\n");
			return;
		}
		
		//SIZE_S stSize;
		//stSize.u32Width = 720;
		//stSize.u32Height = 576;
		//SAMPLE_COMM_VPSS_Start(0, 64, &stSize, 5, NULL);
		//SAMPLE_COMM_VPSS_Stop(0, 64, 5);
		//printf("start-stop-vpss\n");
	}
	else if(TL_BOARD_TYPE_NR1004 == TL_HSLIB_TYPE)
	{
		ARG_VI_NUM_MAX = 4;
		ARG_VO_NUM_MAX = 4;
		
		s32Ret = vio_enable_vo_all(VO_MODE_4MUX);
		if(0 != s32Ret)
		{
			printf("<ERROR>hslib_init fail, vio_enable_vo_all!!!!!!!!!!!!! system will not work!\n");
			return;
		}
	}
	else if(TL_BOARD_TYPE_NR1008 == TL_HSLIB_TYPE)
	{
		ARG_VI_NUM_MAX = 8;
		ARG_VO_NUM_MAX = 9;
		
		s32Ret = vio_enable_vo_all(VO_MODE_9MUX);
		if(0 != s32Ret)
		{
			printf("<ERROR>hslib_init fail, vio_enable_vo_all!!!!!!!!!!!!! system will not work!\n");
			return;
		}
	}

//yaogang modify 20150302
#ifdef	UsePreviewQueue
	//csp modify
	s32 ret;
	pthread_t pid;

#ifndef	PreviewFxnPerChn
	//消息队列yaogang modify 20150306
	lib_gbl_info.PreviewFxnMsgQueue.nBufId = 0;
	lib_gbl_info.PreviewFxnMsgQueue.nLength = sizeof(SPreviewFxnMsg) * ARG_VI_NUM_MAX * 6;
	ret = CreateCircleBuf(&lib_gbl_info.PreviewFxnMsgQueue);
	if(0 != ret)
	{
		fprintf(stderr, "<ERROR-1>hslib_init PreviewFxnMsgQueue fail!!!!!!!!!!!!!\n");
		
		while(1)
		{
			sleep(1);
		}
	}
	
#endif	
	//csp modify
	for (chn=0; chn < ARG_VI_NUM_MAX; chn++)
	{
		pthread_mutex_init(&lib_gbl_info.preview_queue_lock[chn], NULL);
		lib_gbl_info.preview_frame_queue[chn].nBufId = 0;
		lib_gbl_info.preview_frame_queue[chn].nLength = 3*512*1024;
		ret = CreateCircleBuf(&lib_gbl_info.preview_frame_queue[chn]);
		if(0 != ret)
		{
			//fprintf(stderr, "<ERROR-1>hslib_init CreateCircleBuf fail!!!!!!!!!!!!!\n");
			fprintf(stderr, "<ERROR-1>hslib_init CreateCircleBuf chn%d fail!!!!!!!!!!!!!\n", chn);
			
			while(1)
			{
				sleep(1);
			}
		}

	#ifdef	PreviewFxnPerChn
	
		//每个预览通道一个消息队列yaogang modify 20150325
		lib_gbl_info.PreviewFxnMsgQueueChn[chn].nBufId = chn;
		lib_gbl_info.PreviewFxnMsgQueueChn[chn].nLength = sizeof(SPreviewFxnMsg) * 16;
		ret = CreateCircleBuf(&lib_gbl_info.PreviewFxnMsgQueueChn[chn]);
		if(0 != ret)
		{
			fprintf(stderr, "<ERROR-1>hslib_init PreviewFxnMsgQueueChn%d fail!!\n", chn);
			
			while(1)
			{
				sleep(1);
			}
		}
	
	#endif
	}
	
	for (chn=ARG_VI_NUM_MAX; chn < ARG_VI_NUM_MAX*2; chn++)
	{
		//yaogang modify 20170218 in shanghai
		pthread_mutex_init(&lib_gbl_info.preview_queue_lock[chn], NULL);
		lib_gbl_info.preview_frame_queue[chn].nBufId = 0;
		lib_gbl_info.preview_frame_queue[chn].nLength = 512*1024;//256*1024;
		ret = CreateCircleBuf(&lib_gbl_info.preview_frame_queue[chn]);
		if(0 != ret)
		{
			//fprintf(stderr, "<ERROR-1>hslib_init CreateCircleBuf fail!!!!!!!!!!!!!\n");
			fprintf(stderr, "<ERROR-1>hslib_init CreateCircleBuf chn%d fail!!!!!!!!!!!!!\n", chn);
			
			while(1)
			{
				sleep(1);
			}
		}
	}
	//csp modify
	pthread_mutex_init(&lib_gbl_info.venc_main_queue_lock, NULL);
	pthread_mutex_init(&lib_gbl_info.venc_sub_queue_lock, NULL);
	//lib_gbl_info.venc_main_frame_queue.nBufId = 0;
	//lib_gbl_info.venc_main_frame_queue.nLength = 3*1024*1024;
	//ret = CreateCircleBuf(&lib_gbl_info.venc_main_frame_queue);
	//if(0 != ret)
	//{
	//	fprintf(stderr, "<ERROR-2>hslib_init CreateCircleBuf fail!!!!!!!!!!!!!\n");
	//	
	//	while(1)
	//	{
	//		sleep(1);
	//	}
	//}
	//lib_gbl_info.venc_sub_frame_queue.nBufId = 0;
	//lib_gbl_info.venc_sub_frame_queue.nLength = (1*1024*1024)/2;
	//ret = CreateCircleBuf(&lib_gbl_info.venc_sub_frame_queue);
	//if(0 != ret)
	//{
	//	fprintf(stderr, "<ERROR-3>hslib_init CreateCircleBuf fail!!!!!!!!!!!!!\n");
	//	
	//	while(1)
	//	{
	//		sleep(1);
	//	}
	//}
	
	//csp modify
	#ifdef	PreviewFxnPerChn
		for (chn=0; chn < ARG_VI_NUM_MAX; chn++)
		{
			ret = pthread_create(&pid, NULL, NVRPreviewFxnChn, (void *)chn);
			if (ret)
			{
				printf("%s pthread_create NVRPreviewFxnChn%d failure\n", __func__, chn);
			}
		}
	#else
		ret = pthread_create(&pid, NULL, NVRPreviewFxn, NULL);
		if (ret)
		{
			printf("%s pthread_create NVRPreviewFxn failure\n", __func__);
		}
	#endif
	
#endif
//yaogang modify 20150302 end


	//printf("%s ARG_VI_NUM_MAX: %d; ARG_VO_NUM_MAX: %d\n", __func__, ARG_VI_NUM_MAX, ARG_VO_NUM_MAX);
	//lib_gbl_info.last_pre_mode = PREVIEW_CLOSE;//default:preview close
	
	pthread_mutex_init(&plib_gbl_info->lock_md, NULL);
	pthread_mutex_init(&plib_gbl_info->lock_venc, NULL);
	pthread_mutex_init(&plib_gbl_info->lock_venc_min, NULL);
	pthread_mutex_init(&plib_gbl_info->preview_lock, NULL);
	pthread_mutex_init(&plib_gbl_info->i2c_bank_lock, NULL);
	pthread_mutex_init(&plib_gbl_info->lock_ai_aenc, NULL);
	pthread_mutex_init(&plib_gbl_info->lock_adec_ao, NULL);
	pthread_mutex_init(&plib_gbl_info->lock_adec_ao_ref_count, NULL);
	pthread_mutex_init(&plib_gbl_info->lock_vi_set, NULL);
	pthread_mutex_init(&plib_gbl_info->lock_venc_pts, NULL);
	
	//csp modify 20140318
	pthread_mutex_init(&plib_gbl_info->lock_venc_third, NULL);
	
	s32Ret = venc_init();
	if(0 != s32Ret)
	{
		printf("<ERROR>hslib_init fail, venc_init!!!!!!!!!!!!! system will not work!\n");
		return;
	}
	
	//NVR used for NR1008 audio bug
	tl_audio_open();
	
#ifdef FAST_SWITCH_PREVIEW
	int i=0;
	for(i=0;i<ARG_VI_NUM_MAX;i++)
	{
		pthread_mutex_init(&plib_gbl_info->preview_chn_lock[i], NULL);
		plib_gbl_info->byDecoderType[i] = 0;
	}
	vio_bind_vi2vo_all(0, ARG_VI_NUM_MAX, 0, 0, 0);
	nvr_preview_vdec_open(0, ARG_VI_NUM_MAX);
#endif
	
	IPC_RegisterCallback(DealRealStream, DealStreamState);
	IPC_Init(ARG_VI_NUM_MAX);
	
	printf("hslib_init ok!\n");
	
	return;
}

void tl_hslib_init_c(tl_board_type_e type, int client_id)
{
	g_client_id = client_id;
	printf("%s: type: %d, g_client_id = %d\n", __func__, type, g_client_id);// 100  2
	
	tl_hslib_init(type);
	
	return;
}

//设置指定通道对应的网络摄像机
//摄像机参数为空则清除该通道之前之前设置的网络摄像机
int tl_set_ipcamera(int channel, ipc_unit *ipcam)
{
	if(channel >= ARG_VI_NUM_MAX)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	//return WriteMsgCmd(plib_gbl_info->sCmdHdr, ipcam);
	
	plib_gbl_info->ipcam[channel] = *ipcam;
	return IPC_Set(channel, ipcam);
}

//获取指定通道对应的网络摄像机
int tl_get_ipcamera(int channel, ipc_unit *ipcam)
{
	if(channel >= ARG_VI_NUM_MAX)
	{
		return -1;
	}
	
	if(ipcam == NULL)
	{
		return -1;
	}
	
	*ipcam = plib_gbl_info->ipcam[channel];
	
	return 0;
}

