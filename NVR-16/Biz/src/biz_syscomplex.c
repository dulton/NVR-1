#include "biz_syscomplex.h"
#include "biz_encode.h"
#include "biz_manager.h"
#include <sys/reboot.h>
#include <biz_net.h>
#include "biz_record.h"
#include "biz_alarm.h"

void SysComplexEventDeal(EMSYSEVENT emSysEvent, SEventPara *psEventPara);
void SysComplexDataTimeTick(char* pstrDateTime);
void sysComplex_resetcb();
void SysComplexInstallUpdateProgNotify(PFNRemoteUpdateProgressCB pCB);

void sysComplexInstallStopRecCB( void );

void sysComplexStopRecCB( void );
void sysCompleResumeRecCB( void );

extern s32 ConfigSyncFileToFlash(u8 nMode);
extern void GetJpgSize(char* pJpg, int* width, int* height);
extern void FrontBoardEnable( int bEn );

s32 SysComplexInit(u8 nEnable, SBizSysComplex* psInitPara)
{
	if(nEnable && psInitPara)
	{
		s32 ret = 0;
		
		SBizSystemPara sysPara;
		memset(&sysPara, 0, sizeof(SBizSystemPara));
		ConfigGetSystemPara(0, &sysPara);
		
		SDateTime psDateTime;
		memset(&psDateTime, 0, sizeof(psDateTime));
        
		SModSysCmplxInit sModSysCmplx;
		sModSysCmplx.pDMHdr 		= psInitPara->hddHdr;
		sModSysCmplx.pSysNotifyCB 	= SysComplexEventDeal;
		sModSysCmplx.pUpTimeCB 		= SysComplexDataTimeTick;
		sModSysCmplx.pSysResetCB 	= sysComplex_resetcb;
		sModSysCmplx.pStopRecCB		= sysComplexStopRecCB;
		sModSysCmplx.pResumeRecCB	= sysCompleResumeRecCB;        
		sModSysCmplx.pDisablePanelCB = FrontBoardEnable;
		sModSysCmplx.pGetLogoSizeCB	= GetJpgSize;
		sModSysCmplx.nTimeZone = sysPara.nTimeZone;//csp modify 20131213
		
		ret = ModSysComplexInit(&sModSysCmplx);
		if(0 != ret)
		{
			printf("%s %s %d ret=%d\n", __FILE__, __FUNCTION__, __LINE__, ret);
			return -1;
		}
		
		ModSysComplexDTGet(&psDateTime);
		//printf("****************format: %d\n", psDateTime.emDateTimeFormat);
		psDateTime.emDateTimeFormat = (EMDATETIMEFORMAT)sysPara.nDateFormat;
		psDateTime.emTimeFormat = (EMTIMEFORMAT)sysPara.nTimeStandard;
		//printf("****************DateTimeFormat: %d\n", psDateTime.emDateTimeFormat);
		ModSysComplexDTSet(&psDateTime);
		
		SysComplexInstallUpdateProgNotify(BizNetUpdateProgressNoitfyCB);
	}
	
	return 0;
}

void SysComplexEventDeal(EMSYSEVENT emSysEvent, SEventPara *psEventPara)
{
    if (psEventPara)
    {
        SBizEventPara sBizEventPara;
        
        switch (emSysEvent)
        {
        	case EM_SYSEVENT_GETDMINFO:
                sBizEventPara.emType = EM_BIZ_EVENT_GETDMINFO;
                sBizEventPara.sDmInfo.nType = psEventPara->sDmInfo.nType;
                sBizEventPara.sDmInfo.nTotal = psEventPara->sDmInfo.nTotal;
                sBizEventPara.sDmInfo.nFree = psEventPara->sDmInfo.nFree;
                strcpy( sBizEventPara.sDmInfo.szMountPoint, psEventPara->sDmInfo.szMountPoint);
                break;  
		case EM_SYSEVENT_SATARELOAD:
		case EM_SYSEVENT_DISKCHANGED:
			sBizEventPara.emType = (emSysEvent == EM_SYSEVENT_SATARELOAD) ? EM_BIZ_EVENT_SATARELOAD : EM_BIZ_EVENT_DISKCHANGED;
			sBizEventPara.sStoreMgr.nDiskNum = psEventPara->sStoreMgr.nDiskNum;
			sBizEventPara.sStoreMgr.psDevList = psEventPara->sStoreMgr.psDevList;
			break;
            case EM_SYSEVENT_LOCK://see 2.3 nDelay
                sBizEventPara.emType = EM_BIZ_EVENT_LOCK;
                sBizEventPara.nDelay = psEventPara->nDelay;
                break;
            case EM_SYSEVENT_RESTART:		//see 2.3 nDelay
                sBizEventPara.emType = EM_BIZ_EVENT_RESTART;
                sBizEventPara.nDelay = psEventPara->nDelay;
                break;	        
            case EM_SYSEVENT_POWEROFF:		//see 2.3 nDelay
                sBizEventPara.emType = EM_BIZ_EVENT_POWEROFF;
                sBizEventPara.nDelay = psEventPara->nDelay;
                break;	     
            case EM_SYSEVENT_POWEROFF_MANUAL:		//no parameter by this event
                sBizEventPara.emType = EM_BIZ_EVENT_POWEROFF_MANUAL;
                break;
            case EM_SYSEVENT_UPGRADE_INIT:		//no parameter by this event
                sBizEventPara.emType = EM_BIZ_EVENT_UPGRADE_INIT;
                break;
            case EM_SYSEVENT_REMOTEUP_START:		//cw_remote
                sBizEventPara.emType = EM_BIZ_EVENT_REMOTEUP_START;
                break;
            case EM_SYSEVENT_UPGRADE_RUN:		//see SProgress
                sBizEventPara.emType = EM_BIZ_EVENT_UPGRADE_RUN;
                sBizEventPara.sBizProgress.lCurSize = psEventPara->sProgress.lCurSize;
                sBizEventPara.sBizProgress.lTotalSize = psEventPara->sProgress.lTotalSize;
                sBizEventPara.sBizProgress.nProgress = psEventPara->sProgress.nProgress;
                break;	
            case EM_SYSEVENT_UPGRADE_DONE:		//see EMRESULT
                sBizEventPara.emType = EM_BIZ_EVENT_UPGRADE_DONE;
                sBizEventPara.emBizResult = (EMBIZRESULT)psEventPara->emResult;
                break;
            case EM_SYSEVENT_BACKUP_INIT:		//no parameter by this event
               sBizEventPara.emType = EM_BIZ_EVENT_BACKUP_INIT;
                break;
            case EM_SYSEVENT_BACKUP_RUN:		//see SProgress
                sBizEventPara.emType = EM_BIZ_EVENT_BACKUP_RUN;
                sBizEventPara.sBizProgress.lCurSize = psEventPara->sProgress.lCurSize;
                sBizEventPara.sBizProgress.lTotalSize = psEventPara->sProgress.lTotalSize;
                sBizEventPara.sBizProgress.nProgress = psEventPara->sProgress.nProgress;
                break;
            case EM_SYSEVENT_BACKUP_DONE:		//see EMRESULT
                sBizEventPara.emType = EM_BIZ_EVENT_BACKUP_DONE;
                sBizEventPara.emBizResult = (EMBIZRESULT)psEventPara->emResult;
                break;
            case EM_SYSEVENT_FORMAT_INIT:		//no parameter by this event
                sBizEventPara.emType = EM_BIZ_EVENT_FORMAT_INIT;
                break;
            case EM_SYSEVENT_FORMAT_RUN:		//see SProgress
                sBizEventPara.emType = EM_BIZ_EVENT_FORMAT_RUN;
                sBizEventPara.sBizProgress.lCurSize = psEventPara->sProgress.lCurSize;
                sBizEventPara.sBizProgress.lTotalSize = psEventPara->sProgress.lTotalSize;
                sBizEventPara.sBizProgress.nProgress = psEventPara->sProgress.nProgress;
                break;
            case EM_SYSEVENT_FORMAT_DONE:		//see EMRESULT
                sBizEventPara.emType = EM_BIZ_EVENT_FORMAT_DONE;
                sBizEventPara.emBizResult = (EMBIZRESULT)psEventPara->emResult;
                break;
	     case EM_SYSEVENT_DATETIME_STR:
            sBizEventPara.emType = EM_BIZ_EVENT_TIMETICK;
			sBizEventPara.pstrDateTime = psEventPara->szTime;
			EncodeTimeOsdSetAllChn(sBizEventPara.pstrDateTime);
		 	break;
	     case EM_SYSEVENT_DATETIME_YMD:
            sBizEventPara.emType = EM_BIZ_EVENT_DATETIME_YMD;
			sBizEventPara.sBizDateTime.emDateTimeFormat = (EMBIZDATETIMEFORMAT)psEventPara->sDateTime.emDateTimeFormat;
			sBizEventPara.sBizDateTime.nYear = psEventPara->sDateTime.nYear;
			sBizEventPara.sBizDateTime.nMonth = psEventPara->sDateTime.nMonth;
			sBizEventPara.sBizDateTime.nDay = psEventPara->sDateTime.nDay;
			sBizEventPara.sBizDateTime.nHour = psEventPara->sDateTime.nHour;
			sBizEventPara.sBizDateTime.nMinute = psEventPara->sDateTime.nMinute;
			sBizEventPara.sBizDateTime.nSecond = psEventPara->sDateTime.nSecode;
			sBizEventPara.sBizDateTime.nWday = psEventPara->sDateTime.nWday;
		 	break;
		default:
            return;
        }
        
        SendBizEvent(&sBizEventPara);
		
		SysMainTain();//
    }
}

//系统维护(定时重启)
static u8 IsUpgrade = 0;

static int TimeZoneOffset[] = 
{
	-12*3600,
	-11*3600,
	-10*3600,
	-9*3600,
	-8*3600,
	-7*3600,
	-6*3600,
	-5*3600,
	-4*3600-1800,
	-4*3600,
	-3*3600-1800,
	-3*3600,
	-2*3600,
	-1*3600,
	0,
	1*3600,
	2*3600,
	3*3600,
	3*3600+1800,
	4*3600,
	4*3600+1800,
	5*3600,
	5*3600+1800,
	5*3600+2700,
	6*3600,
	6*3600+1800,
	7*3600,
	8*3600,
	9*3600,
	9*3600+1800,
	10*3600,
	11*3600,
	12*3600,
	13*3600,
};

static int GetTZOffset(int index)
{
	if(index < 0 || index >= (int)(sizeof(TimeZoneOffset)/sizeof(TimeZoneOffset[0])))
	{
		return 0;
	}
	
	return TimeZoneOffset[index];
}
//系统维护
s32 SysMainTain()
{
	int ret = 0;
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMMAINTAIN;
	
	SBizMainTainPara bizPreCfg;
	BizGetPara(&bizTar, &bizPreCfg);
	
	time_t cur_time;
	cur_time = time(NULL);
	
	//csp modify 20131213
	SBizSystemPara sysPara;
	memset(&sysPara, 0, sizeof(SBizSystemPara));
	ConfigGetSystemPara(0, &sysPara);
	int nTimeZone = sysPara.nTimeZone;
	cur_time += GetTZOffset(nTimeZone);
	
	//csp modify
	//struct tm *cpm_now = localtime(&cur_time);
	struct tm tm0;
	struct tm *cpm_now = &tm0;
	localtime_r(&cur_time, cpm_now);
	
	if(bizPreCfg.nEnable)
	{
		switch(bizPreCfg.nMainType)
		{
			case 0:
			{
				ret = 1;
			}
			break;
			case 1:
			{
				if(cpm_now->tm_wday == bizPreCfg.nSubType)
				{
					ret = 1;
				}
			}
			break;
			case 2:
			{
				if(cpm_now->tm_mday == bizPreCfg.nSubType + 1)
				{
					ret = 1;
				}
			}
			break;
			default:
			break;
		}
		
		long int time_tmp = cpm_now->tm_hour * 3600 + cpm_now->tm_min * 60 + cpm_now->tm_sec;
		long int time_maintain = 0;
		
		if(0 == bizPreCfg.ntime)
		{
			time_maintain = 23 * 3600 + 59 * 60 + 60;
		}
		else
		{
			time_maintain = bizPreCfg.ntime * 3600;
		}
		
		if((1 == ret) && (1 == IsUpgrade) && (time_maintain - 120 < time_tmp))
		{
			ret = 0;
		}
		
		//printf("nHour = %d,ntime = %d\n",cpm_now->tm_hour,bizPreCfg.ntime);
		if(cpm_now->tm_hour == bizPreCfg.ntime
			&& cpm_now->tm_min == 0
			&& cpm_now->tm_sec < 2
			&& ret == 1)
		{
			BizRecordStopAll();
			
			usleep(2*1000*1000);
			
			PreviewMute(1);//重启后声音会一直存在，包括在uboot和内核阶段
			BizAlarmWorkingEnable(0);//禁用报警模块//yzw
			BizStopPatrol();//停止轮巡
			
			BizWriteLog_2(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_REBOOT);
			
			reboot(RB_AUTOBOOT);
		}
	}
	
	return 0;
}

s32 SysComplexExit(EMBIZSYSEXITTYPE emExitType)
{
    //getconfigset
    u8 nDelay = 1;
    
    EMSYSEXITTYPE emSysExitType = (EMSYSEXITTYPE)emExitType;
    return ModSysComplexExit(emSysExitType, nDelay);
}

s32 SysComplexUpgrade(SBizUpgrade* psBizUpgrade)
{
    if (NULL == psBizUpgrade)
    {
        return -1;
    }
    
    EMUPGRADETARGET emUpdateTarget = (EMUPGRADETARGET)psBizUpgrade->emUpdateTarget;
    EMUPGRADEWAY emUpgradeWay = EM_UPGRADE_USB;
    char* pcUpdateFName = psBizUpgrade->pcUpdateFName;
    
    sysComplexStopRecCB(); // 本地升级之前先停止录像，杂项模块内部会根据升级是否成功恢复录像
    sleep(5);
    
    return ModSysComplexUpgrade(emUpdateTarget, emUpgradeWay, pcUpdateFName);
}

s32 SysComplexUpgradeFilelist(SBizUpgradeFileInfo* spFileInfo)
{
	if (NULL == spFileInfo)
    {
        return -1;
    }

	EMUPGRADETARGET emUpdateTarget = (EMUPGRADETARGET)spFileInfo->emUpdateTarget;
	SUpdateFileInfoList* spFilelist = (SUpdateFileInfoList*)spFileInfo->spFileList;

	ModSysComplexGetUpdateFileInfo(emUpdateTarget, spFilelist);

	return 0;
}

s32 SysComplexConfigFilelist(SBizConfigFileList* spFileInfo)
{
	if (NULL == spFileInfo)
    {
        return -1;
    }
					
	//EMUPGRADETARGET emUpdateTarget = (EMUPGRADETARGET)spFileInfo->emUpdateTarget;
	SUpdateFileInfoList* spFilelist = (SUpdateFileInfoList*)spFileInfo->spFileList;

	ModSysComplexGetConfigFileInfo(spFileInfo->szDir, spFilelist);

	return 0;
}

u8 SysGetInsertDiskStatue()
{
	return ModSysGetInsertDiskStatue();
}

void SysComplexSetProductModel(char* updatemodel)
{
	ModSysComplexSetProductModel(updatemodel);
}

s32 SysComplexMountUsbDirlist(SBizUpdateFileInfoList* spFileInfo)
{
	if (NULL == spFileInfo)
	{
		printf("NULL fsinfo!\n");
	    	return -1;
	}

	//SBizUpdateFileInfoList* spFilelist = spFileInfo->spFileList;
	
	SUsbDirList sDirList;
	memset(&sDirList, 0, sizeof(sDirList));

	if(spFileInfo->nMax>0)
	{
		spFileInfo->nFileNum = 0;
		
		if( 0==ModSysComplexGetUsbDirInfo(&sDirList) )
		{
			if(strlen(sDirList.sInfo[0].szDir)>0)
			{
				spFileInfo->nFileNum = 1;
				strcpy(spFileInfo->pInfo[0].szName, sDirList.sInfo[0].szDir);
			}
		}
	}
	
	printf("@@@@@@ fileNum = %d 2\n",spFileInfo->nFileNum);

	return 0;
}

s32 SysComplexBackup(SBizBackup* pSPara)
{
    if (NULL == pSPara)
    {
        return -1;
    }
	
    EMFILETYPE emFileType = (EMFILETYPE)pSPara->emFileType; 
    PSBackTgtAtt psBackTgtAtt = (PSBackTgtAtt)pSPara->psBackTgtAtt; 
    SSearchCondition* psSearchCondition = (SSearchCondition *)pSPara->psSearchCondition; 
    
    return ModSysComplexBackup(emFileType, psBackTgtAtt, psSearchCondition);
}

s32 SysComplexBreakBackup(int type)
{
    return ModSysComplexBreakBackup(type);
}

s32 SysComplexDiskManage(SBizDiskManage* pSPara)
{
    if (NULL == pSPara)
    {
        return -1;
    }
	
    SStoreDevManage* psStoreDevManage = (SStoreDevManage *)pSPara->psStoreDevManage;
    u8 nMaxDiskNum = pSPara->nMaxDiskNum;
    
    return ModSysComplexDMGetInfo(psStoreDevManage, nMaxDiskNum);
}

s32 SysComplexDiskFormat(SBizDiskFormat* pSPara)
{
    if (NULL == pSPara)
    {
        return -1;
    }

    char *pstrDevPath = pSPara->pstrDevPath;
	//printf("SysComplexDiskFormat:%s\n", pstrDevPath);
    EMFORMATWAY emFormatWay = (EMFORMATWAY)pSPara->emFormatWay;
    
    return ModSysComplexDMFormat(pstrDevPath, emFormatWay);
}

s32 SysComplexDiskSearch(SBizDiskSearch* pSPara)
{
	if (NULL == pSPara)
	{
		return -1;
	}

	EMFILETYPE emFileType = (EMFILETYPE)pSPara->emFileType;
	SSearchPara* psSearchPara = (SSearchPara *)pSPara->psSearchPara;
	SSearchResult* psSearchResult = (SSearchResult *)pSPara->psSearchResult;
	u16 nMaxFileNum = pSPara->nMaxFileNum;
    
	printf("emFileType %d\n", emFileType);	//EM_BIZFILE_SNAP
	printf("mask type %d\n", psSearchPara->nMaskType);
	printf("nMaskChn %d\n", psSearchPara->nMaskChn);
	printf("nStartTime %u\n", psSearchPara->nStartTime);
	printf("nEndTime %u\n", psSearchPara->nEndTime);
	
	return ModSysComplexDMSearch(emFileType, psSearchPara, psSearchResult, nMaxFileNum);
}

s32 SysComplexDTSet(const SBizDateTime* pSPara)
{
    if (NULL == pSPara)
    {
        return -1;
    }
	
	//printf("***TimeFormat: %d\n", psDateTime->emDateTimeFormat);
	
	return ModSysComplexDTSet((SDateTime*)pSPara);
}

s32 SysComplexSetTimeZone(int nTimeZone)
{
	return ModSysComplexSetTimeZone(nTimeZone);
}

s32 SysComplexDTGet(SBizDateTime* pSPara)
{
	if(NULL == pSPara)
	{
		return -1;
	}
	
	SDateTime *psDateTime = (SDateTime*)pSPara;
	
	return ModSysComplexDTGet(psDateTime);
}

s32 SysComplexDTGet_TZ(SBizDateTime* pSPara)
{
	if(NULL == pSPara)
	{
		return -1;
	}
	
	SDateTime *psDateTime = (SDateTime*)pSPara;
	
	return ModSysComplexDTGet_TZ(psDateTime);
}

s32 SysComplexDTGetStr(char* pSPara)
{
	if(NULL == pSPara)
	{
		return -1;
	}
	
	char *pstrDateTime = pSPara;
	
	return ModSysComplexDTGetString(pstrDateTime);
}

void SysComplexDataTimeTick(char* pstrDateTime)
{
#if 0
    if (pstrDateTime)
    {
        SBizEventPara sBizEventPara;
 		
        sBizEventPara.emType = EM_BIZ_EVENT_TIMETICK;
        sBizEventPara.pstrDateTime = pstrDateTime;
        
        SendBizEvent(&sBizEventPara);
    }
#endif
}

void sysComplex_resetcb()
{
	printf("***%s\n",__func__);
	ConfigSyncFileToFlash(0);	
	//system(reboot); //9624调用下行代码无效
	FrontBoardEnable(0);
	rebootSlaveByCmd();//9616需要同时重启从片
	reboot(RB_AUTOBOOT);
}

void SysComplexInstallUpdateProgNotify(PFNRemoteUpdateProgressCB pCB)
{
	ModSysComplexRegNetUpdateProgCB(pCB);
}

void sysComplexStopRecCB( void )
{
	IsUpgrade = 1;
	//yaogang 20150715
	BizSnapPause();
	RecordStopAll();
	//yaogang modify 20150407
	RecordSnapPause(2);//0: format 1: hotplugup 2:update
	AlarmWorkingEnable(0);// byspliang, 关闭所有报警避免升级过程中因报警写日志导致升级失败
	sleep(5);// 等5秒保证录像停止
	//RecordDeInit();//cw_9508S
	DefaultConfigRelease();
	RecordBufRelease();
}

void sysCompleResumeRecCB( void )
{
	int i;
	SBizDvrInfo sInfo;
	
	if( 0 == ConfigGetDvrInfo(0, &sInfo) )
	{
		RecordBufResume();
		//yaogang modify 20150407
		RecordSnapResume(2);//0: format 1: hotplugup 2:update
		for(i=0; i<sInfo.nRecNum; i++)
		{
			RecordResume(i);
		}
	}
	//yaogang 20150715
	BizSnapResume();
	DefaultConfigResume();
	AlarmWorkingEnable(1);//byspliang, 升级失败后需要恢复报警
	
	IsUpgrade = 0;
}

