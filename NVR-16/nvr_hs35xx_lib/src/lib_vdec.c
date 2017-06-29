/******************************************************************************

	TL hslib preview
	
	2008-12-16	created by kong
	2010-11-11	modify by andyrew

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_vdec.h"
#include "hi_comm_vdec.h"

#include "hi_comm_vo.h"
#include "mpi_vo.h"

#include "lib_common.h"
#include "common.h"
#include "lib_vdec.h"
#include "vio_common.h"

//=====================================================
#define PLAY_CTL_USE_RF		1
#define SUPPORT_AUDIO		1
//=====================================================

//csp modify 20140406
#define VDEC_PAUSE_ZOOM_DEBUG

#define VDEC_FRAME_MODE
//#undef VIDEO_MODE_FRAME
//csp modify 20140406
static unsigned char is_playing = 0;
unsigned char GetPlayingStatus()
{
	return is_playing;
}

static int open_chn_num = 0;
static int vdec_buf_num = 1;

struct vdec_information
{
	int open;
	float speed_div;
	int frame_rate;
	int base_speed;
	
	unsigned long long pre_pts[ARG_CHN_MAX];
	unsigned long long dec_pts[ARG_CHN_MAX];
	unsigned long long last_frame_time[ARG_CHN_MAX];
	
	int width[ARG_CHN_MAX];
	int height[ARG_CHN_MAX];
};

struct vdec_information vdec_info;

#define CHECK_OPENED	{if(!vdec_info.open){printf("lib_vdec.c: vdec not open\n"); return -1;}}

extern int tl_adec_ao_start();
extern int tl_adec_ao_stop();

static int channel_show[ARG_CHN_MAX];

extern VO_INTF_SYNC_E g_enIntfSync;

static unsigned char g_vdec_zoom_flag = 0;
static unsigned char g_vdec_elec_zoom_flag = 0;
static int g_vdec_zoom_chn = -1;

static int zoom_in_channel(int channel)
{
	if(g_vdec_elec_zoom_flag)
	{
		pre_rect_s stCapRect;
		memset(&stCapRect,0,sizeof(stCapRect));
		tl_vdec_eletroinc_zoom(PREVIEW_EA_CLOSE,g_vdec_zoom_chn,stCapRect);
	}
	
#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	//int s32VpssOffset = 0;//ARG_VI_NUM_MAX;
#else
	int s32VpssOffset = 0;//ARG_VI_NUM_MAX;
#endif
	
#if 0//最新的SDK还需要这段代码吗???
	if(ARG_VI_NUM_MAX > 16)
	{
		s32VpssOffset = 0;
	}
	else
	{
		s32VpssOffset = ARG_VI_NUM_MAX;
	}
#endif
	
	if(channel < 0 || channel >= open_chn_num)
	{
		HI_S32 s32Ret = HI_SUCCESS;
		HI_U32 u32Width = 0;
		HI_U32 u32Height = 0;
		HI_U32 u32Frm = 0;
		extern VO_INTF_SYNC_E g_enIntfSync;
		s32Ret = SAMPLE_COMM_VO_GetWH(g_enIntfSync,&u32Width,&u32Height,&u32Frm);
		HI_U32 u32WndNum = 1;
		HI_U32 u32Square = 1;
		if(open_chn_num <= 1)
		{
			u32WndNum = 1;
			u32Square = 1;
		}
		else if(open_chn_num <= 4)
		{
			u32WndNum = 4;
			u32Square = 2;
		}
		else if(open_chn_num <= 9)
		{
			u32WndNum = 9;
			u32Square = 3;
		}
		else// if(open_chn_num <= 16)
		{
			u32WndNum = 16;
			u32Square = 4;
		}
		int i;
		for(i=0;i<open_chn_num;i++)
		{
		#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
			#ifdef HI3535
			HI_MPI_VO_HideChn(VO_DEVICE_HD,i);
			#else
			HI_MPI_VO_ChnHide(VO_DEVICE_HD,i);
			#endif
		#else
			HI_MPI_VDEC_StopRecvStream(i);
			
			SAMPLE_COMM_VO_UnBindVpss(VO_DEVICE_HD,i);
			HI_MPI_VO_DisableChn(VO_DEVICE_HD,i);
		#endif
		}
		for(i=0;i<open_chn_num;i++)
		{
			VO_CHN_ATTR_S stChnAttr;
			stChnAttr.stRect.s32X		= ALIGN_BACK((u32Width/u32Square) * (i%u32Square), 2);
			stChnAttr.stRect.s32Y		= ALIGN_BACK((u32Height/u32Square) * (i/u32Square), 2);
			stChnAttr.stRect.u32Width	= ALIGN_BACK(u32Width/u32Square, 2);
			stChnAttr.stRect.u32Height	= ALIGN_BACK(u32Height/u32Square, 2);
			stChnAttr.u32Priority		= 0;
			stChnAttr.bDeflicker		= HI_TRUE;//HI_FALSE;
			s32Ret = HI_MPI_VO_SetChnAttr(VO_DEVICE_HD, i, &stChnAttr);
			if(s32Ret != HI_SUCCESS)
			{
				printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
				//return HI_FAILURE;
			}
			
		#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
			#ifdef HI3535
			HI_MPI_VO_ShowChn(VO_DEVICE_HD,i);
			#else
			HI_MPI_VO_ChnShow(VO_DEVICE_HD,i);
			#endif
		#else
			HI_MPI_VO_EnableChn(VO_DEVICE_HD,i);
			
			SAMPLE_COMM_VO_BindVpss(VO_DEVICE_HD, i, i + s32VpssOffset, VPSS_PRE0_CHN);//csp modify 20130506
			
			HI_MPI_VDEC_StartRecvStream(i);
		#endif
		}
		
		g_vdec_zoom_flag = 0;
		g_vdec_zoom_chn = -1;
	}
	else
	{
		HI_S32 s32Ret = HI_SUCCESS;
		HI_U32 u32Width = 0;
		HI_U32 u32Height = 0;
		HI_U32 u32Frm = 0;
		
		extern VO_INTF_SYNC_E g_enIntfSync;
		s32Ret = SAMPLE_COMM_VO_GetWH(g_enIntfSync,&u32Width,&u32Height,&u32Frm);
		
		int i;
		for(i=0;i<open_chn_num;i++)
		{
		#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
			if(i != channel)
			{
				#ifdef HI3535
				HI_MPI_VO_HideChn(VO_DEVICE_HD,i);
				#else
				HI_MPI_VO_ChnHide(VO_DEVICE_HD,i);
				#endif
			}
		#else
			HI_MPI_VDEC_StopRecvStream(i);
			
			SAMPLE_COMM_VO_UnBindVpss(VO_DEVICE_HD,i);
			HI_MPI_VO_DisableChn(VO_DEVICE_HD,i);
		#endif
		}
		
		VO_CHN_ATTR_S stChnAttr;
		stChnAttr.stRect.s32X		= 0;
		stChnAttr.stRect.s32Y		= 0;
		stChnAttr.stRect.u32Width	= u32Width;
		stChnAttr.stRect.u32Height	= u32Height;
		stChnAttr.u32Priority		= 0;
		stChnAttr.bDeflicker		= HI_TRUE;//HI_FALSE;
	#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
		s32Ret = HI_MPI_VO_SetChnAttr(VO_DEVICE_HD, channel, &stChnAttr);
	#else
		s32Ret = HI_MPI_VO_SetChnAttr(VO_DEVICE_HD, 0, &stChnAttr);
	#endif
		if(s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
			return HI_FAILURE;
		}
		
	#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
		#ifdef HI3535
		HI_MPI_VO_ShowChn(VO_DEVICE_HD,channel);
		#else
		HI_MPI_VO_ChnShow(VO_DEVICE_HD,channel);
		#endif
	#else
		HI_MPI_VO_EnableChn(VO_DEVICE_HD,0);
		
		SAMPLE_COMM_VO_BindVpss(VO_DEVICE_HD, 0, channel + s32VpssOffset, VPSS_PRE0_CHN);//csp modify 20130506
		
		HI_MPI_VDEC_StartRecvStream(channel);
	#endif
		
		g_vdec_zoom_flag = 1;
		g_vdec_zoom_chn = channel;
	}
	
	return 0;
}

void SampleWaitDestroyVdecChn(HI_S32 s32ChnID)
{
	HI_S32 s32ret;
	
	//printf("WaitDestroyVdecChn -1\n");
	
	s32ret = HI_MPI_VDEC_StopRecvStream(s32ChnID);
	if(s32ret != HI_SUCCESS)
	{
		printf("HI_MPI_VDEC_StopRecvStream failed errno 0x%x\n", s32ret);
		//return;
	}
	
	//HI_MPI_VDEC_ResetChn(s32ChnID);//csp modify 20150110
	
	//printf("WaitDestroyVdecChn -2\n");
	
#if 0
	unsigned int last_LeftStreamBytes;
	int not_change_cou;
	
	VDEC_CHN_STAT_S stStat;
	memset(&stStat,0,sizeof(VDEC_CHN_STAT_S));
	
	last_LeftStreamBytes = 0;
	not_change_cou = 0;
	while(1)
	{
		usleep(40000);
		s32ret = HI_MPI_VDEC_Query(s32ChnID, &stStat);
		if(s32ret != HI_SUCCESS)
		{
			printf("line:%d,HI_MPI_VDEC_Query failed errno 0x%x\n", __LINE__, s32ret);
			//return;
		}
		//printf("u32LeftStreamFrames=%u, u32LeftStreamBytes=%u\n", stStat.u32LeftStreamFrames, stStat.u32LeftStreamBytes);//++++++++++++++++
		if(stStat.u32LeftPics == 0 && stStat.u32LeftStreamBytes == 0)
		{
			//printf("vdec had no stream and pic left\n");
			break;
		}
		//in case last frame not complete
		if(stStat.u32LeftStreamBytes == last_LeftStreamBytes)
		{
			if(++not_change_cou >= 3)
			{
				break;
			}
		}
		else
		{
			not_change_cou = 0;
			last_LeftStreamBytes = stStat.u32LeftStreamBytes;
		}
	}
	
	usleep(10000);
#endif
	
	//printf("WaitDestroyVdecChn -3\n");
	
	s32ret = HI_MPI_VDEC_DestroyChn(s32ChnID);
	if(s32ret != HI_SUCCESS)
	{
		printf("HI_MPI_VDEC_DestroyChn failed errno 0x%x\n", s32ret);
		return;
	}
	
	//printf("WaitDestroyVdecChn -4\n");
}

/******************************************************************************
* function : vdec group bind vo chn
******************************************************************************/
HI_S32 SAMLE_COMM_VDEC_BindVo(VDEC_CHN VdChn, VO_DEV VoDev, VO_CHN VoChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	
	stSrcChn.enModId = HI_ID_VDEC;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = VdChn;
	
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = VoDev;
	stDestChn.s32ChnId = VoChn;
	
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_SYS_Bind failed with %#x!\n",s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

/******************************************************************************
* function : vdec group unbind vo chn
******************************************************************************/
HI_S32 SAMLE_COMM_VDEC_UnBindVo(VDEC_CHN VdChn, VO_DEV VoDev, VO_CHN VoChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	
	stSrcChn.enModId = HI_ID_VDEC;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = VdChn;
	
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = VoDev;
	stDestChn.s32ChnId = VoChn;
	
	s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_SYS_UnBind failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	return HI_SUCCESS;
}

/******************************************************************************
* function : vdec group bind vpss chn
******************************************************************************/
HI_S32 SAMLE_COMM_VDEC_BindVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	
	stSrcChn.enModId = HI_ID_VDEC;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = VdChn;
	
	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;
	stDestChn.s32ChnId = 0;
	
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_SYS_Bind failed with %#x!\n",s32Ret);
		return HI_FAILURE;
	}
	
	return HI_SUCCESS;
}

/******************************************************************************
* function : vdec group unbind vpss chn
******************************************************************************/
HI_S32 SAMLE_COMM_VDEC_UnBindVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	
	stSrcChn.enModId = HI_ID_VDEC;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = VdChn;
	
	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;
	stDestChn.s32ChnId = 0;
	
	s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_SYS_UnBind failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	return HI_SUCCESS;
}
/*
//yaogang modify 20150921
//无须加锁，上层函数在使用时已经加锁
typedef struct
{
	u8 pts_state;//状态机
	u8 vdec_state;//状态机，由vdec缓冲帧数量决定
	u8 err_cnt;//错误计数，基准PTS更新	
	u32 frame_cnt;
	u64 reset_time;//每10秒重置pts
	u64 basa_pts;//基准PTS
	u64 pre_pts;//前一帧码流回调过来的PTS	
}vdec_chn_pts_s;//对解码通道PTS的处理

static vdec_chn_pts_s pts_per_chn[ARG_CHN_MAX];
*/

/******************************************************************************
* function : create vdec chn
******************************************************************************/
HI_S32 SAMPLE_VDEC_CreateVdecChn(HI_S32 s32ChnID, SIZE_S *pstSize, PAYLOAD_TYPE_E enType)
{
	VDEC_CHN_ATTR_S stAttr;
	HI_S32 s32Ret;
	
	memset(&stAttr, 0, sizeof(VDEC_CHN_ATTR_S));
	
	stAttr.enType = enType;
	#ifdef HI3520D
	stAttr.u32BufSize = pstSize->u32Height * pstSize->u32Width * 3 / 4;
	//csp modify 20140406
	if(GetPlayingStatus())
	{
		if(open_chn_num > 1)
		{
			//stAttr.u32BufSize = pstSize->u32Height * pstSize->u32Width * 1 / 2;
		}
	}
	#else
	stAttr.u32BufSize = pstSize->u32Height * pstSize->u32Width;//This item should larger than u32Width*u32Height/2
	//stAttr.u32BufSize = pstSize->u32Height * pstSize->u32Width * 3 / 2;
	#endif
	stAttr.u32Priority = 1;//此处必须大于0
	stAttr.u32PicWidth = pstSize->u32Width;
	stAttr.u32PicHeight = pstSize->u32Height;
	
	switch(enType)
	{
		case PT_H264:
			#ifdef HI3520D
			stAttr.stVdecVideoAttr.u32RefFrameNum = 2;//1;//csp modify 20140307
				#ifdef VDEC_FRAME_MODE
					stAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
				#else
					stAttr.stVdecVideoAttr.enMode = VIDEO_MODE_STREAM;
				#endif
			stAttr.stVdecVideoAttr.s32SupportBFrame = 0;//0;
			//csp modify 20140406
			if(GetPlayingStatus())
			{
				if(open_chn_num > 1)
				{
					#ifdef HI3535
					stAttr.stVdecVideoAttr.u32RefFrameNum = 2;
					#else
					stAttr.stVdecVideoAttr.u32RefFrameNum = 1;
					#endif
				}
			}
			//stAttr.stVdecVideoAttr.u32RefFrameNum = 1;
			//printf("create decoder[%d] u32RefFrameNum:%d\n",s32ChnID,stAttr.stVdecVideoAttr.u32RefFrameNum);
			#else
			stAttr.stVdecVideoAttr.u32RefFrameNum = 2;//1;
				#ifdef VDEC_FRAME_MODE
					stAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
				#else
					stAttr.stVdecVideoAttr.enMode = VIDEO_MODE_STREAM;
				#endif
			stAttr.stVdecVideoAttr.s32SupportBFrame = 1;//0;
			#endif
			break;
		case PT_JPEG:
			stAttr.stVdecJpegAttr.enMode = VIDEO_MODE_FRAME;
			break;
		case PT_MJPEG:
			stAttr.stVdecJpegAttr.enMode = VIDEO_MODE_FRAME;
			break;
		default:
			LIB_PRT("err type\n");
			return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VDEC_CreateChn(s32ChnID, &stAttr);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VDEC_CreateChn[%d] failed errno 0x%x (wxh=%dx%d)\n", s32ChnID, s32Ret, pstSize->u32Width, pstSize->u32Height);
		return s32Ret;
	}
	
#if defined(HI3520D)
//#if defined(HI3520D) && !defined(HI3535)//csp modify 20150110
//#ifdef HI3520D//csp modify 20140406
//#if 0//#ifdef HI3520D//csp modify 20140308
	VDEC_PRTCL_PARAM_S stPrtclParam;
	#ifdef HI3535
	s32Ret = HI_MPI_VDEC_GetProtocolParam(s32ChnID, &stPrtclParam);
	#else
	s32Ret = HI_MPI_VDEC_GetPrtclParam(s32ChnID, &stPrtclParam);
	#endif
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VDEC_GetPrtclParam failed errno 0x%x\n", s32Ret);
		HI_MPI_VDEC_DestroyChn(s32ChnID);//csp modify 20150110
		return s32Ret;
	}
	
	#if 0
	stPrtclParam.s32MaxSpsNum = 21;
	stPrtclParam.s32MaxPpsNum = 22;
	stPrtclParam.s32MaxSliceNum = 100;
	#else
	stPrtclParam.s32MaxSpsNum = 1;
	stPrtclParam.s32MaxPpsNum = 1;
	stPrtclParam.s32MaxSliceNum = 16;
		#ifdef HI3520D
	//csp modify 20140406
	//stPrtclParam.s32DisplayFrameNum = 1;
	//csp modify 20140406
	stPrtclParam.s32DisplayFrameNum = 1;//2;//1;
	if(GetPlayingStatus())
	{
		if(open_chn_num > 1)
		{
			stPrtclParam.s32DisplayFrameNum = 1;
		}
	}
			#ifdef HI3535//csp modify 20150110
	//stPrtclParam.s32DisplayFrameNum = 0;//1;
			#endif
	//csp modify 20140406
	//stPrtclParam.s32SCDBufSize = pstSize->u32Height * pstSize->u32Width * 3 / 2;
	//if(stPrtclParam.s32SCDBufSize > 1024*1024)
	//{
	//	stPrtclParam.s32SCDBufSize = pstSize->u32Height * pstSize->u32Width * 3 / 4;
	//}
		#else
	stPrtclParam.s32DisplayFrameNum = 2;
		#endif
	//printf("s32SCDBufSize=%d\n",stPrtclParam.s32SCDBufSize);
	#endif
	
	#ifdef HI3535
	s32Ret = HI_MPI_VDEC_SetProtocolParam(s32ChnID, &stPrtclParam);
	#else
	s32Ret = HI_MPI_VDEC_SetPrtclParam(s32ChnID, &stPrtclParam);
	#endif
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VDEC_SetPrtclParam failed errno 0x%x\n", s32Ret);
		HI_MPI_VDEC_DestroyChn(s32ChnID);//csp modify 20150110
		return s32Ret;
	}
#endif
	
#if 0
	VIDEO_DISPLAY_MODE_E DisplayMode = 0xff;
	s32Ret = HI_MPI_VDEC_GetDisplayMode(s32ChnID, &DisplayMode);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VDEC_GetDisplayMode failed errno 0x%x\n", s32Ret);
		HI_MPI_VDEC_DestroyChn(s32ChnID);//csp modify 20150110
		return s32Ret;
	}
	//printf("haha1,HI_MPI_VDEC_GetDisplayMode:%d^^^^^^\n", DisplayMode);
	
	HI_MPI_VDEC_SetDisplayMode(s32ChnID, VIDEO_DISPLAY_MODE_PREVIEW);
	
	DisplayMode = 0xff;
	s32Ret = HI_MPI_VDEC_GetDisplayMode(s32ChnID, &DisplayMode);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VDEC_GetDisplayMode failed errno 0x%x\n", s32Ret);
		HI_MPI_VDEC_DestroyChn(s32ChnID);//csp modify 20150110
		return s32Ret;
	}
	//printf("haha2,HI_MPI_VDEC_GetDisplayMode:%d^^^^^^\n", DisplayMode);
#endif

	//printf("%s hahaha\n", __func__);
	s32Ret = HI_MPI_VDEC_StartRecvStream(s32ChnID);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VDEC_StartRecvStream failed errno 0x%x\n", s32Ret);
		HI_MPI_VDEC_DestroyChn(s32ChnID);//csp modify 20150110
		return s32Ret;
	}
	
	return HI_SUCCESS;
}

extern int adec_ao_reference_count;
int tl_vdec_open(int vdec_chn_num)
{
	VPSS_GRP VpssGrp;
	VO_CHN VoChn;
	HI_S32 s32Ret = HI_SUCCESS;
	SIZE_S stSize;
	HI_S32 s32VpssFirstGrpIdx = 0;
	HI_S32 i;
	VDEC_CHN VdChn;
	//HI_S32 s32D1Cnt = ARG_VI_NUM_MAX;

	//最新的SDK还需要这段代码吗???
	#if 0
	if(ARG_VI_NUM_MAX > 16)
	{
		s32VpssFirstGrpIdx = 0;
	}
	else
	{
		s32VpssFirstGrpIdx = ARG_VI_NUM_MAX;
	}
	#endif
	
	if(vdec_chn_num == 1)
	{
		g_vdec_zoom_flag = 1;
		g_vdec_zoom_chn = 0;
	}
	else
	{
		g_vdec_zoom_flag = 0;
		g_vdec_zoom_chn = -1;
	}
	
	if(lib_gbl_info.last_pre_mode != PREVIEW_CLOSE)
	{
		printf("lib_vdev.c@tl_vdec_open: you should close preview first\n");
		return -1;
	}
	if(vdec_chn_num != 1 && vdec_chn_num != 4 && vdec_chn_num != 9 && vdec_chn_num != 16)
	{
		printf("lib_vdev.c@tl_vdec_open: not support this num vdec_chn_num = %d\n", vdec_chn_num);
		return -1;
	}
	
	open_chn_num = vdec_chn_num;
	vdec_buf_num = 1;
	
	if(ARG_VI_NUM_MAX == 8)
	{
		if(vdec_chn_num == 9)
		{
			open_chn_num = 8;
		}
	}
	else if(ARG_VI_NUM_MAX == 24)
	{
		if(vdec_chn_num == 25)
		{
			open_chn_num = 24;
		}
	}
	else if(ARG_VI_NUM_MAX == 32)
	{
		if(vdec_chn_num == 36)
		{
			open_chn_num = 32;
		}
	}
	
	is_playing = 1;//csp modify 20140406
	
	//csp modify 20140406
	int max_w = VDEC_MAX_W;
	int max_h = VDEC_MAX_H;
#ifdef HI3520D
	if(open_chn_num > 1)
	{
		if(max_h > 1088)
		{
			max_h = 1088;
		}
	}
#endif
	
	stSize.u32Width = max_w;//csp modify 20140406
	stSize.u32Height = max_h;//csp modify 20140406
	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssFirstGrpIdx, open_chn_num, &stSize, VPSS_MAX_CHN_NUM, NULL);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("vpss start failed!\n");
		goto error1;
	}
	
	/******************************************
	change vo config: cvbs and hdmi
	******************************************/ 
	vio_set_vo_ch_attr_all(vdec_chn_num);
	
	for(i = 0; i < open_chn_num; i++)
	{
		VoChn = i;
		VpssGrp = i + s32VpssFirstGrpIdx;
		s32Ret = SAMPLE_COMM_VO_BindVpss(VO_DEVICE_HD, VoChn, VpssGrp, VPSS_PRE0_CHN);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
			goto error2;
		}
	}
	
	//system("cat /proc/umap/vpss");//csp modify 20140406
	
	/******************************************
	start vdec and bind it to vpss or vo
	******************************************/
	for(i = 0; i < open_chn_num; i++)
	{
		/*** create vdec chn ***/
		VdChn = i;
		printf("%s CreateVdecChn%d stSize u32Width: %d, u32Height: %d\n", __func__, i, stSize.u32Width, stSize.u32Height);
		s32Ret = SAMPLE_VDEC_CreateVdecChn(VdChn, &stSize, PT_H264);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("create vdec chn%d failed!\n", VdChn);
			goto error3;
		}
		else
		{
			LIB_PRT("create vdec chn%d success!\n", VdChn);
		}
		
		/*** bind vdec to vpss ***/
		VpssGrp = i + s32VpssFirstGrpIdx;
		s32Ret = SAMLE_COMM_VDEC_BindVpss(VdChn, VpssGrp);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("vdec(vdch=%d) bind vpss(vpssg=%d) failed!\n", VdChn, VpssGrp);
			goto error3;
		}		
	}
	
	memset((char *)&vdec_info, 0x00, sizeof(struct vdec_information));
	
	vdec_info.open = 1;
	vdec_info.speed_div = 1.0;
	vdec_info.base_speed = 25;
	for(i = 0; i < open_chn_num; i++)
	{
		vdec_info.dec_pts[i] = 0xFFFFFFFFFFFFFFFFLLU;
		vdec_info.dec_pts[i] /= 2;
		vdec_info.pre_pts[i] = 0;
		channel_show[i] = 1;
	}
	
#ifdef SUPPORT_AUDIO
	//start adec and ao
	ADEC_AO_REF_COUNT_LOCK();
	if(!adec_ao_reference_count)
	{
		if(HI_SUCCESS != tl_adec_ao_start())
		{
			adec_ao_reference_count--;
			LIB_PRT("tl_adec_ao_start FAIL\n");
		}
	}
	adec_ao_reference_count++;
	ADEC_AO_REF_COUNT_UNLOCK();
#endif
	
	goto out;
	
error3:
	for(i = 0; i < open_chn_num; i++)
	{
		VoChn = i;
		
		VdChn = i;
		VpssGrp = i + s32VpssFirstGrpIdx;
		
		SampleWaitDestroyVdecChn(VdChn);
		
		SAMLE_COMM_VDEC_UnBindVpss(VdChn, VpssGrp);
	}
	
error2:
	for(i = 0; i < open_chn_num; i++)
	{
		VoChn = i;
		VpssGrp = i + s32VpssFirstGrpIdx;
		SAMPLE_COMM_VO_UnBindVpss(VO_DEVICE_HD, VoChn);
	}
	
error1:
	SAMPLE_COMM_VPSS_Stop(s32VpssFirstGrpIdx, open_chn_num, 5);
	is_playing = 0;//csp modify 20140406
	
out:
	return s32Ret;
}

int tl_vdec_ioctl(int rsv, int cmd, void *arg)
{
	int ret = 0;
	int i;
	
	CHECK_OPENED;
	
	switch(cmd){
		case CMD_VDEC_PAUSE:{
			/*for(i = 0; i < open_chn_num; i++){
				ret = HI_MPI_VO_ChnPause(i);
				if(ret != 0){
					printf("<warning>lib_vdec.c@tl_vdec_ioctl: HI_MPI_VO_ChnPause fail, chn=%d\n", i);
					ret = -1;
				}
			}*/
			printf("<warning>lib_vdec.c@tl_vdec_ioctl: CMD_VDEC_PAUSE not apply\n");
		}break;
		case CMD_VDEC_RESUME:{
			/*for(i = 0; i < open_chn_num; i++){
				ret = HI_MPI_VO_ChnResume(i);
				if(ret != 0){
					printf("<warning>lib_vdec.c@tl_vdec_ioctl: HI_MPI_VO_ChnResume fail\n");
					ret = -1;
				}
			}*/
			printf("<warning>lib_vdec.c@tl_vdec_ioctl: CMD_VDEC_RESUME not apply\n");
		}break;
		case CMD_VDEC_STEP:{
			/*for(i = 0; i < open_chn_num; i++){
				ret = HI_MPI_VO_ChnStep(i);
				if(ret != 0){
					printf("<warning>lib_vdec.c@tl_vdec_ioctl: HI_MPI_VO_ChnStep fail\n");
					ret = -1;
				}
			}*/
			printf("<warning>lib_vdec.c@tl_vdec_ioctl: CMD_VDEC_STEP not apply\n");
		}break;
		case CMD_VDEC_SPEED_BASE:{
			vdec_info.base_speed = (int)arg;
			if(vdec_info.base_speed <= 10 || vdec_info.base_speed > 35)
				vdec_info.base_speed = 30;
			
			//csp modify
			if(!IS_PAL())
			{
				if(vdec_info.base_speed == 25)
				{
					vdec_info.base_speed = 30;
				}
			}
		}break;
		case CMD_VDEC_SPEED:{
			vdec_speed_e speed = (vdec_speed_e)arg;
			printf("vdec speed %d\n", speed);
			float div;
			
			if(speed == VDEC_SPEED_UP_8X){
				 div = 8.0;
				 vdec_buf_num = 16;
			}else if(speed == VDEC_SPEED_UP_4X){
				 div = 4.0;
				 vdec_buf_num = 8;
			}else if(speed == VDEC_SPEED_UP_2X){
				 div = 2.0;
				 vdec_buf_num = 6;
			}else if(speed == VDEC_SPEED_NORMAL){
				 div = 1.0;
				 vdec_buf_num = 4;
			}else if(speed == VDEC_SPEED_DOWN_2X){
				 div = 0.5;
				 vdec_buf_num = 1;
			}else if(speed == VDEC_SPEED_DOWN_4X){
				 div = 0.25;
				 vdec_buf_num = 1;
			}else if(speed == VDEC_SPEED_DOWN_8X){
				 div = 0.125;
				 vdec_buf_num = 1;
			}else if(speed == VDEC_SPEED_INVERSE){
				 div = 0.0;
				 vdec_buf_num = 1;
			}else{
				printf("not support this speed, speed=%d\n", speed);
				return -1;
			}
			
			vdec_info.speed_div = div;
			
			for(i = 0; i < open_chn_num; i++){
				ret = HI_MPI_VO_SetChnFrameRate(VO_DEVICE_HD, i, (div == 0.0) ? vdec_info.base_speed : vdec_info.base_speed*vdec_info.speed_div);
				if(ret != 0){
					printf("<warning>lib_vdec.c@tl_vdec_ioctl: 1, HI_MPI_VO_SetChnFrameRate fail\n");
					ret = -1;
				}
				#ifdef DHD1_WBC_DSD0
					//csp modify
					//printf("~~~~~~~~~~vdec use wbc2~~~~~~~\n");
				#else
					ret = HI_MPI_VO_SetChnFrameRate(VO_DEVICE_CVBS, i, (div == 0.0) ? vdec_info.base_speed : vdec_info.base_speed*vdec_info.speed_div);
					if(ret != 0){
						printf("<warning>lib_vdec.c@tl_vdec_ioctl: 2, HI_MPI_VO_SetChnFrameRate fail\n");
						ret = -1;
					}
				#endif
			}
			
			//printf("vdec_info.speed_div=%f\n", vdec_info.speed_div);
		}break;
		case CMD_VDEC_CHANNEL_SHOW:{
			int show_arg = (int)arg;
			int channel, show;
			int s32ret;
			channel = show_arg & 0xff;
			show = show_arg & (1<<8);
			if(open_chn_num == 1)
			{
				break;
			}
			if(channel >= open_chn_num || channel < 0)
			{
				printf("lib_vdec.c@tl_vdec_ioctl: CMD_VDEC_CHANNEL_SHOW error arg=%d\n", show_arg);
				break;
			}
			if(show)
			{
				if(channel_show[channel])
				{
					break;
				}
				s32ret = HI_MPI_VO_EnableChn(VO_DEVICE_HD, channel);
				if(HI_SUCCESS != s32ret)
				{
					printf("lib_vdec.c@tl_vdec_ioctl:1, CMD_VDEC_CHANNEL_SHOW HI_MPI_VO_EnableChn ret=%d\n", s32ret);
					break;
				}
				#ifndef DHD1_WBC_DSD0
				s32ret = HI_MPI_VO_EnableChn(VO_DEVICE_CVBS, channel);
				if(HI_SUCCESS != s32ret)
				{
					printf("lib_vdec.c@tl_vdec_ioctl:2, CMD_VDEC_CHANNEL_SHOW HI_MPI_VO_EnableChn ret=%d\n", s32ret);
					break;
				}
				#endif
				channel_show[channel] = 1;
			}
			else
			{
				if(channel_show[channel] == 0)
				{
					break;
				}
				s32ret = HI_MPI_VO_DisableChn(VO_DEVICE_HD, channel);
				if(HI_SUCCESS != s32ret)
				{
					printf("lib_vdec.c@tl_vdec_ioctl:1, CMD_VDEC_CHANNEL_SHOW HI_MPI_VO_DisableChn ret=%d\n", s32ret);
					break;
				}
				#ifndef DHD1_WBC_DSD0
				s32ret = HI_MPI_VO_DisableChn(VO_DEVICE_CVBS, channel);
				if(HI_SUCCESS != s32ret)
				{
					printf("lib_vdec.c@tl_vdec_ioctl:2, CMD_VDEC_CHANNEL_SHOW HI_MPI_VO_DisableChn ret=%d\n", s32ret);
					break;
				}
				#endif
				channel_show[channel] = 0;
			}
		}break;
		case CMD_VDEC_ZOOM:{
			#if 1
			int channel = (int)arg;
			if(open_chn_num == 1)
			{
				break;
			}
			zoom_in_channel(channel);
			#endif
		}break;
		default:{
			printf("lib_vdec.c@tl_vdec_ioctl: not support cmd=%d\n", cmd);
			break;
		}
	}
	
	return ret;
}

int tl_vdec_write(int chn, vdec_stream_s *pin_stream)
{
	VDEC_STREAM_S stStream;
	HI_S32 s32ret;
	
	CHECK_OPENED;
	
	if(open_chn_num == 1)
	{
		if(chn != 0)
		{
			printf("lib_vdec.c@tl_vdec_write: error chn=%d\n", chn);
			return -1;
		}
	}
	else
	{
		if(chn < 0 || chn >= open_chn_num)
		{
			printf("lib_vdec.c@tl_vdec_write: error chn=%d\n", chn);
			return -1;
		}
	}
	
	int w = (pin_stream->rsv >> 16) & 0xffff;
	int h = pin_stream->rsv & 0xffff;
	if(vdec_info.width[chn] != w)
	{
		vdec_info.width[chn] = w;
	}
	if(vdec_info.height[chn] != h)
	{
		vdec_info.height[chn] = h;
	}
	
	stStream.pu8Addr = pin_stream->data;
	stStream.u32Len = pin_stream->len;
	//stStream.u64PTS = pin_stream->pts;
	stStream.u64PTS = 0;
	
	#if 0
	//int yhr_count = 0;
	if(1)
	{
		VDEC_CHN_STAT_S stStat;
		//printf("###############HI_MPI_VDEC_Query########\n");
		while(1)
		{
			memset(&stStat, 0, sizeof(stStat));
			s32ret = HI_MPI_VDEC_Query(chn, &stStat);
			if(s32ret != HI_SUCCESS)
			{
				printf("line:%d,HI_MPI_VDEC_Query failed errno 0x%x \n", __LINE__, s32ret);
				break;
			}
			if((int)(stStat.u32LeftStreamFrames) < vdec_buf_num)
			{
				break;
			}
			//printf("###########%d, %d, %u\n", stStat.u32LeftStreamFrames, vdec_buf_num, stStream.u32Len);
			//break;
			//if(yhr_count == 10)
			//{
			//	break;
			//}
			//yhr_count++;
			usleep(4000);
		}
	}
	#endif
	
	#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	if(g_vdec_zoom_flag)
	{
		if(chn != g_vdec_zoom_chn)
		{
			//return pin_stream->len;
		}
	}
	#endif
		
#if 0//csp modify 20150110
	int yhr_count = 0;
	while(1)
	{
		s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, HI_IO_BLOCK);
		if(HI_SUCCESS != s32ret)
		{
			if(s32ret == 0xa005800f)
			{
				yhr_count++;
				if(yhr_count == 40)
				{
					printf("function:%s,chn:%d,line:%d,send to vdec (0x%x:HI_ERR_VDEC_BUF_FULL)\n", __FUNCTION__, chn, __LINE__, s32ret);//csp modify 20150110
					return -1;
				}
				usleep(1000);
			}
			else
			{
				//printf("function:%s,chn:%d,line:%d,send to vdec 0x%x\n", __FUNCTION__, chn, __LINE__, s32ret);//csp modify 20150110
				return -1;
			}
		}
		else
		{
			break;
		}
	}
#else
	//s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, HI_IO_BLOCK);
	s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, -1);
	if(HI_SUCCESS != s32ret)
	{
		printf("function:%s,chn:%d,line:%d,send to vdec 0x%x\n", __FUNCTION__, chn, __LINE__, s32ret);//csp modify 20150110
		return -1;
	}
#endif
	
	return pin_stream->len;
}

void tl_vdec_close(void)
{
	VDEC_CHN VdChn;
	VPSS_GRP VpssGrp;
	VO_CHN VoChn;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_S32 s32VpssFirstGrpIdx = 0;
	int i;
	
	is_playing = 0;//csp modify 20140406
	
	//最新的SDK还需要这段代码吗???
	#if 0
	if(ARG_VI_NUM_MAX > 16)
	{
		s32VpssFirstGrpIdx = 0;
	}
	else
	{
		s32VpssFirstGrpIdx = ARG_VI_NUM_MAX;
	}
	#endif
	
	if(g_vdec_elec_zoom_flag)
	{
		pre_rect_s stCapRect;
		memset(&stCapRect,0,sizeof(stCapRect));
		tl_vdec_eletroinc_zoom(PREVIEW_EA_CLOSE,g_vdec_zoom_chn,stCapRect);
	}
	g_vdec_zoom_flag = 0;
	g_vdec_zoom_chn = -1;
	
	memset((char *)&vdec_info, 0x00, sizeof(struct vdec_information));
	
	//printf("tl_vdec_close: here-1\n");
	
	for(i = 0; i < open_chn_num; i++)
	{
	#ifdef HI3535
		HI_MPI_VO_ResumeChn(VO_DEVICE_HD, i);
	#else
		#ifndef DHD1_WBC_DSD0
		HI_MPI_VO_ChnResume(VO_DEVICE_CVBS, i);
		#endif
		HI_MPI_VO_ChnResume(VO_DEVICE_HD, i);
	#endif
		
		SampleWaitDestroyVdecChn(i);
		
		VdChn = i;
		VpssGrp = i + s32VpssFirstGrpIdx;
		VoChn = i;
		
		#ifdef DHD1_WBC_DSD0
		//csp modify
		//printf("~~~~~~~~~~vdec use wbc3~~~~~~~\n");
		#else
		s32Ret = SAMLE_COMM_VDEC_UnBindVo(VdChn, VO_DEVICE_CVBS, VoChn);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("SAMLE_COMM_VDEC_UnBindVo failed!\n");
		}
		#endif
		
		s32Ret = SAMLE_COMM_VDEC_UnBindVpss(VdChn, VpssGrp);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("SAMLE_COMM_VDEC_UnBindVpss failed!\n");
		}
		
		s32Ret = SAMPLE_COMM_VO_UnBindVpss(VO_DEVICE_HD, VoChn);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("SAMPLE_COMM_VO_UnBindVpss failed!\n");
		}
		
		channel_show[i] = 0;
	}
	
	s32Ret = SAMPLE_COMM_VPSS_Stop(s32VpssFirstGrpIdx, open_chn_num, 5);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VPSS_Stop failed!\n");
	}
	
#ifdef SUPPORT_AUDIO
	//stop adec and ao
	ADEC_AO_REF_COUNT_LOCK();
	adec_ao_reference_count--;
	if(!adec_ao_reference_count)
	{
		if(HI_SUCCESS != tl_adec_ao_stop())
		{
			LIB_PRT("tl_adec_ao_stop FAIL\n");
		}
	}
	ADEC_AO_REF_COUNT_UNLOCK();
#endif
	
	return;
}

int tl_vdec_flush(int chn)
{
	VDEC_CHN_STAT_S stStat;
	HI_S32 s32ret;
	unsigned int old_left;
	int time_out;
	
	if(!vdec_info.open)
	{
		printf("lib_vdec.c@tl_vdec_flush: not open\n");
		return -1;
	}
	
	if(chn < 0 || chn >= open_chn_num)
	{
		printf("lib_vdec.c@tl_vdec_flush: error chn=%d\n", chn);
		return -1;
	}
	
	old_left = 0;
	time_out = 10;//40;//100;
	while(1)
	{
		s32ret = HI_MPI_VDEC_Query(chn, &stStat);
		if(s32ret != HI_SUCCESS)
		{
			printf("lib_vdec.c@tl_vdec_flush: HI_MPI_VDEC_Query, ret=0x%x\n", s32ret);
			return -1;
		}
		
		#if 1//csp modify
		if(stStat.u32LeftStreamBytes <= 8)
		{
			break;
		}
		else if(old_left == stStat.u32LeftStreamBytes)
		{
			if(--time_out == 0)
			{
				printf("lib_vdec.c@rz_vdec_flush: time out\n");
				return -1;
			}
		}
		else
		{
			old_left = stStat.u32LeftStreamBytes;
			time_out = 10;//40;//100;
		}
		#else
		if(stStat.u32LeftStreamFrames == 0)
		{
			break;
		}
		else if(old_left == stStat.u32LeftStreamFrames)
		{
			if(--time_out == 0)
			{
				printf("lib_vdec.c@tl_vdec_flush: time out\n");
				return -1;
			}
		}
		else
		{
			old_left = stStat.u32LeftStreamFrames;
			time_out = 100;
		}
		#endif
		
		//usleep(5000);
		usleep(10000);
	}
	
	return 0;
}

int tl_vdec_eletroinc_zoom(int eaFlag, int firstCh, pre_rect_s stCapRect)
{
#if 1
	//printf("tl_vdec_eletroinc_zoom start:(chn=%d,flag=%d)......\n",firstCh,eaFlag);
	
	if(lib_gbl_info.last_pre_mode != PREVIEW_CLOSE || !g_vdec_zoom_flag || g_vdec_zoom_chn != firstCh)
	{
		printf("vdec eletroinc zoom:param error-1\n");
		return -1;
	}
	
	VO_DEV VoDev = VO_DEVICE_HD;
	
#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	VO_CHN VoChn = firstCh;//PIP
	VO_CHN VoChn_Clip = open_chn_num;//ZOOM
	
	#ifdef HI3535
	VPSS_GRP VpssGrp = firstCh;//PIP
	#endif
	VPSS_GRP VpssGrp_Clip = open_chn_num;//ZOOM
#else
	HI_S32 s32VpssFirstGrpIdx = ARG_VO_NUM_MAX;//32;
	HI_S32 s32VpssGrpCnt = 1;
	
	VPSS_GRP VpssGrp = firstCh;//firstCh+ARG_VI_NUM_MAX;//PIP
	VPSS_GRP VpssGrp_Clip = ARG_VO_NUM_MAX;//32;//ZOOM
	
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
	
	SIZE_S stSize;
	
	HI_S32 s32Ret = HI_SUCCESS;
	
	//最新的SDK还需要这段代码吗???
	#if 0
	if(ARG_VI_NUM_MAX > 16)
	{
		s32VpssFirstGrpIdx = 31;
		VpssGrp_Clip = 31;
		VpssGrp = firstCh;
	}
	#endif
	
	stSize.u32Width = vdec_info.width[firstCh];
	stSize.u32Height = vdec_info.height[firstCh];
	
#ifdef HI3535
	if(eaFlag == PREVIEW_EA_OPEN && !g_vdec_elec_zoom_flag)
	{
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
		//goto END_1D1_CLIP_1;
		goto END_1D1_CLIP_5;
	}
	
	if(eaFlag == PREVIEW_EA_CLOSE)
	{
		//printf("tl_vdec_eletroinc_zoom close......\n");
		if(g_vdec_elec_zoom_flag)
		{
			g_vdec_elec_zoom_flag = 0;
			goto END_1D1_CLIP_5;
		}
		else
		{
			//printf("tl_vdec_eletroinc_zoom:already closed!!!\n");
			return -1;
		}
	}
	
#ifdef HI3535
	if(vdec_info.width[firstCh] == 0)
	{
		printf("vdec eletroinc zoom windows:no video\n");
		return -1;
	}
#endif
	
	if(stCapRect.u32Width < 32)
	{
		stCapRect.u32Width = 32;
	}
	if(stCapRect.u32Height < 16)
	{
		stCapRect.u32Height = 16;
	}
	
	//printf("1-vdec elec zoom rect:(%d,%d,%d,%d),frame width:%d height:%d\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height,stSize.u32Width,stSize.u32Height);
	
	stCapRect.s32X = stSize.u32Width*stCapRect.s32X/720;
	stCapRect.s32Y = stSize.u32Height*stCapRect.s32Y/(IS_PAL()?576:480);
	stCapRect.u32Width = stSize.u32Width*stCapRect.u32Width/720;
	stCapRect.u32Height = stSize.u32Height*stCapRect.u32Height/(IS_PAL()?576:480);
	
	//printf("2-vdec elec zoom rect:(%d,%d,%d,%d)\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height);
	
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
	
	//printf("3-vdec elec zoom rect:(%d,%d,%d,%d)\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height);
	
	if(eaFlag == PREVIEW_EA_OPEN && g_vdec_elec_zoom_flag)
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
		
		//printf("vdec elec zoom rect:(%d,%d,%d,%d)\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height);
		
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
	
#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	printf("rz_vdec_eletroinc_zoom open,VpssGrp_Clip=%d......\n",VpssGrp_Clip);
#else
	//printf("tl_vdec_eletroinc_zoom open,s32VpssFirstGrpIdx=%d......\n",s32VpssFirstGrpIdx);
	
	if(g_vdec_elec_zoom_flag)
	{
		SAMPLE_COMM_VO_UnBindVpss(VO_DEVICE_HD,0);
		SAMPLE_COMM_VO_UnBindVpss(VO_DEVICE_HD,1);
		//printf("vio_unbind_vi2vo_all [2] finish\n");
	}
	else
	{
		SAMPLE_COMM_VO_UnBindVpss(VO_DEVICE_HD,0);
		//printf("vio_unbind_vi2vo_all [1] finish\n");
	}
#endif
	
	HI_MPI_VO_SetAttrBegin(VoDev);
	
#ifdef HI3535
	//printf("HI3535_vdec_eletroinc_zoom-1\n");
	HI_MPI_VO_SetAttrBegin(SAMPLE_VO_LAYER_VPIP);
	SAMPLE_COMM_VO_UnBindVpss(VoDev, VoChn);
	HI_MPI_VO_DisableChn(VoDev, VoChn);
	//printf("HI3535_vdec_eletroinc_zoom-2\n");
#endif
	
#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	//HI_MPI_VO_ChnHide(VoDev, VoChn_Clip);
#else
	HI_MPI_VO_DisableChn(VoDev, VoChn);
	HI_MPI_VO_DisableChn(VoDev, VoChn_Clip);
#endif
	
	if(!g_vdec_elec_zoom_flag)
	{
		//csp modify 20140406
		int k = 0;
		for(k = 0; k < open_chn_num; k++)
		{
			if(k != firstCh)
			{
				SAMPLE_COMM_VPSS_Stop(k, 1, VPSS_MAX_CHN_NUM);
			}
		}
		
		//csp modify 20140406
		int max_w = VDEC_MAX_W;
		int max_h = VDEC_MAX_H;
	#ifdef HI3520D
		if(open_chn_num > 1)
		{
			if(max_h > 1088)
			{
				max_h = 1088;
			}
		}
	#endif
		
		stSize.u32Width = max_w;//csp modify 20140406
		stSize.u32Height = max_h;//csp modify 20140406
	#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
		s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp_Clip, 1, &stSize, VPSS_MAX_CHN_NUM, NULL);
	#else
		s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssFirstGrpIdx, s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, NULL);
	#endif
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
		//printf("HI3535_vdec_eletroinc_zoom-3\n");
		
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
	
	g_vdec_elec_zoom_flag = 1;
	
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
	
	//printf("vdec elec zoom rect:(%d,%d,%d,%d)\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height);
	
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
	#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	//HI_MPI_VO_ChnShow(VoDev, VoChn);
	#else
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
	
#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	//HI_MPI_VO_ChnShow(VoDev, VoChn_Clip);
#endif
	
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
	#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	//printf("tl_vdec_eletroinc_zoom bind???\n");
	#else
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
	
	//printf("vdec elec zoom over@@@@@@\n");
	
	return 0;
	
	/******************************************
	exit process
	******************************************/
END_1D1_CLIP_5:
#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	SAMPLE_COMM_VO_UnBindVpss(VoDev, VoChn_Clip);
	HI_MPI_VO_DisableChn(VoDev, VoChn_Clip);
	#ifdef HI3535
	SAMPLE_COMM_VO_UnBindVpss(SAMPLE_VO_LAYER_VPIP, VoChn);
	HI_MPI_VO_DisableChn(SAMPLE_VO_LAYER_VPIP, VoChn);
	#endif
#else
	//HI_MPI_VDEC_StopRecvStream(firstCh);//csp modify
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
	#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	SAMPLE_COMM_VPSS_Stop(VpssGrp_Clip, 1, VPSS_MAX_CHN_NUM);
	#else
	SAMPLE_COMM_VPSS_Stop(s32VpssFirstGrpIdx, s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
	#endif
//END_1D1_CLIP_1:
	#ifdef VDEC_PAUSE_ZOOM_DEBUG//csp modify 20140406
	//占整个屏幕
	stChnAttr.stRect.u32Width	= ALIGN_BACK(stPipLayerAttr.stDispRect.u32Width, 4);
	stChnAttr.stRect.u32Height	= ALIGN_BACK(stPipLayerAttr.stDispRect.u32Height, 4);
	stChnAttr.stRect.s32X		= ALIGN_BACK(0, 4);
	stChnAttr.stRect.s32Y		= ALIGN_BACK(0, 4);
	stChnAttr.u32Priority		= 0;//原视频层
	stChnAttr.bDeflicker		= HI_TRUE;//HI_FALSE;
	HI_S32 sRet = HI_MPI_VO_SetChnAttr(VoDev, VoChn, &stChnAttr);
	if(HI_SUCCESS != sRet)
	{
		LIB_PRT("HI_MPI_VO_SetChnAttr failed:0x%08x!\n",sRet);
	}
	
	#ifdef HI3535
	HI_MPI_VO_EnableChn(VoDev, VoChn);
	SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VPSS_PRE0_CHN);
	#endif
	
	//csp modify 20140406
	int k = 0;
	for(k = 0; k < open_chn_num; k++)
	{
		if(k != firstCh)
		{
			//csp modify 20140406
			int max_w = VDEC_MAX_W;
			int max_h = VDEC_MAX_H;
		#ifdef HI3520D
			if(open_chn_num > 1)
			{
				if(max_h > 1088)
				{
					max_h = 1088;
				}
			}
		#endif
			
			stSize.u32Width = max_w;//csp modify 20140406
			stSize.u32Height = max_h;//csp modify 20140406
			
			SAMPLE_COMM_VPSS_Start(k, 1, &stSize, VPSS_MAX_CHN_NUM, NULL);
		}
	}
	#else
	//zoom_in_channel(firstCh);
	
	vio_set_vo_ch_attr_all(1);
	//vio_bind_vi2vo_all(firstCh, 1, VoChn, VpssGrp, 0);
	SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VPSS_PRE0_CHN);
	#endif
	
	g_vdec_elec_zoom_flag = 0;
	
	if(s32Ret == HI_SUCCESS)
	{
		printf("vdec elec zoom close\n");
	}
	else
	{
		printf("vdec elec zoom failed\n");
	}
	
	return s32Ret;
#endif
}

static unsigned char g_nvr_preview_vdec_open_flag = 0;
static unsigned int g_nvr_preview_vdec_chn_num = 0;
static unsigned int g_nvr_preview_vdec_first_chn = 0;

unsigned char is_nvr_preview_vdec_open(void)
{
	return g_nvr_preview_vdec_open_flag;
}

int nvr_preview_vdec_req_iframe(int vdec_first_chn, int vdec_chn_num)
{
	//return 0;
	
	HI_S32 i = 0;
	
	if(vdec_first_chn + vdec_chn_num > ARG_VI_NUM_MAX)
	{
		vdec_chn_num = ARG_VI_NUM_MAX - vdec_first_chn;
		if(vdec_chn_num < 0)
		{
			vdec_chn_num = 0;
		}
	}
	
	int nvr_first_chn = vdec_first_chn;
	int open_chn_num = vdec_chn_num;
	
	int n = 0;
	for(n = 0; n < 1; n++)
	{
		//usleep(1);
		
		for(i = nvr_first_chn; i < (nvr_first_chn + open_chn_num); i++)
		{
			if(open_chn_num > plib_gbl_info->preview_chn_threshold)
			{
				IPC_CMD_RequestIFrame(i+ARG_VI_NUM_MAX);
			}
			else
			{
				IPC_CMD_RequestIFrame(i);
			}
		}
	}
	
	return 0;
}

//csp modify 20140406
extern unsigned char is_nvr_third_stream_open(int chn);

#ifdef FAST_SWITCH_PREVIEW
int nvr_preview_vdec_chn_recreate(int chn, int type)
{
	#ifdef DDR4G
	
	//yaogang modify 最大尺寸
	plib_gbl_info->byDecoderType[chn] = type;
	return 1;
	
	#endif
	
	if(!is_nvr_preview_vdec_open())
	{
		return 0;
	}
	
	if(chn < 0 || chn >= ARG_VI_NUM_MAX)
	{
		return 0;
	}
	
	if(chn < g_nvr_preview_vdec_first_chn || chn >= (g_nvr_preview_vdec_first_chn + g_nvr_preview_vdec_chn_num))
	{
		return 0;
	}
	
	if(plib_gbl_info->byDecoderType[chn] == type)
	{
		return 1;
	}
	
	//csp modify 20150110
	//unsigned char reset_vochn = 0;
	
	//csp modify 20140406
	unsigned char restart_thirdstream = 0;
	
	if(plib_gbl_info->byDecoderType[chn])
	{
		//csp modify 20140406
		if(is_nvr_third_stream_open(chn))
		{
			tl_venc_third_stop(chn);
			restart_thirdstream = 1;
		}
		
		//csp modify 20150110
		//VO_CHN VoChn = chn;
		//HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);//HI_TRUE//HI_FALSE
		//HI_MPI_VO_PauseChn(VO_DEVICE_HD, VoChn);
		
		SAMLE_COMM_VDEC_UnBindVpss(chn, chn);
		
		#if 0
		//csp modify 20150110
		if(type == 1)
		{
			SAMPLE_COMM_VO_UnBindVpss(VO_DEVICE_HD, chn);//csp modify 20150110
			HI_MPI_VO_HideChn(VO_DEVICE_HD, chn);//csp modify 20150110
			HI_MPI_VO_DisableChn(VO_DEVICE_HD, chn);//csp modify 20150110
			
			reset_vochn = 1;
		}
		#endif
		
		//csp modify 20150110
		//VPSS_GRP VpssGrp = chn;
		//HI_MPI_VPSS_ResetGrp(VpssGrp);
		
		SampleWaitDestroyVdecChn(chn);
		
		SAMPLE_COMM_VPSS_Stop(chn, 1, VPSS_MAX_CHN_NUM);
		
		plib_gbl_info->byDecoderType[chn] = 0;
	}
	
	if(type == 0)
	{
		return 1;
	}
	
	SIZE_S stSize;
	if(type == 2)
	{
		stSize.u32Width = 768;//768;//720;
		stSize.u32Height = 576;
	}
	else
	{
		stSize.u32Width = VDEC_MAX_W;
		stSize.u32Height = VDEC_MAX_H;
	}
	
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = SAMPLE_COMM_VPSS_Start(chn, 1, &stSize, VPSS_MAX_CHN_NUM, NULL);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("vpss start failed!\n");
		return 0;
	}
	
	s32Ret = SAMPLE_VDEC_CreateVdecChn(chn, &stSize, PT_H264);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("recreate vdec chn[%d] for nvr preview failed!\n", chn);
		SAMPLE_COMM_VPSS_Stop(chn, 1, VPSS_MAX_CHN_NUM);
		return 0;
	}
	else
	{
		//LIB_PRT("recreate vdec chn[%d] for nvr preview success!\n", chn);
	}
	
	//csp modify 20150110
	//VPSS_GRP VpssGrp = chn;
	//HI_MPI_VPSS_ResetGrp(VpssGrp);
	//VO_CHN VoChn = chn;
	//HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);//HI_TRUE//HI_FALSE
	
	#if 0
	//csp modify 20150110
	if(reset_vochn)
	{
		int pre_mode = plib_gbl_info->last_pre_mode;
		int first_Chn = plib_gbl_info->last_mode_arg;
		
		int u32WndNum = 16;
		int u32Square = 4;
		switch(pre_mode)
		{
		case PREVIEW_1D1:
			u32WndNum = 1;
			u32Square = 1;
			break;
		case PREVIEW_4CIF:
			u32WndNum = 4;
			u32Square = 2;
			break;
		case PREVIEW_9CIF:
			u32WndNum = 9;
			u32Square = 3;
			break;
		case PREVIEW_16CIF:
			u32WndNum = 16;
			u32Square = 4;
			break;
		case PREVIEW_25CIF:
			u32WndNum = 25;
			u32Square = 5;
			break;
		case PREVIEW_36CIF:
			u32WndNum = 36;
			u32Square = 6;
			break;
		default:
			u32WndNum = 16;
			u32Square = 4;
			break;
		}
		
		HI_U32 u32Width = 0;
		HI_U32 u32Height = 0;
		HI_U32 u32Frm = 0;
		SAMPLE_COMM_VO_GetWH(g_enIntfSync, &u32Width, &u32Height, &u32Frm);
		
		VO_CHN_ATTR_S stChnAttr;
		if(chn < first_Chn || chn >= (first_Chn + u32WndNum))
		{
			s32Ret = HI_MPI_VO_HideChn(VO_DEVICE_HD, chn);//csp modify 20150110
			if(HI_SUCCESS != s32Ret)
			{
				LIB_PRT("nvr preview HI_MPI_VO_HideChn(VoChn=%d) failed:0x%x!\n", chn, s32Ret);
			}
			
			stChnAttr.stRect.s32X       = ALIGN_BACK((u32Width/u32Square) * ((chn%u32WndNum)%u32Square), 2);
			stChnAttr.stRect.s32Y       = ALIGN_BACK((u32Height/u32Square) * ((chn%u32WndNum)/u32Square), 2);
		}
		else
		{
			s32Ret = HI_MPI_VO_ShowChn(VO_DEVICE_HD, chn);//csp modify 20150110
			if(HI_SUCCESS != s32Ret)
			{
				LIB_PRT("nvr preview HI_MPI_VO_ShowChn(VoChn=%d) failed:0x%x!\n", chn, s32Ret);
			}
			
			stChnAttr.stRect.s32X       = ALIGN_BACK((u32Width/u32Square) * ((chn-first_Chn)%u32Square), 2);
			stChnAttr.stRect.s32Y       = ALIGN_BACK((u32Height/u32Square) * ((chn-first_Chn)/u32Square), 2);
		}
		stChnAttr.stRect.u32Width   = ALIGN_BACK(u32Width/u32Square, 2);
		stChnAttr.stRect.u32Height  = ALIGN_BACK(u32Height/u32Square, 2);
		stChnAttr.u32Priority       = 0;
		stChnAttr.bDeflicker        = HI_TRUE;
		s32Ret = HI_MPI_VO_SetChnAttr(VO_DEVICE_HD, chn, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):HI_MPI_VO_SetChnAttr(VoChn=%d) rect:(%d %d %d %d) preview:(%d %d) failed with %#x!\n", 
				__FUNCTION__, __LINE__, 
				chn, 
				stChnAttr.stRect.s32X, stChnAttr.stRect.s32Y, 
				stChnAttr.stRect.u32Width, stChnAttr.stRect.u32Height, 
				plib_gbl_info->last_pre_mode, lib_gbl_info.last_mode_arg, 
				s32Ret);
			SAMPLE_COMM_VPSS_Stop(chn, 1, VPSS_MAX_CHN_NUM);
			SampleWaitDestroyVdecChn(chn);
			return HI_FAILURE;
		}
		
		s32Ret = HI_MPI_VO_EnableChn(VO_DEVICE_HD, chn);//csp modify 20150110
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("nvr preview HI_MPI_VO_EnableChn(VoChn=%d) failed:0x%x!\n", chn, s32Ret);
			SAMPLE_COMM_VPSS_Stop(chn, 1, VPSS_MAX_CHN_NUM);
			SampleWaitDestroyVdecChn(chn);
			return 0;
		}
		
		SAMPLE_COMM_VO_BindVpss(VO_DEVICE_HD, chn, chn, VPSS_PRE0_CHN);//csp modify 20150110
	}
	#endif

	#if 1
	//yaogang modify 20150921
	//解决按帧解码预览在切换码流时卡的时间较长(10s)
	
	s32Ret = HI_MPI_VPSS_ResetGrp(chn);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("nvr HI_MPI_VPSS_ResetGrp(vdch=%d) failed!\n", chn);
		SAMPLE_COMM_VPSS_Stop(chn, 1, VPSS_MAX_CHN_NUM);
		SampleWaitDestroyVdecChn(chn);
		return 0;
	}
	
	s32Ret = HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, chn, HI_TRUE);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("nvr HI_MPI_VO_ClearChnBuffer(voch=%d) failed!\n", chn);
		SAMPLE_COMM_VPSS_Stop(chn, 1, VPSS_MAX_CHN_NUM);
		SampleWaitDestroyVdecChn(chn);
		return 0;
	}
	#endif
	
	s32Ret = SAMLE_COMM_VDEC_BindVpss(chn, chn);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("nvr preview vdec(vdch=%d) bind vpss(vpssg=%d) failed!\n", chn, chn);
		SAMPLE_COMM_VPSS_Stop(chn, 1, VPSS_MAX_CHN_NUM);
		SampleWaitDestroyVdecChn(chn);
		return 0;
	}
	
	//csp modify 20150110
	//VO_CHN VoChn = chn;
	//HI_MPI_VO_ResumeChn(VO_DEVICE_HD, VoChn);
	
	if(type == 2)
	{
		plib_gbl_info->bySubWaitIFrame[chn] = 1;
	}
	else
	{
		plib_gbl_info->byMainWaitIFrame[chn] = 1;
	}
	
	plib_gbl_info->byDecoderType[chn] = type;
	
	//csp modify 20140406
	if(restart_thirdstream)
	{
		venc_parameter_t para;
		para.bit_rate = 64;
		para.frame_rate = 5;//25;
		para.gop = 20;
		para.is_cbr = 1;
		para.pic_level = 0;
		tl_venc_third_start(chn, VENC_SIZE_CIF, &para);
		restart_thirdstream = 0;
	}
	
	return 1;
}
#endif

int nvr_preview_vdec_open(int vdec_first_chn, int vdec_chn_num)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_S32 i = 0;
	VDEC_CHN VdChn;
	VPSS_GRP VpssGrp;
	SIZE_S stSize;
	
	if(vdec_first_chn + vdec_chn_num > ARG_VI_NUM_MAX)
	{
		vdec_chn_num = ARG_VI_NUM_MAX - vdec_first_chn;
		if(vdec_chn_num < 0)
		{
			vdec_chn_num = 0;
		}
	}
	
	int nvr_first_chn = vdec_first_chn;
	int open_chn_num = vdec_chn_num;
	
	#ifndef DDR4G //0 yaogang modify 最大尺寸 
	if(open_chn_num > plib_gbl_info->preview_chn_threshold)
	{
		stSize.u32Width = 768;//768;//720;
		stSize.u32Height = 576;
	}
	else
	#endif
	{
		stSize.u32Width = VDEC_MAX_W;
		stSize.u32Height = VDEC_MAX_H;
	}

	printf("nvr_first_chn: %d, open_chn_num: %d, VPSS_MAX_CHN_NUM: %d\n",
		nvr_first_chn, open_chn_num, VPSS_MAX_CHN_NUM);
	s32Ret = SAMPLE_COMM_VPSS_Start(nvr_first_chn, open_chn_num, &stSize, VPSS_MAX_CHN_NUM, NULL);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("vpss start failed!\n");
		goto error1;
	}
	
	//ipc_unit ipcam;//csp modify 20150110
	
	for(i = nvr_first_chn; i < (nvr_first_chn + open_chn_num); i++)
	{
		//csp modify 20140406
		unsigned char restart_thirdstream = 0;
		int chn = i;
		
		//csp modify 20140406
		if(is_nvr_third_stream_open(chn))
		{
			tl_venc_third_stop(chn);
			restart_thirdstream = 1;
		}
		
		/*** create vdec chn ***/
		VdChn = i;
		//yaogang 
		printf("%s CreateVdecChn%d stSize u32Width: %d, u32Height: %d\n", __func__, VdChn, stSize.u32Width, stSize.u32Height);
		s32Ret = SAMPLE_VDEC_CreateVdecChn(VdChn, &stSize, PT_H264);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("create vdec chn[%d] for nvr preview failed!\n", VdChn);
			goto error1;
		}
		else
		{
			//LIB_PRT("create vdec chn[%d] for nvr preview success!\n", VdChn);
		}
		
		if(open_chn_num > plib_gbl_info->preview_chn_threshold)
		{
			plib_gbl_info->bySubWaitIFrame[i] = 1;
		}
		else
		{
			plib_gbl_info->byMainWaitIFrame[i] = 1;
		}
		
		/*** bind vdec to vpss ***/
		VpssGrp = i;
		s32Ret = SAMLE_COMM_VDEC_BindVpss(VdChn, VpssGrp);
		if(HI_SUCCESS != s32Ret)
		{
			LIB_PRT("nvr preview vdec(vdch=%d) bind vpss(vpssg=%d) failed!\n", VdChn, VpssGrp);
			goto error1;
		}
		
		#ifdef FAST_SWITCH_PREVIEW
		
		if(open_chn_num > plib_gbl_info->preview_chn_threshold)
		{
			plib_gbl_info->byDecoderType[i] = 2;
		}
		else
		{
			plib_gbl_info->byDecoderType[i] = 1;
		}
		
		#endif
		
		//csp modify 20140406
		if(restart_thirdstream)
		{
			venc_parameter_t para;
			para.bit_rate = 64;
			para.frame_rate = 5;//25;
			para.gop = 20;
			para.is_cbr = 1;
			para.pic_level = 0;
			tl_venc_third_start(chn, VENC_SIZE_CIF, &para);
			restart_thirdstream = 0;
		}
	}
	
	//LIB_PRT("nvr_preview_vdec_open success!\n");
	
	g_nvr_preview_vdec_first_chn = nvr_first_chn;
	g_nvr_preview_vdec_chn_num = open_chn_num;
	g_nvr_preview_vdec_open_flag = 1;
	
	return 0;
	
#if 0
	int n = 0;
	for(n = 0; n < 1; n++)
	{
		//usleep(1);
		
		for(i = nvr_first_chn; i < (nvr_first_chn + open_chn_num); i++)
		{
			if(open_chn_num > plib_gbl_info->preview_chn_threshold)
			{
				IPC_CMD_RequestIFrame(i+ARG_VI_NUM_MAX);
			}
			else
			{
				IPC_CMD_RequestIFrame(i);
			}
		}
	}
	
	return 0;
#endif
	
error1:
	LIB_PRT("nvr_preview_vdec_open failed!\n");
	
	for(i = nvr_first_chn; i < (nvr_first_chn + open_chn_num); i++)
	{
		VdChn = i;
		VpssGrp = i;
		
		SampleWaitDestroyVdecChn(VdChn);
		
		SAMLE_COMM_VDEC_UnBindVpss(VdChn, VpssGrp);
		
		#ifdef FAST_SWITCH_PREVIEW
		plib_gbl_info->byDecoderType[i] = 0;
		#endif
	}
	
	SAMPLE_COMM_VPSS_Stop(nvr_first_chn, open_chn_num, VPSS_MAX_CHN_NUM);
	
	return -1;
}

int nvr_preview_vdec_close(void)
{
	HI_S32 i = 0;
	VDEC_CHN VdChn;
	VPSS_GRP VpssGrp;
	
	g_nvr_preview_vdec_open_flag = 0;
	
	int nvr_first_chn = g_nvr_preview_vdec_first_chn;
	int open_chn_num = g_nvr_preview_vdec_chn_num;
	
	for(i = nvr_first_chn; i < (nvr_first_chn + open_chn_num); i++)
	{
		VdChn = i;
		VpssGrp = i;
		
		//csp modify 20140406
		int chn = i;
		if(is_nvr_third_stream_open(chn))
		{
			tl_venc_third_stop(chn);
			plib_gbl_info->venc[chn].venc_third_start = 1;
		}
		
		SampleWaitDestroyVdecChn(VdChn);
		
		SAMLE_COMM_VDEC_UnBindVpss(VdChn, VpssGrp);
		
		#ifdef FAST_SWITCH_PREVIEW
		plib_gbl_info->byDecoderType[i] = 0;
		#endif
		
		//LIB_PRT("destroy vdec chn[%d] for nvr preview success!\n", VdChn);
	}
	
	HI_S32 s32Ret = SAMPLE_COMM_VPSS_Stop(nvr_first_chn, open_chn_num, VPSS_MAX_CHN_NUM);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VPSS_Stop failed!\n");
	}
	
	g_nvr_preview_vdec_chn_num = 0;
	g_nvr_preview_vdec_first_chn = 0;
	
	//LIB_PRT("nvr_preview_vdec_close success!\n");
	
	return 0;
}

unsigned char is_nvr_preview_chn_open(int chn)
{
	if(!is_nvr_preview_vdec_open())
	{
		//printf("lib_vdec.c@is_nvr_preview_chn_open: vdec not open\n");
		return 0;
	}
	
	if(chn < 0 || chn >= ARG_VI_NUM_MAX)
	{
		printf("lib_vdec.c@is_nvr_preview_chn_open: error chn=<%d>\n", chn);
		return 0;
	}
	
	if(chn < g_nvr_preview_vdec_first_chn || chn >= (g_nvr_preview_vdec_first_chn + g_nvr_preview_vdec_chn_num))
	{
		//printf("lib_vdec.c@is_nvr_preview_chn_open: error chn=[%d]\n", chn);
		return 0;
	}
	
	int u32WndNum = 0;
	int last_pre_mode = lib_gbl_info.last_pre_mode;
	switch(last_pre_mode)
	{
		case PREVIEW_1D1:
			u32WndNum = 1;
			break;
		case PREVIEW_2PIC:
			u32WndNum = 2;
			break;
		case PREVIEW_4CIF:
			u32WndNum = 4;
			break;
		case PREVIEW_9CIF:
			u32WndNum = 9;
			break;
		case PREVIEW_10PIC:
			u32WndNum = 10;
			break;
		case PREVIEW_16CIF:
			u32WndNum = 16;
			break;
		case PREVIEW_25CIF:
			u32WndNum = 25;
			break;
		case PREVIEW_36CIF:
			u32WndNum = 36;
			break;
		case PREVIEW_CLOSE:
			//printf("lib_vdec.c@is_nvr_preview_chn_open: preview closed\n");
			return 0;
	}
	//printf("%s: last_pre_mode=%d\n", __func__, lib_gbl_info.last_pre_mode);
	
	if(chn < lib_gbl_info.last_mode_arg || chn >= (lib_gbl_info.last_mode_arg + u32WndNum))
	{
		//printf("lib_vdec.c@is_nvr_preview_chn_open: not preview channel\n");
		#ifdef FAST_SWITCH_PREVIEW
		return 2;//使用子码流预览
		#else
		return 0;
		#endif
	}
	
	if(u32WndNum > lib_gbl_info.preview_chn_threshold)
	{
		return 2;//使用子码流预览
	}
	
	return 1;//使用主码流预览
}

static unsigned int ob_nums[32] = {0};
static unsigned int ob_pts[32] = {0};

static unsigned char g_recreate_flag = 0;

//#define VDEC_FRAME_MODE

#if 0
int nvr_preview_vdec_write(int chn, vdec_stream_s *pin_stream)
{
	VDEC_STREAM_S stStream;
	HI_S32 s32ret;
	
	//子码流
	int real_chn = chn;
	if(real_chn >= ARG_VI_NUM_MAX)
	{
		real_chn -= ARG_VI_NUM_MAX;
	}
	
	#ifdef FAST_SWITCH_PREVIEW
	PREVIEW_LOCK();
	unsigned char ret = is_nvr_preview_chn_open(real_chn);
	//if(real_chn == 0) printf("chn=%d real_chn=%d ret=%d\n",chn,real_chn,ret);
	if(!ret)
	{
		//csp modify 20140318
		#ifdef P2P_SUB_STREAM
		if(plib_gbl_info->venc[real_chn].venc_third_start)
		{
			if(plib_gbl_info->last_pre_mode != PREVIEW_CLOSE)
			{
				if(chn >= ARG_VI_NUM_MAX)//子码流
				{
					if(plib_gbl_info->byDecoderType[real_chn])
					{
						PREVIEW_UNLOCK();
						
						stStream.pu8Addr = pin_stream->data;
						stStream.u32Len = pin_stream->len;
						//stStream.u64PTS = pin_stream->pts;
						stStream.u64PTS = 0;
						
						chn = real_chn;
						
						//s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, HI_IO_BLOCK);
						s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, -1);//HI3535 SDK API修改
						if(HI_SUCCESS != s32ret)
						{
							printf("function:%s,line:%d,send to vdec 0x%x\n", __FUNCTION__, __LINE__, s32ret);
							return -1;
						}
						else
						{
							//if(chn == 1 && ((pin_stream->data[4] & 0x1f) == 0x07)) printf("chn%d nvr_preview_vdec_write [%d] bytes success\n", index, pin_stream->len);
						}
						
						return pin_stream->len;
					}
				}
			}
		}
		PREVIEW_UNLOCK();
		return -1;
		#else
		PREVIEW_UNLOCK();
		return -1;
		#endif
	}
	if(chn < ARG_VI_NUM_MAX)//当前帧是主码流
	{
		if(ret == 1)//当前此通道需要解主码流
		{
			if(plib_gbl_info->byDecoderType[real_chn] == 1)//当前此通道的解码器是主码流解码器
			{
				//当前此通道的解码器是主码流解码器，则正常解码
			}
			else
			{
				if((pin_stream->data[4] & 0x1f) == 0x07)
				{
					nvr_preview_vdec_chn_recreate(real_chn, 1);
				}
				else
				{
					//printf("1######\n");
					PREVIEW_UNLOCK();
					return -1;
				}
			}
		}
		else//当前此通道需要解子码流
		{
			if(plib_gbl_info->byDecoderType[real_chn] == 1)//当前此通道的解码器是主码流解码器
			{
				//当前此通道的解码器是主码流解码器，需要继续解主码流直到子码流关键帧到来
				//printf("continue decode main stream\n");
			}
			else
			{
				//printf("2######\n");
				PREVIEW_UNLOCK();
				return -1;
			}
		}
	}
	else//当前帧是子码流
	{
		if(ret == 2)//当前此通道需要解子码流
		{
			if(plib_gbl_info->byDecoderType[real_chn] == 2)//当前此通道的解码器是子码流解码器
			{
				//当前此通道的解码器是子码流解码器，则正常解码
			}
			else
			{
				if((pin_stream->data[4] & 0x1f) == 0x07)
				{
					nvr_preview_vdec_chn_recreate(real_chn, 2);
				}
				else
				{
					//printf("3######\n");
					PREVIEW_UNLOCK();
					return -1;
				}
			}
		}
		else//当前此通道需要解主码流
		{
			if(plib_gbl_info->byDecoderType[real_chn] == 2)//当前此通道的解码器是子码流解码器
			{
				//当前此通道的解码器是子码流解码器，需要继续解子码流直到主码流关键帧到来
				//printf("continue decode sub stream\n");
			}
			else
			{
				//printf("4######\n");
				PREVIEW_UNLOCK();
				return -1;
			}
		}
	}
	PREVIEW_UNLOCK();
	#else
	unsigned char ret = is_nvr_preview_chn_open(real_chn);
	if(!ret)
	{
		return -1;
	}
	else if(ret == 1 && chn >= ARG_VI_NUM_MAX)
	{
		return -1;
	}
	else if(ret == 2 && chn < ARG_VI_NUM_MAX)
	{
		return -1;
	}
	#endif
	
	//if(chn == 8 || chn == 0) printf("nvr_preview_vdec_write-1:chn=%d,ARG_VI_NUM_MAX=%d\n",chn,ARG_VI_NUM_MAX);
	
	//int index = chn;
	
	#ifdef FAST_SWITCH_PREVIEW//csp modify 20140429
	if(chn < ARG_VI_NUM_MAX)
	{
		if(plib_gbl_info->byMainWaitIFrame[chn])
		{
			if((pin_stream->data[4] & 0x1f) == 0x07)
			{
				plib_gbl_info->byMainWaitIFrame[chn] = 0;
			}
			else
			{
				//printf("main chn%d wait I Frame\n", chn);
			}
		}
		
		//if(chn == 0) printf("nvr_preview_vdec_write-2.1:chn=%d,ARG_VI_NUM_MAX=%d,DecoderType=%d\n",chn,ARG_VI_NUM_MAX,plib_gbl_info->byDecoderType[chn]);
	}
	else
	{
		chn -= ARG_VI_NUM_MAX;
		
		if(plib_gbl_info->bySubWaitIFrame[chn])
		{
			if((pin_stream->data[4] & 0x1f) == 0x07)
			{
				plib_gbl_info->bySubWaitIFrame[chn] = 0;
			}
			else
			{
				//printf("sub chn%d wait I Frame\n", chn);
			}
		}
		
		//if(chn == 0) printf("nvr_preview_vdec_write-2.2:chn=%d,ARG_VI_NUM_MAX=%d,DecoderType=%d\n",chn,ARG_VI_NUM_MAX,plib_gbl_info->byDecoderType[chn]);
	}
	#else
	if(chn >= ARG_VI_NUM_MAX)
	{
		chn -= ARG_VI_NUM_MAX;
	}
	
	if(ret == 1)
	{
		if(plib_gbl_info->byMainWaitIFrame[chn])
		{
			if((pin_stream->data[4] & 0x1f) == 0x07)
			{
				plib_gbl_info->byMainWaitIFrame[chn] = 0;
			}
			else
			{
				//printf("main chn%d wait I Frame\n", chn);
			}
		}
	}
	else if(ret == 2)
	{
		if(plib_gbl_info->bySubWaitIFrame[chn])
		{
			if((pin_stream->data[4] & 0x1f) == 0x07)
			{
				plib_gbl_info->bySubWaitIFrame[chn] = 0;
			}
			else
			{
				//printf("sub chn%d wait I Frame\n", chn);
			}
		}
	}
	
	//if(chn == 0) printf("nvr_preview_vdec_write-2:chn=%d,ARG_VI_NUM_MAX=%d,DecoderType=%d\n",chn,ARG_VI_NUM_MAX,plib_gbl_info->byDecoderType[chn]);
	#endif
	
	//csp modify 20140423
	/*VDEC_CHN_STAT_S stStat;
	s32ret = HI_MPI_VDEC_Query(chn, &stStat);
	if(HI_SUCCESS != s32ret)
	{
		printf("lib_vdec.c@nvr_preview_vdec_write: HI_MPI_VDEC_Query, ret=0x%x\n", s32ret);
	}
	else
	{
		printf("vdec%d u32LeftPics=%d u32LeftStreamBytes=%d framelen=%d\n",chn,stStat.u32LeftPics,stStat.u32LeftStreamBytes,pin_stream->len);
	}*/
	
	stStream.pu8Addr = pin_stream->data;
	stStream.u32Len = pin_stream->len;
	//stStream.u64PTS = pin_stream->pts;
	//csp modify 20140423
	stStream.u64PTS = 0;
	//stStream.u64PTS = pin_stream->pts;
	
	//s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, HI_IO_BLOCK);
	s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, -1);//HI3535 SDK API修改
	if(HI_SUCCESS != s32ret)
	{
		printf("function:%s,line:%d,send to vdec 0x%x\n", __FUNCTION__, __LINE__, s32ret);
		return -1;
	}
	else
	{
		//if(chn == 1 && ((pin_stream->data[4] & 0x1f) == 0x07))
		//if(chn == 0) printf("chn%d nvr_preview_vdec_write [%d] bytes success\n", index, pin_stream->len);
	}
	
	return pin_stream->len;
}

#else
int nvr_preview_vdec_write(int chn, vdec_stream_s *pin_stream)
{
	VDEC_STREAM_S stStream;
	HI_S32 s32ret;
	
	//子码流
	int real_chn = chn;
	if(real_chn >= ARG_VI_NUM_MAX)
	{
		real_chn -= ARG_VI_NUM_MAX;
	}
	
	#ifdef FAST_SWITCH_PREVIEW
	PREVIEW_LOCK();
	unsigned char ret = is_nvr_preview_chn_open(real_chn);
	//if(real_chn == 0) printf("chn=%d real_chn=%d ret=%d\n",chn,real_chn,ret);
	if(!ret)
	{
		//csp modify 20140318
		#ifdef P2P_SUB_STREAM
		if(plib_gbl_info->venc[real_chn].venc_third_start)
		{
			if(plib_gbl_info->last_pre_mode != PREVIEW_CLOSE)
			{
				if(chn >= ARG_VI_NUM_MAX)//子码流
				{
					if(plib_gbl_info->byDecoderType[real_chn])
					{
						PREVIEW_UNLOCK();
						
						#if 1
						stStream.pu8Addr = pin_stream->data;
						stStream.u32Len = pin_stream->len;
						
						#ifdef VDEC_FRAME_MODE
						stStream.u64PTS = pin_stream->pts;
							#ifdef HI3535
								stStream.bEndOfFrame  = HI_TRUE;
								stStream.bEndOfStream = HI_FALSE;
							#endif
						#else
						stStream.u64PTS = 0;
						#endif
						
						chn = real_chn;

						#ifdef HI3535
							s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, -1);//HI3535 SDK API修改
						#else
							s32ret = HI_MPI_VDEC_SendStream(chn, &stStream, HI_IO_BLOCK);
						#endif
						if(HI_SUCCESS != s32ret)
						{
							//csp modify 20150110
							//printf("function:%s,line:%d,send to vdec 0x%x\n", __FUNCTION__, __LINE__, s32ret);
							printf("function:%s,line:%d,chn%d send to vdec 0x%x\n", __FUNCTION__, __LINE__, chn, s32ret);
							return -1;
						}
						else
						{
							//if(chn == 1 && ((pin_stream->data[4] & 0x1f) == 0x07)) printf("chn%d nvr_preview_vdec_write [%d] bytes success\n", index, pin_stream->len);
						}
						#endif
						
						return pin_stream->len;
					}
				}
			}
		}
		PREVIEW_UNLOCK();
		return -1;
		#else
		PREVIEW_UNLOCK();
		return -1;
		#endif
	}
	
	if(chn < ARG_VI_NUM_MAX)//当前帧是主码流
	{
		if(ret == 1)//当前此通道需要解主码流
		{
			if(plib_gbl_info->byDecoderType[real_chn] == 1)//当前此通道的解码器是主码流解码器
			{
				//当前此通道的解码器是主码流解码器，则正常解码
			}
			else //当前此通道的解码器是子码流解码器
			{
				unsigned char head[4] = {0x00,0x00,0x00,0x01};
				if(((pin_stream->data[4] & 0x1f) == 0x07) && (memcmp(pin_stream->data, head, 4) == 0)) //关键帧
				//if ((pin_stream->data[4] & 0x1f) == 0x07)
				{
					#if 0
					printf("%s 11,", __func__);
					int i;
					for (i=0; i<5; i++)
					{
						printf(" 0x%x", pin_stream->data[i]);
					}
					printf("\n");
					#endif
					//csp modify 20150110
					int hd_decoder_count = 0;
					int k = 0;
					for(k = 0; k < ARG_VI_NUM_MAX; k++)
					{
						if(plib_gbl_info->byDecoderType[k] == 1 && k != real_chn)
						{
							hd_decoder_count++;
						}
					}
					
					if(lib_gbl_info.preview_chn_threshold > 1 && hd_decoder_count >= lib_gbl_info.preview_chn_threshold)
					{
						//printf("0######\n");
						PREVIEW_UNLOCK();
						return -1;
					}
					
					if(g_recreate_flag)
					{
						PREVIEW_UNLOCK();
						return -1;
					}
					
					g_recreate_flag = 1;
					
					PREVIEW_UNLOCK();//csp modify 20150110
					
					//unsigned int t1 = getTimeStamp();
					
					nvr_preview_vdec_chn_recreate(real_chn, 1);
					
					//unsigned int t2 = getTimeStamp();
					//if(t2-t1>30) printf("chn%02d s2m span:%u###\n",real_chn,t2-t1);
					
					g_recreate_flag = 0;
					
					PREVIEW_LOCK();//csp modify 20150110
					
					ob_nums[real_chn] = 0;//6;//7;
					ob_pts[real_chn] = getTimeStamp();
					
					//printf("chn%02d s2m len:%d data:(%02x %02x %02x %02x %02x %02x) pts:%u\n",real_chn,pin_stream->len,
					//	pin_stream->data[0],pin_stream->data[1],pin_stream->data[2],pin_stream->data[3],pin_stream->data[4],pin_stream->data[5],
					//	getTimeStamp());
				}
				else
				{
					#if 0
					printf("%s 12,", __func__);
					int i;
					for (i=0; i<5; i++)
					{
						printf(" 0x%x", pin_stream->data[i]);
					}
					printf("\n");
					#endif
					//printf("1######\n");
					PREVIEW_UNLOCK();
					return -1;
				}
			}
		}
		else//当前此通道需要解子码流
		{
			if(plib_gbl_info->byDecoderType[real_chn] == 1)//当前此通道的解码器是主码流解码器
			{
				//当前此通道的解码器是主码流解码器，需要继续解主码流直到子码流关键帧到来
				//printf("continue decode main stream\n");
			}
			else
			{
				//printf("2######\n");
				PREVIEW_UNLOCK();
				return -1;
			}
		}
	}
	else//当前帧是子码流
	{
		if(ret == 2)//当前此通道需要解子码流
		{
			if(plib_gbl_info->byDecoderType[real_chn] == 2)//当前此通道的解码器是子码流解码器
			{
				//当前此通道的解码器是子码流解码器，则正常解码
			}
			else
			{
				unsigned char head[4] = {0x00,0x00,0x00,0x01};
				if(((pin_stream->data[4] & 0x1f) == 0x07) && (memcmp(pin_stream->data, head, 4) == 0))
				//if ((pin_stream->data[4] & 0x1f) == 0x07)
				{
					#if 0
					printf("%s 21,", __func__);
					int i;
					for (i=0; i<5; i++)
					{
						printf(" 0x%x", pin_stream->data[i]);
					}
					printf("\n");
					#endif
					
					if(g_recreate_flag)
					{
						PREVIEW_UNLOCK();
						return -1;
					}
					
					g_recreate_flag = 1;
					
					PREVIEW_UNLOCK();//csp modify 20150110
					
					//unsigned int t1 = getTimeStamp();
					
					nvr_preview_vdec_chn_recreate(real_chn, 2);
					
					//unsigned int t2 = getTimeStamp();
					//if(t2-t1>30) printf("chn%02d m2s span:%u###\n",real_chn,t2-t1);
					
					g_recreate_flag = 0;
					
					PREVIEW_LOCK();//csp modify 20150110
					
					//printf("chn%02d m2s len:%d data:(%02x %02x %02x %02x %02x %02x)\n",real_chn,pin_stream->len,
					//	pin_stream->data[0],pin_stream->data[1],pin_stream->data[2],pin_stream->data[3],pin_stream->data[4],pin_stream->data[5]);
				}
				else
				{
					#if 0
					printf("%s 22,", __func__);
					int i;
					for (i=0; i<5; i++)
					{
						printf(" 0x%x", pin_stream->data[i]);
					}
					printf("\n");
					#endif
					//printf("3######\n");
					PREVIEW_UNLOCK();
					return -1;
				}
			}
		}
		else//当前此通道需要解主码流
		{
			if(plib_gbl_info->byDecoderType[real_chn] == 2)//当前此通道的解码器是子码流解码器
			{
				//当前此通道的解码器是子码流解码器，需要继续解子码流直到主码流关键帧到来
				//printf("continue decode sub stream\n");
			}
			else
			{
				//printf("4######\n");
				PREVIEW_UNLOCK();
				return -1;
			}
		}
	}
	
	PREVIEW_UNLOCK();
	
	if(ob_nums[real_chn])
	{
		ob_nums[real_chn]--;
		printf("chn%02d main len:%d data:(%02x %02x %02x %02x %02x %02x) ob_nums:%d ob_pts:%u\n",chn,pin_stream->len,
					pin_stream->data[0],pin_stream->data[1],pin_stream->data[2],pin_stream->data[3],pin_stream->data[4],pin_stream->data[5],
					ob_nums[real_chn],getTimeStamp()-ob_pts[real_chn]);
		ob_pts[real_chn] = getTimeStamp();
	}
	#else
	unsigned char ret = is_nvr_preview_chn_open(real_chn);
	if(!ret)
	{
		return -1;
	}
	else if(ret == 1 && chn >= ARG_VI_NUM_MAX)
	{
		return -1;
	}
	else if(ret == 2 && chn < ARG_VI_NUM_MAX)
	{
		return -1;
	}
	#endif
	
	//if(chn == 8 || chn == 0) printf("nvr_preview_vdec_write-1:chn=%d,ARG_VI_NUM_MAX=%d\n",chn,ARG_VI_NUM_MAX);
	
	//int index = chn;
	
	#ifdef FAST_SWITCH_PREVIEW//csp modify 20140429
	if(chn < ARG_VI_NUM_MAX)
	{
		#if 0
		if(plib_gbl_info->byMainWaitIFrame[chn])
		{
			if((pin_stream->data[4] & 0x1f) == 0x07)
			{
				plib_gbl_info->byMainWaitIFrame[chn] = 0;
			}
			else
			{
				//printf("main chn%d wait I Frame\n", chn);
			}
		}
		#else
		if(plib_gbl_info->byMainWaitIFrame[real_chn])
		{
			if((pin_stream->data[4] & 0x1f) == 0x07)
			{
				plib_gbl_info->byMainWaitIFrame[real_chn] = 0;
			}
			else
			{
				//printf("main chn%d wait I Frame\n", chn);
			}
		}
		#endif
		//if(chn == 0) printf("nvr_preview_vdec_write-2.1:chn=%d,ARG_VI_NUM_MAX=%d,DecoderType=%d\n",chn,ARG_VI_NUM_MAX,plib_gbl_info->byDecoderType[chn]);
	}
	else
	{
		#if 0
		chn -= ARG_VI_NUM_MAX;
		
		if(plib_gbl_info->bySubWaitIFrame[chn])
		{
			if((pin_stream->data[4] & 0x1f) == 0x07)
			{
				plib_gbl_info->bySubWaitIFrame[chn] = 0;
			}
			else
			{
				//printf("sub chn%d wait I Frame\n", chn);
			}
		}
		
		//if(chn == 0) printf("nvr_preview_vdec_write-2.2:chn=%d,ARG_VI_NUM_MAX=%d,DecoderType=%d\n",chn,ARG_VI_NUM_MAX,plib_gbl_info->byDecoderType[chn]);

		#else
		
		if(plib_gbl_info->bySubWaitIFrame[real_chn])
		{
			if((pin_stream->data[4] & 0x1f) == 0x07)
			{
				plib_gbl_info->bySubWaitIFrame[real_chn] = 0;
			}
			else
			{
				//printf("sub chn%d wait I Frame\n", chn);
			}
		}
		#endif
	}
	#else

		#if 0
		if(chn >= ARG_VI_NUM_MAX)
		{
			chn -= ARG_VI_NUM_MAX;
		}
		
		if(ret == 1)
		{
			if(plib_gbl_info->byMainWaitIFrame[chn])
			{
				if((pin_stream->data[4] & 0x1f) == 0x07)
				{
					plib_gbl_info->byMainWaitIFrame[chn] = 0;
				}
				else
				{
					//printf("main chn%d wait I Frame\n", chn);
				}
			}
		}
		else if(ret == 2)
		{
			if(plib_gbl_info->bySubWaitIFrame[chn])
			{
				if((pin_stream->data[4] & 0x1f) == 0x07)
				{
					plib_gbl_info->bySubWaitIFrame[chn] = 0;
				}
				else
				{
					//printf("sub chn%d wait I Frame\n", chn);
				}
			}
		}
		
		//if(chn == 0) printf("nvr_preview_vdec_write-2:chn=%d,ARG_VI_NUM_MAX=%d,DecoderType=%d\n",chn,ARG_VI_NUM_MAX,plib_gbl_info->byDecoderType[chn]);
		#else
		
		if(ret == 1)
		{
			if(plib_gbl_info->byMainWaitIFrame[real_chn])
			{
				if((pin_stream->data[4] & 0x1f) == 0x07)
				{
					plib_gbl_info->byMainWaitIFrame[real_chn] = 0;
				}
				else
				{
					//printf("main chn%d wait I Frame\n", chn);
				}
			}
		}
		else if(ret == 2)
		{
			if(plib_gbl_info->bySubWaitIFrame[real_chn])
			{
				if((pin_stream->data[4] & 0x1f) == 0x07)
				{
					plib_gbl_info->bySubWaitIFrame[real_chn] = 0;
				}
				else
				{
					//printf("sub chn%d wait I Frame\n", chn);
				}
			}
		}
		#endif
	#endif
	
	//csp modify 20140423
	/*VDEC_CHN_STAT_S stStat;
	s32ret = HI_MPI_VDEC_Query(chn, &stStat);
	if(HI_SUCCESS != s32ret)
	{
		printf("lib_vdec.c@nvr_preview_vdec_write: HI_MPI_VDEC_Query, ret=0x%x\n", s32ret);
	}
	else
	{
		printf("vdec%d u32LeftPics=%d u32LeftStreamBytes=%d framelen=%d\n",chn,stStat.u32LeftPics,stStat.u32LeftStreamBytes,pin_stream->len);
	}*/
	
	//PREVIEW_LOCK();
	
	unsigned int t1 = getTimeStamp();

	//yaogang modify 20150603 for 轮巡过程中通道在极端情况下处于停止接收码流状态
	#if 1
	//vdec chn
	VDEC_CHN_STAT_S stStat_vdec;
	s32ret = HI_MPI_VDEC_Query(real_chn, &stStat_vdec);
	if(HI_SUCCESS != s32ret)
	{
		printf("chn%d function:%s,line:%d,HI_MPI_VDEC_Query ret: 0x%x\n", chn, __FUNCTION__, __LINE__, s32ret);
		return -1;
	}
	//如果VDEC通道没有启动接收，则开启
	if (!stStat_vdec.bStartRecvStream)
	{
		printf("%s start vdec recv chn%d\n", __func__, chn);
		pthread_mutex_lock(&plib_gbl_info->preview_chn_lock[real_chn]);
		s32ret = HI_MPI_VDEC_StopRecvStream(real_chn);
		if(HI_SUCCESS != s32ret)
		{
			printf("chn%d function:%s,line:%d,HI_MPI_VDEC_StopRecvStream ret: 0x%x\n", chn, __FUNCTION__, __LINE__, s32ret);
			pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[real_chn]);
			return -1;
		}
		
		s32ret = HI_MPI_VDEC_ResetChn(real_chn);
		if(HI_SUCCESS != s32ret)
		{
			printf("chn%d function:%s,line:%d,HI_MPI_VDEC_ResetChn ret: 0x%x\n", chn, __FUNCTION__, __LINE__, s32ret);
			pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[real_chn]);
			return -1;
		}
		
		s32ret = HI_MPI_VDEC_StartRecvStream(real_chn);
		if(HI_SUCCESS != s32ret)
		{
			printf("chn%d function:%s,line:%d,HI_MPI_VDEC_StartRecvStream ret: 0x%x\n", chn, __FUNCTION__, __LINE__, s32ret);
			pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[real_chn]);
			return -1;
		}
		pthread_mutex_unlock(&plib_gbl_info->preview_chn_lock[real_chn]);
	}
	#endif

	
	stStream.pu8Addr = pin_stream->data;
	stStream.u32Len = pin_stream->len;
	#ifdef VDEC_FRAME_MODE
	stStream.u64PTS = pin_stream->pts;
		#ifdef HI3535
			stStream.bEndOfFrame  = HI_TRUE;
			stStream.bEndOfStream = HI_FALSE;
		#endif
	#else
	stStream.u64PTS = 0;
	#endif
	
	#ifdef HI3535
		s32ret = HI_MPI_VDEC_SendStream(real_chn, &stStream, -1);//HI3535 SDK API修改
	#else
		s32ret = HI_MPI_VDEC_SendStream(real_chn, &stStream, HI_IO_BLOCK);
	#endif
	
	unsigned int t2 = getTimeStamp();
	if(t2-t1>30) printf("chn%02d dec span:%u###\n",real_chn,t2-t1);
	
	//PREVIEW_UNLOCK();
	
	if(HI_SUCCESS != s32ret)
	{
		printf("chn%d function:%s,line:%d,send to vdec 0x%x\n", chn, __FUNCTION__, __LINE__, s32ret);
		return -1;
	}
	else
	{
		//if(chn == 1 && ((pin_stream->data[4] & 0x1f) == 0x07))
		//if(chn == 0) printf("chn%d nvr_preview_vdec_write [%d] bytes success\n", index, pin_stream->len);
	}
	
	return pin_stream->len;
}
#endif

