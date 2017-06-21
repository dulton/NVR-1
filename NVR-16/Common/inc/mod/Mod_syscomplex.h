#ifndef __MOD_SYSCOMPLEX_H__
#define __MOD_SYSCOMPLEX_H__

#include "public.h"
#include "common_msgcmd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_DIR_LEN_MAX		64
#define USB_DIR_NUM_MAX		4
#define	DEV_PATH_LEN_MAX	15
#define	UPDATE_FILE_NAME_MAX	64
#define	UPDATE_FILE_EXT_MAX		6
#define SYS_TIME_LEN_MAX	64


// 定时提供更新时间回调类型
// 时间格式遵循当前杂项内部维持的格式设置
typedef void (*PUpdateTimeCB)(char* pstrDateTime);

// Function return errcode
typedef enum
{
	EM_MODSYS_SUCCESS = 0,
	EM_MODSYS_FAILED,
	EM_MODSYSINIT_SUCCESS, 							// 模块初始化成功4	
	EM_MODSYSINIT_FAIL,     						// 模块初始化失败
	EM_MODSYSINIT_NULL,    							// 模块回调为空
	EM_MODSYS_THREAD_READ_CMD_ERR ,
	EM_MODSYS_THREAD_READ_CMD_OK ,
	EM_MODSYS_THREAD_USB_UPDATE ,
	EM_MODSYS_THREAD_FILE_PATH_ERR,
	EM_MODSYS_UPDATE_RUNNING,
} EMMODYSERR;

//2.2 系统杂项事件枚举
typedef enum
{
	EM_SYSEVENT_LOCK = 0,		//see 2.3 nDelay
	EM_SYSEVENT_RESTART,		//see 2.3 nDelay
	EM_SYSEVENT_POWEROFF,		//see 2.3 nDelay
	EM_SYSEVENT_POWEROFF_MANUAL,	//no parameter by this event
	EM_SYSEVENT_UPGRADE_INIT,	//no parameter by this event
	EM_SYSEVENT_UPGRADE_RUN,	//see SProgress
	EM_SYSEVENT_REMOTEUP_START,//cw_remote
	EM_SYSEVENT_UPGRADE_DONE,	//see EMRESULT
	EM_SYSEVENT_BACKUP_INIT,	//no parameter by this event
	EM_SYSEVENT_BACKUP_RUN,	//see SProgress
	EM_SYSEVENT_BACKUP_DONE,	//see EMRESULT
	EM_SYSEVENT_FORMAT_INIT,	//no parameter by this event
	EM_SYSEVENT_FORMAT_RUN,	//see SProgress
	EM_SYSEVENT_FORMAT_DONE,	//see EMRESULT
	EM_SYSEVENT_DATETIME_STR,
	EM_SYSEVENT_DATETIME_YMD,
	EM_SYSEVENT_GETDMINFO,
	//EM_SYSEVENT_PCBHDD,//xdc
	EM_SYSEVENT_SATARELOAD,
	EM_SYSEVENT_DISKCHANGED,
} EMSYSEVENT;

// 远程升级进度通知回调
typedef void (*PFNRemoteUpdateProgressCB)(u8 nProgress);

//2.4 进度参数
typedef struct
{
	u64 lTotalSize;
	u64 lCurSize;
	u8 nProgress;	//0~100
} SProgress;

//2.4.5
typedef enum
{
	EM_UPGRADE_USB,
	EM_UPGRADE_NET,
} EMUPGRADEWAY;

// mainboard || panel
typedef enum
{
	EM_UPGRADE_MAINBOARD,
	EM_UPGRADE_PANEL,
	EM_UPGRADE_STARTLOGO,
	EM_UPGRADE_APPLOGO,
} EMUPGRADETARGET;

//2.5 运行结果枚举
typedef enum
{
	EM_UPGRADE_SUCCESS = 1,//升级成功1
	EM_UPGRADE_RUNNING = -198,
	EM_UPGRADE_FAILED_UNKNOW = -199,// 升级失败 -199~-100请自行酌情增加

	EM_BACKUP_SUCCESS = 2,//备份成功2
	EM_BACKUP_FAILED_UNKNOW = -299,// 备份失败 -299~-200请自行酌情增加
	EM_BACKUP_FAILED_NOSPACE = -298,

	EM_FORMAT_SUCCESS = 3,//格式化成功3
	EM_FORMAT_FAILED_UNKNOW = -399,// 格式化失败 -399~-300请自行酌情增加
	
	EM_SEARCH_SUCCESS = 4,//格式化成功3
	EM_SEARCH_FAILED_UNKNOW = -499,// 格式化失败 -399~-300请自行酌情增加	
} EMRESULT;

// 2.6 系统退出类型枚举
typedef enum
{
	EM_SYSEXIT_LOCK,
	EM_SYSEXIT_RESTART,
	EM_SYSEXIT_POWEROFF,
} EMSYSEXITTYPE;

// 2.7 文件类型枚举
typedef enum
{
	EM_FILE_REC = 0,
	EM_FILE_SNAP,
} EMFILETYPE;

// 2.8 文件搜索方式枚举
typedef enum
{
	EM_SEARCH_BY_TIME,
	EM_SEARCH_BY_FILE,
} EMSEARCHWAY;


// 2.10 按时间搜索参数
typedef struct 
{
	u8 nMaskType;
	u32 nMaskChn;
	u32 nStartTime;
	u32 nEndTime;
	void* pReserverInfo;
} SSearchPara;

#if 0
typedef struct
{
	u8   channel_no;//u32  channel_no;//csp modify
	u8   type;
	u32  start_time;
	u32  end_time;
	u8   image_format;//3:cif;4:4cif
	u8   stream_flag;//0:视频流;1:音频流
	u32	 size;
	u32  offset;
	u8   disk_no;
	u8   ptn_no;
	u16  file_no;
}PACK_NO_PADDING recfileinfo_t;
#endif

// 2.17  文件信息参数
typedef struct
{
	u8 nChn;
	u8 nType;
	u32 nStartTime;
	u32 nEndTime;
	u8 nFormat;      //N制式下 10 = D1，13 = CIF  P制式下 0 = D1， 3 = CIF
	u8 nStreamFlag;  //0:视频流;1:音频流
	u32 nSize;
	u32 nOffset;
	u8 nDiskNo;
	u8 nPtnNo;
	u16 nFileNo;
} PACK_NO_PADDING SRecfileInfo;

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
} PACK_NO_PADDING SRecSnapInfo;

// 2.11 文件搜索结果结构体
typedef struct
{
	u16 nFileNum;
	union
	{
		SRecfileInfo *psRecfileInfo;//用于存放数据的文件信息数组
		SRecSnapInfo *psRecSnapInfo;
	};
} PACK_NO_PADDING SSearchResult;

// 2.12 文件格式
typedef enum
{
	EM_FORMAT_ORIGINAL = 0,
	EM_FORMAT_AVI,
	EM_FORMAT_JPEG,
	EM_FORMAT_BMP,
} EMFILEFORMAT;

// backup device type
typedef enum
{
	EM_BACKUP_USB,
	EM_BACKUP_DVDR,
	
} EMBACKUPDEV;

typedef enum
{
	EM_DVDRW_MODE_NEW,
	EM_DVDRW_MODE_ADD,
	
} EMBACKUPDVDRWMODE;

// Backup target attributes
typedef struct _sBackupTargetAttr
{
	EMFILEFORMAT fmt;
	EMBACKUPDEV  devt;
	union
	{
		u8 reserve;
		EMBACKUPDVDRWMODE mode;
	};
} SBackTgtAtt, *PSBackTgtAtt;

// 2.9 搜索条件参数
typedef struct
{
	EMSEARCHWAY emSearchWay;
	union
	{
		SSearchPara sSearchPara;
		SSearchResult sSearchResult;
	};
} SSearchCondition;

// 2.13 录像设备管理句柄
typedef void* DMHDR;

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
} SDevInfo;

// 2.14 获得存储设备管理信息
typedef struct
{
	u8 		  nDiskNum;
	SDevInfo  *psDevList;
} SStoreDevManage;


// 2.16 格式化方式
typedef enum
{
	EM_FORMAT_FAT32_IFV,						//格式化为fat32并初始化录像格式文件
	EM_FORMAT_FAT32,								//格式化为fat32
} EMFORMATWAY;

// 2.20 日期时间格式
typedef enum
{
	EM_DATETIME_YYYYMMDDHHNNSS, //酌情再加
	EM_DATETIME_MMDDYYYYHHNNSS,
	EM_DATETIME_DDMMYYYYHHNNSS,
	EM_DATETIME_IGNORE,
} EMDATETIMEFORMAT;

// 2.20 时间制式
typedef enum
{
	EM_TIME_24HOUR, //酌情再加
	EM_TIME_12HOUR,
} EMTIMEFORMAT;

// 2.19 日期时间
typedef struct
{
	u16 nYear;
	u8 nMonth;
	u8 nDay;
	u8 nHour;
	u8 nMinute;
	u8 nSecode;
	u8 nWday;
	EMDATETIMEFORMAT emDateTimeFormat;
	EMTIMEFORMAT emTimeFormat;
} SDateTime;

typedef struct
{
	u8 nType;
	u32 nTotal;
	u32 nFree;
	char szMountPoint[32];
	//disk_manager* hddindex;//xdc
} SDMInfo;

//2.3 系统杂项事件参数 parameter for EMSYSEVENT
typedef struct
{
	union
	{
		u8				nDelay;//nDelay seconds，support EM_SYSEVENT_LOCK、EM_SYSEVENT_RESTART、EM_SYSEVENT_POWEROFF
 		SProgress		sProgress;//support EM_SYSEVENT_UPGRADE_RUN
		EMRESULT		emResult;//support EM_SYSEVENT_UPGRADE_DONE EM_SYSEVENT_BACKUP_DONE EM_SYSEVENT_FORMAT_DONE
		char			szTime[SYS_TIME_LEN_MAX];
		SDateTime		sDateTime;
		SDMInfo			sDmInfo;
		SStoreDevManage	sStoreMgr;
	};
} SEventPara;

// 升级文件信息结构体
//
typedef struct _sUpdateFileInfo
{
	char szName[UPDATE_FILE_NAME_MAX];
	char szExt[UPDATE_FILE_EXT_MAX];
	u32  nSize;
	u32  nTimeStamp;
} SUpdateFileInfo;

// 升级文件信息列表结构体
//
typedef struct _sUpdateFileInfoList
{
	u8 nMax;
	u8 nFileNum;
	SUpdateFileInfo* pInfo;
} SUpdateFileInfoList;

// usb设备挂接路径
//
typedef struct _sUsbDirInfo 
{
	char szDir[USB_DIR_LEN_MAX];
} SUsbDirInfo;

// usb设备挂接路径列表
typedef struct _sUsbDirList
{
	SUsbDirInfo sInfo[USB_DIR_NUM_MAX]; // null - no usb dir, else - valid usb mounted dir
} SUsbDirList;

//2.1 系统状态回调函数
typedef void (*PFNSysNotifyCB)(EMSYSEVENT emSysEvent, SEventPara *psEventPara);

// reset回调
typedef void (*PFNSysResetCB)(void);

// 获取升级LOGO尺寸
typedef void (*PFNGetUpdateSize)( char* pJpg, int* width, int* height );

// 使能/禁用前面板
typedef void (*PFNEnablePanel)( int bEn );

// 杂项模块初始化参数结构体
typedef struct _sModSysCmplxInit
{
	DMHDR pDMHdr;
	PFNSysNotifyCB pSysNotifyCB;
	PUpdateTimeCB  pUpTimeCB;
	PFNSysResetCB  pSysResetCB;
	PFNSysResetCB  pStopRecCB;
	PFNSysResetCB  pResumeRecCB;
	PFNGetUpdateSize pGetLogoSizeCB;
	PFNEnablePanel pDisablePanelCB;
	
	//csp modify 20131213
	u8 nTimeZone;
} SModSysCmplxInit, *PSModSysCmplxInit;

//1、接口详细定义
/*
	函数功能:模块初始化
	输入参数：
		pSysCmplxInit:
			类型:PSModSysCmplxInit
			含义:系统杂项初始化结构体包括存储设备管理句柄和事件通知回调函数
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexInit( PSModSysCmplxInit pSysCmplxInit );

/*
	函数功能:模块反初始化
	输入参数：无
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDeinit(void);

/*
  需要两个参数：类型枚举（锁定/重起/关闭等）、延时时间(s)（u8 delay）；
	函数功能:系统退出
	输入参数：
		emSysExitType:
			类型:EMSYSEXITTYPE
			含义:退出类型锁定/重起/关闭系统
		nDelay
			类型:u8
			含义:延时时间(s)
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexExit(EMSYSEXITTYPE emSysExitType, u8 nDelay);

/*
	函数功能:系统升级
	输入参数：
		emUpdateTarget:
			类型:EMUPGRADETARGET
			含义:升级目标主板/前面板
		emUpgradeWay:
			类型:EMUPGRADEWAY
			含义:升级途径（本地/通立协议/FTP）
		pcUpdateFName：
			类型:char *
			含义:升级文件路径（本地升级可以为空，其他不能为空）
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexUpgrade(EMUPGRADETARGET emUpdateTarget, EMUPGRADEWAY emUpgradeWay, char *pcUpdateFName);

/*
函数功能:中断文件备份
*/
s32 ModSysComplexBreakBackup(int type);

/*
	函数功能:文件备份
	输入参数：
		emFileType:
			类型:EMFILETYPE
			含义:文件类型
		psBackTgtAtt:
			类型:PSBackTgtAtt
			含义:备份目标参数			
		psSearchCondition：
			类型:SSearchCondition*
			含义:搜索条件
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexBackup( EMFILETYPE 	emFileType, 
						 PSBackTgtAtt	psBackTgtAtt, 
						 SSearchCondition* psSearchCondition );

/*
	函数功能:获得存储设备管理信息，一个参数，设备管理信息结构体（数量及各设备信息）
	输出参数：
		psStoreDevManage:
			类型:SStoreDevManage
			含义:存储设备管理信息
		MaxDiskNum:
			类型:u8
			含义:硬盘最大数
			 
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDMGetInfo(SStoreDevManage *psStoreDevManage, u8 MaxDiskNum);

/*
	函数功能:格式化
	输入参数：
		pstrDevPath:
			类型:char *
			含义:设备路径
		emFormatWay:
			类型:EMFORMATWAY
			含义:格式化格式及方式
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDMFormat(char *pstrDevPath, EMFORMATWAY emFormatWay);

/*
	函数功能:文件搜索
	输入参数：
		emFileType:
			类型:EMFILETYPE
			含义:文件类型
		psSearchPara:
			类型:SSearchPara
			含义:搜索条件
		psSearchResul:
			类型:SSearchResult
			含义:传入用于存放搜索结果结构体
		nMaxFileNum:
			类型:u16
			含义:psRecfileList的最大个数
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDMSearch(EMFILETYPE emFileType, SSearchPara* psSearchPara, SSearchResult* psSearchResult, u16 nMaxFileNum);

/*
	函数功能:设置系统日期时间
	输入参数：
		psDateTime:
			类型:const SDateTime*
			含义:用于设置日期时间
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDTSet(SDateTime* psDateTime);

s32 ModSysComplexSetTimeZone(int nTimeZone);

/*
	函数功能:获得系统日期时间
	输入参数：
		psDateTime:
			类型:SDateTime*
			含义:用于获得日期时间
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDTGet(SDateTime* psDateTime);
s32 ModSysComplexDTGet_TZ(SDateTime* psDateTime);
s32 ModSysComplexConvertToDT(time_t time, SDateTime* psDateTime);


/*
	函数功能:获得格式化系统日期时间字符串
	输入参数：
		pstrDateTime:
			类型:char*
			含义:用于获得日期时间字符串(先按1.0默认YYYY-MM-DD HH:NN:SS)
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDTGetString(char* pstrDateTime);


/*
	函数功能:获取本地USB设备根目录下的升级文件
	输入参数：
		emTgt:
			类型:EMUPGRADETARGET
			含义:指定获取的升级文件类型
		pInfo:
			类型:SUpdateFileInfo*
			含义:升级文件信息
			
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexGetUpdateFileInfo(EMUPGRADETARGET emTgt, SUpdateFileInfoList* pInfo);


/*
	函数功能:获取本地USB设备挂接路径列表
	输入参数：
		pInfo:
			类型:SUsbDirList*
			含义:挂接路径信息
			
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexGetUsbDirInfo(SUsbDirList* pInfo);

/*
	函数功能:获取本地USB设备挂接路径列表
	输入参数：
		szDir:
			类型: char*
			含义: 指定路径供查找配置文件
		pInfo:
			类型:SUpdateFileInfoList*
			含义:配置文件信息
			
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexGetConfigFileInfo(char* szDir, SUpdateFileInfoList* pInfo);

time_t read_rtc(int utc);

// regist remote update progress notify
void ModSysComplexRegNetUpdateProgCB(PFNRemoteUpdateProgressCB pCB);

PFNSysResetCB GetRecStopCB();//cw_9508S

#ifdef __cplusplus
}
#endif

#endif // __MOD_SYSCOMPLEX_H__

