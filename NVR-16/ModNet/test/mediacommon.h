#ifndef _MEDIA_COMMON_H_
#define _MEDIA_COMMON_H_

#include "common.h"

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
//#include <video/davincifb.h>
//#include <media/davinci_vpfe.h>

/* Davinci specific kernel headers */
//#include <video/davincifb.h>
/* Davinci specific kernel headers */
//#include <media/davinci_vpfe.h>

#define FB_OUTTYPE_CVBS		0
#define FB_OUTTYPE_VGA		1

/*图像格式*/
#define VID_CIF_FORMAT		3
#define VID_4CIF_FORMAT		4

/*码率模式*/
#define VID_CONST_BITRATE	0	/*定码率*/
#define VID_VAR_BITRATE		1	/*变码率*/

/*图像质量*/
#define VID_SPEED_FIRST		0	/*速度优先*/
#define VID_QUALITY_FIRST	1	/*质量优先*/

/*字节序*/
#define B_ENDIAN			0	/*大头序*/
#define S_ENDIAN			1	/*小头序*/
#define DEFAULT_ENDIAN		B_ENDIAN	/*默认字节序为大头序*/

/* Video window to show diagram on */
//#define RESIZER_DEVICE		"/dev/davinci_resizer"
//#define TW2834_DEVICE       "/dev/tw2834"
#define V4L2_DEVICE			"/dev/video0"
//#define SOUND_DEVICE		"/dev/dsp"
//#define FBVID_DEVICE        "/dev/fb3"
#ifdef HISI_3515
#define OSD_DEVICE_STATE_2		"/dev/fb0"//for HD
#define OSD_DEVICE_2			"/dev/fb4"//for HD

#define OSD_DEVICE_STATE		"/dev/fb2"//for SD
#define OSD_DEVICE				"/dev/fb1"//for SD
#ifdef HISI_3515_DEMO
#define OSD_DEVICE_MOUSE		"/dev/fb4"//for SD
#endif
#else
#define OSD_DEVICE          "/dev/fb/1"
#define OSD_DEVICE_STATE          "/dev/fb/0"
#endif

/* Describes a capture frame buffer */
typedef struct VideoBuffer
{
    void    *start;
	int     offset;
    size_t  length;
}VideoBuffer;

/* Custom Davinci FBDEV defines (should be in device driver header) */
#define VID0_INDEX 0
#define VID1_INDEX 1
#define ZOOM_1X    0
#define ZOOM_2X    1
#define ZOOM_4X    2

struct Zoom_Params
{
    u_int32_t WindowID;
    u_int32_t Zoom_H;
    u_int32_t Zoom_V;
};

/***
#define FBIO_WAITFORVSYNC		_IOW('F', 0x20, u_int32_t)
#define FBIO_SETZOOM			_IOW('F', 0x24, struct Zoom_Params)
#define FBIO_GETSTD				_IOR('F', 0x25, u_int32_t)
#define FBIO_SETOUTTYPE			_IOW('F', 0x27, u_int32_t)
#define FBIO_SETSTD				_IOW('F', 0x26, u_int32_t)
***/

// Scaling factors for the video standards 
#ifdef NOSTD
#undef NOSTD
#endif
#ifdef PAL
#undef PAL
#endif
#ifdef NTSC
#undef NTSC
#endif

#define NOSTD 0
#define PAL 12
#define NTSC 10


/*<video/davincifb.h>*/
/*
struct zoom_params
{
	u_int32_t window_id;
	u_int32_t zoom_h;
	u_int32_t zoom_v;
};
#define FBIO_WAITFORVSYNC		_IOW('F', 0x20, u_int32_t)
#define FBIO_SETZOOM			_IOW('F', 0x24, struct zoom_params)
#define FBIO_GETSTD				_IOR('F', 0x25, u_int32_t)
#define FBIO_SETSTD				_IOW('F', 0x26, u_int32_t)
#define FBIO_SETOUTTYPE			_IOW('F', 0x27, u_int32_t)
#define FBIO_SETRESIZE			_IOW('F', 0x28, struct resize_init_params)
#define FBIO_DORESIZE			_IOW('F', 0x29, struct resize_params)
#define LCD						0
#define NTSC					1
#define PAL						2
*/
/*<video/davincifb.h>*/

/* Screen dimensions */
#define SCREEN_BPP				16

/* Black color in UYVY format */
#define UYVY_BLACK				0x10801080

/* The 0-7 transparency value to use for the OSD */
#define OSD_TRANSPARENCY		0x77
#define MIX_TRANSPARENCY		0x55
#define IMAGE_TRANSPARENCY		0x00

extern u8 gOsdTransParency;
void SetTransParencyValue(u8 value);
u8 GetTransParencyValue();

/* Video display is triple buffered */
#define OSD_BUFS				2

/* Video display is triple buffered */
#define DISPLAY_BUFS			3

/* Video capture is triple buffered */
#define CAP_BUFS				3

int getYFactor();
int setYFactor(int yFac);

/* Screen dimensions */
#define D1_WIDTH                720
//#define D1_HEIGHT             576
#define D1_HEIGHT				(576 * getYFactor() / 12)
#define D1_LINE_WIDTH           D1_WIDTH * SCREEN_BPP / 8
#define D1_FRAME_SIZE           D1_LINE_WIDTH * D1_HEIGHT

#ifdef VGA_RESOL_FIX
#define MAX_VGA_WIDTH g_vga_width
#define MAX_VGA_HEIGHT g_vga_height
#else
#define MAX_VGA_WIDTH 800
#define MAX_VGA_HEIGHT 600
#endif


#define VGA_X_OFFSET ((g_vga_width - D1_WIDTH) >> 1)
#define VGA_Y_OFFSET ((g_vga_height - D1_HEIGHT) >> 1)

#define CAP_FIELD_INTERLACED	0
#define CAP_FIELD_SEQ_TB		1

#ifdef __cplusplus
extern "C" {
#endif

/*int initCaptureDevice(VideoBuffer **vidBufsPtr,
					  int *numVidBufsPtr, int *captureSizePtr,int fieldMode);
void cleanupCaptureDevice(int fd, VideoBuffer *vidBufs, int numVidBufs);
inline int waitForFrame(int fd);*/

int initCaptureDevice(int chn);
void cleanupCaptureDevice(int fd);
inline int waitForFrame(int *fdp,int count,fd_set *fdsp);

int encoder_start(int devfd);
int encoder_stop(int devfd);
int encoder_pause(int devfd);
int encoder_reset(int devfd);
int set_pps_sps_status(int devfd, u8 chn, u32 interval);
int set_conf_bandwidth(int devfd, u8 chn, int bitrate);

int initDisplayDevice(char *displays[]);
void cleanupDisplayDevice(int fd, char *displays[]);
int flipDisplayBuffers(int fd, int displayIdx);

int ChangeOutput(int fd,int type);

#ifdef __cplusplus
}
#endif

#endif
