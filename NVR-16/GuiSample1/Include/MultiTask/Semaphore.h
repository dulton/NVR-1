
#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <assert.h>
#include "APIs/Semaphore.h"

class CSemaphore
{
public:
	inline CSemaphore(uint dwInitialCount = 0)
		:m_hSemphore(0)
	{
		SemaphoreCreate(&m_hSemphore, dwInitialCount);
		assert(m_hSemphore);
	};
	inline ~CSemaphore()
	{
		assert(m_hSemphore);
		SemaphoreDestory(m_hSemphore);
	};
	inline int Pend()
	{
		assert(m_hSemphore);
		return SemaphorePend(m_hSemphore);
	};
	inline int Post()
	{
		assert(m_hSemphore);
		return SemaphorePost(m_hSemphore);
	};
protected:
private:
	VD_HANDLE m_hSemphore;
};


#endif //__SEMAPHORE_H__
//
// End of "$Id: Semaphore.h 6920 2007-01-26 02:54:59Z ren_xusheng $"
//
