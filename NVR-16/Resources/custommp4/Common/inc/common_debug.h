#ifndef _COMMON_DEBUG_H_
#define _COMMON_DEBUG_H_

#include "common_basetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PUBPRT(msg) //printf("file:%s fn:%s line:%d\r\n%s\r\n", __FILE__, __FUNCTION__, __LINE__, msg)

#ifdef BIZDBG
    #define BIZPRT(msg) printf("BizDebug:file:%s fn:%s line:%d\r\n%s\r\n", __FILE__, __FUNCTION__, __LINE__, msg)
#else
    #define BIZPRT(msg)
#endif

#ifdef APPDBG
    #define APPPRT(msg) printf("APPDebug:file:%s fn:%s line:%d\r\n%s\r\n", __FILE__, __FUNCTION__, __LINE__, msg)
#else
    #define APPPRT(msg) 
#endif

#ifdef __cplusplus
}
#endif

#endif //_COMMON_DEBUG_H_



