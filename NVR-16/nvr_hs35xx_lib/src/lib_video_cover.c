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
#include <unistd.h>

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
#include "lib_video_cover.h"

int tl_video_cover_create(int chn, video_cover_atr_t *atr)
{
	return 0;
}

void tl_video_cover_destroy(int id)
{
	return;
}

