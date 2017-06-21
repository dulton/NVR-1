#ifndef __UTILS_H
#define __UTILS_H

#define LINUX

#include "common_basetypes.h"
#include "common_geometric.h"
#include "common_mutex.h"
#include "common_semaphore.h"
#include "common_thread.h"
#include "common_debug.h"


#ifdef LINUX

#include <unistd.h>
#include <pthread.h>
typedef pthread_t THREADHANDLE;
typedef void* (*LINUXFUNC)(void*);

#else  //WIN32

#include <wtypes.h>
typedef HANDLE THREADHANDLE;

#endif

//#define BOOL  u8
#ifndef BOOL
#define BOOL int
#endif

#define TRUE  1
#define FALSE 0

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
//#define ABS(a) ((a)>0?(a):(-a))

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************/
/*                              线程相关接口                                 */

#define THREAD_PRI_LOW			100
#define THREAD_PRI_MID			80
#define THREAD_PRI_HIGH			60

#define PRI_TIMER				THREAD_PRI_MID
#define PRI_UI					THREAD_PRI_HIGH
#define PRI_CP					THREAD_PRI_HIGH
#define PRI_CPCHECK				THREAD_PRI_LOW
#define PRI_CAPTURE				THREAD_PRI_HIGH
#define PRI_DISPLAY				THREAD_PRI_HIGH
#define PRI_ENCODE				THREAD_PRI_HIGH
#define PRI_DECODE				THREAD_PRI_HIGH
#define PRI_RECORD				THREAD_PRI_HIGH
#define PRI_MEDIASND			THREAD_PRI_HIGH
#define PRI_MEDIARCV			THREAD_PRI_HIGH
#define PRI_AUDIO				THREAD_PRI_MID
#define PRI_ALARM				THREAD_PRI_MID
#define NTP_CLIENT				THREAD_PRI_MID

#define THREAD_STKSIZE_DEFAULT	(8<<20)

#define STKSIZE_TIMER			(256<<10)
#define STKSIZE_UI				(4<<20)
#define STKSIZE_CP				(512<<10)
#define STKSIZE_CPCHECK			(256<<10)
#define STKSIZE_CAPTURE			(1<<20)
#define STKSIZE_DISPLAY			(1<<20)
#define STKSIZE_ENCODE			(1<<20)
#define STKSIZE_DECODE			(1<<20)
#define STKSIZE_RECORD			(2<<20)
#define STKSIZE_MEDIASND		(512<<10)
#define STKSIZE_MEDIARCV		(512<<10)
#define STKSIZE_AUDIO			(512<<10)
#define STKSIZE_ALARM			(256<<10)
#define STKSIZE_NTPCLIENT		(256<<10)

#ifdef LINUX
THREADHANDLE TL_CreateThread(LINUXFUNC pvTaskEntry, 
									char* szName, 
									u8 byPriority, 
									u32 dwStacksize, 
									u32 dwParam, 
									u16 wFlag, 
									u32 *pdwTaskID);
#else
THREADHANDLE TL_CreateThread(void* pvTaskEntry, 
									char* szName, 
									u8 byPriority, 
									u32 dwStacksize, 
									u32 dwParam, 
									u16 wFlag, 
									u32 *pdwTaskID);
#endif

BOOL TL_ExitThread();
BOOL TL_TerminateThread(THREADHANDLE hTask);


#ifdef __cplusplus
}
#endif


#endif // __UTILS_H

