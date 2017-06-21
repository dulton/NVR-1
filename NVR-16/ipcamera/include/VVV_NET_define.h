#ifndef __VVV_NET_DEFINE_H__
#define __VVV_NET_DEFINE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif 

//下面结构体全部1字节对齐
#pragma pack (4)


#ifdef WIN32 //windows 编译, 在工程里指定了编译选项
#define VVV_OS_TYPE VVV_OS_WIN32
typedef unsigned __int64  VVV_U64;
#else
#define VVV_OS_LINUX 1
#define VVV_OS_WIN32 2
#define VVV_OS_TYPE VVV_OS_LINUX
typedef unsigned long long  VVV_U64;
#define SOCKET       int
#define SOCKET_ERROR (-1)
#endif

/************************************************************************/
/*   通用宏定义                                                         */
/************************************************************************/

#define VVV_MAX_CHN_NUM        16
#define VVV_MAX_DISK_NUM       8
#define VVV_NAME_LEN           32
#define VVV_EMAIL_NUM          3
#define VVV_IP_LEN             36
#define VVV_PATHNAME_LEN       128
#define VVV_MAX_DOMAIN_NAME    64
#define VVV_MAX_SENSOR_IN      16
#define VVV_SENSOR_OUT         16
#define ASSIGN_FOUR_BYTES(number) (((number) + 3) / 4 * 4)
#define LIVE_CLIENT_INVALID_THD_ID    (0xFFFFFFFF)

/************************************************************************/
/*    错误码                                                            */
/************************************************************************/
#define VVV_SUCCESS                           0   //成功
#define VVV_ERR_FAILURE                      -1   //内部错误
#define VVV_ERR_PARA                         -2   //参数错误
#define VVV_ERR_POINT_NULL                   -3   //指针为空
#define VVV_ERR_NOT_SUPPORT_CMD              -4   //不支持的命令
#define VVV_ERR_NET_ERRO                     -5   //网络错误
#define VVV_ERR_NEED_FREE                    -6   //需要释放内存
#define VVV_ERR_SERVER_USER_OPRATE           -7   //服务器用户正在操作
#define VVV_ERR_BADURL                       -11
#define VVV_ERR_CONN_FAILED                  -12
#define VVV_ERR_SOCKREAD_FAILED              -13
#define VVV_ERR_METHOD_ERR                   -14
#define VVV_ERR_PARSE_ERROR                  -15
#define VVV_ERR_SEQ                          -16
#define VVV_ERR_ALREADY_RUNNING              -17
#define VVV_ERR_START_FAILED                 -18
#define VVV_ERR_INLGAL_PARAM                 -19
#define VVV_ERR_ALREAD_START                 -20
#define VVV_ERR_MALLOC_FAILED                -21
#define VVV_ERR_ERROR_PROTOCAL               -22
#define VVV_ERR_RESP_UNSTANDARD              -23
#define VVV_ERR_RESP_NO_CSEQ                 -24
#define VVV_ERR_RESP_CSEQ_ERR                -25
#define VVV_ERR_RESP_NO_SESSION              -26
#define VVV_ERR_RESP_SESSION_ERR             -27
#define VVV_ERR_RESP_NO_TRANSPORT            -28
#define VVV_ERR_RESP_TRANSPORT_ERR           -29
#define VVV_ERR_RESP_NO_VIDEO_ATTR           -30
#define VVV_ERR_RESP_VIDEO_ATTR_ERR          -31
#define VVV_ERR_RESP_UNRECOGNIZE_VIDEO_ATTR  -32
#define VVV_ERR_RESP_NO_AUDIO_ATTR           -33
#define VVV_ERR_RESP_AUDIO_ATTR_ERR          -34
#define VVV_ERR_START_RECV_THREAD_FAILED     -35
#define VVV_ERR_RESP_UNRECOGNIZE_AUDIO_ATTR  -36
#define VVV_ERR_VIDEO_ALREADY_PAUSED         -37
#define VVV_ERR_AUDIO_ALREADY_PAUSED         -38
#define VVV_ERR_DATA_ALREADY_PAUSED          -39
#define VVV_ERR_VIDEO_ALREADY_PLAYED         -40
#define VVV_ERR_AUDIO_ALREADY_PLAYED         -41
#define VVV_ERR_DATA_ALREADY_PLAYED          -42
#define VVV_ERR_ERROR_TRANSTYPE              -43
#define VVV_ERR_INVALID_USER                 -44
#define VVV_ERR_OVER_SERVER_CONNECTION_NUM   -45
#define VVV_ERR_PAUSE_FAILED                 -46
#define VVV_ERR_REPLAY_FAILED                -47
#define VVV_ERR_SOCKSEND_FAILED              -48
#define VVV_ERR_USER_OR_PASSWORD             -49
#define VVV_ERR_ALMSEN_WAIT_TIMEOUT          -50  //等待命令收到超时
#define VVV_ERR_ALMSEN_ERROR_COMMAND         -51  //错误命令
#define VVV_ERR_ALMSEN_UNKNOW_COMMAND        -52  //未知命令
#define VVV_ERR_ALMSEN_FULL_DETECTOR         -53  //添加探测器数量达到上限
#define VVV_ERR_ALMSEN_NULL_DETECTOR         -54  //探测器数量为0
#define VVV_ERR_ALMSEN_FULL_TELECTRL         -55  //添加遥控器数量达到上限
#define VVV_ERR_ALMSEN_NULL_TELECTRL         -56  //遥控器数量为0
#define VVV_ERR_ALMSEN_NOT_RECV_SIGNAL       -57  //学习状态下没有接收到无线设备的信号
#define VVV_ERR_ALMSEN_NOT_STUDY_DEV         -58  //告警主机未进入学习模式

/************************************************************************/
/* 媒体操作指令                                                         */
/************************************************************************/
#define    VVV_CMD_SNAP                0x00000001 //抓拍,fun:VVV_NET_Snap,Param:VVV_SNAP_REQ_S
#define    VVV_CMD_SNAP_CHN			   0x0000000F //抓拍,fun:VVV_NET_Snap,Param:VVV_SNAP_CHN_REQ_S
#define    VVV_CMD_REC_GETCALENDER     0x00000002 //获取录像文件日历,fun:VVV_NET_GetVideoFileCalendar,Param:VVV_TIME_S
#define    VVV_CMD_REC_SEARCH_START    0x00000003 //录像文件搜索,fun:VVV_NET_SearchAVFileList,Param:VVV_FILE_LIST_VMS_S
#define    VVV_CMD_REC_SEARCH_FREE     0x00000004 //释放文件搜索,fun:VVV_NET_FreeAVFileList,Param:VVV_FILE_LIST_S
#define    VVV_CMD_REC_SETFILE_CB      0x00000005 //设置文件下载回调,fun:VVV_NET_SetFileDataCallBackVMS,Param:VVV_ON_FILE_CALLBACK
#define    VVV_CMD_REC_DOWNLOAD_START  0x00000006 //下载录像文件,fun:VVV_NET_VodDownloadClipVMS,Param:VVV_DownloadFileReq_S
#define    VVV_CMD_REC_DOWNLOAD_STOP   0x00000007 //停止下载文件,fun:VVV_NET_StopDownLoadVideoFile,Param:char* pszSouFileName
#define    VVV_CMD_REC_STARTPB_BYTIME  0x00000008 //启动按时间下载文件,fun:VVV_NET_PlayBackByTime
#define    VVV_CMD_REC_STOPPB_BYTIME   0x00000009 //停止按时间下载,fun:VVV_NET_StopPlayBack
#define    VVV_CMD_REC_SET_PB_CB       0x0000000A //设置时间下载回调,fun:VVV_NET_SetPlayBackDataCallBack
#define    VVV_CMD_LOG_GETCALENDAR     0x0000000B //下载日志日历,fun:VVV_NET_GetLogCalendar,Param:VVV_TIME_S
#define    VVV_CMD_LOG_DOWNLOAD        0x0000000C //下载日志,fun:VVV_NET_DownLoadLog,Param:VVV_SEARCH_LOG_CONDITION_S
#define    VVV_CMD_LOG_CLEAR           0x0000000D //清除日志,fun:VVV_NET_ClearLog,Param:VVV_LOG_CLEAR_S
#define    VVV_CMD_UPGRADE             0x0000000E //升级,fun:VVV_NET_Upgrade,Param:VVV_UPGRADE_ATTR_S
#define    VVV_CMD_SCAN_WIFI           0x00000020 //扫描wifi,fun:VVV_NET_ScanWifi,Param:VVV_WIFI_INFO_S

/************************************************************************/
/* 云台指令                                                             */
/************************************************************************/
#define    VVV_CMD_PTZ_UP              0x00001001 //云台向上,fun:VVV_NET_PTZControl,Param:VVV_PTZ_CMD_S
#define    VVV_CMD_PTZ_DOWN            0x00001002 //云台向下,fun:VVV_NET_PTZControl,Param:VVV_PTZ_CMD_S
#define    VVV_CMD_PTZ_LEFT            0x00001003 //云台向左,fun:VVV_NET_PTZControl,Param:VVV_PTZ_CMD_S
#define    VVV_CMD_PTZ_RIGHT           0x00001004 //云台向右,fun:VVV_NET_PTZControl,Param:VVV_PTZ_CMD_S
#define    VVV_CMD_PTZ_FOCUS_SUB       0x00001005 //Focus Far
#define    VVV_CMD_PTZ_FOCUS_ADD       0x00001006 //Focus Near
#define    VVV_CMD_PTZ_ZOOM_SUB        0x00001007 //Zoom Wide
#define    VVV_CMD_PTZ_ZOOM_ADD        0x00001008 //Zoom Tele
#define    VVV_CMD_PTZ_IRIS_SUB        0x00001009 //Iris Close
#define    VVV_CMD_PTZ_IRIS_ADD        0x0000100A //Iris Open
#define    VVV_CMD_PTZ_STOP            0x0000100B
#define    VVV_CMD_PTZ_PRESET          0x0000100C //预置
#define    VVV_CMD_PTZ_CALL            0x0000100D //调用
#define    VVV_CMD_PTZ_AUTO_STRAT      0x0000100E //自动
#define    VVV_CMD_PTZ_AUTO_STOP       0x0000100F
#define    VVV_CMD_PTZ_LIGHT_OPEN      0x00001010 //灯光
#define    VVV_CMD_PTZ_LIGHT_CLOSE     0x00001011
#define    VVV_CMD_PTZ_BRUSH_START     0x00001012 //雨刷
#define    VVV_CMD_PTZ_BRUSH_STOP      0x00001013
#define    VVV_CMD_PTZ_TRACK_START     0x00001014 //轨迹
#define    VVV_CMD_PTZ_TRACK_STOP      0x00001015
#define    VVV_CMD_PTZ_TRACK_RUN       0x00001016
#define    VVV_CMD_PTZ_HOME            0x00001017 //回到原点
#define    VVV_CMD_PTZ_CRUISE_V        0x00001018 //上下巡航
#define    VVV_CMD_PTZ_CRUISE_H        0x00001019 //左右巡航
#define    VVV_CMD_PTZ_CRUISE_HV       0x00001020 //左右,上下巡航
#define    VVV_CMD_PTZ_UP_STEP         0x00001021 //云台向上,fun:VVV_NET_PTZControl,Param:VVV_PTZ_CMD_S
#define    VVV_CMD_PTZ_DOWN_STEP       0x00001022 //云台向下,fun:VVV_NET_PTZControl,Param:VVV_PTZ_CMD_S
#define    VVV_CMD_PTZ_LEFT_STEP       0x00001023 //云台向左,fun:VVV_NET_PTZControl,Param:VVV_PTZ_CMD_S
#define    VVV_CMD_PTZ_RIGHT_STEP      0x00001024 //云台向右,fun:VVV_NET_PTZControl,Param:VVV_PTZ_CMD_S


/************************************************************************/
/* 设置服务器参数命令                                                   */
/************************************************************************/
#define    VVV_CMD_REBOOT              0x00000101  //重启服务器,Param:
#define    VVV_CMD_POWER_DOWN          0x00000102  //关机,Param:
#define    VVV_CMD_RESET               0x00000103  //复位,服务器恢复默认参数,Param:
#define    VVV_CMD_RESTORE_SETTING     0x00000105  //上传参数,Param:VVV_SETTING_S
#define    VVV_CMD_UPDATEFLASH         0x00000107  //保存到Flash,Param:
#define    VVV_CMD_SET_TIME            0x00000108  //服务器时间,Param:VVV_Server_Time_S
#define    VVV_CMD_SET_LANGUAGE        0x00000109  //服务器语言,Param:VVV_LANGUAGE_S
#define    VVV_CMD_SET_MAINTAIN        0x0000010A  //设备维护,Param:VVV_DEVICE_MAINTAIN_S
#define    VVV_CMD_SET_CHN_COLOR       0x0000010B  //通道色彩,Param:VVV_CHN_COLOR_S
#define    VVV_CMD_SET_CHN_IMAGE       0x0000010C  //通道图像属性,Param:VVV_CHN_IMAGE_S
#define    VVV_CMD_SET_CHN_OSDINFO     0x0000010D  //通道OSD,Param:VVV_CHN_OSDINFO_S
#define    VVV_CMD_SET_CHN_LOGO        0x0000010E  //通道Logo,Param:VVV_CHN_LOGO_S
#define    VVV_CMD_SET_CHN_OVERLAY     0x0000010F  //通道遮盖,Param:VVV_CHN_OVERLAY_S

#define    VVV_CMD_SET_VO              0x00000110  //服务器通道输出显示,Param:VVV_VO_SETTING_S
#define    VVV_CMD_SET_CHN_VOOSD       0x00000111  //服务器通道输出OSD显示,Param:VVV_CHN_VOOSD_S

#define    VVV_CMD_SET_VENC_CONFIG     0x00000112  //通道视频编码信息,Param:VVV_VENC_CONFIG_S
#define    VVV_CMD_SET_AENC_CONFIG     0x00000113  //通道视频编码信息,Param:VVV_AENC_CONFIG_S
#define    VVV_CMD_REQUEST_KEYFRAME    0x00000114  //请求关键帧,Param:VVV_CHN_FRAMEREQ_S

#define    VVV_CMD_SET_SNAPSHOT        0x00000115  //服务器抓图参数,Param:VVV_SNAP_S
#define    VVV_CMD_SET_REC_CONFIG      0x00000116  //录像配置,Param:VVV_RECORD_CONFIG_S
#define    VVV_CMD_RECORD_BEGIN        0x00000117  //手动开始服务器录像,Param:VVV_RECORD_MANUEL_S
#define    VVV_CMD_RECORD_STOP         0x00000118  //手动停止服务器录像,Param:VVV_RECORD_MANUEL_S

#define    VVV_CMD_SET_DEC             0x00000119  //服务器解码回放,Param:VVV_DEC_S

#define    VVV_CMD_FORMAT_DISK         0x0000011A  //格式化磁盘,Param:VVV_FORMAT_DISK_S
#define    VVV_CMD_SET_DISK_MNG        0x0000011B  //磁盘录像属性,Param:VVV_DISK_MNG_S

#define    VVV_CMD_SET_ALARM           0x0000011C  //告警结果设置,Param:VVV_ALARM_SET_S
#define    VVV_CMD_SET_MD_ALARM        0x0000011D  //通道移动侦测告警,Param:VVV_CHN_MD_ALARM_S
#define    VVV_CMD_SET_SENSOR_IN_ALARM 0x0000011E  //探头告警,Param:VVV_SENSOR_IN_ALARM_S
#define    VVV_CMD_SET_VIDEO_SHELTER   0x0000011F  //视频遮挡告警,Param:VVV_VIDEO_SHELTER_S
#define    VVV_CMD_SET_EXCEPTION_ALARM 0x00000120  //异常告警,Param:VVV_EXCEPTION_ALARM_S
#define    VVV_CMD_CLEAR_ALARM         0x00000121  //清除告警,Param:VVV_CLEAR_ALARM_S

#define    VVV_CMD_SET_PTZ_CRUISE      0x00000122  //云台巡航设置,Param:VVV_PTZ_CRUISE_S
#define    VVV_CMD_SET_COMINFO         0x00000123  //COM及云台,Param:VVV_SERVER_COMINFO_S
#define    VVV_CMD_SEND_COMDATA        0x00000124  //发送COM信息到服务器,Param:VVV_COM_DATA_S
#define    VVV_CMD_SET_USERINFO        0x00000125  //用户密码和权限,Param:VVV_SERVER_USER_S
#define    VVV_CMD_SET_IECLIENT_USER   0x00000126  //IE和客户端用户,Param:VVV_IE_CLIENT_USER_S
#define    VVV_CMD_SET_IP_DEVLIST      0x00000127  //NVR服务器IP设备管理列表,Param:VVV_IPDEV_LIST_S

#define    VVV_CMD_SET_NETWORK         0x00000128   //服务器网络配置,Param:VVV_SERVER_NETWORK_S
#define    VVV_CMD_SET_DDNSCFG         0x00000129   //设置DDNS参数,Param:VVV_DDNSCFG_S
#define    VVV_CMD_SET_UPNP            0x0000012A   //UPNP配置,Param:VVV_UPNP_CONFIG_S
#define    VVV_CMD_SET_FTP             0x0000012B   //设置服务器FTP上传报警抓拍图片,Param:VVV_FTP_PARAM_S
#define    VVV_CMD_SET_EMAIL           0x0000012C   //Email设置,Param:VVV_EMAIL_PARAM_S
#define    VVV_CMD_SET_NTP             0x0000012D   //NTP设置,Param:VVV_NTP_PARAM_S
#define    VVV_CMD_SET_CENTER_INFO     0x0000012E   //上报中心服务器,Param:VVV_CENTER_INFO_S
#define    VVV_CMD_SET_WIFI            0x0000012F   //WIFI属性,Param:VVV_WIFI_CONFIG_S
#define    VVV_CMD_SET_3G              0x00000130   //3G属性,Param:VVV_3G_CONFIG
#define    VVV_CMD_SET_MOBILE          0x00000131   //手机点播属性,Param:VVV_MOBILE_CONFIG_S
#define    VVV_CMD_SET_P2P             0x00000132   //P2P属性,Param VVV_P2P_CONFIG_S 
#define    VVV_CMD_SET_MICROWAVE_ALARM 0x00000133   //微波告警,Param:VVV_MICROWAVE_ALARM_S
#define    VVV_CMD_SET_BUZZER          0x00000135  // 开/关蜂鸣器,Param:VVV_BUZZER_CONFIG
#define    VVV_CMD_SET_RFID_INFO       0x00000136  //注册和销毁RFID,Param:VVV_SERVER_RFID_S
#define    VVV_CMD_SET_AUDIOIN_GAIN    0x00000137   //设置IPC输入音量大小(0~100)0:静音,100:最大
#define    VVV_CMD_SET_AUDIOOUT_VOL    0x00000138   //设置IPC输出音量大小(0~100)0:静音,100:最大
#define    VVV_CMD_ENABLE_AEC          0x00000139   //启用回声抵消功能
#define    VVV_CMD_DISABLE_AEC         0x0000013a   //禁用回声抵消功能
#define    VVV_CMD_SET_PIR_ALARM       0x0000013b   //PIR告警,Param:VVV_PIR_ALARM_S
#define    VVV_CMD_SET_HUMI_ALARM      0x0000013c   //HUMI告警,Param:QV_HUMI_ALARM_S
#define    VVV_CMD_SET_MD_ALARM_EX     0x0000013d  //通道移动侦测告警,Param:VVV_CHN_MD_ALARM_EX_S
#define    VVV_CMD_SET_NETWORK_PORT    0x0000013e   //服务器网络端口配置,Param:VVV_SERVER_NETWORK_PORT_S
#define    VVV_CMD_SET_PTZ_ATTR        0x0000013F   //PTZ配置,Param:VVV_PTZ_ATTR_S, *PVVV_VVV_PTZ_ATTR_S;
#define    VVV_CMD_SET_SENSOR_IN_ALARM_EX 0x00000140  //探头告警,Param:VVV_SENSOR_IN_ALARM_EX_S
#define    VVV_CMD_SET_REC_CONFIG_EX   0x00000141  //录像配置,Param:VVV_RECORD_CONFIG_EX_S
#define    VVV_CMD_SET_ALARM_SNAPSHOT  0x00000142  //服务器抓图参数,Param:VVV_ALARM_SNAP_S

/************************************************************************/
/* 获取服务器参数命令                                                   */
/************************************************************************/
#define    VVV_CMD_GET_SERVER_INFO     0x10000001  //服务器信息,Param:VVV_SERVER_INFO_S
#define    VVV_CMD_GET_SERVER_CAPS     0x10000002  //服务器能力集,Param:VVV_SERVER_CAP_S
#define    VVV_CMD_BACKUP_SETTING      0x10000003  //备份参数,Param:VVV_SETTING_S
#define    VVV_CMD_GET_TIME            0x10000004  //服务器时间,Param:VVV_Server_Time_S
#define    VVV_CMD_GET_LANGUAGE        0x10000005  //服务器语言,Param:VVV_LANGUAGE_S
#define    VVV_CMD_GET_MAINTAIN        0x10000006  //设备维护,Param:VVV_DEVICE_MAINTAIN_S

#define    VVV_CMD_GET_CHN_COLOR       0x10000007  //通道色彩,Param:VVV_CHN_COLOR_S
#define    VVV_CMD_GET_CHN_IMAGE       0x10000008  //通道图像属性,Param:VVV_CHN_IMAGE_S
#define    VVV_CMD_GET_CHN_OSDINFO     0x10000009  //通道OSD,Param:VVV_CHN_OSDINFO_S
#define    VVV_CMD_GET_CHN_LOGO        0x1000000A  //通道Logo,Param:VVV_CHN_LOGO_S
#define    VVV_CMD_GET_CHN_OVERLAY     0x1000000B  //通道遮盖,Param:VVV_CHN_OVERLAY_S

#define    VVV_CMD_GET_VO              0x1000000C  //服务器通道输出显示,Param:VVV_VO_SETTING_S
#define    VVV_CMD_GET_CHN_VOOSD       0x1000000D  //服务器通道输出OSD显示,Param:VVV_CHN_VOOSD_S

#define    VVV_CMD_GET_VENC_CONFIG     0x1000000E  //通道视频编码信息,Param:VVV_VENC_CONFIG_S
#define    VVV_CMD_GET_AENC_CONFIG     0x1000000F  //通道音频编码信息,Param:VVV_AENC_CONFIG_S

#define    VVV_CMD_GET_SNAPSHOT        0x10000010  //服务器抓图参数,Param:VVV_SNAP_S
#define    VVV_CMD_GET_REC_CONFIG      0x10000011  //录像配置,Param:VVV_RECORD_CONFIG_S
#define    VVV_CMD_GET_REC_STATE       0x10000012  //录像状态,Param:VVV_RECORD_STATE_S

#define    VVV_CMD_GET_DEC             0x10000013  //服务器解码回放,Param:VVV_DEC_S

#define    VVV_CMD_GET_DISK_INFO       0x10000014  //磁盘信息,Param:VVV_DISK_INFO_S
#define    VVV_CMD_GET_DISK_MNG        0x10000015  //磁盘录像属性,Param:VVV_DISK_MNG_S
#define    VVV_CMD_GET_DISK_STATE      0x10000016  //磁盘状态,Param:VVV_DISK_STATE_S

#define    VVV_CMD_GET_ALARM           0x10000017  //告警结果,Param:VVV_ALARM_SET_S
#define    VVV_CMD_GET_MD_ALARM        0x10000018  //通道移动侦测告警,Param:VVV_CHN_MD_ALARM_S
#define    VVV_CMD_GET_SENSOR_IN_ALARM 0x10000019  //探头告警,Param:VVV_SENSOR_IN_ALARM_S
#define    VVV_CMD_GET_VIDEO_SHELTER   0x1000001A  //视频遮挡告警,Param:VVV_VIDEO_SHELTER_S
#define    VVV_CMD_GET_EXCEPTION_ALARM 0x1000001B  //异常告警,Param:VVV_EXCEPTION_ALARM_S

#define    VVV_CMD_GET_PTZ_CRUISE      0x1000001C  //云台巡航设置,Param:VVV_PTZ_CRUISE_S
#define    VVV_CMD_GET_COMINFO         0x1000001D  //COM及云台,Param:VVV_SERVER_COMINFO_S
#define    VVV_CMD_GET_USERINFO        0x1000001E  //用户密码和权限,Param:VVV_SERVER_USER_S
#define    VVV_CMD_GET_IECLIENT_USER   0x1000001F  //IE和客户端用户,Param:VVV_IE_CLIENT_USER_S
#define    VVV_CMD_GET_IP_DEVLIST      0x10000020  //NVR服务器IP设备管理列表,Param:VVV_IPDEV_LIST_S

#define    VVV_CMD_GET_NETWORK         0x10000021   //服务器网络配置,Param:VVV_SERVER_NETWORK_S
#define    VVV_CMD_GET_DDNSCFG         0x10000022   //设置DDNS参数,Param:VVV_DDNSCFG_S
#define    VVV_CMD_GET_UPNP            0x10000023   //UPNP配置,Param:VVV_UPNP_CONFIG_S
#define    VVV_CMD_GET_FTP             0x10000024   //设置服务器FTP上传报警抓拍图片,Param:VVV_FTP_PARAM_S
#define    VVV_CMD_GET_EMAIL           0x10000025   //Email设置,Param:VVV_EMAIL_PARAM_S
#define    VVV_CMD_GET_NTP             0x10000026   //NTP设置,Param:VVV_NTP_PARAM_S
#define    VVV_CMD_GET_CENTER_INFO     0x10000027   //上报中心服务器,Param:VVV_CENTER_INFO_S
#define    VVV_CMD_GET_WIFI            0x10000028   //WIFI属性,Param:VVV_WIFI_CONFIG_S
#define    VVV_CMD_GET_3G              0x10000029   //3G属性,Param:VVV_3G_CONFIG
#define    VVV_CMD_GET_MOBILE          0x1000002A   //手机点播属性,Param:VVV_MOBILE_CONFIG_S
#define	   VVV_CMD_GET_P2P             0x10000132   //P2P属性，Param:VVV_P2P_CONFIG_S
#define    VVV_CMD_GET_MICROWAVE_ALARM 0x10000133   //微波告警,Param:VVV_MICROWAVE_ALARM_S
#define    VVV_CMD_GET_BUZZER          0x10000135   // 开/关蜂鸣器,Param:VVV_BUZZER_CONFIG
#define    VVV_CMD_GET_RFID_INFO       0x10000136   //注册和销毁RFID,Param:VVV_SERVER_RFID_S
#define    VVV_CMD_GET_HUMITURE        0x10000137   //温湿度,Param:VVV_SERVER_HUMITURE_S
#define    VVV_CMD_GET_AUDIOIN_GAIN    0x10000138   //获取IPC输入音量大小(0~100)0:静音,100:最大
#define    VVV_CMD_GET_AUDIOOUT_VOL    0x10000139   //获取IPC输出音量大小(0~100)0:静音,100:最大

#define    VVV_CMD_GET_VIDEO_MP4INFO   0x1000013A   //获取 某路视频的 sps/pps 的sdp信息
#define    VVV_CMD_GET_PIR_ALARM       0x1000013B   //PIR告警,Param:VVV_PIR_ALARM_S
#define    VVV_CMD_GET_HUMI_ALARM      0x1000013c   //HUMI告警,Param:QV_HUMI_ALARM_S
#define    VVV_CMD_GET_MD_ALARM_EX     0x1000013D  //通道移动侦测告警,Param:VVV_CHN_MD_ALARM_EX_S
#define    VVV_CMD_GET_NETWORK_PORT    0x1000013E   //服务器网络端口配置,Param:VVV_SERVER_NETWORK_PORT_S
#define    VVV_CMD_GET_PTZ_ATTR        0x1000013F   //PTZ配置,Param:VVV_PTZ_ATTR_S, *PVVV_VVV_PTZ_ATTR_S;
#define    VVV_CMD_GET_SENSOR_IN_ALARM_EX 0x10000140  //探头告警,Param:VVV_SENSOR_IN_ALARM_EX_S
#define    VVV_CMD_GET_REC_CONFIG_EX   0x10000141  //录像配置,Param:VVV_RECORD_CONFIG_EX_S
#define    VVV_CMD_GET_ALARM_SNAPSHOT  0x10000142  //服务器抓图参数,Param:VVV_ALARM_SNAP_S


/************************************************************************/
/*   码流点播数据结构                                                   */
/************************************************************************/

/*网络协议*/
typedef enum tagNET_PROTOCOL /*网络协议类型*/
{
    NET_PROTOCOL_QV   = 0,/*QV网络协议:媒体传输支持RTSPoverHTTP/RTSP/HTTP,控制协议支持私有协议和Onvif*/
    NET_PROTOCOL_ONVIF= 1,/*ONVIF协议:媒体传输支持RTSPtoHTTP/RTSP/HTTP,控制协议支持CTL_PROTOCOL_TYPE_ONVIF*/
	NET_PROTOCOL_TUTK = 2,	/*	TUTK P2P 协议*/
    NET_PROTOCOL_BUTT
}NET_PROTOCOL_E;
typedef enum tagSTREAM_TRANS_PROTOCOL /*媒体传输协议类型*/
{
    TRANS_PROTOCOL_RTSP_OVER_HTTP= 0, /*RTSP Over Http网络协议*/
    TRANS_PROTOCOL_RTSP          = 1, /*RTSP协议*/
    TRANS_PROTOCOL_HTTP          = 2, /*HTTP协议*/
    TRANS_PROTOCOL_PRIVATE       = 3, /*私有协议*/
    TRANS_PROTOCOL_BUTT
}STREAM_TRANS_PROTOCOL_E;
typedef enum tagNET_SOCKET_TYPE/*数据流的底层传输类型*/
{
    SOCKET_TYPE_TCP = 0, /*在tcp上传输*/
    SOCKET_TYPE_UDP = 1, /*在udp上传输*/
    SOCKET_TYPE_BUTT
}NET_SOCKET_TYPE_E;
typedef enum tagCONTROL_PROTOCOL_TYPE/*控制协议类型*/
{
    CTL_PROTOCOL_TYPE_PRIVATE = 0, /*私有控制协议,暂时只支持私有协议*/
    CTL_PROTOCOL_TYPE_ONVIF   = 2, /*ONVIF控制协议*/
    CTL_PROTOCOL_TYPE_BUTT
}CONTROL_PROTOCOL_TYPE_E;
typedef struct tagVVV_NET_PROTOCOL/*网络协议*/
{
    NET_PROTOCOL_E          eNetProtocol; /*网络协议类型*/
    STREAM_TRANS_PROTOCOL_E eStreamTransProtocol;/*流媒体传输协议*/
    NET_SOCKET_TYPE_E       eSocketType;  /*数据流的底层传输类型*/
    CONTROL_PROTOCOL_TYPE_E eControlProtocol;/*控制协议类型*/
}VVV_NET_PROTOCOL_S;

/*流数据类型 : 视频音频或其他告警数据*/
typedef enum tagSTREAM_TYPE_E
{
    VVV_STREAM_TYPE_VIDEO     = 0x01,/*视频*/
    VVV_STREAM_TYPE_AUDIO     = 0x02,/*音频*/
    VVV_STREAM_VIDEO_AUDIO    = 0x03,/*音视频*/
    VVV_STREAM_MD_ONLY        = 0x04,/*移动侦测告警数据*/
    VVV_STREAM_SENSORIN_ONLY  = 0x08,/*探头输入告警数据*/
    VVV_STREAM_SHELTER_ONLY   = 0x10,/*视频遮挡告警数据*/
    VVV_STREAM_VIDEO_LOSS_ONLY= 0x20,/*视频丢失告警数据*/
    VVV_STREAM_DISK_ERRO_ONLY = 0x40,/*磁盘错误告警数据*/
    VVV_STREAM_SYS_ERRO_ONLY  = 0x80,/*系统错误告警数据*/
   	VVV_STREAM_SNAP_PATH_ONLY = 0x100,/*抓拍路径数据*/
   	VVV_STREAM_MW_ONLY        = 0x200,/*微波告警数据*/
   	VVV_STREAM_PIR_ONLY       = 0x400,/*PIR 告警数据*/   	
    VVV_STREAM_ALL            = 0xFFF,/*所有码流以及告警数据*/
    VVV_STREAM_TYPE_BUTT
}VVV_STREAM_TYPE_E;

typedef enum tagOUTSIDE_ALARM_TYPE_E
{
    OUTSIDE_ALARM_TYPE_HUMI   = 0x01,   /*温湿度，PM2.5*/
    OUTSIDE_ALARM_TYPE_2,               /*未启用*/
    OUTSIDE_ALARM_TYPE_3,          
    OUTSIDE_ALARM_TYPE_4,
    OUTSIDE_ALARM_TYPE_BUTT
}OUTSIDE_ALARM_TYPE_E;

typedef struct tagVVV_OUTSIDE_ALARM/*外部告警信息*/
{
    unsigned int            dwsize;
    OUTSIDE_ALARM_TYPE_E    eOutsideAlarm; /*外部告警类型*/
    unsigned int            AlarmInfoLen;    
	char                    aszAlarmInfo[256];    
	unsigned char           reserve[12];
}VVV_OUTSIDE_ALARM_S;


/* 网络连接状态*/
typedef enum tagLIVE_NET_STAT
{
    NETSTAT_TYPE_CONNING = 0,    /*正在连接*/
    NETSTAT_TYPE_CONNING_FAILED, /*连接不上或用户密码错误*/
    NETSTAT_TYPE_CONNED,         /*已连接*/
    NETSTAT_TYPE_ABORTIBE_DISCONNED, /*已连接上的链接异常断开*/ 
    NETSTAT_TYPE_NORMAL_DISCONNED,   /*链接由用户发起主动断开*/
    NETSTAT_TYPE_BUTT
}LIVE_NET_STAT_E;


/*码流属性定义*/
typedef enum tagVVV_VIDEO_FORMAT_E/*视频编码格式*/
{
    VVV_VIDEO_FORMAT_H261  = 0,  /*H261  */
    VVV_VIDEO_FORMAT_H263  = 1,  /*H263  */
    VVV_VIDEO_FORMAT_MPEG2 = 2,  /*MPEG2 */
    VVV_VIDEO_FORMAT_MPEG4 = 3,  /*MPEG4 */
    VVV_VIDEO_FORMAT_H264  = 4,  /*H264  */
    VVV_VIDEO_FORMAT_MJPEG = 5,  /*MOTION_JPEG*/
    VVV_VIDEO_FORMAT_BUTT
}VVV_VIDEO_FORMAT_E;
typedef enum tagVVV_AUDIO_FORMAT_E/*音频编码格式*/
{
    VVV_AUDIO_FORMAT_G711A   = 1,   /* G.711 A            */
    VVV_AUDIO_FORMAT_G711Mu  = 2,   /* G.711 Mu           */
    VVV_AUDIO_FORMAT_ADPCM   = 3,   /* ADPCM              */
    VVV_AUDIO_FORMAT_G726    = 4,   /* G.726              */
    VVV_AUDIO_FORMAT_AMR     = 5,   /* AMR encoder format */
    VVV_AUDIO_FORMAT_AMRDTX  = 6,   /* AMR encoder formant and VAD1 enable */
    VVV_AUDIO_FORMAT_AAC     = 7,   /* AAC encoder        */
    VVV_AUDIO_FORMAT_BUTT
}VVV_AUDIO_FORMAT_E;
typedef enum tagVVV_AUDIO_BITWIDTH_E/*音频编码格式*/
{
    VVV_AUDIO_BITWIDTH_8   = 0,   /* Bit width is 8 bits   */
    VVV_AUDIO_BITWIDTH_16  = 1,   /* Bit width is 16 bits  */
    VVV_AUDIO_BITWIDTH_32  = 2,   /* Bit width is 32 bits */ 
    VVV_AUDIO_BITWIDTH_BUTT
} VVV_AUDIO_BITWIDTH_E;
typedef enum tagVVV_AUDIO_SAMPLE_RATE_E/*音频采样率*/
{
    VVV_AUDIO_SAMPLE_RATE_8     = 0,   /* 8K Sample rate     */
    VVV_AUDIO_SAMPLE_RATE_11025 = 1,   /* 11.025K Sample rate*/
    VVV_AUDIO_SAMPLE_RATE_16    = 2,   /* 16K Sample rate    */
    VVV_AUDIO_SAMPLE_RATE_22050 = 3,   /* 22.050K Sample rate*/
    VVV_AUDIO_SAMPLE_RATE_24    = 4,   /* 24K Sample rate    */
    VVV_AUDIO_SAMPLE_RATE_32    = 5,   /* 32K Sample rate    */
    VVV_AUDIO_SAMPLE_RATE_441   = 6,   /* 44.1K Sample rate  */
    VVV_AUDIO_SAMPLE_RATE_48    = 7,   /* 48K Sample rate    */
    VVV_AUDIO_SAMPLE_RATE_64    = 8,   /* 64K Sample rate    */
    VVV_AUDIO_SAMPLE_RATE_882   = 9,   /* 88.2K Sample rate  */
    VVV_AUDIO_SAMPLE_RATE_96    = 10,  /* 96K Sample rate    */
    VVV_AUDIO_SAMPLE_RATE_1764  = 11,  /* 176.4K Sample rate */
    VVV_AUDIO_SAMPLE_RATE_192   = 12,  /* 192K Sample rate   */
    VVV_AUDIO_SAMPLE_RATE_BUTT
} VVV_AUDIO_SAMPLE_RATE_E;
typedef enum tagVVV_SOUND_MODE_E/*声音单双声道*/
{
    VVV_SOUND_MODE_MOMO =0,          /*单声道*/
    VVV_SOUND_MODE_STEREO =1,        /*双声道*/
    VVV_SOUND_MODE_BUTT    
}VVV_SOUND_MODE_E;
typedef struct tagSTREAM_AUIDO_ATTR_S
{
    VVV_AUDIO_FORMAT_E    enAudioFormat;  /*音频格式*/         
    VVV_AUDIO_SAMPLE_RATE_E  enSampleRate;/*采样率 */        
    VVV_AUDIO_BITWIDTH_E enBitWidth;      /*采样位宽 */        
    VVV_SOUND_MODE_E     enChannMode;     /*声道模式，单声道或双声道*/
}STREAM_AUIDO_ATTR_S;
typedef struct tagSTREAM_VIDEO_ATTR_S
{
    VVV_VIDEO_FORMAT_E enVedioFormat;     /*视频编码格式*/
    unsigned int     u32PicWidth;        /*视频图像的宽*/
    unsigned int     u32PicHeight;       /*视频图像的高*/
} STREAM_VIDEO_ATTR_S;
typedef struct tagVVV_STREAM_INFO /*码流属性*/
{
    STREAM_VIDEO_ATTR_S struVencChAttr;  /* 视频编码属性 */
    STREAM_AUIDO_ATTR_S struAencChAttr;  /* 音频编码属性 */
}VVV_STREAM_INFO_S;


/* 事件回调*/
typedef int (*VVV_ON_EVENT_CALLBACK)(unsigned int u32ChnHandle, /* 通道句柄 */
                                    unsigned int u32Event,     /* 事件，详见LIVE_NET_STAT_E*/
                                    void* pUserData,           /* 用户数据*/
                                    VVV_STREAM_INFO_S* pStreamInfo);/*码流属性*/

/* 码流回调*/
typedef int (*VVV_ON_STREAM_CALLBACK)(unsigned int u32ChnHandle,/* 通道句柄 */
                                     unsigned int u32DataType,/* 数据类型，详见VVV_STREAM_TYPE_E
                                                                 0x01-视频，0x02-音频*/
                                     unsigned char* pu8Buffer,/* 数据包含帧头 */
                                     unsigned int u32Length,  /* 数据长度 */
                                     VVV_U64  u64TimeStamp,    /* 时间戳*/
                                     VVV_STREAM_INFO_S *pStreamInfo,/*码流属性*/
                                     void* pUserData);        /* 用户数据*/

/* 告警回调*/
typedef int (*VVV_ON_DATA_CALLBACK)(unsigned int u32ChnHandle,/* 通道句柄 */
                                   unsigned int u32DataType, /* 数据类型,详见VVV_STREAM_TYPE_E
                                                                0x04-MD告警数据,
                                                                0x08-探头输入告警数据,
                                                                0x10-视频遮挡告警数据,
                                                                0x20-视频丢失告警数据,
                                                                0x40-磁盘错误告警,
                                                                0x80-系统错误告警*/
                                   unsigned char*  pu8Buffer,/* 数据,MD告警数据为
                                   "Alarm:报警矩形区域号-左上角X坐标-左上角Y坐标-宽-高|报警矩形区域号-左上角X坐标-左上角Y坐标-宽-高|",
                                   比如"Alarm:1-558-461-120-120|2-520-0-120-120|3-0-360-120-120|4-520-360-120-120|"*/
                                   unsigned int u32Length,          /* 数据长度 */
                                   VVV_U64       u64TimeStamp,       /* 时间戳*/
                                   void* pUserData);                /* 用户数据*/

/* 对讲音频回调*/
typedef int (*VVV_ON_TALK_CALLBACK)(unsigned int u32DevHandle,/* 设备句柄 */
                                   unsigned int u32DataType,/* 数据类型，详见STREAM_TYPE_E
                                                               0x02-音频*/
                                   unsigned char* pu8Buffer,/* 数据包含帧头 */
                                   unsigned int u32Length,  /* 数据长度 */
                                   VVV_U64  u64TimeStamp,    /* 时间戳*/
                                   VVV_STREAM_INFO_S *pStreamInfo,/*码流属性*/
                                   void* pUserData);        /* 用户数据*/

/************************************************************************/
/*   抓拍录像回放数据结构                                               */
/************************************************************************/
/*抓拍请求条件VVV_CMD_SNAP*/
typedef struct tagVVV_SNAP_REQ
{
    unsigned int snapformat;     /*抓拍格式,0-jpeg,1-bmp,默认jpeg*/
    unsigned int count;          /*连续抓拍张数,默认1*/
    unsigned int timeinterval;   /*抓拍时间间隔,默认500毫秒，单位毫秒*/
}VVV_SNAP_REQ_S;

/*抓拍请求条件VVV_CMD_SNAP_CHN*/
typedef struct tagVVV_SNAP_CHN_REQ
{
	unsigned int chn;
    unsigned int snapformat;     /*抓拍格式,0-jpeg,1-bmp,默认jpeg*/
    unsigned int count;          /*连续抓拍张数,默认1*/
    unsigned int timeinterval;   /*抓拍时间间隔,默认500毫秒，单位毫秒*/
	unsigned char reserve[32];
}VVV_SNAP_CHN_REQ_S;

typedef struct tagVVV_TIME
{
	unsigned char year;			
	unsigned char month;		
	unsigned char day;
	unsigned char week;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
}VVV_TIME_S;

/*录像文件查询条件VVV_CMD_REC_SEARCH_START*/
typedef struct tagVVV_FindFileReq/*录像文件查询条件VVV_CMD_REC_SEARCH_START*/
{
    int chn;               /*通道号0~VVV_MAX_CHN_NUM*/
    VVV_TIME_S BeginTime;   /*BeginTime EndTime值都为0000-00-00 00:00:00*/
    VVV_TIME_S EndTime;     /*表示查找所有时间*/
    unsigned int nFileType;/*文件类型:1-常规录像,2-报警录像,3-所有录像*/
}VVV_FIND_FILE_REQ_S;

/*搜索到的录像文件参数VVV_CMD_REC_SEARCH_START*/
typedef struct
{
    VVV_U64 file_size;       
    int rec_type;        //录像类型， 1：常规录像 2：报警录像，3 ：所有录像 
    unsigned int chn;    /*通道号, 
                           4个ASCII码，表示2个16进制形式的字节。
                           位值为1表示搜索对应的通道号。
                           bit0~bit15分别代表1~16通道*/
    char file[VVV_PATHNAME_LEN];//录像文件名,要求给设备端的绝对路径
    unsigned int   start_hour;
    unsigned int   start_min;
    unsigned int   start_sec;/*录像文件开始时间*/
    unsigned int   end_hour;
    unsigned int   end_min;
    unsigned int   end_sec;  /*录像文件结束时间*/
	unsigned char reserve[2];
}search_file_setting_s;
typedef struct tag_file_list_s
{
	int num;				// 文件个数
	search_file_setting_s	*pFileInfo;
}VVV_FILE_LIST_VMS_S;

/*释放录像文件参数VVV_CMD_REC_SEARCH_FREE*/
typedef struct tagfile_info_s
{
	char	*pszFileName;
	int		iLenTime;			// Time length, by minute, use on MDVR only 
} file_info_s;


typedef struct snap_file_list_s
{
	int num;	//文件个数
	char   aszFilesName[6][VVV_PATHNAME_LEN]; 
}VVV_SNAP_FILE_LIST_S;

/*下载录像文件条件VVV_CMD_REC_DOWNLOAD_START*/
typedef struct tagVVV_DownloadFileReq
{
    char         pszSouFileName[128];/*要下载的录像文件名*/
    unsigned int lOffSetFlag; /*偏移标志 0-整个录像 1-大小偏移 2-时间偏移*/
    unsigned int lBeginOffSet;
    unsigned int lEndOffSet;
    unsigned int lOffSet;/*偏移量,如果是整个录像,则相对于开始,如果是剪辑,相对于剪辑的开始,
                          注意如果是剪辑，设备默认发文件头，如果不要文件头，loffSet=64*1204,
                          裁剪时确定 if(lBeginOffSet!=0) lBeginOffSet >= 64*1024*/
}VVV_DownloadFileReq_S;
//设备对请求下载文件的应答补充结构
typedef struct tagfile_return_vms
{
	VVV_U64	filesize;				/*文件真实大小*/
	int sess_id;                /*会话ID*/
}VVV_file_return_vms;


/*抓拍回调VVV_CMD_SNAP*/
typedef int(*VVV_ON_SNAP_CALLBACK)(unsigned int u32ChnHandle,/* 通道句柄 */
                                  char *SnapData,         /*抓拍数据指针*/
                                  unsigned int DataSize,  /*抓拍数据长度*/
                                  int width,
                                  int height,/*抓拍返回的图像宽高*/
                                  int count, /*抓拍的第几张*/
                                  void* pUserData);

/* 远程录像下载回调*/
typedef struct tagCallBackData_S

{ 
	int lBufferLen;	//lBufferLen=CALLBACK_OTHER or lBufferLen>0
	VVV_U64 lTotalLen;		//if(lBufferLen==CALLBACK_OTHER) {lTotalLen=CALLBACK_LINKBUILD or CALLBACK_LINKBREAK}
	void* pVoid;
}CallBackData_S;
typedef int (*VVV_ON_FILE_CALLBACK)(void* pUserData, const CallBackData_S* pstuDataInfo);

/*按时间回放回调*/
typedef int (*VVV_ON_pfnPlayBackCallback)(unsigned int u32PlayBackHandle,int dwDataType,
                                           char *pBuffer,unsigned int dwBufSize,void* pUserData);


/*日志查询条件VVV_CMD_LOG_DOWNLOAD*/
typedef enum _taglogtype_/*日志类型*/
{
    VVV_LT_IO = 0,      /*I/O错误,用于写日志*/
    VVV_LT_MD = 1,      /*移动侦测告警,用于写日志*/
    VVV_LT_HDD = 2,     /*硬盘错误,用于写日志*/
    VVV_LT_VIDEOLOSS = 3,/*视频丢失告警,用于写日志*/
    VVV_LT_OPERATION = 4,/*查询和写入共用*/
    VVV_LT_ALARM,       /*查询使用，包括io、md、视频丢失和硬盘*/
    VVV_LT_ALL,         /*查询使用，查询全部*/
    VVV_LT_INVALID
}VVV_logtype_e;
typedef enum _taglogoperation_e
{
    VVV_FIRSTPAGE_SEARCH,   /*第一次搜索或回到第一页*/
    VVV_NEXTPAGE_SEARCH,    /*下一页*/
    VVV_PREPAGE_SEARCH,     /*上一页*/
    VVV_LASTPAGE_SEARCH     /*最后一页*/
}VVV_logoperation_e;
typedef struct tagVVV_SEARCH_LOG_CONDITION
{
    unsigned int    lpnDataCount; /*日志查询个数*/
    VVV_TIME_S       start;
    VVV_TIME_S       end;
    VVV_logtype_e       type;
    VVV_logoperation_e  operation;   /*用户操作:第一次显示;下一页;上一页;最后一页*/
    int             searchmax;    /*希望返回的最大个数*/
    int             ChannelBitMap;
}VVV_SEARCH_LOG_CONDITION_S;

/*日志查询结果*/
typedef struct
{
	VVV_TIME_S	  date; 		//时间
	unsigned int  dwMajorType;	//主类型 1-报警; 2-异常; 3-操作; 0xff-全部
	unsigned int  dwMinorType;	//次类型 0-全部;
	char		  RemoteHostAddr[VVV_NAME_LEN];//远程主机地址
	char		  sOpUser[VVV_NAME_LEN];  //操作人的用户名
	unsigned int bChannel;	   //通道号
	unsigned int bDiskNumber;  //硬盘号
	unsigned int bAlarmInPort; //报警输入端口
	unsigned int bAlarmOutPort;//报警输出端口
	unsigned int bRecorded;    //是否做了录像操作
	char		 csBuf[VVV_NAME_LEN];//用户操作时记录被操作的用户
}log_info_s;
typedef struct tagVVV_LOG_DATA   //size = 44
{
	int num;   //日志文件个数
	log_info_s *pLogInfo;
}VVV_LOG_DATA;

/*日志清除属性VVV_CMD_LOG_CLEAR*/
typedef struct tagVVV_LOG_CLEAR
{
    unsigned int bSysLog;   //系统日志
    unsigned int bAccessLog;//访问日志
}VVV_LOG_CLEAR_S;

//升级结构体VVV_CMD_UPGRADE
typedef struct tagVVV_UPGRADE_ATTR
{
    unsigned int  dwSize;
    char          file[VVV_PATHNAME_LEN];//升级文件路径
    unsigned int  UpgradeAddr; /*升级位置，相对于参数分区的首地址偏移12个字节*/
}VVV_UPGRADE_ATTR_S;

//扫描wifi结果VVV_NET_ScanWifi
typedef struct tagwifi_info
{
    char            szEssid[VVV_NAME_LEN];
    unsigned int    nSecurity;   //0: none,
                                 //1:wep
                                 //2:wpa/wpa2
    unsigned int  byMode;  //1. managed 2. ad-hoc
    int           quality; //信号质量0~100
    char          szWebKey[VVV_NAME_LEN];
}wifi_info;
typedef struct tagVVV_WIFI_INFO_S
{
    unsigned int  num;      //扫描出的wifi个数
	wifi_info    *pWifiInfo;//wifi信息
}VVV_WIFI_INFO_S;


//云台控制结构体,详见"云台指令"
typedef struct tagVVV_PTZ_CMD
{
    unsigned int  dwSize;
    unsigned int  chn;     /*通道号*/
    unsigned int  dwValue; /*云台操作值*/
    int  speed;            /*云台速度,-1采用上一次的速度*/
}VVV_PTZ_CMD_S;

/************************************************************************/
/* 服务器配置信息结构                                                   */
/************************************************************************/
//服务器信息VVV_CMD_GET_SERVER_INFO
typedef struct tagVVV_SERVER_INFO
{
    unsigned int   dwSize;
    char           dwServerFlag[VVV_NAME_LEN];    //服务器类型
    char           dwServerSubFlag[VVV_NAME_LEN]; //服务器子类型
    unsigned int   ServerCPUType;   //当前CPU类型:0X3518,0X3520A,0X3531
    char           serialNum[VVV_NAME_LEN];  //设备序列号
    char           DevID[VVV_NAME_LEN];      //设备ID,每台设备唯一,AutoID也使用该ID
    char           model[VVV_NAME_LEN];      //型号
    char           name[VVV_NAME_LEN];       //服务器名称
    char           hardVersion[VVV_NAME_LEN];//硬件版本
    char           softVersion[VVV_NAME_LEN];//软件版本
    char           startdate[VVV_NAME_LEN];  //系统启动日期时间,比如"2006.09.06.09.11.30"
    unsigned int   runtimes;                //系统已启动时长,单位:分钟
    char           szServerIp[VVV_NAME_LEN];//服务器IP(字符串表示形式)
    char           szMACAddr[VVV_NAME_LEN]; //服务器的物理地址
    unsigned int   ServerPort;      //服务器端口
    unsigned int   ChannelNum;      //通道数量
}VVV_SERVER_INFO_S,*PVVV_SERVER_INFO_S;

//服务器能力集VVV_CMD_GET_SERVER_CAPS
typedef struct tagVVV_SERVER_CAP
{
    unsigned int dwSize;
    unsigned int EncChnMaxNum;   //最大编码数
    unsigned int DecChnMaxNum;   //最大解码数
    unsigned int Stream1Height;  //视频高(1)
    unsigned int Stream1Width;   //视频宽
    unsigned int Stream1CodecID; //视频编码类型号（H264为0x01,MJPEG为0x02,MPEG4为0x04）
    unsigned int Stream2Height;  //视频高(2)
    unsigned int Stream2Width;   //视频宽
    unsigned int Stream2CodecID; //视频编码类型号（H264为0x01,MJPEG为0x02,MPEG4为0x04）
    unsigned int Stream3Height;  //视频高(2)
    unsigned int Stream3Width;   //视频宽
    unsigned int Stream3CodecID; //视频编码类型号（H264为0x01,MJPEG为0x02,MPEG4为0x04）
    unsigned int AudioMaxNum;    //最大音频数
    unsigned int DisplayDev;     //显示输出:0x01-CVBS,0x02-VGA,0x04-HDMI
    unsigned int DISKMaxNum;     //最大硬盘数
    unsigned int SDMaxNum;       //最大SD卡数
    unsigned int AlarmInNum;     //告警输入路数
    unsigned int AlarmOutNum;    //告警输出路数
    unsigned int UserRight;      //当前用户权限:0X01-控制云台,0x02-录像/抓拍,0x04-回放/下载,0x08-设置参数,0x10-开/关音量
    unsigned int NetPriviewRight;//网络观看权限:通道按照bit位计算
    unsigned int NetProtocol;//支持的网络协议(0x01支持RTSPtoHTTP,0x02支持RTSP,0x04支持HTTP,0x08支持ONVIF)
    unsigned int RemoteUserMaxNum;//最大远程用户数
}VVV_SERVER_CAP_S,*PVVV_SERVER_CAP_S;

//备份与上传参数结构体VVV_CMD_RESTORE_SETTING,VVV_CMD_BACKUP_SETTING
typedef struct tagVVV_SETTING
{
    unsigned int dwSize;
    char         file[VVV_PATHNAME_LEN];//备份文件路径
}VVV_SETTING_S;

/*时间VVV_CMD_SET_TIME,VVV_CMD_GET_TIME*/
typedef struct VVV_Server_Time
{
    unsigned int  dwSize;
    unsigned int  year;   //时间年
    unsigned int  month;  //时间月
    unsigned int  day;    //时间日
    unsigned int  hour;   //时间时
    unsigned int  minute; //时间分
    unsigned int  second; //时间秒
    unsigned char DateMode;	/*日期格式，0为MM/DD/YY，1为YY-MM-DD*/
    unsigned char TimeMode;	/*时间格式，0为24小时制，1为12小时制*/
    unsigned char bdstmode; //是否设置时区, 0--关闭,  1--开启
    char          timezone; //时区-12~12,它跟citytimezone是两种不同的时区表达方式
    char          citytimezone[VVV_NAME_LEN];//城市时区
                        /*
                        "Etc/GMT-12",
                        "Pacific/Apia",
                        "Pacific/Honolulu",
                        "America/Anchorage",
                        "America/Los_Angeles",
                        "America/Denver",
                        "America/Tegucigalpa",
                        "America/Phoenix",
                        "America/Winnipeg",
                        "America/Mexico_City",
                        "America/Chicago",
                        "America/Costa_Rica",
                        "America/Indianapolis",
                        "America/New_York",
                        "America/Bogota",
                        "America/Santiago",
                        "America/Caracas",
                        "America/Montreal",
                        "America/St_Johns",
                        "America/Thule",
                        "America/Buenos_Aires",
                        "America/Sao_Paulo",
                        "Atlantic/South_Georgia",
                        "Atlantic/Cape_Verde",
                        "Atlantic/Azores",
                        "Europe/Dublin",
                        "Africa/Casablanca",
                        "Europe/Amsterdam",
                        "Europe/Belgrade",
                        "Europe/Brussels",
                        "Europe/Warsaw",
                        "Africa/Lagos",
                        "Europe/Athens",
                        "Europe/Bucharest",
                        "Africa/Cairo",
                        "Africa/Harare",
                        "Europe/Helsinki",
                        "Asia/Jerusalem",
                        "Asia/Baghdad",
                        "Asia/Kuwait",
                        "Europe/Moscow",
                        "Africa/Nairobi",
                        "Asia/Tehran",
                        "Asia/Dubai",
                        "Asia/Baku",
                        "Asia/Kabul",
                        "Asia/Yekaterinburg",
                        "Asia/Karachi",
                        "Asia/Calcutta",
                        "Asia/Katmandu",
                        "Asia/Almaty",
                        "Asia/Dhaka",
                        "Asia/Colombo",
                        "Asia/Rangoon",
                        "Asia/Bangkok",
                        "Asia/Krasnoyarsk",
                        "Asia/Hong_Kong",
                        "Asia/Irkutsk",
                        "Asia/Kuala_Lumpur",
                        "Australia/Perth",
                        "Asia/Taipei",
                        "Asia/Tokyo",
                        "Asia/Seoul",
                        "Asia/Yakutsk",
                        "Australia/Adelaide",
                        "Australia/Brisbane",
                        "Australia/Sydney",
                        "Pacific/Guam",
                        "Australia/Hobart",
                        "Asia/Vladivostok",
                        "Asia/Magadan",
                        "Pacific/Auckland",
                        "Pacific/Fiji",
                        "Pacific/Tongatapu"*/
    unsigned char Dst;/*夏令时，0-关闭，1-打开*/
    unsigned char DstMode;/*夏令时模式，0-默认，1-自定义*/
    unsigned char StartWeek;/*开始周，0-第一周，1-第二周，2-第三周，3-第四周*/
    unsigned char StartMonth;/*开始月，共12个月:0---11*/
    unsigned char EndWeek;/*结束周，0-第一周，1-第二周，2-第三周，3-第四周*/
    unsigned char EndMonth;/*结束月，共12个月:0---11*/
    unsigned char StartHour;
    unsigned char EndHour;
    unsigned char Offset;
    unsigned char MenuAutoExitTime; //菜单自动退出时间,0:关闭 1:1分钟 2:2分钟 3:3分钟 4:4分钟 5:5分钟
    unsigned char LockGUITime;      //菜单超时锁定 0:关闭 1:1分钟 2:2分钟 3:3分钟 4:4分钟 5:5分钟
    unsigned char bSaveFlash;
}VVV_Server_Time_S;

//服务器语言VVV_CMD_SET_LANGUAGE,VVV_CMD_GET_LANGUAGE
#define VVV_LUNGUAGE_CHINESE        0
#define VVV_LUNGUAGE_ENGLISH        1
#define VVV_LUNGUAGE_RUSSIAN        2   //俄国
#define VVV_LUNGUAGE_FRENCH         3   //法国
#define VVV_LUNGUAGE_ITALIAN        4   //意大利
#define VVV_LUNGUAGE_POLAND         5   //波兰
#define VVV_LUNGUAGE_PORTUGUESE     6   //葡萄牙
#define VVV_LUNGUAGE_SPAISH         7   //西班牙
#define VVV_LUNGUAGE_THAI           8   //泰国
#define VVV_LUNGUAGE_TURKISH        9   //土耳其
#define VVV_LUNGUAGE_KOREAN         10  //朝鲜
#define VVV_LUNGUAGE_JAPANESE       11  //日本
#define VVV_LUNGUAGE_CHINESEBIG5    12  //繁体
#define VVV_LUNGUAGE_VIETNAMES      13  //越南
typedef struct tagVVV_LANGUAGE
{
    unsigned int dwSize;
    unsigned int language;/*系统语言，0-中文，1-英语*/
    unsigned int langenc;
    int          bSaveFlash;
}VVV_LANGUAGE_S,*PVVV_LANGUAGE_S;

//设备维护VVV_CMD_SET_MAINTAIN，VVV_CMD_GET_MAINTAIN
typedef struct tagVVV_DEVICE_MAINTAIN
{
    unsigned int dwSize;
    unsigned int time[4];/*维护的具体时间，时分*/
    unsigned int automaintain;/*自动维护开关，0-关闭，1-开启*/
    unsigned int maintainperiod1;/*维护周期一，0-每天，1-每周，2-每月*/
    unsigned int maintainperiod2;/*维护周期二，0--6每周的周几，7--37每月的几号*/
    int          bSaveFlash;
}VVV_DEVICE_MAINTAIN_S,*PVVV_DEVICE_MAINTAIN_S;

/*--------------------------通道设置结构--------------------------------*/
//通道颜色设置VVV_CMD_SET_CHN_COLOR,VVV_CMD_GET_CHN_COLOR
typedef struct tagVVV_CHN_COLOR
{
    unsigned int dwSize;
    unsigned int chn;        //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int Hue;        //色调0~100
    unsigned int Color;      //色彩0~100
    unsigned int Contrast;   //对比度0~100
    unsigned int Brightness; //亮度0~100
    unsigned int Definition; //清晰度0~100
    int          bSaveFlash;
}VVV_CHN_COLOR_S,*PVVV_CHN_COLOR_S;

/*图像属性VVV_CMD_SET_CHN_IMAGE,VVV_CMD_GET_CHN_IMAGE*/
typedef struct tagVVV_CHN_IMAGE
{
    unsigned int dwSize;
    unsigned char chn;          //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    char         scene[VVV_NAME_LEN];    //场景"outdoor|indoor|auto|manual|lowlight|highlight"
    char         powerfreq[VVV_NAME_LEN];//电源频率"50", "60"
    char         flip[VVV_NAME_LEN];     //"on", "off"
    char         mirror[VVV_NAME_LEN];   //"on", "off"
    unsigned char ircut;                 //0关闭,1开启
    char         awbswitch[VVV_NAME_LEN];//自动白平衡开关"on","off"
    char         color[VVV_NAME_LEN];    //颜色控制:彩"color", 灰"baw"
    unsigned char bSaveFlash;
    unsigned char wdr; //BTaligned,为了对齐增加, 20140116
}VVV_CHN_IMAGE_S,*PVVV_CHN_IMAGE_S;


//通道OSD显示以及通道名称设置VVV_CMD_SET_CHN_OSDINFO,VVV_CMD_GET_CHN_OSDINFO
typedef struct tagVVV_CHN_OSDINFO
{
    unsigned int  dwSize;
    unsigned char chn;        //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char bShowTime;  //是否显示时间
    unsigned char TimePosition;/*0-左上，1-左下，2-右上，3-右下*/
    unsigned char bShowString; //是否显示用户输入字符串(通道名称在此体现)
    unsigned char StringPosition;	  /*0-左上，1-左下，2-右上，3-右下*/
    char          csString[48];       //用户输入字符串
    unsigned char bSaveFlash;
    unsigned char rev1; //BTaligned,为了对齐增加, 20140116
    unsigned char rev2; //BTaligned,为了对齐增加, 20140116
}VVV_CHN_OSDINFO_S,*PVVV_CHN_OSDINFO_S;

//LOGO叠加设置VVV_CMD_SET_CHN_LOGO,VVV_CMD_GET_CHN_LOGO
typedef struct tagVVV_CHN_LOGO
{
    unsigned int   dwSize;
    unsigned char  chn;     //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char  bEnable; //是否启用LOGO显示
    unsigned char  resv1;    //BTaligned,为了对齐增加, 20140116
    unsigned char  resv2;     //BTaligned,为了对齐增加, 20140116
    unsigned int   x;       //LOGO显示起始位置
    unsigned int   y;
    unsigned char  bLogoData[VVV_PATHNAME_LEN];//此LOGO数据可以调用...获得。最大限制为40K
    unsigned char  bSaveFlash;
    unsigned char  resv3;     //BTaligned,为了对齐增加, 20140116
    unsigned char  resv4;     //BTaligned,为了对齐增加, 20140116
    unsigned char  resv5;      //BTaligned,为了对齐增加, 20140116   
}VVV_CHN_LOGO_S;

//遮盖区域设置VVV_CMD_SET_CHN_OVERLAY,VVV_CMD_GET_CHN_OVERLAY
typedef struct tagrect_t
{
    int left;
    int top;
    int right;
    int bottom;
}rect_t;
typedef struct tagVVV_CHN_OVERLAY
{
    unsigned int  dwSize;
    unsigned char chn;            //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char bEnable[4];     //是否进行区域遮挡
    unsigned char resv1; //BTaligned,为了对齐增加, 20140116
    unsigned char resv2; //BTaligned,为了对齐增加, 20140116
    unsigned char resv3; //BTaligned,为了对齐增加, 20140116   
    rect_t        rcShelter[4];   //遮挡区域，最多支持4块的区域遮挡。
    unsigned char u32Color[4];    //区域颜色,像素格式是RGB8888,默认为黑色:值为0 
    unsigned char bSaveFlash;
    unsigned char resv4; //BTaligned,为了对齐增加, 20140116
    unsigned char resv5; //BTaligned,为了对齐增加, 20140116
    unsigned char resv6; //BTaligned,为了对齐增加, 20140116      
}VVV_CHN_OVERLAY_S,*PVVV_CHN_OVERLAY_S;

//服务器输出显示VVV_CMD_SET_VO,VVV_CMD_GET_VO
typedef struct _tagvoattr_
{
    struct _tagchnselect_
    {
        unsigned char twentychn[20];	//二十画面
        unsigned char sixteenchn[16];	//十六画面
        unsigned char eightchn[8];		//1+7画面
        unsigned char sixchn[6];		//1+5画面
        unsigned char fourchn[4];		//四画面
        unsigned char ninechn[9];		//九画面
        unsigned char onechn;			//一画面
        unsigned char key;				//与OutputMode结合应用，ex: OutputMode:0  key=0 -->通道1全屏; OutputMode:1  key=0 -->第一个四分屏
        unsigned char reserve[63];
    } chnselect_s;/*定制预览通道，只能选择其中一种预览模式*/
    unsigned char PollingTime;		/*输出轮询时间间隔: 5秒～1、2、5分钟任选，单位：5秒，0表示不轮询*/
    unsigned char StaticOrDynamic;	/*0-static，1-dynamic(用户手动设置)*/
    unsigned char OutputMode;		/*主输出预览模式:0-一画面，1-四画面，2-九画面，3-十六画面*/
    unsigned char Margin[4];		/*0:左边距, 1:右边距, 2:上边距, 3:下边距, 2像素为单位*/
    unsigned char reserve1[2];		/*保留字节*/
} voattr_s;
typedef struct tagVVV_VO_SETTING
{
    unsigned int dwSize;
    unsigned char IsVoutOrSpot;/*0-Vout,1-Spot*/
    unsigned char vgaresolution;/*分辨率:0-600x480,1-800x600,2-1024x768*/
    unsigned char Bt1120Resolution;/*分辨率:0-800x600，1-1024x768, 2-1280x1024, 3-1920x1080*/
    unsigned char cameramode;/*摄像头制式:0-TS_PAL,1-TS_NTSC*/
    unsigned char vgatype;/*VGA输出类型:0-VGA, 1-BT1120 */
    voattr_s voattr[3];/*voattr[0]-OD_VOUT, voattr[1]-OD_VSPOT, voattr[2]-OD_VGA*/
    unsigned char bSaveFlash;
}VVV_VO_SETTING_S,*PCMD_SET_VO_S;

//服务器通道输出OSD显示VVV_CMD_SET_CHN_VOOSD,VVV_CMD_GET_CHN_VOOSD
typedef struct tagVVV_CHN_VOOSD
{
    unsigned int  dwSize;
    unsigned char Preview[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];/*预览osd开关*/
    unsigned char chnosdPosition[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];/*0-左上，1-左下，2-右上，3-右下*/
    char          ChnName[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)][VVV_NAME_LEN];/*通道名称*/
    unsigned char PreviewTimeSet;/*0-关闭，1-打开*/
    unsigned char audio[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];/*音频绑定视频通道*/
    unsigned char voice;/*0-关闭，1-打开*/
    unsigned char bSaveFlash;
}VVV_CHN_VOOSD_S,*PVVV_CHN_VOOSD_S;

//通道视频编码设置VVV_CMD_SET_VENC_CONFIG,VVV_CMD_GET_VENC_CONFIG
typedef struct tagVVV_VENC_CONFIG
{
    unsigned int  dwSize;
    unsigned int  chn;          //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int  Stream;       //0-主码流,1-次码流,2-第三码流
    unsigned int  VideoFormat;  //视频编码格式:3-MPEG4,4-H264,5-MJPEG,详见VVV_VIDEO_FORMAT_E
    unsigned int  w;            //宽
    unsigned int  h;            //高
    unsigned int  FrameRate;    //帧率 (1-50/60) PAL为25，NTSC为30
    unsigned int  Standard;     //制式 (0为PAL制,1为NTSC制)
    unsigned int  RateType;     //流模式(0为定码流，1为变码流)
    unsigned int  resolution;   //录象细节:0-精细(D1)，1-丰富(HD1)，2-普通(CIF),3-QVGA,4-VGA,5-720P,6-1080P
    unsigned int  dwBitRate;    //码率(8-16384),单位Kbps
    unsigned int  ImageQuality; //编码质量(0-5),0为最好,5为最差
    unsigned int  nMaxKeyInterval;//关键帧间隔(1-100)
    int           bSaveFlash;
}VVV_VENC_CONFIG_S,*PVVV_VENC_CONFIG_S;

typedef struct tagVVV_VENC_MP4INFO
{
    unsigned int dwSize;
    unsigned int chn; //0, ipc固定为0
    unsigned int stream; //0 主码流, 1, 次码流, 2, 手机码流
    unsigned int infolen;
    unsigned char info[128];
}VVV_VENC_MP4INFO_S;

/*音频属性VVV_CMD_SET_AENC_CONFIG,VVV_CMD_GET_AENC_CONFIG*/
typedef struct tagVVV_AENC_CONFIG
{
    unsigned int  dwSize;
    unsigned int  chn;          //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int  bEncodeAudio; //是否编码音频
    unsigned int  AudioFormat;  //音频编码格式:1-G711A,2-G711U,3-ADPCM,4-G726,详见VVV_AUDIO_FORMAT_E
    unsigned int  samplerate;   //采样率: 0-8K,2-16K,4-24K,5-32K,6-44.1K,7-48K,详见VVV_AUDIO_SAMPLE_RATE_E
    unsigned int  bitwidth;     //位宽: 0-8位,1-16位,详见VVV_AUDIO_BITWIDTH_E
    unsigned int  soundmode;    //声音模式, 0-MOMO,1-STEREO,详见VVV_SOUND_MODE_E
    unsigned int  bindvideochn; //音频绑定的视频通道号
    int           bSaveFlash;
}VVV_AENC_CONFIG_S,*PVVV_AENC_CONFIG_S;

//请求关键帧VVV_CMD_REQUEST_KEYFRAME
typedef struct tagVVV_CHN_FRAMEREQ
{
    unsigned int  dwSize;
    unsigned int  chn;         //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int  u32StreamFlag;//主次码流,0:主码流,1:次码流,2:第三码流,3:第四码流
    unsigned int  FrameType;   // 0 - I，其余保留
}VVV_CHN_FRAMEREQ_S, *PVVV_CHN_FRAMEREQ_S;

//抓拍存服务端参数VVV_CMD_SET_SNAPSHOT,VVV_CMD_GET_SNAPSHOT
typedef struct tagVVV_SNAP
{
    unsigned int  dwSize;
    unsigned char chn;        //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char group;      //第几组抓拍
    unsigned char bEnable;    //抓拍使能
    unsigned char operation;  //(1:snap,4:snap&mail,0x10:snap&ftp)
    int           interval;   //定时抓拍时间间隔
    unsigned char hour[7][96];//抓拍计划
    unsigned char Qval;       //Q值0~2
	unsigned char continueNum;   //连续抓拍张数0~5
    unsigned char bSaveFlash;
	unsigned char resv1;
}VVV_SNAP_S, *PVVV_SNAP_S;

//告警后抓拍存服务端参数VVV_CMD_SET_ALMSNAPSHOT,VVV_CMD_GET_ALMSNAPSHOT
typedef struct tagVVV_ALARM_SNAP
{
    unsigned int  dwSize;
    unsigned char enable;
    unsigned char interval; //告警抓拍间隔
    unsigned char time; //告警抓拍时长
    unsigned char quality; //抓拍图片质量，1:最好 2: 好 3:一般，其他默认是最好
    unsigned char bSaveFlash;
	unsigned char resv[39];
}VVV_ALARM_SNAP_S, *PVVV_ALARM_SNAP_S;


//录像设置VVV_CMD_SET_REC_CONFIG,VVV_CMD_GET_REC_CONFIG
typedef struct tagVVV_RECORD_CONFIG
{
    unsigned int  dwSize;
    unsigned char  chn;         //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char  enable;      //录像开关:0-不录像,1-录像
    unsigned char  RecordMode;  //录像模式:0-定时录像，1-开机录像
    unsigned char hour[7][24]; //录像周计划:为0表示不录象，为2表示正常录像，1-报警录像
    unsigned char reserver[8];
    unsigned char  PackTime;    //录像打包时间，0-15分钟，1-30分钟，2-45分钟，3-60分钟
    unsigned char  bRecAudio;   //是否录声音
    unsigned char  bSaveFlash;
    unsigned char  bPrerecord;  //是否开启预录 0-close   1-open
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
}VVV_RECORD_CONFIG_S,*PVVV_RECORD_CONFIG;

//录像设置VVV_CMD_SET_REC_CONFIG_EX,VVV_CMD_GET_REC_CONFIG_EX
typedef struct tagVVV_RECORD_CONFIG_EX
{
    unsigned int  dwSize;
    unsigned char  chn;         //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char  enable;      //录像开关:0-不录像,1-录像
    unsigned char  RecordMode;  //录像模式:0-定时录像，1-开机录像
    unsigned char hour[7][96]; //录像周计划:为0表示不录象，为2表示正常录像，1-报警录像
    unsigned char timerrecStatus;
    unsigned char manualrecStatus;    
    unsigned char reserver[6];
    unsigned int  PackTime;    //录像打包时间，0-15分钟，1-30分钟，2-45分钟，3-60分钟
    unsigned char  bRecAudio;   //是否录声音
    unsigned char  bSaveFlash;
    unsigned char  bPrerecord;  //是否开启预录 0-close   1-open
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
}VVV_RECORD_CONFIG_EX_S,*PVVV_RECORD_CONFIG_EX;


//启动手动录像VVV_CMD_RECORD_BEGIN,VVV_CMD_RECORD_STOP
typedef struct tagVVV_RECORD_BEGIN
{
    unsigned int  dwSize;
    unsigned int  chn;         //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int  Trigertype;  //触发类型:0x1-定时,0x02-事件,0x04-手动
    unsigned int  bTimeControl;//是否时间控制
    unsigned int  RecordTime;  //触发后录像时长(秒为单位)，超过该时长后录像自动停止
    unsigned int  bSizeControl;//是否文件大小控制
    unsigned int  RecordSize;  //触发后录像大小,超过该大小录像自动停止
}VVV_RECORD_BEGIN_S;

//录像状态VVV_CMD_GET_REC_STATE
typedef struct tagVVV_RECORD_STATE
{
    unsigned int  dwSize;
    unsigned int  chn;                //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int  chnRecordState;     //当前通道录像状态
    unsigned int  ServerSpace;        //以M为单位
    unsigned int  SpaceFree;          //自由空间
}VVV_RECORD_STATE_S,*PVVV_RECORD_STATE_S;

//服务器解码回放VVV_CMD_SET_DEC,VVV_CMD_GET_DEC
typedef struct tagVVV_DEC
{
    unsigned int  dwSize;
    unsigned int  selectchn; //回放通道选择，每一个通道一个bit位，关闭时为0,比如6(二进制为110)表示回放第二第三通道
    int           Volume;    //-0-100
    int           Mute;      //-0-关闭，1-打开
    int           bSaveFlash;
} VVV_DEC_S,*PVVV_DEC_S;

/*磁盘信息VVV_CMD_GET_DISK_INFO*/
typedef struct tagDISK_ITEM
{
    unsigned char dwDiskID;       //此Item指的是第几个硬盘上的
    unsigned char dwDiskType;     //磁盘类型
    unsigned char dwPartitionIndex; 
    unsigned char dwPartitionType;
    unsigned char dwPartitionState;//磁盘状态,0:无硬盘;1:未格式化;2:正常;3:损坏
    unsigned char resv1;//BTaligned,为了对齐增加, 20140116
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
    unsigned char resv3;//BTaligned,为了对齐增加, 20140116
    unsigned int  dwPartitionSize; //磁盘容量,以M为单位
    unsigned int  dwPartitionFree; //磁盘剩余空间
}DISK_ITEM;
typedef struct tagVVV_DISK_INFO
{
    unsigned int dwSize;
    unsigned int dwItemCount;
    DISK_ITEM    DiskItem[VVV_MAX_DISK_NUM];
}VVV_DISK_INFO_S,*PVVV_DISK_INFO_S;

/*磁盘格式化VVV_CMD_FORMAT_DISK*/
typedef struct tagVVV_FORMAT_DISK
{
    unsigned int dwSize;
    unsigned int DiskType;         //磁盘类型:0-硬盘,1-USB卡,2-SD卡
    unsigned int dwDiskID;          //磁盘ID,每一个磁盘一个bit位
    unsigned int dwPartitionIndex;  //要格式化的磁盘分区
    unsigned int bFastFormat;       //是否进行快速格式化
    unsigned int dwClusterSize;     //格式化簇大小
    unsigned int dwPartitionSize;   //格式分区大小(G)
}VVV_FORMAT_DISK_S;

/*磁盘录像属性VVV_CMD_SET_DISK_MNG,VVV_CMD_GET_DISK_MNG*/
typedef struct tagVVV_DISK_MNG
{
    unsigned int  dwSize;
    unsigned int  Redundancy;/*32个bit, 每个bit对应一个硬盘属性，=0, 主盘， =1,  冗余盘*/
    unsigned int  OverWrite; /*自动覆盖过期录像开启标识，0-关闭,1-自动,2-1小时,3-3小时,4-1天,5-7天,6-30天,7-90天*/
    int           bSaveFlash;
}VVV_DISK_MNG_S,*PVVV_DISK_MNG_S;

/*磁盘状态VVV_CMD_GET_DISK_STATE*/
typedef struct tagVVV_DISK_STATE
{
    unsigned int  dwSize;
    unsigned char DISKState[VVV_MAX_DISK_NUM];//硬盘状态 0:无硬盘;1:未格式化;2:正常;3:损坏
    VVV_U64 DISKTotalSize[VVV_MAX_DISK_NUM];//单位:字节
    VVV_U64 DISKFreeSize[VVV_MAX_DISK_NUM];
    VVV_U64 DISKFreeRecord[VVV_MAX_DISK_NUM];
}VVV_DISK_STATE_S,*PVVV_DISK_STATE_S;

/**********************************************************/
/*     告警参数                                           */
/**********************************************************/
//告警结果设置VVV_CMD_SET_ALARM,VVV_CMD_GET_ALARM
typedef struct tagsensorout_alarm_info/*探头告警输出*/
{
    unsigned char bEnable;     //使能
    unsigned char AlarmOutTime;//探头报警输出持续时间:0，10，20，40，60s
    unsigned char ActiveMode;  //探头告警输出触发模式:0-Open,1-ground
}sensorout_alarm_info;
typedef struct tagVVV_ALARM_SET
{
    unsigned int dwSize;
    unsigned char BuzzerMooTime;           //蜂鸣器鸣叫，鸣叫时间:0，10，20，40，60s
    unsigned char RecordDelayTime;         //录像延时时间:0，10，20，40，60s
    unsigned char SnapDelayTime;           //抓拍延时时间:0，10，20，40，60s
    unsigned char FullSreenTime;           //全屏输出延时，输出时间:0,5,10，30，60，120，300S
    unsigned char PreRecordTime;           //预录像，预录时间:0，10s
    sensorout_alarm_info  stSensorAlarmOut;//探头告警输出设置
    unsigned char bSaveFlash;
    unsigned char resv1;//BTaligned,为了对齐增加, 20140116
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
    unsigned char resv3;//BTaligned,为了对齐增加, 20140116
}VVV_ALARM_SET_S,*PVVV_ALARM_SET_S;

//视频移动告警VVV_CMD_SET_MD_ALARM,VVV_CMD_GET_MD_ALARM
typedef struct tagVVV_CHN_MD_ALARM
{
    unsigned int dwSize;
    unsigned char chn;                //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char bEnable;            //使能
    unsigned char Sensitive;          //灵敏度,4个等级:0-高，1-较高，2-中，3:低
    unsigned char RegionSetting[32]; //区域设置，180个区域，用32个字节表示，每位表示一个区域
    unsigned char hour[7][24];       //布防周计划:为0表示非使能，1-使能
    unsigned char reserver[8];
    unsigned char bBuzzerMoo;         //蜂鸣器鸣叫使能
    unsigned char bAlarmOut;          //报警输出
    unsigned char bRecord[VVV_MAX_CHN_NUM];  //触发录像(可以触发其他通道录像)
    unsigned char bPreRecord;         //是否预录像
    unsigned char bShotSnap;          //是否自动抓图
    unsigned char bPtzMove;           //PTZ联动
    unsigned char bSendEmail;         //是否发Email
    unsigned char bFTP;               //是否FTP传送
    unsigned char bESMS;              //是否发短信
    unsigned char bFullSreen;         //通道全屏
    unsigned char bSendCenter;        //是否发中心服务器
    unsigned char bSaveFlash;
    unsigned char interval;           //告警间隔
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
}VVV_CHN_MD_ALARM_S,*PVVV_CHN_MD_ALARM_S;

typedef struct tagVVV_CHN_MD_ALARM_EX
{
    unsigned int dwSize;
    unsigned char chn;                //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char bEnable;            //使能
    unsigned char Sensitive;          //灵敏度,4个等级:0-高，1-较高，2-中，3:低
    unsigned char RegionSetting[32]; //区域设置，180个区域，用32个字节表示，每位表示一个区域
    unsigned char hour[7][96];       //布防周计划:为0表示非使能，1-使能
    unsigned char bNotifyRec;         //是否notifySDK
    unsigned char reserver[7];
    unsigned char bBuzzerMoo;         //蜂鸣器鸣叫使能
    unsigned char bAlarmOut;          //报警输出
    unsigned char bRecord[VVV_MAX_CHN_NUM];  //触发录像(可以触发其他通道录像)
    unsigned char bPreRecord;         //是否预录像
    unsigned char bShotSnap;          //是否自动抓图
    unsigned char bPtzMove;           //PTZ联动
    unsigned char bSendEmail;         //是否发Email
    unsigned char bFTP;               //是否FTP传送
    unsigned char bESMS;              //是否发短信
    unsigned char bFullSreen;         //通道全屏
    unsigned char bSendCenter;        //是否发中心服务器
    unsigned char bSaveFlash;
    unsigned char interval;           //告警间隔
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
}VVV_CHN_MD_ALARM_EX_S,*PVVV_CHN_MD_ALARM_EX_S;

/*探头输入告警VVV_CMD_SET_SENSOR_IN_ALARM,VVV_CMD_GET_SENSOR_IN_ALARM*/
typedef struct tagVVV_SENSOR_IN_ALARM
{
    unsigned int dwSize;
    unsigned char nSensorID;        //探头索引,0~VVV_MAX_SENSOR_IN,0xFF配置所有探头
    unsigned char bEnable;          //I/O状态报警,0-常开，1-常关，2-关闭
    unsigned char hour[7][24];     //布防周计划:为0表示非使能，1-使能
    unsigned char reserver[8];
    unsigned char bBuzzerMoo;       //蜂鸣器鸣叫使能
    unsigned char bAlarmOut;        //报警输出(可以触发多个告警输出)
    unsigned char bRecord[VVV_MAX_CHN_NUM];  //触发录像(可以触发其他通道录像)
    unsigned char bPreRecord;               //是否预录像
    unsigned char bShotSnap;//是否自动抓图(可以触发其他通道抓图)
    unsigned char bPtzMove; //PTZ联动(可以触发多个PTZ联动)
    unsigned char bSendEmail;               //是否发Email
    unsigned char bFTP;                     //是否FTP传送
    unsigned char bESMS;                    //是否发短信
    unsigned char bFullSreen;               //通道全屏
    unsigned char bSendCenter;              //是否发中心服务器
    unsigned char bSaveFlash;
    unsigned char interval;//BTaligned,为了对齐增加, 20140116
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
    unsigned char resv3;//BTaligned,为了对齐增加, 20140116
}VVV_SENSOR_IN_ALARM_S,*PVVV_SENSOR_IN_ALARM_S;

typedef struct tagVVV_SENSOR_IN_ALARM_EX
{
    unsigned int dwSize;
    unsigned char nSensorID;        //探头索引,0~VVV_MAX_SENSOR_IN,0xFF配置所有探头
    unsigned char bEnable;          //I/O状态报警,0-常开，1-常关，2-关闭
    unsigned char hour[7][96];     //布防周计划:为0表示非使能，1-使能
    unsigned char reserver[8];
    unsigned char bBuzzerMoo;       //蜂鸣器鸣叫使能
    unsigned char bAlarmOut;        //报警输出(可以触发多个告警输出)
    unsigned char bRecord[VVV_MAX_CHN_NUM];  //触发录像(可以触发其他通道录像)
    unsigned char bPreRecord;               //是否预录像
    unsigned char bShotSnap;//是否自动抓图(可以触发其他通道抓图)
    unsigned char bPtzMove; //PTZ联动(可以触发多个PTZ联动)
    unsigned char bSendEmail;               //是否发Email
    unsigned char bFTPSNAP;                     //是否FTP传送SNAP
    unsigned char bFTPREC;                    //是否FTP传送REC
    unsigned char bFullSreen;               //通道全屏
    unsigned char bSendCenter;              //是否发中心服务器
    unsigned char bSaveFlash;
    unsigned char interval;//BTaligned,为了对齐增加, 20140116
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
    unsigned char resv3;//BTaligned,为了对齐增加, 20140116
}VVV_SENSOR_IN_ALARM_EX_S,*PVVV_SENSOR_IN_ALARM_EX_S;

typedef struct tagVVV_MICROWAVE_ALARM
{
    unsigned int dwSize;
    unsigned char bEnable;          //状态报警,0-关闭 1-打开
    unsigned char hour[7][24];     //布防周计划:为0表示非使能，1-使能
    unsigned char reserver[8];
    unsigned char bBuzzerMoo;       //蜂鸣器鸣叫使能
    unsigned char bAlarmOut;        //报警输出(可以触发多个告警输出)
    unsigned char bRecord[VVV_MAX_CHN_NUM];  //触发录像(可以触发其他通道录像)
    unsigned char bPreRecord;               //是否预录像
    unsigned char bShotSnap;//是否自动抓图(可以触发其他通道抓图)
    unsigned char bPtzMove; //PTZ联动(可以触发多个PTZ联动)
    unsigned char bSendEmail;               //是否发Email
    unsigned char bFTP;                     //是否FTP传送
    unsigned char bESMS;                    //是否发短信
    unsigned char bFullSreen;               //通道全屏
    unsigned char bSendCenter;              //是否发中心服务器
    unsigned char bSaveFlash;
}VVV_MICROWAVE_ALARM_S,*PVVV_MICROWAVE_ALARM_S;

typedef struct tagVVV_PIR_ALARM
{
    unsigned int dwSize;
    unsigned char bEnable;          //状态报警,0-常开，1-常关，2-关闭
    unsigned char hour[7][24];     //布防周计划:为0表示非使能，1-使能
    unsigned char reserver[8];
    unsigned char bBuzzerMoo;       //蜂鸣器鸣叫使能
    unsigned char bAlarmOut;        //报警输出(可以触发多个告警输出)
    unsigned char bRecord[VVV_MAX_CHN_NUM];  //触发录像(可以触发其他通道录像)
    unsigned char bPreRecord;               //是否预录像
    unsigned char bShotSnap;//是否自动抓图(可以触发其他通道抓图)
    unsigned char bPtzMove; //PTZ联动(可以触发多个PTZ联动)
    unsigned char bSendEmail;               //是否发Email
    unsigned char bFTP;                     //是否FTP传送
    unsigned char bESMS;                    //是否发短信
    unsigned char bFullSreen;               //通道全屏
    unsigned char bSendCenter;              //是否发中心服务器
    unsigned char bSaveFlash;
}VVV_PIR_ALARM_S,*PVVV_PIR_ALARM_S;


typedef struct tagVVV_HUMI_ALARM
{
    unsigned int dwSize;
	int tempLow;                    // -10~50
	int tempHigh;                   // -10~50
	int RHLow;                      // 0~100
	int RHHigh;	                    // 0~100
	int PM25;                       // high-1;normail-2;low-3	    
    unsigned char bEnable;          //状态报警,0-关闭，0x1-温度报警，0x2-湿度报警，0x4-PM2.5报警,组合0x7即all
    unsigned char hour[7][24];     //布防周计划:为0表示非使能，1-使能
    unsigned char reserver[8];
    unsigned char bBuzzerMoo;       //蜂鸣器鸣叫使能
    unsigned char bAlarmOut;        //报警输出(可以触发多个告警输出)
    unsigned char bRecord[VVV_MAX_CHN_NUM];  //触发录像(可以触发其他通道录像)
    unsigned char bPreRecord;               //是否预录像
    unsigned char bShotSnap;//是否自动抓图(可以触发其他通道抓图)
    unsigned char bPtzMove; //PTZ联动(可以触发多个PTZ联动)
    unsigned char bSendEmail;               //是否发Email
    unsigned char bFTP;                     //是否FTP传送
    unsigned char bESMS;                    //是否发短信
    unsigned char bFullSreen;               //通道全屏
    unsigned char bSendCenter;              //是否发中心服务器
    unsigned char bSaveFlash;
}VVV_HUMI_ALARM_S,*PVVV_HUMI_ALARM_S;

typedef struct tagSHELTER_S
{
	unsigned char bEnable[4];   /* 是否启用 */
    unsigned char Sensitive[4]; //灵敏度,4个等级:0-高，1-较高，2-中，3:低
    int           s32X[4];
    int           s32Y[4];
    unsigned int  u32Width[4];
    unsigned int  u32Height[4];
}SHELTER_S;
//视频遮挡告警VVV_CMD_SET_VIDEO_SHELTER,VVV_CMD_GET_VIDEO_SHELTER
typedef struct tagVVV_VIDEO_SHELTER
{
    unsigned int dwSize;
    unsigned char chn;                //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned char resv1;//BTaligned,为了对齐增加, 20140116
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116    
    unsigned char resv3;//BTaligned,为了对齐增加, 20140116    
	SHELTER_S     chnvc;              //遮盖区域
    unsigned char hour[7][24];        //布防周计划:为0表示非使能，1-使能
    unsigned char reserver[8];
    unsigned char bBuzzerMoo;         //蜂鸣器鸣叫使能
    unsigned char bAlarmOut;          //报警输出
    unsigned char bRecord[VVV_MAX_CHN_NUM];  //触发录像(可以触发其他通道录像)
    unsigned char bPreRecord;         //是否预录像
    unsigned char bShotSnap;          //是否自动抓图
    unsigned char bPtzMove;           //PTZ联动
    unsigned char bSendEmail;         //是否发Email
    unsigned char bFTP;               //是否FTP传送
    unsigned char bESMS;              //是否发短信
    unsigned char bFullSreen;         //通道全屏
    unsigned char bSendCenter;        //是否发中心服务器
    unsigned char bSaveFlash;
    unsigned char resv4; //BTaligned,为了对齐增加, 20140116   
}VVV_VIDEO_SHELTER_S,*PVVV_VIDEO_SHELTER_S;

/*异常报警参数VVV_CMD_SET_EXCEPTION_ALARM,VVV_CMD_GET_EXCEPTION_ALARM*/
typedef struct tagVVV_EXCEPTION_ALARM
{
    unsigned int dwSize;
    unsigned char InvalidAlarmBuzzer; /*磁盘失效报警蜂鸣器开关，0-关闭，1-开启*/
    unsigned char InvalidAlarmOutput; /*磁盘失效报警输出开关，0-关闭，1-开启*/
    unsigned char NoSpaceAlarmBuzzer;/*磁盘空间不足蜂鸣器鸣叫,磁盘剩余空间:关闭，1G,   5G,  10G,  1小时,  5小时,  10小时, 20小时*/
    unsigned char NoSpaceAlarmOutput;/*磁盘空间不足告警输出,磁盘剩余空间:关闭，1G,   5G,  10G,  1小时,  5小时,  10小时, 20小时*/
    unsigned char VideoLossAlarmBuzzer;/*视频丢失报警蜂鸣器开关，0-关闭，1-开启*/
    unsigned char VideoLossAlarmOutput;/*视频丢失报警输出开关，0-关闭，1-开启*/
    unsigned char VideoCoverAlarmBuzzer;/*视频遮挡报警蜂鸣器开关，0-关闭，1-开启*/
    unsigned char VideoCoverAlarmOutput;/*视频遮挡报警输出开关，0-关闭，1-开启*/	
    unsigned char SDLossAlarmBuzzer;/*SD卡失效报警蜂鸣器开关，0-关闭，1-开启*/	
    unsigned char SDLossAlarmOutput;/*SD卡失效报警输出开关，0-关闭，1-开启*/
    unsigned char sd_interval;      /*SD卡触发告警频率,在这个时间段内发生的告警, 不再重新触发*/
    unsigned char bSaveFlash;
}VVV_EXCEPTION_ALARM_S,*PVVV_EXCEPTION_ALARM_S;

/*清除告警VVV_CMD_CLEAR_ALARM*/
typedef struct tagVVV_CLEAR_ALARM
{
    unsigned int dwSize;
    unsigned int bBuzzerMoo;  //蜂鸣器
    unsigned int bFullSreen;  //全屏状态
    unsigned int bAlarmOut;   //报警输出
}VVV_CLEAR_ALARM_S,*PVVV_CLEAR_ALARM_S;


//云台巡航设置VVV_CMD_SET_PTZ_CRUISE,VVV_CMD_GET_PTZ_CRUISE
typedef struct tagVVV_PTZ_CRUISE
{
    unsigned int   dwSize;
    unsigned int   chn;            //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int   nTotalPts;
    unsigned char  Operate[256];   //0-设置预置点,1-清除预置点,2-goto预置点
    unsigned char nPTZPoint[256]; //预置点(最多支持256个预置点)
    unsigned char nPauseSec[256]; //在此点停留时间(秒)
}VVV_PTZ_CRUISE_S,*PVVV_PTZ_CRUISE_S;

//服务器485线的COM配置VVV_CMD_SET_COMINFO,VVV_CMD_GET_COMINFO
typedef struct tagVVV_SERVER_COMINFO
{
    unsigned int  dwSize;
    unsigned int  chn;          //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int  Protocol;     //协议类型，0-Pelco-D，1-Pelco-P
    unsigned int  BaudRate;     //波特率:1200,2400,4800,9600,19200,38400,43000,56000,57600,115200
    unsigned int  nDataBits;    //数据位:8,7,6,5
    unsigned int  nStopBits;    //停止位:1,2
    unsigned int  nParity;      //校验位:0-None，1-Odd，2-Even，3-Mark，4-Space
    unsigned int  Number;       //编号，范围为1--63
    int   nStreamControl;//流控制
    int   bTransferState;//透明传输
    int   nPrePos;       //云台预置位
    int   nCruise;       //云台巡航
    int   nTrack;        //云台轨迹
    int   nPTZSpeed;     //云台速度
    int   nAddress;      //云台地址
    char  szPTZName[VVV_NAME_LEN];//云台协议名称
    char  PtzContent[512];    //协议内容
    int          bSaveFlash;
}VVV_SERVER_COMINFO_S,*PVVV_SERVER_COMINFO_S;

typedef enum tagVVV_PTZ_SPEED_E
{
	VVV_PTZ_SPEED_HIGH,
	VVV_PTZ_SPEED_MID,
	VVV_PTZ_SPEED_LOW,
	VVV_PTZ_SPEED_BUFF
}VVV_PTZ_SPEED_E;

typedef enum tagVVV_PTZ_STEP_E
{
	VVV_PTZ_STEP_SMALL = 50,
	VVV_PTZ_STEP_MID = 100,
	VVV_PTZ_STEP_BIG = 200,
	VVV_PTZ_STEP_BUFF
}VVV_PTZ_STEP_E;

typedef struct tagVVV_PTZ_MOTOR_ATTR
{
    char bMirror;  /*水平转动方向是否反转*/
	char bFlip;    /*竖直转动方向是否反转*/	
	char bMirror_ko;  /*水平转动方向是否反转*/
	char bFlip_ko;    /*竖直转动方向是否反转*/    
	VVV_PTZ_SPEED_E enPanSpd;  /*水平转动速度*/
	VVV_PTZ_SPEED_E enTiltSpd; /*竖直转动速度*/
	int enPanStep; /*水平运动步长*/
	int enTiltStep;/*竖直运动步长*/
	int s32MaxPanStep; /*水平运动最大步长*/
	int s32MaxTiltStep;/*竖直运动最大步长*/
    int s32StayTime;//second
    int s32TourCnt;//巡航次数
	int s32GuardPosEnable;//是否使能看守位
	int s32GuardPosInterval;//看守位间隔
	int s32GuardPos;//看守位置
}VVV_PTZ_MOTOR_ATTR_S;

typedef struct tagVVV_PTZ_RS485_ATTR
{
    char  szPTZName[VVV_NAME_LEN];//云台协议名称
    unsigned char	m_bTrack;
	unsigned char	m_bCruise;
	unsigned char	m_wPrePos;
	unsigned char	m_wPTZSpeed;
	unsigned int    baud; //波特率:1200,2400,4800,9600,19200,38400,43000,56000,57600,115200
	unsigned int 	databits; //数据位:8,7,6,5
	unsigned int 	parity;    //校验位:0-None，1-Odd，2-Even，3-Mark，4-Space
	unsigned int 	stopbits; //停止位:1,2   
    int Address;
}VVV_PTZ_RS485_ATTR_S;

typedef struct tagVVV_PTZ_ATTR
{
	int s32Type;/*云台类型 0:rs485 1:高速电机*/
    VVV_PTZ_MOTOR_ATTR_S stMotorInfo;
    VVV_PTZ_RS485_ATTR_S stRS485Info;
    int s32track[4][4];
}VVV_PTZ_ATTR_S, *PVVV_VVV_PTZ_ATTR_S;

//直接通过COM口收发数据VVV_CMD_SEND_COMDATA
typedef struct tagVVV_COM_DATA
{
    unsigned int  dwSize;
    unsigned int  chn;          //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    unsigned int  bIs485;       //485 or 232
    char          DataBuf[128]; //最大128
}VVV_COM_DATA_S;

/*用户设置VVV_CMD_SET_USERINFO,VVV_CMD_GET_USERINFO*/
typedef enum
{
	VVV_LOCAL_MANUALRECORD_RIGHT = 0,	
	VVV_LOCAL_PTZ_RIGHT, 			
	VVV_LOCAL_PLAYBACK_RIGHT,		
	VVV_LOCAL_SETPARAM_RIGHT,		
	VVV_LOCAL_AUTOROLL_RIGHT,		
	VVV_LOCAL_ZOOM_RIGHT,			
	VVV_LOCAL_SWITCHDEV_RIGHT,		
	VVV_LOCAL_PIP_RIGHT, 			
	VVV_LOCAL_SYS_RIGHT,		
}VVV_LOCAL_PRI_INDEX_E;
typedef enum
{
	VVV_REMOTE_PTZ_RIGHT = 0,			
	VVV_REMOTE_MANUALRECORD_RIGHT,	
	VVV_REMOTE_PLAYBACK_RIGHT,		
	VVV_REMOTE_SETPARAM_RIGHT,		
	VVV_REMOTE_SWITCHDEV_RIGHT, 	
}VVV_REMOTE_PRI_INDEX_E;
typedef struct _userpri_s
{
	unsigned char byLocalRight[10]; /* 权限 */
                        	/*数组1: 录像*/
                        	/*数组2: PTZ控制*/
                        	/*数组3: 录像回放/备份*/
                        	/*数组4: 设置参数*/
                        	/*数组5: 升级/重启/维护/格式化/恢复参数*/
                        	/*数组6: 静音/开音*/
                        	/*数组7:PIP画面*/
                        	/*数组8:自动轮循*/
                        	/*数组9:电子放大*/
	unsigned char byRemoteRight[10]; /* 权限 */
                        	/*数组1: 云台控制*/
                        	/*数组2: 录像/抓拍*/
                        	/*数组3: 回放/下载*/
                        	/*数组4: 设置参数*/
                        	/*数组5: 开/关音*/
	unsigned char byLocalPreviewRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)]; //本地可以预览的通道 0-有权限，1-无权限
	unsigned char byRemotePreviewRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];//远程可以预览的通道 0-有权限，1-无权限
#if 0
	unsigned char byLocalPlaybackRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];//本地可以回放的通道 0-有权限，1-无权限
	unsigned char byRemotePlaybackRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];//远程可以回放的通道 0-有权限，1-无权限
	unsigned char byLocalRecordRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)]; //本地可以录像的通道 0-有权限，1-无权限
	unsigned char byRemoteRecordRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];//远程可以录像的通道 0-有权限，1-无权限
	unsigned char byLocalPTZRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];//本地可以PTZ的通道 0-有权限，1-无权限
	unsigned char byRemotePTZRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];//远程可以PTZ的通道 0-有权限，1-无权限
	unsigned char byLocalBackupRight[ASSIGN_FOUR_BYTES(VVV_MAX_CHN_NUM)];//本地备份权限通道 0-有权限，1-无权限
#endif
}userpri_s;

typedef struct _userpassword_s
{
	char 		  UserName[VVV_NAME_LEN];/*用户名[名字长度]*/
	char 		  UserPsw[VVV_NAME_LEN];/*用户密码*/	
	userpri_s 	  UserPri;
	char 		  DeviceID1[VVV_NAME_LEN];	/*本机编号*/
    unsigned char DeviceID;		/*本机编号*/
    unsigned char  bHavePwd;		/*是否使用密码，0表示不使用，1表示使用*/
	unsigned char  bUserPrew;	/*是否启用预览权限*/
}userpassword_s;
typedef struct tagVVV_SERVER_USER
{
    unsigned int   dwSize;
	int 		   UserId;/*0, 第一个默认名称为Admin*/
    userpassword_s stUser;
    unsigned char  resv; //BTaligned,为了对齐增加, 20140116
    int            bSaveFlash;
}VVV_SERVER_USER_S,*PVVV_SERVER_USER_S;

/*IE客户端用户设置VVV_CMD_SET_IECLIENT_USER,VVV_CMD_GET_IECLIENT_USER*/
typedef struct tagVVV_IE_CLIENT_USER
{
    unsigned int  dwSize;
    unsigned int  BitRate;     /*码率，单位为K*/
    char          UserName[VVV_NAME_LEN];//普通用户名
    char          UserPwd[VVV_NAME_LEN]; //普通用户密码
    char          AdminName[VVV_NAME_LEN];/*管理员名称*/
    char          AdminPwd[VVV_NAME_LEN];/*管理员密码*/
    unsigned int  HavePwd;     /*是否使用密码，0表示不使用，1表示使用*/
    unsigned int  QualityOrFrame;/*0为画质优先(CIF 6FPS)，1为帧率优先(QCIF 25FPS)*/
    int          bSaveFlash;
}VVV_IE_CLIENT_USER_S,*PVVV_IE_CLIENT_USER_S;

/*NVR服务器IP设备管理列表VVV_CMD_SET_IP_DEVLIST,VVV_CMD_GET_IP_DEVLIST*/
typedef struct tagdev_stream_info
{
    unsigned char s32Index;
    unsigned char bSrteamValid;
    char szStream[VVV_NAME_LEN];
}dev_stream_info_t;
typedef struct tagdev_chn_info
{
    unsigned char bChnValid;//使能状态
    char szChn[VVV_NAME_LEN];
    dev_stream_info_t strStreamInfo[2];
}dev_chn_info_t;
typedef struct tagipdev_t
{
    char szDevName[VVV_NAME_LEN];  /*device name*/
    char szDevDescribe[VVV_NAME_LEN];/*device description*/
    char aszIP[VVV_NAME_LEN];      /*IP地址*/
    char szMacaddr[VVV_NAME_LEN]; /*device mac address*/
    char aszHttpPort[VVV_NAME_LEN];/*HTTP 监听端口*/
	char aszMediaPort[VVV_NAME_LEN];   /*媒体端口*/
    char aszUserName[VVV_NAME_LEN];
    char aszPwd[VVV_NAME_LEN];
    dev_chn_info_t strChnInfo[1];
    unsigned char s32Wnd;           /*设备绑定的窗口通道,小于0表示未绑定窗口*/
    unsigned char Nettype; /*网络类型0局域网1广域网*/
    unsigned char Netstate;/*网络状态,0断开;1连接中;2已连接*/
    unsigned char bSaveFlash;
}ipdev_t;
typedef struct tagVVV_IPDEV_LIST
{
    unsigned int  dwSize;
    ipdev_t       IpDevInfo[64];
    int           bSaveFlash;
}VVV_IPDEV_LIST_S,*PVVV_IPDEV_LIST_S;

/*网络地址VVV_CMD_SET_NETWORK,VVV_CMD_GET_NETWORK*/
typedef struct tagVVV_SERVER_NETWORK
{
    unsigned int  dwSize;
    unsigned int  NetworkMode; /*连网方式，0-DHCP，1-PPPOE，2-手动配置*/
    unsigned int  IPAddr[4];  //IP地址
    unsigned int  NetMask[4]; //掩码
    unsigned int  GateWay[4]; //网关
    char          csPPPoEUserName[VVV_NAME_LEN]; //PPPoE用户名
    char          csPPPoEPassword[VVV_NAME_LEN]; //PPPoE密码
    unsigned int  DnsFirst[4]; //第一DNS
    unsigned int  DnsSecond[4];//第二DNS
    unsigned int  HttpPort;    //Http端口 
    unsigned int  DataPort;    //数据端口
    unsigned int  szMacAddr[6];//网卡MAC地址
    int           bSaveFlash;
}VVV_SERVER_NETWORK_S,*PVVV_SERVER_NETWORK_S;

/*网络地址VVV_CMD_SET_NETWORK_PORT,VVV_CMD_GET_NETWORK_PORT*/
typedef struct tagVVV_SERVER_NETWORK_PORT
{
    unsigned int  dwSize;
    unsigned int  HttpPort;    //Http端口 
    unsigned int  RtspPort;    //Rtsp端口
    unsigned int  OnvifPort;   //Onvif端口
    unsigned int  DnsDyn;   
    unsigned int  Port[16];
    int           bSaveFlash;
}VVV_SERVER_NETWORK_PORT_S,*PVVV_SERVER_NETWORK_PORT_S;


//DDNS参数配置VVV_CMD_SET_DDNSCFG,VVV_CMD_GET_DDNSCFG
typedef struct tagVVV_DDNSCFG
{
    unsigned int  dwSize;
    unsigned int  bEnableDDNS;        //是否启用DDNS
    struct
    {
        unsigned int nDDNSType;/*DDNS服务器类型, 域名解析类型：
                                0:3322.org
                                1:dyndns
                                2:perfecteyes
                                3:nvrddns,中控DDNS,易视网
                                4:nightowldvr
                                5:no-ip
                                6:greatek
                                10:联通的。
                                */
        char csDDNSUsername[VVV_NAME_LEN];
        char csDDNSPassword[VVV_NAME_LEN];
        char csDNSAddress[VVV_IP_LEN];//DNS服务器地址，可以是IP地址或域名
        unsigned int dwDNSPort;                //DNS服务器端口，默认为6500
        unsigned int updateTime;               //刷新域名的时间
    }stDDNS;
    int           bSaveFlash;
}VVV_DDNSCFG_S, *PVVV_DDNSCFG_S;

//UPNP配置VVV_CMD_SET_UPNP,VVV_CMD_GET_UPNP
typedef struct tagVVVUPnPConfig
{
    unsigned int dwSize;
    unsigned int bEnable;        /*是否启用upnp*/
    unsigned int dwMode;         /*upnp工作方式.0为自动端口映射，1为指定端口映射*/
    unsigned int dwLineMode;     /*upnp网卡工作方式.0为有线网卡,1为无线网卡*/
    char         csServerIp[VVV_NAME_LEN]; /*upnp映射主机.即对外路由器IP*/
    unsigned int dwDataPort;     /*upnp映射数据端口*/
    unsigned int dwWebPort;      /*upnp映射网络端口*/
    unsigned int dwMobilePort;   /*upnp映射手机端口*/  
    unsigned int dwDataPort1;    /*upnp已映射成功的数据端口*/
    unsigned int dwWebPort1;     /*upnp已映射成功的网络端口*/
    unsigned int dwMobilePort1;  /*upnp映射成功的手机端口*/
	unsigned int wDataPortOK;
	unsigned int wWebPortOK;
    unsigned int wMobilePortOK;
    int          bSaveFlash;
}VVV_UPNP_CONFIG_S;

/*ftp上传参数VVV_CMD_SET_FTP,VVV_CMD_GET_FTP*/
typedef struct tagVVV_FTP_PARAM
{
    unsigned int   dwSize;
    unsigned int   dwEnableFTP;        //是否启动ftp上传功能
    char           csFTPIpAddress[VVV_NAME_LEN];//ftp 服务器
    unsigned int   dwFTPPort;          //ftp端口
    char           sUserName[VVV_NAME_LEN];//用户名
    char           sPassword[VVV_NAME_LEN];//密码
    unsigned int   wTopDirMode;        //0x0 = 使用设备ip地址,0x1 = 使用设备名,0x2 = OFF
    unsigned int   wSubDirMode;        //0x0 = 使用通道号 ,0x1 = 使用通道名,0x2 = OFF
    unsigned int   bAutoUpData;        //是否启用自动上传图片功能
    unsigned int   dwAutoTime;         //自动定时上传时间
    unsigned int   chn;          //通道号0~VVV_MAX_CHN_NUM,如果为0xFF则为所有通道
    int            bSaveFlash;
}VVV_FTP_PARAM_S, *LVVV_FTP_PARAM_S;

/*Email参数VVV_CMD_SET_EMAIL,VVV_CMD_GET_EMAIL*/
typedef struct tagVVV_EMAIL_PARAM
{
    unsigned int  dwSize;
    char          csEmailServer[VVV_NAME_LEN];//服务器地址
    char          csEMailUser[VVV_NAME_LEN];  //用户名
    char          csEmailPass[VVV_NAME_LEN];  //密码
    char          csEmailFrom[VVV_NAME_LEN];
    char          csEmailTo[VVV_EMAIL_NUM][VVV_NAME_LEN];
    char          subject[VVV_NAME_LEN];      //主题
    char          text[128];       //内容
    char          attachment[VVV_NAME_LEN];  //附件地址
    unsigned char bEnableEmail;
    unsigned char resv1;//BTaligned,为了对齐增加, 20140116
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
    unsigned char resv3;//BTaligned,为了对齐增加, 20140116    
    unsigned int  port;             //服务器端口
    unsigned char ssl;              //是否SSl加密, 0--否, 1--是 
    unsigned char logintype;        //登陆类型,0-CramLogin 1-AuthLogin 2-AuthLoginPlain 3-no_login
    unsigned char intervaltime;     /*邮件发送间隔0-180,1-300,2-600s*/
    unsigned char bSaveFlash;
}VVV_EMAIL_PARAM_S, *PVVV_EMAIL_PARAM_S;

/*NTP参数VVV_CMD_SET_NTP,VVV_CMD_GET_NTP*/
typedef struct tagVVV_NTP_PARAM
{
    unsigned int   dwSize;
    unsigned int   bEnableNTP;      //NTP使能
    char           csNtpServer[128];//服务器地址
    unsigned int   port;            //服务器端口
    unsigned int   Interval;        //ntp同步间隔 0:每小时，1:每天 2:每周 3:每月
    int           bSaveFlash;
}VVV_NTP_PARAM_S, *PVVV_NTP_PARAM_S;

//中心服务器VVV_CMD_SET_CENTER_INFO,VVV_CMD_GET_CENTER_INFO
typedef struct tagVVV_CENTER_INFO
{
    unsigned int  dwSize;
    unsigned int  bEnable;
    char          CenterIp[VVV_NAME_LEN];    //中心IP
    unsigned int  dwCenterPort;    //中心端口
    char          csServerNo[VVV_NAME_LEN];  //服务器序列号
    unsigned int  Status;          //服务器连接状态 0为未连接 1为连接中 2连接成功
    char          csUsername[VVV_NAME_LEN];
    char          csUserpass[VVV_NAME_LEN];
    int           bSaveFlash;
}VVV_CENTER_INFO_S;

//Wifi属性VVV_CMD_SET_WIFI,VVV_CMD_GET_WIFI
#define VVV_WIRELESS_WIFI           0x01/*无线网络*/
#define VVV_WIRELESS_TDSCDMA_ZX     0x02
#define VVV_WIRELESS_EVDO_HUAWEI    0x03
#define VVV_WIRELESS_WCDMA          0x04
typedef struct tagVVV_WIFI_CONFIG
{
    unsigned int    dwSize;
    unsigned int    bWifiEnable; // 0: disable, 1:static ip address, 2:dhcp
    char            NetIpAddr[VVV_NAME_LEN];
    char            NetMask[VVV_NAME_LEN];
    char            Gateway[VVV_NAME_LEN];
    char            MAC[VVV_NAME_LEN];
    char            szEssid[VVV_NAME_LEN];
    unsigned int    nSecurity;   //0: none,
                                 //1:wep
                                 //wpa/wpa2
    unsigned int  byMode;  //1. managed 2. ad-hoc
    unsigned int  byStatus;//0.成功,其他值是错误码
    unsigned int  byRes;
    char          szWebKey[VVV_NAME_LEN];
    int           bSaveFlash;
}VVV_WIFI_CONFIG_S,*PVVV_WIFI_CONFIG_S;

//3G网络属性VVV_CMD_SET_3G,VVV_CMD_GET_3G
typedef struct tagVVV_3G_CONFIG
{
    unsigned int    dwSize;
    unsigned int    bCdmaEnable;
    unsigned int    byDevType;
    unsigned int    byStatus;
    char            NetIpAddr[VVV_NAME_LEN]; //IP  (wifi enable)
    int           bSaveFlash;
}VVV_3G_CONFIG_S,*PVVV_3G_CONFIG_S;

/*手机点播属性VVV_CMD_SET_MOBILE,VVV_CMD_GET_MOBILE*/
typedef struct tagVVV_MOBILE_CONFIG
{
    unsigned int    dwSize;
    unsigned int    chn;
    char name[16];     //手机用户名
    char pwd[16];      //密码
    int  port;         //端口号
    int  quality;      //质量0最优，1优良，2好，3一般
    int           bSaveFlash;
}VVV_MOBILE_CONFIG_S,*PVVV_MOBILE_CONFIG_S;

typedef struct tagVVV_P2P_CONFIG	
{
	unsigned int dwSize;
	char	param[128];
    int           bSaveFlash;
}VVV_P2P_CONFIG_S,*PVVV_P2P_CONFIG_S;

/*温湿度VVV_CMD_SET_HUMITURE,VVV_CMD_GET_HUMITURE*/
typedef struct tagVVV_HUMITURE_S
{
    unsigned int  dwSize;
	int humidity;
	int temperature;
    int pm25;
    int resv[6];
    int           bSaveFlash;
}VVV_HUMITURE_S,*PVVV_HUMITURE_S;

//RFID 配置VVV_CMD_SET_RFID_INFO,VVV_CMD_GET_RFID_INFO
typedef struct tagVVVRfidConfig
{
    unsigned int dwSize;
    char		 action;         /*0-注销; 1-开始注册;-1-停止注册;2-离开;3-进入;4-新注册,5-注销*/
	char         power;
    char 		 resv1;
    char 		 resv2;
    char         csRfid[VVV_NAME_LEN]; /*Rfid卡的id 号*/
    int          bSaveFlash;
}VVV_Rfid_CONFIG_S, *PVVV_Rfid_CONFIG_S;
//配置VVV_CMD_SET_BUZZER,VVV_CMD_GET_BUZZER
typedef struct tagVVVBuzzerConfig
{
    unsigned int dwSize;
    char		 action;        /*0-关闭; 1-打开*/
    unsigned char resv1;//BTaligned,为了对齐增加, 20140116
    unsigned char resv2;//BTaligned,为了对齐增加, 20140116
    unsigned char resv3;//BTaligned,为了对齐增加, 20140116
    
    int          bSaveFlash;
}VVV_BUZZER_CONFIG, *PVVV_BUZZER_CONFIG;

//配置ACODEC音频芯片 
//VVV_CMD_SET_AUDIOOUT_VOL VVV_CMD_GET_AUDIOOUT_VOL
//VVV_CMD_SET_AUDIOIN_GAIN VVV_CMD_GET_AUDIOIN_GAIN
typedef struct tagVVVACODECCFG
{
	int s32AcodecDev;
	int s32Value;
}VVV_ACODEC_CONFIG, *PVVV_ACODEC_CONFIG;

//配置ACODEC音频芯片 
//VVV_CMD_ENABLE_AEC VVV_CMD_DISABLE_AEC
typedef struct tagVVVACODECAEC
{
	int s32AIDevID;
	int s32AIChn;
	int s32AODevID;
	int s32AOChn;
}VVV_ACODEC_AEC_CONFIG, *PVVV_ACODEC_AEC_CONFIG;


/* 控制命令回调*/
typedef int (*VVV_ON_CMD_CALLBACK)(unsigned int dwCmd,       /* 命令*/
                                  int          socket,      /*网传Socket*/
                                  void         *lpInData,   /* 数据 */
                                  unsigned int Size,        /* 输入数据大小*/
                                  void         *lpOutData,  /* 返回数据 */
                                  unsigned int *pSize,      /* 返回数据大小*/
                                  char         Username[VVV_NAME_LEN],/* 用户名*/
                                  char         Password[VVV_NAME_LEN],/* 密码*/
                                  void         *pUserdata); /* 用户数据*/

//字节对齐
#pragma pack ()


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  //__VVV_NET_DEFINE_H__

