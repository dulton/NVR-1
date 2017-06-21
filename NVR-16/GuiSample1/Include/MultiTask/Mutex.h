
#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <assert.h>
#include "APIs/Mutex.h"

class CMutex
{
public:
	inline CMutex(int nType = MUTEX_FAST)
	{
		m_hMutex = 0;
		MutexCreate(&m_hMutex, nType) ;
		assert(m_hMutex);
	};
	inline ~CMutex()
	{
		MutexDestory(m_hMutex);
	};
	inline VD_BOOL Enter()
	{
		assert(m_hMutex);
	
		return MutexEnter(m_hMutex) == 0 ? TRUE : FALSE;
	};
	inline VD_BOOL Leave()
	{
		assert(m_hMutex);

		return MutexLeave(m_hMutex) == 0 ? TRUE : FALSE; 
	};
protected:
private:
	VD_HANDLE m_hMutex;
};

#endif //__MUTEX_H__

//
// End of "$Id: Mutex.h 6920 2007-01-26 02:54:59Z ren_xusheng $"
//
