#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include "common_basetypes.h"
#include "common_debug.h"

typedef void* HDDHDR;

#ifdef __cplusplus
extern "C" {
#endif

s32 PublicInit(void);
HDDHDR PublicGetHddManage(void);
u32 PublicGetTimeStamp(void);

//yaogang modify for bad disk
void PublicDealBadDisk(u8 disk_logic_idx);

s32 PublicIniGetString(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT s8* strOutValue, s32 nMaxStrLen);
s32 PublicIniGetValue(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT s32* nOutValue);
s32 PublicIniGetValueU64(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT u64* nOutValue);
s32 PublicIniGetArrayStrings(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT s8* strOutValue[], PARAOUT s32* nItems, s32 nTableLen,s32 nMaxStrLen);
s32 PublicIniGetArrayValues(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT s32* nOutValue[], PARAOUT s32* nItems, s32 nTableLen);
s32 PublicIniGetArrayValuesU64(s8* strFilePath, s8* strSection, s8* strItem, PARAOUT u64** nOutValue, PARAOUT s32* nItems, s32 nTableLen);
s32 PublicIniGetArrayString(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, PARAOUT s8* strOutValue, s32 nMaxStrLen);
s32 PublicIniGetArrayValue(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, PARAOUT s32* nOutValue);
s32 PublicIniGetArrayValueU64(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, PARAOUT u64* nOutValue);

s32 PublicIniSetString(s8* strFilePath, s8* strSection, s8* strItem, s8* strNewValue);
s32 PublicIniSetValue(s8* strFilePath, s8* strSection, s8* strItem, s32 nNewValue);
s32 PublicIniSetValueU64(s8* strFilePath, s8* strSection, s8* strItem, u64 nNewValue);
s32 PublicIniSetArrayString(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, s8* strNewtValue);
s32 PublicIniSetArrayValue(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, s32 nNewValue);
s32 PublicIniSetArrayValueU64(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, u64 nNewValue);

/***********************************************************/


#ifndef WIN32

#include <unistd.h>
#include <pthread.h>
typedef pthread_t PUBLICTHREADHANDLE;
typedef void* (*PUBLICLINUXFUNC)(void*);

#else  //WIN32

#include <wtypes.h>
typedef HANDLE PUBLICTHREADHANDLE;

#endif

/*                              线程相关接口                                 */

#define PUBLIC_THREAD_PRI_LOW 100
#define PUBLIC_THREAD_PRI_MID 64
#define PUBLIC_THREAD_PRI_HIGH 25

#define PUBLIC_PRI_TIMER PUBLIC_THREAD_PRI_MID
#define PUBLIC_PRI_UI 25
#define PUBLIC_PRI_CP PUBLIC_THREAD_PRI_HIGH
#define PUBLIC_PRI_CPCHECK PUBLIC_THREAD_PRI_LOW
#define PUBLIC_PRI_CAPTURE 20
#define PUBLIC_PRI_DISPLAY PUBLIC_THREAD_PRI_HIGH
#define PUBLIC_PRI_ENCODE PUBLIC_THREAD_PRI_HIGH
#define PUBLIC_PRI_DECODE PUBLIC_THREAD_PRI_HIGH
#define PUBLIC_PRI_RECORD PUBLIC_THREAD_PRI_HIGH
#define PUBLIC_PRI_MEDIASND 39
#define PUBLIC_PRI_MEDIARCV PUBLIC_THREAD_PRI_HIGH
#define PUBLIC_PRI_AUDIO 20
#define PUBLIC_PRI_ALARM PUBLIC_THREAD_PRI_MID
#define PUBLIC_PRI_NTP_CLIENT PUBLIC_THREAD_PRI_MID

#define PUBLIC_STKSIZE_DEFAULT	(8<<20)

#define PUBLIC_STKSIZE_TIMER (256<<10)
#define PUBLIC_STKSIZE_UI (4<<20)
#define PUBLIC_STKSIZE_CP (512<<10)
#define PUBLIC_STKSIZE_CPCHECK	 (256<<10)
#define PUBLIC_STKSIZE_CAPTURE (1<<20)
#define PUBLIC_STKSIZE_CAPTURE_MINOR (512<<20)
#define PUBLIC_STKSIZE_DISPLAY (1<<20)
#define PUBLIC_STKSIZE_ENCODE (1<<20)
#define PUBLIC_STKSIZE_DECODE (1<<20)
#define PUBLIC_STKSIZE_RECORD (2<<20)
#define PUBLIC_STKSIZE_MEDIASND (512<<10)
#define PUBLIC_STKSIZE_MEDIARCV (512<<10)
#define PUBLIC_STKSIZE_AUDIO (256<<10)
#define PUBLIC_STKSIZE_ALARM (256<<10)
#define PUBLIC_STKSIZE_NTPCLIENT (256<<10)

#ifndef WIN32
PUBLICTHREADHANDLE PublicCreateThread(PUBLICLINUXFUNC pvTaskEntry, 
									char* szName, 
									u8 byPriority, 
									u32 dwStacksize, 
									u32 dwParam, 
									u16 wFlag, 
									u32 *pdwTaskID);
#else
PUBLICTHREADHANDLE PublicCreateThread(void* pvTaskEntry, 
									char* szName, 
									u8 byPriority, 
									u32 dwStacksize, 
									u32 dwParam, 
									u16 wFlag, 
									u32 *pdwTaskID);
#endif

s32 PublicExitThread();
s32 PublicTerminateThread(PUBLICTHREADHANDLE hTask);

#ifdef __cplusplus
}
#endif

#endif

