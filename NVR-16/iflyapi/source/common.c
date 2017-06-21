#include "common.h"

#ifdef USE_PIPE
#include <sys/ioctl.h>
#else
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define MAX_MSG_LEN		1024
struct msgbuffer
{
	long mtype;     /* message type, must be > 0 */
	char mtext[MAX_MSG_LEN];  /* message data */
};
#endif

MSGQHANDLE IFly_CreateMsgQueue(u32 dwMsgNumber, u32 dwMsgLength)
{
	int nRet;

	MSGQHANDLE hMsgQ = (MSGQHANDLE)malloc(sizeof(MSGQueue));
	if(hMsgQ == NULL)
	{
		printf("create msg queue failed for malloc.\n");
		return NULL;
	}

	hMsgQ->dwMsgNumber = dwMsgNumber;
	hMsgQ->dwMsgLength = dwMsgLength;
	
#ifdef USE_PIPE
	int fd[2];
	nRet = pipe(fd);
	if(nRet<0)
	{
		free(hMsgQ);
		return NULL;
	}
	hMsgQ->dwReadID  = (u32)fd[0];
	hMsgQ->dwWriteID = (u32)fd[1];

	int ret;
	unsigned long arg = 1;
	//ret = ioctl( fd[0], FIONBIO, (int)&arg );
	ret = ioctl( fd[1], FIONBIO, (int)&arg );

#else
	key_t key;
	int msgflg;
	key = IPC_PRIVATE;
	msgflg = IPC_CREAT | IPC_EXCL;
	nRet = msgget(key, msgflg);
	if(nRet<0)
	{
		printf("create msg queue failed for ipc create.\n");
		free(hMsgQ);
		return NULL;
	}
	hMsgQ->dwReadID  = (u32)nRet;
	hMsgQ->dwWriteID = (u32)nRet;
	printf("create msg queue success ipc id=%d.\n",nRet);
#endif

	return hMsgQ;
}

void IFly_CloseMsgQueue(MSGQHANDLE hMsgQ)
{
	if(hMsgQ == NULL)
	{
		return;
	}
	
#ifdef USE_PIPE
	close((int)hMsgQ->dwReadID);
	close((int)hMsgQ->dwWriteID);
#else
	int nRet;
	nRet = msgctl((int)hMsgQ->dwReadID,IPC_RMID,NULL);
	if(nRet<0)
	{
		fprintf(stderr,"close msg queue failed.\n");
	}
#endif
	
	free(hMsgQ);
}

int IFly_SndMsg(MSGQHANDLE hMsgQ, char *pchMsgBuf, u32 dwLen, int nTimeout)
{
	if(hMsgQ->dwWriteID < 0 || pchMsgBuf == NULL)
	{
		return -1;
	}
	
#ifdef USE_PIPE
	int ret;
	ret = write((int)hMsgQ->dwWriteID,pchMsgBuf,dwLen);
	return ret;
#else

	/*if(dwLen>hMsgQ->dwMsgLength)
	{
		printf("send msg too large.\n");
		return -1;
	}*/
	
	int nRet;
	struct msgbuffer sndbuf;
	sndbuf.mtype = 1;
	memset(sndbuf.mtext,0,sizeof(sndbuf.mtext));
	memcpy(sndbuf.mtext,pchMsgBuf,min(dwLen,MAX_MSG_LEN));
	nRet = msgsnd((int)hMsgQ->dwWriteID,&sndbuf,min(dwLen,MAX_MSG_LEN),0 | IPC_NOWAIT);
	if(nRet == 0)
	{
		return min(dwLen,MAX_MSG_LEN);
	}
	return nRet;
#endif
}

int IFly_RcvMsg(MSGQHANDLE hMsgQ, char *pchMsgBuf, u32 dwLen, int nTimeout)
{
	if(hMsgQ->dwReadID < 0 || pchMsgBuf == NULL)
	{
		return -1;
	}

#ifdef USE_PIPE
	int ret;
	ret = read((int)hMsgQ->dwReadID,pchMsgBuf,dwLen);
	return ret;
#else
	int nRet;
	struct msgbuffer rcvbuf;
	rcvbuf.mtype = 1;
	memset(rcvbuf.mtext,0,sizeof(rcvbuf.mtext));
	nRet = msgrcv((int)hMsgQ->dwReadID,&rcvbuf,sizeof(rcvbuf.mtext),1,0);//IPC_NOERROR
	if(nRet>0)
	{
		memcpy(pchMsgBuf,rcvbuf.mtext,min(dwLen,nRet));
	}
	return nRet;
#endif
}

#ifdef WIN32
THREADHANDLE IFly_CreateThread(void* pvTaskEntry, char* szName, u8 byPriority, u32 dwStacksize, u32 dwParam, u16 wFlag, u32 *pdwTaskID)
#else
THREADHANDLE IFly_CreateThread(LINUXFUNC pvTaskEntry, char* szName, u8 byPriority, u32 dwStacksize, u32 dwParam, u16 wFlag, u32 *pdwTaskID)
#endif
{
	THREADHANDLE  hTask;
	
#ifdef WIN32
	u32 dwTaskID;
	int Priority;

	if(szName == NULL){} /* 用于避免告警 */
	wFlag = 0; /* 用于避免告警 */

	if(byPriority < 50)
	{
		Priority = THREAD_PRIORITY_TIME_CRITICAL;
	}
	else if(byPriority < 100)
	{
		Priority = THREAD_PRIORITY_HIGHEST;
	}
	else if(byPriority < 120)
	{
		Priority = THREAD_PRIORITY_ABOVE_NORMAL;
	}
	else if(byPriority < 150)
	{
		Priority = THREAD_PRIORITY_NORMAL;
	}
	else if(byPriority < 200)
	{
		Priority = THREAD_PRIORITY_BELOW_NORMAL;		
	}	
	else
	{
		Priority = THREAD_PRIORITY_LOWEST;
	}
	
    hTask = CreateThread(NULL, dwStacksize, (LPTHREAD_START_ROUTINE)pvTaskEntry, (char * )dwParam, 0, &dwTaskID);
	if(hTask != NULL)
    {
		SetThreadAffinityMask(hTask, THREADAFFMASK); // 设置线程的处理器姻亲掩码

		if(SetThreadPriority(hTask, Priority) != 0)  
		{
			if(pdwTaskID != NULL) 
			{
				*pdwTaskID = dwTaskID;
			}
			return hTask;
		}
    }
#else
	
	int nRet = 0;
	struct sched_param tSchParam;	
	pthread_attr_t tThreadAttr;
	int nSchPolicy;

	pthread_attr_init(&tThreadAttr);

	// 设置调度策略
	pthread_attr_getschedpolicy(&tThreadAttr, &nSchPolicy);
	nSchPolicy = SCHED_FIFO;
	pthread_attr_setschedpolicy(&tThreadAttr, nSchPolicy);

	// 设置优先级
	pthread_attr_getschedparam(&tThreadAttr, &tSchParam);
	byPriority = 255-byPriority;
	if(byPriority < 60)
	{
		byPriority = 60;
	}
	tSchParam.sched_priority = byPriority;
	pthread_attr_setschedparam(&tThreadAttr, &tSchParam);

	pthread_attr_setstacksize(&tThreadAttr, (size_t)dwStacksize);

	nRet = pthread_create(&hTask, &tThreadAttr, pvTaskEntry, (void *)dwParam);
	if(nRet == 0)
	{
		if(pdwTaskID != NULL)
		{
			*pdwTaskID = (u32)hTask;
		}
		return hTask;
	}
#endif

    return 0;
}

BOOL IFly_ThreadExit()
{
#ifdef WIN32
	ExitThread(0);
	return TRUE;
#else
	static int nRetCode = 0;
	pthread_exit(&nRetCode);
	return TRUE;
#endif
}

BOOL IFly_ThreadTerminate(THREADHANDLE hTask)
{
#ifdef WIN32
	return TerminateThread(hTask, 0);
#else
	void *temp;
	
	pthread_cancel(hTask);
	return ( 0 == pthread_join(hTask, &temp) );
#endif
}

void Dump_Thread_Info(char *name,int tid)
{
#ifndef WIN32
	static FILE *fp = NULL;
	if (NULL == fp)
	{
		fp = fopen("/web/pages/threadinfo.txt", "w");
	}

	if(fp != NULL)
	{
		char msg[64];
		memset(msg,' ',64);
		msg[63] = 0;
		memcpy(msg,name,strlen(name));
		sprintf(msg+32,"%d\n",tid);
		fwrite(msg,strlen(msg),1,fp);
		fflush(fp);
		fsync(fileno(fp));
	}
#endif
}
