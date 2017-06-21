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
#include "hi_comm_venc.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"

#include "lib_common.h"
#include "common.h"
#include "lib_misc.h"
#include "lib_common.h"
#include "vio_common.h"

extern VO_INTF_SYNC_E g_enIntfSync;

int tl_rs485_ctl(int ctl)
{
	unsigned int buf[5] = {0};
	
	if(ctl)
	{
		buf[0] = 1;
	}
	else
	{
		buf[0] = 0;
	}
	
	#ifdef HI3531
	if(ioctl(plib_gbl_info->fd_tl, TL_RS485_CTL, buf) < 0)
	#else
	if(ioctl(plib_gbl_info->fd_tl, TL_RS485_CTL, buf[0]) < 0)
	#endif
	{
		perror("lib_misc.c@tl_rs485_ctl: RS485_CTL error");
		return -1;
	}
	
	return 0;
}

int tl_buzzer_ctl(int ctl)
{
	unsigned int buf[5] = {0};
	
	if(ctl)
	{
		buf[0] = 1;
	}
	else
	{
		buf[0] = 0;
	}
	
	#ifdef HI3531
	if(ioctl(plib_gbl_info->fd_tl, TL_BUZZER_CTL, buf) < 0)
	#else
	if(ioctl(plib_gbl_info->fd_tl, TL_BUZZER_CTL, buf[0]) < 0)
	#endif
	{
		perror("lib_misc.c@tl_buzzer_ctl: BUZZER_CTL error");
		return -1;
	}
	
	//printf("#######################BUZZER CTL:%d#######################\n",ctl);
	
	return 0;
}

int tl_hdmi_detecte(void)
{
	return 0;
}

int tl_vga_resolution(int vga_resol)
{
	VO_PUB_ATTR_S stVoPubAttr; 
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32FrmRate;
	
	LIB_VO_MODE_E previewMode = VO_MODE_16MUX;
	
	printf("start set vga resolution...\n");
	
	switch(vga_resol)
	{
		case VGA_RESOL_P_800_60:
		case VGA_RESOL_N_800_60:
		case VGA_RESOL_P_800_75:
		case VGA_RESOL_N_800_75:
			g_enIntfSync = VO_OUTPUT_800x600_60;
			break;
		case VGA_RESOL_P_1024_60:
		case VGA_RESOL_N_1024_60:
		case VGA_RESOL_P_1024_75:
		case VGA_RESOL_N_1024_75:
			g_enIntfSync = VO_OUTPUT_1024x768_60;
			break;
		case VGA_RESOL_P_1280_60:
		case VGA_RESOL_N_1280_60:
			g_enIntfSync = VO_OUTPUT_1280x1024_60;
			break;
		case VGA_RESOL_P_640_60:
		case VGA_RESOL_N_640_60:
			g_enIntfSync = VO_OUTPUT_USER;
			break;
		case HDMI_RESOL_720P50:
			g_enIntfSync = VO_OUTPUT_720P50;
			break;
		case HDMI_RESOL_720P60:
			g_enIntfSync = VO_OUTPUT_720P60;
			break;
		case HDMI_RESOL_1080I50:
			g_enIntfSync = VO_OUTPUT_1080I50;
			break;
		case HDMI_RESOL_1080I60:
			g_enIntfSync = VO_OUTPUT_1080I60;
			break;	
		case HDMI_RESOL_1080P25:
			g_enIntfSync = VO_OUTPUT_1080P25;
			break;
		case HDMI_RESOL_1080P30:
			g_enIntfSync = VO_OUTPUT_1080P30;
			break;
		case HDMI_RESOL_1080P50:
			g_enIntfSync = VO_OUTPUT_1080P50;
			break;
		case HDMI_RESOL_1080P60:
			g_enIntfSync = VO_OUTPUT_1080P60;
			break;
		default:
			g_enIntfSync = VO_OUTPUT_1080P50;
			printf("<ERROR>lib_misc.c@tl_vga_resolution err.ret=%d\n",vga_resol);
			return -1;
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
	
#ifdef DHD1_WBC_DSD0
	s32Ret = HI_MPI_VO_DisableWbc(VO_DEVICE_HD);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("HI_MPI_VO_DisableWbc fail!%#x\n", s32Ret);
	}
#endif
	
	SAMPLE_COMM_VO_HdmiStop();
	SAMPLE_COMM_VO_StopChn(VO_DEVICE_HD, previewMode); 
	SAMPLE_COMM_VO_StopDevLayer(VO_DEVICE_HD);
	
	u32FrmRate = IS_PAL() ? 25 : 30;
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
	//s32Ret = SAMPLE_COMM_VO_StartDevLayer(VO_DEVICE_HD, &stVoPubAttr, u32FrmRate);
	//s32Ret = SAMPLE_COMM_VO_StartDevLayer(VO_DEVICE_HD, &stVoPubAttr, u32FrmRate*2);
	s32Ret = SAMPLE_COMM_VO_StartDevLayer(VO_DEVICE_HD, &stVoPubAttr, HD_FRAMERATE);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_StartDevLayer failed! vodev=%d\n",VO_DEVICE_HD);
	}
	
	s32Ret = SAMPLE_COMM_VO_StartChn(VO_DEVICE_HD, stVoPubAttr.enIntfSync, previewMode);
	if (HI_SUCCESS != s32Ret)
	{
		LIB_PRT("SAMPLE_COMM_VO_StartChn failed! vodev=%d\n",VO_DEVICE_HD);
	}
	
	// if it's displayed on HDMI, we should start HDMI 
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
		{
			LIB_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
		}
	}
	
#ifdef DHD1_WBC_DSD0
	#ifdef HI3535//csp modify 20150110
	printf("lib_misc.c@rz_vga_resolution: vga resolution = %d\n",vga_resol);
	return 0;
	#endif

	VO_WBC_ATTR_S stWbcAttr;
	
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
	printf("HI_MPI_VO_GetWbcAttr-2 w=%d h=%d\n",stWbcAttr.stTargetSize.u32Width,stWbcAttr.stTargetSize.u32Height);
	
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
	
	printf("HI_MPI_VO_SetWbcAttr-2 w=%d h=%d\n",stWbcAttr.stTargetSize.u32Width,stWbcAttr.stTargetSize.u32Height);
	
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
#endif
	
	printf("lib_misc.c@tl_vga_resolution: resolution = %d\n",vga_resol);
	
	return 0;
}

int tl_license_check(char *dataQ, int size)
{
	char buf[8];
	int ret;
	
	memcpy(buf, dataQ, 8);
	
	ret = ioctl(plib_gbl_info->fd_tl, TL_PW_VALID,buf);
	if(ret)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int tl_keypad_ctl(int ctl)
{
	return 0;
}

int tl_sata_status(void)
{
	return 0;
}

int tl_display_ctl(int ctl)
{
    return 0;
}

int config_hdmi_resolution(int resol)
{
	return 0;
}

int tl_VI_Enable_UserPic(VI_CHN chn)
{
	return 0;
}

int tl_VI_Disable_UserPic(VI_CHN chn)
{
	return 0;
}

HI_S32 tl_VI_SetUserPic(HI_CHAR *pszYuvFile, HI_U32 u32Width, HI_U32 u32Height, HI_U32 u32Stride)
{
	return 0;
}

