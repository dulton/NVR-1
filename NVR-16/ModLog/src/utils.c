#include "utils.h"

/***********************************************************/
/*                              线程相关接口                                 */

#ifdef LINUX
THREADHANDLE TL_CreateThread(LINUXFUNC pvTaskEntry, 
									char* szName, 
									u8 byPriority, 
									u32 dwStacksize, 
									u32 dwParam, 
									u16 wFlag, 
									u32* pdwTaskID)
#else
THREADHANDLE TL_CreateThread(void* pvTaskEntry, 
									char* szName, 
									u8 byPriority, 
									u32 dwStacksize, 
									u32 dwParam, 
									u16 wFlag, 
									u32* pdwTaskID)
#endif
{
	THREADHANDLE  hTask;
	
#ifdef WIN32
	u32 dwTaskID;
	int Priority;

	if(szName == NULL)
	{
		//nothing to do
		//用于避免警告信息
	} 
	
	wFlag = 0;

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
	
    hTask = CreateThread(NULL, 
							dwStacksize, 
							(LPTHREAD_START_ROUTINE)pvTaskEntry, 
							(char*)dwParam, 
							0, 
							&dwTaskID);
	
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
#else  //LINUX
	
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

	nRet = pthread_create(&hTask, &tThreadAttr, pvTaskEntry, (void*)dwParam);
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

BOOL TL_ExitThread()
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

BOOL TL_TerminateThread(THREADHANDLE hTask)
{
#ifdef WIN32
	return TerminateThread(hTask, 0);
#else
	void* temp;
	pthread_cancel(hTask);
	return ( 0 == pthread_join(hTask, &temp) );
#endif
}

