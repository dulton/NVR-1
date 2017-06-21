#ifndef __REMOTEUPDATE_H_
#define __REMOTEUPDATE_H_

#include "common_basetypes.h"
#include "netcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define 

#define UPDATE_IGNORE_UBOOT
#define CHECK_UPDATE
#define CFG_SAVE_IN_FLASH

// typdef 

// extern ..


s32 remoteUpdate_Init(void* pDMgr);

s32 remoteUpdate_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId);
void remoteUpdate_ProgNotify(u8 pos);
s32 remoteFormat_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId);

#ifdef __cplusplus
}
#endif


#endif // __REMOTEUPDATE_H_



