#ifndef _BIZ_SYSCOMPLEX_H_
#define _BIZ_SYSCOMPLEX_H_

#include "biz.h"
#include "mod_syscomplex.h"

//升级参数
typedef struct {
    EMBIZUPGRADETARGET emUpdateTarget;
    char *pcUpdateFName;
} SBizUpgrade;

typedef struct {
	EMBIZUPGRADETARGET emUpdateTarget;
	SBizUpdateFileInfoList* spFileList;
} SBizUpgradeFileInfo;

//备份参数
typedef struct {
    EMBIZFILETYPE emFileType;
    PSBizBackTgtAtt	psBackTgtAtt;
    SBizSearchCondition* psSearchCondition;
} SBizBackup;

//磁盘管理信息
typedef struct {
    SBizStoreDevManage *psStoreDevManage;
    u8 nMaxDiskNum;
} SBizDiskManage;

//磁盘格式化
typedef struct {
    char *pstrDevPath;
    EMBIZFORMATWAY emFormatWay;
} SBizDiskFormat;

//文件搜索
typedef struct {
    EMBIZFILETYPE emFileType;
    SBizSearchPara* psSearchPara;
    SBizSearchResult* psSearchResult;
    u16 nMaxFileNum;
} SBizDiskSearch;

typedef struct
{
    HDDHDR hddHdr;
} SBizSysComplex;

s32 SysComplexInit(u8 nEnable, SBizSysComplex* psInitPara);
s32 SysComplexExit(EMBIZSYSEXITTYPE emExitType);
s32 SysComplexUpgrade(SBizUpgrade* psBizUpgrade);
s32 SysComplexBackup(SBizBackup* pSPara);
s32 SysComplexBreakBackup(int type);
s32 SysComplexDiskManage(SBizDiskManage* pSPara);
s32 SysComplexDiskFormat(SBizDiskFormat* pSPara);
s32 SysComplexDiskSearch(SBizDiskSearch* pSPara);
s32 SysComplexDTSet(const SBizDateTime* pSPara);
s32 SysComplexSetTimeZone(int nTimeZone);
s32 SysComplexDTGet(SBizDateTime* pSPara);
s32 SysComplexDTGet_TZ(SBizDateTime* pSPara);
s32 SysComplexDTGetStr(char* pSPara);

u8 SysGetInsertDiskStatue();
void SysComplexSetProductModel(char* updatemodel);

#endif

