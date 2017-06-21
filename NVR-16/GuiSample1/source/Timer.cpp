
#include "MultiTask/Timer.h"
#include <string.h>

CMutex CTimer::m_Mutex(MUTEX_RECURSIVE);

CTimer::CTimer(VD_PCSTR pName)
{
	SetName(pName);
	m_Started = FALSE;
	m_Called = FALSE;
}

CTimer::~CTimer()
{
	CGuard guard(m_Mutex);

	if(m_Started)
	{
		g_TimerManager.RemoveTimer(this);
	}
}

void CTimer::Start(CObject * pObj, VD_TIMERPROC pTimerFun, uint dwDueTime, uint dwPriod, uint param /* = 0 */, uint timeout /* = 0 */)
{
	CGuard guard(m_Mutex);
	m_pObj = pObj;
	m_pTimerFun = pTimerFun;
	m_CallTime = g_TimerManager.m_CurTime;
	m_CallTime += dwDueTime;  
	m_Priod = dwPriod;
	m_timeout = timeout;
	m_param = param;
	m_Called = FALSE;
	if(m_Started)
	{
		g_TimerManager.RemoveTimer(this);
	}
	g_TimerManager.AddTimer(this);
	m_Started = TRUE;
}

void CTimer::Stop(VD_BOOL bCallNow /* = FALSE */)
{
	CGuard guard(m_Mutex);
	if(!m_Started)
	{
		return;
	}

	//结束时立即调用一次回调函数，在需要提前结束非周期定时器时有用
	if(bCallNow && m_Priod == 0)
	{
		(m_pObj->*m_pTimerFun)(m_param); 
	}
	g_TimerManager.RemoveTimer(this);
	m_Started = FALSE;
}

void CTimer::run()
{
	g_TimerManager.RemoveTimer(this);
	if(m_Priod)
	{
		m_CallTime += m_Priod;
		g_TimerManager.AddTimer(this);
	}
	else
	{
		m_Started = FALSE;
		m_Called = TRUE;	
	}

	m_threadlet.run(m_Name, m_pObj, m_pTimerFun, m_param, m_timeout);
}

VD_PCSTR CTimer::GetName()
{
	return m_Name;
}

void CTimer::SetName(VD_PCSTR pszName)
{
	int len = MIN(strlen(pszName), 31);
	memcpy(m_Name, pszName, len);
	m_Name[len]='\0';
}

VD_BOOL CTimer::IsStarted()
{
	return m_Started;
}

VD_BOOL CTimer::IsCalled()
{
	return m_Called;
}

////////////////////////////////////////////////////////////////////////////////////
// CTimerManager
////////////////////////////////////////////////////////////////////////////////////

PATTERN_SINGLETON_IMPLEMENT(CTimerManager);

CTimerManager::CTimerManager():CThread("TimerManager", TP_TIMER)
{
	m_pHead = NULL;
	
#ifdef VN_IPC_VC2500
	m_CurTime = SystemGetMSCount();
#else
	m_CurTime = SystemGetMSCount64();
#endif
	
	printf("CTimerManager::CTimerManager()>>>>>>>>>\n");
}

CTimerManager::~CTimerManager()
{
	DestroyThread();
}

void CTimerManager::Start()
{
	CreateThread();
}

VD_BOOL CTimerManager::AddTimer(CTimer * pTimer)
{
	CTimer *pp,*pn;

	pp = NULL;
	pn = m_pHead;

	while(pn && (pTimer->m_CallTime > pn->m_CallTime))
	{
		pp = pn;
		pn = pn->m_pNext;
	}

	if(pp)
	{
		pp->m_pNext = pTimer;
	}
	else
	{
		m_pHead = pTimer;
	}
	pTimer->m_pPrev = pp;

	if(pn)
	{
		pn->m_pPrev = pTimer;
	}
	pTimer->m_pNext = pn;
	
	return TRUE;
}

VD_BOOL CTimerManager::RemoveTimer(CTimer * pTimer)
{
	if(pTimer->m_pPrev == NULL)
	{
		m_pHead = pTimer->m_pNext;
		if(m_pHead)
		{
			m_pHead->m_pPrev = NULL;
		}

		return TRUE;
	}

	pTimer->m_pPrev->m_pNext = pTimer->m_pNext;
	if(pTimer->m_pNext != NULL)
		pTimer->m_pNext->m_pPrev = pTimer->m_pPrev;

	return TRUE;
}

void CTimerManager::DumpTimers()
{
	CTimer * p = m_pHead;

	CGuard guard(CTimer::m_Mutex);
	printf("Timers: ( %llu Milli-Seconds Elapsed )\n", m_CurTime);
	printf("_________________________________________\n");
	while (p)
	{
		printf("%16s %11llu %s\n", p->GetName(), p->m_CallTime, p->m_threadlet.isRunning() ? "Running" : "Idel");
		p = p->m_pNext;
	}
}

void CTimerManager::ThreadProc()
{
	printf("Enter CTimerManager::ThreadProc...\n");
	
	//getchar();
	
	printf("Enter CTimerManager::ThreadProc^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^6\n");
	
	do
	{
		SystemSleep(1);

		CGuard guard(CTimer::m_Mutex);

		uint64 OldTime = m_CurTime;
		
#ifdef VN_IPC_VC2500
		m_CurTime = SystemGetMSCount();
#else
		m_CurTime = SystemGetMSCount64();
#endif
		
		//trace("CTimerManager::Check()  %llu , %llu .\n", m_CurTime, OldTime);
		
		// 计时没有改变，可能是因为计时器精度不高
		if(m_CurTime == OldTime)
		{
			continue;
		}

		if(m_CurTime < OldTime)
		{
			printf("CTimerManager::Check() MSCount Overflowed, %llu < %llu .\n", m_CurTime, OldTime);

			m_CurTime = 0;
		}

		while(m_pHead && m_pHead->m_CallTime < m_CurTime)
		{
			//printf("\r\nCTimerManager::ThreadProc() MSCount  curtime:%llu  oldtime:%llu .(m_pHead->m_CallTime:%llu)\r\n", m_CurTime, OldTime,m_pHead->m_CallTime);
			m_pHead->run();
		}
	} while(m_bLoop);
}

