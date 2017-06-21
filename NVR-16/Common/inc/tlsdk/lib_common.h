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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
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
	TL_BOARD_TYPE_R9800_MA,		//4SDI
	TL_BOARD_TYPE_R9800_DA,		//2SDI+8960H
	TL_BOARD_TYPE_R9800_2SDI,	//2SDI
	TL_BOARD_TYPE_R9816_SA,		//16D1
	TL_BOARD_TYPE_R9704S,		//4D1
	TL_BOARD_TYPE_R9708S,		//8D1
	TL_BOARD_TYPE_R9716S,		//16CIF
	TL_BOARD_TYPE_R9716D,		//16D1
	//NVR
	TL_BOARD_TYPE_NR2116 = 100,
	TL_BOARD_TYPE_NR3116,
	TL_BOARD_TYPE_NR1004,
	TL_BOARD_TYPE_NR1008,
	TL_BOARD_TYPE_NR3124,
	TL_BOARD_TYPE_NR3132
}tl_board_type_e;

typedef enum
{
	VGA_RESOL_AUTO = 0,
	VGA_RESOL_P_800_60 = 1,
	VGA_RESOL_N_800_60,
	VGA_RESOL_P_1024_60,
	VGA_RESOL_N_1024_60,
	VGA_RESOL_P_1280_60,		//5
	VGA_RESOL_N_1280_60,
	VGA_RESOL_P_640_60,			//7
	VGA_RESOL_N_640_60,
	VGA_RESOL_P_800_75,			//9
	VGA_RESOL_N_800_75,
	VGA_RESOL_P_1024_75,		//11
	VGA_RESOL_N_1024_75,
	VGA_RESOL_P_1280_75,		//13
	VGA_RESOL_N_1280_75,
	HDMI_RESOL_720P60,			//15
	HDMI_RESOL_1080I50,			//16
	HDMI_RESOL_1080I60,			//17
	HDMI_RESOL_1080P25,			//18
	HDMI_RESOL_1080P30,			//19
	HDMI_RESOL_1080P50,			//20
	HDMI_RESOL_1080P60,			//21
	HDMI_RESOL_720P50,			//22
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
#define TL_VO_WBC				0x100	//HDMI->CVBS

extern int g_client_id;

void tl_hslib_init(tl_board_type_e type);

/*args: 
		client_id  
		bit[0]: TL_CHANNEL_TYPE_L_TO_R   通道顺序从左到右颠倒(HI3515)
		bit[1]: TL_CHANNEL_TYPE_R_TO_L   通道顺序从右到左颠倒(HI3515)
		bit[2]: TL_PAL	视频制式PAL(通用)
		bit[3]: TL_NTSC   视频制式NTSC(通用)
		bit[4]: TL_128MDDR   DDR为128M(Hi3515)
		bit[5]: TL_8D1     8D1(HI3515)
		bit[6]: TL_VGA     支持VGA分辨率切换功能(HI3515)
		bit[7]: TL_FORMAT_AUTO   (无作用)
		bit[8]: TL_VO_WBC  (HI3521、HI3520A)
		bit[11-9]: platform  
				   000    DM6441
				   001    Hisilicom
				   010    Fullhan
		bit[16-12]: chipset	
				   00000   hi3510
				   00001   hi3511
				   00010   hi3512
				   00011   hi3508
				   00100   hi3515
		bit[21-17]: software code specifications
		     		   00000  16CIF(hi3520A)
		     		   00001  4D+12CIF(hi3521)
		     		   00010   16D1(hi3521)
		     		   00011   8*960H(hi3521)
		bit[27-22]: custom id
				   000000   tongli
*/

void tl_hslib_init_c(tl_board_type_e type, int client_id);

void rebootSlaveByCmd(void);

typedef enum
{
	VI_CAP_SIZE_CIF = 0,		//PAL:352x288, NTSC:352x240
	VI_CAP_SIZE_QCIF = 1,		//PAL:176x144, NTSC:176x112
	VI_CAP_SIZE_D1 = 2,			//PAL:704x576, NTSC:704x480
	VI_CAP_SIZE_HALF_D1 = 3,	//PAL:352*576, NTSC:352*480
	VI_CAP_SIZE_720P = 4,		//1280*720
	VI_CAP_SIZE_1080P = 5,		//1920*1080
	VI_CAP_SIZE_960H,			//PAL:960x576, NTSC:960x480
	VI_CAP_NO_INPUT,
	VI_CAP_UNKNOWN_FORMAT,
}vi_cap_size_e;

struct vi_standard_e
{
	vi_cap_size_e vi_cap_size;
	unsigned int framerate;
};

#include "ipcamera.h"

//设置指定通道对应的网络摄像机
//摄像机参数为空则清除该通道之前之前设置的网络摄像机
int tl_set_ipcamera(int channel, ipc_unit *ipcam);

//获取指定通道对应的网络摄像机
int tl_get_ipcamera(int channel, ipc_unit *ipcam);

#ifdef __cplusplus
}
#endif

#endif

