
#ifndef __LIB_COMMON_H
#define __LIB_COMMON_H

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vpss.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vda.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_comm_hdmi.h"
#include "hi_defines.h"

#ifdef HI3535
//#include "sample_comm.h"
#endif

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vda.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_hdmi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>

#include "tl-tw286x.h"
#include "tl-R9624.h"

typedef enum{
	TL_BOARD_TYPE_R9508,
	TL_BOARD_TYPE_R9504,	
	TL_BOARD_TYPE_R9508_shenshiying,
	TL_BOARD_TYPE_R9504_shenshiying,
	TL_BOARD_TYPE_R9504_4D1_shenshiying,
	TL_BOARD_TYPE_R9504_4D1,
	TL_BOARD_TYPE_R9508_jiuan,
	TL_BOARD_TYPE_R9508_TW2868,
	TL_BOARD_TYPE_R9504_LG1702,
	TL_BOARD_TYPE_R9508_RN6266,
	TL_BOARD_TYPE_R9504_RN6266,
	TL_BOARD_TYPE_R9508_LG1702,
	TL_BOARD_TYPE_R9508S,
	TL_BOARD_TYPE_R9504S,
	TL_BOARD_TYPE_R9508SJ,
	TL_BOARD_TYPE_R9504SJ,
	TL_BOARD_TYPE_R9508_youshenjia,
	TL_BOARD_TYPE_R9504_youshenjia,
	TL_BOARD_TYPE_R9504_4D1_youshenjia,
	TL_BOARD_TYPE_R9508_TIANMIN,
	TL_BOARD_TYPE_R9504_TIANMIN,
	TL_BOARD_TYPE_R2004MX,
	TL_BOARD_TYPE_R2008MX,
	TL_BOARD_TYPE_R9504_KEAN,
	TL_BOARD_TYPE_R9508_KEAN,
	TL_BOARD_TYPE_R9316,
	TL_BOARD_TYPE_R9624T,
	TL_BOARD_TYPE_R9632SL,
	TL_BOARD_TYPE_R9632SH,
	TL_BOARD_TYPE_R9624SL,
	TL_BOARD_TYPE_R9624SH,
	TL_BOARD_TYPE_R9616S,
	TL_BOARD_TYPE_R9608S,
	TL_BOARD_TYPE_R9516,
	TL_BOARD_TYPE_R9516_16cif,
	TL_BOARD_TYPE_R9516_TW2826,
	TL_BOARD_TYPE_R9516SJ_TW2826,
	TL_BOARD_TYPE_R9516_FUHONG_TW2826,
	TL_BOARD_TYPE_R9508_junmingshi,
	TL_BOARD_TYPE_R9800_MA,	// 4SDI
	TL_BOARD_TYPE_R9800_DA,	// 2SDI+8960H
	TL_BOARD_TYPE_R9800_2SDI,	// 2SDI
	TL_BOARD_TYPE_R9816_SA,	//16D1
}tl_board_type_e;


typedef enum{
	VGA_RESOL_AUTO = 0,
	VGA_RESOL_P_800_60 =1,	// 1
	VGA_RESOL_N_800_60,
	VGA_RESOL_P_1024_60, 	// 3
	VGA_RESOL_N_1024_60,  
	VGA_RESOL_P_1280_60, 	//5
	VGA_RESOL_N_1280_60,
	VGA_RESOL_P_640_60,  	// 7
	VGA_RESOL_N_640_60,  
	VGA_RESOL_P_800_75,		// 9
	VGA_RESOL_N_800_75,
	VGA_RESOL_P_1024_75,	// 11
	VGA_RESOL_N_1024_75,	
	VGA_RESOL_P_1280_75,	//13
	VGA_RESOL_N_1280_75,	
	HDMI_RESOL_720P60,		//15	
	HDMI_RESOL_1080I50,		//16		
	HDMI_RESOL_1080I60,		//17	
	HDMI_RESOL_1080P25,		//18	
	HDMI_RESOL_1080P30,		//19
	HDMI_RESOL_1080P50,		//20
	HDMI_RESOL_1080P60,		//21
	HDMI_RESOL_720P50,		//22
}tl_vga_resolution_type_e;

#define TL_CHANNEL_TYPE_L_TO_R	0x00
#define TL_CHANNEL_TYPE_R_TO_L	0x01
#define TL_PAL					0x02
#define TL_NTSC					0x04
#define TL_128MDDR				0x08	
#define TL_8D1					0x10	
#define TL_VGA					0x20	
#define TL_FORMAT_AUTO			0x40
#define TL_HDMI					0x80

extern int g_client_id;

void tl_hslib_init(tl_board_type_e type);
void tl_hslib_init_c(tl_board_type_e type, int client_id);

typedef enum
{	
	VI_CAP_SIZE_CIF = 0,		//PAL:352x288, NTSC:352x240
	VI_CAP_SIZE_QCIF = 1,		//PAL:176x144, NTSC:176x112
	VI_CAP_SIZE_D1 = 2,			//PAL:704x576, NTSC:704x480
	VI_CAP_SIZE_HALF_D1 = 3,  	//PAL:352*576, NTSC:352*480
	VI_CAP_SIZE_720P = 4,  		// 1280*720
	VI_CAP_SIZE_1080P = 5,  	// 1920*1080
	VI_CAP_SIZE_960H,			//PAL:960x576, NTSC:960x480
	VI_CAP_NO_INPUT,
	VI_CAP_UNKNOWN_FORMAT,
}vi_cap_size_e;

struct vi_standard_e{
	vi_cap_size_e vi_cap_size;
	unsigned int framerate;
};

void tl_hslib_init_framerate(tl_board_type_e type, int client_id, struct vi_standard_e *vi_standard);

#define SSP_READ_ALT 0x1
#define SSP_WRITE_ALT 0x3
int previous_encoder_video_standard_detect(int chn, struct vi_standard_e *standard);

void rebootSlaveByCmd(void);
HI_S32 SAMPLE_COMM_VPSS_Start(HI_S32 s32FirstGrpIdx, HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt,VPSS_GRP_ATTR_S *pstVpssGrpAttr);
HI_S32 SAMPLE_COMM_VPSS_Stop(HI_S32 s32FirstGrpIdx, HI_S32 s32GrpCnt, HI_S32 s32ChnCnt);
#endif
