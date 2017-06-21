#ifndef __LIB_PREVIEW_H
#define __LIB_PREVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum pre_mode
{
	PREVIEW_1D1 = 0,
	PREVIEW_4CIF,
	PREVIEW_9CIF,
	PREVIEW_8PIC,
	PREVIEW_CLOSE,
	PREVIEW_PIP,
	PREVIEW_6PIC_1,				//1-6
	PREVIEW_6PIC_2,				//3-8
	PREVIEW_4CIF_6,				//3-6
	PREVIEW_13CIF_1,
	PREVIEW_13CIF_2,
	PREVIEW_16CIF,
	PREVIEW_24CIF,
	PREVIEW_25CIF,
	PREVIEW_36CIF,
	PREVIEW_10PIC,
	PREVIEW_2PIC,
}pre_mode_e;

typedef struct{
	unsigned int s32X;
	unsigned int s32Y;
	unsigned int u32Width;
	unsigned int u32Height;
}pre_rect_s; 				//electronic amplification rect

typedef enum pre_ea
{
	PREVIEW_EA_NONE,		//none
	PREVIEW_EA_OPEN = 0,	//electronic amplification open
	PREVIEW_EA_CLOSE, 		//electronic amplification close
}pre_ea_e;

#define PRE_PIP_CHN(m, s1, s2)		(m | (s1<<8) | (s2<<16))	//m,s1,s2 should (>= 0 && <= 7) || (== 0xff), no other value 

void tl_preview(pre_mode_e mode, int arg);

/*
	para:
	int eaFlag, electronic amplification state
	int firstCh, dispaly channel
	pre_rect_s, zoom in on the area 

	return value: 0->ok, other->failed
*/
int tl_zoom_windows(int eaFlag, int firstCh, pre_rect_s stCapRect);

//yaogang modify 20150110
int tl_snap_preview_init(int eaFlag, pre_rect_s * pRectMenu);
int tl_snap_vdec_write(unsigned char *pbuf, unsigned int nSize);

#ifdef __cplusplus
}
#endif

#endif

