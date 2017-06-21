#ifndef __CMDPROC_H_
#define __CMDPROC_H_

#include "common_basetypes.h"
#include "netcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define 
// typdef 

// extern ..

u16 DealCommand(CPHandle cph,u16 event,
		u8 *pbyMsgBuf,int msgLen,u8 *pbyAckBuf,int *pAckLen,void* pContext);

s32 cmdProc_GetLoginUser(SNetUser* pUser);
s32 cmdProc_AddNetUser(SNetUser* pUser);
s32 cmdProc_DelNetUser(SNetUser* pUser);

#ifdef __cplusplus
}
#endif


#endif // __A_H_

