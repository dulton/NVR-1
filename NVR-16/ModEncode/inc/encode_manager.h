#ifndef _ENCODE_MANAGER_H_
#define _ENCODE_MANAGER_H_

#include "mod_encode.h"
#include "lib_common.h"
#include "lib_venc.h"
#include "lib_venc_osd.h"
#include "lib_audio.h"
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	PFNEncodeRequest pfnEncodeProc;
	u32 nContent;
	u8 nStart;
	u8 nPause;
	u8 nRun;
	u8 nRestart;
	u8 nReserve[5];
} SChnInfo;

typedef struct
{
	SEncodeParam sEncodeParam;
	SChnInfo sChnInfo;
	pthread_mutex_t pmLock;
} SChnObj;

typedef struct
{
	u8 nChnNum;
	u32 nMaxFrameSize;
	SChnObj* psChnObj;
	u8 nIsEncoding;
	//next two para just for 9616
	u8 nMasterWorking;//master chip is encoding(chn:1~8)
	u8 nSlaverWorking;//slaver chip is encoding(chn:9~16)
	//end
	sem_t nSemVencReadOrStart;//tl_venc_read和tl_venc_start同时调用会引起段错误，库有问题
} SChnManage;

//#define bufSize 20*228*2

typedef struct
{
	u8 nChnNum;
	s32* fdOsdChn;
	venc_osd_atr_t* voaChn;
	s32* fdOsdTime;
	venc_osd_atr_t* voaTime;
	s32* fdOsdExt1;
	venc_osd_atr_t* voaExt1;
	EMVIDEORESOLUTION* emResoRef;
	EMVIDEORESOLUTION* emResoCur;
	PFNEncodeTextToOsd pfnOsdCB;
	
	//csp modify 20130316
	//u8* osdBuf;
	s8** ChnName;
	
	pthread_mutex_t pmLock;
	
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	s32* fdOsdChnSub;
	s32* fdOsdTimeSub;
	EMVIDEORESOLUTION* emResoCurSub;
	#endif
	
	//csp modify 20130423
	#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	s32* fdOsdChnThird;
	s32* fdOsdTimeThird;
	//EMVIDEORESOLUTION* emResoCurThird;
	#endif
}SOsdObj;

typedef struct
{
	SChnManage sChnMainManage;
	SChnManage sChnSubManage;
	SChnManage sChnSnapManage;
	SChnManage sChnAudioManage;
	SChnManage sChnTalkManage;
	SOsdObj sOsdManage;
	EMENCODEBOARDTYPE emBoardType;//主板类型
	
	//csp modify 20130106
	PFNEncodeEXCEPTION EncExceptCB;
	
	//csp modify 20130316
	u8 nVideoStandard;//视频制式(EMBIZVIDSTANDARD)
	
	//csp modify 20130423
	SChnManage sChnThirdManage;
} SEncodeManage;

void EncodeStatusDealFxn(void* arg);

#if 1//NVR used
int DealMainStream(venc_stream_s *stream);
int DealSubStream(venc_stream_s *stream);
int DealThirdStream(venc_stream_s *stream);
int DealAudioStream(audio_frame_t *stream);
#else
void* MainStreamCap(void* arg);
void* SubStreamCap(void* arg);
void* AudioStreamCap(void* arg);
void* MainStreamCapSlaver(void* arg);
void* SubStreamCapSlaver(void* arg);

//csp modify 20130423
void* ThirdStreamCap(void* arg);
#endif

//csp modify 20130423
s32 InitThirdStreamObj(SChnManage* psChnThirdManage);
s32 CreateThirdStreamObj(SChnManage* psChnThirdManage, u8 nChnNum, u32 nMaxFrameSize);

s32 CreateMainStreamObj(SChnManage* psChnMainManage, u8 nChnNum, u32 nMaxFrameSize);
s32 DestroyMainStreamObj(SChnManage* psChnMainManage);
s32 CreateSubStreamObj(SChnManage* psChnSubManage, u8 nChnNum, u32 nMaxFrameSize);
s32 DestroySubStreamObj(SChnManage* psChnSubManage);
s32 CreateSnapObj(SChnManage* psChnSnapManage, u8 nChnNum, u32 nMaxFrameSize);
s32 DestroySnapObj(SChnManage* psChnSnapManage);
s32 CreateAudioObj(SChnManage* psChnAudioManage, u8 nChnNum, u32 nMaxFrameSize);
s32 CreateTalkObj(SChnManage* psChnAudioManage, u8 nChnNum, u32 nMaxFrameSize);
s32 DestroyTalkObj(SChnManage* psChnAudioManage);
s32 DestroyAudioObj(SChnManage* psChnAudioManage);
s32 CreateOsdObj(SOsdObj* psOsdObj, u8 nChnNum);
s32 DestroyOsdObj(SOsdObj* psOsdObj);


s32 InitMainStreamObj(SChnManage* psChnMainManage);
s32 InitSubStreamObj(SChnManage* psChnMainManage);
s32 InitSnapObj(SChnManage* psChnMainManage);
s32 InitAudioStreamObj(SChnManage* psChnMainManage);
s32 InitTalkStreamObj(SChnManage* psChnMainManage);

s32 MainStreamEncodeSetParam(u8 nChn, SEncodeParam* psEncodeParam);
s32 SubStreamEncodeSetParam(u8 nChn, SEncodeParam* psEncodeParam);
s32 SnapSetParam(u8 nChn, SEncodeParam* psEncodeParam);
s32 AudioEncodeSetParam(u8 nChn, SEncodeParam* psEncodeParam);
s32 EncodePause(u8 nChn, SChnObj* psChnObj);
s32 EncodeResume(u8 nChn, SChnObj* psChnObj);

s32 MainStreamEncodeGetParam(SChnObj* psMainStreamObj, SEncodeParam* psEncodeParam);
s32 SubStreamEncodeGetParam(SChnObj* psSubStreamObj, SEncodeParam* psEncodeParam);
s32 SnapGetParam(SChnObj* psSnapObj, SEncodeParam* psEncodeParam);
s32 AudioEncodeGetParam(SChnObj* psAudioObj, SEncodeParam* psEncodeParam);

s32 GetChnObj(u8 nChn, EMCHNTYPE emChnType, SChnObj** ppsChnObj);
u8 CheckEncodeParamChanged(EMCHNTYPE emChnType, SEncodeParam* psOldEncodeParam, SEncodeParam* psNewEncodeParam);

#ifdef __cplusplus
}
#endif


#endif

