/******************************************************************************

	TL hslib common
	
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
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
	
#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
	
#include "loadbmp.h"
#include "hifb.h"
	
#include "lib_common.h"
#include "common.h"
#include "vio_common.h"
#include "lib_venc.h"


#define PTS_REF_SYS_TIME 1

#ifdef PTS_REF_SYS_TIME
static int venc_time_update_flag[ARG_CHN_MAX] = {1};
static unsigned long long venc_base_timestamp[ARG_CHN_MAX] = {0};
static unsigned long long venc_base_pts[ARG_CHN_MAX] = {0};
static unsigned long long last_frame_timestamp[ARG_CHN_MAX] = {0};
#endif

#define MIN_BIT_RATE		16
#define MAX_BIT_RATE		(1024 * 10)

//csp modify 20140318
static int venc_third_fd[ARG_CHN_MAX];
#define PRE_MALLOC_MAX_PACK	10
static VENC_PACK_S pre_third_malloc_max_pack[PRE_MALLOC_MAX_PACK];

//csp modify 20140406
unsigned char is_nvr_third_stream_open(int chn)
{
	return plib_gbl_info->venc[chn].venc_third_start;
}

int tl_venc_open(void)
{
	return 0;
}

void tl_venc_close(void)
{
	return;
}

int tl_venc_ioctl(int rsv, int cmd, void *arg)
{
	return -1;
}

/******************************************************************************
* function : get picture size(w*h), according Norm and enPicSize
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_GetPicSize(PIC_SIZE_E enPicSize, SIZE_S *pstSize)
{
	switch(enPicSize)
	{
		case VENC_SIZE_QCIF:
			pstSize->u32Width = 176;
			pstSize->u32Height = IS_PAL()?144:120;
			break;
		case VENC_SIZE_CIF:
			pstSize->u32Width = 352;
			pstSize->u32Height = IS_PAL()?288:240;
			break;
		case VENC_SIZE_D1:
			pstSize->u32Width = 704;
			pstSize->u32Height = IS_PAL()?576:480;
			break;
		case VENC_SIZE_960H:
			pstSize->u32Width = 960;
			pstSize->u32Height = IS_PAL()?576:480;
			break;
		#if 0
		case PIC_480H:
			pstSize->u32Width = 480;
			pstSize->u32Height = 270;
			break;
		#endif
		case VENC_SIZE_QVGA:    /* 320 * 240 */
			pstSize->u32Width = 320;
			pstSize->u32Height = 240;
			break;
		case VENC_SIZE_VGA:     /* 640 * 480 */
			pstSize->u32Width = 640;
			pstSize->u32Height = 480;
			break;
		case VENC_SIZE_XGA:     /* 1024 * 768 */
			pstSize->u32Width = 1024;
			pstSize->u32Height = 768;
			break;
		case VENC_SIZE_SXGA:    /* 1400 * 1050 */
			pstSize->u32Width = 1400;
			pstSize->u32Height = 1050;
			break;
		case VENC_SIZE_UXGA:    /* 1600 * 1200 */
			pstSize->u32Width = 1600;
			pstSize->u32Height = 1200;
			break;
		case VENC_SIZE_QXGA:    /* 2048 * 1536 */
			pstSize->u32Width = 2048;
			pstSize->u32Height = 1536;
			break;
		case VENC_SIZE_WVGA:    /* 854 * 480 */
			pstSize->u32Width = 854;
			pstSize->u32Height = 480;
			break;
		case VENC_SIZE_WSXGA:   /* 1680 * 1050 */
			pstSize->u32Width = 1680;
			pstSize->u32Height = 1050;
			break;
		case VENC_SIZE_WUXGA:   /* 1920 * 1200 */
			pstSize->u32Width = 1920;
			pstSize->u32Height = 1200;
			break;
		case VENC_SIZE_WQXGA:   /* 2560 * 1600 */
			pstSize->u32Width = 2560;
			pstSize->u32Height = 1600;
			break;
		case VENC_SIZE_HD720:   /* 1280 * 720 */
			pstSize->u32Width = 1280;
			pstSize->u32Height = 720;
			break;
		case VENC_SIZE_HD1080:  /* 1920 * 1080 */
			pstSize->u32Width = 1920;
			pstSize->u32Height = 1080;
			break;
		default:
			return HI_FAILURE;
	}
	return HI_SUCCESS;
}

//csp modify 20140318
/******************************************************************************
* funciton : Start venc stream mode (h264, mjpeg)
* note      : rate control parameter need adjust, according your case.
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_Start(VENC_GRP VencGrp, VENC_CHN VencChn, VENC_CHN_ATTR_S *pstVencChnAttr)
{
	HI_S32 s32Ret = HI_SUCCESS;
	
	/******************************************
	step 1: Greate Venc Group
	******************************************/
	#ifdef HI3535
	#else
	s32Ret = HI_MPI_VENC_CreateGroup(VencGrp);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_CreateGroup[%d] failed with %#x!\n",VencGrp, s32Ret);
		goto out;
	}
	#endif
	
	/******************************************
	step 2:  Create Venc Channel
	******************************************/
	s32Ret = HI_MPI_VENC_CreateChn(VencChn, pstVencChnAttr);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",	VencChn, s32Ret);
		goto error1;
	}
	
	/******************************************
	step 3:  Regist Venc Channel to VencGrp
	******************************************/
	#ifdef HI3535
	#else
	s32Ret = HI_MPI_VENC_RegisterChn(VencGrp, VencChn);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_RegisterChn faild with %#x!\n", s32Ret);
		goto error2;
	}
	#endif
	
	/******************************************
	step 4:  Start Recv Venc Pictures
	******************************************/
	s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
		goto error3;
	}
	
	//printf("SAMPLE_COMM_VENC_Start [%d,%d] success\n",VencGrp,VencChn);
	
	goto out;
	
error3:
	#ifdef HI3535
	#else
	s32Ret = HI_MPI_VENC_UnRegisterChn(VencChn);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_UnRegisterChn vechn[%d] failed with %#x!\n", VencChn, s32Ret);
		return HI_FAILURE;
	}
	#endif
	
#ifdef HI3535
//error2:
#else
error2:
#endif
	s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n", VencChn, s32Ret);
		return HI_FAILURE;
	}
	
error1:
	#ifdef HI3535
	#else
	s32Ret = HI_MPI_VENC_DestroyGroup(VencGrp);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n", VencGrp, s32Ret);
		return HI_FAILURE;
	}
	#endif
	
out:
	return s32Ret;
}

//csp modify 20140318
HI_S32 SAMPLE_COMM_VENC_Stop(VENC_GRP VencGrp,VENC_CHN VencChn)
{
	HI_S32 s32Ret;
	
	/******************************************
	step 1:  Stop Recv Pictures
	******************************************/
	s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n", VencChn, s32Ret);
		return HI_FAILURE;
	}
	
	/******************************************
	step 2:  UnRegist Venc Channel
	******************************************/
	#ifdef HI3535
	#else
	s32Ret = HI_MPI_VENC_UnRegisterChn(VencChn);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_UnRegisterChn vechn[%d] failed with %#x!\n", VencChn, s32Ret);
		return HI_FAILURE;
	}
	#endif
	
	/******************************************
	step 3:  Distroy Venc Channel
	******************************************/
	s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n", VencChn, s32Ret);
		return HI_FAILURE;
	}
	
	/******************************************
	step 4:  Distroy Venc Group
	******************************************/
	#ifdef HI3535
	#else
	s32Ret = HI_MPI_VENC_DestroyGroup(VencGrp);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n", VencGrp, s32Ret);
		return HI_FAILURE;
	}
	#endif
	
    return HI_SUCCESS;
}

//csp modify 20140318
/******************************************************************************
* function : venc bind vpss           
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_BindVpss(VENC_GRP VenGrp,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
	
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;
	
	#ifdef HI3535
	stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VenGrp;
	#else
    stDestChn.enModId = HI_ID_GROUP;
    stDestChn.s32DevId = VenGrp;
    stDestChn.s32ChnId = 0;
	#endif
	
    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        LIB_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
	
	//printf("SAMPLE_COMM_VENC_BindVpss [%d,%d,%d] success\n",VpssGrp,VpssChn,VenGrp);
	
    return s32Ret;
}

//csp modify 20140318
/******************************************************************************
* function : venc unbind vpss           
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_UnBindVpss(VENC_GRP VenGrp,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
	
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;
	
	#ifdef HI3535
	stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VenGrp;
	#else
    stDestChn.enModId = HI_ID_GROUP;
    stDestChn.s32DevId = VenGrp;
    stDestChn.s32ChnId = 0;
	#endif
	
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        LIB_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
	
    return s32Ret;
}

int tl_venc_start(int chn, venc_size_e size, venc_parameter_t *para)
{
	int ch_max = ARG_VI_NUM_MAX;
	
	if(chn < 0 || chn >= ch_max || para == NULL)
	{
		printf("lib_venc.c@rz_venc_start: error chn=%d\n", chn);
		return -1;
	}
	
	if(lib_gbl_info.venc[chn].start)
	{
		return 0;
	}
	
	if((size != VENC_SIZE_D1) && (size != VENC_SIZE_HD1080) && (size != VENC_SIZE_HD720) && (size != VENC_SIZE_960H) && (size != VENC_SIZE_CIF))
	{
		printf("lib_venc.c@rz_venc_start: chn=%d, size=%d, not support!\n", chn, size);
		return -1;
	}
	
	if(para->bit_rate < MIN_BIT_RATE || para->bit_rate > MAX_BIT_RATE || para->frame_rate == 0)// || para->frame_rate > ((IS_PAL() == 0)?30:25))
	{
		printf("lib_venc.c@rz_venc_start: invalid arg, bit rate=%d, frame rate=%d\n", para->bit_rate, para->frame_rate);
		return -1;
	}
	
	if((para->pic_level < 0) || (para->pic_level > 5))
	{
		printf("lib_venc.c@rz_venc_start: invalid arg, pic_level=%u\n", para->pic_level);
		para->pic_level = 0;
	}
	
	if((para->is_cbr < RC_MODE_VBR) || (para->is_cbr > RC_MODE_FIXQP))
	{
		printf("lib_venc.c@rz_venc_start: invalid arg, pic_level=%u\n", para->is_cbr);
		para->is_cbr = RC_MODE_CBR;
	}
	
	if((para->gop < 1) || (para->gop > 200))
	{
		printf("lib_venc.c@rz_venc_start: invalid arg, gop=%u\n", para->gop);
		para->gop = 40;
	}
	
	VENC_LOCK();
	
	lib_gbl_info.venc[chn].start = 1;
	
	VENC_UNLOCK();
	
	return 0;
}

int tl_venc_stop(int chn)
{
	int ch_max = ARG_VI_NUM_MAX;
	
	if(chn < 0 || chn >= ch_max)
	{
		printf("lib_venc.c@rz_venc_stop: error chn=%d\n", chn);
		return -1;
	}
	
	if(lib_gbl_info.venc[chn].start == 0)
	{
		//LIB_PRT("chn not start, chn=%d\n", chn);
		return 0;
	}
	
	VENC_LOCK();
	
	lib_gbl_info.venc[chn].start = 0;
	
	VENC_UNLOCK();
	
	return 0;
}

void dumpH264Stream(int chn, FILE* fpH264File, VENC_STREAM_S *pstStream)
{
	printf("chn=%d,pack_count=%d", chn, pstStream->u32PackCount);
	
	int i;
	for(i=0; i<pstStream->u32PackCount; i++)
	{
	#ifdef HI3535
		printf("#pk[%d]=%d,ty=%d,pts=%llu", i, pstStream->pstPack[i].u32Len,
																		pstStream->pstPack[i].DataType.enH264EType, pstStream->pstPack[i].u64PTS);
	#else
		printf("#pk[%d]=%d,ty=%d,pts=%llu", i, pstStream->pstPack[i].u32Len[0] + ((pstStream->pstPack[i].u32Len[1] > 0) ? pstStream->pstPack[i].u32Len[1]:0),
																		pstStream->pstPack[i].DataType.enH264EType, pstStream->pstPack[i].u64PTS);
	#endif
	}
	
	printf("\n");
}

#include "ipcamera.h"

unsigned long long venc_get_sys_pts(int chn, unsigned long long pts)
{
	unsigned long long new_pts = pts;
	
#ifdef PTS_REF_SYS_TIME
	if(chn < ARG_VI_NUM_MAX && chn >= 0)
	{
		VENC_PTS_LOCK();
		
		if((pts < last_frame_timestamp[chn]) || ((pts-last_frame_timestamp[chn]) > 3*1000*1000))
		{
			venc_time_update_flag[chn] = 1;
		}
		last_frame_timestamp[chn] = pts;
		
		if(venc_time_update_flag[chn])
		{
			venc_time_update_flag[chn] = 0;
			
			struct timeval time_pos;
			gettimeofday(&time_pos, NULL);
			
			venc_base_timestamp[chn] = 1000*1000;
			venc_base_timestamp[chn] = venc_base_timestamp[chn] * time_pos.tv_sec + time_pos.tv_usec;
			venc_base_pts[chn] = pts;
			
			//if(chn == 0)
			//{
			//	time_t t = time_pos.tv_sec;
			//	printf("[base] chn%d pts:(%llu,%llu) time:%s",chn,venc_base_timestamp[chn],venc_base_pts[chn],ctime(&t));
			//}
		}
		new_pts = venc_base_timestamp[chn] + (pts - venc_base_pts[chn]);
		
		VENC_PTS_UNLOCK();
		
		//if(chn == 0)
		//{
		//	time_t t = new_pts/1000000;
		//	printf("[run] chn%d pts:(%llu,%llu) time:%s",chn,pts,new_pts,ctime(&t));
		//}
	}
#endif
	
	return new_pts;
}

extern s32 ReadFrameFromVencMainQueue(unsigned char *in_buf, unsigned int in_len, real_stream_s *stream);
extern s32 ReadFrameFromVencSubQueue(unsigned char *in_buf, unsigned int in_len, real_stream_s *stream);

int tl_venc_read(unsigned char *in_buf, unsigned int in_len, venc_stream_s *pout_stream)
{
	if(in_buf == NULL || in_len <= 0 || pout_stream == NULL)
	{
		printf("ReadFrameFromVencMainQueue failed-1\n");
		return -1;
	}
	
	if(plib_gbl_info->venc_main_frame_queue.nLenUsed <= 0)
	{
		//printf("ReadFrameFromVencMainQueue failed-2\n");
		return -1;
	}
	
	real_stream_s stream;
	memset(&stream, 0, sizeof(stream));
	
	int ret = ReadFrameFromVencMainQueue(in_buf, in_len, &stream);
	if(0 != ret)
	{
		printf("ReadFrameFromVencMainQueue failed-3\n");
		return -1;
	}
	
	if((stream.chn >= ARG_VI_NUM_MAX) || (stream.chn < 0))
	{
		printf("ReadFrameFromVencMainQueue failed-4\n");
		return -1;
	}
	
	if(!plib_gbl_info->venc[stream.chn].start)
	{
		//printf("ReadFrameFromVencMainQueue failed-5\n");
		return -1;
	}
	
#ifdef PTS_REF_SYS_TIME
	VENC_PTS_LOCK();
	if(venc_time_update_flag[stream.chn])
	{
		venc_time_update_flag[stream.chn] = 0;
		
		struct timeval time_pos;
		gettimeofday(&time_pos, NULL);
		
		venc_base_timestamp[stream.chn] = 1000*1000;
		venc_base_timestamp[stream.chn] = venc_base_timestamp[stream.chn] * time_pos.tv_sec;
		venc_base_timestamp[stream.chn] = venc_base_timestamp[stream.chn] + time_pos.tv_usec;
		venc_base_pts[stream.chn] = stream.pts;
	}
	pout_stream->pts = venc_base_timestamp[stream.chn] + (stream.pts - venc_base_pts[stream.chn]);
	VENC_PTS_UNLOCK();
#endif
	
	pout_stream->chn = stream.chn;
	pout_stream->data = stream.data;
	pout_stream->len = stream.len;
	//pout_stream->pts = stream.pts;
	pout_stream->type = stream.frame_type;
	pout_stream->rsv = stream.rsv;
	
	//if(stream.chn == 0 || stream.chn == 15)
	//{
	//	printf("chn%d tl_venc_read len:%d\n",stream.chn,stream.len);
	//}
	
	return pout_stream->len;
}

int venc_init(void)
{
	int ch_max = ARG_VI_NUM_MAX;
	
	int i = 0;
	for(i = 0; i < ch_max; i++)
	{
		lib_gbl_info.venc[i].open = 0;
		lib_gbl_info.venc[i].start = 0;
		lib_gbl_info.venc[i].venc_min_start = 0;
		lib_gbl_info.venc[i].venc_third_start = 0;//csp modify 20140318
	}
	
	for(i = 0; i < ARG_CHN_MAX; i++)
	{
		venc_time_update_flag[i] = 1;
		venc_base_timestamp[i] = 0;
		venc_base_pts[i] = 0;
		last_frame_timestamp[i] = 0;
		
		venc_third_fd[i] = -1;//csp modify 20140406
	}
	
	return 0;
}

int tl_venc_set_cb(EncStreamCB pMainStreamCB, EncStreamCB pSubStreamCB, EncStreamCB pThirdStreamCB)
{
	lib_gbl_info.pMainStreamCB = pMainStreamCB;
	lib_gbl_info.pSubStreamCB = pSubStreamCB;
	lib_gbl_info.pThirdStreamCB = pThirdStreamCB;
	return 0;
}

int tl_venc_get_para(int channel, int stream, venc_parameter_t *para)//0-主码流,1-次码流
{
	int ch_max = ARG_VI_NUM_MAX;
	int ret;

	//printf("yg tl_venc_get_para ch_max:%d\n", ch_max);
	
	if(channel < 0 || channel >= ch_max || NULL == para)
	{
		printf("lib_venc.c@tl_venc_set_para: error arg, channel=%d\n", channel);
		return -1;
	}
	VideoEncoderParam VideoEncPara;
	ret = IPC_CMD_GetVideoEncoderParam(channel, stream, &VideoEncPara);
	if (ret == 0)
	{
		para->bit_rate = VideoEncPara.BitRate;
		para->is_cbr = VideoEncPara.RateType;//流模式(0为定码流，1为变码流)
		//para->frame_rate = VideoEncPara.FrameRate;
		//para->pic_level = VideoEncPara.Quality;
		//para->gop = VideoEncPara.GovLength;
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int tl_venc_set_para(int channel, int stream, venc_parameter_t *para)//0-主码流,1-次码流
{
	int ch_max = ARG_VI_NUM_MAX;

	//printf("yg tl_venc_set_para\n");
	
	if(channel < 0 || channel >= ch_max || NULL == para)
	{
		printf("lib_venc.c@tl_venc_set_para: error arg, channel=%d\n", channel);
		return -1;
	}
	
	if(para->bit_rate < MIN_BIT_RATE || para->bit_rate > MAX_BIT_RATE || para->frame_rate == 0)// || para->frame_rate > ((IS_PAL() == 0)?30:25))
	{
		printf("lib_venc.c@tl_venc_set_para: invalid arg, bit rate=%d, frame rate=%d\n", para->bit_rate, para->frame_rate);
		return -1;
	}

	VideoEncoderParam VideoEncPara;
	VideoEncPara.BitRate = para->bit_rate;
	VideoEncPara.RateType = para->is_cbr;//流模式(0为定码流，1为变码流)
	//VideoEncPara.FrameRate = para->frame_rate;
	//VideoEncPara.Quality = para->pic_level;
	//VideoEncPara.GovLength = para->gop;
	IPC_CMD_SetVideoEncoderParam(channel, stream, &VideoEncPara);//0-主码流,1-次码流
	
	return 0;
}

//==================================================================================
//minor encode stream

int tl_venc_minor_start(int chn, venc_size_e size, venc_parameter_t *para)
{
	int ch_max = ARG_VI_NUM_MAX;
	
	if(chn < 0 || chn >= ch_max || para == NULL)
	{
		printf("lib_venc.c@rz_venc_minor_start: invalid arg, chn=%d\n", chn);
		return -1;
	}
	
	if(plib_gbl_info->venc[chn].venc_min_start)
	{
		printf("lib_venc.c@rz_venc_minor_start: already start, chn=%d\n", chn);
		return 0;
	}
	
	if((size != VENC_SIZE_CIF) && (size != VENC_SIZE_D1) && (size != VENC_SIZE_QCIF))
	{
		printf("lib_venc.c@rz_venc_minor_start: invalid arg, size=%d\n", size);
		return -1;
	}
	
	if(para->bit_rate < MIN_BIT_RATE || para->bit_rate > MAX_BIT_RATE || para->frame_rate == 0)// || para->frame_rate > ((IS_PAL() == 0)?30:25))
	{
		printf("lib_venc.c@rz_venc_minor_start: invalid arg, bit rate=%d, frame rate=%d\n", para->bit_rate, para->frame_rate);
		return -1;
	}
	
	if((para->pic_level < 0) || (para->pic_level > 5))
	{
		printf("lib_venc.c@rz_venc_minor_start: invalid arg, pic_level=%u\n", para->pic_level);
		para->pic_level = 0;
	}
	
	if((para->is_cbr < RC_MODE_VBR) || (para->is_cbr > RC_MODE_FIXQP))
	{
		printf("lib_venc.c@rz_venc_minor_start: invalid arg, pic_level=%u\n", para->is_cbr);
		para->is_cbr = RC_MODE_CBR;
	}
	
	if((para->gop < 1) || (para->gop > 200))
	{
		printf("lib_venc.c@rz_venc_minor_start: invalid arg, gop=%u\n", para->gop);
		para->gop = 40;
	}
	
	VENC_MIN_LOCK();
	
	lib_gbl_info.venc[chn].venc_min_start = 1;
	
	VENC_MIN_UNLOCK();
	
	return 0;
}

int tl_venc_minor_stop(int chn)
{
	int ch_max = ARG_VI_NUM_MAX;
	
	if(chn < 0 || chn >= ch_max)
	{
		printf("lib_venc.c@rz_venc_minor_stop: invalid arg, chn=%d\n", chn);
		return -1;
	}
	
	if(plib_gbl_info->venc[chn].venc_min_start == 0)
	{
		printf("lib_venc.c@rz_venc_minor_stop: not start, chn=%d\n", chn);
		return 0;
	}
	
	VENC_MIN_LOCK();
	
	lib_gbl_info.venc[chn].venc_min_start = 0;
	
	VENC_MIN_UNLOCK();
	
	return 0;
}

int tl_venc_minor_read(unsigned char *in_buf, unsigned int in_len, venc_stream_s *pout_stream)
{
	//printf("tl_venc_minor_read-1\n");
	
	if(in_buf == NULL || in_len <= 0 || pout_stream == NULL)
	{
		//printf("tl_venc_minor_read-2\n");
		return -1;
	}
	
	if(plib_gbl_info->venc_sub_frame_queue.nLenUsed <= 0)
	{
		//printf("tl_venc_minor_read-3\n");
		return -1;
	}
	
	real_stream_s stream;
	memset(&stream, 0, sizeof(stream));
	
	int ret = ReadFrameFromVencSubQueue(in_buf, in_len, &stream);
	if(0 != ret)
	{
		//printf("tl_venc_minor_read-4\n");
		return -1;
	}
	
	if((stream.chn >= ARG_VI_NUM_MAX) || (stream.chn < 0) || (!plib_gbl_info->venc[stream.chn].venc_min_start))
	{
		//printf("tl_venc_minor_read-5\n");
		return -1;
	}
	
	//printf("tl_venc_minor_read-6\n");
	
	pout_stream->chn = stream.chn;
	pout_stream->data = stream.data;
	pout_stream->len = stream.len;
	pout_stream->pts = stream.pts;
	pout_stream->type = stream.frame_type;
	pout_stream->rsv = stream.rsv;
	
	return pout_stream->len;
}

int tl_venc_minor_get_para(int channel, venc_parameter_t *para)
{
	return 0;
}

int tl_venc_minor_set_para(int channel, venc_parameter_t *para)
{
	return 0;
}

int tl_venc_read_slave(unsigned char **in_buf, unsigned int *pLen)
{
	return 0;
}

int tl_venc_slave_release(unsigned char *pAddr, unsigned int Len)
{
	return 0;
}

int tl_venc_minor_read_slave(unsigned char **in_buf, unsigned int *pLen)
{
	return 0;
}

int tl_venc_minor_slave_release(unsigned char *pAddr, unsigned int Len)
{
	return 0;
}

//csp modify 20140318
static unsigned char third_stream_capture_flag = 0;

//csp modify 20140318
void* ThirdStreamThrFxn(void* arg)
{
	unsigned char pEncBuf[80*1024];
	
	while(1)
	{
		venc_stream_s venc_stream;
		memset(&venc_stream, 0, sizeof(venc_stream));
		
		int ret = tl_venc_third_read(pEncBuf, sizeof(pEncBuf), &venc_stream);
		if(ret <= 0)
		{
			usleep(1);
			continue;
		}
		
		if(ret > 0 && venc_stream.chn >= 0 && venc_stream.chn < ARG_VI_NUM_MAX)
		{
			//csp modify 20140406
			if(lib_gbl_info.last_pre_mode != PREVIEW_CLOSE)
			{
				if(lib_gbl_info.pThirdStreamCB != NULL)
				{
					//csp modify 20150110
					venc_stream.width = 640;//352;//320;//352;//320;
					venc_stream.height = 480;//288;//240;//288;//240;
					lib_gbl_info.pThirdStreamCB(&venc_stream);
				}
			}
		}
		else
		{
			usleep(1);//usleep(1000);//csp modify
		}
	}
	
	return 0;
}

//csp modify 20140318
//yaogang modify 20150402 mobile stream

int tl_venc_third_set_para(int channel, venc_parameter_t *para)
{
	VENC_CHN VeChn;
	VENC_CHN_ATTR_S stVencChnAttr;
	VENC_ATTR_H264_CBR_S    stH264Cbr;
	VENC_ATTR_H264_VBR_S    stH264Vbr;
	VENC_ATTR_H264_FIXQP_S  stH264FixQp;
	HI_S32 s32Ret;
	
	//printf("$%d$\n", __LINE__);
	
	int ch_max = ARG_VI_NUM_MAX;
	if(channel < 0 || channel >= ch_max || NULL == para)
	{
		printf("lib_venc.c@tl_venc_third_set_para: error arg, channel=%d\n", channel);
		return -1;
	}
	
	VeChn = channel + ch_max + ch_max;
	
	if(para->bit_rate < MIN_BIT_RATE || para->bit_rate > MAX_BIT_RATE || para->frame_rate == 0)// || para->frame_rate > ((IS_PAL() == 0)?30:25))
	{
		printf("lib_venc.c@tl_venc_third_set_para: invalid arg, bit rate=%d, frame rate=%d\n", para->bit_rate, para->frame_rate);
		return -1;
	}
	if(!para->is_cbr && para->pic_level > 5)
	{
		printf("lib_venc.c@tl_venc_third_set_para: invalid arg, pic_level=%u\n", para->pic_level);
		return -1;
	}
	
	if(para->frame_rate > 25)
	{
		para->frame_rate = 25;
	}
	
	//csp modify 20140812
	plib_gbl_info->venc[channel].venc_third_para = *para;
	
	if(plib_gbl_info->venc[channel].venc_third_start == 0)
	{
		printf("lib_venc.c@rz_venc_third_set_para: not start, chn=%d\n", channel);
		return 0;
	}
	
	//printf("$%d$\n", __LINE__);
	
	s32Ret = HI_MPI_VENC_GetChnAttr(VeChn, &stVencChnAttr);
	if(s32Ret < 0)
	{
		printf("lib_venc.c@tl_venc_third_set_para: HI_MPI_VENC_GetChnAttr, ret=0x%x\n", s32Ret);
		return -1;
	}
	if(stVencChnAttr.stVeAttr.enType != PT_H264)
	{
		printf("lib_venc.c@tl_venc_third_set_para: stVencChnAttr.stVeAttr.enType=%d\n", stVencChnAttr.stVeAttr.enType);
		return -1;
	}
	
	//printf("$%d$\n", __LINE__);
	
	switch(para->is_cbr)
	{
		case RC_MODE_CBR:
			stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
			stH264Cbr.u32Gop            = para->gop;
			stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
			stH264Cbr.u32SrcFrmRate      = 25;/* input (vi) frame rate */
			stH264Cbr.fr32DstFrmRate = para->frame_rate;/* target frame rate */
			stH264Cbr.u32BitRate = para->bit_rate; /* average bit rate */
			stH264Cbr.u32FluctuateLevel = para->pic_level;
			memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
			break;
		case RC_MODE_VBR:
			stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
			stH264Vbr.u32Gop = para->gop;
			stH264Vbr.u32StatTime = 1;
			stH264Vbr.u32SrcFrmRate = 25;
			stH264Vbr.fr32DstFrmRate = para->frame_rate;
			stH264Vbr.u32MinQp = 24;
			stH264Vbr.u32MaxQp = 32;
			stH264Vbr.u32MaxBitRate = para->bit_rate;
			memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
			break;
		case RC_MODE_FIXQP:
			stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
			stH264FixQp.u32Gop = para->gop;
			stH264FixQp.u32SrcFrmRate = 25;
			stH264FixQp.fr32DstFrmRate = para->frame_rate;
			stH264FixQp.u32IQp = 20;
			stH264FixQp.u32PQp = 23;
			memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
			break;
		default:
			LIB_PRT("VENC RC mode config error!\n");
			return HI_FAILURE;
	}
	
	//printf("$%d$\n", __LINE__);
	
	s32Ret = HI_MPI_VENC_SetChnAttr(VeChn, &stVencChnAttr);
	if(s32Ret < 0)
	{
		printf("lib_venc.c@tl_venc_third_set_para: HI_MPI_VENC_SetChnAttr, ret=0x%x\n", s32Ret);
		return -1;
	}
	
	//printf("rz_venc_third_set_para: chn=%d, BR=%u, FR=%u,pic_level=%u,is_cbr=%u,gop=%u\n", channel, para->bit_rate, para->frame_rate, para->pic_level, para->is_cbr, para->gop);
	
	return 0;
}


int tl_venc_third_start(int chn, venc_size_e size, venc_parameter_t *para)
{
	VPSS_GRP VpssGrp;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	VENC_ATTR_H264_S stH264Attr;
	VENC_CHN_ATTR_S stVencChnAttr;
	VENC_ATTR_H264_CBR_S	stH264Cbr;
	VENC_ATTR_H264_VBR_S	stH264Vbr;
	VENC_ATTR_H264_FIXQP_S	stH264FixQp;
	SIZE_S stPicSize;
	
	HI_S32 s32Ret = HI_SUCCESS;
	int ch_max = ARG_VI_NUM_MAX;
	
	//printf("$%d$\n", __LINE__);
	
	if(chn < 0 || chn >= ch_max || NULL == para)
	{
		printf("lib_venc.c@rz_venc_third_start: invalid arg, chn=%d\n", chn);
		return -1;
	}
	
	if(plib_gbl_info->venc[chn].venc_third_start)
	{
		printf("lib_venc.c@rz_venc_third_start: already start, chn=%d\n", chn);
		return 0;
	}
	
	//csp modify
	if((size != VENC_SIZE_CIF) && (size != VENC_SIZE_D1) && (size != VENC_SIZE_QCIF) && (size != VENC_SIZE_VGA))
	{
		printf("lib_venc.c@rz_venc_third_start: invalid arg, size=%d\n", size);
		return -1;
	}
	if(para->bit_rate < MIN_BIT_RATE || para->bit_rate > MAX_BIT_RATE || para->frame_rate == 0)// || para->frame_rate > ((IS_PAL() == 0)?30:25))
	{
		printf("lib_venc.c@rz_venc_third_start: invalid arg, bit rate=%d, frame rate=%d\n", para->bit_rate, para->frame_rate);
		return -1;
	}
	
	if(!para->is_cbr && para->pic_level > 5)
	{
		printf("lib_venc.c@rz_venc_third_start: invalid arg, pic_level=%u\n", para->pic_level);
		return -1;
	}
	
	if(!IPC_GetLinkStatus(chn))
	{
		//printf("lib_venc.c@rz_venc_third_start: chn%d no link\n", chn);
		return -1;
	}
	
	if(!third_stream_capture_flag)
	{
		pthread_t pid;
		pthread_create(&pid, NULL, ThirdStreamThrFxn, NULL);
		
		third_stream_capture_flag = 1;
	}
	
	if(para->frame_rate > 25)
	{
		para->frame_rate = 25;
	}
	
	//printf("$%d$\n", __LINE__);
	
	{//venc chn config
		s32Ret = SAMPLE_COMM_SYS_GetPicSize(size, &stPicSize);
		if (HI_SUCCESS != s32Ret)
		{
			LIB_PRT("Get picture size failed!\n");
			return -1;
		}
		//printf("%s stPicSize: width: %d, lenght: %d\n", __func__, stPicSize.u32Width, stPicSize.u32Height);
		//yaogang modify 20150402 mobile stream
		#if 0
		if(size == VENC_SIZE_CIF)
		{
			//csp modify 20150110
			stPicSize.u32Width = 704;//640;//352;//320;//352;//320;
			stPicSize.u32Height = 576;//480;//288;//240;//288;//240;
		}
		#endif
		stH264Attr.u32MaxPicWidth = stPicSize.u32Width;
		stH264Attr.u32MaxPicHeight = stPicSize.u32Height;
		stH264Attr.u32PicWidth = stPicSize.u32Width;/*the picture width*/
		stH264Attr.u32PicHeight = stPicSize.u32Height;/*the picture height*/
		#ifdef HI3520D//csp modify 20140406
		stH264Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height * 1;/*stream buffer size*/
		#else
		//csp modify 20140406
		//stH264Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height * 3 / 2;/*stream buffer size*/
		stH264Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height * 1;/*stream buffer size*/
		#endif
		stH264Attr.u32Profile  = 0;/*0: baseline; 1:MP; 2:HP   ? */
		stH264Attr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
		#ifdef HI3535
		#else
		stH264Attr.bField = HI_FALSE;  /* surpport frame code only for hi3516, bfield = HI_FALSE */
		stH264Attr.bMainStream = HI_TRUE; /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
		stH264Attr.u32Priority = 0; /*channels precedence level. invalidate for hi3516*/
		stH264Attr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame. Invalidate for hi3516*/
		#endif
		
		memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));
		stVencChnAttr.stVeAttr.enType = PT_H264;
		//printf("$%d$\n", __LINE__);
		
		//para->gop = 15;
		
		switch(para->is_cbr)
		{
			case RC_MODE_CBR:
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
				stH264Cbr.u32Gop			= para->gop;
				stH264Cbr.u32StatTime		= 1; /* stream rate statics time(s) */
				#ifdef HI3535
				stH264Cbr.u32SrcFrmRate 	= 25;/* input (vi) frame rate */
				stH264Cbr.fr32DstFrmRate	= 10;//8;//6;//para->frame_rate;//csp modify 20150110
				#else
				stH264Cbr.u32ViFrmRate		= 25;/* input (vi) frame rate */
				stH264Cbr.fr32TargetFrmRate = para->frame_rate;/* target frame rate */
				#endif
				stH264Cbr.u32BitRate = 160;//128;//64;//48;//para->bit_rate; /* average bit rate */ //csp modify 20150110
				stH264Cbr.u32FluctuateLevel = 0;//para->pic_level;
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
				break;
			case RC_MODE_VBR:
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
				stH264Vbr.u32Gop = para->gop;
				stH264Vbr.u32StatTime = 1;
				#ifdef HI3535
				stH264Vbr.u32SrcFrmRate 	= 25;/* input (vi) frame rate */
				stH264Vbr.fr32DstFrmRate	= para->frame_rate;
				#else
				stH264Vbr.u32ViFrmRate = 25;
				stH264Vbr.fr32TargetFrmRate = para->frame_rate;
				#endif
				stH264Vbr.u32MinQp = 24;
				stH264Vbr.u32MaxQp = 32;
				stH264Vbr.u32MaxBitRate = para->bit_rate;
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
				break;
			case RC_MODE_FIXQP:
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
				stH264FixQp.u32Gop = para->gop;
				#ifdef HI3535
				stH264FixQp.u32SrcFrmRate 	= 25;/* input (vi) frame rate */
				stH264FixQp.fr32DstFrmRate	= para->frame_rate;
				#else
				stH264FixQp.u32ViFrmRate = 25;
				stH264FixQp.fr32TargetFrmRate = para->frame_rate;
				#endif
				stH264FixQp.u32IQp = 20;
				stH264FixQp.u32PQp = 23;
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
				break;
			default:
				LIB_PRT("VENC RC mode config error!\n");
				return HI_FAILURE;
		}
	}//end venc chn config
	//printf("$%d$\n", __LINE__);
	
	VencGrp = chn + ch_max + ch_max;
	VencChn = chn + ch_max + ch_max;
	VpssGrp = chn;
	
	VENC_THIRD_LOCK();//csp modify
	
	s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, &stVencChnAttr);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("Start Venc failed!\n");
		goto out;
	}
	//printf("$%d$\n", __LINE__);
	
	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, THIRD_STREAM_VPSS_CHN);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("Start Venc failed!\n");
		goto error1;
	}
	//printf("$%d$\n", __LINE__);
	
	/* Set Venc Fd. */
	venc_third_fd[chn] = HI_MPI_VENC_GetFd(VencChn);
	//printf("vencchn: %d, vencfd: %d~~~~~~~~~~~~~~\n", chn, venc_third_fd[chn]);
	if (venc_third_fd[chn] < 0)
	{
		LIB_PRT("HI_MPI_VENC_GetFd failed with %#x!\n", venc_third_fd[chn]);
		goto error2;
	}
	
	plib_gbl_info->venc[chn].venc_third_start = 1;
	//printf("$%d$\n", __LINE__);
	goto out;
	
error2:
	SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, THIRD_STREAM_VPSS_CHN);
	
error1:
	SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
	
out:
	VENC_THIRD_UNLOCK();//csp modify
	return s32Ret;
}

//csp modify 20140318
int tl_venc_third_stop(int chn)
{
	VPSS_GRP VpssGrp;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	HI_S32 s32Ret;
	int ch_max = ARG_VI_NUM_MAX;
	//printf("$%d$\n", __LINE__);
	
	if(chn < 0 || chn >= ch_max)
	{
		printf("lib_venc.c@rz_venc_third_stop: invalid arg, chn=%d\n", chn);
		return -1;
	}
	
	if(plib_gbl_info->venc[chn].venc_third_start == 0)
	{
		printf("lib_venc.c@rz_venc_third_stop: not start, chn=%d\n", chn);
		return 0;
	}
	
	VencGrp = chn + ch_max + ch_max;
	VencChn = chn + ch_max + ch_max;
	VpssGrp = chn;
	//printf("$%d$\n", __LINE__);
	
	VENC_THIRD_LOCK();
	
	plib_gbl_info->venc[chn].venc_third_start = 0;
	
	s32Ret = SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, THIRD_STREAM_VPSS_CHN);
	//printf("$%d$\n", __LINE__);
	
	s32Ret = SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
	
	VENC_THIRD_UNLOCK();
	
	//printf("$%d$\n", __LINE__);
	return s32Ret;
}

//csp modify 20140318
int tl_venc_third_read(unsigned char *in_buf, unsigned int in_len, venc_stream_s *pout_stream)
{
	static int venc_ch = 0;
	int maxfd = -1;
	fd_set read_fds;
	struct timeval TimeoutVal;
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	int s32ret, f_ret = 0;
	int i = 0, ret = -1;
	int ch_max = ARG_VI_NUM_MAX;
	//printf("$%d$\n", __LINE__);
	
	FD_ZERO(&read_fds);
	for(i = 0; i < ch_max; i++)
	{
		if(!plib_gbl_info->venc[i].venc_third_start)
		{
			continue;
		}
		//printf("$%d$\n", __LINE__);
		FD_SET(venc_third_fd[i],&read_fds);
		//printf("$%d$\n", __LINE__);
		if(venc_third_fd[i] > maxfd)
		{
			maxfd = venc_third_fd[i];
		}
	}
	//printf("$%d$\n", __LINE__);
	if(maxfd < 0)
	{
		//printf("lib_venc.c@rz_venc_third_read: no enc channel start\n");
		return -1;
	}
	//printf("$%d$\n", __LINE__);
	
	TimeoutVal.tv_sec = 2;
	TimeoutVal.tv_usec = 0;
	s32ret = select(maxfd + 1,&read_fds,NULL,NULL,&TimeoutVal);
	if(s32ret < 0)
	{
		//csp modify
		printf("venc_third_read: select err\n");
		return -1;
	}
	else if(s32ret == 0)
	{
		//csp modify
		printf("venc_third_read: select time out\n");
		return -1;
	}
	
	for (i = 0; i < ch_max; i++)
	{
		venc_ch = (venc_ch + 1) % ch_max + ch_max + ch_max;
		
		if(!plib_gbl_info->venc[venc_ch - ch_max - ch_max].venc_third_start)
		{
			continue;
		}
		//printf("$%d$\n", __LINE__);
		
		if(FD_ISSET(venc_third_fd[venc_ch - ch_max - ch_max], &read_fds))
		{
			memset(&stStream,0,sizeof(stStream));
			//printf("$%d$\n", __LINE__);
			s32ret = HI_MPI_VENC_Query(venc_ch, &stStat);
			if (s32ret != HI_SUCCESS)
			{
				printf("HI_MPI_VENC_Query:0x%x\n",s32ret);
				ret = -1;
				goto out;
			}
			//printf("$%d$\n", __LINE__);
			
			if(stStat.u32CurPacks > PRE_MALLOC_MAX_PACK)
			{
				printf("~~~~third malloc~~~~~~~the packs count is %d\n", stStat.u32CurPacks);
				stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S)*stStat.u32CurPacks);
				if (NULL == stStream.pstPack)
				{
					printf("malloc third stream pack err!\n");
					ret = -1;
					goto out;
				}
			}
			else
			{
				stStream.pstPack = pre_third_malloc_max_pack;
			}
			//printf("$%d$\n", __LINE__);
			
			stStream.u32PackCount = stStat.u32CurPacks;
			
			VENC_THIRD_LOCK();
			s32ret = HI_MPI_VENC_GetStream(venc_ch, &stStream, HI_TRUE);
			if (s32ret != HI_SUCCESS || stStream.u32PackCount == 0)
			{
				VENC_THIRD_UNLOCK();
				if(stStat.u32CurPacks > PRE_MALLOC_MAX_PACK)
				{
					free(stStream.pstPack);
				}
				stStream.pstPack = NULL;
				printf("HI_MPI_VENC_GetStream err 0x%x\n",s32ret);
				ret = -1;
				goto out;
			}
			//printf("$%d$\n", __LINE__);
			
		#if 0
			dumpH264Stream(venc_ch, NULL, &stStream);
			f_ret = -1;
		#else
			{//copy stream to the function caller
				int k;
				unsigned int total_size;
				VENC_STREAM_S *pstStream = &stStream;
				
				total_size = 0;
				for(k = 0; k < pstStream->u32PackCount; k++)
				{
				#ifdef HI3535
					total_size = total_size + pstStream->pstPack[k].u32Len;
				#else
					total_size = total_size + pstStream->pstPack[k].u32Len[0] + ((pstStream->pstPack[k].u32Len[1] > 0) ? pstStream->pstPack[k].u32Len[1]:0);
				#endif
				}
				if(total_size <= in_len)
				{
					pout_stream->chn = venc_ch - ch_max - ch_max;
					pout_stream->type = FRAME_TYPE_P;
					pout_stream->pts = pstStream->pstPack[0].u64PTS;					
					pout_stream->len = total_size;
					pout_stream->data = in_buf;
					total_size = 0;
					for (k = 0; k < pstStream->u32PackCount; k++)
					{
						if(H264E_NALU_ISLICE == pstStream->pstPack[k].DataType.enH264EType)
						{
							pout_stream->type = FRAME_TYPE_I;
							pout_stream->pts = pstStream->pstPack[k].u64PTS;
						}
						//printf("$%d$\n", __LINE__);
						
						#ifdef HI3535
						memcpy(in_buf + total_size, pstStream->pstPack[k].pu8Addr, pstStream->pstPack[k].u32Len);
						total_size += pstStream->pstPack[k].u32Len;
						#else
						memcpy(in_buf + total_size, pstStream->pstPack[k].pu8Addr[0], pstStream->pstPack[k].u32Len[0]);
						total_size += pstStream->pstPack[k].u32Len[0];
						if(pstStream->pstPack[k].u32Len[1] > 0)
						{
							memcpy(in_buf + total_size, pstStream->pstPack[k].pu8Addr[1], pstStream->pstPack[k].u32Len[1]);
							total_size += pstStream->pstPack[k].u32Len[1];
						}
						#endif
						//printf("$%d$\n", __LINE__);
					}
					f_ret = pout_stream->len;

					//xdc multi stream
					VENC_CHN_ATTR_S stChnAttr;
					s32ret = HI_MPI_VENC_GetChnAttr(venc_ch, &stChnAttr);
					if (s32ret != HI_SUCCESS)
					{
						printf("%s: HI_MPI_VENC_GetChnAttr err 0x%x\n", __func__, s32ret);
						pout_stream->width = 320;//352;//320;
						pout_stream->height = 240;//288;//240;
					}
					else
					{
						pout_stream->width = stChnAttr.stVeAttr.stAttrH264e.u32PicWidth;
						pout_stream->height = stChnAttr.stVeAttr.stAttrH264e.u32PicHeight;
						//printf("rz_venc_third_read: HI_MPI_VENC_GetChnAttr succ,w=%d,h=%d\n",pout_stream->width,pout_stream->height);
					}
				}
				else if(in_len == 0)
				{
					printf("!!!!!!!!!!third the buf size is 0\n");
					f_ret = 0;//dump stream
				}
				else
				{
					printf("!!!!!!!!!!third the buf is small: bufsize: %d, totalsize: %d\n", in_len, total_size);
					f_ret = -2;//in_len smaller;
				}
			}
		#endif
			//printf("$%d$\n", __LINE__);
			s32ret = HI_MPI_VENC_ReleaseStream(venc_ch,&stStream);
			VENC_THIRD_UNLOCK();
			//printf("$%d$\n", __LINE__);
			if (s32ret != HI_SUCCESS)
			{
				if(stStat.u32CurPacks > PRE_MALLOC_MAX_PACK)
				{
					free(stStream.pstPack);
				}
				stStream.pstPack = NULL;
				printf("HI_MPI_VENC_ReleaseStream err 0x%x\n",s32ret);
				ret = -1;
				goto out;
			}
			
			if(stStat.u32CurPacks > PRE_MALLOC_MAX_PACK)
			{
				free(stStream.pstPack);
			}
			stStream.pstPack = NULL;
			ret = f_ret;
			goto out;
		}
	}
	
	printf("lib_venc.c@rz_venc_third_read: should not here\n");
	
out:
	//printf("$%d$\n", __LINE__);
	return ret;
}

extern void audio_update_basetime(void);
void tl_venc_update_basetime(void)
{
#ifdef PTS_REF_SYS_TIME
	VENC_PTS_LOCK();
	int i = 0;
	for(i = 0; i < ARG_CHN_MAX; i++)
	{
		venc_time_update_flag[i] = 1;
	}
	audio_update_basetime();
	VENC_PTS_UNLOCK();
	//printf("lib_venc.c@venc_update_basetime\n");
#endif
}

void venc_update_time(int chn)
{
#ifdef PTS_REF_SYS_TIME
	if(chn < ARG_VI_NUM_MAX && chn >= 0)
	{
		VENC_PTS_LOCK();
		venc_time_update_flag[chn] = 1;
		audio_update_basetime();
		VENC_PTS_UNLOCK();
	}
#endif
}

void nvr_sync_time(void)
{
	int i;
	for(i=0;i<ARG_VI_NUM_MAX;i++)
	{
		IPC_CMD_SetTime(i, 0, 0);
	}
}




