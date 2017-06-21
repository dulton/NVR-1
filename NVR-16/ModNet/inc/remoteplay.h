#ifndef __REMOTEPLAY_H_
#define __REMOTEPLAY_H_

#include "common_basetypes.h"
#include "netcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define

// match old code macros
// delete them later if not need
// 
#define NEW_REMOTE_PLAYBACK
#define USE_AUDIO_PCMU
//===

// module macros
// 
#define RMTPLAY_TX_ERR_DELAY	(50*1000)	// usleep when network transfer fault
#define RMTPLAY_PROG_SEG_MAX	10			

// typdef 

typedef enum
{
	EM_RMTPLAY_TOO_MANY_REQUEST,
	EM_RMTPLAY_CREATE_THREAD_FAIL,
	
} EM_REMOTEPLAY_ERROR;

typedef enum
{
	EM_RMTPLAY_STATE_OK = 0,
	EM_RMTPLAY_STATE_CONNLOST,
	EM_RMTPLAY_STATE_PAUSE,
	EM_RMTPLAY_STATE_STOP,
} EM_REMOTEPLAY_STATE;

typedef enum
{
	EM_RMTPLAY_CHK_STEPFWD = 0,
	EM_RMTPLAY_CHK_STOP,
	EM_RMTPLAY_CHK_CONTINUE,
} EM_REMOTEPLAY_CHECKDO;


typedef enum
{
	EM_RMTPLAY_SKIP_FWD = 0,
	EM_RMTPLAY_SKIP_BACK,
} EM_REMOTEPLAY_SKIP;


// extern ..

s32 remotePlay_Init( 
	PRemotePlayCB pRpCB,
	u8 nVideMediaType,
	u8 nAudioMediaType,
	u8 nVideFrameRate,
	u8 nAudioFrameRate,
	u8 nAudioMode,
	u8 nChnMax
);

s32 remotePlay_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId);

s32 remoteTimePlay_Request(u32 nChMsk, ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId);

#ifdef __cplusplus
}
#endif


#endif // __A_H_

