#ifndef __LIB_VENC_H
#define __LIB_VENC_H

#include "lib_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	VENC_SIZE_QCIF = 0,	//PAL:176x144, NTSC:176x120
	VENC_SIZE_CIF,		//PAL:352x288, NTSC:352x240
	VENC_SIZE_2CIF,		//PAL:352x576, NTSC:352x480
	VENC_SIZE_HALF_D1,	//
	VENC_SIZE_HD1,		//PAL:704x288, NTSC:704x240
	VENC_SIZE_D1,		//PAL:704x576, NTSC:704x480
	VENC_SIZE_960H,		//PAL:960x576, NTSC:960x480
	
	VENC_SIZE_QVGA,		//320 x 240
	VENC_SIZE_VGA,		//640 x 480     
	VENC_SIZE_XGA,		//1024 x 768 
	VENC_SIZE_SXGA,		//1400 x 1050   
	VENC_SIZE_UXGA,		//1600 x 1200  
	VENC_SIZE_QXGA,		//2048 x 1536 
	
	VENC_SIZE_WVGA,		//854 x 480 
	VENC_SIZE_WSXGA,	//1680 x 1050   
	VENC_SIZE_WUXGA,	//1920 x 1200 
	VENC_SIZE_WQXGA,	//2560 x 1600
	
	VENC_SIZE_HD720,	//1280 x 720 
	VENC_SIZE_HD1080,	//1920 x 1080 
}venc_size_e;

typedef enum
{
	FRAME_TYPE_P = 1,	/*PSLICE types*/
	FRAME_TYPE_I = 5,	/*ISLICE types*/
}venc_frame_type_e;

typedef enum
{
	RC_MODE_VBR = 0,
	RC_MODE_CBR,
	RC_MODE_FIXQP,
}rc_mode_e;

typedef struct
{
	int chn;
	venc_frame_type_e type;
	unsigned long long pts;
	int rsv;
	unsigned int len;
	unsigned char *data;
	unsigned short width;
	unsigned short height;
}venc_stream_s;

typedef struct
{
	unsigned char is_cbr;
	unsigned int pic_level;//[range: 0~5,(much small, much better)]
	unsigned int bit_rate;
	unsigned int frame_rate;
	unsigned int gop;//zlb GOP_SET
}venc_parameter_t;

typedef int (*EncStreamCB)(venc_stream_s *stream);
int tl_venc_set_cb(EncStreamCB pMainStreamCB, EncStreamCB pSubStreamCB, EncStreamCB pThirdStreamCB);

int tl_venc_open(void);
void tl_venc_close(void);

//main encode
int tl_venc_start(int chn, venc_size_e size, venc_parameter_t *para);
int tl_venc_stop(int chn);
int tl_venc_read(unsigned char *in_buf, unsigned int in_len, venc_stream_s *pout_stream);
int tl_venc_get_para(int channel, int stream, venc_parameter_t *para);//0-主码流,1-次码流
int tl_venc_set_para(int channel, int stream, venc_parameter_t *para);//0-主码流,1-次码流

int tl_venc_ioctl(int rsv, int cmd, void *arg);

//minor encode
int tl_venc_minor_start(int chn, venc_size_e size, venc_parameter_t *para);
int tl_venc_minor_stop(int chn);
int tl_venc_minor_read(unsigned char *in_buf, unsigned int in_len, venc_stream_s *pout_stream);
int tl_venc_minor_get_para(int channel, venc_parameter_t *para);
int tl_venc_minor_set_para(int channel, venc_parameter_t *para);

int tl_venc_read_slave(unsigned char **in_buf, unsigned int *pLen);
int tl_venc_slave_release(unsigned char *pAddr, unsigned int Len);

int tl_venc_minor_read_slave(unsigned char **in_buf, unsigned int *pLen);
int tl_venc_minor_slave_release(unsigned char *pAddr, unsigned int Len);

int tl_venc_third_start(int chn, venc_size_e size, venc_parameter_t *para);
int tl_venc_third_stop(int chn);
int tl_venc_third_read(unsigned char *in_buf, unsigned int in_len, venc_stream_s *pout_stream);

int tl_venc_third_set_para(int channel, venc_parameter_t *para);

#ifdef __cplusplus
}
#endif

#endif

