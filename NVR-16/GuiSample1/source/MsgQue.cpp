#include "MultiTask/MsgQue.h"
#include "MultiTask/Guard.h"

CMsgQue::CMsgQue(int size /* = 1024 */)// : m_Queue(size)
{
/************************************************************************
	消息队列的初始化工作：
	1、一次性把队列的所有消息结构体对象都创建起来;
	2、把这些对象都加到空闲队列m_FreeQueue;
	3、把消息队列可访问标志置为有效；
************************************************************************/
	for (int i = 0; i < MSG_PRIORITY; i++)
	{
//		m_aMQIterator[i] = m_Queue.begin();
	}
	m_nMaxMsg = size;
	m_nMsg = 0;
	m_bMsgFlg = TRUE;
}

CMsgQue::~CMsgQue()
{

}

VD_BOOL CMsgQue::SendMessage(uint msg, uint wpa /* = 0 */, uint lpa /* = 0 */, uint priority /* = 0 */)
{
	VD_MSG l_MSG;
	MSGQUEUE::iterator pi;

	m_Mutex.Enter();
	if(m_nMsg >= m_nMaxMsg)
	{
		m_Mutex.Leave();
		return FALSE;
	}
	if(priority >= MSG_PRIORITY)
	{
		m_Mutex.Leave();
		return FALSE;
	}
	if (!m_bMsgFlg) 
	{
		m_Mutex.Leave();
		return FALSE;
	}

	//鼠标消息特殊处理, 合并鼠标移动消息
	if(msg == XM_MOUSEMOVE && !m_Queue.empty())
	{
		for (pi = m_Queue.begin(); pi != m_Queue.end(); pi++)
		{
			if((*pi).msg >= XM_LBUTTONDOWN && (*pi).msg <= XM_MBUTTONDBLCLK)
			{
				break;
			}
			if((*pi).msg == XM_MOUSEMOVE)
			{
				(*pi).wpa = wpa;
				(*pi).lpa = lpa;
				(*pi).time = SystemGetMSCount();
				m_Mutex.Leave();
				return TRUE;
			}
		}
	}
/************************************************************************
	发送消息:
	1、按照优先级把该消息插入队列m_Queue；
//	2、在该队列中查找该消息节点，直到找不到该节点才退出循环并从该函数返回；
************************************************************************/
	l_MSG.msg = msg;
	l_MSG.wpa = wpa;
	l_MSG.lpa = lpa;
	l_MSG.time = SystemGetMSCount();
//	m_aMQIterator[priority] = m_Queue.insert(m_aMQIterator[priority], l_MSG);
//printf("<<<<<<<<<<l_MSG.msg==%x l_MSG.wpa==%x l_MSG.lpa==%x\n",	l_MSG.msg, l_MSG.wpa, l_MSG.lpa);
	m_Queue.push_front(l_MSG);
	m_nMsg++;
	m_Mutex.Leave();

	m_Semaphore.Post();
	return TRUE;
}

VD_BOOL CMsgQue::RecvMessage(VD_MSG *pMsg, VD_BOOL wait /* = TRUE */)
{
/************************************************************************
	接收消息，如果等待则一直等到有消息时返回，否则直接返回。
	1、从消息忙队列m_Queue取元素，如果取成功，则直接返回；
	2、否则循环从消息忙m_Queue中取元素，直到取成功才退出循环；
************************************************************************/
	if(wait)
	{
		m_Semaphore.Pend();
	}

	CGuard guard(m_Mutex);
//	trace("RecvMessage %d\n", m_nMsg);

	if (m_Queue.empty()) 
	{
//		tracepoint();
		return FALSE;
	}
	if(!wait)
	{
		m_Semaphore.Pend();
	}
	assert(m_nMsg);
	*pMsg = m_Queue.back();
	m_Queue.pop_back();
//	printf("pMsg==%x  l_MSG.msg==%x l_MSG.wpa==%x l_MSG.lpa==%x\n",	pMsg, l_MSG.msg, l_MSG.wpa, l_MSG.lpa);
	m_nMsg--;

	return TRUE;
}

void CMsgQue::QuitMessage ()
{
	CGuard guard(m_Mutex);

	m_bMsgFlg = FALSE;
}

void CMsgQue::ClearMessage()
{
	CGuard guard(m_Mutex);

	int n = m_nMsg;
	for(int i = 0; i < n; i++)
	{
		m_Semaphore.Pend();
		m_Queue.pop_back();
		m_nMsg--;
	}
}

int CMsgQue::GetMessageCount()
{
	CGuard guard(m_Mutex);

	return m_nMsg;
}

int CMsgQue::GetMessageSize()
{
	CGuard guard(m_Mutex);

	return m_nMaxMsg;
}

void CMsgQue::SetMessageSize(int size)
{
	CGuard guard(m_Mutex);

	m_nMaxMsg = size;
}
