
#ifndef __LIB_VIDEO_H
#define __LIB_VIDEO_H

#include "lib_common.h"

typedef struct{
	unsigned short hue;
	unsigned short saturation;
	unsigned short contrast;
	unsigned short brightness;
}video_image_para_t;

typedef struct{
	unsigned short voLuma;
	unsigned short voContrast;
	unsigned short voSaturation;
	unsigned short voHue;
}vo_image_para_t;


//return value: 1->PAL, 0->NTSC
int tl_video_get_format(void);

//return value: bit[0]->vin0, bit[1]->vin1, ...
//bit[n] == 1, video connection; bit[n] == 0, video loss
unsigned int tl_video_connection_status(void);
unsigned int tl_get_alarm_IPCExt(void);
unsigned int tl_get_alarm_IPCCover(void);



int tl_video_get_img_para(int channel, video_image_para_t *para);
int tl_video_set_img_para(int channel, video_image_para_t *para);

//return value: 0->ok, other->failed
int tl_video_get_luma(int chn, unsigned int *luma);

//return value: 0->OK, other->failed
int tl_vo_get_img_para(unsigned int voType, vo_image_para_t *para);
//return value: 0->OK, other->failed
int tl_vo_set_img_para(unsigned int voType,vo_image_para_t *para);
//return value: 0->OK, other->failed
int tl_vo_set_img_para_all(vo_image_para_t *para);

void vo_default_attr(void);

#endif
