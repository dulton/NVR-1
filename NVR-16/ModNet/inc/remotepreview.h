#ifndef __REMOTEPREVIEW_H_
#define __REMOTEPREVIEW_H_

#include "netcomm.h"
#include "mobile.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define 
#define VOIP_IDLE_WAIT	1 // seconds
			
// typdef 
typedef struct _sVoipCtrl
{
	BOOL	bUsed;
	int 	sockfd;
	u32     nLinkId;
} SVIOPCtrl;

typedef struct _sVoipFrame
{
	u32 nMediaType;
	u32 nAudioMode;
	u32 nFrameRate;
	u32 nFrameID;
	u32 nTStamp;
	u32 nSize;
	u8* pData;
} SVOIPFrame;

// extern ..

s32 remotePreview_Init(
	    u32 yFactor,
		u16 nStreamTcpNum,
		u16 nVidTcpNum,
		u16 nAudTcpNum,
		u8	nVoipMode,
		u8	nAudioMdType,
		u8	nVideoMdType,
		PStreamRequestCB pFxn,
		PRequestKeyFrameCB pReqKeyCB,
		PNetCommVOIPOut pVOIP,
		PNetCommVOIPIn pVOIPIn
);

s32 remotePreview_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nId);
s32 remoteVoip_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nId);

s32 RemoteVoipSendData( SVOIPFrame * pFrame );
u8  RemoteVoip_GetStatus( void );

#ifdef __cplusplus
}
#endif

#endif // __REMOTEPREVIEW_H_

