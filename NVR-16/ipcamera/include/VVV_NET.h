#ifndef __VVV_NET_H__
#define __VVV_NET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif 

#define LINUX

#if defined(WIN32)

#if defined(VVV_NET_DLL_EXPORTS)
#define VVV_NET_API      __declspec( dllexport )
#define VVV_NET_APICALL  __stdcall

#elif	defined(VVV_NET_LIB)
#define VVV_NET_API      
#define VVV_NET_APICALL  

#else 
#define VVV_NET_API      __declspec( dllimport )
#define VVV_NET_APICALL  __stdcall
#error "win32 platform!"
#endif
#elif defined(LINUX)
#define VVV_NET_API
#define VVV_NET_APICALL
#else
#error "Please specify a design-in platform!"
#endif

#include "VVV_NET_define.h"



/************************************************************************/
/* 媒体函数区                                                           */
/************************************************************************/

/* 初始化和去初始化只需调用一次  */
VVV_NET_API int VVV_NET_APICALL VVV_NET_Init();

VVV_NET_API int VVV_NET_APICALL VVV_NET_DeInit();

/*  用户登录 */
VVV_NET_API int VVV_NET_APICALL VVV_NET_Login(unsigned int* pu32DevHandle, /*返回设备句柄,后续所有设备操作都基于该设备句柄进行*/
                 const char* psUsername,     /*输入用户名*/
                 const char* psPassword,     /*输入密码*/
                 const char* psHost,         /*服务器IP*/
                 unsigned short u16Port,     /*服务器端口号*/
                 VVV_NET_PROTOCOL_S stNetProtocol, /*网络协议*/
                 int s32ConnectTimeout,/*网络连接超时时间，默认超时是5秒，单位是毫秒*/
                 VVV_ON_EVENT_CALLBACK cbEventCallBack,/*事件回调，主要为网络连接状态*/
                 userpassword_s *pUserAuth,  /*返回当前用户权限*/
                 void* pUserData);           /*用户数据，会从事件回调函数中返回出来*/


/* 用户登出*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_Logout(unsigned int u32DevHandle);


/* 设置网络超时时间*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_SetTimeout(unsigned int u32DevHandle,
                      int s32ConnectTimeout,/*网络连接超时时间，默认超时是5秒，单位是毫秒*/
                      int s32TransTimeout); /*网络传输超时时间，默认超时是10秒，单位是毫秒*/


/* 开始流传输 */
VVV_NET_API int VVV_NET_APICALL VVV_NET_StartStream(unsigned int *pu32ChnHandle,/*返回通道句柄，后续所有通道操作都基于该通道句柄进行*/
					   unsigned int u32DevHandle,/*输入设备句柄*/
                       unsigned int chn,/*通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道*/
                       unsigned int u32StreamType,/*流数据类型,详见VVV_STREAM_TYPE_E
                                                              0x01:VVV_STREAM_TYPE_VIDEO,
                                                              0x02:VVV_STREAM_TYPE_AUDIO,
                                                              0x03:VVV_STREAM_VIDEO_AUDIO,
                                                              0x04:VVV_STREAM_MD_ONLY,
                                                              0x08:VVV_STREAM_SENSORIN_ONLY,
                                                              0x10:VVV_STREAM_SHELTER_ONLY,
                                                              0x20:VVV_STREAM_VIDEO_LOSS_ONLY,
                                                              0x40:VVV_STREAM_DISK_ERRO_ONLY,
                                                              0xFFF:VVV_STREAM_ALL*/
                       unsigned int u32StreamFlag,/*主次码流,0:连接主码流,1:次码流,2:第三码流,3:第四码流*/
                       VVV_STREAM_INFO_S *pstruStreamInfo,/*返回码流信息*/
                       VVV_ON_STREAM_CALLBACK cbStreamCallBack,/*码流回调*/
                       VVV_ON_DATA_CALLBACK cbDataCallBack,/*告警数据回调*/
                       void* pUserData);/*用户数据*/

/* 开始流传输 */
VVV_NET_API int VVV_NET_APICALL VVV_NET_StartStream_EX(unsigned int *pu32ChnHandle,/*返回通道句柄，后续所有通道操作都基于该通道句柄进行*/
					   unsigned int u32DevHandle,/*输入设备句柄*/
                       unsigned int chn,/*通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道*/
                       unsigned int u32StreamType,/*流数据类型,详见VVV_STREAM_TYPE_E
                                                              0x01:VVV_STREAM_VIDEO_ONLY,
                                                              0x02:VVV_STREAM_AUDIO_ONLY,
                                                              0x03:VVV_STREAM_VIDEO_AUDIO,
                                                              0x04:VVV_STREAM_MD_ONLY,
                                                              0x08:VVV_STREAM_SENSORIN_ONLY,
                                                              0x10:VVV_STREAM_SHELTER_ONLY,
                                                              0x20:VVV_STREAM_VIDEO_LOSS_ONLY,
                                                              0x40:VVV_STREAM_DISK_ERRO_ONLY,
                                                              0xFFF:VVV_STREAM_ALL*/
                       unsigned int u32StreamFlag,/*主次码流,0:连接主码流,1:次码流,2:第三码流,3:第四码流*/
                       VVV_STREAM_INFO_S *pstruStreamInfo,/*返回码流信息*/
                       VVV_ON_STREAM_CALLBACK cbStreamCallBack,/*码流回调*/
                       VVV_ON_DATA_CALLBACK cbDataCallBack,/*告警数据回调*/
                       void* pUserData,/*用户数据*/
                       unsigned int u32MaxPacketBuff);

/* 停止流传输  */
VVV_NET_API int VVV_NET_APICALL VVV_NET_StopStream(unsigned int u32ChnHandle); /*输入通道句柄*/


/* 语音对讲*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_StartTalk(unsigned int u32DevHandle,
                     VVV_ON_TALK_CALLBACK cbTalkCallBack,/*设置对讲音频数据回调:服务端->客户端*/
                     void* pUserData);

VVV_NET_API int VVV_NET_APICALL VVV_NET_StopTalk(unsigned int u32DevHandle);


/* 发送语音数据:客户端->服务端*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_SendTalkData(unsigned int u32DevHandle, unsigned char* psBuf, unsigned int u32BufLen, VVV_U64 u64Pts);

/*手动抓拍VVV_CMD_SNAP*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_Snap(unsigned int u32ChnHandle,         /*输入通道句柄*/
                VVV_SNAP_REQ_S stSnapAttr,          /*抓拍属性*/
                VVV_ON_SNAP_CALLBACK cbSnapCallBack,/*抓拍数据回调*/
                void* pUserData);

/*手动抓拍VVV_CMD_SNAP_CHN*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_Snap_Chn(unsigned int u32ChnHandle,         /*输入通道句柄*/
                VVV_SNAP_CHN_REQ_S stSnapAttr,          /*抓拍属性*/
                VVV_ON_SNAP_CALLBACK cbSnapCallBack,/*抓拍数据回调*/
                void* pUserData);

/*获取录像文件日历VVV_CMD_REC_GETCALENDER*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_GetVideoFileCalendar(unsigned int u32DevHandle, VVV_TIME_S date, int* lCalendarMap);


/*录像文件搜索VVV_CMD_REC_SEARCH_START*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_SearchAVFileList(unsigned int u32DevHandle,
                            VVV_FIND_FILE_REQ_S SearchReq,/*查询条件*/
                            VVV_FILE_LIST_VMS_S* FileList); /*查询出的文件列表结果*/

/*释放录像文件搜索VVV_CMD_REC_SEARCH_FREE*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_FreeAVFileList(unsigned int u32DevHandle,
                          VVV_FILE_LIST_VMS_S* FileList);

/*设置录像文件下载回调VVV_CMD_REC_SETFILE_CB*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_SetFileDataCallBackVMS(unsigned int u32DevHandle,void* pUserData,VVV_ON_FILE_CALLBACK cbAVFileCallBack);


/*远程录像文件的下载(包括剪辑)VVV_CMD_REC_DOWNLOAD_START*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_VodDownloadClipVMS(unsigned int u32DevHandle, 
                              VVV_DownloadFileReq_S DownloadFileReq);/*下载条件*/


/*停止录像文件下载VVV_CMD_REC_DOWNLOAD_STOP*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_StopDownLoadVideoFile(unsigned int u32DevHandle);


// 按时间回放	
/*VVV_CMD_REC_STARTPB_BYTIME*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_PlayBackByTime(unsigned int *pPlayBackHandle, unsigned int u32DevHandle,
                          VVV_FIND_FILE_REQ_S DownloadFileReq);/*按时间回放条件*/

/*VVV_CMD_REC_STOPPB_BYTIME*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_StopPlayBack(unsigned int u32PlayBackHandle);



/*VVV_CMD_REC_SET_PB_CB*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_SetPlayBackDataCallBack(unsigned int u32PlayBackHandle,void* pUserData,VVV_ON_pfnPlayBackCallback fnPlayBackDataCallback);

/*获取日志文件日历VVV_CMD_LOG_GETCALENDAR*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_GetLogCalendar(unsigned int u32DevHandle,VVV_TIME_S date, int* lCalendarMap);

/*下载日志VVV_CMD_LOG_DOWNLOAD*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_DownLoadLog(unsigned int u32DevHandle,
                       VVV_SEARCH_LOG_CONDITION_S lpLogQueryInfo,/*日志查询条件*/
                       VVV_LOG_DATA* lpLogData );    /*日志查询结果*/

/*清除日志VVV_CMD_LOG_CLEAR*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_ClearLog(unsigned int u32DevHandle, VVV_LOG_CLEAR_S stLogAttr);

/*云台控制,dwPTZCmd取值详见"云台指令"*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_PTZControl(unsigned int u32DevHandle, unsigned int chnNo, unsigned int dwPTZCmd,unsigned int dwValue);

/*升级,VVV_CMD_UPGRADE命令*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_Upgrade(unsigned int u32DevHandle, VVV_UPGRADE_ATTR_S stUpgradeAttr);

/*扫描wifi,VVV_CMD_SCAN_WIFI命令*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_ScanWifi(unsigned int u32DevHandle, VVV_WIFI_INFO_S *pWifiInfo);

/*配置服务器参数,dwCmd取值详见"设置服务器参数命令"*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_SetServerConfig(unsigned int u32DevHandle, unsigned int dwCmd, void* lpData, unsigned int Size);

/*获取服务器参数,dwCmd取值详见"获取服务器参数命令"*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_GetServerConfig(unsigned int u32DevHandle, unsigned int dwCmd, void* lpData, unsigned int *pSize);

/*获取网络库版本号*/
VVV_NET_API int VVV_NET_APICALL VVV_NET_GetVersion(char *pVersion);


#if defined(WIN32)
#define HI_LOG_DEBUG
#define HI_LOG_SYS
#else
#define HI_LOG_DEBUG(pszMode,u32LogLevel, args ...) printf(args)
#define HI_LOG_SYS(pszMode,u32LogLevel, args ...) printf(args)


#endif

 
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  //__VVV_NET_H__


