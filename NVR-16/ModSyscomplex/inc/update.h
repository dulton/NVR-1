#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "mod_syscomplex.h"
#include "diskmanage.h"
#include <unistd.h>

#define UPDATE_IGNORE_UBOOT
//#define CHECK_UPDATE
#define CFG_SAVE_IN_FLASH

//#define _MODSYS_DEBUG
//#define _MODSYS_ASSERT_TEST_  // assertion

#define MODSYS_PATH_LEN_MAX	64

#define KB			1000
#define MB			(1000*1000)

#ifndef WIN32
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#define dummy() do{}while(0)

#ifdef _MODSYS_ASSERT_TEST_  // 若使用断言测试
#define  MODSYS_ASSERT( condition )\
do{if (condition) dummy();else modsys_assert( __FILE__, __LINE__ );}while(0)
#else
#define  MODSYS_ASSERT( condition )
#define EXAM_ASSERT(condition)
#endif  /* end of ASSERT */


#define EvNotify(x,y,z)\
	do{if(x){(*x)(y,z);}else{printf("\nEvent Notifier Fail!\n");}}while(0)\

//** local functions
#ifdef _MODSYS_DEBUG
#define MODSYS_DEBUG(x)\
	do { printf("\n[MODSYS]%s %d %s %d\n", __FILE__, __LINE__, __FUNCTION__, (int)x);} while(0)

#define MODSYS_DEBUG_STR(s)\
	do { printf("\n[MODSYS]%s %d %s %s\n", __FILE__, __LINE__, __FUNCTION__, s);} while(0)
#else
#define MODSYS_DEBUG(x)
#define MODSYS_DEBUG_STR(s)
// empty
#endif

// MOD_SYSCOMPLEX module structure
//
typedef struct _SModSyscplx
{
	BOOL bInit;
	BOOL bBreakBackup;
	BOOL bSetTime;
	
	BOOL bRefreshSata;
	BOOL bUpdate;
	
	// mutex for cmd struct
	pthread_mutex_t mtxBreakBackup;
	//pthread_mutex_t modsys_mutex;
	
	// cmdhdr
	SMsgCmdHdr pMsgCmd;
	
	PFNSysNotifyCB pFNSysNotifyCB;// 系统杂项事件通知回调函数
	PFNRemoteUpdateProgressCB pRemoteUpdateNotifyCB;
	
	PUpdateTimeCB  pUpdateTimeCB; // update time by interval
	
	PFNSysResetCB  pSysResetCB;
	PFNSysResetCB  pStopRecCB, pResumeRecCB;
	PFNEnablePanel pDisablePanelCB;
	PFNGetUpdateSize	pGetUpdateSizeCB;
	
	// datetime fmt
	EMDATETIMEFORMAT eTimeFmt;
	EMTIMEFORMAT eTmFormat;
	
	// storage dev handle
	disk_manager *pDiskManager;
	
	s8 path[MODSYS_PATH_LEN_MAX];
	
	BOOL bFormatCheck;
	BOOL bBackupCheck;
	BOOL bFormating;
	BOOL bBackuping;
	
	//csp modify 20131213
	u8 nTimeZone;
	//yg modify
	u8 nFormatdiskIdx;
} SModSyscplx, *PSModSyscplx;

// extern functions
extern void modsysCmplx_SetProductModel(char* updatemodel);
extern void modsys_assert( char * file_name, u32 line_no );
extern s32 modSysCmplx_Update_USB_Do( EMUPGRADETARGET emUpdateTarget, char* pPath, void* extra);
extern s32 get_usb_device( char * usb_device );
extern void modsysCmplx_SetBreakBackup( int type );
extern void modsysCmplx_ClrBreakBackup( void );
extern BOOL modsysCmplx_QueryBreakBackup( void );
extern int umount_user(char *user_path);
extern int modsys_MountUser(char *mounted_path,char *user_path);
s32 modSysCmplx_DmInfoNotify(
	SDMInfo *psInfo,
	s32 	evt,
	s32 	emRslt
);
s32 modSysCmplx_StorgeNotify(
	SStoreDevManage *psMgr,
	s32 	evt,
	s32 	emRslt
);

#endif // __UPDATE_H__

