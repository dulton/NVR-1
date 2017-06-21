/******************************************************************************

	TL hslib video cover
	
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
#include "mpi_vdec.h"
#include "hi_comm_vdec.h"

#include "hi_comm_vo.h"
#include "mpi_vo.h"

#include "common.h"
#include "lib_venc_osd.h"

int tl_venc_osd_create(int venc_idx, venc_osd_atr_t *atr, int is_public)
{
	return HI_SUCCESS;
}

int tl_venc_osd_set_bitmap(int id, char *bitmap_data, unsigned int width, unsigned int height)
{
	return HI_SUCCESS;
}

int tl_venc_osd_set_postion(int id, int x, int y)
{
	return HI_SUCCESS;
}

int tl_venc_osd_show(int id, int show)
{
	return HI_SUCCESS;
}

void tl_venc_osd_destroy(int id)
{
	return;
}

