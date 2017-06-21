#ifndef _FHDEV_NET_DEFINE_H_
#define _FHDEV_NET_DEFINE_H_

#ifdef WIN32    // win32

#else           // linux

#ifndef BOOL
#   define BOOL         int
#endif

#ifndef DWORD
#   define DWORD        unsigned int
#endif

#ifndef LPDWORD
#   define LPDWORD      unsigned int*
#endif

#ifndef WORD
#   define WORD         unsigned short
#endif

#ifndef BYTE
#   define BYTE         unsigned char
#endif

#ifndef LPVOID
#   define LPVOID       void*
#endif

#ifndef DWORD64
#   define DWORD64      unsigned long long
#endif

#ifndef COLORREF
#   define COLORREF     unsigned int
#endif

#ifndef TRUE
#   define TRUE    1
#endif
#ifndef FALSE
#   define FALSE   0
#endif
#ifndef NULL
#   define NULL    0
#endif

#endif



#define FHNP_MACRO_NAMELEN_MAX              32      // 用户名最大长度
#define FHNP_MACRO_COVER_MAX                4       // 区域屏蔽最大个数
#define FHNP_MACRO_IPLEN_MAX                32      // IP地址最大长度
#define FHNP_MACRO_DOMAINLEN_MAX            128     // 网址最大长度
#define FHNP_MACRO_LOGLEN_MAX               256     // 日志内容最大长度
#define FHNP_MACRO_CHANNEL_MAX              32      // 最多支持32个通道
#define FHNP_MACRO_DAILY_SCHED_ITEM_MAX     3       // 定时录像每天最多设置的录像段数

//////////////////////////////////////////////////////////////////////////
/* 全局错误码 */
#define FHNPEN_OK                           0       // 没有错误
#define FHNPEN_ERR_SetConfig                1       // 设置/操作失败
#define FHNPEN_ERR_GetConfig                2       // 获取失败
#define FHNPEN_ERR_GetData                  3       // 获取的数据有误
#define FHNPEN_ERR_Parameter                4       // 参数有误
#define FHNPEN_ERR_Authority                5       // 无权限
#define FHNPEN_ERR_UserInexistence          6       // 用户不存在
#define FHNPEN_ERR_Password                 7       // 密码错误
#define FHNPEN_ERR_ReLogin                  8       // 重复登录
#define FHNPEN_ERR_Upgrading                10      // 升级中
#define FHNPEN_ERR_OutOfMemory              11      // 内存不足
#define FHNPEN_ERR_RecvOverTime             12      // 接收超时
#define FHNPEN_ERR_Upgrade_DataError        13      // 升级包数据错误
#define FHNPEN_ERR_Upgrade_SameVersion      14      // 升级包与设备版本一样
#define FHNPEN_ERR_Upgrade_OldVersion       15      // 升级包为旧版本
#define FHNPEN_ERR_UpgradeErr               16      // 升级失败
#define FHNPEN_ERR_Upgrade_WriteFlashFail   17      // 升级过程中写flash失败
#define FHNPEN_ERR_NotSupport               20      // 功能不支持
/* 本地SDK错误码 */
#define FHNPEN_ERR_SDK_Index                2000
#define FHNPEN_ERR_SDK_NoInit               (FHNPEN_ERR_SDK_Index + 1)  // 没有初始化
#define FHNPEN_ERR_SDK_NoLogin              (FHNPEN_ERR_SDK_Index + 2)  // 用户没有登录
#define FHNPEN_ERR_SDK_NoHandle             (FHNPEN_ERR_SDK_Index + 3)  // 传入的句柄无效
#define FHNPEN_ERR_SDK_ParamIn              (FHNPEN_ERR_SDK_Index + 4)  // 传入参数不合法
#define FHNPEN_ERR_SDK_RecvPacketSize       (FHNPEN_ERR_SDK_Index + 5)  // 接收到的数据包大小不符
#define FHNPEN_ERR_SDK_NetCreate            (FHNPEN_ERR_SDK_Index + 6)  // 创建SOCKET失败
#define FHNPEN_ERR_SDK_NetConf              (FHNPEN_ERR_SDK_Index + 7)  // SOCKET属性配置失败
#define FHNPEN_ERR_SDK_NetBind              (FHNPEN_ERR_SDK_Index + 8)  // SOCKET绑定失败
#define FHNPEN_ERR_SDK_NetException         (FHNPEN_ERR_SDK_Index + 9)  // 目标设备连不上SOCKET异常
#define FHNPEN_ERR_SDK_NetConn              (FHNPEN_ERR_SDK_Index + 10) // 无法连接
#define FHNPEN_ERR_SDK_NetSend              (FHNPEN_ERR_SDK_Index + 11) // 发送失败
#define FHNPEN_ERR_SDK_NetRecv              (FHNPEN_ERR_SDK_Index + 12) // 接收超时
#define FHNPEN_ERR_SDK_OpenFile             (FHNPEN_ERR_SDK_Index + 13) // 文件打开失败
#define FHNPEN_ERR_SDK_OutOfMemory          (FHNPEN_ERR_SDK_Index + 14) // 内存申请失败
#define FHNPEN_ERR_SDK_NotSupport           (FHNPEN_ERR_SDK_Index + 15) // 功能不支持
#define FHNPEN_ERR_SDK_IllegalDevice        (FHNPEN_ERR_SDK_Index + 16) // 非法设备
#define FHNPEN_ERR_SDK_Unknow               (FHNPEN_ERR_SDK_Index + 40) // 未知错误


//////////////////////////////////////////////////////////////////////////
/* 配置类型 */
#define FHNPEN_CFGType_EncodeOSD            1    // 编码OSD
#define FHNPEN_CFGType_EncodeCover          2    // 编码区域屏蔽
#define FHNPEN_CFGType_EncodeAudio          4    // 音频编码配置
#define FHNPEN_CFGType_SystemConfig         6    // 系统配置
#define FHNPEN_CFGType_DSTConfig            7    // 夏令时配置
#define FHNPEN_CFGType_NTPConfig            8    // NTP配置
#define FHNPEN_CFGType_SystemTime           9    // 设备时间
#define FHNPEN_CFGType_TimeZone             10   // 设备时区
#define FHNPEN_CFGType_NetConfig            11   // 网络配置
#define FHNPEN_CFGType_SerialPortConfig     12   // 串口配置
#define FHNPEN_CFGType_AutoRebootConfig     14   // 自动维护配置
#define FHNPEN_CFGType_StoragePolicy        15   // 盘组存储策略配置
#define FHNPEN_CFGType_RecordPlan           16   // 定时录像计划
#define FHNPEN_CFGType_WifiConfig           33   // wifi配置


//////////////////////////////////////////////////////////////////////////
/* 上报消息类型(设备消息) */
#define FHNPEN_SYS_Kick                     1   // 被踢除
#define FHNPEN_SYS_OffLine                  2   // 与设备断开连接(心跳停止)
#define FHNPEN_SYS_ShutDown                 4   // 设备关机
#define FHNPEN_SYS_Reboot                   5   // 设备重启
#define FHNPEN_SYS_Reset                    6   // 设备恢复出厂设置
#define FHNPEN_SYS_Upgrade                  7   // 设备升级
#define FHNPEN_SYS_ReConnect                13  // 设备重连成功
#define FHNPEN_SYS_ImportConfig             14  // 导入配置
/* 上报消息类型(报警消息) */
#define FHNPEN_ALM_SDError                  80  // SD卡错误
#define FHNPEN_ALM_SDFull                   81  // SD卡满
#define FHNPEN_ALM_VideoLost                86  // 信号丢失
#define FHNPEN_ALM_IPConflict               88  // IP冲突

//////////////////////////////////////////////////////////////////////////
/* 传输模式 */
#define FHNPEN_TransMode_TCP                0   // TCP
#define FHNPEN_TransMode_UDP				1   // UDP



/* 用户组 */
typedef enum
{
    FHNPEN_UG_Admin = 0x01,     // 管理员
    FHNPEN_UG_User,             // 用户
}FHNPEN_UserGroup_e;


/* 日期格式 */
typedef enum
{
    FHNPEN_TF_Y_M_D = 0,      // YYYY-MM-DD(年-月-日)
    FHNPEN_TF_M_D_Y,          // MM-DD-YYYY(月-日-年)
    FHNPEN_TF_D_M_Y,          // DD-MM-YYYY(日-月-年)
}FHNPEN_TimeFormat_e;


/* 小时制式 */
typedef enum
{
    FHNPEN_TS_24 = 0,         // 24小时制
    FHNPEN_TS_12,             // 12小时制
}FHNPEN_TimeStandard_e;


/* 区域屏蔽类型 */
typedef enum
{
    FHNPEN_CDTYPE_COLOR = 0,  // 色块
    FHNPEN_CDTYPE_MOSAIC,     // 马赛克
}FHNPEN_CoverType_e;


/* 编码分辨率 */
typedef enum
{
    FHNPEN_ER_QCIF = 0,     // QCIF
    FHNPEN_ER_CIF,          // CIF
    FHNPEN_ER_HALFD1,       // HALFD1
    FHNPEN_ER_4CIF,         // 4CIF
    FHNPEN_ER_D1,           // D1
}FHNPEN_EncResolution_e;


/* 视频编码格式 */
typedef enum
{
    FHNPEN_VF_H264 = 0,     // H264
}FHNPEN_EncVideoFormat_e;


/* 帧类型 */
typedef enum
{
    FHNPEN_FType_IFrame = 0,    // I FRAME
    FHNPEN_FType_PFrame,        // P FRAME
    FHNPEN_FType_BFrame,        // B FRAME
    FHNPEN_FType_AFrame,        // AUDIO FRAME
    FHNPEN_FType_JFrame,        // JPEG FRAME
} FHNPEN_FrameType_e;


/* 码率控制类型 */
typedef enum
{
    FHNPEN_BRCTRL_FIXQP = 0,    // FixQP
    FHNPEN_BRCTRL_CBR,          // CBR
    FHNPEN_BRCTRL_VBR,          // VBR
}FHNPEN_EncBRCtrl_e;


/* 去噪级别 */
typedef enum
{
    FHNPEN_DENOISE_OFF = 0,      // 关闭
    FHNPEN_DENOISE_SUPER_WEAK,   // 弱
    FHNPEN_DENOISE_WEAK,         // 较弱
    FHNPEN_DENOISE_NORMAL,       // 一般
    FHNPEN_DENOISE_STRONG,       // 较强
    FHNPEN_DENOISE_SUPER_STRONG, // 强
}FHNPEN_EncDenoise_e;


/* 去隔行类型 */
typedef enum
{
    FHNPEN_DEINT_NONE = 0,        // 关闭
    FHNPEN_DEINT_5TAP_WEAK,       // 5阶滤波(弱)
    FHNPEN_DEINT_5TAP_MEDIUM,     // 5阶滤波(中)
    FHNPEN_DEINT_5TAP_STRONG,     // 5阶滤波(强)
    FHNPEN_DEINT_5TAP_SUPER_STRONG,// 5阶滤波(超强)
    FHNPEN_DEINT_ADAPT_AVG,       // Adapt AVG
    FHNPEN_DEINT_MEDIUM,          // Medium
    FHNPEN_DEINT_ADAPT_MEDIAN,    // Adapt Medium
    FHNPEN_DEINT_MA3,             // MA3
}FHNPEN_EncDeinterlace_e;


/* 音频编码格式 */
typedef enum
{
    FHNPEN_AF_G711_ALAW = 0,    // G711 ALAW
    FHNPEN_AF_G711_ULAW,        // G711 ULAW
    FHNPEN_AF_PCM,              // PCM
    FHNPEN_AF_G726,             // G726
    FHNPEN_AF_AMR,              // AMR
    FHNPEN_AF_AMRDTX,           // AMRDTX
    FHNPEN_AF_AAC,              // AAC
}FHNPEN_EncAudioFormat_e;


/* 音频采样率 */
typedef enum
{
    FHNPEN_ASRATE_8000 = 8000,
    FHNPEN_ASRATE_11025 = 11025,
    FHNPEN_ASRATE_16000 = 16000,
    FHNPEN_ASRATE_22050 = 22050,
    FHNPEN_ASRATE_24000 = 24000,
    FHNPEN_ASRATE_32000 = 32000,
    FHNPEN_ASRATE_44100 = 44100,
    FHNPEN_ASRATE_48000 = 48000,
}FHNPEN_EncAudioSampleRate_e;


/* 音频位宽 */
typedef enum
{
    FHNPEN_ABITWIDTH_8 = 0,         // 8bit
    FHNPEN_ABITWIDTH_16,            // 16bit
    FHNPEN_ABITWIDTH_32,            // 32bit
}FHNPEN_EncAudioBitWidth_e;


/* 音频声道 */
typedef enum
{
    FHNPEN_ATRACK_MONO = 0,          // 单声道
    FHNPEN_ATRACK_STEREO,            // 立体声
}FHNPEN_EncAudioTrack_e;


/* 视频输入格式 */
typedef enum
{
    FHNPEN_VideoInputMode_PAL = 0,   // PAL
    FHNPEN_VideoInputMode_NTSC,      // NTSC
}FHNPEN_VideoInputMode_e;


/* 夏令时模式 */
typedef enum
{
    FHNPEN_DSTMode_Date = 0,         // 日期模式
    FHNPEN_DSTMode_Week,             // 星期模式
}FHNPEN_DSTMode_e;


/* 时区 */
typedef enum
{
    FHNPEN_TIMEZONE_GMT_1200 = 0,
    FHNPEN_TIMEZONE_GMT_1100,
    FHNPEN_TIMEZONE_GMT_1000,
    FHNPEN_TIMEZONE_GMT_0900,
    FHNPEN_TIMEZONE_GMT_0800,
    FHNPEN_TIMEZONE_GMT_0700,
    FHNPEN_TIMEZONE_GMT_0600,
    FHNPEN_TIMEZONE_GMT_0500,
    FHNPEN_TIMEZONE_GMT_0430,
    FHNPEN_TIMEZONE_GMT_0400,
    FHNPEN_TIMEZONE_GMT_0330,
    FHNPEN_TIMEZONE_GMT_0300,
    FHNPEN_TIMEZONE_GMT_0200,
    FHNPEN_TIMEZONE_GMT_0100,
    FHNPEN_TIMEZONE_GMT,
    FHNPEN_TIMEZONE_GMT0100,
    FHNPEN_TIMEZONE_GMT0200,
    FHNPEN_TIMEZONE_GMT0300,
    FHNPEN_TIMEZONE_GMT0330,
    FHNPEN_TIMEZONE_GMT0400,
    FHNPEN_TIMEZONE_GMT0430,
    FHNPEN_TIMEZONE_GMT0500,
    FHNPEN_TIMEZONE_GMT0530,
    FHNPEN_TIMEZONE_GMT0545,
    FHNPEN_TIMEZONE_GMT0600,
    FHNPEN_TIMEZONE_GMT0630,
    FHNPEN_TIMEZONE_GMT0700,
    FHNPEN_TIMEZONE_GMT0800,
    FHNPEN_TIMEZONE_GMT0900,
    FHNPEN_TIMEZONE_GMT0930,
    FHNPEN_TIMEZONE_GMT1000,
    FHNPEN_TIMEZONE_GMT1100,
    FHNPEN_TIMEZONE_GMT1200,
    FHNPEN_TIMEZONE_GMT1300,
}FHNPEN_TimeZone_e;


/* 校验 */
typedef enum
{
    FHNPEN_PARITY_NONE = 0,     // 无校验
    FHNPEN_PARITY_ODD,          // 奇校验
    FHNPEN_PARITY_EVEN,         // 偶校验
}FHNPEN_Parity_e;


/* 数据流控制 */
typedef enum
{
    FHNPEN_FCTRL_NONE = 0,      // 无
    FHNPEN_FCTRL_XONXOFF,       // Xon / Xoff
    FHNPEN_FCTRL_HARD,          // 硬件
}FHNPEN_FlowControl_e;


/* 自动维护模式 */
typedef enum
{
    FHNPEN_ARMode_Disable = 0,  // 禁用
    FHNPEN_ARMode_EveryDay,     // 每天循环
    FHNPEN_ARMode_EveryWeek,    // 每周循环
    FHNPEN_ARMode_EveryMonth,   // 每月循环
}FHNPEN_AutoRebootMode_e;


/* 日志类型 */
typedef enum
{
    FHNP_LogType_All = 0,       // 所有日志类型
    FHNP_LogType_System,        // 系统日志类型
    FHNP_LogType_Operate,       // 操作日志类型
    FHNP_LogType_Alarm,         // 报警日志类型
}FHNP_LogType_e;


/* 回调流类型 */
typedef enum
{
    FHNPEN_ST_FRAME = 0,        // 帧类型
    FHNPEN_ST_TS,               // TS类型
}FHNPEN_StreamType_e;


/* 升级状态 */
typedef enum
{
    FHNPEN_UState_Finish = 0x01, // 升级完成
    FHNPEN_UState_Sending,       // 发送中
    FHNPEN_UState_Waiting,       // 等待设备更新
    FHNPEN_UState_Fail,          // 升级失败
    FHNPEN_UState_Unknown,       // 未知错误
}FHNPEN_UpgradeState_e;


/* WIFI工作模式 */
typedef enum
{
    FHNPEN_WLAN_MODE_INFRASTRUCTURE = 0,
    FHNPEN_WLAN_MODE_ADHOC,
    FHNPEN_WLAN_MODE_UAP,
    FHNPEN_WLAN_MODE_P2P,
    FHNPEN_WLAN_MODE_P2P_UAP,
}FHNPEN_WifiMode_e;


/* WIFI安全类型 */
typedef enum 
{
	FHNPEN_WLAN_SECURITY_NONE = 0,     // The network does not use security
	FHNPEN_WLAN_SECURITY_WEP_OPEN,     // The network uses WEP security with opn key
	FHNPEN_WLAN_SECURITY_WEP_SHARED,   // The network uses WEP security with shared key
	FHNPEN_WLAN_SECURITY_WPA,          // The network uses WPA security with PSK
	FHNPEN_WLAN_SECURITY_WPA2,         // The network uses WPA2 security with PSK
}FHNPEN_WifiSecurityType_e;



/* 回放控制 */
typedef enum
{
    FHNPEN_PBCtrl_Play = 1,      // 播放
    FHNPEN_PBCtrl_Stop,          // 停止
    FHNPEN_PBCtrl_Pause,         // 暂停
    FHNPEN_PBCtrl_Continue,      // 暂停情况下继续播放
    FHNPEN_PBCtrl_Slow,          // 慢速(当前速度/2)
    FHNPEN_PBCtrl_Fast,          // 快速(当前速度*2)
    FHNPEN_PBCtrl_SetPlaySpeed,  // 设置播放速度(FHNPEN_PlaySpeed_e)
    FHNPEN_PBCtrl_FramePrev,     // 跳转到上一个I帧
    FHNPEN_PBCtrl_FrameNext,     // 跳转到下一个I帧
    FHNPEN_PBCtrl_NOIFrameNext,  // 跳转到下一帧(非I帧)
}FHNPEN_PlayCtrl_e;


/* 回放速率 */
typedef enum
{
    FHNPEN_PBSpeed_Normal = 0,    // 正常速率
    FHNPEN_PBSpeed_2,             // 2倍速率
    FHNPEN_PBSpeed_4,             // 4倍速率
    FHNPEN_PBSpeed_8,             // 8倍速率
    FHNPEN_PBSpeed_16,            // 16倍速率
    FHNPEN_PBSpeed_32,            // 32倍速率(不支持)
    FHNPEN_PBSpeed_64,            // 64倍速率(不支持)
    FHNPEN_PBSpeed_1_2,           // 1/2倍速率
    FHNPEN_PBSpeed_1_4,           // 1/4倍速率
    FHNPEN_PBSpeed_1_8,           // 1/8倍速率
    FHNPEN_PBSpeed_1_16,          // 1/16倍速率
    FHNPEN_PBSpeed_1_32,          // 1/32倍速率(不支持)
    FHNPEN_PBSpeed_1_64,          // 1/64倍速率(不支持)
}FHNPEN_PlaySpeed_e;

/* 硬盘状态 */
typedef enum
{
    FHNPEN_SDCardState_FOUND        = (1 << 0),     // 找到SD卡
    FHNPEN_SDCardState_LOADED       = (1 << 1),     // SD卡已加载
    FHNPEN_SDCardState_NORMAL       = (1 << 2),     // SD卡正常
    FHNPEN_SDCardState_FORMATING    = (1 << 3),     // 格式化过程中
}FHNPEN_SDCardState_e;


/* 硬盘格式化类型 */
typedef enum
{
    FHNPEN_FmtType_Fast = 0,  // 快速格式化
    FHNPEN_FmtType_Slow,      // 慢速格式化
    FHNPEN_FmtType_Low,       // 低速格式化
}FHNPEN_FormatType_e;



/* 录像类型 */
typedef enum
{
    FHNPEN_RecType_All    = 0,            // 所有录像
    FHNPEN_RecType_Manual = (1 << 0),     // 手动录像
    FHNPEN_RecType_Time   = (1 << 1),     // 定时录像
    FHNPEN_RecType_Alarm  = (1 << 2),     // 报警录像
}FHNPEN_RecordType_e;


/* 截图类型 */
typedef enum
{
    FHNPEN_PicType_All    = 0,            // 所有截图
    FHNPEN_PicType_Manual = (1 << 0),     // 手动截图
    FHNPEN_PicType_Time   = (1 << 1),     // 定时截图
    FHNPEN_PicType_Alarm  = (1 << 2),     // 报警截图
}FHNPEN_PictureType_e;



typedef enum
{
    FHNPEN_EState_Finish = 0x01, // 操作成功
    FHNPEN_EState_Ing,           // 执行过程中
    FHNPEN_EState_Fail,          // 失败
    FHNPEN_EState_Error,         // 未知错误
}FHNPEN_DurationState_e;






/* IP地址结构体 */
typedef struct
{
    char sIPV4[16];     // IPv4地址
    BYTE sIPV6[128];    // 保留
} FHNP_IPAddr_t, *LPFHNP_IPAddr_t;


/* 矩形尺寸 */
typedef struct 
{
    int x;          // x坐标
    int y;          // y坐标
    int w;          // 宽
    int h;          // 高
} FHNP_Rect_t, *LPFHNP_Rect_t;


/* 系统时间 */
typedef struct 
{
    WORD  year;     // 年(1970-2038)
    BYTE  month;    // 月(1-12)
    BYTE  day;      // 日(1-31)
    BYTE  wday;     // 星期(0-6)(0-星期日, 1-星期一...)
    BYTE  hour;     // 时(0-23)
    BYTE  minute;   // 分(0-59)
    BYTE  second;   // 秒(0-59)
    DWORD msecond;  // 毫秒(0-999)
} FHNP_Time_t, *LPFHNP_Time_t;


/* 用户信息 */
typedef struct 
{
    char chUser[FHNP_MACRO_NAMELEN_MAX];    // 用户名
    char chPassword[FHNP_MACRO_NAMELEN_MAX];// 用户密码
    BYTE btUserGroup;                       // 用户组(FHNPEN_UserGroup_e)
    BYTE btUserOnline;                      // 是否在线
    BYTE btRes[2];                          // 保留
} FHNP_User_t, *LPFHNP_User_t;


/* 编码区域屏蔽 */
typedef struct  
{
    BYTE btEnable;                              // 使能
    BYTE btCoverType;                           // 区域屏蔽类型(FHNPEN_CoverType_e)
    BYTE btRes[2];                              // 保留
    FHNP_Rect_t stRect[FHNP_MACRO_COVER_MAX];   // 区域屏蔽区域(基于720x576)
} FHNP_EncodeCover_t, *LPFHNP_EncodeCover_t;


/* 视频编码配置 */
typedef struct 
{
    BYTE btResolution;          // 编码分辨率(FHNPEN_EncResolution_e)
    BYTE btBRCtrl;              // 码率控制(FHNPEN_EncBRCtrl_e)
    BYTE btDenoise;             // 去噪(FHNPEN_EncDenoise_e)
    BYTE btDeinter;             // 去隔行(FHNPEN_EncDeinterlace_e)
    int  iBitRate;              // 码率
    int  iFrameRate;            // 帧率
    int  iGOP;                  // I帧间隔
} FHNP_EncodeVideo_t, *LPFHNP_EncodeVideo_t;


/* 编码OSD配置 */
typedef struct 
{
    BYTE btShowTimeOSD;                     // 是否显示TimeOSD
    BYTE btTimeFmt;                         // 日期格式(FHNPEN_TimeFormat_e)
    BYTE btTimeStd;                         // 小时制式(FHNPEN_TimeStandard_e)
    BYTE btRes;                             // 保留
    int  iTimeOSDX, iTimeOSDY;              // TimeOSD坐标(基于720x576)
    COLORREF colTimeOSD;                    // TimeOSD颜色

    BYTE btShowCustomTOSD;                  // 是否显示自定义TOSD
    BYTE btRes1[3];                         // 保留
    int  iCustomTOSDX, iCustomTOSDY;        // 自定义TOSD坐标(基于720x576)
    char chCustomTOSD[FHNP_MACRO_NAMELEN_MAX]; // 自定义TOSD字符串
    COLORREF colCustomOSD;                  // 自定义TOSD颜色   
} FHNP_EncodeOSD_t, *LPFHNP_EncodeOSD_t;


/* 数字水印配置 */
typedef struct
{
    BYTE btEnable;              // 使能
    BYTE btRes[3];              // 保留
    char chText[256];           // 水印内容
} FHNP_EncodeWaterMark_t, *LPFHNP_EncodeWaterMark_t;


/* 音频编码配置 */
typedef struct 
{
    BYTE  btAudioFmt;           // 音频编码格式(FHNPEN_EncAudioFormat_e)
    BYTE  btBitWidth;           // 位宽(FHNPEN_EncAudioBitWidth_e)
    BYTE  btAudioTrack;         // 声道(FHNPEN_EncAudioTrack_e)
    BYTE  btRes;                // 保留
    DWORD dwSampleRate;         // 采样率(FHNPEN_EncAudioSampleRate_e)    
    DWORD dwDataLen;            // 数据长度
} FHNP_EncodeAudio_t, *LPFHNP_EncodeAudio_t;


/* 系统配置 */
typedef struct 
{
    char chDeviceName[FHNP_MACRO_DOMAINLEN_MAX];    // 设备名称
    int  iDeviceIndex;                              // 设备编号
    BYTE btVideoInputMode;                          // 视频输入制式(FHNPEN_VideoInputMode_e)
    BYTE btRes[3];                                  // 保留
} FHNP_DeviceConfig_t, *LPFHNP_DeviceConfig_t;


/* 夏令时星期模式下的时间格式 */
typedef struct 
{
    BYTE month;                 // 月份[1-12]
    BYTE wdayIndex;             // 第几个星期[1-5]
    BYTE wDay;                  // 星期几[0-6]
    BYTE hour;                  // 小时[0-23]
    BYTE minute;                // 分钟[0-59]
    BYTE btRes[3];              // 保留
} FHNP_DSTWeekModeTime_t, *LPFHNP_DSTWeekModeTime_t;


/* 夏令时日期格式 */
typedef struct 
{
    FHNP_Time_t stStartTime;    // 开始时间
    FHNP_Time_t stStopTime;     // 结束时间
} FHNP_DSTDateMode_t, *LPFHNP_DSTDateMode_t;


/* 夏令时星期模式 */
typedef struct 
{
    FHNP_DSTWeekModeTime_t stStartTime; // 开始时间
    FHNP_DSTWeekModeTime_t stStopTime;  // 结束时间
} FHNP_DSTWeekMode_t, *LPFHNP_DSTWeekMode_t;


/* 夏令时 */
typedef struct 
{
    BYTE btDST;                         // 使能
    BYTE btDSTMode;                     // 夏令时类型(FHNPEN_DSTMode_e)
    BYTE btRes[2];                      // 保留
    FHNP_DSTDateMode_t stDSTDateMode;   // 日期模式
    FHNP_DSTWeekMode_t stDSTWeekMode;   // 星期模式
} FHNP_DST_t, *LPFHNP_DST_t;


/* 设备性能参数 */
typedef struct 
{
    char chSerialNum[FHNP_MACRO_NAMELEN_MAX];// 序列号
    BYTE btDevType;         // 设备类型(FHNPEN_DevType_e)
    BYTE btRes[3];          // 保留
    
    DWORD dwHWVersion;      // 硬件版本

    DWORD dwFWVersion;      // 固件版本
    DWORD dwFWBuildDate;    // 固件版本时间

    DWORD dwSWVersion;      // 软件版本
    DWORD dwSWBuildDate;    // 软件版本时间

    BYTE btVideoInNum;      // 视频输入数
    BYTE btAudioInNum;      // 音频输入数
    BYTE btAlarmInNum;      // 报警输入个数
    BYTE btAlarmOutNum;     // 报警输出个数 

    BYTE btTalkPortNum;     // 语音口个数
    BYTE btRS232Num;        // 232串口个数
    BYTE btRS485Num;        // 485串口个数
    BYTE btNetWorkPortNum;  // 网口个数

    BYTE btStorageNum;      // 存储设备个数
    BYTE btChannelNum;      // 模拟通道个数
    BYTE btDecodeChannelNum;// 设备解码路数
    BYTE btVGANum;          // VGA个数
    BYTE btHDMINum;         // HDMI个数
    BYTE btUSBNum;          // USB个数
    BYTE btSpotNum;         // 辅口个数
    BYTE btIPCanNum;        // 最大数字通道
    BYTE btRes2[4];         // 保留
} FHNP_Capacity_t, *LPFHNP_Capacity_t;


/* NTP配置 */
typedef struct 
{
    BYTE btNTP;             // 是否启用NTP
    BYTE btIsSuccess;       // 是否连接成功
    BYTE btRes[2];          // 保留
    char chNTPServer[FHNP_MACRO_DOMAINLEN_MAX];// NTP服务器地址    
    FHNP_Time_t stPrevTime; // 上一次成功更新的时间
    FHNP_Time_t stNextTime; // 下一次即将要更新的时间
} FHNP_NTP_t, *LPFHNP_NTP_t;


/* 网络配置 */
typedef struct 
{
    BYTE  btDynamicIP;                  // 是否自动获取IP
    BYTE  btRes1[3];                    // 保留
    FHNP_IPAddr_t stIP;                 // IP地址
    FHNP_IPAddr_t stNetMask;            // 子网掩码
    FHNP_IPAddr_t stGateWay;            // 网关
    char  chMAC[FHNP_MACRO_IPLEN_MAX];  // MAC地址
    DWORD dwPort;                       // 端口

    BYTE btDynamicDNS;                  // 是否自动获取DNS
    BYTE btRes2[3];                     // 保留
    FHNP_IPAddr_t stHostDNS;            // 首选DNS服务器
    FHNP_IPAddr_t stSecondDNS;          // 备用DNS服务器
} FHNP_NetConfig_t, *LPFHNP_NetConfig_t;


/* 串口配置 */
typedef struct 
{
    int  iBaudRate;                     // 波特率
    int  iDataBit;                      // 数据位
    int  iStopBit;                      // 停止位
    BYTE btParity;                      // 奇偶校验(FHNPEN_Parity_e)
    BYTE btFlowCtrl;                    // 数据流控制(FHNPEN_FlowControl_e)
    BYTE btRes[2];                      // 保留
} FHNP_SerialPort_t, *LPFHNP_SerialPort_t;


/* 实时监控请求 */
typedef struct 
{
    BYTE btChannel;                     // 通道(暂无效, 填0)
    BYTE btEncID;                       // 编码ID(暂无效, 填0)
    BYTE btTransMode;                   // 传输协议(FHNPEN_TransMode_TCP/FHNPEN_TransMode_UDP)
    BYTE btBlocked;                     // 0-异步, 1-同步
    char chMultiCastIP[FHNP_MACRO_IPLEN_MAX];   // 多播IP地址(暂无效)
} FHNP_Preview_t, *LPFHNP_Preview_t;


/* 自动维护配置 */
typedef struct 
{
    BYTE btARebootMode;         // 自动维护模式(FHNPEN_AutoRebootMode_e)
    BYTE btDay;                 // 天
    BYTE btHour;                // 时
    BYTE btRes;                 // 保留
} FHNP_AutoReboot_t, *LPFHNP_AutoReboot_t;


/* 日志查询 */
typedef struct 
{
    BYTE btType;                // 日志查询类型(FHNP_LogType_e)
    BYTE btRes[3];              // 保留
    FHNP_Time_t stStartTime;    // 开始时间
    FHNP_Time_t stStopTime;     // 结束时间
} FHNP_LogSearch_t, *LPFHNP_LogSearch_t;


/* 日志信息 */
typedef struct 
{
    FHNP_Time_t stTime;                 // 日志时间
    BYTE btType;                        // 日志类型(FHNP_LogType_e)
    BYTE btChannel;                     // 通道号
    BYTE btRes[2];                      // 保留
    char chUser[FHNP_MACRO_NAMELEN_MAX];// 用户名
    char chIP[FHNP_MACRO_IPLEN_MAX];    // IP地址
    char chLog[FHNP_MACRO_LOGLEN_MAX];  // 日志内容
} FHNP_Log_t, *LPFHNP_Log_t;


/* wifi配置 */
typedef struct
{
    char sSSID[33];
    char sDummy[3];
    DWORD dwChannel;
    int iMode;                // FHNPEN_WifiMode_e
    int iType;                // FHNPEN_WifiSecurityType_e
    char sPSK[128];
    BYTE btPSKLen;
    BYTE btRes[3];
} FHNP_WifiConfig_t, *LPFHNP_WifiConfig_t;


/* 视频帧头信息 */
typedef struct
{
    BYTE    btFrameType;        // 帧类型(FHNPEN_FrameType_e)
    BYTE    btVideoFormat;      // 视频编码格式(FHNPEN_EncVideoFormat_e)
    BYTE    btRestartFlag;      // 图像幅面或配置发生更改
    BYTE    btRes;              // 保留
    WORD    wWidth;             // 图像宽
    WORD    wHeight;            // 图像高
    DWORD64 ullTimeStamp;       // 时间戳
    BYTE    btRes2[4];          // 保留
} FHNP_VFrameHead_t, *LPFHNP_VFrameHead_t;


/* 音频帧头信息 */
typedef struct
{
    BYTE    btFrameType;        // 帧类型(FHNPEN_FrameType_e)
    BYTE    btAudioFormat;      // 音频编码格式(FHNPEN_EncAudioFormat_e)
    BYTE    btBitWidth;         // 音频位宽(FHNPEN_EncAudioBitWidth_e)
    BYTE    btTrace;            // 声道(FHNPEN_EncAudioTrack_e)
    DWORD64 ullTimeStamp;       // 时间戳
    DWORD   dwSampleRate;       // 采样率(FHNPEN_EncAudioSampleRate_e)
    DWORD   dwDataLen;          // 数据长度
    BYTE    btRes[4];           // 保留
} FHNP_AFrameHead_t, *LPFHNP_AFrameHead_t;


/* */
typedef struct 
{
    BYTE    btFrameType;        // 帧类型(FHNPEN_FrameType_e)
    DWORD64 ullTimeStamp;       // 时间戳
    BYTE    btRes[4];           // 保留
} FHNP_JFrameHead_t, *LPFHNP_JFrameHead_t;


/* 帧头信息(联合体) */
typedef union
{
    FHNP_VFrameHead_t stVFrameHead; // 视频帧头
    FHNP_AFrameHead_t stAFrameHead; // 音频帧头
    FHNP_JFrameHead_t stJFrameHead; // 截图帧头
} FHNP_FrameHead_t, *LPFHNP_FrameHead_t;


/* SDK状态 */
typedef struct
{
    DWORD dwTotalLoginNum;		// 当前login用户数
    DWORD dwTotalRealPlayNum;	// 当前realplay路数
    DWORD dwTotalUpgradeNum;	// 当前升级路数
    DWORD dwTotalSerialNum;     // 当前串口透传路数
    DWORD dwRes[10];            // 保留
} FHNP_SDKState_t, *LPFHNP_SDKState_t;


/* 上报消息 */
typedef struct 
{
    BYTE  btNotify;         // 上报消息ID
    BYTE  btFlag;           // 开启/关闭
    BYTE  btChannel;        // 通道
    BYTE  btRes;            // 保留
    DWORD dwUserID;         // UserID
    DWORD dwBufLen;         // 数据长度
    char  chBuffer[4096];   // 数据内容
} FHNP_Notify_t, *LPFHNP_Notify_t;




/* 远程回放请求 */
typedef struct 
{
    BYTE  btChannel;                // 通道
    BYTE  btEncID;                  // 编码ID
    BYTE  btTransMode;              // 传输协议
    BYTE  btRes;                    
    DWORD dwRecTypeMask;            // 录像类型(FHNPEN_RecordType_e按位与)
    FHNP_Time_t stStartTime;        // 开始时间
    FHNP_Time_t stStopTime;         // 结束时间    
} FHNP_Playback_t, *LPFHNP_Playback_t;


/* 远程录像查询 */
typedef struct
{
    BYTE  btLockFlag;            // 0全部 1锁定 2未锁定
    BYTE  btChanNum;             // 通道个数
    BYTE  btRes[2];
    BYTE  btChannel[FHNP_MACRO_CHANNEL_MAX];    // 通道列表
    DWORD dwRecTypeMask;        // 录像类型(FHNPEN_RecordType_e按位与)
    FHNP_Time_t stStartTime;    // 开始时间
    FHNP_Time_t stStopTime;     // 结束时间
} FHNP_RecSearch_t, *LPFHNP_RecSearch_t;


/* 远程截图查询 */
typedef struct 
{
    BYTE  btLockFlag;            // 0全部 1锁定 2未锁定
    BYTE  btChanNum;             // 通道个数
    BYTE  btRes[2];
    BYTE  btChannel[FHNP_MACRO_CHANNEL_MAX];    // 通道列表    
    DWORD dwPicTypeMask;        // 截图类型(FHNPEN_PictureType_e按位与)
    FHNP_Time_t stStartTime;    // 开始时间
    FHNP_Time_t stStopTime;     // 结束时间
} FHNP_PicSearch_t, *LPFHNP_PicSearch_t;


/* 远程录像信息 */
typedef struct 
{
    BYTE btChannel;             // 通道
    BYTE btRecType;             // 录像类型(FHNPEN_RecordType_e)
    BYTE btLockFlag;            // 锁定状态(0未锁, 1已锁)
    BYTE btRes;
    DWORD64 ullStartTime;       // 开始时间(微秒)
    DWORD64 ullStopTime;        // 结束时间(微秒)    
    DWORD64 ullDataSize;
} FHNP_Record_t, *LPFHNP_Record_t;


/* 远程截图信息 */
typedef struct 
{
    BYTE btChannel;             // 通道
    BYTE btPicType;             // 截图类型(FHNPEN_PictureType_e)
    BYTE btLockFlag;            // 锁定状态(0未锁, 1已锁)
    BYTE btRes;
    DWORD64 ullStartTime;       // 开始时间(微秒)
    DWORD64 ullStopTime;        // 结束时间(微秒)    
    DWORD64 ullDataSize;
    DWORD64 ullFrameCount;      // 张数
} FHNP_Picture_t, *LPFHNP_Picture_t;


/* 盘组存储策略 */
typedef struct 
{
    DWORD dwMaxHour;            // 盘组上最多写多少小时的数据(0忽略)
    DWORD dwFullThreshold;      // 小于多少MB时认为满
    BYTE  btRecycleFlag;        // 盘组满时是否循环覆盖(0-不覆盖, 1-覆盖)
    BYTE  btCleanData;          // 是否彻底清除数据(0-不清除数据, 1-清除)
    BYTE  btRes[2];    
} FHNP_StoragePolicy_t, *LPFHNP_StoragePolicy_t;


typedef struct 
{
    BYTE btState;               // SD卡状态(FHNPEN_SDCardState_e)
    BYTE btRes[3];
    DWORD64 ullTotalSize;       // SD卡总容量
    DWORD64 ullUsedSize;        // SD卡已用空间
} FHNP_SDCardInfo_t, *LPFHNP_SDCardInfo_t;


/* 每段录像时间表 */
typedef struct 
{
    BYTE btEnable;         // 使能
    BYTE btRecAudio;       // 是否录制音频
    BYTE btStartHour;      // 开始时间(时)
    BYTE btStartMinute;    // 开始时间(分)
    BYTE btStopHour;       // 结束时间(时)
    BYTE btStopMinute;     // 结束时间(分)
    BYTE btReserve[2];
} FHNP_RecScheduleSegment_t, *LPFHNP_RecScheduleSegment_t;

/* 每日录像时间表 */
typedef struct 
{
    FHNP_RecScheduleSegment_t stSegment[FHNP_MACRO_DAILY_SCHED_ITEM_MAX];
    BYTE btEnable;
    BYTE btReserve[3];
} FHNP_RecScheduleDaily_t, *LPFHNP_RecScheduleDaily_t;

/* 定时录像时间表 */
typedef struct 
{
    FHNP_RecScheduleDaily_t stDaily[7];     // 0-周日
    BYTE btEnable;
    BYTE btReserve[3];
} FHNP_RecScheduleTiming_t, *LPFHNP_RecScheduleTiming_t;


#endif

