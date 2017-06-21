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
#include <pthread.h>

#include "lib_common.h"
#include "common.h"
#include "lib_md.h"

int tl_md_enable(int chn)
{
	int chn_max = ARG_VI_NUM_MAX;
	//printf("#%d#\n", __LINE__);
	
	if(chn < 0 || chn >= chn_max)
	{
		printf("lib_md.c@tl_md_enable: invalid arg, venc_idx=%d\n", chn);
		return -1;
	}
	
	plib_gbl_info->mdenable[chn] = 1;
	
	return 0;
}

int tl_md_disable(int chn)
{
	int chn_max = ARG_VI_NUM_MAX;
	//printf("#%d#\n", __LINE__);
	
	if(chn < 0 || chn >= chn_max)
	{
		printf("lib_md.c@tl_md_disable: invalid arg, venc_idx=%d\n", chn);
		return -1;
	}
	
	plib_gbl_info->mdenable[chn] = 0;
	
	return 0;
}

int tl_md_set_atr(int chn, md_atr_t *atr)
{
	return 0;
}

int tl_md_open(void)
{
	return 0;
}

int tl_md_read_result(md_result_t *result, unsigned int timeout_us)
{
	static int cur_md_chn = 0;
	int ch_max = ARG_VI_NUM_MAX;
	
	if(result == NULL)
	{
		printf("lib_md.c@tl_md_read_result: invalid arg\n");
		return -1;
	}
	
	int i = 0;
	for(i = 0; i < ch_max; i++)
	{
		cur_md_chn = (cur_md_chn + 1) % ch_max;
		if(plib_gbl_info->mdenable[cur_md_chn] == 0)
		{
			continue;
		}
		
		int chn = cur_md_chn;
		if(chn == 0)
		{
			//printf("chn%d read md result at:%u\n",chn,SystemGetMSCount());
		}
		
		if(plib_gbl_info->mdevent[chn])
		{
			//printf("chn%d md event\n",chn);
		}
		
		if(plib_gbl_info->mdenable[chn])
		{
			if(plib_gbl_info->mdevent[chn])
			{
				if(!plib_gbl_info->mdnotify[chn])
				{
					//printf("chn%d md start...\n",chn);
					result->venc_idx = chn;
					plib_gbl_info->mdnotify[chn] = 1;
					return 1;
				}
			}
			else
			{
				//if(plib_gbl_info->mdnotify[chn])
				{
					//printf("chn%d md resume...\n",chn);
					result->venc_idx = chn;
					plib_gbl_info->mdnotify[chn] = 0;
					return 2;
				}
			}
		}
		else
		{
			//if(plib_gbl_info->mdnotify[chn])
			{
				//printf("chn%d md resume...\n",chn);
				result->venc_idx = chn;
				plib_gbl_info->mdnotify[chn] = 0;
				return 2;
			}
		}
		
		return -1;
	}
	
	return -1;
}

int tl_md_close(void)
{
	return 0;
}

