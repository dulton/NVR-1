#ifndef _MSG_QUE_H_
#define _MSG_QUE_H_

#if defined(WIN32)
	#pragma warning (disable : 4786)
#endif
#include <list>
#include "APIs/System.h"
#include "Mutex.h"
#include "Semaphore.h"
#include "System/pool_allocator.h"

#define MSG_PRIORITY	10

typedef struct VD_tagMSG 
{
	uint		msg;
	uint		wpa;
	uint		lpa;
	uint		time;
}VD_MSG;

typedef std::list<VD_MSG, pool_allocator<VD_MSG> > MSGQUEUE;

class CMsgQue
{
public:
	CMsgQue(int size = 1024);
	virtual ~CMsgQue();

	VD_BOOL SendMessage (uint msg, uint wpa = 0, uint lpa = 0, uint priority = 0);
	VD_BOOL RecvMessage (VD_MSG *pMsg, VD_BOOL wait = TRUE);
	void QuitMessage ();
	void ClearMessage();
	int GetMessageCount();
	int GetMessageSize();
	void SetMessageSize(int size);

protected:
private:
	MSGQUEUE m_Queue;
//	MSGQUEUE::iterator m_aMQIterator[MSG_PRIORITY]; 
	VD_BOOL m_bMsgFlg;
	CMutex m_Mutex;
	CSemaphore m_Semaphore;
	int m_nMsg;
	int m_nMaxMsg;
};

#endif// _MSG_QUE_H_
