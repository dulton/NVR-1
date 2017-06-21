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

#include "lib_common.h"
#include "common.h"
#include "lib_video.h"
#include "lib_misc.h"

int tl_video_get_format(void)
{
	if(IS_PAL())
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

extern unsigned char is_nvr_preview_chn_open(int chn);

unsigned int tl_get_alarm_IPCExt(void)
{
	return  IPC_get_alarm_IPCExt();
}

unsigned int tl_get_alarm_IPCCover(void)
{
	return IPC_get_alarm_IPCCover();
}

//状态机状态
#define WAIT_CONNECT			0
#define CONNECT_TO_CHECK_DELAY	1
#define CHECK_VLOST				2
//从检测到IPC连接到启动检测的延时，因为IPC的连接需要一段时间
#define TIME_TO_CHECK (1*60)  //second

//返回值32位的每个位标示一个IPC视频是否丢失1:ok  0:lost
unsigned int tl_video_connection_status(void)
{

	//csp modify
	static unsigned char init_flag = 0;
	static unsigned char set_user_pic_flag[ARG_CHN_MAX*2];
	if(!init_flag)
	{
		memset(set_user_pic_flag,0,sizeof(set_user_pic_flag));
		init_flag = 1;
	}

	//yg modify 20140811 IPCá??óò???ê±??oó￡??aê??ì2a
	//×′ì??ú
	//??????±êê?ò???IPCêó?μê?·??aê§1:ok  0:lost
	unsigned int rtn = ~0;
	int i;
	ipc_unit ipcam;
	static int status = WAIT_CONNECT;
	static time_t starttime;

	//printf("%s status: %d\n", __func__, status);
	switch(status)
	{
		case WAIT_CONNECT: {
			
			for(i=0;i<ARG_VI_NUM_MAX;i++)
			{
				if(IPC_Get(i, &ipcam) == 0 && ipcam.enable)
				{
					status = CONNECT_TO_CHECK_DELAY;
					time(&starttime);
					//printf("starttime: %u\n", starttime);
					break;
				}
			}
			
		}break;
		case CONNECT_TO_CHECK_DELAY: {
			
			int flag = 0;//μ±?°ê?·?óDIPC:  ipcam.enable==1
			
			for(i=0;i<ARG_VI_NUM_MAX;i++)
			{
				if(IPC_Get(i, &ipcam) == 0 && ipcam.enable)
				{
					flag = 1;
					break;
				}
			}
			
			if (flag)//óDIPC′|óúá??ó×′ì?
			{
				//printf("time(NULL): %u\n", time(NULL));
				//printf("starttime: %u\n", starttime);
				if (time(NULL) - starttime >= TIME_TO_CHECK)
				{
					status = CHECK_VLOST;
				}
			}
			else
			{
				status = WAIT_CONNECT;
			}
			
		}break;
		case CHECK_VLOST: {

			unsigned char ret;
			int chn;
			int flag = 0;//μ±?°ê?·?óDIPC:  ipcam.enable==1
			
			for(i=0;i<ARG_VI_NUM_MAX;i++)
			{
				ret = is_nvr_preview_chn_open(i);// 1:使用主码流预览 2: 子码流
				if (ret == 1 || ret == 2)
				{
					if (ret == 1)
					{
						chn = i;
					}
					else
					{
						chn = i + ARG_VI_NUM_MAX;
					}
					//printf("set_user_pic_flag[%d]: %d\n", chn, set_user_pic_flag[chn]);
					//printf("IPC_GetLinkStatus(%d): %d\n", chn, IPC_GetLinkStatus(chn));
					if(IPC_Get(i, &ipcam) == 0 && ipcam.enable)
					{
						int ipc_status = IPC_GetLinkStatus(chn);
						//printf("IPC_GetLinkStatus(%d): %d\n", chn, ipc_status);
						if (ipc_status)// 1:ok 0:lost
						{
							if(set_user_pic_flag[chn] == 0)
							{
								set_user_pic_flag[chn] = 1;
							}							
						}
						else
						{
							//í¨μàò??-ê1?ü￡?μ?á??ó×′ì??TD§￡??òêó?μ?aê§
							rtn &= ~(1 << (i));//(IPC_GetLinkStatus(chn) == 0 && ipcam.enable)
							
							if(set_user_pic_flag[chn])//????
							{
								//printf("VPSS RESET\n");
								PREVIEW_LOCK();
								VPSS_GRP VpssGrp = i;
								HI_MPI_VPSS_ResetGrp(VpssGrp);
								VO_CHN VoChn = i;
								HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
								PREVIEW_UNLOCK();
								
								set_user_pic_flag[chn] = 0;
							}
						}
						
					}
				}

				if(IPC_Get(i, &ipcam) == 0 && ipcam.enable)
				{
					flag++;
				}
			}
			//printf("flag: %d\n", flag);
			if (flag == 0)
			{
				status = WAIT_CONNECT;
			}
		}break;
		
		default:
			printf("%s status exception\n", __func__);
			break;
	}
	
	return rtn;
}

#if 0
unsigned int tl_video_connection_status(void)
{
	unsigned int rtn = 0x00000000;
	
	//csp modify
	static unsigned char init_flag = 0;
	static unsigned char set_user_pic_flag[ARG_CHN_MAX*2];
	if(!init_flag)
	{
		memset(set_user_pic_flag,0,sizeof(set_user_pic_flag));
		init_flag = 1;
	}
	
	//csp modify
	//rtn = (unsigned int)(-1);
	//return rtn;
	
	int i = 0;
	for(i=0;i<ARG_VI_NUM_MAX;i++)
	{
		unsigned char ret = is_nvr_preview_chn_open(i); // 1主码流；2子码流
		if(ret == 1) 
		{
			int chn = i;
			int status = IPC_GetLinkStatus(chn);
			if(status)
			{
				if(set_user_pic_flag[chn])
				{
					set_user_pic_flag[chn] = 0;
				}
			}
			else
			{
				if(!set_user_pic_flag[chn])
				{
					PREVIEW_LOCK();
					VPSS_GRP VpssGrp = i;
					HI_MPI_VPSS_ResetGrp(VpssGrp);
					VO_CHN VoChn = i;
					HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
					PREVIEW_UNLOCK();
					
					set_user_pic_flag[chn] = 1;
				}
				else
				{
					//VO_CHN VoChn = i;
					//HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
				}
			}
		}
		else if(ret == 2)
		{
			int chn = i+ARG_VI_NUM_MAX;
			int status = IPC_GetLinkStatus(chn);
			if(status)
			{
				if(set_user_pic_flag[chn])
				{
					set_user_pic_flag[chn] = 0;
				}
			}
			else
			{
				if(!set_user_pic_flag[chn])
				{
					PREVIEW_LOCK();
					VPSS_GRP VpssGrp = i;
					HI_MPI_VPSS_ResetGrp(VpssGrp);
					VO_CHN VoChn = i;
					HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
					PREVIEW_UNLOCK();
					
					set_user_pic_flag[chn] = 1;
				}
				else
				{
					//VO_CHN VoChn = i;
					//HI_MPI_VO_ClearChnBuffer(VO_DEVICE_HD, VoChn, HI_TRUE);
				}
			}
		}
	}
	
	for(i=0;i<ARG_VI_NUM_MAX;i++)
	{
		int status = IPC_GetLinkStatus(i);
		if(status)
		{
			rtn |= 1 << (i);
		}
	}
	
	return rtn;
}
#endif

int tl_video_get_img_para(int channel, video_image_para_t *para)
{
	int chn = channel;
	int chn_max = ARG_VI_NUM_MAX;
	
	if(chn < 0 || chn >= chn_max)
	{
		printf("lib_video.c@video_get_img_para: invalid channel=%d\n", chn);
		return -1;
	}
	
	//printf("<lib_dbg>get_img_para: luma=%u,con=%u,sat=%u,hue=%u\n", para->brightness,para->contrast,para->saturation,para->hue);
	
	return 0;
}

int tl_video_set_img_para(int channel, video_image_para_t *para)
{
	int chn = channel;
	int chn_max = ARG_VI_NUM_MAX;
	
	if(chn < 0 || chn >= chn_max)
	{
		printf("lib_video.c@video_set_img_para: invalid channel=%d\n", chn);
		return -1;
	}
	
	return IPC_CMD_SetImageParam(channel, para);
}

int tl_video_get_luma(int chn, unsigned int *luma)
{
	return 0;
}

int tl_vo_set_img_para(unsigned int voType, vo_image_para_t *para)
{
	VO_CSC_S stpubcscparm;
	HI_S32 ret;
	
#ifdef DHD1_WBC_DSD0
	if((voType != VO_DEVICE_HD))
#else
	if((voType != VO_DEVICE_HD) && (voType != VO_DEVICE_CVBS))
	{
		printf("lib_video.c@tl_vo_set_img_para: invalid arg, VODEV=%d\n", voType);	
		return -1;
	}
#endif
	
	if(para == NULL)
	{
 		printf("lib_video.c@tl_vo_set_img_para: invalid para\n");	
		return -1;
	}
	
	if(para->voLuma > 100 || para->voLuma < 0)
	{
		printf("lib_video.c@tl_vo_set_img_para: invalid luminance=0x%x\n",para->voLuma);	
		para->voLuma = 50;
	}
	
	if(para->voContrast > 100 || para->voContrast < 0)
	{
		printf("lib_video.c@tl_vo_set_img_para: invalid contrast=0x%x\n",para->voContrast);	
		para->voContrast = 50;
	}
	
	if(para->voSaturation > 100 || para->voSaturation < 0)
	{
		printf("lib_video.c@tl_vo_set_img_para: invalid satuature=0x%x\n",para->voSaturation);	
		para->voSaturation = 50;
	}
	
	if(para->voHue > 100 || para->voHue < 0)
	{
		printf("lib_video.c@tl_vo_set_img_para: invalid hue=0x%x\n",para->voHue);	
		para->voHue = 50;
	}
	
	#ifdef HI3535
	ret = HI_MPI_VO_GetVideoLayerCSC(voType, &stpubcscparm);
	#else
	ret = HI_MPI_VO_GetDevCSC(voType, &stpubcscparm);
	#endif
	if(0 != ret)
	{
		printf("lib_video.c@HI_MPI_VO_GetDevCSC, VO_CSC_SATU error ret=0x%x",ret);
		return -1;
	}
 	stpubcscparm.u32Luma = para->voLuma;
	stpubcscparm.u32Contrast = para->voContrast;
	stpubcscparm.u32Hue = para->voHue;
	stpubcscparm.u32Satuature = para->voSaturation;
	
	#ifdef HI3535
	ret = HI_MPI_VO_SetVideoLayerCSC(voType, &stpubcscparm);
	#else
	ret = HI_MPI_VO_SetDevCSC(voType, &stpubcscparm);
	#endif
	if(0 != ret)
	{
		printf("lib_video.c@HI_MPI_VO_SetDevCSC, VO_CSC_LUMA error ret=0x%x",ret);
		return -1;
	}
	
	return 0;
}

int tl_vo_get_img_para(unsigned int voType, vo_image_para_t *para)
{
	VO_CSC_S  stpubcscparm;
	HI_S32 ret;
	
#ifdef DHD1_WBC_DSD0
	if((voType != VO_DEVICE_HD))
#else
	if((voType != VO_DEVICE_HD) && (voType != VO_DEVICE_CVBS))
#endif
	{
		printf("lib_video.c@tl_vo_get_img_para: invalid arg, VODEV=%d\n", voType);	
		return -1;
	}
	
	if(para == NULL)
	{
 		printf("lib_video.c@tl_vo_get_img_para: invalid para.n");	
		return -1;
	}
	
	#ifdef HI3535
	ret = HI_MPI_VO_GetVideoLayerCSC(voType, &stpubcscparm);
	#else
  	ret = HI_MPI_VO_GetDevCSC(voType, &stpubcscparm);
	#endif
	if(0 != ret)
	{
		printf("lib_video.c@HI_MPI_VO_GetDevCSC, VO_CSC_SATU error ret=0x%x",ret);
		return -1;
	}
	
	para->voLuma = stpubcscparm.u32Luma;
	para->voContrast= stpubcscparm.u32Contrast;
	para->voHue= stpubcscparm.u32Hue;
	para->voSaturation= stpubcscparm.u32Satuature; 
	
	return 0;
}

int tl_vo_set_img_para_all(vo_image_para_t *para)
{	
	HI_S32 ret;
	
	if(para == NULL)
	{
 		printf("lib_video.c@tl_vo_get_img_para: invalid para.n");	
		return -1;
	}
	
	ret = tl_vo_set_img_para(VO_DEVICE_HD, para);
	if(ret)
	{
		printf("lib_video.c@tl_vo_set_img_para_all, CVBS set error ret=0x%x",ret);
		return -1;
	}
	
#ifndef DHD1_WBC_DSD0
	ret = tl_vo_set_img_para(VO_DEVICE_CVBS, para);
	if(ret)
	{
		printf("lib_video.c@tl_vo_set_img_para_all, CVBS set error ret=0x%x",ret);
		return -1;
	}
#endif
	
	return 0;
}

void vo_default_attr(void)
{
	return;
}

