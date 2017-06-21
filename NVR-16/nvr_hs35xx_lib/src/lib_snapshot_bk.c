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
#include "lib_venc.h"

#ifdef HI3535
#else
static unsigned int gSnapChn = 0;
static unsigned int gSnapStatus = 0;

#define PRE_MALLOC_MAX_PACK	10
static VENC_PACK_S pre_malloc_max_pack[PRE_MALLOC_MAX_PACK];
#endif

const HI_U8 g_SOI[2] = {0xFF, 0xD8};
const HI_U8 g_EOI[2] = {0xFF, 0xD9};

typedef struct
{
    VENC_GRP VeGroup;   /*snap group*/
	VENC_CHN SnapChn;   /*snap venc chn*/
	VPSS_CHN VpssChn;	/*snap vpss chn*/
	VPSS_GRP VpssGrp;	/*snap vpss group*/
	VI_DEV ViDev;       /*vi device,it has the vichn which snap group bind to*/  
	VI_CHN ViChn;       /*vi channel which snap group binded to*/
	HI_U32 in_len;
	HI_U8 *in_buf;
	HI_U32 ret_len;
}SAMPLE_SNAP_THREAD_ATTR;

HI_S32 GetSnapShotBuffer(VENC_CHN SnapChn, HI_U32 in_len, HI_U8 *in_buf, HI_U32 *out_len)
{
#ifdef HI3535
	return HI_FAILURE;
#else
	HI_S32 s32Ret;
	HI_S32 s32VencFd;
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	fd_set read_fds;
	int total_size, k;
	
	*out_len = 0;
	
	s32VencFd = HI_MPI_VENC_GetFd(SnapChn);
	if(s32VencFd < 0)
	{
		printf("HI_MPI_VENC_GetFd err \n");
		return HI_FAILURE;
	}
	
	FD_ZERO(&read_fds);
	FD_SET(s32VencFd,&read_fds);
	
	s32Ret = select(s32VencFd+1, &read_fds, NULL, NULL, NULL);
	if(s32Ret < 0)
	{
		printf("select err\n");
		return HI_FAILURE;
	}
	else if(0 == s32Ret)
	{
		printf("time out\n");
		return HI_FAILURE;
	}
	else
	{
		if(FD_ISSET(s32VencFd, &read_fds))
		{
			s32Ret = HI_MPI_VENC_Query(SnapChn, &stStat);
			if(s32Ret != HI_SUCCESS)
			{
				printf("HI_MPI_VENC_Query:0x%x\n",s32Ret);
				return HI_FAILURE;
			}
			
			if(stStat.u32CurPacks > PRE_MALLOC_MAX_PACK)
			{
				stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S)*stStat.u32CurPacks);
				if(NULL == stStream.pstPack)
				{
					printf("malloc stream pack err!\n");
					return -1;
				}
			}
			else
			{
				stStream.pstPack = pre_malloc_max_pack;
			}
			
			stStream.u32PackCount = stStat.u32CurPacks;
			
			s32Ret = HI_MPI_VENC_GetStream(SnapChn, &stStream, HI_TRUE);
			if(HI_SUCCESS != s32Ret)
			{
				printf("HI_MPI_VENC_GetStream:0x%x\n",s32Ret);
				if(stStat.u32CurPacks > PRE_MALLOC_MAX_PACK)
				{
					free(stStream.pstPack);
				}
				stStream.pstPack = NULL;
				return HI_FAILURE;
			}
			
			{
				VENC_STREAM_S *pstStream = &stStream;
				
				total_size = sizeof(g_SOI)+sizeof(g_EOI);
				for(k = 0; k < pstStream->u32PackCount; k++)
				{
					total_size = total_size + pstStream->pstPack[k].u32Len[0] + ((pstStream->pstPack[k].u32Len[1] > 0) ? pstStream->pstPack[k].u32Len[1]:0);
				}
				
				if(total_size <= in_len)
				{
					memcpy(in_buf, g_SOI, sizeof(g_SOI));
					total_size = sizeof(g_SOI);
					
					for(k = 0; k < pstStream->u32PackCount; k++)
					{
						memcpy(in_buf + total_size, pstStream->pstPack[k].pu8Addr[0], pstStream->pstPack[k].u32Len[0]);
						total_size += pstStream->pstPack[k].u32Len[0];
						if(pstStream->pstPack[k].u32Len[1] > 0)
						{
							memcpy(in_buf+total_size, pstStream->pstPack[k].pu8Addr[1], pstStream->pstPack[k].u32Len[1]);
							total_size += pstStream->pstPack[k].u32Len[1];
						}
					}
					memcpy(in_buf+total_size, g_EOI, sizeof(g_EOI));
					total_size += sizeof(g_SOI);
					*out_len = total_size;
				}
				else
				{
					printf("GetSnapShotBuffer: buffer too small\n");
					return HI_FAILURE;
				}
			}
			
			s32Ret = HI_MPI_VENC_ReleaseStream(SnapChn,&stStream);
			if(s32Ret) 
			{
				printf("HI_MPI_VENC_ReleaseStream:0x%x\n",s32Ret);
				if(stStat.u32CurPacks > PRE_MALLOC_MAX_PACK)
				{
					free(stStream.pstPack);
				}
				stStream.pstPack = NULL;
				return HI_FAILURE;
			}
			
			if(stStat.u32CurPacks > PRE_MALLOC_MAX_PACK)
			{
				free(stStream.pstPack);
			}
			stStream.pstPack = NULL;
		}
	}
	
	return HI_SUCCESS;
#endif
}

/*****************************************************************************
 snap by mode 1
 how to snap:
 1)only creat one snap group
 2)bind to a vichn to snap and then unbind
 3)repeat 2) to snap all vichn in turn
 
 features:
 1)save memory, because only one snap group and snap channel
 2)efficiency lower than mode 2, pictures snapped will not more than 8. 
*****************************************************************************/
HI_VOID* SampleStartSnapByMode1(HI_VOID *p)
{
#ifdef HI3535
	return NULL;
#else
	HI_S32 s32Ret;
	VENC_GRP VeGroup = 0;
	VENC_CHN SnapChn = 0;
	VI_DEV ViDev = 0;
	VI_CHN ViChn = 0;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 0;
	SAMPLE_SNAP_THREAD_ATTR *pThdAttr = NULL;
	
	pThdAttr = (SAMPLE_SNAP_THREAD_ATTR*)p;
	VeGroup = pThdAttr->VeGroup;
	SnapChn = pThdAttr->SnapChn;
	ViDev   = pThdAttr->ViDev;
	ViChn   = pThdAttr->ViChn;
	VpssGrp = pThdAttr->VpssGrp;
	VpssChn = pThdAttr->VpssChn;
	
	/******************************************
	step 1:  Regist Venc Channel to VencGrp
	******************************************/
	s32Ret = HI_MPI_VENC_RegisterChn(VeGroup, SnapChn);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_RegisterChn faild with %#x!\n", s32Ret);
		return NULL;
	}
	
	/******************************************
	step 2:  Venc Chn bind to Vpss Chn
	******************************************/
	extern HI_S32 SAMPLE_COMM_VENC_BindVpss(VENC_GRP VenGrp,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);
	s32Ret = SAMPLE_COMM_VENC_BindVpss(VeGroup, VpssGrp, VpssChn);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VENC_BindVpss failed!\n");
		return NULL;
	}
	
	/******************************************
	step 3:  Start Recv Venc Pictures
	******************************************/
	s32Ret = HI_MPI_VENC_StartRecvPic(SnapChn);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
		return NULL;
	}
	
	/******************************************
	step 4:  recv picture
	******************************************/
	s32Ret = GetSnapShotBuffer(SnapChn, pThdAttr->in_len, pThdAttr->in_buf, &(pThdAttr->ret_len));
	if(s32Ret != HI_SUCCESS)
	{
		printf("SampleSaveSnapPic err 0x%x\n",s32Ret);
		return NULL;
	}
	
	/******************************************
	step 1:  Stop Recv Pictures
	******************************************/
	s32Ret = HI_MPI_VENC_StopRecvPic(SnapChn);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n", SnapChn, s32Ret);
		return NULL;
	}
	
	/******************************************
	step 2:  UnRegist Venc Channel
	******************************************/
	s32Ret = HI_MPI_VENC_UnRegisterChn(SnapChn);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("HI_MPI_VENC_UnRegisterChn vechn[%d] failed with %#x!\n", SnapChn, s32Ret);
		return NULL;
	}
	
	extern HI_S32 SAMPLE_COMM_VENC_UnBindVpss(VENC_GRP VenGrp,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);
	s32Ret = SAMPLE_COMM_VENC_UnBindVpss(VeGroup, VpssGrp, VpssChn);
	if(HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VENC_UnBindVpss, VeGroup=%d, VpssGrp=%d, VpssChn=%d, s32Ret=0x%x\n", VeGroup, VpssGrp, VpssChn, s32Ret);
		return NULL;
	}
	
	return NULL;
#endif
}

HI_S32 SampleDestroySnapChn(VENC_GRP VeGroup,VENC_CHN SnapChn)
{
#ifdef HI3535
	return HI_FAILURE;
#else
	HI_S32 s32Ret;
	s32Ret = HI_MPI_VENC_DestroyChn(SnapChn);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_VENC_DestroyChn snapchn %d err 0x%x\n",SnapChn,s32Ret);
		return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VENC_DestroyGroup(VeGroup);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_VENC_DestroyGroup snap group %d err 0x%x\n",SnapChn,s32Ret);
		return HI_FAILURE;
	}
	
	return HI_SUCCESS;
#endif
}

HI_S32 SampleCreateSnapChn(VENC_GRP VeGroup,VENC_CHN SnapChn, VENC_CHN_ATTR_S *pstAttr)
{
#ifdef HI3535
	return HI_FAILURE;
#else
	HI_S32 s32Ret;
	
	/*创建抓拍通道*/
	s32Ret = HI_MPI_VENC_CreateGroup(VeGroup);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_VENC_CreateGroup err 0x%x\n",s32Ret);
		return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VENC_CreateChn(SnapChn,pstAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_VENC_CreateChn err 0x%x\n",s32Ret);
		return HI_FAILURE;
	}
	
	return HI_SUCCESS;
#endif
}

int tl_1CIFJpeg_Init(void)
{
#ifdef HI3535
	return -1;
#else
	VENC_GRP SnapGrpId = 0;
	VENC_CHN SnapChn = 0;
	VENC_CHN_ATTR_S stAttr;
	VENC_ATTR_JPEG_S stJpegAttr;
	
	/*jpeg chn attr*/
	stAttr.stVeAttr.enType = PT_JPEG;
	stJpegAttr.u32MaxPicWidth  = 352;
	stJpegAttr.u32MaxPicHeight = 288;
	stJpegAttr.u32PicWidth  = 352;
	stJpegAttr.u32PicHeight = 288;
	stJpegAttr.u32BufSize = 352*288*2;
	stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
	stJpegAttr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame?*/
	stJpegAttr.u32Priority = 0;/*channels precedence level*/
	memcpy(&stAttr.stVeAttr.stAttrJpeg, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));
	
	if(gSnapStatus)
	{
		printf("lib_snapshot.c: Already init...\n");
		return -1;
	}
	
	gSnapChn = ARG_VI_NUM_MAX*2+1;
	
	SnapGrpId = gSnapChn;
	SnapChn   = gSnapChn;
	if(HI_SUCCESS != SampleCreateSnapChn(SnapGrpId,SnapChn,&stAttr))
	{
		return -1;
	}
	
	gSnapStatus = 1;
	
	return 0;
#endif
}

int tl_1CIFJpeg_Capture(int Chn, unsigned char *in_buf, unsigned int in_len)
{
#ifdef HI3535
	return -1;
#else
	VI_DEV ViDev = 0;
	VI_CHN ViChn = 0;
	SAMPLE_SNAP_THREAD_ATTR stThdAttr;
	HI_S32 ch_max = ARG_VI_NUM_MAX;
	VPSS_GRP VpssGrp = 0;
	
	if((Chn < 0) || (Chn >= ch_max))
	{
		printf("Jpeg_Capture: invalid chn=%d\n", Chn);
		return -1;
	}
	
	if(gSnapStatus == 0)
	{
		printf("lib_snapshot.c: Please init snapshot first\n");
		return -1;
	}
	
	//vi_chn_adjust(Chn, &ViDev, &ViChn, &VpssGrp);
	
	stThdAttr.VeGroup = gSnapChn;
	stThdAttr.SnapChn = gSnapChn;
	stThdAttr.VpssGrp = VpssGrp;
	stThdAttr.VpssChn = VPSS_PRE0_CHN;
	stThdAttr.ViDev = ViDev;
	stThdAttr.ViChn = ViChn;
	
	stThdAttr.in_len = in_len;
	stThdAttr.in_buf = in_buf;
	stThdAttr.ret_len = 0;
	
	SampleStartSnapByMode1(&stThdAttr);
	
	printf("size=%d\n", stThdAttr.ret_len);
	
	return stThdAttr.ret_len;
#endif
}

void tl_1CIFJpeg_Exit(void)
{
#ifdef HI3535
	return;
#else
	if(gSnapStatus)
	{
		SampleDestroySnapChn(gSnapChn, gSnapChn);
		gSnapStatus = 0;
	}
#endif
}

//yaogang modify 20150213 **********************************************************************

#ifdef	HI3535
#define	SAMPLE_PRT	printf
#define	MaxChnNum	3
typedef struct
{
	int bIinited;
	SIZE_S stSize;
	pthread_mutex_t lock;
} ENC_JPEG_CHN_INFO;

static ENC_JPEG_CHN_INFO EncChnInfo[MaxChnNum];

/******************************************************************************
* funciton : save jpeg stream  for test
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveJPEG(FILE *fpJpegFile, VENC_STREAM_S *pstStream)
{
    VENC_PACK_S*  pstData;
    HI_U32 i;

    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        pstData = &pstStream->pstPack[i];
        fwrite(pstData->pu8Addr+pstData->u32Offset, pstData->u32Len-pstData->u32Offset, 1, fpJpegFile);
        fflush(fpJpegFile);
    }

    return HI_SUCCESS;
}


/******************************************************************************
* funciton : save snap stream for test
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveSnap(VENC_STREAM_S *pstStream)
{
    char acFile[128]  = {0};
    FILE *pFile;
    HI_S32 s32Ret;
	static int gs_s32SnapCnt = 0;

    sprintf(acFile, "/mnt/snap_%d.jpg", gs_s32SnapCnt);
    pFile = fopen(acFile, "wb");
    if (pFile == NULL)
    {
        printf("open file err\n");
        return HI_FAILURE;
    }
    s32Ret = SAMPLE_COMM_VENC_SaveJPEG(pFile, pstStream);
    if (HI_SUCCESS != s32Ret)
    {
        printf("save snap picture failed!\n");
        return HI_FAILURE;
    }
    fclose(pFile);
    gs_s32SnapCnt++;
    return HI_SUCCESS;
}



/******************************************************************************
* funciton : Start snap
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SnapStart(VENC_CHN VencChn, SIZE_S *pstSize)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_JPEG_S stJpegAttr;

    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVeAttr.enType = PT_JPEG;
    
    stJpegAttr.u32MaxPicWidth  = (pstSize->u32Width+3) /4 * 4;//必须是4 的整数倍
    stJpegAttr.u32MaxPicHeight = (pstSize->u32Height+3) /4 * 4;
    stJpegAttr.u32PicWidth  = (pstSize->u32Width+3) /4 * 4;
    stJpegAttr.u32PicHeight = (pstSize->u32Height+3) /4 * 4;
    stJpegAttr.u32BufSize = (pstSize->u32Width * pstSize->u32Height * 2 + 63)/64*64;//必须是64 的整数倍
    stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
    memcpy(&stVencChnAttr.stVeAttr.stAttrJpeg, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }
    return HI_SUCCESS;
}

int tl_enc_jpeg_create_chn(int EncChn, int Width, int Height)
{
	HI_S32 s32Ret;
	SIZE_S stSize;
	VENC_PARAM_JPEG_S stJpegParam;

	if (EncChn < 0 || EncChn >= MaxChnNum)
	{
		printf("%s EncChn%d invalid, chn 0-2\n", __func__, EncChn);
		return HI_FAILURE;
	}
	
	stSize.u32Width = Width;
	stSize.u32Height = Height;

	s32Ret = SAMPLE_COMM_VENC_SnapStart(EncChn, &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("%s SAMPLE_COMM_VENC_SnapStart EncChn%d failed with %d!\n", \
			__func__, EncChn, s32Ret);
		return s32Ret;
	}

	s32Ret = HI_MPI_VENC_GetJpegParam(EncChn, &stJpegParam);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("%s HI_MPI_VENC_GetJpegParam EncChn%d failed with %d!\n", \
			__func__, EncChn, s32Ret);
		return s32Ret;
	}
	//printf("%s u32Qfactor: %d\n", __func__, stJpegParam.u32Qfactor);
		
	stJpegParam.u32Qfactor = 30;
	s32Ret = HI_MPI_VENC_SetJpegParam(EncChn, &stJpegParam);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("%s HI_MPI_VENC_SetJpegParam EncChn%d failed with %d!\n", \
			__func__, EncChn, s32Ret);
		return s32Ret;
	}

	EncChnInfo[EncChn].bIinited = 1;
	EncChnInfo[EncChn].stSize = stSize;
	pthread_mutex_init(&EncChnInfo[EncChn].lock, NULL);
		
	return s32Ret;
}

int tl_enc_jpeg_destroy_chn(int EncChn)
{
	HI_S32 s32Ret;

	pthread_mutex_lock(&EncChnInfo[EncChn].lock);
	
	s32Ret = HI_MPI_VENC_DestroyChn(EncChn);
	if (s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_VENC_DestroyChn%d failed with %#x!\n", EncChn, s32Ret);
		return HI_FAILURE;
	}

	pthread_mutex_unlock(&EncChnInfo[EncChn].lock);
	
	return s32Ret;
}

int tl_enc_snap(u8 StreamChn, u8 EncChn, u8 *pdata, u32 *psize)
{
	HI_S32 s32Ret;
	fd_set read_fds;
	HI_S32 s32VencFd;
	
	VENC_STREAM_S stStream;
	VENC_CHN_STAT_S stStat;
	
	struct timeval tv;
	struct timeval TimeoutVal;

	pthread_mutex_lock(&EncChnInfo[EncChn].lock);
	
	/******************************************
	step 2:  Start Recv Venc Pictures
	******************************************/
	s32Ret = HI_MPI_VENC_StartRecvPic(EncChn);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VENC_StartRecvPic EncChn%d faild with%#x!\n", EncChn, s32Ret);
		goto error1;
	}

	gettimeofday(&tv, NULL);
	printf("%s tv.tv_sec: %d, tv.tv_usec: %d\n", __func__, tv.tv_sec, tv.tv_usec);

	
	/******************************************
	step 3:  HI_MPI_VPSS_GetGrpFrame
	******************************************/
	VIDEO_FRAME_INFO_S stFrame;//stVFrame
	s32Ret = HI_MPI_VPSS_GetGrpFrame(StreamChn,  &stFrame, 0);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VPSS_GetGrpFrame StreamChn[%d] faild with %#x!\n",\
				StreamChn, s32Ret);
		goto error2;
	}
	/*
	printf("%s width: %d, height: %d, format: %d\n", 
		__func__, stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height, stFrame.stVFrame.enVideoFormat);
	*/
	
	/******************************************
	step 4:  send one frame to venc  
	******************************************/
	stFrame.stVFrame.u64pts = 0;
	s32Ret = HI_MPI_VENC_SendFrame(EncChn, &stFrame, -1);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VENC_SendFrame EncChn%d faild with %#x!\n", EncChn, s32Ret);
		HI_MPI_VPSS_ReleaseGrpFrame(StreamChn,  &stFrame);
		//return s32Ret;
		goto error3;
	}

	s32Ret = HI_MPI_VPSS_ReleaseGrpFrame(StreamChn,  &stFrame);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VPSS_ReleaseGrpFrame StreamChn%d faild with %#x!\n", StreamChn, s32Ret);
		//return s32Ret;
		goto error3;
	}

	/******************************************
	step 5:  recv picture
	******************************************/
	s32VencFd = HI_MPI_VENC_GetFd(EncChn);
	if (s32VencFd < 0)
	{
		printf("HI_MPI_VENC_GetFd EncChn: %d faild with%#x!\n", EncChn, s32VencFd);
		//return HI_FAILURE;
		goto error3;
	}

	FD_ZERO(&read_fds);
	FD_SET(s32VencFd, &read_fds);

	TimeoutVal.tv_sec  = 10;
	TimeoutVal.tv_usec = 0;
	s32Ret = select(s32VencFd+1, &read_fds, NULL, NULL, &TimeoutVal);
	if (s32Ret < 0) 
	{
		printf("snap select EncChn: %d failed!\n", EncChn);
		//return s32Ret;
		goto error3;
	}
	else if (0 == s32Ret) 
	{
		printf("snap EncChn: %d time out!\n", EncChn);
		//return s32Ret;
		goto error3;
	}
	else
	{
		if (FD_ISSET(s32VencFd, &read_fds))
		{
			s32Ret = HI_MPI_VENC_Query(EncChn, &stStat);
			if (s32Ret != HI_SUCCESS)
			{
				printf("HI_MPI_VENC_Query EncChn: %d failed with %#x!\n", EncChn, s32Ret);
				//return s32Ret;
				goto error3;
			}

			stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
			if (NULL == stStream.pstPack)
			{
				printf("malloc memory failed!\n");
				//return s32Ret;
				goto error3;
			}

			stStream.u32PackCount = stStat.u32CurPacks;
			s32Ret = HI_MPI_VENC_GetStream(EncChn, &stStream, -1);
			if (HI_SUCCESS != s32Ret)
			{
				printf("HI_MPI_VENC_GetStream EncChn: %d failed with %#x!\n", EncChn, s32Ret);
				free(stStream.pstPack);
				stStream.pstPack = NULL;
				//return s32Ret;
				goto error3;
			}


			gettimeofday(&tv, NULL);
			printf("%s tv.tv_sec: %d, tv.tv_usec: %d\n", __func__, tv.tv_sec, tv.tv_usec);

			VENC_PACK_S*  pstData;
			u32 framelen = 0;
			int i;
			for (i = 0; i < stStream.u32PackCount; i++)
			{
				pstData = &stStream.pstPack[i];
				framelen += pstData->u32Len - pstData->u32Offset;
			}

			if (framelen > *psize)
			{
				printf("%s framelen %d > *psize %d\n", __func__, framelen, *psize);
				*psize = 0;
			}
			else
			{
				framelen = 0;
				for (i = 0; i < stStream.u32PackCount; i++)
				{
					pstData = &stStream.pstPack[i];
					memcpy(pdata+framelen, pstData->pu8Addr+pstData->u32Offset, pstData->u32Len-pstData->u32Offset);
					framelen += pstData->u32Len - pstData->u32Offset;
				}
				*psize = framelen;
				
				/*
				//save file
				char acFile[128]  = {0};
				FILE *pFile;
				static int gs_s32SnapCnt = 0;

				sprintf(acFile, "/mnt/snap_%d.jpg", gs_s32SnapCnt);
				pFile = fopen(acFile, "wb");
				if (pFile == NULL)
				{
				printf("open file err\n");
				goto error3;
				}

				fwrite(pdata, *psize, 1, pFile);
				fflush(pFile);
				fclose(pFile);
				gs_s32SnapCnt++;
				*/
			}

			s32Ret = HI_MPI_VENC_ReleaseStream(EncChn, &stStream);
			if (s32Ret)
			{
				printf("HI_MPI_VENC_ReleaseStream EncChn: %d failed with %#x!\n", EncChn, s32Ret);
				free(stStream.pstPack);
				stStream.pstPack = NULL;
				//return HI_FAILURE;
				goto error3;
			}

			free(stStream.pstPack);
			stStream.pstPack = NULL;
		}
	}

error3:	
	/******************************************
	step 6:  HI_MPI_VPSS_GetGrpFrame
	******************************************/
	//VIDEO_FRAME_INFO_S stFrame;//stVFrame
	//HI_MPI_VPSS_ReleaseGrpFrame(StreamChn,  &stFrame);

error2:
	 /******************************************
	step 7:  stop recv picture
	******************************************/
	HI_MPI_VENC_StopRecvPic(EncChn);

error1:
	pthread_mutex_unlock(&EncChnInfo[EncChn].lock);

	return s32Ret;
}

#endif
