#ifndef _BIZ_TYPES_H_
#define _BIZ_TYPES_H_

///////////////////////业务层初始化所有模块相关//////////////////////////
//param used by SBizEventPara
typedef enum
{
    EM_BIZ_EVENT_UNKNOW = -1,
    EM_BIZ_EVENT_RECORD = 0,  //录像状态及类型
    EM_BIZ_EVENT_ALARM_SENSOR, //传感器报警状态
    EM_BIZ_EVENT_ALARM_VMOTION, //移动侦测状态
    EM_BIZ_EVENT_ALARM_VBLIND,  //遮挡状态
    EM_BIZ_EVENT_ALARM_VLOSS, //视频丢失状态
    EM_BIZ_EVENT_ALARM_OUT, //报警输出状态
    EM_BIZ_EVENT_ALARM_BUZZ,  //蜂鸣器状态
    EM_BIZ_EVENT_ALARM_IPCEXT,//IPC外部传感器报警事件
    EM_BIZ_EVENT_ALARM_IPCCOVER,
    EM_BIZ_EVENT_ALARM_DISK_LOST,
    EM_BIZ_EVENT_ALARM_DISK_ERR,
    EM_BIZ_EVENT_ALARM_DISK_NONE,
    EM_BIZ_EVENT_ALARM_485EXT,
    

    EM_BIZ_EVENT_LOCK = 50,
    EM_BIZ_EVENT_RESTART,		//系统将在nDelay后重起
    EM_BIZ_EVENT_POWEROFF,		//系统将在nDelay后关机
    EM_BIZ_EVENT_POWEROFF_MANUAL,	//提示可以手动关机了

    EM_BIZ_EVENT_TIMETICK = 90,
    EM_BIZ_EVENT_DATETIME_YMD,
	
    EM_BIZ_EVENT_FORMAT_INIT = 100,  //格式化初始化
    EM_BIZ_EVENT_FORMAT_RUN, //格式化中
    EM_BIZ_EVENT_FORMAT_DONE, //格式化结束

    EM_BIZ_EVENT_PLAYBACK_INIT = 150,  //回放初始化
    EM_BIZ_EVENT_PLAYBACK_RUN, //回放中
    EM_BIZ_EVENT_PLAYBACK_DONE, //回放结束

    EM_BIZ_EVENT_BACKUP_INIT = 200,  //备份初始化
    EM_BIZ_EVENT_BACKUP_RUN, //备份中
    EM_BIZ_EVENT_BACKUP_DONE, //备份结束

    
    EM_BIZ_EVENT_UPGRADE_INIT = 250,  //升级初始化
    EM_BIZ_EVENT_UPGRADE_RUN, //升级中
    EM_BIZ_EVENT_UPGRADE_DONE, //升级结束
    EM_BIZ_EVENT_REMOTEUP_START,//cw_remote
    EM_BIZ_EVENT_GETDMINFO,
    EM_BIZ_EVENT_SATARELOAD,//
    EM_BIZ_EVENT_DISKCHANGED,//

    EM_BIZ_EVENT_PREVIEW_REFRESH = 300,
    EM_BIZ_EVENT_LIVE_REFRESH = 301,
    EM_BIZ_EVENT_SHOWTIME_REFRESH = 302,

	EM_BIZ_EVENT_ENCODE_GETRASTER = 350,

	EM_BIZ_EVENT_NET_STATE_DHCP = 400,
	EM_BIZ_EVENT_NET_STATE_PPPOE,
	EM_BIZ_EVENT_NET_STATE_MAIL,
	EM_BIZ_EVENT_NET_STATE_DDNS,
	EM_BIZ_EVENT_NET_STATE_CONN,
	EM_BIZ_EVENT_NET_STATE_UPDATEMAINBOARDSTART,
	EM_BIZ_EVENT_NET_STATE_UPDATEPANNELSTART,
	EM_BIZ_EVENT_NET_STATE_UPDATESTARTLOGOSTART,
	EM_BIZ_EVENT_NET_STATE_UPDATEAPPLOGOSTART,
	EM_BIZ_EVENT_NET_STATE_FORMATSTART,

	EM_BIZ_EVENT_NET_STATE_DHCP_STOP = 410,
	EM_BIZ_EVENT_NET_STATE_PPPOE_STOP,
	EM_BIZ_EVENT_NET_STATE_MAIL_STOP,
	EM_BIZ_EVENT_NET_STATE_DDNS_STOP,
	EM_BIZ_EVENT_NET_STATE_CONN_STOP,

	EM_BIZ_EVENT_NET_STATE_SGUPLOAD,
	
	EM_BIZ_EVENT_NET_CHANGEPARA_RESET,
	
}EMBIZEVENT;

typedef enum
{
	EM_BIZENCODE_FONTSIZE_SMALL,
	EM_BIZENCODE_FONTSIZE_NORMAL,
	EM_BIZENCODE_FONTSIZE_ALL
} EMBIZNCODEFONTSIZE;

typedef struct
{
	u8 nEnable;
	u8 nMainType;
	u8 nSubType;
	u8 ntime;
} SBizMainTainPara;

typedef struct
{
	u8* pBuffer;
	 s32 w;
	 s32 h;
	 s8* strOsd;
	 EMBIZNCODEFONTSIZE emSize;
} SBizRaster;

typedef struct
{
	u64 lTotalSize;
	u64 lCurSize;
	u8 nProgress;	//0~100
} SBizProgress;

#define	BIZ_UPDATE_FILE_NAME_MAX	64
#define	BIZ_UPDATE_FILE_EXT_MAX		6

typedef struct
{
	char szName[BIZ_UPDATE_FILE_NAME_MAX];
	char szExt[BIZ_UPDATE_FILE_EXT_MAX];
	u32  nSize;
	u32  nTimeStamp;
} SBizUpdateFileInfo;

// 升级文件信息列表结构体
typedef struct
{
	u8 nMax;
	u8 nFileNum;
	SBizUpdateFileInfo* pInfo;
} SBizUpdateFileInfoList;

typedef enum
{	
	EM_BIZUPGRADE_SUCCESS = 1,//升级成功1
	EM_BIZUPGRADE_RUNNING = -198,
	EM_BIZUPGRADE_FAILED_UNKNOW = -199,// 升级失败 -199~-100请自行酌情增加

	EM_BIZBACKUP_SUCCESS = 2,//备份成功2
	EM_BIZBACKUP_FAILED_UNKNOW = -299,// 备份失败 -299~-200请自行酌情增加
	EM_BIZBACKUP_FAILED_NOSPACE = -298,// 备份失败 -299~-200请自行酌情增加

	EM_BIZFORMAT_SUCCESS = 3,//格式化成功3
	EM_BIZFORMAT_FAILED_UNKNOW = -399,// 格式化失败 -399~-300请自行酌情增加
	
	EM_BIZSEARCH_SUCCESS = 4,//格式化成功3
	EM_BIZSEARCH_FAILED_UNKNOW = -499,// 格式化失败 -399~-300请自行酌情增加	

	EM_BIZ_NET_STATE_SUCCESS = 0,
	EM_BIZ_NET_STATE_FAIL = -599,	// 网络模块操作失败
} EMBIZRESULT;

//for SBizPreviewPara.emBizPreviewMode /SBizPreviewData
typedef enum
{
    EM_BIZPREVIEW_1SPLIT = 1,
    EM_BIZPREVIEW_4SPLITS = 4, //nModePara=0~最大视频通道数- 1; 0~3 for 1-4; 4-7 for 5-8; 8-11 for 9-12 ....  
    EM_BIZPREVIEW_6SPLITS = 6, //nModePara=0~最大视频通道数 - 1; 0~5 for 1-6; 6-11 for 7-12 ；nModePara 大画面通道号
    EM_BIZPREVIEW_8SPLITS = 8, //类似EM_BIZPREVIEW_6SPLITS
    EM_BIZPREVIEW_9SPLITS = 9, //类似EM_BIZPREVIEW_4SPLITS
    EM_BIZPREVIEW_10SPLITS = 10, //类似EM_BIZPREVIEW_4SPLITS
    EM_BIZPREVIEW_12SPLITS = 12, //类似EM_BIZPREVIEW_6SPLITS
    EM_BIZPREVIEW_13SPLITS = 13, //类似EM_BIZPREVIEW_6SPLITS
    EM_BIZPREVIEW_16SPLITS = 16, //类似EM_BIZPREVIEW_4SPLITS
    EM_BIZPREVIEW_25SPLITS = 25,//24,
    EM_BIZPREVIEW_24SPLITS = 24,//25, //类似EM_BIZPREVIEW_4SPLITS
    EM_BIZPREVIEW_36SPLITS = 36,//32, //类似EM_BIZPREVIEW_6SPLITS
    EM_BIZPREVIEW_32SPLITS = 32,//36, //类似EM_BIZPREVIEW_4SPLITS
    EM_BIZPREVIEW_64SPLITS = 64, //类似EM_BIZPREVIEW_4SPLITS
    EM_BIZPREVIEW_PIP = 254, //
    EM_BIZPREVIEW_CLOSE = 255,   //
} EMBIZPREVIEWMODE;

//预览状态更新回调
typedef struct
{
    EMBIZPREVIEWMODE emBizPreviewMode; //预溃窗口分割模式
    u8 nModePara; //extend parameter for EMBIZPREVIEWMODE
    u8 nAdioChn; //当前音频输出通道
    u8 nMute; //输出静音
    u8 nVolume; //输出音量
} SBizPreviewData;

typedef enum
{
    EM_BIZREC_MASK_MANUAL = (1 << 0), //手动录像
    EM_BIZREC_MASK_TIMING = (1 << 1), //定时录像
    EM_BIZREC_MASK_MOTION = (1 << 2), //移动侦测录像
    EM_BIZREC_MASK_SENSORALARM = (1 << 3), //传感器报警录像
} EMBIZRECSTATUSMASK; //第8位是预录的标志

typedef struct
{
    u8 nChn; //通道号
    u8 nLastStatus; //上次状态 // >0 表示正在录像；每位代表一种类型；请参考EMBIZRECSTATUSMASK
    u8 nCurStatus; //当前状态 // >0 表示正在录像；每位代表一种类型；请参考EMBIZRECSTATUSMASK
} SBizRecStatus;

typedef struct
{
    u8 nChn; //通道号或者传感器id
    u8 nLastStatus; //上次状态 // >0 表示有报警, 0表示无
    u8 nCurStatus; //上次状态 // >0 表示有报警, 0表示无
    time_t nTime; //最后一次报警时间
} SBizAlarmStatus;

// 2.20 日期时间格式
typedef enum
{
	//酌情再加
	EM_BIZDATETIME_YYYYMMDDHHNNSS, //year-month-day
	EM_BIZDATETIME_MMDDYYYYHHNNSS,
	EM_BIZDATETIME_DDMMYYYYHHNNSS //day-month-year
} EMBIZDATETIMEFORMAT;

typedef enum
{
	//酌情再加
	EM_BIZTIME_24HOUR, 
	EM_BIZTIME_12HOUR,
} EMBIZTIMEFORMAT;

// 2.19 日期时间
typedef struct
{
	u16 nYear;
	u8 nMonth;
	u8 nDay;
	u8 nHour;
	u8 nMinute;
	u8 nSecond;
	u8 nWday;
	EMBIZDATETIMEFORMAT emDateTimeFormat;
	EMBIZTIMEFORMAT emTimeFormat;
} SBizDateTime;

typedef struct
{
	u8  nType; // 0 hdd, 1 usb
	u32 nTotal;
	u32 nFree;
	char szMountPoint[32];
} SBizDMInfo;

// remote format
typedef struct 
{
	u8 nDiskIdx;
	EMBIZRESULT emBizResult;
} SBizRemoteFormat;

// 2.15 设备信息
#ifndef HDD_UNFORMAT
#define HDD_UNFORMAT 0 //未格式化
#endif
#ifndef HDD_FORMATTING
#define HDD_FORMATTING 1 //格式化中
#endif
#ifndef HDD_INUSE
#define HDD_INUSE 2 //使用中
#endif
#ifndef HDD_IDLE
#define HDD_IDLE 3 //空闲
#endif
#ifndef HDD_SLEEP
#define HDD_SLEEP 4 //休眠
#endif
#ifndef HDD_ERROR
#define HDD_ERROR 5 //出错
#endif

typedef struct
{
	u8 nType;//0 hdd; 1 usb
	char strDevPath[32];
	u8 nPartitionNo;
	u32 nStatus;//u8 nStatus;//0 ok; 1 unformated; 2 err
	u32 nTotal;
	u32 nFree;
	//u8 storage_type;//'s'->sata, 'u'->'usb'
	u8 disk_physical_idx;//主板上硬盘的物理编号
	u8 disk_logic_idx;//主板上硬盘的逻辑编号
	u8 disk_system_idx;//系统检测到的硬盘编号sda->0, sdb->1...
	//char disk_name[32];//硬盘设备名/dev/sda, /dev/sdb...
	char disk_sn[64];//硬盘序列号、唯一
} SBizDevInfo;

// 2.14 获得存储设备管理信息
typedef struct
{
	u8 		  nDiskNum;
	SBizDevInfo*  psDevList;
} SBizStoreDevManage;

//param used by FPBIZEVENTCB
typedef struct
{
	EMBIZEVENT emType;
	union
	{
		u16 nDelay;
		SBizProgress sBizProgress; //进度
		EMBIZRESULT emBizResult; //执行结果枚举
		SBizPreviewData sPreviewData; //当前预览模式
		SBizRecStatus sBizRecStatus;
		SBizAlarmStatus sBizAlaStatus;
		s8* pstrDateTime;
		SBizDateTime sBizDateTime;
		SBizRaster sBizRaster;
		SBizDMInfo sDmInfo;
		SBizStoreDevManage sStoreMgr;
		SBizRemoteFormat sBizFormat;
	};   
} SBizEventPara;

//param used by  SBizInitPara.pfnBizEventCb
//typedef void (* FNBIZEVENTCB)(SBizEventPara* sBizEventPara); //业务层事件回调函数
typedef int (* FNBIZEVENTCB)(SBizEventPara* sBizEventPara); //业务层事件回调函数

//模块掩码取值如下
#define MASK_BIZ_MOD_NONE 0 //无启用模块
//******mask definition for each module start******
#define MASK_BIZ_MOD_PREVIEW (1 << 0) //预览
#define MASK_BIZ_MOD_ENCODE (1 << 1) //编码
#define MASK_BIZ_MOD_RECORD (1 << 2) //录像
#define MASK_BIZ_MOD_PLAYBACK (1 << 3) //回放
#define MASK_BIZ_MOD_ALARM (1 << 4) //报警
#define MASK_BIZ_MOD_CONFIG (1 << 5) //配置
#define MASK_BIZ_MOD_SYSCOMPLEX (1 <<6) //系统杂项
#define MASK_BIZ_MOD_PTZ (1 <<7) //系统杂项
#define MASK_BIZ_MOD_USER (1 <<8) //用户
#define MASK_BIZ_MOD_LOG (1 <<9) //日志
#define MASK_BIZ_MOD_NET (1 << 15) //网络
//yaogang modify 20150324
#define MASK_BIZ_MOD_SNAP (1 << 16) //抓图--biz_snap.cpp

//******mask definition for each module end******
#define MASK_BIZ_MOD_ALL 0xffffffff //启用所有模块

//模块掩码nModMask
//param used by BizInit
typedef struct
{
	u32 nModMask;
	FNBIZEVENTCB pfnBizEventCb;
} SBizInitPara;

///////////////////////预览相关//////////////////////////
//for BizElectronicZoom
typedef struct{
	unsigned int s32X;
	unsigned int s32Y;
	unsigned int u32Width;
	unsigned int u32Height;
}SBizPreviewElecZoom;
//for BizStartPreview
typedef struct
{
    EMBIZPREVIEWMODE emBizPreviewMode; //预溃窗口分割模式
    u8 nModePara; //extend parameter for EMBIZPREVIEWMODE
} SBizPreviewPara;

// for EMBIZPARATYPE.EM_BIZ_PATROL
typedef struct
{
    u8 nIsPatrol; //轮巡是否启用(数字:0=否;1=是)
    u8 nInterval; //轮巡切换时间(数字:单位秒s)
    u8 nPatrolMode; //轮巡时所用的预览模式(数字:参考Mode)
    u8 nStops; //轮巡站数(数字:一个轮回停几个点)
    u8 pnStopModePara[255]; //轮巡每站所使用的预览扩展参数(数字列表:参考ModePara)
} SBizCfgPatrol;

///////////////////////回放模块相关//////////////////////////
typedef enum
{
	EM_BIZPLAY_TYPE_FILE = 0,
	EM_BIZPLAY_TYPE_TIME,
} EMBIZPLAYTYPE;

typedef enum
{
	EM_BIZCTL_NULL=0,
	EM_BIZCTL_STOP,
	EM_BIZCTL_PAUSE,
	EM_BIZCTL_RESUME,
	EM_BIZCTL_STEP,
	EM_BIZCTL_PRE_SECT,
	EM_BIZCTL_NXT_SECT,
	EM_BIZCTL_SPEED_DOWN,
	EM_BIZCTL_SPEED_UP,
	EM_BIZCTL_SET_SPEED,
	EM_BIZCTL_SEEK,
	EM_BIZCTL_FORWARD,
	EM_BIZCTL_BACKWARD,
	EM_BIZCTL_MUTE,
	EM_BIZCTL_PBTIME,
	EM_BIZCTL_PBFILE
} EMBIZPLAYCTRLCMD;

///////////////////////云台模块相关控制命令//////////////////////////
typedef enum
{
	EM_BIZPTZ_CMD_START_TILEUP = 0,
	EM_BIZPTZ_CMD_START_TILEDOWN,
	EM_BIZPTZ_CMD_START_PANLEFT,
	EM_BIZPTZ_CMD_START_PANRIGHT,
	EM_BIZPTZ_CMD_START_LEFTUP,
	EM_BIZPTZ_CMD_START_LEFTDOWN,
	EM_BIZPTZ_CMD_START_RIGHTUP,
	EM_BIZPTZ_CMD_START_RIGHTDOWN,
	EM_BIZPTZ_CMD_START_ZOOMWIDE,
	EM_BIZPTZ_CMD_START_ZOOMTELE,
	EM_BIZPTZ_CMD_START_FOCUSNEAR,
	EM_BIZPTZ_CMD_START_FOCUSFAR,
	EM_BIZPTZ_CMD_START_IRISSMALL,
	EM_BIZPTZ_CMD_START_IRISLARGE,
	EM_BIZPTZ_CMD_START_WIPEROPEN,
	EM_BIZPTZ_CMD_START_WIPERCLOSE,
	//
	EM_BIZPTZ_CMD_STOP_TILEUP = 30,
	EM_BIZPTZ_CMD_STOP_TILEDOWN,
	EM_BIZPTZ_CMD_STOP_PANLEFT,
	EM_BIZPTZ_CMD_STOP_PANRIGHT,
	EM_BIZPTZ_CMD_STOP_LEFTUP,
	EM_BIZPTZ_CMD_STOP_LEFTDOWN,
	EM_BIZPTZ_CMD_STOP_RIGHTUP,
	EM_BIZPTZ_CMD_STOP_RIGHTDOWN,
	EM_BIZPTZ_CMD_STOP_ZOOMWIDE,
	EM_BIZPTZ_CMD_STOP_ZOOMTELE,
	EM_BIZPTZ_CMD_STOP_FOCUSNEAR,
	EM_BIZPTZ_CMD_STOP_FOCUSFAR,
	EM_BIZPTZ_CMD_STOP_IRISSMALL,
	EM_BIZPTZ_CMD_STOP_IRISLARGE,
	EM_BIZPTZ_CMD_STOP_WIPEROPEN,
	EM_BIZPTZ_CMD_STOP_WIPERCLOSE,
	
	EM_BIZPTZ_CMD_STOP_ALL = 99,

	EM_BIZPTZ_CMD_PRESET_SET = 100,
	//
	EM_BIZPTZ_CMD_PRESET_GOTO = 102,
	//
	EM_BIZPTZ_CMD_AUTOSCAN_ON = 110,
	EM_BIZPTZ_CMD_AUTOSCAN_OFF,

	EM_BIZPTZ_CMD_START_TOUR = 130,
	EM_BIZPTZ_CMD_STOP_TOUR,
	EM_BIZPTZ_CMD_START_TOUR_AUTO = 134,
	EM_BIZPTZ_CMD_STOP_TOUR_AUTO,
	
	//
	EM_BIZPTZ_CMD_START_PATTERN = 140,
	EM_BIZPTZ_CMD_STOP_PATTERN,
	EM_BIZPTZ_CMD_STARTREC_PATTERN,
	EM_BIZPTZ_CMD_STOPREC_PATTERN,

	//
	EM_BIZPTZ_CMD_AUX_ON = 150,
	EM_BIZPTZ_CMD_AUX_OFF,
	//
	EM_BIZPTZ_CMD_LIGHT_ON = 160,
	EM_BIZPTZ_CMD_LIGHT_OFF,
	//
	EM_BIZPTZ_CMD_SETSPEED = 253,
	EM_BIZPTZ_CMD_NULL = 254,
}EMBIZPTZCMD;

///////////////////////杂项模块相关//////////////////////////
// 2.6 系统退出类型枚举
typedef enum
{
	EM_BIZSYSEXIT_LOCK,
	EM_BIZSYSEXIT_RESTART,
	EM_BIZSYSEXIT_POWEROFF,
} EMBIZSYSEXITTYPE;

// mainboard || panel
typedef enum
{
	EM_BIZUPGRADE_MAINBOARD,
	EM_BIZUPGRADE_PANEL,
	EM_BIZUPGRADE_STARTLOGO,
	EM_BIZUPGRADE_APPLOGO,
} EMBIZUPGRADETARGET;

// 2.7 文件类型枚举
typedef enum
{
	EM_BIZFILE_REC = 0,
	EM_BIZFILE_SNAP,
} EMBIZFILETYPE;

// 2.12 文件格式
typedef enum
{
	EM_BIZFORMAT_ORIGINAL = 0,
	EM_BIZFORMAT_AVI,
	EM_BIZFORMAT_JPEG,
	EM_BIZFORMAT_BMP,
} EMBIZFILEFORMAT;

// backup device type
typedef enum
{
	EM_BIZBACKUP_USB,
	EM_BIZBACKUP_DVDR,
	
} EMBIZBACKUPDEV;

typedef enum
{
	EM_BIZDVDRW_MODE_NEW,
	EM_BIZDVDRW_MODE_ADD,
	
} EMBIZBACKUPDVDRWMODE;

// Backup target attributes
typedef struct
{
	EMBIZFILEFORMAT fmt;
	EMBIZBACKUPDEV  devt;
	union
	{
		u8 reserve;
		EMBIZBACKUPDVDRWMODE mode;
	};
} SBizBackTgtAtt, *PSBizBackTgtAtt;

// 2.8 文件搜索方式枚举
typedef enum
{
	EM_BIZSEARCH_BY_TIME,
	EM_BIZSEARCH_BY_FILE,
} EMBIZSEARCHWAY;

// 2.10 按时间搜索参数
typedef struct 
{
	u8 nMaskType;
	u32 nMaskChn;
	u32 nStartTime;
	u32 nEndTime;
	void* pReserverInfo;
} SBizSearchPara;

// 2.17  文件信息参数
typedef struct
{
	u8 nChn;
	u8 nType;
	u32 nStartTime;
	u32 nEndTime;
	u8 nFormat;  //N制式下 10 = D1，13 = CIF  P制式下 0 = D1， 3 = CIF    
	u8 nStreamFlag;  //0:视频流;1:音频流
	u32 nSize;
	u32 nOffset;
	u8 nDiskNo;
	u8 nPtnNo;
	u16 nFileNo;
} PACK_NO_PADDING SBizRecfileInfo;
typedef struct
{
	u8 nChn;
	u8 nType;
	u8 nFormat;  //格式,0-jpeg,1-bmp,默认jpeg 
	u8 nReserver;
	u16 	width;
	u16	height;
	u32 nSize;
	u32 nOffset;
	u8 nDiskNo;
	u8 nPtnNo;
	u16 nFileNo;
	u32 nStartTime;
	u32 nStartTimeus;
} PACK_NO_PADDING SBizRecSnapInfo;

// 2.11 文件搜索结果结构体
typedef struct
{
	u16 nFileNum;
	union
	{
		SBizRecfileInfo *psRecfileInfo;//用于存放数据的文件信息数组
		SBizRecSnapInfo *psRecSnapInfo;
	};
} PACK_NO_PADDING SBizSearchResult;

// 2.9 搜索条件参数
typedef struct
{
	EMBIZSEARCHWAY emSearchWay;
	union
	{
		SBizSearchPara sSearchPara;
		SBizSearchResult sSearchResult;
	};
} SBizSearchCondition;


// 2.16 格式化方式
typedef enum
{
	EM_BIZFORMAT_FAT32_IFV,	//格式化为fat32并初始化录像格式文件
	EM_BIZFORMAT_FAT32,	//格式化为fat32
} EMBIZFORMATWAY;


// 2.18 系统信息
typedef struct
{
	
} SBizSysComplexInfo; //酌情增加

///////////////////////配置模块相关//////////////////////////
typedef enum
{
	EM_BIZCONFIG_PARA_SYSTEM = 0,
	EM_BIZCONFIG_PARA_PREVIEW = 100,
	EM_BIZCONFIG_PARA_IMAGE = 200,
	EM_BIZCONFIG_PARA_MAIN_OSD = 300,
	EM_BIZCONFIG_PARA_STR_OSD = 330,
	EM_BIZCONFIG_PARA_RECT_OSD = 360,	
	EM_BIZCONFIG_PARA_SENSOR = 400,
	EM_BIZCONFIG_PARA_VMOTION = 500,
	EM_BIZCONFIG_PARA_VLOST = 600,
	EM_BIZCONFIG_PARA_VBLIND = 700,
	EM_BIZCONFIG_PARA_ALARMOUT = 800,
	EM_BIZCONFIG_PARA_BUZZ = 900,
	EM_BIZCONFIG_PARA_SENSOR_SCH = 1000,
	EM_BIZCONFIG_PARA_VMOTION_SCH = 1100,
	EM_BIZCONFIG_PARA_VLOST_SCH = 1200,
	EM_BIZCONFIG_PARA_VBLIND_SCH = 1300,
	EM_BIZCONFIG_PARA_ALARMOUT_SCH = 1400,
	EM_BIZCONFIG_PARA_BUZZ_SCH = 1500,	
	//
	EM_BIZCONFIG_PARA_ENC_MAIN = 4000,
	EM_BIZCONFIG_PARA_ENC_SUB = 4100,
	//
	EM_BIZCONFIG_PARA_RECORD = 6000,
	EM_BIZCONFIG_PARA_TIMER_REC_SCH = 6100,
	EM_BIZCONFIG_PARA_SENSOR_REC_SCH = 6200,
	EM_BIZCONFIG_PARA_VMOTION_REC_SCH = 6300,
	EM_BIZCONFIG_PARA_VLOST_REC_SCH = 6400,
	EM_BIZCONFIG_PARA_VBLIND_REC_SCH = 6500,	
	//
	EM_BIZCONFIG_PARA_USER = 8000,
	EM_BIZCONFIG_PARA_PTZ = 8100,
	EM_BIZCONFIG_PARA_NETWORK = 8200,	
	//
	EM_BIZCONFIG_PARA_DVR_PROPERTY = 9000,
	//
	EM_BIZCONFIG_PARA_ALL = 10000,
} EMBIZCONFIGPARATYPE;

///////////////////////编码模块相关//////////////////////////
//编码类型(定码率/变码率)
typedef enum
{
	EM_BIZENC_BITRATE_VARIABLE = 0, 		
	EM_BIZENC_BITRATE_CONST, 				
}EMBIZENCBITRATETYPE;

//图像分辨率 0=QCIF/1=CIF/2=HALF_D1/3=4Cif/4=D1
typedef enum
{
	EM_BIZENC_RESOLUTION_QCIF = 0,
	EM_BIZENC_RESOLUTION_CIF = 1,
	EM_BIZENC_RESOLUTION_HD1 = 2,
	EM_BIZENC_RESOLUTION_4CIF = 3,
	EM_BIZENC_RESOLUTION_D1 = 4,
	EM_BIZENC_RESOLUTION_720P = 5,
	EM_BIZENC_RESOLUTION_1080P = 6,
	EM_BIZENC_RESOLUTION_960H = 7,
}EMBIZENCRESOLUTION;

//图像质量(数字列表:0=最好/1=更好/2=好/3=一般/4=差/5=更差/6=最差)
typedef enum
{
	EM_BIZENC_PICLEVEL_BEST = 0, 		
	EM_BIZENC_PICLEVEL_BETTER = 1,
	EM_BIZENC_PICLEVEL_GOOD = 2,
	EM_BIZENC_PICLEVEL_NORMAL =3,
	EM_BIZENC_PICLEVEL_BAD =4,
	EM_BIZENC_PICLEVEL_WORSE = 5,
	EM_BIZENC_PICLEVEL_WORST = 6,
} EMBIZENCPICLEVEL;

//编码参数
typedef struct 
{
	u32 nBitRate; // 位率(单位为Kb)
	u8 nFrameRate; // 帧率
	u32 nGop; // 关键帧间隔
	u32 nMinQP; // (暂不支持)
	u32 nMaxQP; // (暂不支持)

	u8 nVideoResolution;  // 视频分辨率(EMBIZENCRESOLUTION)
	u8 nEncodeType; // 视频编码类型(EMBIZENCTYPE数字,暂时只支持固定取值98,H.264) 不支持设置
	u8 nBitRateType; // 位率类型(EMBIZENCBITRATETYPE)
	u8 nPicLevel; // 图像质量(EMBIZENCPICLEVEL)
} SBizEncodePara;

//布防时间段
typedef struct 
{
	u32 nStartTime;	//每个时间段的起始时间
	u32 nStopTime;	//结束时间
} SBizSchTime;

//布防类型；目前仅支持EM_ALARM_SCH_WEEK_DAY
typedef enum					//布防类型枚举
{
	EM_BIZ_SCH_WEEK_DAY = 0,		//每周的哪几天
	EM_BIZ_SCH_MONTH_DAY,		//每月的哪几天
	EM_BIZ_SCH_EVERY_DAY, 	//每天
	EM_BIZ_SCH_ONCE_DAY,		//只一次有效 / 今天
} EMBIZSCHTYPE;

#define MAX_SCHEDULE_SEGMENTS 12	//每天可设置的时间段的最大数目

typedef struct 
{
	u8 nSchType; //布防的时间类型	EMBIZSCHTYPE
	SBizSchTime sSchTime[31][MAX_SCHEDULE_SEGMENTS];	//各段布防时间
} SBizSchedule;

///////////////////////云台模块相关//////////////////////////
//SBizPtzPara
typedef enum
{
	EM_BIZ_BAUDRATE_110 	= 110,
	EM_BIZ_BAUDRATE_300 	= 300,
	EM_BIZ_BAUDRATE_600 	= 600,
	EM_BIZ_BAUDRATE_1200 	= 1200,
	EM_BIZ_BAUDRATE_2400 	= 2400,
	EM_BIZ_BAUDRATE_4800 	= 4800,
	EM_BIZ_BAUDRATE_9600 	= 9600,
	EM_BIZ_BAUDRATE_19200 	= 19200,
	EM_BIZ_BAUDRATE_38400 	= 38400,
	EM_BIZ_BAUDRATE_57600 	= 57600,
	EM_BIZ_BAUDRATE_115200 	= 115200,
	EM_BIZ_BAUDRATE_230400 	= 230400,
	EM_BIZ_BAUDRATE_460800 	= 460800,
	EM_BIZ_BAUDRATE_921600 	= 921600,
} EMBIZBAUDRATE;

//SBizPtzPara
typedef enum
{
	EM_BIZ_DATABIT_6 = 6,
	EM_BIZ_DATABIT_7 = 7,
	EM_BIZ_DATABIT_8 = 8,
	//
} EMBIZZDATABIT;

//SBizPtzPara
typedef enum
{
	EM_BIZ_STOPBIT_1 = 1,
	EM_BIZ_STOPBIT_2 = 2,
	//
} EMBIZSTOPBIT;

//SBizPtzPara
typedef enum
{
	EM_BIZ_CRCCHECK_TYPE_NONE = 0,	//鏃犳牎楠?
	EM_BIZ_CRCCHECK_TYPE_ODD,		//濂囨牎楠?
	EM_BIZ_CRCCHECK_TYPE_EVEN,		//鍋舵牎楠?
	//
} EMBIZCRCCHECKTYPE;

//SBizPtzPara
typedef enum
{
	EM_BIZ_FLOWCTRL_NONE = 0,
	EM_BIZ_FLOWCTRL_HARDWARE,
	EM_BIZ_FLOWCTRL_XON_XOFF,
	//
} EMBIZFLOWCTRL;

//SBizTourPath
typedef struct
{
    u8 nPresetPos; //预置点号
    u16 nDwellTime; //逗留时间
    u8 nSpeed; //速度共10档
} SBizTourPoint;

#define TOUR_POINT_MAX	128
#define TOUR_PATH_MAX 	32
#define PRESET_MAX		128

//SBizPtzAdvancedPara
typedef struct
{
    u8 nPathNo;
    SBizTourPoint sTourPoint[TOUR_POINT_MAX];
} SBizTourPath;

//SBizPtzPara
typedef struct
{
	u8 nIsPresetSet[PRESET_MAX]; //预置点位置最多128个//0表示不存在 ；1表示存在
	SBizTourPath sTourPath[TOUR_PATH_MAX]; //sTourPath[128];
} SBizPtzAdvancedPara;

//EM_BIZ_PTZPARA
typedef struct
{
	u8 nCamAddr; // 云台地址 (0-255)
	u32 nBaudRate; // 波特率EMBIZBAUDRATE
	u8 nDataBit; // 数据位EMBIZZDATABIT
	u8 nStopBit; // 停止位EMBIZSTOPBIT
	u8 nCheckType; // 校验类型EMBIZCRCCHECKTYPE
	u8 nFlowCtrlType; // 流控方式EMBIZFLOWCTRL
	u8 nProtocol; // 协议index
	u8 nEnable; // 是否启用(数字:0=否;1=是)
    SBizPtzAdvancedPara sAdvancedPara;
} SBizPtzPara;

///////////////////////报警模块相关//////////////////////////
#define MAX_ALANAME_LEN 16 

//报警器输出/传感器报警电平(数字列表:0=常闭/低电平;1=常开/高电平)
typedef enum
{
    	EM_BIZ_ALARM_LOW_LEVEL = 0, //低电平时报警
    	EM_BIZ_ALARM_HIGH_LEVEL = 1, //高电平时报警
    	EM_BIZ_ALARM_NORMAL_CLOSE = 0, //闭合时为常态，不报警
    	EM_BIZ_ALARM_NORMAL_OPEN = 1, //断开时为常态，不报警
} EMBIZALARMLEVELTYPE;

//EM_BIZ_SENSORPARA
typedef struct
{
	u8 nEnable; //是否是能(0=否;1=是)
	u8 nType; // 类型(EMBIZALARMLEVELTYPE)
	u16 nDelay; // 延时时间 
	u8 nSchType; // 布防类型(EMBIZSCHTYPE)
	s8 name[MAX_ALANAME_LEN + 1];
} SBizAlarmSensorPara;

//yaogang modify 20141010
//EM_BIZ_IPCEXTSENSORPARA
typedef struct
{
	u8 nEnable; //是否是能(0=否;1=是)
	u16 nDelay; // 延时时间 
} SBizAlarmIPCExtSensorPara;
typedef struct
{
	u8 nEnable; //是否是能(0=否;1=是)
	//u16 nDelay; // 延时时间 
} SBizAlarmHDDPara;


//EM_BIZ_VMOTIONPARA
typedef struct
{
    u8 nEnable; //是否启用
    u8 nSensitivity; //灵敏度
    u16 nDelay; //延时
    u8 nSchType; // 布防类型(EMBIZSCHTYPE)
    u64 nBlockStatus[64]; //对应区域块状态
} SBizAlarmVMotionPara;

//EM_BIZ_VLOSTPARA
typedef struct
{
	u8 nEnable;	
	u16 nDelay;
	u8 nSchType; // 布防类型(EMBIZSCHTYPE)
} SBizAlarmVLostPara;

//EM_BIZ_VBLINDPARA
typedef struct
{
	u8 nEnable;
	u16 nDelay;
	u8 nSchType; // 布防类型(EMBIZSCHTYPE)
} SBizAlarmVBlindPara;

//EM_BIZ_ALARMOUTPARA
typedef struct
{
    u8 nType; //EMBIZALARMLEVELTYPE
    u8 nEnable;	//是否启用
    u16 nDelay; //延时	
    s8 name[MAX_ALANAME_LEN + 1];//以及名称设置
    u8 nSchType; // 布防类型(EMBIZSCHTYPE)
} SBizAlarmOutPara;

//EM_BIZ_ALARMBUZZPARA
typedef struct
{
    u8 nEnable; //是否启用
    u16 nDelay; //延时	
} SBizAlarmBuzzPara;

//云台联动类型选项(0=无;1=预置点;2=巡航;3=轨迹)
typedef enum
{
	EM_BIZALARM_PTZ_NULL = 0, //不联动
	EM_BIZALARM_PTZ_PRESET, //联动预置点
	EM_BIZALARM_PTZ_PATROL, //巡航线
	EM_BIZALARM_PTZ_LOCUS, //轨迹
} EMBIZALARMPTZTYPE;

//SBizAlarmDispatch
typedef struct
{
	u8 nALaPtzType; //云台联动类型EMBIZALARMPTZTYPE
	u8 nChn; //通道号
	u8 nId; //表示预置点id/巡航线路id/轨迹(无效)
} SBizAlarmPtz;

//EM_BIZ_DISPATCH_SENSOR
//EM_BIZ_DISPATCH_VMOTION
//EM_BIZ_DISPATCH_VBLIND
//EM_BIZ_DISPATCH_VLOST
typedef struct
{
	//;;;事件触发处理
	u8 nFlagBuzz; // 发蜂鸣器(数字:0=否;1=是)
	u8 nZoomChn; // 大画面报警通道号(255 表示无效)
	u8 nFlagEmail; // 邮件(0=否;1=是)
	u8 nSnapChn[64]; // 存放触发抓图的通道号 (最多同时触发64个,255 表示无效)
	u8 nAlarmOut[64]; // 触发报警 (最多同时触发64个,255 表示无效)
	u8 nRecordChn[64]; // 触发录像通道 (最多同时触发64个,255 表示无效)
	SBizAlarmPtz sAlarmPtz[64]; //最多同时触发64个联动
} SBizAlarmDispatch;

typedef SBizSchedule SBizAlarmSchedule;

//视频制式(数字 : 12-PAL; 10-NTSC)
typedef enum
{
	EM_BIZ_NTSC = 10,
	EM_BIZ_PAL = 12,
} EMBIZVIDSTANDARD;

//输出(数字 : 0-CVBS;1-VGA 800X600;2-VGA 1024X768;3-VGA 1280X1024)
typedef enum
{
	EM_BIZ_OUTPUT_CVBS = 0,
	EM_BIZ_OUTPUT_VGA_800X600 = 1,
	EM_BIZ_OUTPUT_VGA_1024X768 = 2,
	EM_BIZ_OUTPUT_VGA_1280X1024 = 3,
	//
	EM_BIZ_OUTPUT_HDMI_1280X720 = 4,
	EM_BIZ_OUTPUT_HDMI_1920X1080 = 5,
} EMBIZOUTPUT;

//语言 
typedef enum
{
	EM_BIZ_LANG_ENG = 0, //英文
	EM_BIZ_LANG_SCH = 1, //中文简体
	EM_BIZ_LANG_TCH = 2, //中文繁体
} EMBIZLANGUAGE;

 //日期时间格式(数字 : 0=年月日;1=月日年;2=日月年)
typedef enum
{
	EM_BIZ_DATEFMT_YYMMDD = 0,
	EM_BIZ_DATEFMT_MMDDYY = 1,
	EM_BIZ_DATEFMT_DDMMYY = 2,
} EMBIZDATEFORMAT;

 //时间制式(数字 : 0=24小时制;1=12小时制)
 typedef enum
{
	EM_BIZ_TIMESTD_24H = 0,
	EM_BIZ_TIMESTD_12H = 1,
} EMBIZTIMESTANDARD;

//EM_BIZ_SYSTEMPARA
typedef struct
{
    s8 strDevName[32]; //设备名(字符串)
    u16 nDevId; //设备号(数字 : 0-65535)
    u8 nManual;
    u8 nVideoStandard; //视频制式(EMBIZVIDSTANDARD)
    u8 nAuthCheck; //是否校验权限(数字 : 0-不校验; 1-校验)
    u8 nOutput; //输出(EMBIZOUTPUT)
    u8 nLangId; //语言(EMBIZLANGUAGE)
    u8 nShowGuide; //是否显示开机向导(数字 : 0-否; 1-是)
    u16 nLockTime; //登录无操作超时锁定时间(数字:0表示永不; 单位秒s)
    u8 nDateFormat; //日期时间格式(EMBIZDATEFORMAT)
    u8 nTimeStandard; //时间制式(EMBIZTIMESTANDARD)
    u8 nCyclingRecord; //是否循环录像(数字:0=否;1=是)
    u8 nShowFirstMenue;
	//csp modify
	s8 sn[32];
	//QYJ
	u8 nTimeZone;
	u8 nSyscCheck;//自动同步时间服务器
	s8 strNtpServerAdress[32];
	u8 nSyncTimeToIPC;
	u8 P2PEnable;
	//QYJ
	u8 reserved[256-36];//QYJ
} SBizSystemPara;

typedef struct
{
	unsigned char channel_no;
	unsigned char enable;
	unsigned char stream_type;
	unsigned char trans_type;
	unsigned int protocol_type;
	unsigned int dwIp;
	unsigned short wPort;
	unsigned char force_fps;
	unsigned char frame_rate;
	char user[32];
	char pwd[32];
	char name[32];
	char uuid[64];
	char address[64];//onvif使用
	char ipc_type;
	//char reserved[63];
	//yaogang modify for yuetian private
	char main_audio_sw;//主码流音频开关
	char sub_audio_sw;//次码流音频开关
	char reserved[61];
} SBizIPCameraPara;

//EM_BIZ_PREVIEWPARA
typedef struct
{
    u8 nMode; //预览模式(EMBIZPREVIEWMODE)
    u8 nModePara; //预览模式扩展参数(数字:各模式下的左上角通道号)

    u8 nAudioOutChn; //音频输出通道 (数字:255表示自动选择)
    u8 nVolume; //音频输出音量(数字: 0~100)
    u8 nMute; //预览静音(数字 : 0=非静音;1=静音)

    u8 nShowTime; // 否显示时间(数字列表:0=否;1=是)
    //yaogang modify 20140918
    u8 nShowChnKbps;// 是否显示通道的实时码流带宽
    SPoint sTimePos; // 时间XY坐标
    u8 nShowRecState; // 录像状态显示与否(数字列表:0=否;1=是)

    u8 nVideoSrcType[128];	//视频输入源的类型
    
} SBizCfgPreview;

//复合流/视频流(0=视频流/1=复合流)
typedef enum
{
	EM_BIZ_REC_VID_ONLY = 0,
	EM_BIZ_REC_VID_AUDIO,
} EMBIZRECSTREAMTYPE;

//录像对应的编码通道类型 (0=主码流/1=子码流)
typedef enum
{
	EM_BIZ_CHN_VIDEO_SUB = 0, 		//视频主通道
	EM_BIZ_CHN_VIDEO_MAIN, 			//视频副通道
} EMBIZCHNTYPE;

//EM_BIZ_RECPARA
typedef struct
{
	u8 bRecording;  //是否正在录像
	u8 nEnable; //启用与否
	u8 nStreamType; //复合流/视频流(EMBIZRECSTREAMTYPE)
	u8 nEncChn; //录像对应的编码通道类型
	u8 nEncChnType; //录像对应的编码通道类型 (EMBIZCHNTYPE)
	u8 nPreTime; //预录时间
	u16 nRecDelay; //延时时间 
	u8 nRecExpire; // : 录像过期天数 (255表示永不过期)
	u8 nSchRecSchtype; //定时录像布防类型(EMBIZSCHTYPE)
	u8 nMDRecSchtype; //移动侦测录像布防类型(EMBIZSCHTYPE)
	u8 nSensorRecSchtype; //传感器报警录像布防类型(EMBIZSCHTYPE)
} SBizRecPara;

//EM_BIZ_IMAGEPARA
typedef struct
{
    u16 nHue; //各通道色度(数字：0-255)
    u16 nSaturation; //饱和度
    u16 nContrast; //对比度
    u16 nBrightness; //亮度
} SBizPreviewImagePara;

typedef struct
{
	u8 nBrightness;
	u8 nContrast;
	u8 nSaturation;
	u8 nHue;
} SBizVoImagePara;

//EM_BIZ_STROSD
typedef struct { 
    s8 strChnName[32];  // 各通道通道名
    u8 nShowChnName;  // 各通道是否显示通道名(数字列表:0=否;1=是)
    SPoint sChnNamePos; // 各通道名XY坐标
    u8 nEncShowChnName; // 编码各通道是否显示通道名(数字列表:0=否;1=是)
    SPoint sEncChnNamePos; // 编码各通道名XY坐标(以D1为基准)
    u8 nEncShowTime; // 编码各通道否显示时间(数字列表:0=否;1=是)
    SPoint sEncTimePos; // 编码各通道时间XY坐标(以D1为基准)
} SBizCfgStrOsd;

//EM_BIZ_RECTOSD
typedef struct { 
    u8 nShowRectOsd; // 各通道是否显示遮盖(数字列表:0=否;1=是)
    u8 nRealNum; //矩形遮盖个数
    u8 nMaxNum; //最大可返回的矩形遮盖个数;获得参数时用
    SRect* psRectOsd; // 各通道遮盖遮盖个数及XYWH坐标及宽高(以D1为基准)
} SBizCfgRectOsd;

//预览osd实现方式 :0=视频层;1=独立状态层OSD;2=界面层实现
typedef enum
{
    EM_BIZ_OSD_OPERATOR_VIDLAYER = 0,
    EM_BIZ_OSD_OPERATOR_STATUSLAYER,
    EM_BIZ_OSD_OPERATOR_UILAYER,
} EMBIZOSDOPERATOR;

 // 编码类习惯
typedef enum
{
	//视频编码类型
	 EM_BIZ_ENCODE_VIDEO_MJPEG = 26, 		//Motion JPEG
	 EM_BIZ_ENCODE_VIDEO_H261 = 31, 		//H.261
	 EM_BIZ_ENCODE_VIDEO_MP2 = 33, 			//MPEG2 video
	 EM_BIZ_ENCODE_VIDEO_H263 = 34, 		//H.263
	 EM_BIZ_ENCODE_VIDEO_MP4 = 97, 			//MPEG-4
	 EM_BIZ_ENCODE_VIDEO_H264 = 98, 		//H.264
	
	 //语音编码类型
	 EM_BIZ_ENCODE_AUDIO_PCMU = 0, 			//G.711 ulaw
	 EM_BIZ_ENCODE_AUDIO_PCMA = 8, 			//G.711 Alaw
	 EM_BIZ_ENCODE_AUDIO_G722 = 9, 			//G.722
	 EM_BIZ_ENCODE_AUDIO_G7231 = 4, 		//G.7231
	 EM_BIZ_ENCODE_AUDIO_G728 = 15, 		//G.728
	 EM_BIZ_ENCODE_AUDIO_G729 = 18, 		//G.729
	 EM_BIZ_ENCODE_AUDIO_RAW = 19, 			//raw audio
	 EM_BIZ_ENCODE_AUDIO_ADPCM = 20, 		//adpcm
	 EM_BIZ_ENCODE_AUDIO_ADPCM_HS = 21, 	//海思adpcm
	 EM_BIZ_ENCODE_AUDIO_MP3 = 96,			//mp3
} EMBIZENCTYPE;

//菜单输出同步(数字 : 0=Vga to Cvbs; 1=CvbsToVga; 2=同时只有一个输出; 3=分别输出;4=仅VGA(CVBS自动);5=仅CVBS(VGA自动))
typedef enum
{
	EM_BIZ_OUTPUT_VGA2CVBS = 0, 
	EM_BIZ_OUTPUT_CVBS2VGA,
	EM_BIZ_OUTPUT_VGAORCVBS,
	EM_BIZ_OUTPUT_VGAANDCVBS,
	EM_BIZ_OUTPUT_VGA_CVBSAUTO,
	EM_BIZ_OUTPUT_CVBS_VGAAUTO,
} EMBIZOUTPUTSYNC;

////////////////////////用户
#define MOD_BIZ_MAX_GROUP_NUM 16			//所支持的最大的用户组数目
#define MOD_BIZ_MAX_USER_NUM 128			//所支持的最大的用户数目

typedef enum						//return value
{
	EM_BIZ_USER_AUTHOR_ERROR = -1,	//check failed
	EM_BIZ_USER_AUTHOR_NO = 0,		//user has no author
	EM_BIZ_USER_AUTHOR_YES = 1,		//user has author
}EMBIZUSERCHECKAUTHOR;

typedef enum						//return value from UserCheckLoginPara()
{
	EM_BIZ_LOGIN_ERR_NOTHISUSER = -2,
	EM_BIZ_LOGIN_ERR_WRONGPASSWD = -3,
	EM_BIZ_LOGIN_ERR_NOLOGINTWINCE = -4,
	EM_BIZ_LOGIN_ERR_IP_NOPERMIT = -5,
	EM_BIZ_LOGIN_ERR_MAC_NOPERMIT = -6,
	//
}EMBIZUSERLOGINERR;

typedef enum						//return value from ModUserAddUser()
{
	EM_BIZ_ADDUSER_INVALID_NAME = -2,
	EM_BIZ_ADDUSER_EXIST_NAME = -3,		
	EM_BIZ_ADDUSER_TOMAXNUM = -4,	
}EMBIZADDUSERERR;

typedef enum						//return value from ModUserDeleteUser
{
	EM_BIZ_DELUSER_INVALID_NAME = -2,
	EM_BIZ_DELUSER_USER_LOGINED = -3,	
}EMBIZDELUSERERR;

typedef enum
{
	EM_BIZ_USER_AUTHOR_SHUTDOWN = 0,
	EM_BIZ_USER_AUTHOR_SYSTEM_RESET,
	EM_BIZ_USER_AUTHOR_SYSTEM_UPDATE,
	EM_BIZ_USER_AUTHOR_SYSTEM_MAINTAIN,
	EM_BIZ_USER_AUTHOR_USER_MANAGE,	
	EM_BIZ_USER_AUTHOR_AUDIO_TALK,
	EM_BIZ_USER_AUTHOR_DISK_MANAGE,
	EM_BIZ_USER_AUTHOR_DATA_MANAGE,
	EM_BIZ_USER_AUTHOR_SYSINFO_VIEW,
	EM_BIZ_USER_AUTHOR_LOG_VIEW,
	EM_BIZ_USER_AUTHOR_LOG_DELETE,
	EM_BIZ_USER_AUTHOR_LOG_BACKUP,	
	EM_BIZ_USER_AUTHOR_REMOTE_LOGIN,
	EM_BIZ_USER_AUTHOR_CHN_NAME_SETUP,
	EM_BIZ_USER_AUTHOR_SYSTEM_SETUP,
	EM_BIZ_USER_AUTHOR_NORMAL_SETUP,
	EM_BIZ_USER_AUTHOR_RECORD_SETUP,
	EM_BIZ_USER_AUTHOR_ENCODE_SETUP,
	EM_BIZ_USER_AUTHOR_PTZ_SETUP,
	EM_BIZ_USER_AUTHOR_ALARM_SETUP,
	EM_BIZ_USER_AUTHOR_NETWORK_SETUP,
	EM_BIZ_USER_AUTHOR_VIDEOMOTION_SETUP,
	//
	EM_BIZ_USER_AUTHOR_LIVEVIEW = 50,
	EM_BIZ_USER_AUTHOR_REMOTEVIEW,
	EM_BIZ_USER_AUTHOR_RECORD,
	EM_BIZ_USER_AUTHOR_PLAYBACK,
	EM_BIZ_USER_AUTHOR_RECFILE_BACKUP,
	EM_BIZ_USER_AUTHOR_PTZCTRL,
	//
	EM_BIZ_USER_AUTHOR_NULL = 65,
}EMBIZUSEROPERATION;

typedef enum
{
	EM_BIZUSER_LOGINFROM_GUI = 0,
	EM_BIZUSER_LOGINFROM_WEB,
	EM_BIZUSER_LOGINFROM_CONSOLE,
	//
}EMBIZUSERLOGINFROM;

typedef struct
{
	u8 UserName[15];
	u8 Password[15];
	EMBIZUSERLOGINFROM emFrom;
	u32 nPcIp;
	u64 nPcMac;
}SBizUserLoginPara;

typedef struct
{
	u8 nAuthor[64];
	u64 nLiveView[4];
	u64 nRemoteView[4];
	u64 nRecord[4];
	u64 nPlayback[4];
	u64 nBackUp[4];
	u64 nPtzCtrl[4];
}SBizUserAuthority;

typedef struct
{
	u8 GroupName[15];
	u8 GroupDescript[30];
	u8 nDoWithUser;	//对组进行修改和删除时，是否使操作应用到该组下的所有用户；[0]:否，[非0]:是
	SBizUserAuthority GroupAuthor;
}SBizUserGroupPara;

typedef struct
{
	u8 UserName[15];
	u8 Password[15];
	u8 GroupBelong[15];
	u8 UserDescript[30];
	u8 emLoginTwince;
	u8 emIsBindPcMac;
	u64 PcMacAddress;
	SBizUserAuthority UserAuthor;
}SBizUserUserPara;

typedef struct
{
	u64 nFlagGroupSetted;
	u64 nFlagUserSetted[2];
	SBizUserUserPara sUserPara[MOD_BIZ_MAX_USER_NUM];
	SBizUserGroupPara sGroupPara[MOD_BIZ_MAX_GROUP_NUM];
}SBizUserPara;

typedef struct 
{
	s8** list;
	s32 nLength;
	s32 nRealLength;
}SBizUserListPara;

//日志

//系统控制
#define BIZ_LOG_MASTER_SYSCONTROL     					0x01      
#define BIZ_LOG_SLAVE_START_DWELL      					0x01
#define BIZ_LOG_SLAVE_STOP_DWELL       					0x02
#define BIZ_LOG_SLAVE_MUTE            					0x03
#define BIZ_LOG_SLAVE_CHANGE_VOICE    					0x04
#define BIZ_LOG_SLAVE_CONTROL_PTZ      					0x05
#define BIZ_LOG_SLAVE_SNAP_SUCCESS     					0x06
#define BIZ_LOG_SLAVE_START_MANUAL_REC 					0x07
#define BIZ_LOG_SLAVE_STOP_MANUAL_REC  					0x08
#define BIZ_LOG_SLAVE_MANUAL_ALARM     					0x09
#define BIZ_LOG_SLAVE_CLEAR_ALARM      					0x0a

#define BIZ_LOG_SLAVE_POWER_OFF        					0x0b
#define BIZ_LOG_SLAVE_REBOOT   	       					0x0c
#define BIZ_LOG_SLAVE_LOGIN            					0x0d
#define BIZ_LOG_SLAVE_LOGOFF           					0x0e
#define BIZ_LOG_SLAVE_FORMAT_SUCCESS   				0x0f
#define BIZ_LOG_SLAVE_FORMAT_FAIL      				0x10
#define BIZ_LOG_SLAVE_UPDATE           					0x11
//csp modify
#define BIZ_LOG_SLAVE_POWER_ON        					0x12
//yg modify 20140814
#define BIZ_LOG_SLAVE_START_MD         					0x13
#define BIZ_LOG_SLAVE_STOP_MD          					0x14

#define BIZ_LOG_SLAVE_VIDEO_LOST        				0x15
#define BIZ_LOG_SLAVE_VIDEO_RESUME        				0x16

#define BIZ_LOG_SLAVE_START_LOCAL_ALARM        		0x17
#define BIZ_LOG_SLAVE_STOP_LOCAL_ALARM        			0x18

#define BIZ_LOG_SLAVE_START_IPCEXT        				0x19
#define BIZ_LOG_SLAVE_STOP_IPCEXT        				0x1a

#define BIZ_LOG_SLAVE_START_485EXT        				0x1b
#define BIZ_LOG_SLAVE_STOP_485EXT        				0x1c


#define BIZ_LOG_SLAVE_DISK_WRERR        				0x1d

#define BIZ_LOG_SLAVE_DISK_NONE	        				0x1e

#define BIZ_LOG_SLAVE_IPCCOVER        					0x1f
#define BIZ_LOG_SLAVE_IPCCOVER_RESUME        			0x20

#define BIZ_LOG_SLAVE_DISK_LOST        					0x21





//系统设置
#define BIZ_LOG_MASTER_SYSCONFIG 						0x02
#define BIZ_LOG_SLAVE_ENABLE_PWDCHECK                   0x01
#define BIZ_LOG_SLAVE_DISABLE_PWDCHECK                  0x02
#define BIZ_LOG_SLAVE_CHANGE_VO                         0x03
#define BIZ_LOG_SLAVE_CHANGE_MAX_NETUSER                0x04
#define BIZ_LOG_SLAVE_CHANGE_VIDEOFORMAT                0x05
#define BIZ_LOG_SLAVE_CHANGE_LANGUAGE                   0x06
#define BIZ_LOG_SLAVE_CHANGE_ZONE                       0x07
#define BIZ_LOG_SLAVE_ENABLE_AUTO_TIMESYNC              0x08
#define BIZ_LOG_SLAVE_DISABLE_AUTO_TIMESYNC             0x09
#define BIZ_LOG_SLAVE_CHANGE_NTP_SERVER                 0x0a
#define BIZ_LOG_SLAVE_DST_CONFIG                        0x0b
#define BIZ_LOG_SLAVE_CHANGE_CHN_NAME                   0x0c
#define BIZ_LOG_SLAVE_CHANGE_REC_ENABLE                 0x0d
#define BIZ_LOG_SLAVE_CHANGE_REC_AUDIO_ENABLE           0x0e
#define BIZ_LOG_SLAVE_CHANGE_REC_CODESTREAM             0x0f
#define BIZ_LOG_SLAVE_CHANGE_TIME_BEFORE_ALARM          0x10
#define BIZ_LOG_SLAVE_CHANGE_TIME_AFTER_ALARM           0x11
#define BIZ_LOG_SLAVE_CHANGE_REC_EXPIRETIME             0x12
#define BIZ_LOG_SLAVE_CHANGE_OSD                        0x13
#define BIZ_LOG_SLAVE_CHANGE_TIMER_RECSCHE              0x14
#define BIZ_LOG_SLAVE_CHANGE_MD_RECSCHE                 0x15
#define BIZ_LOG_SLAVE_CHANGE_SENSORALARM_RECSCHE        0x16
#define BIZ_LOG_SLAVE_CHANGE_SENSOR_ENABLE              0x17
#define BIZ_LOG_SLAVE_CHANGE_SENSOR_TYPE                0x18
#define BIZ_LOG_SLAVE_CHANGE_SENSOR_ALARMDEAL           0x19
#define BIZ_LOG_SLAVE_CHANGE_SENSOR_DETECTION_SCHE      0x1a
#define BIZ_LOG_SLAVE_CHANGE_MD_ENABLE                  0x1b
#define BIZ_LOG_SLAVE_CHANGE_MD_AREA_SETUP              0x1c
#define BIZ_LOG_SLAVE_CHANGE_MD_ALARMDEAL               0x1d
#define BIZ_LOG_SLAVE_CHANGE_MD_DETECTION_SCHE          0x1f
#define BIZ_LOG_SLAVE_CHANGE_VIDEOLOSS_ALARMDEAL        0x20
#define BIZ_LOG_SLAVE_CHANGE_ALARMOUT_SCHE              0x21
#define BIZ_LOG_SLAVE_ENABLE_SOUND_ALARM                0x22
#define BIZ_LOG_SLAVE_DISABLE_SOUND_ALARM               0x23
#define BIZ_LOG_SLAVE_CHANGE_IP                         0x24
#define BIZ_LOG_SLAVE_CHANGE_HTTP_PORT                  0x25
#define BIZ_LOG_SLAVE_CHANGE_SERVER_PORT                0x26
#define BIZ_LOG_SLAVE_CHANGE_NET_CODESTREAM             0x27
#define BIZ_LOG_SLAVE_CHANGE_OUTBOX_CONFIG              0x28
#define BIZ_LOG_SLAVE_CHANGE_INBOX_CONFIG               0x29
#define BIZ_LOG_SLAVE_CHANGE_DNS_CONFIG                 0x2a
#define BIZ_LOG_SLAVE_ADD_USER                          0x2b
#define BIZ_LOG_SLAVE_DELETE_USER                       0x2c
#define BIZ_LOG_SLAVE_CHANGE_USER_AUTH                  0x2d
#define BIZ_LOG_SLAVE_CHANGE_PWD                        0x2e
#define BIZ_LOG_SLAVE_CHANGE_SERIAL_CONFIG              0x2f
#define BIZ_LOG_SLAVE_CHANGE_CRUISELINE                 0x30
#define BIZ_LOG_SLAVE_FACTORY_RESET                     0x31

//回放
#define BIZ_LOG_MASTER_PLAYBACK 						0x03
#define BIZ_LOG_SLAVE_PB_STOP 							0x01
#define BIZ_LOG_SLAVE_PB_PAUSE 							0x02
#define BIZ_LOG_SLAVE_PB_RESUME 						0x03
#define BIZ_LOG_SLAVE_PB_BACKWARD 						0x04
#define BIZ_LOG_SLAVE_PB_FORWARD 						0x05
#define BIZ_LOG_SLAVE_PB_START 							0x06  //cw_log

//备份
#define BIZ_LOG_MASTER_BACKUP 							0x04
#define BIZ_LOG_SLAVE_START_BACKUP 						0x01
#define BIZ_LOG_SLAVE_END_BACKUP 						0x02

//检索
#define BIZ_LOG_MASTER_SEARCH 							0x05
#define BIZ_LOG_SLAVE_SEARCH_DATA_BYTIME 				0x01
#define BIZ_LOG_SLAVE_SEARCH_RECTIME 					0x02

//查看信息
#define BIZ_LOG_MASTER_CHECKINFO 						0x06

//错误
#define BIZ_LOG_MASTER_ERROR 							0x07
#define BIZ_LOG_SLAVE_ERROR_CRASH 						0x01  //cw_log
#define BIZ_LOG_SLAVE_ERROR_LOGINPWD 					0x02  //cw_log
#define BIZ_LOG_SLAVE_ERROR_HDD_ABNORMAL				0x03  //硬盘异常
#define BIZ_LOG_SLAVE_ERROR_ENC_ABNORMAL				0x04  //编码异常
#define BIZ_LOG_SLAVE_ERROR_FILE_ABNORMAL 				0x05  //录像文件异常

typedef struct
{
	u32 nMaxLogs;
	void* pHddMgr;
}SBizLogInitPara;

typedef struct
{
	u8  nMode;									//查询方式
	u16 nMasterMask;							//查询类型
	time_t nStart;								//起始时间
	time_t nEnd;								//结束时间
}SBizLogSearchPara;

typedef struct
{
	s8  nMasterType;							//返回主类型
	s8  nSlaveType;								//返回副类型
	u8  nchn;									//报警源的序号或通道
	char aIp[16];								//IP地址
	char aUsername[32];							//用户名
}SBizLogInfo;

typedef struct
{
	u16  nType;									//返回类型
	u8  nchn;									//报警源的序号或通道
	time_t  nTime;								//返回起始时间
	char aIp[16];								//IP地址
	char aUsername[32];							//用户名
	u32 logId;									//cw_log
}SBizLogInfoResult;

typedef struct
{
	u32 nRealNum;								//返回的日志数
	u32 nMaxLogs;
	SBizLogInfoResult* psLogList;	    		//返回日志查询的结果
}SBizLogResult;

typedef void (*BIZLOGEXPORTCB)(SBizLogInfoResult* pLog, u8* strLog);
typedef void (*BIZPBREGFUNCCB)(void);

//EM_BIZ_DVRINFO
typedef struct
{
    s8 strVersion[64]; //版本号
    s8 strModel[32]; //型号
    s8 sproductnumber[32];//产品号  9624
    u8 nPreviewNum; // 预览通道数(数字)
    u8 nVidMainNum; // 主码流通道数(数字)
    u8 nVidSubNum; // 子码流通道数(数字)
    u8 nAudNum; // 音频通道数(数字)
    u8 nSnapNum; // 抓图通道数(数字)
    u8 nVoipNum; // 语音对讲通道数(数字)
    u8 nRecNum; // 录像通道数(数字)
    u8 nSensorNum; // 传感器通道数(数字)
    u8 nAlarmoutNum; // 报警输出通道数(数字)
    u8 nBuzzNum; // 蜂鸣器个数(数字)
    u8 nMaxHddNum; // 硬盘支持个数(数字)
    u32 nMaxMainFrameSize; // 最大主码流帧大小(数字:十六进制)
    u32 nMaxSubFrameSize; // 最大子码流帧大小(数字:十六进制)
    u32 nMaxAudFrameSize; // 最大音频帧大小(数字:十六进制)
    u32 nMaxSnapFrameSize; // 最大抓图文件大小(数字:十六进制)

    u8 nOsdOperator; // 预览osd实现方式 参考(EMBIZOSDOPERATOR)
    u8 nStrOsdNum; // 预览支持的字符串osd数(数字)
    u8 nImgOsdNum; // 预览支持的图片osd数(数字)
    u8 nRectOsdNum; // 预览支持的矩形遮盖osd数(数字)
    SRgb sRectOsdRgb; // 预览矩形遮盖osd色彩(十六进制rgb:如ff ff ff)
    u8 nSplitLineNum; // 预览支持的分割线osd数(数字:横线+竖线)
    u16 nRefWidth; // Osd参考宽
    u16 nRefHeight; // Osd参考高

    u8 nAudioEncType; // 音频编码类习惯(EMBIZENCTYPE)
    u32 nSampleRate; // 采样率(数字:8000/16000/32000)
    u8 nSampleDuration; // 采样时长(数字,单位毫秒)
    u32 nSampleSize; // 采样大小(数字)
    u8 nBitWidth; // 采样位宽(数字8/16)

    u8 nVoipEncType; // 音频编码类习惯(EMBIZENCTYPE)
    u32 nVoipSampleRate; // 采样率(数字:8000/16000/32000)
    u8 nVoipSampleDuration; // 采样时长(数字,单位毫秒)
    u32 nVoipSampleSize; // 采样大小(数字)
    u8 nVoipBitWidth; // 采样位宽(数字8/16)

    u8 nMDAreaRows; // 移动侦测区域行数
    u8 nMDAreaCols; // 移动侦测区域列数
    u8 nMDSenseMin; // 移动侦测灵敏度最小值
    u8 nMDSenseMax; // 移动侦测灵敏度最大值
    
    u8 nSchSegments; //布防时间段数最大值
    u8 nOutputSync; //菜单输出同步(EMBIZOUTPUTSYNC)

	u8 nBuzzDuration; //蜂鸣时间时长非零将是间歇式蜂鸣(单位s) BuzzDuration同时设非零有效
	u8 nBuzzInterval; //蜂鸣时间间隔非零将是间歇式蜂鸣(单位s)
	u8 nNVROrDecoder;	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
} SBizDvrInfo;

/*
 配置接口存取网络参数结构
*/

typedef struct _sNetAdvancePara
{
	char	szSmtpServer[64];
	
	// 发件人邮箱
	char	szSendMailBox[64];
	char	szSMailPasswd[64];

    u8		nSSLFlag;

	// 收件人邮箱
	char	szReceiveMailBox[64];
	char	szReceiveMailBox2[64];
	char	szReceiveMailBox3[64];

	// 附加图片
	char	szSnapPic[64];
	
	// 抓图间隔
	u32		nSnapIntvl;
	
} SBizNetAdvancePara;

typedef struct _sNetConfigPara
{
	// 基本参数
	u8     	Version[8];			    // 8字节的版本信息
    char   	HostName[16];           // 主机名
    u32  	HostIP;                 // IP 地址
    u32  	Submask;                // 子网掩码
    u32  	GateWayIP;              // 网关 IP
    u32  	DNSIP;                  // DNS IP
    u32  	DNSIPAlt;                 // DNS IP
    u8 		MAC[18];				// MAC Address

    // 外部接口
    u32   	AlarmServerIP;          // 报警中心IP
    u16 	AlarmServerPort;        // 报警中心端口
    u32   	SMTPServerIP;           // SMTP server IP
    u16 	SMTPServerPort;         // SMTP server port
	
    u32   	LogServerIP;            // Log server IP
    u16 	LogServerPort;          // Log server port

    // 本机服务端口
    u16 	HttpPort;               	// HTTP服务端口号
    u16 	MobilePort;              	// 手机监控端口号
    
    u16 	TCPMaxConn;             	// TCP 最大连接数
    u16 	TCPPort;                	// TCP 侦听端口

	u8		nSubStreamMax;				// 子码流数上限
	u8		nSendBufNumPerChn;			// 每通道发送buff数
	u32		nFrameSizeMax;				// 帧buff大小上限
	
	// 网路子码流
	u8		nVideoMediaType;			// 视频编码类型
	u8		nAudioMediaType;			// 预览音频编码类型
	u8		nVoipMediaType;				// 对讲音频编码类型
	u8		nAudioSampleMode;			// 音频采样模式

    u16 	UDPPort;                	// UDP 侦听端口

    u8		McastFlag;
    u16 	McastPort;              	// 组播端口
    u32 	McastIP;                	// 组播IP
    
	u8		PPPOEFlag;					// pppoe 使能开关
	u8		PPPOEUser[64];
	u8		PPPOEPasswd[64];

	u8		DhcpFlag;					// Dhcp 使能开关
	
	u8		DDNSFlag;					// DDNS 控制位
	u8		DDNSDomain[64];
	u8		DDNSUser[64];
	u8		DDNSPasswd[64];
	u16		UpdateIntvl;				// DDNS	IP更新间隔
	
    // 其他
    u8    	MonMode;                	// 监视协议 TCP|UDP|MCAST
    u8    	PlayMode;               	// 回放协议 TCP|UDP|MCAST
    u8    	AlmSvrStat;             	// 报警中心状态 <开/关>
	
	// 高级配置
    SBizNetAdvancePara	sAdancePara;
	
	//csp modify 20130321
	u8		UPNPFlag;
} SBizNetPara;

/*
 Network state
*/
typedef enum
{
	EM_BIZNS_CONNECTED = 0,
	EM_BIZNS_DISCONNECTED,
	
} EM_BIZNET_STATE;


/*
 邮件参数
*/
typedef struct _sBizMailContent
{
	char* pSrcUsr;
	char* pSrcBox;
	char* pToUsr;
	char* pToBox;

	char* pUserName;
	char* pPasswd;

	char* pSmtp;
	char* pSubject;
	char* pBody;

	u8	  bAttach;	
} SBizMailContent;


#define BIZ_DDNS_USER_LEN_MAX	32

/*
 DDNS action
*/
typedef enum
{
	EM_BIZDDNS_START,
	EM_BIZDDNS_REGIST,
	EM_BIZDDNS_STOP,
} EM_BIZ_DDNS_ACT;

/*
 DDNS type
*/
typedef enum
{
	EM_BIZDDNS_PROT_TL = 1,
	EM_BIZDDNS_PROT_3322,
	EM_BIZDDNS_PROT_DYDNS,
	EM_BIZDDNS_PROT_DVRNET,
	EM_BIZDDNS_PROT_NOIP,
	EM_BIZDDNS_PROT_JMDVR,
	EM_BIZDDNS_PROT_CHANGEIP,
	EM_BIZDDNS_PROT_JSJDVR,
	EM_BIZDDNS_PROT_CCTVSVR,
	EM_BIZDDNS_PROT_CCTVDVR,
	EM_BIZDDNS_PROT_KONLAN,
	EM_BIZDDNS_PROT_MYQSEE,
	EM_BIZDDNS_PROT_DYNDDNS,
	EM_BIZDDNS_PROT_PEOPLE,
	EM_BIZDDNS_PROT_MAX
} EM_BIZ_DDNS_PROT;

/*
 DDNS control para
*/
typedef struct _sBizNetDdnsPara
{
	EM_BIZ_DDNS_PROT	eProt;

	char szDomain[BIZ_DDNS_USER_LEN_MAX];
	char szUser[BIZ_DDNS_USER_LEN_MAX];
	char szPasswd[BIZ_DDNS_USER_LEN_MAX];
	
} sBizNetDdnsPara;


// 网络状态及通知回调
typedef enum
{
	EM_BIZNET_STATE_DHCP,
	EM_BIZNET_STATE_PPPOE,
	EM_BIZNET_STATE_MAIL,
	EM_BIZNET_STATE_DDNS,
	EM_BIZNET_STATE_CONN,
} EM_BIZNET_ACT;

typedef struct _sBizNetStateRslt
{
	u8 nErr; // 0 succ, 1 fail
} SBizNetStateRslt;

typedef struct _sBizNetStateNotify
{
	EM_BIZNET_ACT eAct;
	SBizNetStateRslt sRslt;
} SBizNetStateNotify;

/*
	动态IP, 
	静态IP,
	PPPoE拨号连接
	..
*/
typedef enum
{
	BIZ_ECW_STATIC,
	BIZ_ECW_DHCP,
	BIZ_ECW_PPPOE,
	BIZ_ECW_MAX
} BIZ_E_CONN_WAY;

/*
	已连接，
	未连接
	..
*/
typedef enum
{
	BIZ_ECS_ONLINE,
	BIZ_ECS_OFFLINE,
	BIZ_ECS_MAX
} BIZ_E_CONN_STATE;

typedef struct _sBizNetConnSta
{
	BIZ_E_CONN_WAY 		eWay;
	BIZ_E_CONN_STATE	eState;
} SBizNetConnStatus;


///////////////////////设置与获取参数配置及其他所有模块相关//////////////////////////
//for BizSetPara/BizGetPara
typedef enum
{
	//系统型参数
	EM_BIZ_DVRINFO = 0, //dvr设备属性;只读
	EM_BIZ_SYSTEMPARA, //设备信息;SBizSystemPara
	EM_BIZ_PREVIEWPARA, //获得预览参数;参考SBizCfgPreview
	EM_BIZ_PATROLPARA, //视频轮巡或巡逻;参考SBizCfgPatrol
	EM_BIZ_SYSTEMMAINTAIN,//系统维护信息

	EM_BIZ_USERPARA=10,  //用户参数
	EM_BIZ_USER_GETUSERLIST,//得到用户列表
	EM_BIZ_USER_GETUSERINFO,//得到某个用户信息
	EM_BIZ_USER_ADDUSER,//添加用户
	EM_BIZ_USER_MODIFYUSER,//修改用户
	EM_BIZ_USER_DELETEUSER,//删除用户
	EM_BIZ_USER_GETGROUPLIST,//得到用户组列表
	EM_BIZ_USER_GETGROUPINFO,//得到某个用户组信息
	
	EM_BIZ_SYSCOMPLEXPARA=30, //系统杂项参数

	//通道型参数
	EM_BIZ_IMAGEPARA = 100,  //预览色彩SBizPreviewImagePara
	EM_BIZ_STROSD,  //字符串osd设置SBizCfgStrOsd
	EM_BIZ_RECTOSd,  //矩形osd设置SBizCfgRectOsd
	EM_BIZ_ENCMAINPARA,  //主码流编码参数SBizEncodePara
	EM_BIZ_ENCSUBPARA, //子码流编码参数SBizEncodePara
	EM_BIZ_ENCMOBPARA, //手机码流编码参数SBizEncodePara
	EM_BIZ_RECPARA, //录像参数SBizRecPara
	EM_BIZ_SENSORPARA, //传感器参数SBizAlarmSensorPara
	//yaogang modify 20141010
	EM_BIZ_IPCCOVERPARA,
	EM_BIZ_IPCEXTSENSORPARA, //IPC外部传感器参数SBizAlarmIPCExtSensorPara
	EM_BIZ_485EXTSENSORPARA,
	EM_BIZ_HDDPARA,
	
	EM_BIZ_VMOTIONPARA, //移动侦测SBizAlarmVMotionPara
	EM_BIZ_VBLINDPARA, //遮挡报警参数SBizAlarmVBlindPara
	EM_BIZ_VLOSTPARA, //视频丢失参数SBizAlarmVLostPara
	EM_BIZ_ALARMOUTPARA, //报警输出参数SBizAlarmOutPara
	EM_BIZ_ALARMBUZZPARA, //蜂鸣器参数SBizAlarmBuzzPara
   	 //一般报警事件触发参数设置
	EM_BIZ_DISPATCH_SENSOR, //SBizAlarmDispatch
	EM_BIZ_DISPATCH_IPCCOVER,
	EM_BIZ_DISPATCH_IPCEXTSENSOR, //SBizAlarmDispatch
	EM_BIZ_DISPATCH_485EXTSENSOR, //SBizAlarmDispatch
	EM_BIZ_DISPATCH_HDD, 
	
	EM_BIZ_DISPATCH_VMOTION, //SBizAlarmDispatch
	EM_BIZ_DISPATCH_VBLIND, //SBizAlarmDispatch
	EM_BIZ_DISPATCH_VLOST, //SBizAlarmDispatch
	//一般报警事件布防设置
	EM_BIZ_SCHEDULE_SENSOR, //传感器报警SBizAlarmSchedule
	EM_BIZ_SCHEDULE_IPCEXTSENSOR,
	EM_BIZ_SCHEDULE_VMOTION, //移动侦测报警布防SBizAlarmSchedule
	EM_BIZ_SCHEDULE_VBLIND, //遮挡报警布防SBizAlarmSchedule
	EM_BIZ_SCHEDULE_VLOST, //视频丢失报警布防SBizAlarmSchedule
	//报警输出事件布防设置
	EM_BIZ_SCHEDULE_ALARMOUT, //报警输出布防SBizAlarmSchedule
	EM_BIZ_SCHEDULE_BUZZ, //暂时不支持;蜂鸣器布防SBizAlarmSchedule
	//回放色彩控制
	EM_BIZ_VO_IMAGEPARA, //SBizPreviewImagePara
	EM_BIZ_SNAP_PARA, //yaogang modify 20150105
	
	//NVR used
	//IPCamera参数控制
	EM_BIZ_IPCAMERAPARA = 200,//
	//yaogang modify 20141209
	//深广平台参数
	EM_BIZ_SGPARA,
	EM_BIZ_SGALARMPARA,
	EM_BIZ_SGDAILYPARA,

	EM_BIZ_SCHEDULE_RECTIMING = 1000, //定时录像布防SBizSchedule
	EM_BIZ_SCHEDULE_RECSENSOR, //传感器报警录像复方SBizSchedule
	EM_BIZ_SCHEDULE_RECMOTION, //移动侦测录像布防SBizSchedule
    
	EM_BIZ_PTZPARA = 2000, //云台参数//SBizPtzPara

	EM_BIZ_NETPARA = 3000, // 网络参数 //SBizNetPara
} EMBIZPARATYPE;

typedef struct
{
    EMBIZPARATYPE emBizParaType; //参数类型
    u8 nChn; //对通道型参数有效
} SBizParaTarget;

//参数选项列表枚举
typedef enum
{
    //**********************参数列表（只读）
    EM_BIZ_CFG_PARALIST_BASE = 0,
    EM_BIZ_CFG_PARALIST_LANGSUP, // 支持的语言列表
    EM_BIZ_CFG_PARALIST_LANGSRC, // 支持的语言资源文件列表 ;仅支持字符串列表
    EM_BIZ_CFG_PARALIST_PREVIEWMODE, // 支持的预览模式
    EM_BIZ_CFG_PARALIST_BITRATE, // 支持的位率列表
    EM_BIZ_CFG_PARALIST_BITTYPE, // 位率类型列表
    EM_BIZ_CFG_PARALIST_PICLEVEL, // 图像质量列表
    EM_BIZ_CFG_PARALIST_VMAINRESOLH_NUM, // 主码流通用支持的分辨率列表//cw_9508S
    EM_BIZ_CFG_PARALIST_VMAINRESOL, // 主码流通用支持的分辨率列表
    EM_BIZ_CFG_PARALIST_VMAINRESOL2, // 主码流通用支持的分辨率列表
    EM_BIZ_CFG_PARALIST_VSUBRESOL, // 子码流通用支持的分辨率列表
    EM_BIZ_CFG_PARALIST_VMOBRESOL, // 手机码流通用支持的分辨率列表
    EM_BIZ_CFG_PARALIST_FPSPAL, // P制常用下支持的帧率列表
    EM_BIZ_CFG_PARALIST_FPSNTSC, // N制常用下支持的帧率列表
    EM_BIZ_CFG_PARALIST_FPSPALD1, // P制D1下支持的帧率列表
    EM_BIZ_CFG_PARALIST_FPSNTSCD1, // N制D1下支持的帧率列表
    EM_BIZ_CFG_PARALIST_FPSPALCIF, // P制Cif下支持的帧率列表
    EM_BIZ_CFG_PARALIST_FPSNTSCCIF, // N制Cif下支持的帧率列表
    EM_BIZ_CFG_PARALIST_STREAMTYPE, // 录像码流类型列表
    EM_BIZ_CFG_PARALIST_VIDEOSTANDARD, // 视频制式列表
    EM_BIZ_CFG_PARALIST_OUTPUT, // 视频输出列表
    EM_BIZ_CFG_PARALIST_SWITCHPICTURE, //轮巡画面选择
    EM_BIZ_CFG_PARALIST_LOCKTIME, // 锁定超时间选项列表
    EM_BIZ_CFG_PARALIST_DATEFORMAT, // 日期格式列表
    EM_BIZ_CFG_PARALIST_TIMESTANDARD, // 时间制式列表
    EM_BIZ_CFG_PARALIST_SITEMAINOUT, // 现场主输出列表
    EM_BIZ_CFG_PARALIST_INTERVAL, // 时间间隔列表
    EM_BIZ_CFG_PARALIST_SENSORTYPE, // 传感器类型列表
    EM_BIZ_CFG_PARALIST_ALARMOUTTYPE, // 报警输出类型列表
    EM_BIZ_CFG_PARALIST_DELAY, // 延时时间选项列表
    EM_BIZ_CFG_PARALIST_PRETIME, // 预录时间选项列表
    EM_BIZ_CFG_PARALIST_RECDELAY, // 录像延时时间选项列表
    EM_BIZ_CFG_PARALIST_RECEXPIRE, // 录像过期天数选项列表
    EM_BIZ_CFG_PARALIST_SCHEDULETYP, // 布防类型选项列表
    EM_BIZ_CFG_PARALIST_PTZLINKTYPE, // 云台联动类型选项列表
    EM_BIZ_CFG_PARALIST_BAUDRATE, // 波特率选项列表
    EM_BIZ_CFG_PARALIST_DATABIT, // 数据位类型选项列表
    EM_BIZ_CFG_PARALIST_STOPBIT, // 停止位类型选项列表
    EM_BIZ_CFG_PARALIST_CHECKTYPE, // 校验类型类型选项列表
    EM_BIZ_CFG_PARALIST_FLOWCTRLTYPE, // 流控方式类型选项列表
    EM_BIZ_CFG_PARALIST_DDNSDOMAIN, // DDNSDomain
    EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL, // DDNS IP update 
    EM_BIZ_CFG_PARALIST_VIDEOSRCTYPE, // 视频源类型选项列表
	
    //
    EM_BIZ_CFG_PARALIST_PTZPROTOCOL, // 云台协议选项列表
    //yaogang modify 20141210 ShenGuang
    EM_BIZ_CFG_PARALIST_RECORD_SNAP_RES,//抓图分辨率
    EM_BIZ_CFG_PARALIST_RECORD_SNAP_QUALITY,//抓图质量
    EM_BIZ_CFG_PARALIST_RECORD_SNAP_INTERVAL,//抓图时间间隔
    EM_BIZ_CFG_PARALIST_RepairName,//报修系统名称
    EM_BIZ_CFG_PARALIST_RepairType,//报修系统类型
    EM_BIZ_CFG_PARALIST_Maintain,//维护维修
    EM_BIZ_CFG_PARALIST_Test,//测试
    EM_BIZ_CFG_PARALIST_AlarmInput, //报警图片配置: 输入端子
} EMBIZCFGPARALIST;

//yaogang modify 20141209
//深广平台参数
typedef struct
{
	char AgentID[100];
	char CenterIP[100];
	u8 RunEnable;//运行状态启用
	u8 AlarmEnable;//报警联动启用
	u8 DailyPicEnable;//日常图像启用
	u8 normal_exit;// 1: 系统正常关机0: 异常退出
} SBizSGParam;

//报警图片配置，每个报警通道一个结构
//m路本地，n路IPC外部报警(m 16--n 32)
typedef struct
{
	u8 PreTimes;		//前置时间
	u8 Interval;		//间隔时间
	u8 StillTimes;		//延录时间
	u8 AlarmInput;	//输入端子
	u32 IpcChn;		//每个IPC通道各占一位，0xffff ffff表示所有通道
} SBizAlarmPicCFG;

//日常图片配置
typedef struct
{
	u8 TimeEnable;	//定时上传时间使能
	u8 hour;
	u8 min;
	u8 sec;
} SBizSGTime;

typedef struct
{
	SBizSGTime Time1;		//定时上传时间1 
	SBizSGTime Time2;		//定时上传时间2
	u32 IpcChn1;		//每个IPC通道各占一位，0xffff ffff表示所有通道
	u32 IpcChn2;		//每个IPC通道各占一位，0xffff ffff表示所有通道
} SBizDailyPicCFG;

#if 0
typedef struct
{
	u8 TimeEnable;	//定时上传时间使能
	u8 hour;
	u8 min;
	u8 sec;
} SBizSGTime;
typedef struct
{
	u8 PreTimes;		//前置时间
	u8 Interval;		//间隔时间
	u8 StillTimes;		//延录时间
	u8 AlarmInput;	//输入端子
	u32 IpcChn;		//16个IPC通道各占一位，0x1000 0000表示所有通道
} SBizAlarmPicCFG;
typedef struct
{
	SBizSGTime Time1;		//定时上传时间1 
	SBizSGTime Time2;		//定时上传时间2
	u32 IpcChn1;		//16个IPC通道各占一位，0x1000 0000表示所有通道
	u32 IpcChn2;		//16个IPC通道各占一位，0x1000 0000表示所有通道
} SBizDailyPicCFG;
typedef struct
{
	char AgentID[100];
	char CenterIP[100];
	u8 RunEnable;//运行状态启用
	u8 AlarmEnable;//报警联动启用
	u8 DailyPicEnable;//日常图像启用
	SBizAlarmPicCFG SAlarmPicParam;//报警图片配置参数
	SBizDailyPicCFG SDailyPicParam;//日常图片配置参数
} SBizSGParam;
#endif

//yaogang modify 20150105
typedef struct 
{
	u8 Enable;	//使能
	u8 Res;		//分辨率
	u8 Quality;	//图片质量
	u8 Interval;	//时间间隔
} SBizSnapPara;
typedef struct 
{
	SBizSnapPara TimeSnap;		//定时抓图
	SBizSnapPara EventSnap;	//事件抓图
} SBizSnapChnPara;

#endif

