

#ifndef __MUTEX_API_H__
#define __MUTEX_API_H__

#include "DVRDEF.H"

#ifdef WIN32

#include <stddef.h>
/*----------------------------------------------------------------------*\
* Exported data structures are completely opaque.
* To determine the size the structures, use the given sizeof functions.
\*----------------------------------------------------------------------*/
typedef void pthread_mutex_t;
typedef void pthread_mutexattr_t;
size_t pthread_mutex_sizeof(void);

typedef void pthread_cond_t;
typedef void pthread_condattr_t;
size_t pthread_cond_sizeof(void);

typedef unsigned long pthread_t;
typedef void pthread_attr_t;

typedef void *(pthread_start_t)(void *arg);

/*----------------------------------------------------------------------*\
* Implemented API functions.
\*----------------------------------------------------------------------*/
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_create(pthread_t *id, pthread_attr_t *attr, 
				   pthread_start_t *start, void *arg);
pthread_t pthread_self(void);
int pthread_equal(pthread_t t1, pthread_t t2);

/*----------------------------------------------------------------------*\
* They are just present to avoid compile errors and memory leaks
* in luathreads under Win32.
\*----------------------------------------------------------------------*/
int pthread_detach(pthread_t th);
int pthread_cleanup(pthread_t th);

/*======================================================================*\
* Pthreads stuff.
\*======================================================================*/
#else 

struct timespec;

#include <pthread.h>

#define pthread_mutex_sizeof() (sizeof(pthread_mutex_t))
#define pthread_cond_sizeof() (sizeof(pthread_cond_t))
int pthread_cleanup(pthread_t th);

#endif // #ifndef LINUX


#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup MutexAPI API Mutex
/// 互斥量API，封装了不同操作系统的互斥量对象。
///	\n 调用流程图:
/// \code
///    ===========================
///                |
///           MutexCreate
///                |------------+
///      MutexEnter,MutexLeave  |
///                |------------+
///           MutexDestory
///                |
///    ===========================
/// \endcode
/// @{

/// 互斥量类型
enum mutex_type
{
	/// 快速互斥量，如果临界区已经被一个线程获取，其他线程或者该线程再次获取时，
	/// 调用线程直接被挂起。
	MUTEX_FAST = 0,

	/// 递归互斥量，已经获取临界区的线程可以多次获取，互斥量对象内部应有获取次数
	/// 计数。如果要完全释放临界区，一定要保证获取次数和释放次数一致。
	MUTEX_RECURSIVE,
};


/// 创建互斥量。
/// 
/// \param [out] phMutex 互斥量创建成功时，传出的互斥量句柄。
/// \param [in] nType 互斥量的类型，取mutex_type枚举类型值
/// \retval 0  创建成功
/// \retval <0  创建失败
int MutexCreate(VD_HANDLE *phMutex, int nType);


/// 销毁互斥量。
/// 
/// \param [in] hMutex 互斥量句柄。
/// \retval 0  销毁成功
/// \retval <0 销毁失败
int MutexDestory(VD_HANDLE hMutex);


/// 进入临界区。
/// 
/// \param [in] hMutex 互斥量句柄。
/// \retval 0  操作成功
/// \retval <0  操作失败
int MutexEnter(VD_HANDLE hMutex);


/// 离开临界区。
/// 
/// \param [in] hMutex 互斥量句柄。
/// \retval 0  操作成功
/// \retval <0  操作失败
int MutexLeave(VD_HANDLE hMutex);

/// @} end of group

#ifdef __cplusplus
}
#endif

#endif //__MUTEX_API_H__

