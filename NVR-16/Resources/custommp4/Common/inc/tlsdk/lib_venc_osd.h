
#ifndef __LIB_VENC_OSD_H
#define __LIB_VENC_OSD_H

#include "lib_common.h"

typedef struct{
	int x;
	int y;
	unsigned int width;
	unsigned int height;
	unsigned int bg_alpha;
	unsigned int fg_alpha;
	unsigned int bg_color;
}venc_osd_atr_t;

int tl_venc_osd_create(int venc_idx, venc_osd_atr_t *atr, int is_public);
int tl_venc_osd_set_bitmap(int id, char *bitmap_data, unsigned int width, unsigned int height);
int tl_venc_osd_set_postion(int id, int x, int y);
int tl_venc_osd_show(int id, int show);
void tl_venc_osd_destroy(int id);

#endif
