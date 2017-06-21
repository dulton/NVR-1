#ifndef _FHDEV_DISCOVER_H_
#define _FHDEV_DISCOVER_H_

#include "FHDEV_Discover_Define.h"

#ifdef WIN32        // win32

#ifdef __cplusplus
#   ifdef FHDEV_DISCOVER_EXPORTS
#       ifndef FHDS_API 
#           define FHDS_API extern "C" __declspec (dllexport)
#       endif
#   else
#       ifndef FHDS_API
#           define FHDS_API extern "C" __declspec (dllimport)
#       endif
#   endif
#else    
#   ifdef FHDEV_DISCOVER_EXPORTS 
#       ifndef FHDS_API 
#           define FHDS_API __declspec (dllexport)
#       endif
#   else
#       ifndef FHDS_API
#           define FHDS_API __declspec (dllimport)
#       endif
#   endif
#endif

#ifndef CALLBACK
#   define CALLBACK    __stdcall
#endif
#ifndef CALL_METHOD
#   define CALL_METHOD __stdcall   // __cdecl
#endif

#else               // linux

#ifndef FHDS_API
#   ifdef __cplusplus
#       define FHDS_API   extern "C"
#   else
#       define FHDS_API
#   endif
#endif
#ifndef CALLBACK
#   define CALLBACK
#endif
#ifndef CALL_METHOD
#   define CALL_METHOD
#endif

#endif      // endif


typedef void (CALLBACK *fDevStateCallBack) (LPFHDS_DevState_t lpDevState, LPVOID lpUser);

FHDS_API BOOL  CALL_METHOD FHDEV_DS_Init();
FHDS_API BOOL  CALL_METHOD FHDEV_DS_InitEx(char* sBindLocalIP);
FHDS_API BOOL  CALL_METHOD FHDEV_DS_Cleanup();
FHDS_API DWORD CALL_METHOD FHDEV_DS_GetSDKVersion();
FHDS_API DWORD CALL_METHOD FHDEV_DS_GetSDKBuildVersion();

FHDS_API BOOL  CALL_METHOD FHDEV_DS_RegisterDevStateFun(FHNPEN_DevType_e eDevType, fDevStateCallBack pFun, LPVOID lpUser);

FHDS_API BOOL  CALL_METHOD FHDEV_DS_SearchDevice(DWORD dwWaitTime);

FHDS_API DWORD CALL_METHOD FHDEV_DS_FindDevState(FHNPEN_DevType_e eDevType);
FHDS_API BOOL  CALL_METHOD FHDEV_DS_FindNextDevState(DWORD dwFindDevHandle, LPFHDS_DevState_t lpDevState);
FHDS_API BOOL  CALL_METHOD FHDEV_DS_CloseFindDevState(DWORD dwFindDevHandle);

FHDS_API BOOL  CALL_METHOD FHDEV_DS_ModifyNetAddr(LPFHDS_ModifyNetAddr_t lpModifyNetAddr, DWORD dwWaitTime);



#endif
