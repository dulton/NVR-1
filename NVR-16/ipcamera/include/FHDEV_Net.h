#ifndef _FHDEV_NET_H_
#define _FHDEV_NET_H_

#include "FHDEV_Net_Define.h"


#ifdef WIN32        // win32

#ifdef __cplusplus
#   ifdef FHDEV_NET_EXPORTS
#       ifndef FHNET_API 
#           define FHNET_API extern "C" __declspec (dllexport)
#       endif
#   else
#       ifndef FHNET_API
#           define FHNET_API extern "C" __declspec (dllimport)
#       endif
#   endif
#else    
#   ifdef FHDEV_NET_EXPORTS 
#       ifndef FHNET_API 
#           define FHNET_API __declspec (dllexport)
#       endif
#   else
#       ifndef FHNET_API
#           define FHNET_API __declspec (dllimport)
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

#ifndef FHNET_API
#   ifdef __cplusplus
#       define FHNET_API   extern "C"
#   else
#       define FHNET_API
#   endif
#endif
#ifndef CALLBACK
#   define CALLBACK
#endif
#ifndef CALL_METHOD
#   define CALL_METHOD
#endif

#endif      // endif


typedef void (CALLBACK *fNotifyCallBack)(LPFHNP_Notify_t lpNotify, LPVOID lpUser);
// 当dwBufLen=0时表示结束(录像下载/截图下载/远程录像回放情况下)
typedef void (CALLBACK *fDataCallBack)(DWORD dwPlayHandle, FHNPEN_StreamType_e eType, LPFHNP_FrameHead_t lpFrameHead, char* pBuf, DWORD dwBufLen, LPVOID lpUser);
typedef void (CALLBACK *fSerialDataCallBack)(DWORD dwSerialHandle, char* pRecvDataBuf, DWORD dwBufLen, LPVOID lpUser);
typedef void (CALLBACK *fTalkDataCallBack)(DWORD dwTalkHandle, LPFHNP_AFrameHead_t lpFrameHead, char* pBuf, DWORD dwBufLen, LPVOID lpUser);



FHNET_API BOOL  CALL_METHOD FHDEV_NET_Init();
FHNET_API BOOL  CALL_METHOD FHDEV_NET_Cleanup();

/////////////////////网络连接/////////////////////////////////////////////////////
// dwWaitTime: ms
// dwInterval: ms
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SetConnectTime(DWORD dwWaitTime, DWORD dwTryTimes);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SetRecvTimeOut(DWORD dwRecvTimeOut);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SetReconnect(DWORD dwInterval, BOOL bEnableRecon);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_RegisterDevNotifyFun(fNotifyCallBack pFun, LPVOID lpUser);

///////////////////SDK版本及信息///////////////////////////////////////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_GetSDKVersion();
FHNET_API DWORD CALL_METHOD FHDEV_NET_GetSDKBuildVersion();
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetSDKState(LPFHNP_SDKState_t lpSDKState);

///////////////////错误码///////////////////////////////////////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_GetLastError();
FHNET_API char* CALL_METHOD FHDEV_NET_GetErrorMsg(LPDWORD lpErrorID);

///////////////////登录/注销///////////////////////////////////////////////////////
// sDevIP可以是域名, sBindLocalIP用于多网卡管理时绑定
FHNET_API DWORD CALL_METHOD FHDEV_NET_Login(char* sDevIP, WORD wDevPort, char* sUserName, char* sPassword, LPFHNP_Capacity_t lpCapacity);
FHNET_API DWORD CALL_METHOD FHDEV_NET_LoginEx(char* sDevIP, WORD wDevPort, char* sUserName, char* sPassword, char* sBindLocalIP, WORD wBindLocalPort, LPFHNP_Capacity_t lpCapacity);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_Logout(DWORD dwUserID);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_TimeConvert(DWORD dwUserID, DWORD dwSecond, FHNP_Time_t* pTime);

///////////////////实时预览///////////////////////////////////////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_StartRealPlay(DWORD dwUserID, LPFHNP_Preview_t lpPreview, FHNPEN_StreamType_e eType, fDataCallBack pFun, LPVOID lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_StopRealPlay(DWORD dwRealPlayHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_RegisterRealDataFun(DWORD dwRealPlayHandle, FHNPEN_StreamType_e eType, fDataCallBack pFun, LPVOID lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_OpenRealAudio(DWORD dwRealPlayHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_CloseRealAudio(DWORD dwRealPlayHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetRealAudioState(DWORD dwRealPlayHandle, BOOL* pIsOped);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_DevMakeKeyFrame(DWORD dwUserID, BYTE btChannel, BYTE btEncID);

FHNET_API BOOL  CALL_METHOD FHDEV_Net_GetDevCapacity(DWORD dwUserID, LPFHNP_Capacity_t lpCapacity);

///////////////////用户管理///////////////////////////////////////////////////////
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetUserAuth(DWORD dwUserID, FHNPEN_UserGroup_e* pUserGroup);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetUserList(DWORD dwUserID, FHNP_User_t stUser[32]);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_AddUser(DWORD dwUserID, LPFHNP_User_t lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_ModifyUser(DWORD dwUserID, LPFHNP_User_t lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_DeleteUser(DWORD dwUserID, char sUserName[FHNP_MACRO_NAMELEN_MAX]);

///////////////////设备维护///////////////////////////////////////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_StartUpgrade(DWORD dwUserID, char* pBuf, DWORD dwBufLen, BOOL bForce);
FHNET_API DWORD CALL_METHOD FHDEV_NET_StartUpgradeEx(DWORD dwUserID, char* pUpgradeFilePath, BOOL bForce);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_StopUpgrade(DWORD dwUpgradeHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetUpgradeState(DWORD dwUpgradeHandle, LPDWORD lpProgress, FHNPEN_UpgradeState_e* pStatus);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_ExportConfig(DWORD dwUserID, char* pBuf, DWORD dwBufLen, LPDWORD lpReturnLen);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_ImportConfig(DWORD dwUserID, char* pBuf, DWORD dwBufLen);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_ExportConfigEx(DWORD dwUserID, char* sConfigFileName);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_ImportConfigEx(DWORD dwUserID, char* sConfigFileName);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_ResetDev(DWORD dwUserID, BOOL bKeepWifiMode);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_RebootDev(DWORD dwUserID);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_ShutDownDev(DWORD dwUserID);

///////////////////日志管理///////////////////////////////////////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_SearchLog(DWORD dwUserID, LPFHNP_LogSearch_t lpLogSearch);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SearchNextLog(DWORD dwLogSearchHandle, LPFHNP_Log_t lpLog);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_CloseSearchLog(DWORD dwLogSearchHandle);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_ClearLog(DWORD dwUserID);

///////////////////布防///////////////////////////////////////////////////////
FHNET_API BOOL  CALL_METHOD FHDEV_NET_ManualAlarm(DWORD dwUserID, BOOL bEnable);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetManualAlarmState(DWORD dwUserID, BOOL* pIsEnable);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_Defence(DWORD dwUserID, BOOL bEnable);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetDefenceState(DWORD dwUserID, BOOL* pIsEnable);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_ClearAlarm(DWORD dwUserID);

///////////////////透明通道///////////////////////////////////////////////////////
// dwSerialPort=0(232) dwSerialPort=1(485)
FHNET_API DWORD CALL_METHOD FHDEV_NET_StartSerial(DWORD dwUserID, DWORD dwSerialPort, DWORD dwSerialIndex, fSerialDataCallBack pFun, LPVOID lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SendSerial(DWORD dwSerialHandle, char* pSendBuf, DWORD dwBufLen);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_StopSerial(DWORD dwSerialHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SendToSerialPort(DWORD dwUserID, DWORD dwSerialPort, DWORD dwSerialIndex, char* pSendBuf, DWORD dwBufLen);

///////////////////配置///////////////////////////////////////////////////////
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetDevConfig(DWORD dwUserID, DWORD dwCfgType, BYTE btChannel, LPVOID lpOutBuf, DWORD dwOutBufLen, LPDWORD lpReturnLen);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SetDevConfig(DWORD dwUserID, DWORD dwCfgType, BYTE btChannel, LPVOID lpInBuf, DWORD dwInBufLen);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetEncodeVideo(DWORD dwUserID, BYTE btChannel, BYTE btEncID, LPFHNP_EncodeVideo_t lpEncVideo);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SetEncodeVideo(DWORD dwUserID, BYTE btChannel, BYTE btEncID, LPFHNP_EncodeVideo_t lpEncVideo);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetEncodeWaterMark(DWORD dwUserID, BYTE btChannel, BYTE btEncID, LPFHNP_EncodeWaterMark_t lpEncWaterMark);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SetEncodeWaterMark(DWORD dwUserID, BYTE btChannel, BYTE btEncID, LPFHNP_EncodeWaterMark_t lpEncWaterMark);

// pdwStatus: 0失败, 1进行中, 2成功
FHNET_API BOOL  CALL_METHOD FHDEV_NET_TestWifiConfig(DWORD dwUserID, LPFHNP_WifiConfig_t lpWifiCfg, DWORD* pdwStatus);

///////////////////////远程回放///////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_StartPlayBack(DWORD dwUserID, LPFHNP_Playback_t lpPlayback, FHNPEN_StreamType_e eType, fDataCallBack pFun, LPVOID lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_StopPlayBack(DWORD dwPlayBackHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_RegisterPlayBackDataFun(DWORD dwPlayBackHandle, FHNPEN_StreamType_e eType, fDataCallBack pFun, LPVOID lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_PlayBackControl(DWORD dwPlayBackHandle, FHNPEN_PlayCtrl_e eCtrl, FHNPEN_PlaySpeed_e eSpeed);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_OpenPlayBackAudio(DWORD dwPlayBackHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_ClosePlayBackAudio(DWORD dwPlayBackHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_JumpPlayBack(DWORD dwPlayBackHandle, DWORD64 dw64Time);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_StartDevRecord(DWORD dwUserID, BYTE btChannel);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_StopDevRecord(DWORD dwUserID, BYTE btChannel);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetDevRecordState(DWORD dwUserID, BYTE btChannel, BOOL* pIsRecording);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_EnableDevRecAudio(DWORD dwUserID, BYTE btChannel, BOOL bEnable);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetDevRecAudioState(DWORD dwUserID, BYTE btChannel, BOOL* pIsEnable);

// btStore: 是否存盘
FHNET_API BOOL  CALL_METHOD FHDEV_NET_DevShot(DWORD dwUserID, BYTE btChannel, BYTE btStore, LPVOID lpBuf, LPDWORD lpBufLen);

///////////////////语音对讲///////////////////////////////////////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_StartTalk(DWORD dwUserID, DWORD dwTalkChan, fTalkDataCallBack fFun, LPVOID lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_StopTalk(DWORD dwTalkHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_TalkSendData(DWORD dwTalkHandle, LPFHNP_AFrameHead_t lpFrameHead, char* pSendBuf, DWORD dwBufLen);

///////////////////////录像相关///////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_SearchRecord(DWORD dwUserID, LPFHNP_RecSearch_t lpRecSearch);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SearchNextRecord(DWORD dwRecSearchHandle, LPFHNP_Record_t lpRecord);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_CloseSearchRecord(DWORD dwRecSearchHandle);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_LockRecord(DWORD dwUserID, DWORD dwCount, FHNP_Record_t stRecord[]);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_UnlockRecord(DWORD dwUserID, DWORD dwCount, FHNP_Record_t stRecord[]);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_DeleteRecord(DWORD dwUserID, DWORD dwCount, FHNP_Record_t stRecord[]);

FHNET_API DWORD CALL_METHOD FHDEV_NET_CreateRecDownload(DWORD dwUserID, LPFHNP_Record_t lpRecord, FHNPEN_StreamType_e eType, fDataCallBack pFun, LPVOID lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_DestoryRecDownload(DWORD dwRecDownloadHandle);

///////////////////////截图相关///////////////////////
FHNET_API DWORD CALL_METHOD FHDEV_NET_SearchPicture(DWORD dwUserID, LPFHNP_PicSearch_t lpPicSearch);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_SearchNextPicture(DWORD dwPicSearchHandle, LPFHNP_Picture_t lpPicture);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_CloseSearchPicture(DWORD dwPicSearchHandle);

FHNET_API BOOL  CALL_METHOD FHDEV_NET_LockPicture(DWORD dwUserID, DWORD dwCount, FHNP_Picture_t stPicture[]);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_UnlockPicture(DWORD dwUserID, DWORD dwCount, FHNP_Picture_t stPicture[]);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_DeletePicture(DWORD dwUserID, DWORD dwCount, FHNP_Picture_t stPicture[]);

FHNET_API DWORD CALL_METHOD FHDEV_NET_CreatePicDownload(DWORD dwUserID, LPFHNP_Picture_t lpPicture, fDataCallBack pFun, LPVOID lpUser);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_DestoryPicDownload(DWORD dwPicDownloadHandle);

///////////////////////SDCard存储管理///////////////////////
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetSDCardInfo(DWORD dwUserID, LPFHNP_SDCardInfo_t lpSDCard);
FHNET_API DWORD CALL_METHOD FHDEV_NET_StartSDCardFormat(DWORD dwUserID, FHNPEN_FormatType_e eFormat);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_StopSDCardFormat(DWORD dwFormatHandle);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_GetSDCardFormatState(DWORD dwFormatHandle, LPDWORD lpProgress, FHNPEN_DurationState_e* pState);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_LoadSDCard(DWORD dwUserID);
FHNET_API BOOL  CALL_METHOD FHDEV_NET_UnloadSDCard(DWORD dwUserID);


#endif
