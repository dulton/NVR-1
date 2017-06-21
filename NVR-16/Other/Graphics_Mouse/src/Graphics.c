//#include "DVRDEF.H"
//#include "DVRINC.H"
#include "DVRINC.H"
#include "DVRDEF.H"

#include "Graphics.H"
#include "APIs/Graphics.H"

#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <errno.h>

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
#else
#include "mkp_vd.h"
#endif

#include "hifb.h"

#define PUBPRT(msg) //printf("file:%s fn:%s line:%d\r\n%s\r\n", __FILE__, __FUNCTION__, __LINE__, msg)

GRAPHIC_DEV g_GraphicDev;

#define OSD_WIDTH_VGA (g_GraphicDev.nWidth)
#define OSD_HEIGHT_VGA (g_GraphicDev.nHeight)

#define OSD_WIDTH_CVBS 720
#define OSD_HEIGHT_CVBS ((VIDEO_STANDARD_PAL == g_GraphicDev.Vstd) ? 576 : 480)

static emOutputType emGraphOut = EM_GRAPH_OUTPUT_VGA;
static emChipType g_emChip = EM_GRAPH_CHIP_HISI3515;

static char* g_mmapBuf = NULL;
static char* g_tmpMapBuf = NULL;

#define ONE_BUF

//csp modify
//#define VGA_OUT
//#define SINGLE_OUT
#define ALL_OUT

#if defined(CHIP_HISI3531)
typedef enum hiVOU_DEV_E
{
#if 1//csp modify 20130328
	HD0 = 0,
	HD1 = 1,
	SD0 = 2,
	VOU_DEV_BUTT,
#else
	HD0 = 1,
	HD1 = 0,
	SD0 = 2,
	VOU_DEV_BUTT,
#endif
} VOU_DEV_E;
typedef enum hiVOU_GLAYER_E
{
	G4 = 0,
	G5 = 1,
	G6 = 2,
	VOU_GLAYER_BUTT,
}VOU_GLAYER_E;
#elif defined(CHIP_HISI3521)
typedef enum hiVOU_DEV_E
{
	HD0 = 0,
	SD0 = 1,
	VOU_DEV_BUTT,
} VOU_DEV_E;
typedef enum hiVOU_GLAYER_E
{
	G4 = 0,
	G5 = 1,
	G6 = 2,
	VOU_GLAYER_BUTT,
}VOU_GLAYER_E;
#else
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
#endif

int GraphicsSetOutput(emOutputType emOut)
{
	emGraphOut = emOut;
	return 0;
}

emOutputType GraphicsGetOutput(void)
{
	return emGraphOut;
}

int GraphicsSetChipType(emChipType emChip)
{
	g_emChip = emChip;
	return 0;
}

emChipType GraphicsGetChipType(void)
{
	return g_emChip;
}

int GraphicsCreate()
{
	g_GraphicDev.nHeight = 600;
	g_GraphicDev.nWidth = 800;
	
#if defined(CHIP_HISI3531)
	if(g_emChip == EM_GRAPH_CHIP_HISI3531)
	{
		VOU_GFX_BIND_LAYER_E nLayer = G4;
		VO_DEV nVoDev = HD1;
		if(EM_GRAPH_OUTPUT_CVBS == emGraphOut)
		{
			//nVoDev = SD0;//csp modify
		}
		
		printf("######Hi3531 GraphicsCreate nVoDev=%d######\n",nVoDev);
		
		#if 1
		VOU_GFX_BIND_LAYER_E enGfxBindLayer;
		for(enGfxBindLayer = G4; enGfxBindLayer < VOU_GLAYER_BUTT; ++enGfxBindLayer)
		{
			if(0 != HI_MPI_VO_GfxLayerUnBindDev(enGfxBindLayer, 0))
			{
				printf("fb unbind [%d, %d] failed!\n", enGfxBindLayer, 0);
				return -1;
			}
			
			#if 0
			if(0 != HI_MPI_VO_GfxLayerUnBindDev(enGfxBindLayer, 1))
			{
				printf("fb unbind [%d, %d] failed!\n", enGfxBindLayer, 1);
				return -1;
			}
			
			if(0 != HI_MPI_VO_GfxLayerUnBindDev(enGfxBindLayer, SD0))
			{
				printf("fb unbind [%d, %d] failed!\n", enGfxBindLayer, SD0);
				return -1;
			}
			else
			{
				//printf("fb unbind [%d, %d] success!\n", enGfxBindLayer, SD0);
			}
			#endif
		}
		#else
		if(0 != HI_MPI_VO_GfxLayerUnBindDev(nLayer, 0))
		{
			printf("fb unbind [%d, %d] failed!\n", nLayer, nVoDev);
			return -1;
		}
		#endif
		
		if(0 != HI_MPI_VO_GfxLayerBindDev(nLayer, nVoDev))
		{
			printf("fb bind [%d, %d] failed!\n", nLayer, nVoDev);
			return -1;
		}
		
		if(HI_API_TDE_Open() != HI_SUCCESS)
		{
			printf("HI_API_TDE_Open failed! line:%d\n",__LINE__);
			return -1;
		}
		
		g_GraphicDev.Fd = open(FB_DEV, O_RDWR);//frame buffer设备主要用于显示2D图形
		if(g_GraphicDev.Fd < 0)
		{
			perror("open FB_DEV");
			return -1;
		}
		
		#ifndef SINGLE_OUT
		/*g_GraphicDev.Fd_cvbs = open(FB_DEV2, O_RDWR);//cvbs
		if(g_GraphicDev.Fd_cvbs < 0)
		{
			perror("open FB_DEV2");
			return -1;
		}*/
		#endif
		
		g_GraphicDev.Fd_cvbs = -1;
		
		//pthread_mutex_init(&(g_GraphicDev.TdeLock), NULL);
		
		return 0;
	}
#elif defined(CHIP_HISI3521)
	if(g_emChip == EM_GRAPH_CHIP_HISI3521)//"/dev/fb0",G0,HD0
	{
		printf("######Hi3521 GraphicsCreate nVoDev=%d######\n",HD0);
		
		if(HI_API_TDE_Open() != HI_SUCCESS)
		{
			printf("HI_API_TDE_Open failed! line:%d\n",__LINE__);
			return -1;
		}
		
		//printf("HI_API_TDE_Open success! line:%d\n",__LINE__);
		//fflush(stdout);

		//printf("\n\n\n\n");
		//sleep(10);
		
		g_GraphicDev.Fd = open("/dev/fb0", O_RDWR);
		if(g_GraphicDev.Fd < 0)
		{
			perror("open FB_DEV");
			return -1;
		}
		
		g_GraphicDev.Fd_cvbs = -1;

		//printf("open /dev/fb0 success! line:%d\n",__LINE__);
		//fflush(stdout);
		
		//printf("\n\n\n\n");
		//sleep(10);
		
		return 0;
	}
#else
	if(g_emChip == EM_GRAPH_CHIP_HISI3515)
	{
		HI_S32 s32Ret;		
		VD_BIND_S stBind;
		int fd;
		
		printf("######Hi3515 GraphicsCreate nVoDev=%d######\n",HD);
		
		/*---------1 bind graphic layer G4(here used to show cursor) to HD, defaultly----------*/
		
		/*1.1 open vd fd*/
		fd = open("/dev/vd", O_RDWR, 0);
		if(fd < 0)
		{
			printf("open vd failed!\n");
			
			return -1;
		}
		
		stBind.s32GraphicId = HC;
		
		#ifndef SINGLE_OUT
		stBind.DevId = HD;
		#else
		if (EM_GRAPH_OUTPUT_VGA == emGraphOut)
		{
			stBind.DevId = HD;
		}
		else
		{
			stBind.DevId = SD;
		}
		#endif
		
		if (ioctl(fd, VD_SET_GRAPHIC_BIND, &stBind) != HI_SUCCESS)
		{
			printf("[fd:%d]set bind glayer %d to dev %d failed!\n", fd, HC, stBind.DevId);
			
			close(fd);
			
			return -1;
		}
		
		#if 0//ndef SINGLE_OUT
		/*1.3 bind G1 to SD*/
		stBind.s32GraphicId = G1;
		stBind.DevId = SD;
		if (ioctl(fd, VD_SET_GRAPHIC_BIND, &stBind) != HI_SUCCESS)
		{
			printf("[fd:%d]set bind glayer %d to dev %d failed!\n",fd,G1,SD);
			
			close(fd);
			
			return -1;
		}
		#endif
		
		s32Ret = HI_API_TDE_Open();
		if(s32Ret!=HI_SUCCESS)
		{
			return 0;
		}
		
		g_GraphicDev.Fd = open(FB_DEV, O_RDWR);//frame buffer设备主要用于显示2D图形
		if(g_GraphicDev.Fd < 0)
		{
			perror("open FB_DEV");
			return -1;
		}
		
		#ifndef SINGLE_OUT
		g_GraphicDev.Fd_cvbs = open(FB_DEV2, O_RDWR);//cvbs
		if(g_GraphicDev.Fd_cvbs < 0)
		{
			perror("open FB_DEV2");
			return -1;
		}
		#endif
		
		//pthread_mutex_init(&(g_GraphicDev.TdeLock), NULL);
		
		printf("######Hi3515 GraphicsCreate finish######\n");
		
		return 0;
	}
	else if(g_emChip == EM_GRAPH_CHIP_HISI3520)
	{
		HI_S32 s32Ret;
		VD_BIND_S stBind;
		int fd;
		fd = open("/dev/vd", O_RDWR, 0);
		if(fd < 0)
		{
			printf("open vd failed!\n");

			return -1;
		}

		if(EM_GRAPH_OUTPUT_VGA == emGraphOut)
		{
			stBind.s32GraphicId = HC;
			stBind.DevId = HD;
			if (ioctl(fd, VD_SET_GRAPHIC_BIND, &stBind) != HI_SUCCESS)
			{
				printf("[fd:%d]set bind glayer %d to dev %d failed!\n", fd, HC, stBind.DevId);
				close(fd);
				return -1;
			}		
		}
		
		s32Ret = HI_API_TDE_Open();
		if(s32Ret!=HI_SUCCESS)
		{
			return 0;
		}
		
		if(EM_GRAPH_OUTPUT_VGA == emGraphOut)
		{
			g_GraphicDev.Fd = open(FB_DEV, O_RDWR);//frame buffer设备主要用于显示2D图形
		}
		else
		{
			g_GraphicDev.Fd = open(FB_DEV3, O_RDWR);//frame buffer设备主要用于显示2D图形
		}
		if(g_GraphicDev.Fd < 0)
		{
			perror("open FB_DEV");
			return -1;
		}
		
		//pthread_mutex_init(&(g_GraphicDev.TdeLock), NULL);
		
		return 0;
	}
#endif
	else
	{
		return -1;
	}
}

int GraphicsDestory()
{
	close(g_GraphicDev.Fd);
	g_GraphicDev.Fd = -1;
	
	#ifndef SINGLE_OUT
	if(g_GraphicDev.Fd_cvbs != -1)
	{
		close(g_GraphicDev.Fd_cvbs);
		g_GraphicDev.Fd_cvbs = -1;
	}
	#endif
	
	return 0;
}

int GraphicsCreate2()
{
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	return 0;
#endif
	
#ifdef ALL_OUT
	return -1;
#else
	g_GraphicDev.nHeight = 600;
	g_GraphicDev.nWidth = 800;
	
	#ifdef CHIP_HISI3531
	if(g_emChip == EM_GRAPH_CHIP_HISI3531)
	{
		VOU_GFX_BIND_LAYER_E nLayer = G4;
		VO_DEV nVoDev = HD1;
		
		if(0 != HI_MPI_VO_GfxLayerUnBindDev(nLayer, 0))
		{
			printf("fb unbind [%d, %d] failed!\n", nLayer, nVoDev);
			return -1;
		}
		
		if(0 != HI_MPI_VO_GfxLayerBindDev(nLayer, nVoDev))
		{
			printf("fb bind [%d, %d] failed!\n", nLayer, nVoDev);
			return -1;
		}
		
		g_GraphicDev.Fd = open(FB_DEV, O_RDWR);//frame buffer设备主要用于显示2D图形
		if (g_GraphicDev.Fd < 0)
		{
			perror("open FB_DEV");
			return -1;
		}
		
		g_GraphicDev.Fd_cvbs = open(FB_DEV2, O_RDWR);//cvbs
		if (g_GraphicDev.Fd_cvbs< 0)
		{
			perror("open FB_DEV2");
			return -1;
		}
		
		if(HI_API_TDE_Open() != HI_SUCCESS)
		{
			printf("HI_API_TDE_Open failed! line:%d\n",__LINE__);
			return -1;
		}
		
		//pthread_mutex_init(&(g_GraphicDev.TdeLock), NULL);
		
		return 0;
	}
	#else
	if(g_emChip == EM_GRAPH_CHIP_HISI3515)
	{
		HI_S32 s32Ret;		
		VD_BIND_S stBind;
		int fd;
		
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
			printf("[fd:%d]set bind glayer %d to dev %d failed!\n", fd, HC, stBind.DevId);
			
			close(fd);
			
			return -1;
		}
		
		#if 0
		stBind.s32GraphicId = G1;
		stBind.DevId = SD;
		if (ioctl(fd, VD_SET_GRAPHIC_BIND, &stBind) != HI_SUCCESS)
		{
			printf("[fd:%d]set bind glayer %d to dev %d failed! line:%d\n",fd,G1,SD);
			
			close(fd);
			
			return -1;
		}
		#endif
		
		s32Ret = HI_API_TDE_Open();
		if(s32Ret!=HI_SUCCESS)
		{
			return 0;
		}
		
		g_GraphicDev.Fd = open(FB_DEV, O_RDWR);//frame buffer设备主要用于显示2D图形
		if (g_GraphicDev.Fd < 0)
		{
			perror("open FB_DEV");
			return -1;
		}
		
		g_GraphicDev.Fd_cvbs = open(FB_DEV2, O_RDWR);//cvbs
		if (g_GraphicDev.Fd_cvbs< 0)
		{
			perror("open FB_DEV2");
			return -1;
		}
		
		//pthread_mutex_init(&(g_GraphicDev.TdeLock), NULL);
		
		return 0;
	}
	else if(g_emChip == EM_GRAPH_CHIP_HISI3520)
	{
		HI_S32 s32Ret;
		VD_BIND_S stBind;
		int fd;
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
			printf("[fd:%d]set bind glayer %d to dev %d failed!\n", fd, HC, stBind.DevId);
			close(fd);
			return -1;
		}
		
		s32Ret = HI_API_TDE_Open();
		if(s32Ret!=HI_SUCCESS)
		{
			return 0;
		}
		
		g_GraphicDev.Fd = open(FB_DEV, O_RDWR);//frame buffer设备主要用于显示2D图形
		if (g_GraphicDev.Fd < 0)
		{
			perror("open FB_DEV");
			return -1;
		}				
		
		g_GraphicDev.Fd_cvbs = open(FB_DEV3, O_RDWR);//frame buffer设备主要用于显示2D图形
		if (g_GraphicDev.Fd < 0)
		{
			perror("open FB_DEV");
			return -1;
		}
		
		//pthread_mutex_init(&(g_GraphicDev.TdeLock), NULL);
		printf("%s OK!!!!\n", __FUNCTION__);
		
		return 0;
	}
	#endif
	else
	{
		return -1;
	}
#endif
}

#ifndef ALL_OUT
#if 0
static unsigned char g_Graphics2_show = 0;
#else
static unsigned char g_Graphics2_show = 1;
#endif
#endif

int GraphicsDestory2()
{
#ifndef ALL_OUT
	g_Graphics2_show = 0;
	
	usleep(100*1000);
	
	close(g_GraphicDev.Fd);
	g_GraphicDev.Fd = -1;
	
	if(g_GraphicDev.Fd_cvbs != -1)
	{
		close(g_GraphicDev.Fd_cvbs);
		g_GraphicDev.Fd_cvbs = -1;
	}
#endif
	
	return 0;
}

int GraphicsSetVstd(DWORD dwStandard)
{
	g_GraphicDev.Vstd = dwStandard;
	
	return 0;
}

int GraphicsSetAlpha(int alpha, int delta)
{
	HIFB_ALPHA_S stAlpha;
	
	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = alpha;
	stAlpha.u8Alpha1 = 0;
	
	if(ioctl(g_GraphicDev.Fd, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
	{
		perror("FBIOPUT_ALPHA_HIFB-1");
		return -1;
	}
	
	if(g_emChip == EM_GRAPH_CHIP_HISI3515 || g_emChip == EM_GRAPH_CHIP_HISI3520)
	{
		//#ifndef SINGLE_OUT
		stAlpha.bAlphaEnable = HI_TRUE;
		stAlpha.bAlphaChannel = HI_FALSE;
		stAlpha.u8Alpha0 = alpha;
		stAlpha.u8Alpha1 = 0;
		if(ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
		{
			perror("FBIOPUT_ALPHA_HIFB-2");
			return -1;
		}
		//#endif
	}
	
	printf("func:%s, line:%d, alpha:%d\n", __FUNCTION__, __LINE__, alpha);
	
	return 0;
}

int GraphicsSetColorKey(DWORD dwKeyLow, DWORD dwKeyHigh)
{
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//3531/3521只有鼠标层支持FBIOPUT_COLORKEY_HIFB
#else
	if(g_emChip == EM_GRAPH_CHIP_HISI3515 || g_emChip == EM_GRAPH_CHIP_HISI3520)
	{
		HIFB_COLORKEY_S fbck;
		memset(&fbck, 0, sizeof(HIFB_COLORKEY_S));
		
		fbck.bKeyEnable = 1;
		fbck.u32Key = dwKeyLow;
		fbck.bMaskEnable = 1;
		fbck.u8RedMask = ((dwKeyHigh - dwKeyLow) & 0xff0000) >> 16;	
		fbck.u8GreenMask = ((dwKeyHigh - dwKeyLow) & 0x00ff00) >> 8;	
		fbck.u8BlueMask = ((dwKeyHigh - dwKeyLow) & 0x0000ff);				
		if(ioctl(g_GraphicDev.Fd, FBIOPUT_COLORKEY_HIFB, &fbck) <0)
		{
			perror("FBIOPUT_COLORKEY_HIFB-1");
			return -1;
		}
		
		#ifndef SINGLE_OUT
		if(ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_COLORKEY_HIFB, &fbck) <0)
		{
			perror("FBIOPUT_COLORKEY_HIFB-2");
			return -1;
		}
		#endif
	}
#endif
	
	return 0;
}

typedef int Int32;
typedef unsigned char Uint8;
typedef int UINT;
//typedef unsigned char BYTE;

Uint8 Avg(Uint8 a, Uint8 b)
{
	return ((a+b+1)>>1);
}

void ColorChFilter3 (UINT w, UINT h, BYTE *chbits) //zlb20111116
{
	BYTE  *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12, *p13;
	BYTE  *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12, *s13;
	BYTE  in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13;
	UINT  ix, iy;
	BYTE psrcbits[w*h*sizeof(BYTE)]; //zlb20111116 原变量是malloc方式

	memcpy(psrcbits,chbits,w*h*sizeof(BYTE));
	int ih = 0;
	if(h>=13)
	{
		s1 = psrcbits;
		s2 = (psrcbits + w);
		s3 = (psrcbits + w*2);
		s4 = (psrcbits + w*3);
		s5 = (psrcbits + w*4);
		s6 = (psrcbits + w*5);
		s7 = (psrcbits + w*6);
		s8 = (psrcbits + w*7);
		s9 = (psrcbits + w*8);
		s10= (psrcbits + w*9);
		s11= (psrcbits + w*10);
		s12= (psrcbits + w*11);
		s13= (psrcbits + w*12);
		p1 = chbits;
		p2 = (chbits + w);
		p3 = (chbits + w*2);
		p4 = (chbits + w*3);
		p5 = (chbits + w*4);
		p6 = (chbits + w*5);
		p7 = (chbits + w*6);
		p8 = (chbits + w*7);
		p9 = (chbits + w*8);
		p10= (chbits + w*9);
		p11= (chbits + w*10);
		p12= (chbits + w*11);
		p13= (chbits + w*12);
		
		for (iy = 0; iy <= h-13; iy+=11)
		{
			for (ix = 0; ix < w; ix++)
			{
				in1 = *(s1 + ix);
				in2 = *(s2 + ix);
				in3 = *(s3 + ix);
				in4 = *(s4 + ix);
				in5 = *(s5 + ix);
				in6 = *(s6 + ix);
				in7 = *(s7 + ix);
				in8 = *(s8 + ix);
				in9 = *(s9 + ix);
				in10 = *(s10 + ix);
				in11 = *(s11 + ix);
				in12 = *(s12 + ix);
				in13 = *(s13 + ix);
				
				*(p2 + ix) = Avg(in2, Avg(in1, in3));
				*(p3 + ix) = Avg(in3, Avg(in2, in4));
				*(p4 + ix) = Avg(in4, Avg(in3, in5));
				*(p5 + ix) = Avg(in5, Avg(in4, in6));
				*(p6 + ix) = Avg(in6, Avg(in5, in7));
				*(p7 + ix) = Avg(in7, Avg(in6, in8));
				*(p8 + ix) = Avg(in8, Avg(in7, in9));
				*(p9 + ix) = Avg(in9, Avg(in8, in10));
				*(p10 + ix) = Avg(in10, Avg(in9, in11));
				*(p11 + ix) = Avg(in11, Avg(in10, in12));
				*(p12 + ix) = Avg(in12, Avg(in11, in13));
			}
			s1 += (w*11);
			s2 += (w*11);
			s3 += (w*11);
			s4 += (w*11);
			s5 += (w*11);
			s6 += (w*11);
			s7 += (w*11);
			s8 += (w*11);
			s9 += (w*11);
			s10 += (w*11);
			s11 += (w*11);
			s12 += (w*11);
			s13 += (w*11);
			p1 += (w*11);
			p2 += (w*11);
			p3 += (w*11);
			p4 += (w*11);
			p5 += (w*11);
			p6 += (w*11);
			p7 += (w*11);
			p8 += (w*11);
			p9 += (w*11);
			p10 += (w*11);
			p11 += (w*11);
			p12 += (w*11);
			p13 += (w*11);
		}
		ih = iy;
	}
	if(ih<h-2)
	{
		for (iy = ih; iy < h-1; iy++)
		{
			if((ih)&&(iy==ih))
				continue;
			if(iy == 0) continue;
			s1 = psrcbits+w*(iy-1);
			s2 = (psrcbits + w*iy);
			s3 = (psrcbits + w*(iy+1));
			p1 = chbits+w*(iy-1);
			p2 = (chbits + w*iy);
			p3 = (chbits + w*(iy+1));
			for (ix = 0; ix < w; ix++)
			{
				in1 = *(s1 + ix);
				in2 = *(s2 + ix);
				in3 = *(s3 + ix);
				*(p2 + ix) = Avg(in2, Avg(in1, in3));
			}
			s1 += (w*3);
			s2 += (w*3);
			s3 += (w*3);
			p1 += (w*3);
			p2 += (w*3);
			p3 += (w*3);
		}
	}
}
	
Int32 VertFilter (Int32 w, Int32 h,Int32 stride, Uint8 *bits)  //zlb20111116
{
	#if 1
	Uint8  rtmp1, gtmp1, btmp1, rtmp2, gtmp2, btmp2, rtmp3, gtmp3, btmp3, ravg, gavg, bavg;
	unsigned short pixtmp;
	Int32 i, j;
	
	for (i = 0; i < w; i++)
	{
		pixtmp = *((unsigned short *)(bits + 2*i));
		btmp1 = (pixtmp) & 0x1f;
		gtmp1 = (pixtmp>>5) & 0x1f;
		rtmp1 = (pixtmp>>10) & 0x1f;	

		pixtmp = *((unsigned short *)(bits + (stride<<1) + 2*i));
		btmp2 = (pixtmp) & 0x1f;
		gtmp2 = (pixtmp>>5) & 0x1f;
		rtmp2 = (pixtmp>>10) & 0x1f;

		for (j = 1; j < h - 1; j++)
		{
			pixtmp = *((unsigned short *)(bits + (stride<<1)*(j + 1) + 2*i));
			btmp3 = (pixtmp) & 0x1f;
			gtmp3 = (pixtmp>>5) & 0x1f;
			rtmp3 = (pixtmp>>10) & 0x1f;

			ravg = Avg(rtmp2, Avg(rtmp1, rtmp3));
			gavg = Avg(gtmp2, Avg(gtmp1, gtmp3));
			bavg = Avg(btmp2, Avg(btmp1, btmp3));
			
			*((unsigned short *)(bits + 2*i + (stride<<1)*j)) = (bavg&0x1f)|(( gavg&0x1f)<<5)|((ravg&0x1f)<<10);

			btmp1 = btmp2;
			gtmp1 = gtmp2;
			rtmp1 = rtmp2;
			
			btmp2 = btmp3;
			gtmp2 = gtmp3;
			rtmp2 = rtmp3;
		}
	}
	#else
	Uint8  rtmp, gtmp, btmp;
	unsigned short pixtmp;
	Int32 i, j;
	//zlb20111116 原变量rin, gin, bin是malloc方式
	Uint8 rin[w*h*sizeof(Uint8)];
	Uint8 gin[w*h*sizeof(Uint8)];
	Uint8 bin[w*h*sizeof(Uint8)];

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			pixtmp = *((unsigned short *)(bits + (stride<<1)*j + 2*i));
			btmp = (pixtmp) & 0x1f;
			gtmp = (pixtmp>>5) & 0x1f;
			rtmp = (pixtmp>>10) & 0x1f;
			*(rin+i+j*w) = rtmp;
			*(gin+i+j*w) = gtmp;
			*(bin+i+j*w) = btmp;
		}
	}
	ColorChFilter3 (w, h, rin); 
	ColorChFilter3 (w, h, gin);
	ColorChFilter3 (w, h, bin);
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			*((unsigned short *)(bits + 2*i + (stride<<1)*j)) = (*(bin+i+w*j)&0x1f)|(( *(gin+i+j*w)&0x1f)<<5)|((*(rin+i+j*w)&0x1f)<<10);
		}
	}
	#endif
	
	return 0;
}

int GraphicsCvbsDefilter(SCVBSDEFILTERPARA* sFliterPara)
{
#ifndef SINGLE_OUT
	sFliterPara->mem = g_GraphicDev.GraphicMem[0].VirtAddr;
	
	if((NULL == sFliterPara) || (NULL == sFliterPara->mem))
	{
		return -1;
	}
	
	return VertFilter(sFliterPara->right - sFliterPara->left,
						sFliterPara->bottom - sFliterPara->top,
						sFliterPara->stride >> 1,
						sFliterPara->mem + sFliterPara->top * sFliterPara->stride + (sFliterPara->left << 1));
#endif
	
	return 0;
}

//#define SYNC_COPY

int GraphicsSyncVgaToCvbs(void)
{
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	return 0;
#endif
	
	//csp test
	//printf(".");
	//return 0;
	
#ifndef SINGLE_OUT	
	static int flag = 0;
	static char* memVirt = NULL;
	
	//printf("GraphicsSyncVgaToCvbs out\n");
	
	if(flag == 0)
	{
		struct fb_var_screeninfo vinfo;
		
		struct fb_bitfield stA16 = {15, 1, 0};
		struct fb_bitfield stR16 = {10, 5, 0};
		struct fb_bitfield stG16 = {5, 5, 0};
		struct fb_bitfield stB16 = {0, 5, 0};
		
		#if 1//csp modify
		printf("GraphicsSyncVgaToCvbs Fd_cvbs=%d\n",g_GraphicDev.Fd_cvbs);
		
		HIFB_ALPHA_S stAlpha;
		if(ioctl(g_GraphicDev.Fd_cvbs, FBIOGET_ALPHA_HIFB, &stAlpha) < 0)
		{
			perror("FBIOGET_ALPHA_HIFB");
			return -1;
		}
		printf("GraphicsSyncVgaToCvbs u8Alpha0=%d,u8Alpha1=%d\n",stAlpha.u8Alpha0,stAlpha.u8Alpha1);
		
		//here must set
		stAlpha.bAlphaEnable = HI_TRUE;
		stAlpha.bAlphaChannel = HI_FALSE;
		stAlpha.u8Alpha0 = 0xff;
		stAlpha.u8Alpha1 = 0;
		if(ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
		{
			perror("FBIOPUT_ALPHA_HIFB-3");
			return -1;
		}
		#endif
		
		if(ioctl(g_GraphicDev.Fd_cvbs, FBIOGET_VSCREENINFO, &vinfo) < 0)
		{
			perror("FBIOGET_VSCREENINFO");
			return -1;
		}
		
		HIFB_POINT_S stPoint = {0};
		if(ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0)
	    {
	        printf("set screen original show position failed!\n");
	        return -1;
	    }
		
		printf("GraphicsSyncVgaToCvbs FBIOGET_VSCREENINFO xres_virtual=%d,yres_virtual=%d\n",vinfo.xres_virtual,vinfo.yres_virtual);
		
		vinfo.xres_virtual = OSD_WIDTH_CVBS;
		vinfo.xres = OSD_WIDTH_CVBS;
		vinfo.yres_virtual = OSD_HEIGHT_CVBS;
		vinfo.yres = OSD_HEIGHT_CVBS;
		vinfo.activate = FB_ACTIVATE_NOW;
		vinfo.bits_per_pixel = 16;
		vinfo.xoffset = 0;
		vinfo.yoffset = 0;
		vinfo.red   = stR16;
		vinfo.green = stG16; 
		vinfo.blue  = stB16;
		vinfo.transp = stA16;
		
		if (ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_VSCREENINFO, &vinfo) < 0)
		{
			perror("FBIOPUT_VSCREENINFO");
			return -1;
		}
		
		printf("GraphicsSyncVgaToCvbs FBIOPUT_VSCREENINFO success xres_virtual=%d,yres_virtual=%d\n",vinfo.xres_virtual,vinfo.yres_virtual);
		
		vinfo.yoffset = 0;//csp modify
		//vinfo.yres_virtual = 0;//csp modify
		if (ioctl(g_GraphicDev.Fd_cvbs, FBIOPAN_DISPLAY, &vinfo) < 0)
		{
			perror("FBIOPAN_DISPLAY failed!\n");
			return -1;
		}
		
		struct fb_fix_screeninfo finfo;
		if (ioctl(g_GraphicDev.Fd_cvbs, FBIOGET_FSCREENINFO, &finfo) < 0)
		{
			perror("FBIOGET_FSCREENINFO");
			return -1;
		}
		
		memVirt = mmap(NULL, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, g_GraphicDev.Fd_cvbs, 0);
		if (memVirt == (char *)-1 || memVirt == NULL)
		{
			memVirt = NULL;
			return -1;
		}
		
		//csp modify
		memset(memVirt, 0x00, finfo.smem_len);
		printf("GraphicsSyncVgaToCvbs mmap success@@@\n");
		
		flag = 1;
	}
	
	struct fb_fix_screeninfo finfo;
	struct fb_fix_screeninfo finfo2;
	
	if (ioctl(g_GraphicDev.Fd_cvbs, FBIOGET_FSCREENINFO, &finfo) < 0)
	{
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}
	//printf(" finfo.smem_len=%d\n",  finfo.smem_len);
	
	if (ioctl(g_GraphicDev.Fd, FBIOGET_FSCREENINFO, &finfo2) < 0)
	{
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}
	//printf(" finfo2.smem_len=%d\n",  finfo2.smem_len);
	
	int s32Ret;
	TDE_HANDLE s32Handle;
	TDE2_SURFACE_S stSrc, stDst;
	TDE2_RECT_S stSrcRect, stDstRect;
	
	memset(&stSrc, 0, sizeof(TDE2_SURFACE_S));
	memset(&stDst, 0, sizeof(TDE2_SURFACE_S));
	memset(&stSrcRect, 0, sizeof(TDE2_RECT_S));
	memset(&stDstRect, 0, sizeof(TDE2_RECT_S));
	
	s32Handle = HI_TDE2_BeginJob();
	if(HI_ERR_TDE_INVALID_HANDLE == s32Handle)
	{
		printf("HI_TDE2_BeginJob failed\n");
		return -1;
	}
	
	stSrc.enColorFmt = TDE2_COLOR_FMT_ARGB1555;
	stSrc.u32PhyAddr = (unsigned int)finfo2.smem_start;
	stSrc.u32Width = OSD_WIDTH_VGA;
	stSrc.u32Height = OSD_HEIGHT_VGA;
	stSrc.u32Stride = (OSD_WIDTH_VGA << 1);
	stSrc.bAlphaExt1555 = HI_TRUE;
	stSrc.u8Alpha0 = 0xff;//0;//csp modify
	stSrc.u8Alpha1 = 0;//0xff;//csp modify
	
	#ifdef SYNC_COPY
	stSrcRect.s32Xpos = ((OSD_WIDTH_VGA - OSD_WIDTH_CVBS) >> 1);
	stSrcRect.s32Ypos =  ((OSD_HEIGHT_VGA - OSD_HEIGHT_CVBS) >> 1);
	stSrcRect.u32Width = OSD_WIDTH_CVBS;
	stSrcRect.u32Height = OSD_HEIGHT_CVBS;
	#else
	stSrcRect.s32Xpos = 0;
	stSrcRect.s32Ypos = 0;
	stSrcRect.u32Width = OSD_WIDTH_VGA;
	stSrcRect.u32Height = OSD_HEIGHT_VGA;
	#endif
	
	stDst.enColorFmt = TDE2_COLOR_FMT_ARGB1555;
	stDst.u32PhyAddr = (unsigned int)finfo.smem_start;
	stDst.u32Width = OSD_WIDTH_CVBS;
	stDst.u32Height = OSD_HEIGHT_CVBS;
	stDst.u32Stride = (OSD_WIDTH_CVBS << 1);
	stDst.bAlphaExt1555 = HI_TRUE;
	stDst.u8Alpha0 = 0xff;//0;//csp modify
	stDst.u8Alpha1 = 0;//0xff;//csp modify
	
	stDstRect.s32Xpos = 0;
	stDstRect.s32Ypos = 0;
	stDstRect.u32Width = OSD_WIDTH_CVBS;
	stDstRect.u32Height = OSD_HEIGHT_CVBS;
	
#ifdef SYNC_COPY
	s32Ret = HI_TDE2_QuickCopy(s32Handle, &stSrc, &stSrcRect, &stDst, &stDstRect);
	if(HI_SUCCESS != s32Ret) 
	{
		printf("HI_TDE2_QuickCopy command failed s32Ret=0x%08x!\n", s32Ret);
	}
	else
	{
		printf("HI_TDE2_QuickCopy command ok s32Ret=0x%08x!\n", s32Ret);
	}
#else
	#if 1//csp modify
	s32Ret = HI_TDE2_QuickDeflicker(s32Handle, &stSrc, &stSrcRect, &stDst, &stDstRect);
	#else
	s32Ret = HI_TDE2_QuickResize(s32Handle, &stSrc, &stSrcRect, &stDst, &stDstRect);
	#endif
	if(HI_SUCCESS != s32Ret)
	{
		printf("HI_TDE2_QuickResize command failed s32Ret=0x%08x!\n", s32Ret);
	}
	else
	{
		#if 1//csp modify
		//printf("HI_TDE2_QuickDeflicker command success\n");
		#else
		//printf("HI_TDE2_QuickResize command success\n");
		#endif
	}
#endif
	
	#if 0//csp modify
	HI_TDE2_WaitForDone(s32Handle);
	//HI_TDE2_EndJob(s32Handle, HI_FALSE, 0, 0);
	HI_TDE2_EndJob(s32Handle, HI_TRUE, HI_TRUE, 100);
	#else
	HI_TDE2_EndJob(s32Handle, HI_FALSE, 0, 0);
	HI_TDE2_WaitForDone(s32Handle);
	#endif
#endif
	
	return 0;
}

#ifndef SINGLE_OUT
void SyncVgaToCvbs(void)
{
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	return;
#endif
	
	while(1)
	{
		if(0 != GraphicsSyncVgaToCvbs())
		{
			printf("error: GraphicsSyncVgaToCvbs\n");
		}
		usleep(5000 * 1000);
	}
}
#endif

static struct fb_var_screeninfo g_vinfo;

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
static HIFB_BUFFER_S g_stCanvasBuf;
static HIFB_BUFFER_S g_stCanvasBuf2;
#ifndef ALL_OUT
static HIFB_BUFFER_S g_stCanvasBuf3;
#endif

void* RefreshFB(void* param)
{
	g_stCanvasBuf.UpdateRect.x = 0;
	g_stCanvasBuf.UpdateRect.y = 0;
	g_stCanvasBuf.UpdateRect.w = g_stCanvasBuf.stCanvas.u32Width;
	g_stCanvasBuf.UpdateRect.h = g_stCanvasBuf.stCanvas.u32Height;
	
	//return NULL;
	
	printf("$$$$$$$$$$$$$$$$$$RefreshFB id:%d\n",getpid());
	
	while(1)
	{
		//printf("$$$$$$$$$$$$$$$$$$FBIO_REFRESH-1\n");
		//fflush(stdout);
		
		if(ioctl(g_GraphicDev.Fd, FBIO_REFRESH, &g_stCanvasBuf) < 0)
		{
			printf("REFRESH failed!\n");
			return HI_NULL;
		}
		
		//printf("$$$$$$$$$$$$$$$$$$FBIO_REFRESH-2\n");
		//fflush(stdout);
		
		#if 1//csp modify 20130101
		usleep(40*1000);
		//usleep(30*1000);
		#else
		usleep(30*1000);
		#endif
	}
	
	return NULL;
}
#endif

int GraphicsGetBuffer(GRAPHICS_SURFACE *pBuffer)
{
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	if(g_emChip == EM_GRAPH_CHIP_HISI3531 || g_emChip == EM_GRAPH_CHIP_HISI3521)
	{
		//struct fb_fix_screeninfo finfo;
		struct fb_var_screeninfo vinfo;
		HI_U32 *mapped_mem;
		HI_U32 mapped_phyaddr;
		unsigned long mapped_memlen;
		struct fb_bitfield stA16 = {15, 1, 0};
		struct fb_bitfield stR16 = {10, 5, 0};
		struct fb_bitfield stG16 = {5, 5, 0};
		struct fb_bitfield stB16 = {0, 5, 0};
		HIFB_POINT_S stPoint = {0};
		
		memset(pBuffer, 0, sizeof(GRAPHICS_SURFACE));
		
		if(ioctl(g_GraphicDev.Fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
		{
			perror("FBIOGET_VSCREENINFO");
			return -1;
		}
		
		if(ioctl(g_GraphicDev.Fd, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0)
		{
			printf("set screen original show position failed!\n");
			return -1;
		}
		
		//printf("\n\n\n\n");
		//fflush(stdout);
		//sleep(10);
		
		vinfo.xres_virtual = OSD_WIDTH_VGA;
		vinfo.xres = OSD_WIDTH_VGA;
		vinfo.yres_virtual = OSD_HEIGHT_VGA;
		vinfo.yres = OSD_HEIGHT_VGA;
		vinfo.activate = FB_ACTIVATE_NOW;
		vinfo.bits_per_pixel = 16;
		vinfo.xoffset = 0;
		vinfo.yoffset = 0;
		vinfo.red   = stR16;
		vinfo.green = stG16;
		vinfo.blue  = stB16;
		vinfo.transp = stA16;
		printf("GraphicsGetBuffer-1:w[%d] h[%d]\n",OSD_WIDTH_VGA,OSD_HEIGHT_VGA);
		
		//printf("\n\n\n\n");
		//fflush(stdout);
		//sleep(10);
		
		if(ioctl(g_GraphicDev.Fd, FBIOPUT_VSCREENINFO, &vinfo) < 0)
		{
			printf("func:%s, line:%d\n",__FUNCTION__,__LINE__);
			perror("FBIOPUT_VSCREENINFO");
			return -1;
		}
		
		pBuffer->width = vinfo.xres;
		pBuffer->height = vinfo.yres;
		printf("GraphicsGetBuffer-2:w[%d] h[%d]\n",pBuffer->width,pBuffer->height);
		
		usleep(40*1000);//csp modify 20130918
		
		//csp modify 20150110
		//for NVR-Hi3515A-4CH NVR-Hi3535-16CH
		if(1)//if(0 == access("/root/simple.txt", F_OK))
		{
			HIFB_LAYER_INFO_S stLayerInfo = {0};
			stLayerInfo.BufMode = HIFB_LAYER_BUF_NONE;//HIFB_LAYER_BUF_ONE;
			stLayerInfo.u32Mask = HIFB_LAYERMASK_BUFMODE;
			if(ioctl(g_GraphicDev.Fd, FBIOPUT_LAYER_INFO, &stLayerInfo) < 0)
			{
				printf("PUT_LAYER_INFO failed!\n");
				return -1;
			}
			int Show = HI_TRUE;
			if(ioctl(g_GraphicDev.Fd, FBIOPUT_SHOW_HIFB, &Show) < 0)
			{
				printf("FBIOPUT_SHOW_HIFB failed!\n");
				return -1;
			}
			
			struct fb_fix_screeninfo finfo;
			if (ioctl(g_GraphicDev.Fd, FBIOGET_FSCREENINFO, &finfo) < 0)
			{
				perror("FBIOGET_FSCREENINFO");
				return -1;
			}
			
			printf("simple nvr:finfo.smem_len=%d\n", finfo.smem_len);
			
			mapped_memlen = finfo.smem_len;
			mapped_phyaddr  = finfo.smem_start;
			mapped_mem = mmap(NULL, mapped_memlen, PROT_READ|PROT_WRITE, MAP_SHARED, g_GraphicDev.Fd, 0);
			if(mapped_mem == (void *)-1)
			{
				mapped_mem = NULL;
				return -1;
			}
			
			vinfo.yoffset = 0;//csp modify
			if(ioctl(g_GraphicDev.Fd, FBIOPAN_DISPLAY, &vinfo) < 0)
			{
				perror("FBIOPAN_DISPLAY failed!\n");
				return -1;
			}
			memcpy(&g_vinfo, &vinfo, sizeof(vinfo));
			
			g_stCanvasBuf.stCanvas.u32PhyAddr = mapped_phyaddr;
			g_stCanvasBuf.stCanvas.u32Height = OSD_HEIGHT_VGA;
			g_stCanvasBuf.stCanvas.u32Width = OSD_WIDTH_VGA;
			g_stCanvasBuf.stCanvas.u32Pitch = OSD_WIDTH_VGA*2;
			g_stCanvasBuf.stCanvas.enFmt = HIFB_FMT_ARGB1555;
		}
		else
		{
			HIFB_LAYER_INFO_S stLayerInfo = {0};
			stLayerInfo.BufMode = HIFB_LAYER_BUF_ONE;
			stLayerInfo.u32Mask = HIFB_LAYERMASK_BUFMODE;
			if(ioctl(g_GraphicDev.Fd, FBIOPUT_LAYER_INFO, &stLayerInfo) < 0)
			{
				printf("PUT_LAYER_INFO failed!\n");
				return -1;
			}
			int Show = HI_TRUE;
			if(ioctl(g_GraphicDev.Fd, FBIOPUT_SHOW_HIFB, &Show) < 0)
			{
				printf("FBIOPUT_SHOW_HIFB failed!\n");
				return -1;
			}
			
			HIFB_BUFFER_S stCanvasBuf;
			if(HI_FAILURE == HI_MPI_SYS_MmzAlloc(&(stCanvasBuf.stCanvas.u32PhyAddr), ((void**)&mapped_mem), 
				NULL, NULL, OSD_WIDTH_VGA*OSD_HEIGHT_VGA*2))
			{
				printf("allocate memory (maxW*maxH*2 bytes) failed\n");
				return HI_NULL;
			}
			stCanvasBuf.stCanvas.u32Height = OSD_HEIGHT_VGA;
			stCanvasBuf.stCanvas.u32Width = OSD_WIDTH_VGA;
			stCanvasBuf.stCanvas.u32Pitch = OSD_WIDTH_VGA*2;
			stCanvasBuf.stCanvas.enFmt = HIFB_FMT_ARGB1555;
			
			mapped_memlen = OSD_WIDTH_VGA*OSD_HEIGHT_VGA*2;
			mapped_phyaddr = stCanvasBuf.stCanvas.u32PhyAddr;
			
			memcpy(&g_stCanvasBuf, &stCanvasBuf, sizeof(stCanvasBuf));
			
			static unsigned char flg = 0;
			if(!flg)
			{
				pthread_t pid = 0;
				pthread_create(&pid, NULL, RefreshFB, NULL);
				
				flg = 1;
			}
			
			vinfo.yres_virtual = 0;
			if(0)//(ioctl(g_GraphicDev.Fd, FBIOPAN_DISPLAY, &vinfo) < 0)
			{
				perror("FBIOPAN_DISPLAY failed!\n");
				return -1;
			}
			memcpy(&g_vinfo, &vinfo, sizeof(vinfo));
		}
		
		memset(mapped_mem, 0x80, mapped_memlen);
		
		g_GraphicDev.GraphicMem[0].VirtAddr = (unsigned char *)mapped_mem;
		g_GraphicDev.GraphicMem[0].MemLen = mapped_memlen;
		g_GraphicDev.GraphicMem[0].TdeInfo.enColorFmt = TDE_COLOR_FMT_RGB1555;
		g_GraphicDev.GraphicMem[0].TdeInfo.u32PhyAddr = (unsigned int)mapped_phyaddr;
		g_GraphicDev.GraphicMem[0].TdeInfo.u32Height = pBuffer->height;
		g_GraphicDev.GraphicMem[0].TdeInfo.u32Width = pBuffer->width;
		g_GraphicDev.GraphicMem[0].TdeInfo.u32Stride = pBuffer->width * 2;  
		memset(&g_GraphicDev.GraphicMem[1], 0, sizeof(GRAPHIC_MEM));
		
		pBuffer->mem = (unsigned char *)mapped_mem;
		
		g_mmapBuf = (char *)mapped_mem;
		
		pBuffer->pitch = pBuffer->width * 2;
		pBuffer->format = GRAPHICS_CF_RGB555;
		
		//printf("GraphicsGetBuffer over\n");
	}
#else
	if((g_emChip == EM_GRAPH_CHIP_HISI3515) 
		|| (g_emChip == EM_GRAPH_CHIP_HISI3520))//3520/3515
	{
		struct fb_fix_screeninfo finfo;
		struct fb_var_screeninfo vinfo;
		char *mapped_mem;
		HI_U32 mapped_phyaddr;
		unsigned long mapped_memlen;
		struct fb_bitfield stA16 = {15, 1, 0};
		struct fb_bitfield stR16 = {10, 5, 0};
		struct fb_bitfield stG16 = {5, 5, 0};
		struct fb_bitfield stB16 = {0, 5, 0};
		
		memset(pBuffer, 0, sizeof(GRAPHICS_SURFACE));
		
		if(ioctl(g_GraphicDev.Fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
		{
			perror("FBIOGET_VSCREENINFO");
			return -1;
		}
		
		vinfo.xres_virtual = OSD_WIDTH_VGA;
		vinfo.xres = OSD_WIDTH_VGA;
		vinfo.yres_virtual = OSD_HEIGHT_VGA;
		vinfo.yres = OSD_HEIGHT_VGA;
				
		#ifndef ONE_BUF
		vinfo.yres_virtual *= 2;
		#endif
		
		pBuffer->width = vinfo.xres;
		pBuffer->height = vinfo.yres;
		
		vinfo.activate = FB_ACTIVATE_NOW;
		vinfo.bits_per_pixel = 16;
		vinfo.xoffset = 0;
		vinfo.yoffset = 0;
		vinfo.red   = stR16;
		vinfo.green = stG16; 
		vinfo.blue  = stB16;
		vinfo.transp = stA16;
		
		printf("w[%d] h[%d]\n",pBuffer->width,pBuffer->height);
		
		if (ioctl(g_GraphicDev.Fd, FBIOPUT_VSCREENINFO, &vinfo) < 0)
		{
			printf("func:%s, line:%d\n",__FUNCTION__,__LINE__);
			perror("FBIOPUT_VSCREENINFO");
			return -1;
		}
		
		if (ioctl(g_GraphicDev.Fd, FBIOGET_FSCREENINFO, &finfo) < 0 )
		{
			perror("FBIOGET_FSCREENINFO");
			return -1;
		}
		
		//printf(" finfo.smem_len=%d\n",  finfo.smem_len);
		
		mapped_memlen = finfo.smem_len;
		mapped_phyaddr  = finfo.smem_start;
		mapped_mem = mmap(NULL, mapped_memlen, PROT_READ|PROT_WRITE, MAP_SHARED, g_GraphicDev.Fd, 0);
		if(mapped_mem == (char *)-1)
		{
			mapped_mem = NULL;
			return -1;
		}
		
		//csp modify
		#ifndef ONE_BUF
		//vinfo.yres_virtual /= 2;
		#else
		//vinfo.yres_virtual = 0;
		#endif
		vinfo.yoffset = 0;//csp modify
		if(ioctl(g_GraphicDev.Fd, FBIOPAN_DISPLAY, &vinfo) < 0)
		{
			perror("FBIOPAN_DISPLAY failed!\n");
			return -1;
		}
		memcpy(&g_vinfo, &vinfo, sizeof(vinfo));
		
		memset(mapped_mem, 0x00, mapped_memlen);
		
		g_GraphicDev.GraphicMem[0].VirtAddr = mapped_mem;
		#ifndef ONE_BUF
		g_GraphicDev.GraphicMem[0].MemLen = mapped_memlen/2;
		#else
		g_GraphicDev.GraphicMem[0].MemLen = mapped_memlen;
		#endif
		
		g_GraphicDev.GraphicMem[0].TdeInfo.enColorFmt = TDE_COLOR_FMT_RGB1555;
		#ifdef HI3515
		g_GraphicDev.GraphicMem[0].TdeInfo.u32PhyAddr = mapped_phyaddr;
		g_GraphicDev.GraphicMem[0].TdeInfo.u32Height = pBuffer->height;
		g_GraphicDev.GraphicMem[0].TdeInfo.u32Width = pBuffer->width;
		g_GraphicDev.GraphicMem[0].TdeInfo.u32Stride = pBuffer->width * 2;
		#else
		g_GraphicDev.GraphicMem[0].TdeInfo.pu8PhyAddr = mapped_phyaddr;
		g_GraphicDev.GraphicMem[0].TdeInfo.u16Height = pBuffer->height;
		g_GraphicDev.GraphicMem[0].TdeInfo.u16Width = pBuffer->width;
		g_GraphicDev.GraphicMem[0].TdeInfo.u16Stride = pBuffer->width * 2;
		#endif
		
		#ifndef ONE_BUF
		g_GraphicDev.GraphicMem[1].VirtAddr = (void *)mapped_mem + pBuffer->height* pBuffer->width * 2;
		g_GraphicDev.GraphicMem[1].MemLen = mapped_memlen/2;

		g_GraphicDev.GraphicMem[1].TdeInfo.enColorFmt = TDE_COLOR_FMT_RGB1555;
		#ifdef HI3515
		g_GraphicDev.GraphicMem[1].TdeInfo.u32PhyAddr = (void *)mapped_phyaddr + pBuffer->height * pBuffer->width * 2;
		g_GraphicDev.GraphicMem[1].TdeInfo.u32Height = pBuffer->height;
		g_GraphicDev.GraphicMem[1].TdeInfo.u32Width = pBuffer->width;
		g_GraphicDev.GraphicMem[1].TdeInfo.u32Stride = pBuffer->width * 2;
		#else
		g_GraphicDev.GraphicMem[1].TdeInfo.pu8PhyAddr = (void *)mapped_phyaddr + pBuffer->height * pBuffer->width * 2;
		g_GraphicDev.GraphicMem[1].TdeInfo.u16Height = pBuffer->height;
		g_GraphicDev.GraphicMem[1].TdeInfo.u16Width = pBuffer->width;
		g_GraphicDev.GraphicMem[1].TdeInfo.u16Stride = pBuffer->width * 2;
		#endif
		#else
		memset(&g_GraphicDev.GraphicMem[1], 0, sizeof(GRAPHIC_MEM));
		#endif
		
		pBuffer->mem = (void *)mapped_mem;
		
		g_mmapBuf = mapped_mem;
		
		pBuffer->pitch = pBuffer->width * 2;
		pBuffer->format = GRAPHICS_CF_RGB555;
		
		#ifndef ALL_OUT
		#ifndef SINGLE_OUT
		pthread_t pid;
		static int flag = 0;
		if(0 == flag)
		{
			if(0 != pthread_create(&pid, NULL, (void*)SyncVgaToCvbs, NULL))
			{
				printf("pthread_create failed! func:%s\n", __FUNCTION__);
				return -1;
			}
			flag = 1;
		}
		#endif
		#endif
	}
#endif
	else
	{
		printf("warning:GraphicsGetBuffer g_emChip=%d#################################################################\n",g_emChip);
		return -1;
	}
	
	return 0;
}

int GraphicsSyncVgaToCvbs2(void)
{
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	return 0;
#endif
	
#ifndef ALL_OUT
	static int flag = 0;
	static char* memVirt = NULL;
	
	if(flag == 0)
	{
		struct fb_var_screeninfo vinfo;
		struct fb_bitfield stA16 = {15, 1, 0};
		struct fb_bitfield stR16 = {10, 5, 0};
		struct fb_bitfield stG16 = {5, 5, 0};
		struct fb_bitfield stB16 = {0, 5, 0};
		
		if (ioctl(g_GraphicDev.Fd_cvbs, FBIOGET_VSCREENINFO, &vinfo) < 0)
		{
			perror("FBIOGET_VSCREENINFO");
			return -1;
		}
		
		vinfo.xres_virtual = OSD_WIDTH_CVBS;
		vinfo.xres = OSD_WIDTH_CVBS;
		vinfo.yres_virtual = OSD_HEIGHT_CVBS;
		vinfo.yres = OSD_HEIGHT_CVBS;
		vinfo.activate = FB_ACTIVATE_NOW;
		vinfo.bits_per_pixel = 16;
		vinfo.xoffset = 0;
		vinfo.yoffset = 0;
		vinfo.red   = stR16;
		vinfo.green = stG16; 
		vinfo.blue  = stB16;
		vinfo.transp = stA16;
		
		#ifdef CHIP_HISI3531
		if(g_emChip == EM_GRAPH_CHIP_HISI3531)
		{
			HIFB_POINT_S stPoint = {0};
			if (ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0)
		    {
		        printf("set screen original show position failed!\n");
		        return -1;
		    }
		    
			if (ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_VSCREENINFO, &vinfo) < 0)
			{
				printf("func:%s, line:%d\n",__FUNCTION__,__LINE__);
				perror("FBIOPUT_VSCREENINFO");
				return -1;
			}
			
			HIFB_LAYER_INFO_S stLayerInfo = {0};
			stLayerInfo.BufMode = HIFB_LAYER_BUF_ONE;
			stLayerInfo.u32Mask = HIFB_LAYERMASK_BUFMODE;
		    if(ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_LAYER_INFO, &stLayerInfo) < 0)
			{
				printf("PUT_LAYER_INFO failed!\n");
				return -1;
			} 
			int Show = HI_TRUE;
		    if (ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_SHOW_HIFB, &Show) < 0)
		    {
		        printf("FBIOPUT_SHOW_HIFB failed!\n");
		        return -1;
		    }
			
			HIFB_BUFFER_S stCanvasBuf;
			if (HI_FAILURE == HI_MPI_SYS_MmzAlloc(&(stCanvasBuf.stCanvas.u32PhyAddr), ((void**)&memVirt), 
		            NULL, NULL, OSD_WIDTH_CVBS*OSD_HEIGHT_CVBS*2))
		    {
		        printf("allocate memory (maxW*maxH*2 bytes) failed\n");
		        return HI_NULL;
		    }
		    memcpy(&g_stCanvasBuf3, &stCanvasBuf, sizeof(g_stCanvasBuf3));
			
		    //if (ioctl(g_GraphicDev.Fd_cvbs, FBIOPAN_DISPLAY, &vinfo) < 0)
			//{
			//	perror("FBIOPAN_DISPLAY failed!\n");
			//	return -1;
			//}
		}
		#else
		if (ioctl(g_GraphicDev.Fd_cvbs, FBIOPUT_VSCREENINFO, &vinfo) < 0)
		{
			perror("FBIOPUT_VSCREENINFO");
			return -1;
		}	
		
		if (ioctl(g_GraphicDev.Fd_cvbs, FBIOPAN_DISPLAY, &vinfo) < 0)
		{
			perror("FBIOPAN_DISPLAY failed!\n");
			return -1;
		}
		
		struct fb_fix_screeninfo finfo;
		if (ioctl(g_GraphicDev.Fd_cvbs, FBIOGET_FSCREENINFO, &finfo) < 0 )
		{
			perror("FBIOGET_FSCREENINFO");
			return -1;
		}
		
		memVirt = mmap(NULL, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, g_GraphicDev.Fd_cvbs, 0);
		#endif
		
		if (memVirt == (char *)-1)
		{
			memVirt = NULL;
			return -1;
		}
		//memset(memVirt, 0, OSD_WIDTH_CVBS*OSD_HEIGHT_CVBS*2);
		
		flag = 1;
	}
	
	struct fb_fix_screeninfo finfo;
	struct fb_fix_screeninfo finfo2;
	
	if (ioctl(g_GraphicDev.Fd_cvbs, FBIOGET_FSCREENINFO, &finfo) < 0 )
	{
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}
	
	if (ioctl(g_GraphicDev.Fd, FBIOGET_FSCREENINFO, &finfo2) < 0 )
	{
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}
	
	int s32Ret;
	TDE_HANDLE s32Handle;
	TDE2_SURFACE_S stSrc, stDst;
	TDE2_RECT_S stSrcRect, stDstRect;
	
	memset(&stSrc, 0, sizeof(TDE2_SURFACE_S));
	memset(&stDst, 0, sizeof(TDE2_SURFACE_S));
	memset(&stSrcRect, 0, sizeof(TDE2_RECT_S));
	memset(&stDstRect, 0, sizeof(TDE2_RECT_S));
	
	s32Handle = HI_TDE2_BeginJob();
	if(HI_ERR_TDE_INVALID_HANDLE == s32Handle)
	{
		printf("HI_TDE2_BeginJob failed\n");
		return -1;
	}
	
	stSrc.enColorFmt = TDE2_COLOR_FMT_ARGB1555;
	stSrc.u32PhyAddr = (unsigned int)finfo2.smem_start;
	stSrc.u32Width = OSD_WIDTH_VGA;
	stSrc.u32Height = OSD_HEIGHT_VGA;
	stSrc.u32Stride = (OSD_WIDTH_VGA << 1);
	stSrc.bAlphaExt1555 = HI_TRUE;
	stSrc.u8Alpha0 = 0;
	stSrc.u8Alpha1 = 0xff;
	stSrcRect.s32Xpos = 0;
	stSrcRect.s32Ypos = 0;
	stSrcRect.u32Width = OSD_WIDTH_VGA;
	stSrcRect.u32Height = OSD_HEIGHT_VGA;
	
	stDst.enColorFmt = TDE2_COLOR_FMT_ARGB1555;
	stDst.u32PhyAddr = (unsigned int)finfo.smem_start;
	stDst.u32Width = OSD_WIDTH_CVBS;
	stDst.u32Height = OSD_HEIGHT_CVBS;
	stDst.u32Stride = (OSD_WIDTH_CVBS << 1);
	stDst.bAlphaExt1555 = HI_TRUE;
	stDst.u8Alpha0 = 0;
	stDst.u8Alpha1 = 0xff;
	stDstRect.s32Xpos = 0;
	stDstRect.s32Ypos = 0;
	stDstRect.u32Width = OSD_WIDTH_CVBS;
	stDstRect.u32Height = OSD_HEIGHT_CVBS;
	
	#ifdef CHIP_HISI3531
	stSrc.u32PhyAddr = g_stCanvasBuf.stCanvas.u32PhyAddr;
	stDst.u32PhyAddr = g_stCanvasBuf3.stCanvas.u32PhyAddr;
	#endif
	
	s32Ret = HI_TDE2_QuickResize(s32Handle, &stSrc, &stSrcRect, &stDst, &stDstRect);
	if(HI_SUCCESS != s32Ret) 
	{
		printf("HI_TDE2_QuickResize command failed s32Ret=0x%08x!\n", s32Ret);
	}
	
	HI_TDE2_EndJob(s32Handle, HI_FALSE, 0, 0);
	HI_TDE2_WaitForDone(s32Handle);
#endif
	
	return 0;
}

void SyncVgaToCvbs2(void)
{
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	return;
#endif
	
#ifndef ALL_OUT
	#ifdef CHIP_HISI3531
	g_stCanvasBuf3.UpdateRect.x = 0;
	g_stCanvasBuf3.UpdateRect.y = 0;
	g_stCanvasBuf3.UpdateRect.w = g_stCanvasBuf3.stCanvas.u32Width;
	g_stCanvasBuf3.UpdateRect.h = g_stCanvasBuf3.stCanvas.u32Height;
	
	int ret = 0;
	while(g_Graphics2_show)
	{
		if(0 != GraphicsSyncVgaToCvbs2())
		{
			printf("error: GraphicsSyncVgaToCvbs2\n");
		}
		
		if((ret = ioctl(g_GraphicDev.Fd_cvbs, FBIO_REFRESH, &g_stCanvasBuf3)) < 0)
		{
			printf("REFRESH failed! ret = %d\n",ret);
		}
		
		usleep(30 * 1000);
	}
	
	printf("SyncVgaToCvbs2 over!!!!!!!!!\n");
	#endif
#endif
}

int GraphicsGetBuffer2(GRAPHICS_SURFACE *pBuffer)
{
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	return 0;
#endif
	
#ifndef ALL_OUT
	static int flag = 0;
	
	if(!GraphicsGetBuffer(pBuffer))
	{
		if(0 == flag)
		{
			pthread_t pid;
			if(0 != pthread_create(&pid, NULL, (void*)SyncVgaToCvbs2, NULL))
			{
				printf("pthread_create failed! func:%s\n", __FUNCTION__);
				return -1;
			}
			flag = 1;
		}
		
		return 0;
	}
#endif
	
	return -1;
}

int GraphicsDeFlicker(RECT *pRect, int level)
{
#ifdef HI3515
	#ifdef SINGLE_OUT
	if(EM_GRAPH_OUTPUT_CVBS == emGraphOut && g_GraphicDev.GraphicMem[0].VirtAddr)
	{
		return VertFilter(pRect->right - pRect->left,
						pRect->bottom - pRect->top,
						OSD_WIDTH_CVBS,
						g_GraphicDev.GraphicMem[0].VirtAddr + pRect->top * (OSD_WIDTH_CVBS << 1) + (pRect->left << 1));
	}
	#endif
#else
	int s32Ret;
	int i;
	TDE_HANDLE s32Handle;
	TDE_DEFLICKER_COEF_S stDefCoef;
	unsigned char HDfCoef[8];
	unsigned char VDfCoef[8];
	TDE_SURFACE_S  stDst;
	TDE_SURFACE_S  stSrc;
	RECT rect;
	
	if(pRect == NULL)
	{
		return -1;
	}
	
	pthread_mutex_lock(&(g_GraphicDev.TdeLock));
	
	memcpy(&rect,pRect,sizeof(RECT));
	
	stDefCoef.pu8HDfCoef = HDfCoef;
	stDefCoef.pu8VDfCoef = VDfCoef;
	
	stDefCoef.u32HDfLevel = 0;
	stDefCoef.u32VDfLevel = 2;
	stDefCoef.pu8HDfCoef[0] = 0x00;
	stDefCoef.pu8VDfCoef[0] = 0x80;
	
	s32Handle = HI_API_TDE_BeginJob();
	if(HI_ERR_TDE_INVALID_HANDLE == s32Handle)
	{
		pthread_mutex_unlock(&(g_GraphicDev.TdeLock));
		return -1;
	}
	
	memcpy(&stSrc,&(g_GraphicDev.GraphicMem[0].TdeInfo),sizeof(TDE_SURFACE_S));
	
	stSrc.pu8PhyAddr += (g_GraphicDev.GraphicMem[0].TdeInfo.u16Stride * rect.top + rect.left * 2);
	stSrc.u16Height = rect.bottom - rect.top;
	stSrc.u16Width = rect.right - rect.left;
	
	memcpy(&stDst,&(g_GraphicDev.GraphicMem[1].TdeInfo),sizeof(TDE_SURFACE_S));
	
	stDst.pu8PhyAddr += (g_GraphicDev.GraphicMem[1].TdeInfo.u16Stride * rect.top + rect.left * 2);
	stDst.u16Height = rect.bottom - rect.top;
	stDst.u16Width = rect.right - rect.left;
	
	for(i = 0; i < level; i++)
	{
		s32Ret = HI_API_TDE_Deflicker(s32Handle
					, &stSrc
					, &stDst
					, stDefCoef);
		if(s32Ret != HI_SUCCESS)
		{
			pthread_mutex_unlock(&(g_GraphicDev.TdeLock));
			return -1;
		}
		
		s32Ret = HI_API_TDE_Deflicker(s32Handle
					, &stDst
					, &stSrc
					, stDefCoef);
		if(s32Ret != HI_SUCCESS)
		{
			pthread_mutex_unlock(&(g_GraphicDev.TdeLock));
			return -1;
		}
	}
	
	s32Ret = HI_API_TDE_EndJob(s32Handle, HI_TRUE, 100);
	if(s32Ret != HI_SUCCESS)
	{
		pthread_mutex_unlock(&(g_GraphicDev.TdeLock));
		return -1;
	}
	
	pthread_mutex_unlock(&(g_GraphicDev.TdeLock));
#endif
	
	return 0;
}

int GraphicsSetResolution(int w, int h)
{
	if((w < 720) || (w > 1920) || (h < 480) || (h > 1080))
	{
		printf("Invalid para! function:%s\n", __FUNCTION__);
		g_GraphicDev.nHeight = 600;
		g_GraphicDev.nWidth = 800;
		return -1;
	}
	
	g_GraphicDev.nHeight = h;
	g_GraphicDev.nWidth = w;
	
	printf("GraphicsSetResolution w=%d h=%d#########################################\n",w,h);
	
	return 0;
}

#include <jpeglib.h>

typedef struct _simplejpeg {
    int x;
    int y;
    int w;
    int h;
    unsigned short *bufp;
} _simplejpeg;

// 检查文件头4个字节简单鉴别是否升级文件是JPG格式
// 避免非JPG格式做解码时直接崩溃
int SimpleCheckFormat(FILE* pFile)
{
	int ret = -1, cnr;
	char buf[4] = {0};
	
	if(pFile)
	{
		cnr = fread(buf, 1, 4, pFile);
		
		printf("JPG : (%02X %02X %02X %02X)\n", buf[0], buf[1], buf[2], buf[3]);
		
		if(cnr == 4)
		{
			if( buf[0] == 0xFF &&
				buf[1] == 0xD8 &&
				buf[2] == 0xFF &&
				//csp modify 20130318
				//(buf[3]&0xFF) == 0xE0 )
				(buf[3]&0xF0) == 0xE0 )
			{
				ret = 0;
				fseek(pFile, 0, SEEK_SET);
			}
		}
	}
	
	return ret;
}

int jpeg_create(char *filename, _simplejpeg *jpegp)
{
    struct jpeg_decompress_struct cinfo;
    FILE * infile;
    JSAMPARRAY buffer;
    int pos;
    int row_stride;
    struct jpeg_error_mgr jerr;
    int x;
    unsigned short *dst;
    int r, g, b;
	
    infile = fopen(filename, "rb");
    if(infile == NULL)
	{        
        return -1;
    }
	
    if(0 != SimpleCheckFormat(infile))
    {
    	fclose(infile);
        return -3;		
    }
	
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    
    jpeg_start_decompress(&cinfo);
	
    row_stride = cinfo.output_width * cinfo.output_components;
	
    buffer = (*cinfo.mem->alloc_sarray)
             ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	
	//printf("%s %d\n", __FUNCTION__, __LINE__);
	
	//jpegp->bufp = malloc(row_stride * cinfo.output_height * 2 / 3);
    jpegp->w = cinfo.output_width;
    jpegp->h = cinfo.output_height;
	
	//printf("%s %d\n", __FUNCTION__, __LINE__);
	
    if(jpegp->bufp == NULL)
	{
    	fclose(infile);
        return -2;
    }
	
    dst = jpegp->bufp;
	
    while(cinfo.output_scanline < cinfo.output_height)
	{
        jpeg_read_scanlines(&cinfo, (JSAMPARRAY) buffer, 1);
		
        for(x = 0; x < cinfo.output_width; x++)
		{
			//__D("buffer[0][%d] = %#x\n", x, buffer[0][x]);
            if(cinfo.output_components == 1)
			{
				// Grayscale
                r = buffer[0][x] * (1<<5) / (1<<8);
                g = buffer[0][x] * (1<<6) / (1<<8);
                b = buffer[0][x] * (1<<5) / (1<<8);
            }
            else
			{
				// RGB
                pos = x * cinfo.output_components;
                r = buffer[0][pos + 0] * (1<<5) / (1<<8);
                g = buffer[0][pos + 1] * (1<<6) / (1<<8);
                b = buffer[0][pos + 2] * (1<<5) / (1<<8);
            }
			
            dst[x] = b | (g << 5) | (r << 11);
        }
		
        dst += cinfo.output_width;
    }
	
	//printf("%s %d\n", __FUNCTION__, __LINE__);
	
    jpeg_finish_decompress(&cinfo);
    fclose(infile);
	
	//printf("%s %d\n", __FUNCTION__, __LINE__);
	
    return 0;
}

int Change_RGB32_To_ARGB1555(char* pStream, unsigned int nStreamLength)
{
	if(NULL == pStream)
	{
		return -1;
	}
	
	unsigned int i = 0;
	unsigned short tmp = 0;
	
	for(i = 0; i < nStreamLength; )
	{
		//tmp = 0x0000|(((unsigned short)pStream[i+1]>>3)<<10)|(((unsigned short)pStream[i+2]>>3)<<5)|((unsigned short)pStream[i+3]>>3);
		//pStream[i>>1] = tmp&0xff;
		//pStream[(i>>1)+1] = tmp>>8;
		tmp = 0x0000|(((unsigned short)pStream[i+2]>>3)<<10)|(((unsigned short)pStream[i+2]>>3)<<5)|((unsigned short)pStream[i]>>3);
		pStream[i] = tmp&0xff;
		pStream[i+1] = tmp>>8;
		pStream[i+2] = tmp&0xff;
		pStream[i+3] = tmp>>8;
		i += 4;
	}
	//memset(pStream+(nStreamLength>>1), 0, nStreamLength>>1);
	
	return nStreamLength;//(nStreamLength>>1);
}

int Change_RGB565_To_ARGB1555(char* pStream, unsigned int nStreamLength)
{
	if(NULL == pStream)
	{
		return -1;
	}
	
	unsigned int i = 0;
	unsigned short tmp = 0;
	unsigned short *p = (unsigned short *)pStream;
	
	for(i = 0; i < nStreamLength; )
	{
		tmp = p[i>>1];
		tmp = ((tmp>>6)<<5)|(tmp&0x001f);
		p[i>>1] = tmp;	
		i += 2;
	}
	//memset(pStream+(nStreamLength>>1), 0, nStreamLength>>1);
	
	return nStreamLength;//(nStreamLength>>1);
}

int StartLogo(int w, int h)
{
	if(g_emChip == EM_GRAPH_CHIP_HISI3515)
	{
		g_GraphicDev.Fd_tmp = open(FB_DEV2, O_RDWR);
	}
	else if(g_emChip == EM_GRAPH_CHIP_HISI3520)
	{
		if(EM_GRAPH_OUTPUT_VGA == emGraphOut)
		{
			g_GraphicDev.Fd_tmp = open(FB_DEV3, O_RDWR);//显示logo中转用
		}
		else
		{
			g_GraphicDev.Fd_tmp = open(FB_DEV, O_RDWR);//显示logo中转用
		}
	}
	else if(g_emChip == EM_GRAPH_CHIP_HISI3531 || g_emChip == EM_GRAPH_CHIP_HISI3521)
	{
		//csp modify
		//g_GraphicDev.Fd_tmp = open(FB_DEV2, O_RDWR);
		g_GraphicDev.Fd_tmp = -1;
	}
	else
	{
		return -1;
	}
	
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	if(g_emChip == EM_GRAPH_CHIP_HISI3531 || g_emChip == EM_GRAPH_CHIP_HISI3521)
	{		
		HIFB_BUFFER_S stCanvasBuf;
		if(HI_FAILURE == HI_MPI_SYS_MmzAlloc(&(stCanvasBuf.stCanvas.u32PhyAddr), ((void**)&g_tmpMapBuf), NULL, NULL, w*h*2))
		{
			printf("allocate memory (maxW*maxH*2 bytes) failed\n");
			return HI_NULL;
		}
		
		memcpy(&g_stCanvasBuf2, &stCanvasBuf, sizeof(g_stCanvasBuf2));
	}
#else
	if(g_GraphicDev.Fd_tmp < 0)
	{
		printf("open dev for logo failed! func:%s\n", __FUNCTION__);
		return -1;
	}
	else
	{
		printf("open dev for logo success! func:%s\n", __FUNCTION__);
		//return -1;//csp modify 20130110
	}
	
	//csp modify 20130110
	//清除临时设备层的logo
	HIFB_ALPHA_S stAlpha;
	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0;
	stAlpha.u8Alpha1 = 0;
	if(ioctl(g_GraphicDev.Fd_tmp, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
	{
		perror("FBIOPUT_ALPHA_HIFB-4");
		return -1;
	}
	
	struct fb_var_screeninfo vinfo;
	struct fb_bitfield stA16 = {15, 1, 0};
	struct fb_bitfield stR16 = {10, 5, 0};
	struct fb_bitfield stG16 = {5, 5, 0};
	struct fb_bitfield stB16 = {0, 5, 0};
	
	if(ioctl(g_GraphicDev.Fd_tmp, FBIOGET_VSCREENINFO, &vinfo) < 0)
	{
		perror("FBIOGET_VSCREENINFO");
		return -2;
	}
	
	vinfo.xres_virtual = w;
	vinfo.xres = w;
	vinfo.yres_virtual = h;
	vinfo.yres = h;
	vinfo.activate = FB_ACTIVATE_NOW;
	vinfo.bits_per_pixel = 16;
	vinfo.xoffset = 0;
	vinfo.yoffset = 0;
	vinfo.red   = stR16;
	vinfo.green = stG16;
	vinfo.blue  = stB16;
	vinfo.transp = stA16;
	
	if(ioctl(g_GraphicDev.Fd_tmp, FBIOPUT_VSCREENINFO, &vinfo) < 0)
	{
		perror("FBIOPUT_VSCREENINFO");
		return -3;
	}
	
	if(ioctl(g_GraphicDev.Fd_tmp, FBIOPAN_DISPLAY, &vinfo) < 0)
	{
		perror("FBIOPAN_DISPLAY failed!\n");
		return -4;
	}
	
	struct fb_fix_screeninfo finfo;
	if(ioctl(g_GraphicDev.Fd_tmp, FBIOGET_FSCREENINFO, &finfo) < 0 )
	{
		perror("FBIOGET_FSCREENINFO");
		return -5;
	}
	
	g_tmpMapBuf = mmap(NULL, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, g_GraphicDev.Fd_tmp, 0);
#endif
	
	if(g_tmpMapBuf == (char *)-1)
	{
		g_tmpMapBuf = NULL;
		printf("func:%s, mmap error!\n", __FUNCTION__);
		return -6;
	}
	else
	{
		printf("StartLogo g_tmpMapBuf=0x%08x\n",(unsigned int)g_tmpMapBuf);
	}
	
	return 0;
}

int ResizeFBToFitScreen(char* pFBaddrDes, char* pFBaddrSrc, int width, int height)
{
	if((!pFBaddrDes) || (!pFBaddrSrc) || (width <=0) || (height<=0))
	{
		return -1;
	}
	
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
#else
	struct fb_fix_screeninfo finfo;
	if(ioctl(g_GraphicDev.Fd_tmp, FBIOGET_FSCREENINFO, &finfo) < 0)
	{
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}
	//printf("finfo.smem_len=%d\n", finfo.smem_len);
#endif
	
	struct fb_fix_screeninfo finfo2;
	if(ioctl(g_GraphicDev.Fd, FBIOGET_FSCREENINFO, &finfo2) < 0)
	{
		perror("FBIOGET_FSCREENINFO");
		return -1;
	}
	//printf("finfo2.smem_len=%d\n", finfo2.smem_len);
	
#ifdef ALL_OUT
	struct fb_var_screeninfo vinfo;
	if(ioctl(g_GraphicDev.Fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
	{
		perror("FBIOGET_VSCREENINFO");
		return -1;
	}
#endif
	
	int s32Ret = 0;
	TDE_HANDLE s32Handle;
	TDE2_SURFACE_S stSrc, stDst;
	TDE2_RECT_S stSrcRect, stDstRect;
	
	memset(&stSrc, 0, sizeof(TDE2_SURFACE_S));
	memset(&stDst, 0, sizeof(TDE2_SURFACE_S));
	memset(&stSrcRect, 0, sizeof(TDE2_RECT_S));
	memset(&stDstRect, 0, sizeof(TDE2_RECT_S));
	
	s32Handle = HI_TDE2_BeginJob();
	if(HI_ERR_TDE_INVALID_HANDLE == s32Handle)
	{
		printf("HI_TDE2_BeginJob failed\n");
		return -1;
	}
	
	stSrc.enColorFmt = TDE2_COLOR_FMT_ARGB1555;
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	stSrc.u32PhyAddr = (unsigned int)g_stCanvasBuf2.stCanvas.u32PhyAddr;
#else
	stSrc.u32PhyAddr = (unsigned int)finfo.smem_start;
#endif
	stSrc.u32Width = width;
	stSrc.u32Height = height;
	stSrc.u32Stride = (width << 1);
	stSrc.bAlphaExt1555 = HI_TRUE;
	stSrc.u8Alpha0 = 0xff;//0;//csp modify 20130110
	stSrc.u8Alpha1 = 0;//0xff;//csp modify 20130110
	stSrcRect.s32Xpos = 0;
	stSrcRect.s32Ypos = 0;
	stSrcRect.u32Width = width;
	stSrcRect.u32Height = height;
	
	stDst.enColorFmt = TDE2_COLOR_FMT_ARGB1555;
	stDst.u32PhyAddr = (unsigned int)finfo2.smem_start;
	#ifdef ALL_OUT
	stDst.u32Width = vinfo.xres;
	stDst.u32Height = vinfo.yres;
	stDst.u32Stride = (vinfo.xres << 1);
	#else
	stDst.u32Width = OSD_WIDTH_VGA;
	stDst.u32Height = OSD_HEIGHT_VGA;
	stDst.u32Stride = (OSD_WIDTH_VGA << 1);
	#endif
	stDst.bAlphaExt1555 = HI_TRUE;
	stDst.u8Alpha0 = 0xff;//0;//csp modify 20130110
	stDst.u8Alpha1 = 0;//0xff;//csp modify 20130110
	stDstRect.s32Xpos = 0;
	stDstRect.s32Ypos = 0;
	#ifdef ALL_OUT
	stDstRect.u32Width = vinfo.xres;
	stDstRect.u32Height = vinfo.yres;
	#else
	stDstRect.u32Width = OSD_WIDTH_VGA;
	stDstRect.u32Height = OSD_HEIGHT_VGA;
	#endif
	
	#ifndef ALL_OUT
	if(EM_GRAPH_OUTPUT_CVBS == emGraphOut)
	{
		stDst.u32Width = OSD_WIDTH_CVBS;
		stDst.u32Height = OSD_HEIGHT_CVBS;
		stDst.u32Stride = (OSD_WIDTH_CVBS << 1);
		stDstRect.u32Width = OSD_WIDTH_CVBS;
		stDstRect.u32Height = OSD_HEIGHT_CVBS;
	}
	#endif
	
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	stSrc.u32PhyAddr = g_stCanvasBuf2.stCanvas.u32PhyAddr;
	stDst.u32PhyAddr = g_stCanvasBuf.stCanvas.u32PhyAddr;
	#endif
	
	//printf("HI_TDE2_QuickResize-1\n");
	
	s32Ret = HI_TDE2_QuickResize(s32Handle, &stSrc, &stSrcRect, &stDst, &stDstRect);
	if(HI_SUCCESS != s32Ret) 
	{
		printf("HI_TDE2_QuickResize command failed s32Ret=0x%08x!\n", s32Ret);
	}
	
	//printf("HI_TDE2_QuickResize-2\n");
	
#if 0//csp modify
	HI_TDE2_WaitForDone(s32Handle);
	
	printf("HI_TDE2_WaitForDone OK\n");
	
	//HI_TDE2_EndJob(s32Handle, HI_FALSE, 0, 0);
	HI_TDE2_EndJob(s32Handle, HI_TRUE, HI_TRUE, 100);
	
	printf("HI_TDE2_EndJob OK\n");
#else
	HI_TDE2_EndJob(s32Handle, HI_FALSE, 0, 0);
	
	//printf("HI_TDE2_EndJob OK!\n");
	
	HI_TDE2_WaitForDone(s32Handle);
	
	//printf("HI_TDE2_WaitForDone OK!\n");
#endif
	
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	//csp modify 20150110
	//for NVR-Hi3515A-4CH NVR-Hi3535-16CH
	if(1)//if(0 == access("/root/simple.txt", F_OK))
	{
		
	}
	else
	{
		//printf("FBIO_REFRESH start......\n");
		if(ioctl(g_GraphicDev.Fd, FBIO_REFRESH, &g_stCanvasBuf) < 0)
		{
			printf("ResizeFBToFitScreen:REFRESH failed!\n");
			//return HI_NULL;
		}
		else
		{
			printf("ResizeFBToFitScreen:REFRESH success!\n");
		}
	}
#else
	//清除临时设备层的logo
	HIFB_ALPHA_S stAlpha;
	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0;
	stAlpha.u8Alpha1 = 0;
	if(ioctl(g_GraphicDev.Fd_tmp, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
	{
		perror("FBIOPUT_ALPHA_HIFB-4");
		return -1;
	}
	
	printf("ResizeFBToFitScreen:success!\n");
#endif
	
	return 0;
}

void GetJpgSize(char* pJpg, int* width, int* height)
{
	if(!pJpg || !width || !height)
	{
		printf("ilegal parameters!\n");

		return;
	}
	
	_simplejpeg jpegp;
	memset(&jpegp, 0, sizeof(jpegp));
	
	jpegp.bufp = NULL;
	
	//printf("***********0\n");
	
	if(-2 == jpeg_create(pJpg, &jpegp))
	{
		*width = jpegp.w;
		*height = jpegp.h;
	}
	else
	{
		*width = 0;
		*height = 0;//csp modify
	}
}

//char tmp[800*600*2] = {0};

int GraphicShowJpegPic(unsigned char nShow, char* pJPEGFileName)
{
	if(nShow)
	{
		if(NULL == pJPEGFileName)
		{
			return -1;
		}
		
		_simplejpeg jpegp;
		memset(&jpegp, 0, sizeof(jpegp));
		
		int width = 0, height = 0;
		GetJpgSize(pJPEGFileName, &width, &height);
		if(width==0 || height==0 || width>800 || height>600)
		{
			return -1;
		}
		
		//csp modify
		//char tmp[768*1024*2] = {0};
		char tmp[800*600*2] = {0};
		jpegp.bufp = (unsigned short *)tmp;
		
		//printf("***********1\n");
		
		if(0 != jpeg_create(pJPEGFileName, &jpegp))
		{
			//printf("***********2\n");
			//jpegp.w = width;
			//jpegp.h = height;
			return -1;
		}
		printf("w[%d] h[%d]\n",jpegp.w,jpegp.h);
		//fflush(stdout);
		int len = jpegp.h*jpegp.w*2;
		len = Change_RGB565_To_ARGB1555(tmp, len);
		if(len > 0)
		{
			int ret = 0;
			//printf("StartLogo-1\n");
			ret = StartLogo(jpegp.w, jpegp.h);//启动logo缓冲层
			//printf("StartLogo-2\n");
			if(0 == ret)
			{
				//printf("StartLogo-3\n");
				memcpy(g_tmpMapBuf, tmp, len);
				//printf("StartLogo-4\n");
				ret = ResizeFBToFitScreen(g_mmapBuf, g_tmpMapBuf, jpegp.w, jpegp.h);
				//printf("StartLogo-5\n");
				if(0 == ret)
				{
					printf("show logo OK!!!\n");
				}
				else
				{
					printf("show logo failed!!!\n");
				}
				
				//csp modify
				memset(g_tmpMapBuf, 0, len);
				
				#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
				HI_MPI_SYS_MmzFree(g_stCanvasBuf2.stCanvas.u32PhyAddr, g_tmpMapBuf);
				g_tmpMapBuf = NULL;
				#else
				munmap(g_tmpMapBuf, len);
				g_tmpMapBuf = NULL;
				#endif
				
				if(g_GraphicDev.Fd_tmp != -1)
				{
					close(g_GraphicDev.Fd_tmp);
					g_GraphicDev.Fd_tmp = -1;
				}
			}
		}
	}
	else//只需要隐藏cvbs, 会花屏
	{
		//printf("*********** %d line\n", __LINE__);
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
		printf("Hide Logo do nothing!!!\n");
		return 0;
		#endif
		
		int fd1 = -1;
		int fd2 = -1;
		int fd3 = -1;
		
		fd1 = open(FB_DEV1, O_RDWR);
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
		fd2 = open("/dev/fb0", O_RDWR);
		#else
		fd2 = open(FB_DEV3, O_RDWR);
		#endif
		fd3 = open(FB_DEV2, O_RDWR);//Hi3515主要是隐藏这个设备
		
		HIFB_ALPHA_S stAlpha;
		stAlpha.bAlphaEnable = HI_TRUE;
		stAlpha.bAlphaChannel = HI_FALSE;
		stAlpha.u8Alpha0 = 0;
		stAlpha.u8Alpha1 = 0;
		
		if(ioctl(fd3, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
		{
			printf("*********** %d line\n", __LINE__);
			perror("FBIOPUT_ALPHA_HIFB-7");
		}
		
		if(ioctl(fd1, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
		{
			printf("*********** %d line\n", __LINE__);
			perror("FBIOPUT_ALPHA_HIFB-5");
		}
		
		if(ioctl(fd2, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
		{
			printf("*********** %d line\n", __LINE__);
			perror("FBIOPUT_ALPHA_HIFB-6");
		}
		
		//csp modify
		if(fd1 != -1) close(fd1);
		if(fd2 != -1) close(fd2);
		if(fd3 != -1) close(fd3);
		
		printf("Hide Logo OK!!!\n");
	}
	
	return 0;
}

