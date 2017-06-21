#ifndef __FORMAT_H_
#define __FORMAT_H_

#include "common_basetypes.h"
#include "mod_syscomplex.h"


#ifdef __cplusplus
extern "C" {
#endif

//csp modify 20121018
//#define STR_DEV_MAX	64
#define STR_DEV_MAX		16

// #define
#define HISI_3515

typedef struct _sStoreDevMgr
{
	u8				 nMax;
	SStoreDevManage* pDevMgr;
} SStoreDevMgr, *PSStoreDevMgr;

// typdef
typedef struct _SFormat
{
	char* strDevPath;
	EMFORMATWAY emFormatWay;
} SFormat, *PSFormat;

// extern ..
extern s32 modSysCmplx_DMFormat(char *pstrDevPath, EMFORMATWAY emFormatWay);
extern s32 modSysCmplx_GetDMInfo( SStoreDevMgr* pStoreDev );
extern s32 modSysComplex_GetUpdateFileInfo(EMUPGRADETARGET emTgt, SUpdateFileInfoList* pInfo);
extern int modsys_MountUser(char *mounted_path, char *user_path);
extern s32 modSysComplex_GetUsbDevDir(SUsbDirList* pInfo);
s32 modSysComplex_GetConfigFileInfo(char* szDir, SUpdateFileInfoList* pLstInfo);
u8 modsys_GetInsertDiskStatue();

#ifdef __cplusplus
}
#endif


#endif // __FORMAT_H_

