
#ifndef __LIB_VDEC_H
#define __LIB_VDEC_H

#include "lib_common.h"
#include "lib_preview.h"

typedef struct{
	int bitrate;
}vdec_attr_s;

typedef enum{
	VDEC_SPEED_UP_8X = 8,
	VDEC_SPEED_UP_4X = 4,
	VDEC_SPEED_UP_2X = 2,
	VDEC_SPEED_NORMAL =	1,
	VDEC_SPEED_DOWN_2X = -2,
	VDEC_SPEED_DOWN_4X = -4,
	VDEC_SPEED_DOWN_8X = -8,
	VDEC_SPEED_INVERSE = 0
}vdec_speed_e;

typedef enum{
	CMD_VDEC_PAUSE = 0,
	CMD_VDEC_RESUME,
	CMD_VDEC_STEP,
	CMD_VDEC_SPEED,
	CMD_VDEC_SPEED_BASE,
	CMD_VDEC_CHANNEL_SHOW,
	CMD_VDEC_ZOOM
}cmd_vdec_e;

typedef struct{
	int rsv;
	unsigned long long pts;
	unsigned int len;
	unsigned char *data;
}vdec_stream_s;

int tl_vdec_open(int vdec_chn_num);
int tl_vdec_ioctl(int rsv, int cmd, void *arg);
int tl_vdec_write(int chn, vdec_stream_s *stream);
void tl_vdec_close(void);
int tl_vdec_flush(int chn);

#define tl_vdec_pause()		tl_vdec_ioctl(0, CMD_VDEC_PAUSE, NULL)
#define tl_vdec_step()			tl_vdec_ioctl(0, CMD_VDEC_STEP, NULL)
#define tl_vdec_resume()		tl_vdec_ioctl(0, CMD_VDEC_RESUME, NULL)
#define tl_vdec_speed(speed)	tl_vdec_ioctl(0, CMD_VDEC_SPEED, (void *)(speed))
#define tl_vdec_speed_base(speed_base)	tl_vdec_ioctl(0, CMD_VDEC_SPEED_BASE, (void *)(speed_base))
//show_cmd:bit[0~7]->channel, bit[8]->show
#define tl_vdec_channel_show(show_arg)	tl_vdec_ioctl(0, CMD_VDEC_CHANNEL_SHOW, (void *)(show_arg))
#define tl_vdec_zoom(channel)	tl_vdec_ioctl(0, CMD_VDEC_ZOOM, (void *)(channel))

/*
	para:
	int eaFlag, VDENC electronic amplification state
	int firstCh, dispaly channel
	pre_rect_s, zoom in on the area 
	
	return value: 0->ok, other->failed
*/
int tl_vdec_eletroinc_zoom(int eaFlag, int firstCh, pre_rect_s stCapRect);

#endif
