#ifndef _BIZ_LOG_H_
#define _BIZ_LOG_H_

#include "biz.h"
#include "mod_log.h"
#include "biz_manager.h"

s32 LogInit(u32 nEnable, SBizLogInitPara* psInitPara);
s16 LogMakeType(s8 mainType, s8 slaveType);
s8 LogGetMaster(s16 type);
s8 LogGetSlave(s16 type);
s32 WriteLog(SBizLogInfo* psLogInfo);
s32 SearchLog(SBizLogSearchPara* psLogSearch, SBizLogResult* psLogResult);

//导出日志文件格式由回调函数决定
s32 RegistLogExportCB(BIZLOGEXPORTCB logExportCb);

//如果psLogResult为空，则导出所有日志
//否则，导出psLogResult列表日志
s32 ExportLog(SBizLogResult* psLogResult, u8* pPath);

void LogExportCB(SBizLogInfoResult* pLog, u8* strLog);

#endif//_BIZ_LOG_H_

