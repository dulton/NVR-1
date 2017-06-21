#include <semaphore.h>
#include "biz_log.h"

static u8 g_NoHardDisk = 0;

s32 LogInit(u32 nEnable, SBizLogInitPara* psInitPara)
{
	if(nEnable)
	{
		if(NULL == psInitPara)
		{
			return -1;
		}
		
		SLogInitPara para;
		memset(&para, 0, sizeof(para));
		para.nMaxLogs = psInitPara->nMaxLogs;
		para.pHddMgr = psInitPara->pHddMgr;
		
		int ret = ModLogInit(&para);
		if(ret == -2)
		{
			g_NoHardDisk = 1;
			ret = 0;
		}
		
		if(0!=RegistLogExportCB(LogExportCB))
		{
			printf("RegistLogExportCB error!\n");
		}
		
		return ret;
	}
	
	return 0;
}

s16 LogMakeType(s8 mainType, s8 slaveType)
{
	return g_NoHardDisk ? 0 : ModMakeType(mainType, slaveType);
}

s8 LogGetMaster(s16 type)
{
	return g_NoHardDisk ? 0 : ModGetMaster(type);
}

s8 LogGetSlave(s16 type)
{
	return g_NoHardDisk ? 0 : ModGetSlave(type);
}

s32 WriteLog(SBizLogInfo* psLogInfo)
{
	if(g_NoHardDisk)
	{
		return 0;
	}
	if(NULL == psLogInfo)
	{
		return -1;
	}
	
	SLogInfo info;
	memset(&info, 0, sizeof(info));
	info.magic1= psLogInfo->nchn;//报警源的序号或通道
	info.nTime = time(NULL);
	strcpy(info.aIp, psLogInfo->aIp);
	strcpy(info.aUsername, psLogInfo->aUsername);
	info.nType = ModMakeType(psLogInfo->nMasterType, psLogInfo->nSlaveType);
	return ModWriteLog(&info);
}

s32 SearchLog(SBizLogSearchPara* psLogSearch, SBizLogResult* psLogResult)
{
	if(g_NoHardDisk)
	{
		return 0;
	}
	if((NULL == psLogSearch) || (NULL == psLogResult))
	{
		return -1;
	}

	SLogSearchPara search;
	memset(&search, 0, sizeof(search));
	SLogResult result;
	memset(&result, 0, sizeof(result));

	search.nMode = psLogSearch->nMode;
	search.nMasterMask = psLogSearch->nMasterMask;
	search.nStart = psLogSearch->nStart;
	search.nEnd = psLogSearch->nEnd;
/*
	result.nRealNum = psLogResult->nRealNum;
	result.nMaxLogs = psLogResult->nMaxLogs;
	result.psLogList = psLogResult->psLogList;

	return ModSearchLog(&search,&result);
	*/
	return ModSearchLog(&search,(SLogResult*)psLogResult);
}

s32 RegistLogExportCB(BIZLOGEXPORTCB logExportCb)
{	
	return ModRegistLogExportCB((LOGEXPORTCB) logExportCb);
}

s32 ExportLog(SBizLogResult* psLogResult, u8* pPath)
{
	return ModExportLog((SLogResult *)psLogResult, pPath);
}

static char* g_LogInfo[7][50] = {
	{	"Open channel polling                     ",
		"Stop channel polling                     ",
		"Set mute                                 ",
		"Adjust volume                            ",
		"PTZ control                              ",
		"Snap picture                             ",
		"Start manual record                      ",
		"Stop manual record                       ",
		"Manual alarm                             ",
		"Clear alarm                              ",
		"Motion detected                          ",
		"Motion stop                              ",
		"Power off                                ",
		"Reboot                                   ",
		"Login                                    ",
		"Logout                                   ",
		"Format hard disk successfully!           ",
		"Failed to format hard disk!              ",
		"System upgrade                           ",
	},
	{	"Enable password checking                 ",
		"Disable password checking                ",
		"Set the video output device              ",
		"Set the maximum amount of network users  ",
		"Set video format                         ",
		"Set the system language                  ",
		"Set time zone                            ",
		"Enable automatic time synchronization    ",
		"Disable automatic time synchronization   ",
		"Set network time server                  ",
		"Set system time                          ",
		"Set channel names                        ",
		"Set the record channel                   ",
		"Enable or disable audio recording        ",
		"Set the video stream                     ",
		"Set record time before alarm             ",
		"Set record time after alarm              ",
		"Set expiration time of record files      ",
		"Set the video OSD                        ",
		"Set the timer recording schedule         ",
		"Set the motion recording schedule        ",
		"Set the sensor recording schedule        ",
		"Enable or disable the sensor             ",
		"Set sensor type                          ",
		"Set sensor deal                          ",
		"Set the sensor alarm schedule            ",
		"Enable or disable video motion detection ",
		"Set motion detection area                ",
		"Set motion detection deal                ",
		"                                         ",
		"Set motion detection schedule            ",
		"Set video lost deal                      ",
		"Set alarmout schedule                    ",
		"Enable buzz                              ",
		"Disable buzz                             ",
		"Set IP                                   ",
		"Set http port                            ",
		"Set server port                          ",
		"Set net stream                           ",
		"Set the sender mailbox                   ",
		"Set the recipient's mailbox              ",
		"Set DNS                                  ",
		"Add user                                 ",
		"Delete user                              ",
		"Set user's authority                     ",
		"Set user's password                      ",
		"Set serial parameters                    ",
		"Set the cruise line                      ",
		"Restore factory settings                 ",
	},
	{	"Stop play                                ",
		"Pause play                               ",
		"Resume play                              ",
		"Backward(playing)                        ",
		"Forward(playing)                         ",
		"Start play	   	                          ",
	},
	{	"Start backup                             ",
		"Stop backup                              ",
	},
	{	"Search data                              ",
		"Search record files                      ",
	},
	{	"View Information                         ",
	},
	{
		"Exception error                          ",
		"Password error                           ",
		"Hard disk error                          ",
		"Encoding error                           ",
		"Video file error                         ",		
	},										
};

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

void LogExportCB(SBizLogInfoResult* pLog, u8* strLog)
{
	if((!pLog) || (!strLog))
	{
		return;
	}
	
	u8 ip[32] = {0};
	u8 username[48] = {0};
	u8 type[100] = {0};
	u8 stTime[64] = {0};
	
#if 1
	char* tmp = "**************************************************";
	sprintf(type,"Type:  %s\n*",((pLog->nType >> 8) != 6) ? g_LogInfo[(pLog->nType >> 8) - 1][(pLog->nType & 0xff) - 1] : g_LogInfo[(pLog->nType >> 8) - 1][0]);
	sprintf(ip,"Ip  :  %s\n",pLog->aIp);
	sprintf(username,"User:  %s\n*",pLog->aUsername);
	
	time_t time = pLog->nTime;
	
	//csp modify 20131213
	SBizSystemPara sysPara;
	memset(&sysPara, 0, sizeof(SBizSystemPara));
	ConfigGetSystemPara(0, &sysPara);
	int nTimeZone = sysPara.nTimeZone;
	time += GetTZOffset(nTimeZone);
	
	struct tm sTime;
	//csp modify
	//sTime = *localtime(&time);
	localtime_r(&time,&sTime);
	
	sprintf(stTime,"Time:  %s",asctime(&sTime));
	stTime[strlen(stTime)-1] = '\n';
	stTime[strlen(stTime)] = '*';
	stTime[strlen(stTime)+1] = '\0';
	
	sprintf(strLog,"%s\n*%s%s%s%s%s\n", tmp, stTime, type, username, ip, tmp);
#else
	sprintf(type,"Type:[0x%04x]\t",pLog->nType);
	sprintf(ip,"Ip:%s\n",pLog->aIp);
	sprintf(username,"User:%-15s",pLog->aUsername);
	
	time_t time = pLog->nTime;
	
	struct tm sTime;
	//csp modify
	//sTime = *localtime(&time);
	localtime_r(&time,&sTime);
	
	sprintf(stTime,"Time:[%s]",asctime(&sTime));
	stTime[strlen(stTime)-2] = ']';
	stTime[strlen(stTime)-1] = '\t';
	stTime[strlen(stTime)] = '\0';
	
	sprintf(strLog,"%s%s%s%s",type,username,stTime,ip);
#endif	
}

