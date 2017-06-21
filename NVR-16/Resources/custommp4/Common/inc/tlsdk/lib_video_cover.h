
#ifndef __LIB_VIDEO_COVER_H
#define __LIB_VIDEO_COVER_H

#include "lib_common.h"

typedef struct{
	int x;	//range: PAL->0~720, NTSC->0~720
	int y;	//range: PAL->0~576, NTSC->0~480
	unsigned int width;
	unsigned int height;
	unsigned int color;
}video_cover_atr_t;

//return: id
int tl_video_cover_create(int chn, video_cover_atr_t *atr);
void tl_video_cover_destroy(int id);

#endif
