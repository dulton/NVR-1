

#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "encode_manager.h"
#include "mkp_vd.h"
#include "hifb.h"
#include "lib_common.h"
#include "lib_preview.h"
#include "lib_misc.h"

typedef struct 
{
	u8 byChn;
	EMCHNTYPE emChnType;
} SChnTy;

typedef enum hiVOU_DEV_E
{
	HD = 0,
	AD = 1,
	SD = 2,
	VOU_DEV_BUTT,
}VOU_DEV_E;

typedef enum hiVOU_GLAYER_E
{
	G0 = 0,
	G1 = 1,
	G2 = 2,
	G3 = 3,
	HC = 4,
	VOU_GLAYER_BUTT,
}VOU_GLAYER_E;

void StreamDeal(SEncodeHead* psEncodeHead, u32 nContent)
{
	static int count = 0, fd = -1;
	static int fd1 = -1;
	
	if (psEncodeHead && nContent > 0)
	{
		SChnTy* psChn;

		psChn = (SChnTy *) nContent;
		if (0 == count)
		{
			fd = open("/root/video.h264", O_RDWR | O_CREAT, 777);
			//fd1 = open("/root/video1.h264", O_RDWR | O_CREAT, 777);;
		}
		
		if (fd > 0)
		{
			if (count <= 500)
			{
			
				write(fd, psEncodeHead->pData, psEncodeHead->nLen);
				
				count++;
			}
			else
			{		
				close(fd);
				//close(fd1);
				fd = -1;
				//fd1 = -1;
			}
		}

		//printf("psEncodeHead->pData = %s\n", psEncodeHead->pData);
		//printf("psEncodeHead->dwLen = %d\n", psEncodeHead->nLen);
	}
}

void AudioDeal(SEncodeHead* psEncodeHead, u32 nContent)
{
	static int count = 0, fd = -1;
	
	if (psEncodeHead && nContent > 0)
	{
		SChnTy* psChn;

		psChn = (SChnTy *) nContent;
		if (0 == count)
		{
			fd = open("/root/audio.h264", O_RDWR | O_CREAT, 777);
		}
		
		if (fd > 0)
		{
			if (count <= 500)
			{
				write(fd, psEncodeHead->pData, psEncodeHead->nLen);
				count++;
			}
			else
			{		
				close(fd);
				fd = -1;
			}
		}

		//printf("psAudioHead->dwLen = %d\n", psEncodeHead->nLen);
	}
}


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
			//printf("get fb show 1 success\n");
		}
		
		if(bShow)
		{
			if(ioctl(fd, FBIOPUT_SHOW_HIFB, &bShow) == -1)
			{
				printf("Error reading variable information.\n");
				close(fd);
				return FALSE;
			}
			else
			{
				//printf("set fb show 1 success\n");
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


#if 1
int initStateOSD()
{
		
		HIFB_ALPHA_S stAlpha;
		struct fb_var_screeninfo varInfo;
		int size;
		u16* pmap;
		int fd;

	#if 1
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
		#endif
	
		

		fd = open("/dev/fb4", O_RDWR);

		setOsdTransparency(fd ,0xff);
	
		//printf();
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
	
		size = 800 * 600 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
		//pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
	
		
		//memset(pmap ,0x00,size);
	
		
		close(fd);
	
		
		fd = open("/dev/fb1", O_RDWR);
			//printf();
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
	
		size = 720 * 576 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
		pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
		if (pmap == MAP_FAILED)
		{
		
			close(fd);
			return FALSE;
		}
		
		memset(pmap ,0x00,size);
	
		close(fd);

		
	
		fd = open("/dev/fb0", O_RDWR);
	
		//printf();
		if(fd == -1)
		{
			printf("Failed to open fb device\n");
			return FALSE;
		}
		
		//HIFB_ALPHA_S stAlpha;
		//struct fb_var_screeninfo varInfo;
	
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
	
		size = 800 * 600 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
		pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
		if (pmap == MAP_FAILED)
		{
		
			close(fd);
			return FALSE;
		}
		
		memset(pmap ,0x00,size);
	
		fd = open("/dev/fb2", O_RDWR);
			//printf();
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
	
		size = 720 * 576 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
		pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
		if (pmap == MAP_FAILED)
		{
		
			close(fd);
			return FALSE;
		}
		
		memset(pmap ,0x00,size);
	
		fd = open("/dev/fb0", O_RDWR);
	
		//printf();
		if(fd == -1)
		{
			printf("Failed to open fb device\n");
			return FALSE;
		}
		
		//HIFB_ALPHA_S stAlpha;
		//struct fb_var_screeninfo varInfo;
	
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
	
		size = 800 * 600 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
		pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
		if (pmap == MAP_FAILED)
		{
		
			close(fd);
			return FALSE;
		}
		
		memset(pmap ,0x00,size);
	
	
		
		fd = open("/dev/fb1", O_RDWR);
			//printf();
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
	
		size = 720 * 576 * 2;//varInfo.xres * varInfo.yres * varInfo.bits_per_pixel / 8;
		pmap = (u16 *) mmap(NULL, size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
		if (pmap == MAP_FAILED)
		{
		
			close(fd);
			return FALSE;
		}
		
		memset(pmap ,0x00,size);
}
#endif

s32 TextToOsd(PARAOUT u8* pBuffer, s32 w, s32 h, s8* strOsd, EMENCODEFONTSIZE emSize)
{
	u32 i = 0;
	for(i=0; i<w*h*2; i++)
	{
		pBuffer[i] = 0xf0;
	}
}

#include <semaphore.h>
static sem_t g_sem;

#if 0//csp modify
//static u64 GetTimeStamp()
//{
//	struct timeval tv;
//	gettimeofday(&tv, NULL);
//	
//	return (((u64)(tv.tv_sec))*1000000) + (u64)tv.tv_usec;
//}
#endif

void ReadFrame()
{
	venc_stream_s venc_stream;
	char pEncBuf[256<<10] = {0};
	int ret = 0;
	
	while(1)
	{
		//printf("1###%llu\n", GetTimeStamp());
		sem_wait(&g_sem);
		//printf("2###%llu\n", GetTimeStamp());
		ret = tl_venc_read(pEncBuf, 256<<10, &venc_stream);
		if(ret > 0)
		{
			printf("chn[%d] frame\n", venc_stream.chn);
		}
		else
		{
			printf("Read No Frame\n");
		}
		//printf("3###%llu\n", GetTimeStamp());
		sem_post(&g_sem);
	}
}

int main()
{
	int init_param = 0;
	init_param |= TL_NTSC;
	init_param |= TL_VGA;
	tl_hslib_init_c(TL_BOARD_TYPE_R9616S, init_param);

	int ret = tl_venc_open();
	if(ret != 0)
	{
		printf("tl_venc_open failed ! ret = %d\n", ret);
	}

	venc_parameter_t para;
	memset(&para, 0, sizeof(venc_parameter_t));
	para.bit_rate = 2048;
	para.frame_rate = 25;
	para.gop = 40;
	para.is_cbr = 1;
	para.pic_level = 5;
	
	u8 i = 0;
	for(i = 0; i < 8; i++)
	{
		ret |= tl_venc_start(i, VENC_SIZE_D1, &para);
	}
	if(ret != 0)
	{
		printf("tl_venc_start failed ! ret = %d\n", ret);
	}

	sem_init(&g_sem, 0, 1);

	pthread_t pid = 0;
	pthread_create(&pid, NULL, ReadFrame, NULL);
	
	venc_size_e size[2] = {VENC_SIZE_CIF, VENC_SIZE_D1};
	
	while(1)
	{
		sem_wait(&g_sem);
		
		ret |= tl_venc_stop(0);
		ret |= tl_venc_stop(1);
		if(ret != 0)
		{
			printf("tl_venc_stop failed ! ret = %d\n", ret);
			ret = 0;
		}
		//usleep(100*1000);
		
		i = i?0:1;
		ret |= tl_venc_start(0, size[i?0:1], &para);
		ret |= tl_venc_start(1, size[i?1:0], &para);
		if(ret != 0)
		{
			printf("tl_venc_start failed ! ret = %d\n", ret);
			ret = 0;
		}
		sem_post(&g_sem);
		
		usleep(200*1000);
	}

	return 0;
}


int main1()
{
	initStateOSD();
	
	u32 nChn = 1;
//	EMCHNTYPE emChnType = EM_CHN_VIDEO_MAIN;
	EMCHNTYPE emChnType = EM_CHN_VIDEO_SUB;
//	EMCHNTYPE emChnType = EM_CHN_AUDIO;

	SEncodeCapbility* psEncodeCap = malloc(sizeof(SEncodeCapbility));
	printf("malloc SEncodeCapbility successful!!\n");
		
	SEncodeParam * psEncodeParam = malloc(sizeof(SEncodeParam));
	printf("malloc psEncodeParam successful!!\n");


	SChnTy sChn;
	
	psEncodeCap->nMainNum = 32;
    psEncodeCap->nMainMaxFrameSize = 80*1024*8;
	psEncodeCap->nSubNum = 32;
    psEncodeCap->nSubMaxFrameSize = 80*1024*8;
	psEncodeCap->nAudioNum = 4;
    psEncodeCap->nAudioMaxFrameSize = 4*480*2;
	psEncodeCap->nSnapNum = 0;
    psEncodeCap->nSnapMaxFrameSize = 4*480*4;

	#ifdef HI3515_48
	int init_param = 0;
	init_param |= TL_128MDDR;
	init_param |= TL_PAL;
	init_param |= TL_VGA;
	tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);
	#else
	tl_hslib_init(TL_BOARD_TYPE_R9624T);
	//tl_hslib_init(TL_BOARD_TYPE_R9632SL);
	#endif
	
	printf("start init encode mod\n");
	
	ModEncodeInit(psEncodeCap);
	
	//ModEncodeOsdResolRef(0, EM_VIDEO_D1);
	SRect sRect;
	sRect.x = 32;
	sRect.y = 16;
	sRect.w = 280;
	sRect.h = 20;

	//SRgb sRgb;
	//ModEncodeOsdInit(1, EM_ENCODE_OSD_TIME, &sRect, &sRgb);
	//ModEncodeOsdResolRef(1, EM_VIDEO_D1);
		
	//ModEncodeOsdRegistCB(TextToOsd);
	//ModEncodeOsdSet(0, EM_ENCODE_OSD_TIME, &sRect, NULL, 0, NULL );
	//ModEncodeOsdSet(1, EM_ENCODE_OSD_TIME, &sRect, 0, "" );

	//ModEncodeOsdShow(0, EM_ENCODE_OSD_TIME, 1);
	//ModEncodeOsdShow(1, EM_ENCODE_OSD_TIME, 1);

//	sChn.emChnType = EM_CHN_VIDEO_MAIN;
	sChn.emChnType = EM_CHN_VIDEO_SUB;

	sleep (1);
	
	psEncodeParam->sVideoMainParam.nBitRate = 512;
	psEncodeParam->sVideoMainParam.nFrameRate = 25;
	psEncodeParam->sVideoMainParam.nGop = 25;

	int i = 0;
	for(i = 0; i < 24; i++)
	{
		sChn.byChn = i;

		ModEncodeSetParam(i, emChnType, psEncodeParam);
		
		ModEncodeGetParam(i, emChnType, psEncodeParam);

		printf("psEncodeParam->sVideoMainParam.nBitRate = %d\n", psEncodeParam->sVideoMainParam.nBitRate);
		printf("psEncodeParam->sVideoMainParam.nGop = %d\n", psEncodeParam->sVideoMainParam.nGop);

		free(psEncodeParam);
		free(psEncodeParam);
		
		//ModEncodeDeinit();
		ModEncodeRequestStart(i, EM_CHN_VIDEO_SUB, StreamDeal, &sChn);
	}
	
	//sChn.emChnType = EM_CHN_AUDIO;
	//ModEncodeRequestStart(nChn, EM_CHN_AUDIO, AudioDeal, &sChn);

	sleep(5);

	ModEncodeRequestStop(nChn,EM_CHN_AUDIO);

	while(1)
	{
		sleep (1);
	}
	
	return 0;
}

