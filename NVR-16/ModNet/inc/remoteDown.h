#ifndef __REMOTEDWON_H_
#define __REMOTEDWON_H_

#include "common_basetypes.h"
#include "netcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define 
// typdef 

// extern ..

s32 remoteDownload_Init( PRemotePlayCB pCB );
s32 remoteDownload_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId);

#ifdef __cplusplus
}
#endif


#endif // __REMOTEDWON_H_

