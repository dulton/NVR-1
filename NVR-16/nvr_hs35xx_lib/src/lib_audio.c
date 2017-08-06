/******************************************************************************

	TL hslib audio
	
	2008-12-16	created by kong
	2010-11-11	modify by andyrew

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include<pthread.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "hi_comm_aio.h"
#include "hi_comm_aenc.h"
#include "hi_comm_adec.h"

#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_aenc.h"
#include "mpi_adec.h"

#include "common.h"
#include "lib_audio.h"

#ifdef HI3535
//#define SAMPLE_AUDIO_PTNUMPERFRM   320
#define SAMPLE_AUDIO_HDMI_AO_DEV 1
#define SAMPLE_AUDIO_AI_DEV 0
#define SAMPLE_AUDIO_AO_DEV 0
#endif

#ifdef HI3531
#define MAX_AUDIO_CH			37//32
#else
#define MAX_AUDIO_CH			17//32
#endif

#ifdef HI3531
#define VOIP_AENC_CH			0
#else
#define VOIP_AENC_CH			TL_AUDIO_CH_TALK_BACK
#endif

#define PTS_REF_SYS_TIME		1

static unsigned char g_audio_dec_buffer[MAX_AUDIO_CH][(642+1024)*2];
static int g_audio_dec_len[MAX_AUDIO_CH] = {0};

extern int unsigned long long venc_get_sys_pts(int chn, unsigned long long pts);//NVR used

static int audio_out_channel = 0;//NVR used

static int audio_venc_fd[MAX_AUDIO_CH];
static int audio_start[MAX_AUDIO_CH];

#ifdef PTS_REF_SYS_TIME
static int time_update_flag = 1;
static unsigned long long base_timestamp = 0;
static unsigned long long base_pts = 0;
#endif

static int VOIP_dev_id 	= 1;	//语音对讲设备号
static int VOIP_ch_id 	= 0;	//语音对讲通道
static int NOIP_dev_id	= 0;	//普通语音设备号

#define SUPPORT_AENC_PT_LPCM  	1 //普通语音不编码

#if defined(HI3531)
	#define AUDIO_AO_DEV			4
#elif defined(HI3535)
	#define AUDIO_AO_DEV			0
#elif defined(HI3520D)
	#define AUDIO_AO_DEV			0
#else
	#define AUDIO_AO_DEV			2
#endif

#ifdef HDMI_HAS_AUDIO
HI_S32 SAMPLE_COMM_AUDIO_StartHdmi()
{
    HI_S32 s32Ret;
    HI_HDMI_ATTR_S stHdmiAttr;
    HI_HDMI_ID_E enHdmi = HI_HDMI_ID_0;
    
	#if 1
	AIO_ATTR_S stAioAttr;
	stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	stAioAttr.u32EXFlag = 1;
	stAioAttr.u32FrmNum = 30;
	stAioAttr.u32PtNumPerFrm = 321;//320;//csp modify
	stAioAttr.u32ChnCnt = 2;
	stAioAttr.u32ClkSel = 0;
	AIO_ATTR_S *pstAioAttr = &stAioAttr;
	#else
	VO_PUB_ATTR_S stPubAttr;
    VO_DEV VoDev = 0;
	
    stPubAttr.u32BgColor = 0x000000ff;
    stPubAttr.enIntfType = VO_INTF_HDMI; 
    stPubAttr.enIntfSync = VO_OUTPUT_1080P30;
	
    if(HI_SUCCESS != SAMPLE_COMM_VO_StartDev(VoDev, &stPubAttr))
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
        return HI_FAILURE;
    }
	
    s32Ret = SAMPLE_COMM_VO_HdmiStart(stPubAttr.enIntfSync);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
        return HI_FAILURE;
    }
	#endif
	
    s32Ret = HI_MPI_HDMI_SetAVMute(enHdmi, HI_TRUE);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_HDMI_GetAttr(enHdmi, &stHdmiAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
        return HI_FAILURE;
    }
    
    stHdmiAttr.bEnableAudio = HI_TRUE;        /**< if enable audio */
    stHdmiAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S; /**< source of HDMI audio, HI_HDMI_SND_INTERFACE_I2S suggested.the parameter must be consistent with the input of AO*/
    stHdmiAttr.enSampleRate = pstAioAttr->enSamplerate;        /**< sampling rate of PCM audio,the parameter must be consistent with the input of AO */
    stHdmiAttr.u8DownSampleParm = HI_FALSE;    /**< parameter of downsampling  rate of PCM audio，default :0 */
    
    stHdmiAttr.enBitDepth = 8 * (pstAioAttr->enBitwidth+1);   /**< bitwidth of audio,default :16,the parameter must be consistent with the config of AO */
    stHdmiAttr.u8I2SCtlVbit = 0;        /**< reserved，should be 0, I2S control (0x7A:0x1D) */
    
    stHdmiAttr.bEnableAviInfoFrame = HI_TRUE; /**< if enable  AVI InfoFrame*/
    stHdmiAttr.bEnableAudInfoFrame = HI_TRUE;; /**< if enable AUDIO InfoFrame*/
	
    s32Ret = HI_MPI_HDMI_SetAttr(enHdmi, &stHdmiAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_HDMI_SetAVMute(enHdmi, HI_FALSE);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}
#endif

/******************************************************************************
* function : Ao bind Adec
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_AoBindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn)
{
	MPP_CHN_S stSrcChn,stDestChn;
	
	stSrcChn.enModId = HI_ID_ADEC;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = AdChn;
	stDestChn.enModId = HI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;
	
	return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

/******************************************************************************
* function : Ao unbind Adec
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_AoUnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn)
{
	MPP_CHN_S stSrcChn,stDestChn;
	
	stSrcChn.enModId = HI_ID_ADEC;
	stSrcChn.s32ChnId = AdChn;
	stSrcChn.s32DevId = 0;
	stDestChn.enModId = HI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;
	
	return HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
}

/******************************************************************************
* function : Aenc bind Ai
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_AencBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn)
{
	MPP_CHN_S stSrcChn,stDestChn;
	
	stSrcChn.enModId = HI_ID_AI;
	stSrcChn.s32DevId = AiDev;
	stSrcChn.s32ChnId = AiChn;
	stDestChn.enModId = HI_ID_AENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = AeChn;
	
	return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

/******************************************************************************
* function : Aenc unbind Ai
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_AencUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn)
{
	MPP_CHN_S stSrcChn,stDestChn;
	
	stSrcChn.enModId = HI_ID_AI;
	stSrcChn.s32DevId = AiDev;
	stSrcChn.s32ChnId = AiChn;
	stDestChn.enModId = HI_ID_AENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = AeChn;
	
	return HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
}

/******************************************************************************
* function : Start Ao
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StartAo(AUDIO_DEV AoDevId, AO_CHN AoChn, AIO_ATTR_S *pstAioAttr)
{
	HI_S32 s32Ret;
	
	s32Ret = HI_MPI_AO_SetPubAttr(AoDevId, pstAioAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("%s: HI_MPI_AO_SetPubAttr(%d) failed with %#x!\n", __FUNCTION__, AoDevId,s32Ret);
		return HI_FAILURE;
	}
	s32Ret = HI_MPI_AO_Enable(AoDevId);
	if(HI_SUCCESS != s32Ret)
	{
		printf("%s: HI_MPI_AO_Enable(%d) failed with %#x!\n", __FUNCTION__, AoDevId, s32Ret);
		return HI_FAILURE;
	}
	s32Ret = HI_MPI_AO_EnableChn(AoDevId, AoChn);
	if(HI_SUCCESS != s32Ret)
	{
		printf("%s: HI_MPI_AO_EnableChn(%d) failed with %#x!\n", __FUNCTION__, AoChn, s32Ret);
		goto error;
	}
	
	return HI_SUCCESS;
	
error:
	HI_MPI_AO_Disable(AoDevId);
	return HI_FAILURE;
}

/******************************************************************************
* function : Stop Ao
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StopAo(AUDIO_DEV AoDevId, AO_CHN AoChn)
{
	HI_S32 s32Ret, ret = HI_SUCCESS;
	s32Ret = HI_MPI_AO_DisableChn(AoDevId, AoChn);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_AO_DisableChn error! %#x\n", s32Ret);
		ret = HI_FAILURE;
	}
	
	HI_MPI_AO_Disable(AoDevId);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_AO_Disable error! %#x\n", s32Ret);
	}
	
	return ret;
}

/******************************************************************************
* function : Start Adec
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StartAdec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType)
{
	HI_S32 s32Ret;
	ADEC_CHN_ATTR_S stAdecAttr;
	
	stAdecAttr.enType = enType;
	stAdecAttr.u32BufSize = 20;
	stAdecAttr.enMode = ADEC_MODE_STREAM;/* propose use pack mode in your app */
	
	if(PT_ADPCMA == stAdecAttr.enType)
	{
		ADEC_ATTR_ADPCM_S stAdpcm;
		stAdecAttr.pValue = &stAdpcm;
		stAdpcm.enADPCMType = ADPCM_TYPE_DVI4;//ADPCM_TYPE_IMA ;
	}
	else if(PT_G711A == stAdecAttr.enType || PT_G711U == stAdecAttr.enType)
	{
		ADEC_ATTR_G711_S stAdecG711;
		stAdecAttr.pValue = &stAdecG711;
	}
	else if(PT_G726 == stAdecAttr.enType)
	{
		ADEC_ATTR_G726_S stAdecG726;
		stAdecAttr.pValue = &stAdecG726;
		stAdecG726.enG726bps = MEDIA_G726_40K ;      
	}
	else if(PT_LPCM == stAdecAttr.enType)
	{
		ADEC_ATTR_LPCM_S stAdecLpcm;
		stAdecAttr.pValue = &stAdecLpcm;
		stAdecAttr.enMode = ADEC_MODE_PACK;/* lpcm must use pack mode */
	}
	else
	{
		printf("%s: invalid aenc payload type:%d\n", __FUNCTION__, stAdecAttr.enType);
		return HI_FAILURE;
	}
	
	/* create adec chn*/
	s32Ret = HI_MPI_ADEC_CreateChn(AdChn, &stAdecAttr);
	if(s32Ret)
	{
		printf("%s: HI_MPI_ADEC_CreateChn(%d) failed with %#x!\n", __FUNCTION__, AdChn,s32Ret);
		return s32Ret;
	}
	
	return HI_SUCCESS;
}

/******************************************************************************
* function : Stop Adec
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StopAdec(ADEC_CHN AdChn)
{
	return HI_MPI_ADEC_DestroyChn(AdChn);
}

/******************************************************************************
* function : Start Aenc
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StartAenc(AENC_CHN AeChn, PAYLOAD_TYPE_E enType)
{
	HI_S32 s32Ret;
	AENC_CHN_ATTR_S stAencAttr;
	
	/* set AENC chn attr */
	stAencAttr.enType = enType;
	stAencAttr.u32BufSize = 30;
	
	if(PT_ADPCMA == stAencAttr.enType)
	{
		AENC_ATTR_ADPCM_S stAdpcmAenc;
		stAencAttr.pValue       = &stAdpcmAenc;
		stAdpcmAenc.enADPCMType = ADPCM_TYPE_DVI4;//ADPCM_TYPE_IMA;
	}
	else if(PT_G711A == stAencAttr.enType || PT_G711U == stAencAttr.enType)
	{
		AENC_ATTR_G711_S stAencG711;
		stAencAttr.pValue       = &stAencG711;
	}
	else if(PT_G726 == stAencAttr.enType)
	{
		AENC_ATTR_G726_S stAencG726;
		stAencAttr.pValue       = &stAencG726;
		stAencG726.enG726bps    = MEDIA_G726_40K;
	}
	else if(PT_LPCM == stAencAttr.enType)
	{
		AENC_ATTR_LPCM_S stAencLpcm;
		stAencAttr.pValue = &stAencLpcm;
	}
	else
	{
		printf("%s: invalid aenc payload type:%d\n", __FUNCTION__, stAencAttr.enType);
		return HI_FAILURE;
	}
	
	/* create aenc chn*/
	s32Ret = HI_MPI_AENC_CreateChn(AeChn, &stAencAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("%s: HI_MPI_AENC_CreateChn(%d) failed with %#x!\n", __FUNCTION__, AeChn, s32Ret);
		return HI_FAILURE;
	}
	
	//printf("~~~~~~~~~~~~~~aechn:%d~~~~~~~~~~~\n", AeChn);
	
	return HI_SUCCESS;
}

/******************************************************************************
* function : Stop Aenc
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StopAenc(AENC_CHN AeChn)
{
	return HI_MPI_AENC_DestroyChn(AeChn);
	//return HI_SUCCESS;
}

#ifdef HI3535
#include "acodec.h"

#define ACODEC_FILE     "/dev/acodec"

HI_S32 SAMPLE_INNER_CODEC_CfgAudio(AUDIO_SAMPLE_RATE_E enSample, HI_BOOL bMicin)
{
    HI_S32 fdAcodec = -1;
    HI_S32 ret = HI_SUCCESS;
    unsigned int i2s_fs_sel = 0;
    unsigned int mixer_mic_ctrl = 0;
    unsigned int gain_mic = 0;

    fdAcodec = open(ACODEC_FILE,O_RDWR);
    if (fdAcodec < 0) 
    {
        printf("%s: can't open Acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        ret = HI_FAILURE;
    }
    if(ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL))
    {
    	printf("Reset audio codec error\n");
    }

    if ((AUDIO_SAMPLE_RATE_8000 == enSample)
        || (AUDIO_SAMPLE_RATE_11025 == enSample)
        || (AUDIO_SAMPLE_RATE_12000 == enSample)) 
    {
        i2s_fs_sel = 0x18;
    } 
    else if ((AUDIO_SAMPLE_RATE_16000 == enSample)
        || (AUDIO_SAMPLE_RATE_22050 == enSample)
        || (AUDIO_SAMPLE_RATE_24000 == enSample)) 
    {
        i2s_fs_sel = 0x19;
    } 
    else if ((AUDIO_SAMPLE_RATE_32000 == enSample)
        || (AUDIO_SAMPLE_RATE_44100 == enSample)
        || (AUDIO_SAMPLE_RATE_48000 == enSample)) 
    {
        i2s_fs_sel = 0x1a;
    } 
    else 
    {
        printf("%s: not support enSample:%d\n", __FUNCTION__, enSample);
        ret = HI_FAILURE;
    }

    if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel)) 
    {
        printf("%s: set acodec sample rate failed\n", __FUNCTION__);
        ret = HI_FAILURE;
    }
    
    if (HI_TRUE == bMicin)
    {
#if 1
        mixer_mic_ctrl = ACODEC_MIXER_MICIN;
        if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl))
        {
            printf("%s: set acodec micin failed\n", __FUNCTION__);
			close(fdAcodec);
            return HI_FAILURE;
        }
#endif

        /* set volume plus (0~0x1f,default 0) */
        gain_mic = 0xc;
        if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICL, &gain_mic))
        {
            printf("%s: set acodec micin volume failed\n", __FUNCTION__);
			close(fdAcodec);
            return HI_FAILURE;
        }

        if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICR, &gain_mic))
        {
            printf("%s: set acodec micin volume failed\n", __FUNCTION__);
			close(fdAcodec);
            return HI_FAILURE;
        }
    }
    
    close(fdAcodec);
    return ret;
}

/* config codec */ 
HI_S32 SAMPLE_COMM_AUDIO_CfgAcodec(AIO_ATTR_S *pstAioAttr, HI_BOOL bMicIn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*** INNER AUDIO CODEC ***/
    s32Ret = SAMPLE_INNER_CODEC_CfgAudio(pstAioAttr->enSamplerate, bMicIn); 
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s:SAMPLE_INNER_CODEC_CfgAudio failed\n", __FUNCTION__);
        return s32Ret;
    }
	
	#if 0
    /*** ACODEC_TYPE_TLV320 ***/ 
    s32Ret = SAMPLE_Tlv320_CfgAudio(pstAioAttr->enWorkmode, pstAioAttr->enSamplerate);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: SAMPLE_Tlv320_CfgAudio failed\n", __FUNCTION__);
        return s32Ret;
    }
	#endif
    
    return HI_SUCCESS;
}
#endif

/******************************************************************************
* function : Start Ai
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StartAi(AUDIO_DEV AiDevId, AIO_ATTR_S *pstAioAttr)
{
	HI_S32 s32Ret;
	
	s32Ret = HI_MPI_AI_SetPubAttr(AiDevId, pstAioAttr);
	if(s32Ret)
	{
		printf("%s: HI_MPI_AI_SetPubAttr(%d) failed with %#x\n", __FUNCTION__, AiDevId, s32Ret);
		return HI_FAILURE;
	}
	s32Ret = HI_MPI_AI_Enable(AiDevId);
	if(s32Ret)
	{
		printf("%s: HI_MPI_AI_Enable(%d) failed with %#x\n", __FUNCTION__, AiDevId, s32Ret);
		return HI_FAILURE;
	}
	
	return HI_SUCCESS;
}

/******************************************************************************
* function : Stop Ai
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StopAi(AUDIO_DEV AiDevId)
{
	return HI_MPI_AI_Disable(AiDevId);
}

static int adec_ao_start_flag = 0;	// 0:stop, 1:start
int adec_ao_reference_count = 0;	// used in function: tl_audio_enable(), tl_audio_disable(), tl_vdec_open(), tl_vdec_close()

/********************************************
start Adec & Ao. 
********************************************/
int tl_adec_ao_start()
{
	HI_S32 s32Ret;
	AIO_ATTR_S stAioAttr;
	ADEC_CHN	AdChn = 0;
	AUDIO_DEV	AoDev = AUDIO_AO_DEV;
	AO_CHN		AoChn = 0;
	
	stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	#ifdef HI3535
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	#else
	stAioAttr.enWorkmode = AIO_MODE_I2S_SLAVE;
	#endif
	stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	stAioAttr.u32EXFlag = 1;
	stAioAttr.u32FrmNum = 30;
	stAioAttr.u32PtNumPerFrm = 321;//321;//320;//csp modify
	stAioAttr.u32ChnCnt = 2;
	stAioAttr.u32ClkSel = 0;
	
	if(is_decoder_rn631x)
	{
		stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	}
	
	s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, PT_LPCM);//PT_ADPCMA
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("Start Adec failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("Start Ao failed with %#x!\n", s32Ret);
		goto error1;
	}
	
	s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("AoBindAdec failed with %#x!\n", s32Ret);
		goto error2;
	}
	
	#ifdef HDMI_HAS_AUDIO
	SAMPLE_COMM_AUDIO_StartHdmi();
	
	s32Ret = SAMPLE_COMM_AUDIO_StartAo(SAMPLE_AUDIO_HDMI_AO_DEV, AoChn, &stAioAttr);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("Start Ao failed with %#x!\n", s32Ret);
		goto error1;
	}
	
	s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(SAMPLE_AUDIO_HDMI_AO_DEV, AoChn, AdChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("AoBindAdec failed with %#x!\n", s32Ret);
		goto error2;
	}
	#endif
	
	adec_ao_start_flag = 1;
	
	return HI_SUCCESS;
	
error2:
	SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn);
error1:
	SAMPLE_COMM_AUDIO_StopAdec(AdChn);
	return HI_FAILURE;
}

int tl_adec_ao_stop()
{
	HI_S32 s32Ret, ret = HI_SUCCESS;
	ADEC_CHN    AdChn = 0;
	AUDIO_DEV   AoDev = AUDIO_AO_DEV;
	AO_CHN      AoChn = 0;
	
	adec_ao_start_flag = 0;
	
	ADEC_AO_LOCK();
	
	s32Ret = SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("AoUnbindAdec error! %#x\n", s32Ret);
		ret = HI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("Stop Ao failed with %#x!\n", s32Ret);
		ret = HI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_AUDIO_StopAdec(AdChn);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("Stop Adec failed with %#x!\n", s32Ret);
		ret = HI_FAILURE;
	}
	
	ADEC_AO_UNLOCK();
	
	return ret;
}

void *audioThrFxn(void *arg)
{
	unsigned char pEncBuf[1024];
	audio_frame_t audio_frame;
	
	//csp modify 20140525
	if(no_audio_chip)
	{
		return 0;
	}
	
	while(1)
	{
		if(!audio_start[TL_AUDIO_CH_TALK_BACK])
		{
			usleep(1000);
			continue;
		}
		
		//printf("TL_AUDIO_CH_TALK_BACK=%d,MAX_AUDIO_CH=%d\n",TL_AUDIO_CH_TALK_BACK,MAX_AUDIO_CH);
		
		memset(&audio_frame, 0, sizeof(audio_frame));
		
		int ret = tl_audio_read_data(pEncBuf, sizeof(pEncBuf), &audio_frame, 200*1000);
		//int ret = 0;
		
		//printf("tl_audio_read_data ret=%d\n",ret);
		
		if(ret <= 0)
		{
			usleep(1);
			continue;
		}
		
		if(ret > 0 && audio_frame.channel == TL_AUDIO_CH_TALK_BACK)
		{
			if(lib_gbl_info.pAudioStreamCB != NULL)
			{
				lib_gbl_info.pAudioStreamCB(&audio_frame);
			}
		}
	}
	
	return 0;
}

int tl_audio_set_cb(AudioStreamCB pAudioStreamCB)
{
	lib_gbl_info.pAudioStreamCB = pAudioStreamCB;
	return 0;
}

int tl_audio_open(void)
{
//#ifdef HI3535
//	return 0;
//#endif
	
	HI_S32 s32Ret;
	int max_ch, i;
	AUDIO_DEV AiDev;
	AIO_ATTR_S stAioAttr;
	
	//NVR used for NR1008 audio bug
	static unsigned char open_flag = 0;
	if(open_flag)
	{
		printf("hslib open audio already!!!\n");
		return 0;
	}
	printf("hslib open audio start...\n");
	open_flag = 1;
	
	for(i = 0; i < MAX_AUDIO_CH; i++)
	{
		audio_venc_fd[i] = -1;
		audio_start[i] = 0;
	}
	
	memset(g_audio_dec_buffer, 0, sizeof(g_audio_dec_buffer));
	memset(g_audio_dec_len, 0, sizeof(g_audio_dec_len));
	
	if(TL_BOARD_TYPE_NR2116 == TL_HSLIB_TYPE)
	{
	#ifdef HI3535
		VOIP_dev_id = 0;
		VOIP_ch_id = 1;
		NOIP_dev_id = 0;
		max_ch = 2;
	#else
		VOIP_dev_id = 1;
		VOIP_ch_id = 0;
		NOIP_dev_id = 0;
		max_ch = 16;
	#endif
	}
	else if(TL_BOARD_TYPE_NR3116 == TL_HSLIB_TYPE)
	{
		VOIP_dev_id = 4;
		VOIP_ch_id = 0;
		NOIP_dev_id = 1;
		max_ch = 16;
		
		if(is_decoder_techwell)
		{
			//I2C_BANK_LOCK();
			//tw286x_wr(0, 0x40, 0);
			//tw286x_wr(0, 0xDB, 0xe1);
			//tw286x_wr(0, 0x89, 0x05);/* AIN5MD=1, AFS384=0 */
			//tw286x_wr(0, 0xD2, 0x02);
			//I2C_BANK_UNLOCK();
		}
	}
	else if(TL_BOARD_TYPE_NR3132 == TL_HSLIB_TYPE)
	{
		VOIP_dev_id = 4;
		VOIP_ch_id = 0;
		NOIP_dev_id = 1;
		max_ch = 16;
	}
	else if(TL_BOARD_TYPE_NR3124 == TL_HSLIB_TYPE)
	{
		VOIP_dev_id = 4;
		VOIP_ch_id = 0;
		NOIP_dev_id = 1;
		max_ch = 16;
	}
	else if(TL_BOARD_TYPE_NR1004 == TL_HSLIB_TYPE || TL_BOARD_TYPE_NR1008 == TL_HSLIB_TYPE)
	{
		VOIP_dev_id = 0;
		VOIP_ch_id = 0;//8;
		NOIP_dev_id = 0;
		max_ch = 16;
	}
	else
	{
		printf("lib_audio.c@audio_open: not support this type=%d\n", TL_HSLIB_TYPE);
		return -1;
	}
	
	//csp modify 20140525
	if(no_audio_chip)
	{
		printf("#####################################skip rz_audio_open because no audio chip\n");
		return 0;
	}
	
	//#ifdef HI3535
	//SAMPLE_INNER_CODEC_CfgAudio(AUDIO_SAMPLE_RATE_8000, HI_FALSE);
	//#endif
	
	stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	#ifdef HI3535
	stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
	#else
	stAioAttr.enWorkmode = AIO_MODE_I2S_SLAVE;
	#endif
	stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	stAioAttr.u32EXFlag = 1;
	stAioAttr.u32FrmNum = 30;
	stAioAttr.u32PtNumPerFrm = 321;//321;//320;//csp modify
	stAioAttr.u32ChnCnt = max_ch;
	stAioAttr.u32ClkSel = 0;
	
	AiDev = VOIP_dev_id;
	s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, &stAioAttr);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("Start Ai failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	//start adec and ao
	ADEC_AO_REF_COUNT_LOCK();
	if(!adec_ao_reference_count)
	{
		if(HI_SUCCESS != tl_adec_ao_start())
		{
			LIB_PRT("tl_adec_ao_start FAIL\n");
			ADEC_AO_REF_COUNT_UNLOCK();
			return -1;
		}
	}
	adec_ao_reference_count++;
	ADEC_AO_REF_COUNT_UNLOCK();
	
	for(i = 0; i < MAX_AUDIO_CH; i++)
	{
		audio_start[i] = 0;
	}
	
	//NVR used
	pthread_t pid;
	pthread_create(&pid, NULL, audioThrFxn, NULL);
	
	printf("#####################################rz_audio_open finished\n");
	
	return 0;
}

int tl_audio_close(void)
{
	HI_S32 s32Ret;
	AUDIO_DEV AiDev;
	
	AiDev = VOIP_dev_id;
	s32Ret = SAMPLE_COMM_AUDIO_StopAi(AiDev);
	if(s32Ret != HI_SUCCESS)
	{
		LIB_PRT("Start Ai failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	//stop adec and ao
	ADEC_AO_REF_COUNT_LOCK();
	adec_ao_reference_count--;
	if(!adec_ao_reference_count)
	{
		if(HI_SUCCESS != tl_adec_ao_stop())
		{
			LIB_PRT("tl_adec_ao_stop FAIL\n");
		}
	}
	ADEC_AO_REF_COUNT_UNLOCK();
	
	return 0;
}

/*
 * acoustic coding
*/
int tl_audio_read_data(unsigned char *in_buf, unsigned int in_buf_len, audio_frame_t *out_frame, unsigned int timeout_us)
{
	static int cur_audio_ch = 0;
	
	HI_S32 s32Ret;
	int dev_id, ch_id;
	int maxfd = -1;
	fd_set read_fds;
	struct timeval TimeoutVal; 
	int i = 0;
	AUDIO_STREAM_S stStream;
	AENC_CHN AeChn;
	
	if(in_buf == NULL || out_frame == NULL)
	{
		printf("lib_audio.c@tl_audio_enc_read_data: invalid arg\n");
		return -1;
	}
	
	FD_ZERO(&read_fds);
	for(i = 0; i < MAX_AUDIO_CH; i++)
	{
		if(i != TL_AUDIO_CH_TALK_BACK)//NVR used
		{
			continue;
		}
		if(audio_start[i] == 0)
		{
			continue;
		}
		FD_SET(audio_venc_fd[i],&read_fds);
		if(audio_venc_fd[i] > maxfd)
		{
			maxfd = audio_venc_fd[i];
		}
	}
	if(maxfd < 0)
	{
		printf("lib_audio.c@tl_audio_enc_read_data: no audio channel enable\n");
		return 0;
	}
	
	TimeoutVal.tv_sec = timeout_us/1000000;
	TimeoutVal.tv_usec = timeout_us%1000000;
	s32Ret = select(maxfd+1, &read_fds, NULL, NULL, &TimeoutVal);
	if(s32Ret < 0)
	{
		printf("lib_audio.c@tl_audio_enc_read_data: select err, ret=0x%x\n", s32Ret);
		return -1;
	}
	else if(0 == s32Ret)
	{
		printf("audio select time out\n");
		return 0;
	}
	else
	{
		//printf("audio select ok\n");
	}
	
	for(i = 0; i < MAX_AUDIO_CH; i++)
	{
		cur_audio_ch = (cur_audio_ch + 1) % MAX_AUDIO_CH;
		if(cur_audio_ch != TL_AUDIO_CH_TALK_BACK)//NVR used
		{
			continue;
		}
		if(audio_start[cur_audio_ch] == 0)
		{
			continue;
		}
		
		if(FD_ISSET(audio_venc_fd[cur_audio_ch],&read_fds))
		{
			if(cur_audio_ch == TL_AUDIO_CH_TALK_BACK)
			{
				dev_id = VOIP_dev_id;
				ch_id = VOIP_ch_id;
				AeChn = VOIP_AENC_CH;
			}
			else
			{
				dev_id = NOIP_dev_id;
				ch_id = cur_audio_ch;
				AeChn = cur_audio_ch;
			}
			
			//printf("audio has data\n");
			
			AI_AENC_LOCK();
			
			s32Ret = HI_MPI_AENC_GetStream(AeChn, &stStream, HI_IO_NOBLOCK);
			if(HI_SUCCESS != s32Ret)
			{
				printf("lib_audio.c@tl_audio_enc_read_data: HI_MPI_AENC_GetStream ret=0x%x\n", s32Ret);
				AI_AENC_UNLOCK();
				return -1;
			}
			if(in_buf_len < stStream.u32Len)
			{
				printf("lib_audio.c@tl_audio_enc_read_data: buffer too small, need=%d, in_len=%d\n", stStream.u32Len, in_buf_len);
				HI_MPI_AENC_ReleaseStream(AeChn, &stStream);
				AI_AENC_UNLOCK();
				return -1;
			}
			
			//printf("audio get data size:%d\n",stStream.u32Len);
			
			out_frame->channel = cur_audio_ch;
			
			out_frame->data = in_buf;
			out_frame->len = stStream.u32Len;
			out_frame->time_stamp = stStream.u64TimeStamp;
			
		#ifdef PTS_REF_SYS_TIME
			if(time_update_flag)
			{
				time_update_flag = 0;
				struct timeval time_pos;
				gettimeofday(&time_pos, NULL);
				base_timestamp = 1000*1000;
				base_timestamp = base_timestamp * time_pos.tv_sec;
				base_timestamp = base_timestamp + time_pos.tv_usec;
				base_pts = stStream.u64TimeStamp;
			}
			out_frame->time_stamp = base_timestamp + (stStream.u64TimeStamp - base_pts);
		#endif /* PTS_REF_SYS_TIME */
			
		#ifdef  SUPPORT_AENC_PT_LPCM
			out_frame->is_raw_data = 1;
		#else
			out_frame->is_raw_data = 0;
		#endif  /*  SUPPORT_AENC_PT_LPCM */
			
			memcpy(out_frame->data, stStream.pStream, out_frame->len);
			HI_MPI_AENC_ReleaseStream(AeChn, &stStream);
			
			AI_AENC_UNLOCK();
			
			return out_frame->len;
		}
	}
	
	return -1;
}

/*
 * Voice coding
*/
int tl_audio_write_data(audio_frame_t *in_frame)
{
	static int cur_adec_ch = 0;
	AUDIO_STREAM_S stAudioStream;
	HI_S32 s32ret = -1;
	
	//csp modify 20140525
	if(no_audio_chip)
	{
		return 0;
	}
	
	if(in_frame->len > MAX_AUDIO_FRAME_LEN*2)
	{
		printf("lib_audio.c@tl_audio_dec_write_data: buf too large, max=%d, in_len=%d\n", MAX_AUDIO_FRAME_LEN*2, in_frame->len);
		return -1;
	}
	
	stAudioStream.pStream = in_frame->data;
	stAudioStream.u32Len = in_frame->len;
	
	ADEC_AO_LOCK();
	
	if(adec_ao_start_flag)
	{
		s32ret = HI_MPI_ADEC_SendStream(cur_adec_ch, &stAudioStream, HI_TRUE);
		if(s32ret)
		{
			ADEC_AO_UNLOCK();
			printf("lib_audio.c@tl_audio_dec_write_data: HI_MPI_ADEC_SendStream ret=0x%x\n", s32ret);
			return -1;
		}
	}
	else
	{
		in_frame->len = 0;
	}
	
	ADEC_AO_UNLOCK();
	
	return in_frame->len;
}

/*
 *Audio channel enabled
*/
int tl_audio_enable(int channel)
{
	HI_S32 s32Ret;
	int ai_dev, ai_ch;
	AENC_CHN AeChn;
	
	//printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$rz_audio_enable channel=%d,MAX_AUDIO_CH=%d,TL_AUDIO_CH_TALK_BACK=%d\n",channel,MAX_AUDIO_CH,TL_AUDIO_CH_TALK_BACK);
	
	if(channel < 0 || channel >= MAX_AUDIO_CH)
	{
		printf("lib_audio.c@tl_audio_enable: invalid arg, channel=%d\n", channel);
		return -1;
	}
	
	if(audio_start[channel])
	{
		printf("lib_audio.c@tl_audio_enable: already enable, channel=%d\n", channel);
		return 0;
	}
	
	if(channel == TL_AUDIO_CH_TALK_BACK)
	{
		ai_dev = VOIP_dev_id;
		ai_ch = VOIP_ch_id;
		
		//printf("rz_audio_enable:start voip\n");
		
		//csp modify 20140525
		if(no_audio_chip)
		{
			//audio_start[channel] = 1;
			return 0;
		}
		
		/********************************************
		Enable AiChn
		********************************************/
		s32Ret = HI_MPI_AI_EnableChn(ai_dev, ai_ch);
	    if(s32Ret != HI_SUCCESS)
	    {
	        printf("%s: HI_MPI_AI_EnableChn(%d,%d) failed with %#x\n", __FUNCTION__, ai_dev, ai_ch, s32Ret);
	        return -1;
	    }

		//printf("%s 1\n", __func__);
#ifdef HI3535
		static unsigned char aenc_start_flag = 0;
		if(aenc_start_flag == 0)
		{
			/********************************************
			start Aenc
			********************************************/
			AeChn = VOIP_AENC_CH;
			s32Ret = SAMPLE_COMM_AUDIO_StartAenc(AeChn, PT_LPCM);//PT_ADPCMA
			if(s32Ret != HI_SUCCESS)
			{
				LIB_PRT("Start Aenc failed with %#x!\n", s32Ret);
				goto error1;
			}
			
			audio_venc_fd[channel] = HI_MPI_AENC_GetFd(AeChn);
			if(audio_venc_fd[channel] < 0)
			{
				printf("lib_audio.c@tl_audio_enable: HI_MPI_AENC_GetFd, ret=0x%x\n", audio_venc_fd[channel]);
				goto error2;
			}
			
			aenc_start_flag = 1;
		}
#else
		/********************************************
		start Aenc
		********************************************/
		AeChn = VOIP_AENC_CH;
		s32Ret = SAMPLE_COMM_AUDIO_StartAenc(AeChn, PT_LPCM);//PT_ADPCMA
		if(s32Ret != HI_SUCCESS)
		{
			LIB_PRT("Start Aenc failed with %#x!\n", s32Ret);
			goto error1;
		}
		
		audio_venc_fd[channel] = HI_MPI_AENC_GetFd(AeChn);
		if(audio_venc_fd[channel] < 0)
		{
			printf("lib_audio.c@tl_audio_enable: HI_MPI_AENC_GetFd, ret=0x%x\n", audio_venc_fd[channel]);
			goto error2;
		}
#endif
		//printf("%s 2\n", __func__);
		
		/********************************************
		Aenc bind Ai Chn
		********************************************/
		s32Ret = SAMPLE_COMM_AUDIO_AencBindAi(ai_dev, ai_ch, AeChn);
		if(s32Ret != HI_SUCCESS)
		{
			LIB_PRT("AencBindAi failed with %#x!\n", s32Ret);
			goto error2;
		}
	}
	else
	{
		ai_dev = NOIP_dev_id;
		ai_ch = channel;
	}
	
	audio_start[channel] = 1;
	
	return 0;
	
error2:
	SAMPLE_COMM_AUDIO_StopAenc(AeChn);
error1:
	HI_MPI_AI_DisableChn(ai_dev, ai_ch);
	return -1;
}

/*
 *Audio channel disabled
*/
int tl_audio_disable(int channel)
{
	HI_S32 s32Ret, ret = 0;
	int ai_dev, ai_ch;
	AENC_CHN AeChn;
	
	if(channel < 0 || channel >= MAX_AUDIO_CH)
	{
		printf("lib_audio.c@tl_audio_disable: invalid arg, channel=%d\n", channel);
		return -1;
	}
	
	if(audio_start[channel] == 0)
	{
		printf("lib_audio.c@tl_audio_disable: not enable, channel=%d\n", channel);
		return 0;
	}
	
	AI_AENC_LOCK();
	
	//printf("tl_audio_disable channel=%d,TL_AUDIO_CH_TALK_BACK=%d\n",channel,TL_AUDIO_CH_TALK_BACK);
	//fflush(stdout);
	
	if(channel == TL_AUDIO_CH_TALK_BACK)
	{
		//printf("tl_audio_disable talk\n");
		//fflush(stdout);
		
		ai_dev = VOIP_dev_id;
		ai_ch = VOIP_ch_id;
		
		audio_start[channel] = 0;
		
		//csp modify 20140525
		if(no_audio_chip)
		{
			AI_AENC_UNLOCK();
			return 0;
		}
		
		AeChn = VOIP_AENC_CH;
		
		s32Ret = SAMPLE_COMM_AUDIO_AencUnbindAi(ai_dev, ai_ch, AeChn);
		if(s32Ret != HI_SUCCESS)
		{
			LIB_PRT("AencUnbindAi error!%#x\n", s32Ret);
			ret = -1;
		}
		
#ifdef HI3535
		//Not stop
#else
		s32Ret = SAMPLE_COMM_AUDIO_StopAenc(AeChn);
		if(s32Ret != HI_SUCCESS)
		{
			LIB_PRT("StopAenc error!%#x\n", s32Ret);
			ret = -1;
		}
#endif
		
		s32Ret = HI_MPI_AI_DisableChn(ai_dev, ai_ch);
		if(s32Ret != HI_SUCCESS)
		{
			LIB_PRT("AI_DisableChn error!%#x\n", s32Ret);
			ret = -1;
		}
	}
	else
	{
		ai_dev = NOIP_dev_id;
		ai_ch = channel;
		
		audio_start[channel] = 0;
	}
	
	//printf("tl_audio_disable over\n");
	//fflush(stdout);
	
	AI_AENC_UNLOCK();
	
	return ret;
}

/*
 * Audio channels so that it can export
 */
int tl_audio_out_sel(int channel)
{
	unsigned char chn = channel;
	printf("%s: chn%d\n", __func__, channel);
	
	if(TL_HSLIB_TYPE == TL_BOARD_TYPE_NR2116 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR3116 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR3132 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR3124 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR1004 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR1008)
	{
		audio_out_channel = chn;//NVR used
		
		//csp modify 20140525
		if(no_audio_chip)
		{
			return 0;
		}
		
		if(1)//if(TL_AUDIO_CH_PLAY_BACK == chn || TL_AUDIO_CH_TALK_BACK == chn)//NVR used
		{
			if(is_decoder_techwell)
			{
				I2C_BANK_LOCK();
				tw286x_wr(0, 0x40, 0);
				tw286x_wr(0, 0xE0, 0x10);
				I2C_BANK_UNLOCK();
			}
			else if(is_decoder_nvp)
			{
				nvp_wr(0, 0x01, 0x23, 0x10);
			}
			else if(is_decoder_rn631x)
			{
				I2C_BANK_LOCK();
				tw286x_wr(0, 0xFF, 0x04);
				tw286x_wr(0, 0x76, 0x10);
				I2C_BANK_UNLOCK();
			}
			else
			{
				//tw286x_wr(1, 0x2010, 0xA0);
			}
		}
		else
		{
			if(is_decoder_techwell)
			{
				I2C_BANK_LOCK();
				tw286x_wr(0, 0x40, 0);
				tw286x_wr(0, 0xE0, chn);
				I2C_BANK_UNLOCK();
			}
			else if(is_decoder_nvp)
			{
				nvp_wr(0, 0x01, 0x23, chn);
			}
			else if(is_decoder_rn631x)
			{
				I2C_BANK_LOCK();
				tw286x_wr(0, 0xFF, 0x04);
				tw286x_wr(0, 0x76, 0x1c);
				I2C_BANK_UNLOCK();
			}
			else
			{
				//value = ((chn & 0x0f) << 4) | 0x00;
				//tw286x_wr(1, 0x2010, value);
			}
		}
	}
	
	return 0;
}

#define AUDIO_VOLUME_MIN	1
#define AUDIO_VOLUME_MAX	15

/*
 *  Receive audio Volume properties
*/
int tl_audio_get_volume_atr(audio_volume_atr_t *patr)
{
	if(patr == NULL)
	{
		printf("lib_audio.c@tl_audio_get_volume_atr: atr==NULL\n");
		return -1;
	}
	patr->min_val = AUDIO_VOLUME_MIN;
	patr->max_val = AUDIO_VOLUME_MAX;
	return 0;
}

/*
 *  Set Input audio volume
*/
int tl_audio_set_volume(int val)
{
	//csp modify 20140525
	if(no_audio_chip)
	{
		return 0;
	}
	
	unsigned char reg_val = 0;
	
	if(val < AUDIO_VOLUME_MIN)
	{
		reg_val = 0;
	}
	else if(val > AUDIO_VOLUME_MAX)
	{
		reg_val = AUDIO_VOLUME_MAX;
	}
	else
	{
		reg_val = val;
	}
	
	if(TL_HSLIB_TYPE == TL_BOARD_TYPE_NR2116 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR3116 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR3132 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR3124 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR1004 || 
		TL_HSLIB_TYPE == TL_BOARD_TYPE_NR1008)
	{
		#ifdef HI3535
		HI_S32 s32VolumeDb = -36;
		if(val <= AUDIO_VOLUME_MIN)
		{
			s32VolumeDb = -36;
		}
		else if(val >= AUDIO_VOLUME_MAX)
		{
			s32VolumeDb = 6;
		}
		else
		{
			s32VolumeDb = -36 + 42 * (val - AUDIO_VOLUME_MIN) / (AUDIO_VOLUME_MAX - AUDIO_VOLUME_MIN);
		}
		//printf("tl_audio_set_volume: %d\n",s32VolumeDb);
		//HI_MPI_AO_SetVolume(SAMPLE_AUDIO_AO_DEV, s32VolumeDb);
		#ifdef HDMI_HAS_AUDIO
		//HI_MPI_AO_SetVolume(SAMPLE_AUDIO_HDMI_AO_DEV, s32VolumeDb);
		#endif
		#endif
		
		if(is_decoder_techwell)
		{
			I2C_BANK_LOCK();
			
			unsigned char banksave = 0;
			tw286x_rd(0, 0x40, &banksave);
			
			//tw286x_wr(0, 0x40, 0x00);
			//tw286x_wr(0, 0xd0, (reg_val<<4)|reg_val);
		    //tw286x_wr(0, 0xd1, (reg_val<<4)|reg_val);
		    //tw286x_wr(0, 0x7f, (reg_val<<4)|reg_val);
			
		    //tw286x_wr(0, 0x40, 0x01);
		    //tw286x_wr(0, 0xd0, (reg_val<<4)|reg_val);
		    //tw286x_wr(0, 0xd1, (reg_val<<4)|reg_val);
		    //tw286x_wr(0, 0x7f, (reg_val<<4)|reg_val);
			
			tw286x_wr(0, 0x40, 0x00);
			tw286x_wr(0, 0xdf, (reg_val<<4)|0);
			
			tw286x_wr(0, 0x40, banksave);
			
			I2C_BANK_UNLOCK();
		}
		else if(is_decoder_nvp)
		{
			//nvp_wr(0, 0x01, 0x01, (reg_val<<4)|reg_val);//AI
			//nvp_wr(0, 0x01, 0x02, (reg_val<<4)|reg_val);//AI
			//nvp_wr(0, 0x01, 0x03, (reg_val<<4)|reg_val);//AI
			//nvp_wr(0, 0x01, 0x04, (reg_val<<4)|reg_val);//AI
			//nvp_wr(0, 0x01, 0x05, (reg_val<<4)|reg_val);//MIC
			nvp_wr(0, 0x01, 0x22, (reg_val<<4)|0);//AO
		}
		else if(is_decoder_rn631x)
		{
			I2C_BANK_LOCK();
			tw286x_wr(0, 0xFF, 0x04);
			//tw286x_wr(0, 0x69, reg_val);
			tw286x_wr(0, 0x5D, ((2*reg_val)+(reg_val?1:0))|0x60);
			I2C_BANK_UNLOCK();
		}
		else
		{
			//reg_val = val&0x3F;
			//tw286x_wr(1, 0x20A9, reg_val);
		}
    }
	
	//printf("FUC:%s, line:%d, reg_val:%d\n", __FUNCTION__, __LINE__, reg_val);
	
    return 0;
}

/*
 * this function be call from lib_venc.c@tl_venc_update_basetime
 */
void audio_update_basetime(void)
{
	#ifdef PTS_REF_SYS_TIME
	time_update_flag = 1;
	#endif
}

typedef short _int16;

#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS		(8)		/* Number of A-law segments. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */

_int16 g711_alaw2linear(unsigned char a_val)
{
	_int16		t;
	_int16		seg;

	a_val ^= 0x55;

	t = (a_val & QUANT_MASK) << 4;
	seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
	switch (seg) {
	case 0:
		t += 8;
		break;
	case 1:
		t += 0x108;
		break;
	default:
		t += 0x108;
		t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
}

void ulaw_expand(long lseg, char *logbuf, short *linbuf)
//  long            lseg;
//  short          *logbuf;
//  short          *linbuf;
{
	long            n;		/* aux.var. */
	short           segment;	/* segment (Table 2/G711, column 1) */
	short           mantissa;	/* low  nibble of log companded sample */
	short           exponent;	/* high nibble of log companded sample */
	short           sign;		/* sign of output sample */
	short           step;
	short           temp;  	  
	
	for(n = 0; n < lseg; n++)
	{
		temp = logbuf[n]&0x00FF;
		sign = temp < (0x0080)? -1:1;	//sign-bit = 1 for positiv values 
		mantissa = ~logbuf[n];	// 1's complement of input value 
		
		exponent = (mantissa >> 4) & (0x0007);	/* extract exponent */
		segment = exponent + 1;	/* compute segment number */
		mantissa = mantissa & (0x000F);	/* extract mantissa */
		
		/* Compute Quantized Sample (14 bit left justified!) */
		step = (4) << segment;	/* position of the LSB */
		/* = 1 quantization step) */
		linbuf[n] = sign *		/* sign */
					(
						((0x0080) << exponent)	/* '1', preceding the mantissa */
						+ step * mantissa	/* left shift of mantissa */
						+ step / 2		/* 1/2 quantization step */
						- 4 * 33
					);
	}
}

int DealRealAudio(real_stream_s *stream, unsigned int dwContext)
{
	if(stream == NULL || stream->chn != dwContext)
	{
		printf("DealRealAudio: param error\n");
		return -1;
	}
	
	int channel = stream->chn;
	
	if(channel >= ARG_VI_NUM_MAX || channel < 0)
	{
		printf("DealRealAudio: channel error: %d\n", channel);
		return -1;
	}
	
	//printf("###chn%d real audio data, media_type: %d, len: %d###\n", stream->chn, stream->media_type, stream->len);
	
	short *audio_out_buffer = (short *)((unsigned char *)g_audio_dec_buffer[channel]+g_audio_dec_len[channel]);
	int audio_out_len = 0;
	
	if(stream->media_type == MEDIA_PT_G711 || stream->media_type == MEDIA_PT_PCMU || stream->media_type == MEDIA_PT_G726)
	{
		if(audio_start[channel] || (lib_gbl_info.last_pre_mode != PREVIEW_CLOSE && channel == audio_out_channel && !audio_start[TL_AUDIO_CH_TALK_BACK]))
		{
			if(stream->media_type == MEDIA_PT_G711)
			{
				if(g_audio_dec_len[channel] + (stream->len * 2) > sizeof(g_audio_dec_buffer[channel]))
				{
					return -1;
				}
				
				int k = 0;
				for(k=0;k<stream->len;k++)
				{
					audio_out_buffer[k] = g711_alaw2linear(stream->data[k]);
				}
				audio_out_len = stream->len * 2;
			}
			else if(stream->media_type == MEDIA_PT_PCMU)
			{
				if(g_audio_dec_len[channel] + (stream->len * 2) > sizeof(g_audio_dec_buffer[channel]))
				{
					return -1;
				}
				
				ulaw_expand(stream->len, (char *)stream->data, audio_out_buffer);
				audio_out_len = stream->len * 2;
			}
			else
			{
				void g726_Decode(int index,unsigned char *bitstream,unsigned char *speech,int len);
				g726_Decode(channel,(unsigned char *)stream->data,(unsigned char *)audio_out_buffer,stream->len);
				audio_out_len = stream->len * 8;
			}
			g_audio_dec_len[channel] += audio_out_len;
			stream->media_type = MEDIA_PT_RAWAUDIO;
			stream->data = g_audio_dec_buffer[channel];
			stream->len = g_audio_dec_len[channel];
			/*if(channel == 0 && audio_out_len > 0)
			{
				FILE *fp = fopen("/mnt/ipc.pcm","ab");
				if(fp != NULL)
				{
					printf("audio len:%d\n",audio_out_len);
					fwrite(audio_out_buffer, audio_out_len, 1, fp);
					fclose(fp);
				}
			}*/
			//return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
	
	if(stream->media_type != MEDIA_PT_RAWAUDIO)
	{
		return -1;
	}
	
	/*if(audio_start[channel])
	{
		audio_frame_t sAudioFrame;
		sAudioFrame.channel = channel;
		sAudioFrame.data = stream->data;
		sAudioFrame.len = stream->len;
		sAudioFrame.time_stamp = venc_get_sys_pts(channel, stream->pts);
		sAudioFrame.is_raw_data = 1;
		if(lib_gbl_info.pAudioStreamCB != NULL)
		{
			lib_gbl_info.pAudioStreamCB(&sAudioFrame);
		}
	}
	
	if(lib_gbl_info.last_pre_mode != PREVIEW_CLOSE && !audio_start[TL_AUDIO_CH_TALK_BACK])
	{
		if(channel == audio_out_channel)
		{
			audio_frame_t sAudioFrame;
			sAudioFrame.channel = channel;
			sAudioFrame.data = stream->data;
			sAudioFrame.len = stream->len;
			sAudioFrame.time_stamp = 0;
			sAudioFrame.is_raw_data = 1;
			tl_audio_write_data(&sAudioFrame);
		}
	}*/
	
	int remain = g_audio_dec_len[channel];
	int used = 0;
	while(remain >= 642)
	{
		audio_frame_t sAudioFrame;
		sAudioFrame.channel = channel;
		sAudioFrame.data = (unsigned char *)g_audio_dec_buffer[channel] + used;
		sAudioFrame.len = 642;
		sAudioFrame.time_stamp = venc_get_sys_pts(channel, stream->pts);
		sAudioFrame.is_raw_data = 1;
		if(audio_start[channel])
		{
			if(lib_gbl_info.pAudioStreamCB != NULL)
			{
				lib_gbl_info.pAudioStreamCB(&sAudioFrame);
			}
		}
		if(lib_gbl_info.last_pre_mode != PREVIEW_CLOSE && channel == audio_out_channel && !audio_start[TL_AUDIO_CH_TALK_BACK])
		{
			sAudioFrame.time_stamp = 0;
			tl_audio_write_data(&sAudioFrame);
		}
		remain -= 642;
		used += 642;
	}
	if(remain > 0)
	{
		memmove(g_audio_dec_buffer[channel], (unsigned char *)g_audio_dec_buffer[channel]+used, remain);
	}
	g_audio_dec_len[channel] = remain;
	
	return 0;
}

