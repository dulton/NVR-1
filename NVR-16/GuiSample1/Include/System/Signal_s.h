

#define SIGNAL_NODE MACRO_JOIN(TSigNode,SIGNAL_NUMBER)
#define SIGNAL_SIGNAL MACRO_JOIN(TSignal,SIGNAL_NUMBER)
#define SIGNAL_SIGNAL_S MACRO_JOIN(SIGNAL_SIGNAL, _S)
#define SIGNAL_NODE_EMPTY	0x00
#define SIGNAL_NODE_NEW		0x01
#define SIGNAL_NODE_DELETE  0x02
#define SIGNAL_NODE_NORMAL	0x03

#ifdef WIN32
#define INLINE
#else
#define INLINE inline
#endif
     
#include <vector>
#include "MultiTask/Mutex.h"
#include "MultiTask/Semaphore.h"
#include "Object.h"

template <SIGNAL_CLASS_TYPES> class SIGNAL_NODE
{
	typedef void (CObject::*SigProc)(SIGNAL_TYPES);
public:
	SIGNAL_NODE( ) 
		:m_pObj(NULL),m_pProc(NULL),m_Status(SIGNAL_NODE_EMPTY)
	{
	};
	
	INLINE void operator()(SIGNAL_TYPE_ARGS)
	{
		(m_pObj->*m_pProc)(SIGNAL_ARGS);
	};

	CObject * 	m_pObj;
	SigProc	m_pProc;
	int	m_Status;	//added by jili,0x00:Empty; 0x01:New; 0x02:Delete;0x03:Normal
};

template <SIGNAL_CLASS_TYPES> class SIGNAL_SIGNAL
{
public:
	typedef void (CObject::*SigProc)(SIGNAL_TYPES);
	typedef SIGNAL_NODE <  SIGNAL_TYPES > MySignalNode;
private:
	int m_nMaxSlots;
	std::vector < MySignalNode > m_Vector;
	CMutex m_Mutex;
	int calcSize() 
	{
		int i, ret;

		for(i = 0, ret = 0; i < m_nMaxSlots; i++) 
			if(m_Vector[i].m_Status == SIGNAL_NODE_NORMAL) 
				ret ++;
				
		return ret;
	}
		
public:

	int vecSize()
	{
		return calcSize();
	}
	
	SIGNAL_SIGNAL(int MaxSlots) :
	  m_nMaxSlots(MaxSlots), m_Vector(MaxSlots)
	{
		if(MaxSlots < 4) {
			m_nMaxSlots = 4;
			m_Vector.resize(m_nMaxSlots);
		}
	};
	
	~SIGNAL_SIGNAL()
	{
	};

	int Attach(void * pObj, SigProc pProc)
	{
		int i, ret = -1;
		
		m_Mutex.Enter();
		for(i = 0; i < m_nMaxSlots; i++)
		{
			if(m_Vector[i].m_pObj == pObj 
				//&& m_Vector[i].m_pProc == pProc
				&& (memcmp(&m_Vector[i].m_pProc, &pProc, sizeof(SigProc)) == 0) //
				&& m_Vector[i].m_Status == SIGNAL_NODE_NORMAL)
			{
				ret = -2;
				break;
			}
			
			if(m_Vector[i].m_Status == SIGNAL_NODE_EMPTY)
			{
				m_Vector[i].m_pObj   = (CObject*)pObj;
				m_Vector[i].m_pProc  = pProc;
				m_Vector[i].m_Status = SIGNAL_NODE_NORMAL;
				ret = calcSize();
				
				break;
			}
		}
		m_Mutex.Leave();	

		return ret;
	};			

	int Detach(void * pObj, SigProc pProc)
	{
		int i, ret = -1;

		m_Mutex.Enter();
		for(i = 0; i < m_nMaxSlots; i++)
		{
			if(m_Vector[i].m_pObj == pObj 
				//&& m_Vector[i].m_pProc == pProc )
				&& (memcmp(&m_Vector[i].m_pProc, &pProc, sizeof(SigProc)) == 0)) //__TCS__ 
			{
				m_Vector[i].m_Status = SIGNAL_NODE_EMPTY;
				
				ret = calcSize();
				
				break;
			}
		};
		m_Mutex.Leave();	

		return ret;
	};

	int IsAttached(void * pObj, SigProc pProc)
	{
		int i , ret = -1;

		m_Mutex.Enter();
		for(i = 0; i < m_nMaxSlots; i++)
		{
			if(m_Vector[i].m_pObj == pObj 
				//&& m_Vector[i].m_pProc == pProc
				&& (memcmp(&m_Vector[i].m_pProc, &pProc, sizeof(SigProc)) == 0) //__TCS__
				&& m_Vector[i].m_Status == SIGNAL_NODE_NORMAL)
			{
				ret = calcSize();
				break;
			}
		}
		m_Mutex.Leave();

		return ret;
	};

	void operator()(SIGNAL_TYPE_ARGS)
	{
		int i;

		for(i=0; i<m_nMaxSlots; i++)
		{
			if(m_Vector[i].m_Status == SIGNAL_NODE_NORMAL)			//可操作状态
			{
				m_Vector[i](SIGNAL_ARGS);							//call back
			}
		}
	};

	void ClearSignal()
	{
		m_Vector.clear();
	}
};

template <SIGNAL_CLASS_TYPES> class SIGNAL_SIGNAL_S
{
public:
	typedef void (CObject::*SigProc)(SIGNAL_TYPES);
	typedef SIGNAL_NODE <  SIGNAL_TYPES > MySignalNode;
private:
	int m_nMaxSlots;
	std::vector < MySignalNode > m_Vector;
	CMutex m_Mutex;
	bool m_bCalled;
	bool m_bSempore;
	CSemaphore m_sem;
	int calcSize()
	{
		int i, ret;

		for(i = 0, ret = 0; i < m_nMaxSlots; i++)
			if(m_Vector[i].m_Status == SIGNAL_NODE_NORMAL)
				ret ++;

		return ret;
	}

public:

	SIGNAL_SIGNAL_S(int MaxSlots) :
	  m_nMaxSlots(MaxSlots), m_Vector(MaxSlots), m_bCalled(false), m_bSempore(false)
	{
		if(MaxSlots < 4) {
			m_nMaxSlots = 4;
			m_Vector.resize(m_nMaxSlots);
		}
	};

	~SIGNAL_SIGNAL_S()
	{
	};

	int Attach(void * pObj, SigProc pProc)
	{
		int i, ret = -1;

		m_Mutex.Enter();
		for(i = 0; i < m_nMaxSlots; i++)
		{
			if(m_Vector[i].m_pObj == pObj
				//&& m_Vector[i].m_pProc == pProc
				&& (memcmp(&m_Vector[i].m_pProc, &pProc, sizeof(SigProc)) == 0) //
				&& m_Vector[i].m_Status == SIGNAL_NODE_NORMAL)
			{
				ret = -2;
				break;
			}

			if(m_Vector[i].m_Status == SIGNAL_NODE_EMPTY)
			{
				m_Vector[i].m_pObj   = (CObject*)pObj;
				m_Vector[i].m_pProc  = pProc;
				m_Vector[i].m_Status = SIGNAL_NODE_NORMAL;
				ret = calcSize();

				break;
			}
		}
		m_Mutex.Leave();

		return ret;
	};

	int Detach(void * pObj, SigProc pProc)
	{
		int i, ret = -1;

		m_Mutex.Enter();
		for(i = 0; i < m_nMaxSlots; i++)
		{
			if(m_Vector[i].m_pObj == pObj
				//&& m_Vector[i].m_pProc == pProc )
				&& (memcmp(&m_Vector[i].m_pProc, &pProc, sizeof(SigProc)) == 0)) //__TCS__
			{
				m_Vector[i].m_Status = SIGNAL_NODE_EMPTY;

				ret = calcSize();

				break;
			}
		}

		if( m_bCalled )
		{
			m_bSempore = true;

			m_Mutex.Leave();
			m_sem.Pend();
			m_Mutex.Enter();
		}
		m_Mutex.Leave();

		return ret;
	};

	int IsAttached(void * pObj, SigProc pProc)
	{
		int i , ret = -1;

		m_Mutex.Enter();

		for(i = 0; i < m_nMaxSlots; i++)
		{
			if(m_Vector[i].m_pObj == pObj
				//&& m_Vector[i].m_pProc == pProc
				&& (memcmp(&m_Vector[i].m_pProc, &pProc, sizeof(SigProc)) == 0) //__TCS__
				&& m_Vector[i].m_Status == SIGNAL_NODE_NORMAL)
			{
				ret = calcSize();
				break;
			}
		}

		m_Mutex.Leave();

		return ret;
	};

	void operator()(SIGNAL_TYPE_ARGS)
	{
		int i;

		m_Mutex.Enter();
		m_bCalled = true;
		m_Mutex.Leave();

		for(i=0; i<m_nMaxSlots; i++)
		{
			if(m_Vector[i].m_Status == SIGNAL_NODE_NORMAL)			//可操作状态
			{
				m_Vector[i](SIGNAL_ARGS);							//call back
			}
		}

		m_Mutex.Enter();
		m_bCalled = false;
		if(m_bSempore)
		{
			m_bSempore = false;
			m_sem.Post();
		}
		m_Mutex.Leave();
	};
};

#undef INLINE
#undef SIGNAL_NODE
#undef SIGNAL_SIGNAL
#undef SIGNAL_SIGNAL_S
#undef SIGNAL_NODE_EMPTY	
#undef SIGNAL_NODE_NEW		
#undef SIGNAL_NODE_DELETE  
#undef SIGNAL_NODE_NORMAL	

