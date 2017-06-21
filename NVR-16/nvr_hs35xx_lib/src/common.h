#ifndef __COMMON_H
#define __COMMON_H

#include "lib_common.h"
#include "lib_preview.h"
#include "lib_venc.h"
#include "lib_audio.h"

#include <pthread.h>

#include "tl-board.h"
#include "tl-tw286x.h"

#include <semaphore.h>

#include "common_basetypes.h"
#include "common_msgcmd.h"
#include "circlebuf.h"

#ifdef HI3535
#define SAMPLE_VO_WBC_BASE  0

#define SAMPLE_VO_LAYER_VHD0 0
#define SAMPLE_VO_LAYER_VHD1 1
#define SAMPLE_VO_LAYER_VSD0 3
#define SAMPLE_VO_LAYER_VPIP 2
#endif

//csp modify 20140318
#define P2P_SUB_STREAM

#define FAST_SWITCH_PREVIEW

#ifdef HI3520D
#define VDEC_MAX_W	1920
#define VDEC_MAX_H	1200//1200//1088
#else
#define VDEC_MAX_W	1920
#define VDEC_MAX_H	1200
#endif

#define SmallMemory_DoubleFrame_value	HI_FALSE//HI_TRUE//HI_FALSE
#define HD_FRAMERATE					60//30

extern unsigned char is_decoder_techwell;
extern unsigned char is_decoder_nvp;
extern unsigned char is_decoder_rn631x;

//csp modify 20140525
extern unsigned char no_audio_chip;

#define VPSS_BSTR_CHN   0
#define VPSS_LSTR_CHN   1
#define VPSS_PRE0_CHN   2
#define VPSS_PRE1_CHN   3
#define VPSS_BYPASS_CHN 4

//===========================================================
#define VIDEO_INPUT_CHANNEL_ADUST	1	//Support channel correction
#define SUPPORT_DEFLICKER			1	//support bDeflicker

#ifdef HI3535
//#define HDMI_HAS_AUDIO
#endif

#ifdef HI3531
#define ARG_CHN_MAX					36	//support max channel number
#else
#define ARG_CHN_MAX					32	//support max channel number
#endif
//===========================================================

//vo device
#if defined(HI3531)
#define SAMPLE_VO_DEV_DHD0 0
#define SAMPLE_VO_DEV_DHD1 1
#define SAMPLE_VO_DEV_DSD0 2
#define SAMPLE_VO_DEV_DSD1 3
#define SAMPLE_VO_DEV_DSD2 4
#define SAMPLE_VO_DEV_DSD3 5
#define SAMPLE_VO_DEV_DSD4 6
#define SAMPLE_VO_DEV_DSD5 7
#elif defined(HI3535)
#define SAMPLE_VO_DEV_DHD0	0
#define SAMPLE_VO_DEV_DHD1	1
#define SAMPLE_VO_DEV_DSD0	2
#else
#define SAMPLE_VO_DEV_DHD0	0
#define SAMPLE_VO_DEV_DSD0	1
#define SAMPLE_VO_DEV_DSD1	2
#endif

#define DHD1_WBC_DSD0		1

//csp modify 20140318
#define THIRD_STREAM_VPSS_CHN		VPSS_LSTR_CHN//VPSS_PRE1_CHN//VPSS_LSTR_CHN//VPSS_BSTR_CHN

/* define by driver, fix value */
#ifndef DBG_MEM_RW_SIZE
#define DBG_MEM_RW_SIZE 	5
#endif

//csp modify 20140318
typedef struct lib_global_info_enc
{
	int open;
	int start;
	venc_size_e venc_size;
	venc_size_e venc_min_size;
	int venc_min_start;//csp modify
	int venc_third_start;//csp modify 20140318

	venc_size_e venc_third_size;
	venc_parameter_t venc_third_para;
}lib_global_info_enc;

struct lib_global_info
{
	tl_board_type_e btype;
	int is_pal;
	
	//struct vi_standard_e vi_standard[ARG_CHN_MAX];
	lib_global_info_enc venc[ARG_CHN_MAX];//struct lib_global_info_enc venc[ARG_CHN_MAX];//csp modify 20140318
	
	pre_mode_e last_pre_mode;
	int last_mode_arg;
	
	int fd_tw286x;
	int fd_tl;
	
	/* use to vb config */
	unsigned int vi_1080P_num;
	unsigned int vi_720P_num;
	unsigned int vi_960H_num;
	unsigned int vi_D1_num;
	
	/* venc sync use */
	pthread_mutex_t lock_md;
	pthread_mutex_t lock_venc;
	pthread_mutex_t lock_venc_min;
	pthread_mutex_t preview_lock;
	pthread_mutex_t i2c_bank_lock;
	pthread_mutex_t lock_ai_aenc;
	pthread_mutex_t lock_adec_ao;
	pthread_mutex_t lock_adec_ao_ref_count;//reference count
	pthread_mutex_t lock_vi_set;
	pthread_mutex_t lock_venc_pts;
	
	//csp modify 20140318
	pthread_mutex_t lock_venc_third;
	
	/* sys config info */
	int vi_num_max;
	int vo_num_max;
	
	/* vo device info */
	int vo_device_cvbs;
	int vo_device_hd;
	int vo_device_spot;
	
	/* nvr used */
	ipc_unit ipcam[ARG_CHN_MAX];
	//SCircleBufInfo preview_frame_queue;//yaogang
	//pthread_mutex_t preview_queue_lock;
	//yaogang modify 20170218 in shanghai
	SCircleBufInfo preview_frame_queue[ARG_CHN_MAX];// *2 主子码流独立，各有一个帧队列
	pthread_mutex_t preview_queue_lock[ARG_CHN_MAX];// *2
	SCircleBufInfo PreviewFxnMsgQueue;//yaogang modif 20150306 预览线程的消息队列
	SCircleBufInfo PreviewFxnMsgQueueChn[ARG_CHN_MAX/2];//*2#ifdef	PreviewFxnPerChn 主、子码流共用一个消息队列
	
	unsigned int preview_chn_threshold;
	unsigned char byMainStreamOpen[ARG_CHN_MAX];
	unsigned char bySubStreamOpen[ARG_CHN_MAX];
	unsigned int main_frame_count[ARG_CHN_MAX];
	unsigned int sub_frame_count[ARG_CHN_MAX];
	unsigned int last_main_frame_count[ARG_CHN_MAX];
	unsigned int last_sub_frame_count[ARG_CHN_MAX];
	unsigned char byMainWaitIFrame[ARG_CHN_MAX];
	unsigned char bySubWaitIFrame[ARG_CHN_MAX];
	//SMsgCmdHdr sCmdHdr;
	SCircleBufInfo venc_main_frame_queue;
	pthread_mutex_t venc_main_queue_lock;
	SCircleBufInfo venc_sub_frame_queue;
	pthread_mutex_t venc_sub_queue_lock;
	EncStreamCB pMainStreamCB;
	EncStreamCB pSubStreamCB;
	EncStreamCB pThirdStreamCB;
	AudioStreamCB pAudioStreamCB;
	SIZE_S main_stream_size[ARG_CHN_MAX];
	SIZE_S sub_stream_size[ARG_CHN_MAX];
	#ifdef FAST_SWITCH_PREVIEW
	unsigned char byDecoderType[ARG_CHN_MAX];
	pthread_mutex_t preview_chn_lock[ARG_CHN_MAX];
	#endif
	unsigned char mdenable[ARG_CHN_MAX];
	unsigned char mdevent[ARG_CHN_MAX];
	unsigned char mdnotify[ARG_CHN_MAX];
};

extern struct lib_global_info lib_gbl_info;
extern struct lib_global_info *plib_gbl_info;

#define IS_PAL()					(lib_gbl_info.is_pal)
#define TL_HSLIB_TYPE				(lib_gbl_info.btype)

#define ARG_VI_NUM_MAX 				(lib_gbl_info.vi_num_max)
#define ARG_VO_NUM_MAX 				(lib_gbl_info.vo_num_max)

#define VO_DEVICE_CVBS				(lib_gbl_info.vo_device_cvbs)
#define VO_DEVICE_SPOT				(lib_gbl_info.vo_device_spot)
#define VO_DEVICE_HD				(lib_gbl_info.vo_device_hd)

#define MD_LOCK()					pthread_mutex_lock(&plib_gbl_info->lock_md)
#define MD_UNLOCK()					pthread_mutex_unlock(&plib_gbl_info->lock_md)
#define VENC_LOCK()					pthread_mutex_lock(&plib_gbl_info->lock_venc)
#define VENC_UNLOCK()				pthread_mutex_unlock(&plib_gbl_info->lock_venc)
#define VENC_MIN_LOCK()				pthread_mutex_lock(&plib_gbl_info->lock_venc_min)
#define VENC_MIN_UNLOCK()			pthread_mutex_unlock(&plib_gbl_info->lock_venc_min)
#define PREVIEW_LOCK()				pthread_mutex_lock(&plib_gbl_info->preview_lock)
#define PREVIEW_UNLOCK()			pthread_mutex_unlock(&plib_gbl_info->preview_lock)
#define I2C_BANK_LOCK()				pthread_mutex_lock(&plib_gbl_info->i2c_bank_lock)
#define I2C_BANK_UNLOCK()			pthread_mutex_unlock(&plib_gbl_info->i2c_bank_lock)
#define AI_AENC_LOCK() 				pthread_mutex_lock(&plib_gbl_info->lock_ai_aenc)
#define AI_AENC_UNLOCK() 			pthread_mutex_unlock(&plib_gbl_info->lock_ai_aenc)
#define ADEC_AO_LOCK() 				pthread_mutex_lock(&plib_gbl_info->lock_adec_ao)
#define ADEC_AO_UNLOCK() 			pthread_mutex_unlock(&plib_gbl_info->lock_adec_ao)
#define ADEC_AO_REF_COUNT_LOCK()	pthread_mutex_lock(&plib_gbl_info->lock_adec_ao_ref_count)
#define ADEC_AO_REF_COUNT_UNLOCK() 	pthread_mutex_unlock(&plib_gbl_info->lock_adec_ao_ref_count)
#define VI_SET_LOCK()				pthread_mutex_lock(&plib_gbl_info->lock_vi_set)				
#define VI_SET_UNLOCK()				pthread_mutex_unlock(&plib_gbl_info->lock_vi_set)

//csp modify 20140318
#define VENC_THIRD_LOCK()			pthread_mutex_lock(&plib_gbl_info->lock_venc_third)
#define VENC_THIRD_UNLOCK()			pthread_mutex_unlock(&plib_gbl_info->lock_venc_third)

#define VENC_PTS_LOCK()				pthread_mutex_lock(&plib_gbl_info->lock_venc_pts)
#define VENC_PTS_UNLOCK()			pthread_mutex_unlock(&plib_gbl_info->lock_venc_pts)

int nvp_wr(int chip_id, int bankvalue, unsigned int adr, unsigned char data);
int nvp_rd(int chip_id, int bankvalue, unsigned int adr, unsigned char *out_val);

int tw286x_rd(int chip_id, unsigned int adr, unsigned char *out_val);
int tw286x_wr(int chip_id, unsigned int adr, unsigned char data);

void hs3521_wr(unsigned int reg, unsigned int data);
unsigned int hs3521_rd(unsigned int reg, unsigned int *val);

HI_S32 SAMPLE_COMM_VPSS_Start(HI_S32 s32FirstGrpIdx, HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt,VPSS_GRP_ATTR_S *pstVpssGrpAttr);
HI_S32 SAMPLE_COMM_VPSS_Stop(HI_S32 s32FirstGrpIdx, HI_S32 s32GrpCnt, HI_S32 s32ChnCnt);

#define ALIGN_BACK(x, a)		 ((a) * (((x) / (a))))

#define MULTI_OF_2_POWER(x,a)    (!((x) & ((a) - 1) ) )
#define CEILING_2_POWER(x,a)     ( ((x) + ((a) - 1) ) & ( ~((a) - 1) ) )
#define FLOOR_2_POWER(x,a)       (  (x) & (~((a) - 1) ) )
#define HIALIGN(x, a)              ((a) * (((x) + (a) - 1) / (a)))
#define HICEILING(x, a)           (((x)+(a)-1)/(a))

#define LIB_PRT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
       }while(0)

#endif

