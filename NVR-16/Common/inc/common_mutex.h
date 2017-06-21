#ifndef _MUTEX_API_H_
#define _MUTEX_API_H_

#include "common_basetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup MutexAPI API Mutex
/// 互斥量API，封装了不同操作系统的互斥量对象。
///	\n 调用流程图:
/// \code
///    ===========================
///                |
///           DvrMutexCreate
///                |------------+
///      DvrMutexLock,DvrMutexUnlock|
///                |------------+
///           DvrMutexDestory
///                |
///    ===========================
/// \endcode
/// @{

/*/// 互斥量类型
enum mutex_type
{
	/// 快速互斥量，如果临界区已经被一个线程获取，其他线程或者该线程再次获取时，
	/// 调用线程直接被挂起。
	MUTEX_FAST = 0,

	/// 递归互斥量，已经获取临界区的线程可以多次获取，互斥量对象内部应有获取次数
	/// 计数。如果要完全释放临界区，一定要保证获取次数和释放次数一致。
	MUTEX_RECURSIVE,
};*/


/// 创建互斥量。
/// 
/// \param [out] phMutex 互斥量创建成功时，传出的互斥量句柄。
/// \param [in] nType 互斥量的类型，取mutex_type枚举类型值
/// \retval 0  创建成功
/// \retval <0  创建失败
s32 DvrMutexCreate(HANDLE* phMutex);


/// 销毁互斥量。
/// 
/// \param [in] hMutex 互斥量句柄。
/// \retval 0  销毁成功
/// \retval <0 销毁失败
s32 DvrMutexDestory(HANDLE* hMutex);


/// 进入临界区。
/// 
/// \param [in] hMutex 互斥量句柄。
/// \retval 0  操作成功
/// \retval <0  操作失败
s32 DvrMutexLock(HANDLE* hMutex);


/// 离开临界区。
/// 
/// \param [in] hMutex 互斥量句柄。
/// \retval 0  操作成功
/// \retval <0  操作失败
s32 DvrMutexUnlock(HANDLE* hMutex);

/// @} end of group

#ifdef __cplusplus
}
#endif

#endif //__MUTEX_API_H__

