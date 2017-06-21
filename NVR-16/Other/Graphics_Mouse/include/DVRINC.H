#ifndef __DVRINC_H__
#define __DVRINC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <assert.h>

#include <sys/mman.h>
#include <linux/fb.h>

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
#else
#include <asm/page.h>
#include "hi_comm_avenc.h"
#include "hi_comm_dwm.h"
#include "hi_comm_group.h"
#include "hi_comm_md.h"
#include "hi_comm_vpp.h"
#include "mpi_avenc.h"
#include "mpi_md.h"
#include "mpi_vpp.h"
#endif

#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_aio.h"
#include "hi_comm_ao.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_vdec.h"
#include "hi_comm_venc.h"
#include "hi_comm_vi.h"
#include "hi_comm_video.h"
#include "hi_comm_vo.h"
#include "hi_common.h"
#include "hi_debug.h"
#include "hi_errno.h"
#include "hi_type.h"
#include "hifb.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vdec.h"
#include "mpi_venc.h"
#include "mpi_vi.h"
#include "mpi_vo.h"

#include "hi_tde_api.h"

#define HI3515

#ifdef HI3515
#define TDE_SURFACE_S TDE2_SURFACE_S
#define HI_API_TDE_Open HI_TDE2_Open
#define TDE_COLOR_FMT_RGB1555 TDE2_COLOR_FMT_ARGB1555
enum video_standard_t {
	VIDEO_STANDARD_PAL = 0,	///< PAL制式。
	VIDEO_STANDARD_NTSC,	///< NTSC制式。
	VIDEO_STANDARD_SECAM	///< SECAM制式。
};
#endif

#if 0
#include "tde_interface.h"
#include "Video.h"
#include "Memory.h"
#include "Capture.h"
#include "OSDTitle.h"
#include "Split.h"
#include "ListManager.h"

#include "../APIs/Audio.h"
#include "Audio.h"
#include "../APIs/MotionDetect.h"
#include "MotionDetect.h"
#include "../APIs/BlindDetect.h"
#include "BlindDetect.h"

#include "../APIs/Split.h"
#include "Split.h"
#include "../APIs/Garphics.h"
#include "Graphics.h"

#include "PointFont.h"
#endif

#endif

