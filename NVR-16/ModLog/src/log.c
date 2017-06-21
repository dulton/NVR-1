#include <semaphore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

#include "custommp4.h"
#include "diskmanage.h"
#include "partitionindex.h"

#include "mod_log.h"
#include "circlebuf.h"

//#include "biz_types.h"

static u8 g_HotPlugFlag = 0;

s32 ModLogSetHotPlugFlag(u8 flag)
{
	g_HotPlugFlag = flag;
	printf("ModLogSetHotPlugFlag:%d\n",flag);
	return 0;
}
/*
extern int tl_power_atx_check();

extern int LogManager_Init(ModLog *pLogManager);
extern int LogManager_Write(ModLog *pLogManager, SBizAlarmInfo* info);
extern int LogManager_Read(ModLog *pLogManager, SAlarmLogSearchPara* psLogSearch, SAlarmLogResult* psLogResult);

extern int DiskLoger_Init(DiskLog *pLogManager, void* pHddMgr);
extern int DiskLoger_Write(DiskLog *pLogManager, SBizAlarmInfo* info);
extern int DiskLoger_Read(DiskLog *pLogManager, SAlarmLogSearchPara* psLogSearch, SAlarmLogResult* psLogResult);
*/
#define MAX_MASTER_TYPE 16

#define PRINT(s) printf("FUNC:%s, LINE:%d, %s", __FUNCTION__, __LINE__, s)

//static ModLog g_flashLog;
//static DiskLog g_diskLog;//csp modify 20140812

//csp modify 20140812
static u8 g_formating = 0;
s32 ModLogSetFormatFlag(u8 flag)
{
	g_formating = flag;
	return 0;
}

//cw_log
typedef struct
{
	u32 emType;
	u32 index;//csp modify
}SLogMsgHeader;

#define EM_INSERT_LOG	1
#define EM_CLEAN_LOG	2//csp modify

#define LOGQUEUE
static SCircleBufInfo g_LogMsgQueueInfo;

s32 LogCreateMsgQueue(void);
s32 LogDestroyMsgQueue(void);
s32 LogWriteMsgQueue(SLogMsgHeader* pHeader);
s32 LogReadMsgQueue(SLogMsgHeader* pHeader);

typedef struct
{
	//sem_t semInsert;//csp modify
	//sem_t semSync;//csp modify
	sem_t semLock;
	u32 nHead;
	u32 nLen;
	u32 nMaxLogs;
	SLogInfo* psLogList;
	void* pHddMgr;
	THREADHANDLE hLogThread;
	u32 nLogID;//cw_log
}SLogMgr;

SLogMgr* g_pLogMgr = NULL;
LOGEXPORTCB g_LogExportCB = NULL;

s16 ModMakeType(s8 mainType, s8 slaveType)
{
	return ((mainType&0xff) << 8) | (slaveType&0xff);
}

s8 ModGetMaster(s16 type)
{
	return (type&0xff00) >> 8;
}

s8 ModGetSlave(s16 type)
{
	return type&0x00ff;
}

u32 GetLogID()//cw_log
{
#if 1//csp modify
	g_pLogMgr->nLogID++;
	return g_pLogMgr->nLogID;
#else
	if(g_pLogMgr->nLogID < 32760)//???
		g_pLogMgr->nLogID++;
	else
		g_pLogMgr->nLogID = (g_pLogMgr->nLogID%g_pLogMgr->nMaxLogs+1);
	return g_pLogMgr->nLogID;
#endif
}

s32 InsertLogList(SLogInfo* pLog)
{
	if(!pLog)
	{
		PRINT("Insert log error\n");
		return -1;
	}
	
	//sem_wait(&g_pLogMgr->semSync);//csp modify
	
	//格式化硬盘，清除日志
	if(ModGetMaster(pLog->nType) == MASTER_SYSCONTROL && ModGetSlave(pLog->nType) == SLAVE_FORMAT_SUCCESS)
	{
		g_pLogMgr->nHead = 0;
		g_pLogMgr->nLen = 0;
		
		//csp modify
		SLogMsgHeader msg;
		msg.emType = EM_CLEAN_LOG;
		msg.index = 0;
		ResetCircleBuf(&g_LogMsgQueueInfo);
		LogWriteMsgQueue(&msg);
	}
	
	u32 head = g_pLogMgr->nHead;
	memcpy(&g_pLogMgr->psLogList[head], pLog, sizeof(SLogInfo));
	
	//printf("log.time = %d \n",g_pLogMgr->psLogList[head].nTime);
	//printf("log.nType = %d \n",g_pLogMgr->psLogList[head].nType);
	//printf("log.aUsername = %s \n",g_pLogMgr->psLogList[head].aUsername);
	
	u32 maxLogs = g_pLogMgr->nMaxLogs;
	if(g_pLogMgr->nLen == maxLogs)
	{
		g_pLogMgr->nHead = (g_pLogMgr->nHead + 1)%maxLogs;
	}
	else
	{
		g_pLogMgr->nHead = (g_pLogMgr->nHead + 1)%maxLogs;
		g_pLogMgr->nLen++;
	}
	
	SLogMsgHeader msg;
	msg.emType = EM_INSERT_LOG;
	msg.index = head;//csp modify
	LogWriteMsgQueue(&msg);
	
	//sem_post(&g_pLogMgr->semSync);//csp modify
	
	return head;
}

//csp modify
s32 InsertLogList2(SLogInfo* pLog)
{
	if(!pLog)
	{
		PRINT("Insert log error\n");
		return -1;
	}
	
	//sem_wait(&g_pLogMgr->semSync);//csp modify
	
	#if 0//csp modify
	//格式化硬盘，清除日志
	if(ModGetMaster(pLog->nType) == MASTER_SYSCONTROL && ModGetSlave(pLog->nType) == SLAVE_FORMAT_SUCCESS)
	{
		g_pLogMgr->nHead = 0;
		g_pLogMgr->nLen = 0;
		
		//csp modify
		SLogMsgHeader msg;
		msg.emType = EM_CLEAN_LOG;
		msg.index = 0;
		ResetCircleBuf(&g_LogMsgQueueInfo);
		LogWriteMsgQueue(&msg);
	}
	#endif
	
	u32 head = g_pLogMgr->nHead;
	memcpy(&g_pLogMgr->psLogList[head], pLog, sizeof(SLogInfo));
	
	//printf("log.time = %d\n",g_pLogMgr->psLogList[head].nTime);
	//printf("log.nType = %d\n",g_pLogMgr->psLogList[head].nType);
	//printf("log.aUsername = %s\n",g_pLogMgr->psLogList[head].aUsername);
	
	u32 maxLogs = g_pLogMgr->nMaxLogs;
	if(g_pLogMgr->nLen == maxLogs)
	{
		g_pLogMgr->nHead = (g_pLogMgr->nHead + 1)%maxLogs;
	}
	else
	{
		g_pLogMgr->nHead = (g_pLogMgr->nHead + 1)%maxLogs;
		g_pLogMgr->nLen++;
	}
	
	//csp modify
	//SLogMsgHeader msg;
	//msg.emType = EM_INSERT_LOG;
	//msg.index = head;//csp modify
	//LogWriteMsgQueue(&msg);
	
	//sem_post(&g_pLogMgr->semSync);//csp modify
	
	return head;
}

s32 ReadLog()
{
	//检索硬盘上log文件
	disk_manager* pHddMgr = (disk_manager*)g_pLogMgr->pHddMgr;
	char logFile[32] = {0};
	u32 pLastID = 0;
	u32 i = 0;
	for(i=0; i<MAX_HDD_NUM; i++)
	{
		if(pHddMgr->hinfo[i].is_disk_exist && pHddMgr->hinfo[i].is_partition_exist[0])
		{
			sprintf(logFile,"rec/%c%d/log.hex",'a'+i,1);
			break;
		}
	}
	if(i >= MAX_HDD_NUM)
	{
		PRINT("No HDD\n");
		return -1;
	}
	
	FILE* pFile = fopen(logFile,"rb");//"r"//csp modify
	if(pFile != NULL)
	{
		fseek(pFile, 0, SEEK_END);
		long filelen = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
		if(filelen != g_pLogMgr->nMaxLogs*sizeof(SLogInfo))
		{
			printf("ReadLog: sys log file len is error,recreate...\n");
			fclose(pFile);
			pFile = NULL;
			//remove(logFile);
		}
	}
	if(pFile == NULL)
	{
		pFile = fopen(logFile,"wb+");//"w+"//csp modify
		if(pFile != NULL)
		{
			SLogInfo log;
			memset(&log, 0, sizeof(log));
			for(i=0; i<g_pLogMgr->nMaxLogs; i++)
			{
				fwrite(&log, sizeof(log), 1, pFile);
			}
			fflush(pFile);
			fsync(fileno(pFile));
			
			fclose(pFile);
		}
		return -1;
	}
	
	SLogInfo log;
	s32 nLen = 0;
	
	while(1)
	{
		if((nLen = fread(&log, sizeof(log), 1, pFile)) <= 0)//<//csp modify
		{
			PRINT("Read log file error\n");
			fclose(pFile);//csp modify
			return -1;
		}
		//yaogang modify 20141128
		//if(log.magic1 == 0x5A && log.magic2 == 0xA5)
		if(log.magic2 == 0xA5)//magic1 用于记录报警源的序号或通道
		{
			break;
		}
	}
	
	u32 flag_offset = 0;
	u8 flag_set = 0;
	SLogInfo logPast;
	
	if(nLen > 0)
	{
		flag_offset++;
		pLastID = log.logId;
	}
	
	while(nLen > 0)
	{
		//csp modify
		//InsertLogList(&log);
		InsertLogList2(&log);
		
		memcpy(&logPast, &log, sizeof(SLogInfo));
		
		while(1)
		{
			nLen = fread(&log, sizeof(log), 1, pFile);
			if(nLen <= 0)//<//csp modify
			{
				PRINT("Read log file finished-1\n");
				break;
			}
			
			//yaogang modify 20141128
			//if(log.magic1 == 0x5A && log.magic2 == 0xA5)
			if(log.magic2 == 0xA5)//magic1 用于记录报警源的序号或通道
			{
				break;
			}
		}
		if(nLen <= 0)//<//csp modify
		{
			PRINT("Read log file finished-2\n");
			break;
		}
		
		if(pLastID < log.logId)//cw_log
		{
			pLastID = log.logId;
		}
		
		if(0 == flag_set)
		{
			flag_offset++;
			if((logPast.logId+1) != log.logId)//cw_log
			{
				flag_set = 1;
				flag_offset--;
			}
		}
	}
	
	if(flag_offset >= g_pLogMgr->nMaxLogs)
	{
		flag_offset = 0;
	}
	
#if 1//csp modify
	if(flag_set && g_pLogMgr->nLen == g_pLogMgr->nMaxLogs)
	{
		g_pLogMgr->nHead = flag_offset;
	}
#else
	//if(flag_set)//cw_log
	{
		g_pLogMgr->nHead = flag_offset;
	}
#endif
	
	g_pLogMgr->nLogID = pLastID;//cw_log
	
	//printf("cw*****%s,nHead = %d,nID = %d\n",__func__,g_pLogMgr->nHead,g_pLogMgr->nLogID);
	
	fclose(pFile);
	
	return 0;
}

void *SyncLogFxn(void *param)
{
	if(g_pLogMgr == NULL)
	{
		return 0;
	}
	
	printf("$$$$$$$$$$$$$$$$$$SyncLogFxn id:%d\n",getpid());
	
	disk_manager* pHddMgr = (disk_manager*)g_pLogMgr->pHddMgr;
	char logFile[32] = {0};
	
	s32 i = 0;
	for(i = 0; i < MAX_HDD_NUM; ++i)
	{
		if(pHddMgr->hinfo[i].is_disk_exist && pHddMgr->hinfo[i].is_partition_exist[0])
		{
			sprintf(logFile,"rec/%c%d/log.hex",'a'+i,1);
			break;
		}
	}
	if(i >= MAX_HDD_NUM)
	{
		sprintf(logFile, "rec/log.hex");
	}
	
	SLogMsgHeader nMsg;
	memset(&nMsg, 0, sizeof(SLogMsgHeader));
	
	printf("^^^^^^^%s,%d,pid=%d\n",__func__,__LINE__,getpid());
	
	while(1)
	{
		usleep(1);//csp modify
		
		//需要加锁:可能与硬盘格式化或升级冲突
		if(g_LogMsgQueueInfo.nLenUsed <= 0)
		{
			continue;
		}
		LogReadMsgQueue(&nMsg);
		
		//csp modify
		if(nMsg.emType == EM_CLEAN_LOG)
		{
			remove(logFile);
			sync();
			continue;
		}
		if(nMsg.emType != EM_INSERT_LOG)
		{
			continue;
		}
		if(nMsg.index >= g_pLogMgr->nMaxLogs)
		{
			continue;
		}
		
		//csp modify 20140812
		//unsigned char cur_atx_flag = tl_power_atx_check();
		//if(g_formating || cur_atx_flag == 0 || g_HotPlugFlag)
		if(g_formating || g_HotPlugFlag)
		{
			continue;
		}
		
		#if 0//csp modify
		u32 nHead = g_pLogMgr->nHead;
		u32 nLen = g_pLogMgr->nLen;
		if(nLen <= 0)
		{
			continue;
		}
		#endif
		
		for(i = 0; i < MAX_HDD_NUM; ++i)
		{
			if(pHddMgr->hinfo[i].is_disk_exist && pHddMgr->hinfo[i].is_partition_exist[0])
			{
				sprintf(logFile,"rec/%c%d/log.hex",'a'+i,1);
				break;
			}
		}
		if(i >= MAX_HDD_NUM)
		{
			//sprintf(logFile, "rec/log.hex");
			continue;
		}
		
		FILE* pFile = fopen(logFile,"rb+");
		if(pFile != NULL)
		{
			long filelen = 0;
			
			struct stat statbuf;
			if(fstat(fileno(pFile), &statbuf) < 0)
			{
				fseek(pFile, 0, SEEK_END);
				filelen = ftell(pFile);
				//fseek(pFile, 0, SEEK_SET);
			}
			else
			{
				filelen = statbuf.st_size;
			}
			
			if(filelen != g_pLogMgr->nMaxLogs*sizeof(SLogInfo))
			{
				printf("SyncLogFxn: sys log file len is error,recreate...\n");
				fclose(pFile);
				pFile = NULL;
				//remove(logFile);
			}
		}
		if(pFile == NULL)
		{
			pFile = fopen(logFile,"wb+");
			if(pFile == NULL)
			{
				printf("warning: open log file failed! fun:%s\n", __FUNCTION__);
				continue;//return 0;//csp modify
			}
			
			SLogInfo log;
			memset(&log, 0, sizeof(log));
			for(i=0; i<g_pLogMgr->nMaxLogs; i++)
			{
				fwrite(&log, sizeof(log), 1, pFile);
			}
			fflush(pFile);
			fsync(fileno(pFile));
			
			//fseek(pFile, 0, SEEK_SET);
			
			//continue;
		}
		
		#if 1//csp modify
		SLogInfo log = g_pLogMgr->psLogList[nMsg.index];
		fseek(pFile, nMsg.index*sizeof(log), SEEK_SET);
		if(fwrite(&log,sizeof(log),1,pFile) < 0)
		{
			PRINT("sync log error");
		}
		#else
		SLogInfo* pLog = NULL;
		u32 nMaxLogs = g_pLogMgr->nMaxLogs;
		if(0 == nHead)
		{
			pLog = &g_pLogMgr->psLogList[nMaxLogs-1];
			fseek(pFile, (nMaxLogs-1)*sizeof(*pLog), SEEK_SET);
		}
		else
		{
			pLog = &g_pLogMgr->psLogList[nHead-1];
			fseek(pFile, (nHead-1)*sizeof(*pLog), SEEK_SET);
		}
		if(fwrite(pLog,sizeof(*pLog),1,pFile) < 0)
		{
			PRINT("sync log error");
		}
		#endif
		
		fflush(pFile);
		fsync(fileno(pFile));
		fclose(pFile);
		
		//printf("^^^^^^^^^^^^^^^^^^writelog %lu,0x%04x,max=%d,nHead=%d\n",log.nTime,log.nType,g_pLogMgr->nMaxLogs,g_pLogMgr->nHead);
	}
	
	return 0;//csp modify
}

s32 ModLogInit(SLogInitPara* psInitPara)
{
	//ModLogInit: sizeof(SLogInfo)=60, sizeof(SBizAlarmInfo)=64
	//printf("ModLogInit: sizeof(SLogInfo)=%d, sizeof(SBizAlarmInfo)=%d\n", sizeof(SLogInfo), sizeof(SBizAlarmInfo));
	
	if(psInitPara == NULL)
	{
		PRINT("Log InitPara error\n");
		return -1;
	}
	
	if(NULL == psInitPara->pHddMgr)
	{
		PRINT("Log InitPara error\n");
		return -1;
	}
	
	if(psInitPara->nMaxLogs <= 0)
	{
		PRINT("Log InitPara error\n");
		return -1;
	}
	
	if(!(g_pLogMgr = malloc(sizeof(SLogMgr))))
	{
		PRINT("malloc LogMgr error\n");
		return -1;
	}
	
	memset(g_pLogMgr,0,sizeof(SLogMgr));
	
	g_pLogMgr->nLen = 0;
	g_pLogMgr->nHead = 0;
	g_pLogMgr->nMaxLogs = psInitPara->nMaxLogs;	
	g_pLogMgr->nLogID = 0;//cw_log
	g_pLogMgr->pHddMgr = (disk_manager*)psInitPara->pHddMgr;
	
	if(!(g_pLogMgr->psLogList = (SLogInfo *)malloc(sizeof(SLogInfo)*g_pLogMgr->nMaxLogs)))
	{
		PRINT("malloc LogList error\n");
		free(g_pLogMgr);//csp modify
		return -1;
	}
	
	//sem_init(&g_pLogMgr->semInsert,0,0);//csp modify
	//sem_init(&g_pLogMgr->semSync,0,1);//csp modify
	sem_init(&g_pLogMgr->semLock,0,1);
	
	LogCreateMsgQueue();
	
	//LogManager_Init(&g_flashLog);
	//DiskLoger_Init(&g_diskLog, g_pLogMgr->pHddMgr);//csp modify 20140812
	
	disk_manager* pHddMgr = (disk_manager*)g_pLogMgr->pHddMgr;
	
	u32 i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if(pHddMgr->hinfo[i].is_disk_exist && pHddMgr->hinfo[i].is_partition_exist[0])
		{
			break;
		}
	}
	if(i >= MAX_HDD_NUM)
	{
		PRINT("No HDD\n");
		return -2;
	}
	
	ReadLog();
	
	g_pLogMgr->hLogThread = TL_CreateThread(SyncLogFxn, 
											NULL, 
											PRI_DECODE, 
											(512<<10), 
											0, 
											0, 
											NULL);
	
	return 0;
}

s32 ModLogDeinit()
{
	if(!g_pLogMgr)
	{
		PRINT("deinit error\n");
		return -1;
	}
	
	TL_TerminateThread(g_pLogMgr->hLogThread);//csp modify
	
	if(g_pLogMgr->psLogList)
	{
		free(g_pLogMgr->psLogList);
		g_pLogMgr->psLogList = NULL;
	}
	
	//sem_destroy(&g_pLogMgr->semInsert);//csp modify
	//sem_destroy(&g_pLogMgr->semSync);//csp modify
	sem_destroy(&g_pLogMgr->semLock);
	
	//TL_TerminateThread(g_pLogMgr->hLogThread);//csp modify
	LogDestroyMsgQueue();//csp modify
	
	free(g_pLogMgr);
	g_pLogMgr = NULL;
	
	return 0;
}

s32 ModWriteLog(SLogInfo* psLogInfo)
{
	if(!g_pLogMgr)
	{
		PRINT("log module not init\n");
		return -1;
	}
	
	if(!psLogInfo)
	{
		PRINT("log info error\n");
		return -1;
	}
	
	//csp modify 20140812
	//unsigned char cur_atx_flag = tl_power_atx_check();
	//if(g_formating || cur_atx_flag == 0 || g_HotPlugFlag)
	if(g_formating || g_HotPlugFlag)	
	{
		PRINT("formatting or without_atx or hotplug\n");
		return -1;
	}
	
	sem_wait(&g_pLogMgr->semLock);
	
	psLogInfo->logId = GetLogID();//cw_log
	//printf("%d,cw**********Insert ID = %d\n",__LINE__,psLogInfo->logId);
	
	//psLogInfo->magic1 = 0x5A;//magic1 用于记录报警源的序号或通道
	psLogInfo->magic2 = 0xA5;
	
	InsertLogList(psLogInfo);
	
	sem_post(&g_pLogMgr->semLock);
	
	return 0;
}
/*
s32 ModWriteAlarmLog(SBizAlarmInfo* psLogInfo)
{
	psLogInfo->magic = 0x5A;
	
	//csp modify 20140812
	unsigned char cur_atx_flag = tl_power_atx_check();
	if(!g_formating && cur_atx_flag && !g_HotPlugFlag)
	{
		DiskLoger_Write(&g_diskLog, psLogInfo);
	}
	
	return LogManager_Write(&g_flashLog, psLogInfo);
}
*/
/*
s32 ModAlarmSearchLog(SAlarmLogSearchPara* psLogSearch, SAlarmLogResult* psLogResult)
{
	//csp modify 20140812
	if(psLogSearch == NULL || psLogResult == NULL)
	{
		return -1;
	}
	
	//csp modify 20140812
	if(psLogSearch->nMode == 0)
	{
		printf("search alarm log from flash\n");
		return LogManager_Read(&g_flashLog, psLogSearch, psLogResult);
	}
	else
	{
		printf("search alarm log from disk\n");
		return DiskLoger_Read(&g_diskLog, psLogSearch, psLogResult);
	}
}
*/
s32 ModSearchLog(SLogSearchPara* psLogSearch, SLogResult* psLogResult)
{
	if(!psLogSearch || !psLogResult || !psLogResult->psLogList)
	{	
		PRINT("search param error");
		return -1;
	}
	
	if(!g_pLogMgr)
	{
		PRINT("log module not init\n");
		return -1;
	}
	
	s32 nMaxLogs = g_pLogMgr->nMaxLogs;
	if(psLogResult->nMaxLogs < nMaxLogs)
	{
		PRINT("log result list not enough");
		return -1;
	}
	
	sem_wait(&g_pLogMgr->semLock);
	
	psLogResult->nRealNum = 0;
	
	SLogInfo* pLog = NULL;
	s16 masterMask = psLogSearch->nMasterMask;
	s32 nHead = g_pLogMgr->nHead;
	s32 nLen = g_pLogMgr->nLen;
	while(nLen)
	{
		if(0 == nHead)
		{
			nHead = nMaxLogs - 1;
		}
		else
		{
			nHead--;
		}
		
		pLog = &g_pLogMgr->psLogList[nHead];
		
		s32 i = 0;
		while(i < MAX_MASTER_TYPE)
		{
			if(masterMask&(1<<i))
			{
				s8 masterType = i+1;
				if(masterType == ModGetMaster(pLog->nType)
					&& pLog->nTime >= psLogSearch->nStart
					&& pLog->nTime <= psLogSearch->nEnd)
				{
					psLogResult->psLogList[psLogResult->nRealNum++] = *pLog;
					break;//csp modify
				}
			}
			
			i++;
		}
		
		nLen--;
	}
	
	sem_post(&g_pLogMgr->semLock);
	
	return 0;
}

s32 ModRegistLogExportCB(LOGEXPORTCB logExportCb)
{
	if(logExportCb)
	{
		g_LogExportCB = logExportCb;
		return 0;
	}
	
	return -1;
}

s32 ModExportLog(SLogResult* psLogResult, char* pPath)
{
	if(!pPath)
	{
		PRINT("export path error\n");
		return -1;
	}
	
	if(!g_LogExportCB)
	{
		PRINT("not regist export callback\n");
		return -1;
	}
	
	//csp modify
	//FILE* pFile = fopen(pPath,"rb+");
	FILE* pFile = fopen(pPath,"ab+");
	if(pFile == NULL)
	{
		pFile = fopen(pPath,"wb+");
		if(pFile == NULL)
		{
			return -1;
		}
	}
	
	if(psLogResult == NULL)
	{
		//导出所有日志
	}
	else
	{
		//导出查找结果日志
		u8 strLog[256] = {0};
		SLogInfo* pLog = NULL;
		
		s32 nRealNum = psLogResult->nRealNum;
		
		s32 i = 0;
		for(i=0; i<nRealNum; i++)
		{
			pLog = &psLogResult->psLogList[i];
			
			g_LogExportCB(pLog, strLog);
			
			//printf("%s\n",strLog);
			if(fwrite(strLog,1,strlen((char *)strLog),pFile) < 0)
			{
				PRINT("export log error");
				fclose(pFile);//csp modify
				return -1;
			}
		}
	}
	
	//csp modify
	fflush(pFile);
	fsync(fileno(pFile));
	
	fclose(pFile);
	
	//sync();//csp modify
	
	return 0;
}

s32 LogCreateMsgQueue(void)//cw_log
{
	g_LogMsgQueueInfo.nBufId = 0;
	g_LogMsgQueueInfo.nLength = g_pLogMgr->nMaxLogs * sizeof(SLogMsgHeader);
	
	s32 ret = CreateCircleBuf(&g_LogMsgQueueInfo);
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	printf("msg queue created succesful!!\n");
	
	return ret;
}

s32 LogDestroyMsgQueue(void)
{
	s32 ret = DestroyCircleBuf(&g_LogMsgQueueInfo);
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

s32 LogWriteMsgQueue(SLogMsgHeader* pHeader)
{
	s32 ret = 0;
	ret = WriteDataToBuf(&g_LogMsgQueueInfo, (u8 *)pHeader, sizeof(SLogMsgHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

s32 LogReadMsgQueue(SLogMsgHeader* pHeader)
{
	s32 ret = 0;
	ret = ReadDataFromBuf(&g_LogMsgQueueInfo, (u8 *)pHeader, sizeof(SLogMsgHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

