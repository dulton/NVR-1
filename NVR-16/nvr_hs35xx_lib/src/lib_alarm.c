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

unsigned int tl_get_alarm_input(void)
{
	unsigned int buf = 0;
	
	if(ioctl(plib_gbl_info->fd_tl, TL_ALARM_IN, &buf) < 0)
	{
		printf("lib_alarm.c@get_alarm_input: GET_ALM_IN error\n");
		return 0;
	}
	
	return (buf & 0xffff);
}

int tl_set_alarm_out(int channel, int val)
{
	unsigned int buf[5] = {0};
	
	buf[0] = channel;
	buf[1] = val;
	
	if(ioctl(plib_gbl_info->fd_tl, TL_ALARM_OUT, buf) < 0)
	{
		printf("lib_alarm.c@set_alarm_out: SET_ALARM_OUT error\n");
		return -1;
	}
	
	return 0;
}

