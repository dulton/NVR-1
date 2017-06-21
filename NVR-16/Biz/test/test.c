#include "biz.h"

//#define TEST_INIT

#ifdef TEST_INIT
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "mkp_vd.h"
#include "hifb.h"

#include "lib_common.h"
#include "lib_misc.h"

int initStateOSD(void);
#endif


//业务层事件回调函数申明
void BizEventDeal(SBizEventPara* psBizEventPara);

//预览测试
void* TestPreview(void* arg);
void TestPreviewSwitch(void);
void TestStartPreview(EMBIZPREVIEWMODE emBizPreviewMode, u8 nModePara);

//配置测试测试
void TestConfig(void);

int main()
{
    SBizInitPara sBizInitPara;
    pthread_t previewThrd;
    s32 ret;
	
    #ifdef TEST_INIT
    int init_param = 0;
    init_param |= TL_128MDDR;
    init_param |= TL_VGA;
    init_param |= TL_PAL;
	
    tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);
	
    tl_vga_resolution(VGA_RESOL_P_800_60);
    
    initStateOSD();
    #endif
	
    sBizInitPara.nModMask = MASK_BIZ_MOD_ALL;
    sBizInitPara.pfnBizEventCb = BizEventDeal;
    if (BIZ_SUCCESS != BizInit(&sBizInitPara))
    {
        APPPRT("BizInit failed\n");
        return -1;
    }
	
    #if 0
    ret = pthread_create(&previewThrd, NULL, TestPreview, NULL);
    if (ret != 0)
    {
        return -2;
    }
    #endif
	
    TestConfig();

    while (1)
    {
    	sleep (1);
    }
    
    return 0;
}

//业务层事件回调函数定义
void BizEventDeal(SBizEventPara* psBizEventPara)
{
	
}

//开始预览
void TestStartPreview(EMBIZPREVIEWMODE emBizPreviewMode, u8 nModePara)
{
    SBizPreviewPara sBizPreviewPara;
    
    sBizPreviewPara.emBizPreviewMode = emBizPreviewMode;
    sBizPreviewPara.nModePara = nModePara;
    BizStartPreview(&sBizPreviewPara);
}

//预览切换测试
void TestPreviewSwitch(void)
{
    TestStartPreview(EM_BIZPREVIEW_4SPLITS, 0);
    
    sleep(2);
    BizChnPreview(0);

    sleep(2);
    BizChnPreview(1);
    
    sleep(2);
    BizResumePreview(0);
    
    sleep(2);
    BizChnPreview(2);

    sleep(2);
    BizResumePreview(0);

    sleep(2);
    BizChnPreview(3);

    sleep(2);
    BizResumePreview(0);
    
    sleep(2);
    BizStopPreview();

    sleep(2);
    BizResumePreview(0);
}


//预览测试程序
void* TestPreview(void* arg)
{
    while (1)
    {
        TestPreviewSwitch();
    }
}



//配置测试测试
void TestConfig(void)
{
    EMBIZCFGPARALIST emParaType = EM_BIZ_CFG_PARALIST_BAUDRATE;//EM_BIZ_CFG_PARALIST_PTZPROTOCOL;//EM_BIZ_CFG_PARALIST_LANGSUP;
    u8 nMaxListLen = 20, index = 0;
    u8 nMaxStrLen = 64;
    s8* pstrList[nMaxListLen];
    s8 strList[nMaxListLen][nMaxStrLen];
    u8 nRealListLen;
    s32 ret, i;

    for (i = 0; i < nMaxListLen; i++)
    {
        pstrList[i] = &strList[i];
        printf("pstrList[%d]=%p\n", i, pstrList[i]);
    }
    
    ret = BizConfigGetParaStr(emParaType, pstrList, &nRealListLen, nMaxListLen, nMaxStrLen);
    if (0 == ret)
    {
        printf("nRealListLen=%lu\n", nRealListLen);
        for (i = 0; i < nRealListLen; i++)
        {
            printf("%s\n", strList[i]);
        }
        #if 1
        for (i = 0; i < nRealListLen; i++)
        {
            if (0 == BizConfigGetParaListValue(emParaType, i, &ret))
            {
                printf("BizConfigGetParaListValue ok index=%d value=%d\n", i, ret);
                if (0 == BizConfigGetParaListIndex(emParaType, ret, &index))
                {
                    printf("BizConfigGetParaListIndex ok index=%d value=%d\n", index, ret);
                }
            }
            else
            {
                printf("BizConfigGetParaListValue FAILED\n");
            }
        }
        #endif
    }
    else
    {
         PUBPRT("EM_BIZ_CFG_PARALIST_LANGSUP err");
    }
}

#ifdef TEST_INIT
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

int initStateOSD()
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
#endif


