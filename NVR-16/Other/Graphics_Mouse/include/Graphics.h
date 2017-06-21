
#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define FB_VGA

#if 1
#define CS_DEV			"/dev/fb0"	//for VGA/HDMI(hi3521)
#define FB_DEV1			"/dev/fb1"	//for VGA/HDMI(hi3531) CVBS (hi3521)
#define FB_DEV2			"/dev/fb2"	//for CVBS (hi3515)(hi3531)
#define FB_DEV3			"/dev/fb3"	//for CVBS (hi3520)
#define FB_DEV 			"/dev/fb4"	//for VGA (hi3515/hi3520) VGA/HDMI/CVBS(hi3531)
#else
#define FB_DEV 			"/dev/fb/0"
#define CS_DEV			"/dev/fb/1"
#endif

typedef struct tagGRAPHIC_MEM
{
	unsigned long MemLen;
	unsigned char *VirtAddr;
	TDE_SURFACE_S TdeInfo;
}GRAPHIC_MEM;

typedef struct tagCURSOR_DEV
{
	int Fd;
	POINT Point;
	unsigned long MemLen;
	unsigned char *PhyAddr;
	unsigned char *VirtAddr;
}CURSOR_DEV;

typedef struct tagGRAPHIC_DEV
{
	int Fd_cvbs;
	int Fd;
	int Fd_tmp;
	unsigned short nWidth;
	unsigned short nHeight;
	int Vstd;
	pthread_mutex_t TdeLock;
	CURSOR_DEV CursorDev;
	GRAPHIC_MEM GraphicMem[2];
}GRAPHIC_DEV;

typedef struct
{
	int left;
	int right;
	int top;
	int bottom;
	int stride;
	unsigned char* mem;	
}SCVBSDEFILTERPARA;

#ifdef __cplusplus
}
#endif

#endif	





















