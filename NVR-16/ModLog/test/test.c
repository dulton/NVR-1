#include <stdio.h>
#include <time.h>
#include "utils.h"
#include "mod_log.h"

void LogExportCB(SLogInfo* pLog, u8* strLog)
{
	if(!pLog)
	{
		return;
	}

	u8 ip[32] = {0};
	u8 username[48] = {0};
	u8 master[24] = {0};
	u8 slave[24] = {0};
	u8 stTime[64] = {0};

	u8 masterType = ModGetMaster(pLog->nType);
	u8 slaveType = ModGetSlave(pLog->nType);
	sprintf(master,"MasterType:%d\n\r",masterType);
	sprintf(slave,"SlaveType:%d\n\r",slaveType);
	
	sprintf(ip,"Ip:%s\n\r",pLog->aIp);
	sprintf(username,"Username:%s\n\r",pLog->aUsername);
	
	time_t time = pLog->nTime;
	
	struct tm sTime;
	//csp modify
	//sTime = *localtime(&time);
	localtime_r(&time, &sTime);
	sprintf(stTime,"Time:%s",asctime(&sTime));
	
	sprintf(strLog,"%s",stTime);
	sprintf(strLog+strlen(stTime),"%s",ip);
	sprintf(strLog+strlen(stTime)+strlen(ip),"%s",username);
	sprintf(strLog+strlen(stTime)+strlen(ip)+strlen(username),"%s",master);
	sprintf(strLog+strlen(stTime)+strlen(ip)+strlen(username)+strlen(master),"%s\n\r",slave);
}

int main()
{
	disk_manager g_sHddManager;
	init_disk_manager(&g_sHddManager);	
	
	SLogInitPara initPara;
	memset(&initPara,0,sizeof(SLogInitPara));

	initPara.pHddMgr = &g_sHddManager;
	initPara.nMaxLogs = 1024;
	ModLogInit(&initPara);

	SLogInfo log;
	memset(&log,0,sizeof(SLogInfo));

	ModRegistLogExportCB(LogExportCB);

	SLogSearchPara searchPara;
	memset(&searchPara,0,sizeof(searchPara));
	searchPara.nMasterMask = 0x000f;
	searchPara.nStart = time(NULL) - 20000;
	searchPara.nEnd = time(NULL);

	SLogResult logResult;
	memset(&logResult,0,sizeof(SLogResult));
	logResult.nMaxLogs = 1024;
	logResult.psLogList = malloc(sizeof(SLogInfo)*logResult.nMaxLogs);
	
	ModSearchLog(&searchPara,&logResult);
	
	ModExportLog(&logResult,"export.log");

	#if 0
	int i = 0;
	for(i=0; i<7; i++)
	{
		log.nTime = time(NULL);
		log.nType = ModMakeType(i+1,1);
		strcpy(log.aIp,"127.0.0.1");
		strcpy(log.aUsername,"guest");

		//printf("log.time = %d \n",log.nTime);
		//printf("log.nType = %d \n",log.nType);

		ModWriteLog(&log);

		//sleep(2);
	}
	#endif


	#if 0
	SLogSearchPara searchPara;
	memset(&searchPara,0,sizeof(searchPara));
	searchPara.nMasterMask = 0x000f;
	searchPara.nStart = time(NULL) - 20000;
	searchPara.nEnd = time(NULL);

	//printf("start time: %d\n",searchPara.nStart);
	//printf("end time: %d\n",searchPara.nEnd);

	SLogResult logResult;
	memset(&logResult,0,sizeof(SLogResult));
	logResult.nMaxLogs = 1024;
	logResult.psLogList = malloc(sizeof(SLogInfo)*logResult.nMaxLogs);
	
	ModSearchLog(&searchPara,&logResult);

	//printf("Real log num = %d \n",logResult.nRealNum);

	for(i=0; i<logResult.nRealNum; i++)
	{
		printf("log %d: \n",i+1);
		printf("type: %d\n",logResult.psLogList[i].nType);
		printf("time: %d\n",logResult.psLogList[i].nTime);
		printf("ip: %s\n",logResult.psLogList[i].aIp);
		printf("username: %s\n",logResult.psLogList[i].aUsername);
		printf("\n");
	}
	#endif

	//ModLogDeinit();

	printf("go to usershell\n");
	while(1)
	{
		usleep(100*1000);
	}

	return 0;
}

