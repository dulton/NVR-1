



#include "APIs/DVRDEF.H"
// #include "dvr_inc.h"
// #include "dvr_dev.h"
// #include "dvr_debug.h"
#include "APIs/Mutex.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef WIN32
#include <pthread.h>
#endif

#define TS_LOCK(lock)	pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*)lock );	pthread_mutex_lock( lock )

#define TS_UNLOCK(lock)	pthread_cleanup_pop( 1 )

void *ts_malloc(size_t s)
{
	void *			addr;

	static pthread_mutex_t	malloc_lock = PTHREAD_MUTEX_INITIALIZER;

	TS_LOCK(&malloc_lock);
	addr = malloc(s);
	TS_UNLOCK(&malloc_lock);

	return(addr);

} 


void ts_free(void *addr)
{
	static pthread_mutex_t	free_lock = PTHREAD_MUTEX_INITIALIZER;

	TS_LOCK(&free_lock);
	free(addr);
	TS_UNLOCK(&free_lock);

} 

//#ifdef REC_MUTEX
#if 1

int MutexCreate(HANDLE *phMutex,int nType)
{
	int ret;	
	pthread_mutex_t *lock;
	pthread_mutexattr_t attr;


	lock = ts_malloc(sizeof(pthread_mutex_t));
	if (lock == NULL)
	{
		ERR_PRINT("No Mem\n");
		return -ENOMEM;
	}
	
	switch (nType) 
	{
		case MUTEX_FAST:
		{
			ret = pthread_mutex_init(lock, NULL);
			if( ret != 0 )
			{
				ts_free(lock);
				errno = ret;
				ERR_PRINT("Pthread Mutex Init: %s\n", strerror(ret));
				*phMutex = (HANDLE)(NULL);
				return -ret;
			}
			break;
		}
		case MUTEX_RECURSIVE:
		{
    		if ((ret = pthread_mutexattr_init(&attr)) != 0)
    		{
     			ts_free(lock);
    			ERR_PRINT("Pthread_mutexattr_init Failed\n");	
    			return -ENOMEM;
    		}
     		
		    if ((ret = pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP)) != 0)
		    {
		    	ts_free(lock);
		    	ERR_PRINT("Can't Set Recursive Type\n");
		    	*phMutex = (HANDLE)(NULL);
		    	return -ret;
		    }
		    
		    if ((ret = pthread_mutex_init(lock, &attr)) != 0)
		    {
		    	ERR_PRINT("Can't Create Recursive Mutex\n");
		    	*phMutex = (HANDLE)(NULL);
		    	return -ret;
		    }
		}
		break;
		default:
		{
			ret = pthread_mutex_init(lock, NULL);
			if( ret != 0 )
			{
				ts_free(lock);
				errno = ret;
				ERR_PRINT("Pthread Mutex Init: %s\n", strerror(ret));
				*phMutex = (HANDLE)(NULL);
				return -ret;
			}
			break;
		}
	}
	
	*phMutex = (HANDLE)lock;
	
	return 0;
}

#else
//#warning "Not Use recursive mutex!"


//int MutexCreate(HANDLE* phmutex)
int MutexCreate(HANDLE *phmutex,int nType)
{
	int			ret;
	pthread_mutex_t *	lock;
	
	lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (lock)
	{
		ret = pthread_mutex_init(lock, NULL);
		if (ret != 0)
		{
			free(lock);
			errno	= ret;
			printf("Pthread Mutex Init\n");
			*phmutex	= (HANDLE)(NULL);
			return -ret;
		}
	
		*phmutex = (HANDLE)lock;
	
		return 0;
	}
	
	printf(("MutexCreate No Mem\n"));
	return -1;
} 
#endif


int MutexDestory(HANDLE hmutex)
{
	pthread_mutex_destroy((pthread_mutex_t *)hmutex);

	free((void*)hmutex);
	return 0;
} 


int MutexEnter(HANDLE hmutex)
{
  return -pthread_mutex_lock((pthread_mutex_t *)hmutex);
} 


int MutexLeave(HANDLE hmutex)
{
  return -pthread_mutex_unlock((pthread_mutex_t *)hmutex);
} 



#if 0
int RWMutexCreate(HANDLE* phmutex)
{
	int			ret;
	pthread_rwlock_t *lock;
	
	lock = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t));
	if (lock)
	{
		ret = pthread_rwlock_init(lock, NULL);
		if (ret != 0)
		{
			free(lock);
			errno	= ret;
			printf("Pthread Mutex Init\n");
			*phmutex	= (HANDLE)(NULL);
			return -ret;
		}
	
		*phmutex = (HANDLE)lock;
	
		return 0;
	}
	
	return -1;
} 


int RWMutexDestory(HANDLE hmutex)
{
	pthread_rwlock_destroy((pthread_rwlock_t *)hmutex);

	free((void*)hmutex);
	return 0;
} 


int WriteMutexEnter(HANDLE hmutex)
{
  return -pthread_rwlock_wrlock((pthread_rwlock_t *)hmutex);
} 

int ReadMutexEnter(HANDLE hmutex)
{
  return -pthread_rwlock_rdlock((pthread_rwlock_t *)hmutex);
} 


int WriteMutexLeave(HANDLE hmutex)
{
  return -pthread_rwlock_unlock((pthread_rwlock_t *)hmutex);
} 

int ReadMutexLeave(HANDLE hmutex)
{
  return -pthread_rwlock_unlock((pthread_rwlock_t *)hmutex);
}
#endif 
