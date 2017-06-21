#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include "mod_alarm.h"
#include "lib_common.h"
#include "lib_misc.h"
#include "lib_venc.h"

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"

#include "mkp_vd.h"

#include "hifb.h"

#include "lib_preview.h"

#if 1

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

    return 0;
}
#endif

void AlarmEventDeal(SAlarmCbData* psAlarmCbData)
{
    if (psAlarmCbData)
    {
        printf("SAlarmCbData:event:%d chn:%d, nData:%d, time:%lu\n", psAlarmCbData->emAlarmEvent, psAlarmCbData->nChn
            , psAlarmCbData->nData, psAlarmCbData->nTime);
    }
    else
    {
        printf("%s:lin3%d err\n", __FUNCTION__, __LINE__);
    }
}

int main(int argc, char* argv[])
{
    AlaHdr tstAlaHdr;
    int init_param = 0;
    init_param |= TL_128MDDR;
    init_param |= TL_VGA;
    init_param |= TL_PAL;

    tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);

    tl_vga_resolution(VGA_RESOL_P_800_60);

    initStateOSD();

//    tl_venc_open();

    SAlarmInitPara mypara;
    mypara.nVideoChnNum = 4;
    mypara.nAlarmSensorNum = 4;
    mypara.nAlarmOutNum = 1;
    mypara.nBuzzNum = 1;
    mypara.nVBlindLuma = 0x00f00000;
    mypara.pfnAlarmCb = AlarmEventDeal;
    if (0 != ModAlarmInit(&mypara, &tstAlaHdr))
    {
        printf("ModAlarmInit failed\n");
        return -1;
    }

	SAlarmPara para;
	memset(&para, 0, sizeof(SAlarmPara));

	#if 0
	para.sAlaOutPara.emType = EM_ALARM_SENSOR_NORMALOPEN;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_ALARMOUT,0, &para);
    printf("*\n");
	sleep(3);
	para.sAlaOutPara.emType = EM_ALARM_SENSOR_NORMALCLOSE;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_ALARMOUT,0, &para);
    printf("*\n");
	sleep(3);
	para.sAlaOutPara.emType = EM_ALARM_SENSOR_NORMALOPEN;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_ALARMOUT,0, &para);
    printf("*\n");
	sleep(3);
	#endif

	#if 1
	para.sAlaSchedule.nSchType = EM_ALARM_SCH_WEEK_DAY;
    para.sAlaSchedule.nSchTime[1][0].nStartTime = 1;
    para.sAlaSchedule.nSchTime[1][0].nStopTime = 24 * 3600 - 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_SCHEDULE_VMOTION,0, &para);

	memset(&para, 0, sizeof(SAlarmPara));
	para.sAlaBuzzPara.nEnable = 1;
    para.sAlaBuzzPara.nDelay= 0;
	ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_BUZZ,0, &para);

	memset(&para, 0, sizeof(SAlarmPara));
	para.sAlaDispatch.nFlagBuzz = 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_DISPATCH_VMOTION,0, &para);

	getchar();
	
	memset(&para, 0, sizeof(SAlarmPara));
	para.sAlaVMotionPara.emSetType = EM_ALMARM_VMOTION_AREA_SELECTALL;
    para.sAlaVMotionPara.emResol = EM_RESOL_D1;
    para.sAlaVMotionPara.nEnable = 1;
    para.sAlaVMotionPara.nSensitivity = 5;
    para.sAlaVMotionPara.nDelay = 3;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_VMOTION, 0, &para);

	getchar();
	
	para.sAlaVMotionPara.nEnable = 0;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_VMOTION, 0, &para);
    #endif
	
	#if 0
	para.sAlaSchedule.nSchType = EM_ALARM_SCH_WEEK_DAY;
    para.sAlaSchedule.nSchTime[1][0].nStartTime = 1;
    para.sAlaSchedule.nSchTime[1][0].nStopTime = 24 * 3600 - 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_SCHEDULE_SENSOR,0, &para);

	memset(&para, 0, sizeof(SAlarmPara));
	para.sAlaBuzzPara.nEnable = 1;
    para.sAlaBuzzPara.nDelay= 0;
	//para.sAlaBuzzPara.nDuration = 1;
	//para.sAlaBuzzPara.nInterval = 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_BUZZ,0, &para);

	memset(&para, 0, sizeof(SAlarmPara));
	para.sAlaDispatch.nFlagBuzz = 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_DISPATCH_SENSOR,0, &para);
	
	memset(&para, 0, sizeof(SAlarmPara));
	para.sAlaSensorPara.emType = EM_ALARM_SENSOR_HIGH;
    para.sAlaSensorPara.nEnable = 1;
    para.sAlaSensorPara.nDelay = 0;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_SENSOR,0, &para);
	printf("*\n");
	sleep(5);
	para.sAlaSensorPara.nEnable = 0;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_SENSOR,0, &para);
	printf("**\n");
	sleep(5);
	para.sAlaSensorPara.nEnable = 1;
    para.sAlaSensorPara.nDelay = 5;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_SENSOR,0, &para);
	printf("***\n");
	sleep(5);
	para.sAlaSensorPara.emType = EM_ALARM_SENSOR_LOW;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_SENSOR,0, &para);
	printf("****\n");
	sleep(5);
	printf("*****\n");	
	#endif
	
	#if 0   
    // 设置报警输入参数
    SAlarmPara para1, paraget1;
    para1.sAlaSensorPara.emType = EM_ALARM_SENSOR_HIGH;
    para1.sAlaSensorPara.nEnable = 0;
    para1.sAlaSensorPara.nDelay = 2;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_SENSOR,1, &para1);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_PARA_SENSOR,1, &paraget1);
    if (0 == memcmp(&para1,&paraget1, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
        printf("get AlaSensorPara:chn 1, nType:%d, nEnable:%d,nDelay=%d \n",paraget1.sAlaSensorPara.emType,
        paraget1.sAlaSensorPara.nEnable, paraget1.sAlaSensorPara.nDelay);
    }

    //设置移动侦测
    SAlarmPara para2, paraget2;
    para2.sAlaVMotionPara.emSetType = EM_ALMARM_VMOTION_AREA_SELECTALL;
    para2.sAlaVMotionPara.nEnable = 1;
    para2.sAlaVMotionPara.nSensitivity = 1;
    para2.sAlaVMotionPara.nDelay = 7;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_VMOTION, 2, &para2);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_PARA_VMOTION,2, &paraget2);
    if (0 == memcmp(&para2,&paraget2, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);////
        printf("get sAlaOutPara:chn 3, emSetType:%d, nEnable:%d,nSensitivity:%d, nDelay=%d \n",para2.sAlaVMotionPara.emSetType,
        para2.sAlaVMotionPara.nEnable, para2.sAlaVMotionPara.nSensitivity,para2.sAlaVMotionPara.nDelay );
    }

    //视频遮挡
    SAlarmPara para3, paraget3;
    para3.sAlaVBlindPara.nEnable= 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_VBLIND,2, &para3);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_PARA_VBLIND,2, &paraget3);    
    if (0 == memcmp(&para3,&paraget3, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    }     

    //设置视频丢失
    SAlarmPara para4, paraget4;
    para4.sAlaVLostPara.nEnable= 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_VLOST,3, &para4);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_PARA_VLOST,3, &paraget4);    
    if (0 == memcmp(&para4,&paraget4, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    }
   
    //报警输出设置
    SAlarmPara para5,paraget5;
    para5.sAlaOutPara.emType = EM_ALARM_SENSOR_NORMALOPEN;
    para5.sAlaOutPara.nEnable= 1;
    para5.sAlaOutPara.nDelay = 5;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_ALARMOUT,0, &para5);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_PARA_ALARMOUT,0, &paraget5);
    if (0 == memcmp(&para5,&paraget5, sizeof(SAlarmPara)))
    {
     printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
     printf("para get!=set line:%d\n", __LINE__);
     printf("get sAlaOutPara:chn 0, emType:%d, nEnable:%d,nDelay=%d \n",paraget5.sAlaOutPara.emType,
     paraget5.sAlaOutPara.nEnable, paraget5.sAlaOutPara.nDelay);
    }
    
    //设置蜂鸣器
    SAlarmPara para6,paraget6;
    para6.sAlaBuzzPara.nEnable = 1;
    para6.sAlaBuzzPara.nDelay= 0;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_PARA_BUZZ,0, &para6);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_PARA_BUZZ,0, &paraget6);    
    if (0 == memcmp(&para6,&paraget6, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 

    //报警处理
    SAlarmPara para7,paraget7;
    para7.sAlaDispatch.nFlagBuzz = 1;
    para7.sAlaDispatch.nZoomChn = 99;
    para7.sAlaDispatch.nFlagEmail= 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_DISPATCH_SENSOR,2, &para7);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_DISPATCH_SENSOR,2, &paraget7);    
    if (0 == memcmp(&para7,&paraget7, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 

    SAlarmPara para8,paraget8;
    para8.sAlaDispatch.nFlagBuzz = 1;
    para8.sAlaDispatch.nZoomChn = 99;
    para8.sAlaDispatch.nFlagEmail= 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_DISPATCH_VMOTION,2, &para8);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_DISPATCH_VMOTION,2, &paraget8);    
    if (0 == memcmp(&para8,&paraget8, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 

    SAlarmPara para9,paraget9;
    para9.sAlaDispatch.nFlagBuzz = 1;
    para9.sAlaDispatch.nZoomChn = 99;
    para9.sAlaDispatch.nFlagEmail= 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_DISPATCH_VBLIND,2, &para9);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_DISPATCH_VBLIND,2, &paraget9);
    if (0 == memcmp(&para9,&paraget9, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 

    SAlarmPara para10,paraget10;
    para10.sAlaDispatch.nFlagBuzz = 1;
    para10.sAlaDispatch.nZoomChn = 99;
    para10.sAlaDispatch.nFlagEmail= 1;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_DISPATCH_VLOST,2, &para10);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_DISPATCH_VLOST,2, &paraget10);    
    if (0 == memcmp(&para10,&paraget10, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 


    //报警输入布防
    SAlarmPara para11,paraget11;
    para11.sAlaSchedule.nSchType = EM_ALARM_SCH_WEEK_DAY;
    para11.sAlaSchedule.nSchTime[0][0].nStartTime = 1;
    para11.sAlaSchedule.nSchTime[0][0].nStopTime = 300;
    para11.sAlaSchedule.nSchTime[1][0].nStartTime = 1;
    para11.sAlaSchedule.nSchTime[1][0].nStopTime = 3000;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_SCHEDULE_SENSOR,2, &para11);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_SCHEDULE_SENSOR,2, &paraget11);    
    if (0 == memcmp(&para11,&paraget11, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 

    //移动布防
    SAlarmPara para12,paraget12;
    para12.sAlaSchedule.nSchType = EM_ALARM_SCH_WEEK_DAY;
    para11.sAlaSchedule.nSchTime[0][0].nStartTime = 2;
    para11.sAlaSchedule.nSchTime[0][0].nStopTime = 4000;
    para11.sAlaSchedule.nSchTime[1][0].nStartTime = 1;
    para11.sAlaSchedule.nSchTime[1][0].nStopTime = 3000;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_SCHEDULE_VMOTION,2, &para12);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_SCHEDULE_VMOTION,2, &paraget12);   
    if (0 == memcmp(&para12,&paraget12, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 

    //视频遮挡布防
    SAlarmPara para13,paraget13;
    para13.sAlaSchedule.nSchType = EM_ALARM_SCH_WEEK_DAY;
    para11.sAlaSchedule.nSchTime[0][0].nStartTime = 2;
    para11.sAlaSchedule.nSchTime[0][0].nStopTime = 8000;
    para11.sAlaSchedule.nSchTime[1][0].nStartTime = 1000;
    para11.sAlaSchedule.nSchTime[1][0].nStopTime = 9000;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_SCHEDULE_VBLIND,2, &para13);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_SCHEDULE_VBLIND,2, &paraget13);    
    if (0 == memcmp(&para13,&paraget13, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 


    //视频丢失布防
    SAlarmPara para14,paraget14;
    para14.sAlaSchedule.nSchType = EM_ALARM_SCH_WEEK_DAY;
    para11.sAlaSchedule.nSchTime[0][0].nStartTime = 2;
    para11.sAlaSchedule.nSchTime[0][0].nStopTime = 8000;
    para11.sAlaSchedule.nSchTime[1][0].nStartTime = 1000;
    para11.sAlaSchedule.nSchTime[1][0].nStopTime = 9000;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_SCHEDULE_VLOST,2, &para14);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_SCHEDULE_VLOST,2, &paraget14);    
    if (0 == memcmp(&para14,&paraget14, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    } 

    //报警输出布防
    SAlarmPara para15,paraget15;
    para15.sAlaSchedule.nSchType = EM_ALARM_SCH_WEEK_DAY;
    para11.sAlaSchedule.nSchTime[0][0].nStartTime = 2;
    para11.sAlaSchedule.nSchTime[0][0].nStopTime = 8000;
    para11.sAlaSchedule.nSchTime[1][0].nStartTime = 1000;
    para11.sAlaSchedule.nSchTime[1][0].nStopTime = 9000;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_SCHEDULE_ALARMOUT,0, &para15);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_SCHEDULE_ALARMOUT,0, &paraget15);    
    if (0 == memcmp(&para15,&paraget15, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);////////
    }

    //蜂鸣器布防
    SAlarmPara para16,paraget16;
    para16.sAlaSchedule.nSchType = EM_ALARM_SCH_WEEK_DAY;
    para11.sAlaSchedule.nSchTime[0][0].nStartTime = 2;
    para11.sAlaSchedule.nSchTime[0][0].nStopTime = 8000;
    para11.sAlaSchedule.nSchTime[1][0].nStartTime = 1000;
    para11.sAlaSchedule.nSchTime[1][0].nStopTime = 9000;
    ModAlarmSetParam(tstAlaHdr, EM_ALARM_SCHEDULE_BUZZ,0, &para16);
    ModAlarmGetParam(tstAlaHdr, EM_ALARM_SCHEDULE_BUZZ,0, &paraget16);    
    if (0 == memcmp(&para16,&paraget16, sizeof(SAlarmPara)))
    {
        printf("para get==set line:%d\n", __LINE__);
    }
    else
    {
        printf("para get!=set line:%d\n", __LINE__);
    }
    #endif

    while (1)
    {
        sleep(1);
    }

    return 0;
}

