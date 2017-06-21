#include <stdio.h>
#include "mod_preview.h"
#include "lib_common.h"
#include "lib_preview.h"
#include "lib_misc.h"

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"

#include <sys/mman.h>

#include <fcntl.h>

#include <assert.h>

#include <linux/fb.h>

#include <sys/ioctl.h>

#include <errno.h>

 

#include "mkp_vd.h"

#include "hifb.h"

#include "lib_preview.h"

#ifndef BOOL
#define BOOL int
#endif


typedef enum hiVOU_DEV_E
{
	HD = 0,
	AD = 1,
	SD = 2,
	VOU_DEV_BUTT,
} VOU_DEV_E;

typedef enum hiVOU_GLAYER_E
{
	G0 = 0,
	G1 = 1,
	G2 = 2,
	G3 = 3,
	HC = 4,
	VOU_GLAYER_BUTT,
}VOU_GLAYER_E;

BOOL setOsdTransparency(int nfd,u8 trans)
{
	int fd;
	BOOL bShow = TRUE;
	HIFB_ALPHA_S stAlpha;
	fd = nfd;
	
    if(fd == -1)
    {
		printf("Failed to open fb window \n");

		return FALSE;
    }

	if(trans)
    {
		bShow = FALSE;

		if(ioctl(fd, FBIOGET_SHOW_HIFB, &bShow) == -1)
		{
			printf("Error reading fb show information.\n");
			close(fd);

			return FALSE;
		}
		else
		{
			bShow = !bShow;
		}

		if(bShow)
		{
			if(ioctl(fd, FBIOPUT_SHOW_HIFB, &bShow) == -1)
			{
				printf("Error reading variable information.\n");

 				close(fd);

				return FALSE;
			}
		}

		stAlpha.bAlphaEnable = HI_TRUE;

		stAlpha.bAlphaChannel = HI_FALSE;

		stAlpha.u8Alpha0 = 0x0;

		stAlpha.u8Alpha1 = trans;

		if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
		{
			printf("Set alpha failed!\n");

			close(fd);

			return -1;
		}
	}
	else
	{
		bShow = FALSE;
	}

	return TRUE;
}

int initStateOSD(void)
{
	HIFB_ALPHA_S stAlpha;

	struct fb_var_screeninfo varInfo;

	int size;

	u16* pmap;

	int fd;

	VD_BIND_S stBind;

	/*---------1 bind graphic layer G4(here used to show cursor) to HD, defaultly----------*/

	/*1.1 open vd fd*/

	fd = open("/dev/vd", O_RDWR, 0);

	if(fd < 0)
	{
		printf("open vd failed!\n");

		return -1;
	}

	stBind.s32GraphicId = HC;
	stBind.DevId = HD;

	if (ioctl(fd, VD_SET_GRAPHIC_BIND, &stBind) != HI_SUCCESS)
	{
		printf("[fd:%d]set bind glayer %d to dev %d failed!\n",fd,HC,HD);

		close(fd);

		return -1;
	}

	/*1.3 bind G1 to SD*/

	stBind.s32GraphicId = G1;
	stBind.DevId = SD;

	if (ioctl(fd, VD_SET_GRAPHIC_BIND, &stBind) != HI_SUCCESS)
	{
		printf("[fd:%d]set bind glayer %d to dev %d failed!\n",fd,HC,SD);

		close(fd);

		return -1;
	}

	fd = open("/dev/fb4", O_RDWR);

	setOsdTransparency(fd ,0xff);

	if(fd == -1)
	{
		printf("Failed to open fb device\n");

		return FALSE;
	}

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0x90;

	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");

		close(fd);

		return -1;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");

		close(fd);

		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 800;
	varInfo.yres = varInfo.yres_virtual = 600;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");

		close(fd);

		return FALSE;
	}

	size = 800 * 600 * 2;

	close(fd);

	fd = open("/dev/fb1", O_RDWR);

	if(fd == -1)
	{
		printf("Failed to open fb device\n");

		return FALSE;
	}
	
	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;

	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");

		close(fd);

		return -1;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
        printf("Failed ioctl FBIOGET_VSCREENINFO on\n");

        close(fd);

        return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 720;
	varInfo.yres = varInfo.yres_virtual = 576;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");

		close(fd);

		return FALSE;
	}

	size = 720 * 576 * 2;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);

	if (pmap == MAP_FAILED)
	{
		close(fd);

		return FALSE;
	}

	memset(pmap ,0x00,size);

	close(fd);

	fd = open("/dev/fb0", O_RDWR);

	if(fd == -1)
	{
		printf("Failed to open fb device\n");

		return FALSE;
	}

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;

	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");

		close(fd);

		return -1;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");

		close(fd);

		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 800;
	varInfo.yres = varInfo.yres_virtual = 600;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");

		close(fd);

		return FALSE;
	}

	size = 800 * 600 * 2;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);

	if (pmap == MAP_FAILED)
	{
		close(fd);

		return FALSE;
	}

	memset(pmap ,0x00,size);

	fd = open("/dev/fb2", O_RDWR);

	if(fd == -1)
	{
		printf("Failed to open fb device\n");

		return FALSE;
	}

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;

	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");

		close(fd);

		return -1;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");

		close(fd);

		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 720;
	varInfo.yres = varInfo.yres_virtual = 576;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");

		close(fd);

		return FALSE;
	}

	size = 720 * 576 * 2;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);

	if (pmap == MAP_FAILED)
	{
		close(fd);

		return FALSE;
	}

	memset(pmap ,0x00,size);

	fd = open("/dev/fb0", O_RDWR);

	if(fd == -1)
	{
		printf("Failed to open fb device\n");

		return FALSE;
	}

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;

	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");

		close(fd);

		return -1;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");

		close(fd);

		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 800;
	varInfo.yres = varInfo.yres_virtual = 600;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");

		close(fd);

		return FALSE;
	}

	size = 800 * 600 * 2;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);

	if (pmap == MAP_FAILED)
	{
		close(fd);

		return FALSE;
	}

	memset(pmap ,0x00,size);

	fd = open("/dev/fb1", O_RDWR);

	if(fd == -1)
	{
		printf("Failed to open fb device\n");

		return FALSE;
	}

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;

	if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");

		close(fd);

		return -1;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOGET_VSCREENINFO on\n");

		close(fd);

		return FALSE;
	}

	varInfo.xres = varInfo.xres_virtual = 720;
	varInfo.yres = varInfo.yres_virtual = 576;
	varInfo.bits_per_pixel = 16;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo) == -1)
	{
		printf("Failed ioctl FBIOPUT_VSCREENINFO on \n");

		close(fd);

		return FALSE;
	}

	size = 720 * 576 * 2;
	pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);

	if (pmap == MAP_FAILED)
	{
		close(fd);

		return FALSE;
	}

	memset(pmap ,0x00,size);
}

void test_preview(void)
{
    SPreviewPara sPreviewPara;
    
    sPreviewPara.emPreviewMode = EM_PREVIEW_4SPLITS;
    sPreviewPara.nModePara = 0;
    
    while(1)
    {

    	ModPreviewStart(&sPreviewPara);
    	sleep(5);
        
    	ModPreviewZoomChn(0);
    	sleep(5);

    	ModPreviewResume(0);
    	sleep(5);
        
    	ModPreviewZoomChn(1);
    	sleep(5);

    	ModPreviewResume(0);
    	sleep(5);
        
    	ModPreviewZoomChn(2);
    	sleep(5);

    	ModPreviewResume(0);
    	sleep(5);

    	ModPreviewZoomChn(3);
    	sleep(5);

    	ModPreviewResume(0);
    	sleep(5);

    	ModPreviewStop();
    	sleep(5);

    	ModPreviewResume(0);
    	sleep(5);
    }
}

void test_patrol()
{
    u8 nStopModePara[4];
    int i;
    SPreviewPatrolPara sPreviewPatrol;

    sPreviewPatrol.emPreviewMode = EM_PREVIEW_1SPLIT;
    sPreviewPatrol.nStops = 4;
    sPreviewPatrol.nInterval = 2;
    sPreviewPatrol.nIsPatrol = 1;
    sPreviewPatrol.pnStopModePara = nStopModePara;
    for (i = 0; i < sizeof(nStopModePara); i ++)
    {
        nStopModePara[i] = i;
    }
    
    ModPreviewSetPatrol(&sPreviewPatrol);

    sleep(20);
    ModPreviewStopPatrol();

    sleep(20);
    ModPreviewStartPatrol();

    while (1)
    {
        sleep(1);
    }
}

int main()
{    
    int init_param = 0;
    init_param |= TL_128MDDR;
    init_param |= TL_VGA;
    init_param |= TL_NTSC;

    //tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);
	tl_hslib_init_c(TL_BOARD_TYPE_R9624T, init_param);

    tl_vga_resolution(VGA_RESOL_P_800_60);
    
	// TEST SWITCH 1-SCR
	int i = 0;
	while(1)
	{
    	tl_preview(PREVIEW_1D1, i++%4);
		usleep(1000*1000);
    }

    initStateOSD();

    //tl_audio_open( );

    printf("just for test\n");

    SPreviewInitPara sPreviewInit;

    sPreviewInit.nAudioNum = 4;
    sPreviewInit.nVideoNum = 4;
    sPreviewInit.pfnPreviewCb = NULL;
    ModPreviewInit(&sPreviewInit);

    test_preview();

    //test_patrol();

    return 0;
}
