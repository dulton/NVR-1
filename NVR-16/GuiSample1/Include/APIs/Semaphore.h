

#ifndef __SEMAPHORE_API_H__
#define __SEMAPHORE_API_H__

#include "DVRDEF.H"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup SemaphoreAPI API Semaphore
/// 信号量API，封装了不同操作系统的信号量对象。
///	\n 调用流程图:
/// \code
///    ================================
///                |
///         SemaphoreCreate
///                |------------------+
///    SemaphorePost,SemaphorePend    |
///                |------------------+
///        SemaphoreDestory
///                |
///    ================================
/// \endcode
/// @{


/// 创建信号量。
/// 
/// \param [out] phSemaphore 信号量创建成功时，传出的信号量句柄。
/// \param [in] dwInitialCount 信号量计数的初始值。
/// \retval 0  创建成功
/// \retval <0  创建失败
int SemaphoreCreate(VD_HANDLE *phSemaphore, uint dwInitialCount);


/// 销毁信号量。
/// 
/// \param [in] hSemaphore 信号量句柄。
/// \retval 0  销毁成功
/// \retval <0 销毁失败
int SemaphoreDestory(VD_HANDLE hSemaphore);


/// 减少信号量计数。
/// 
/// \param [in] hSemaphore 信号量句柄。
/// \retval 0  操作成功
/// \retval <0  操作失败
int SemaphorePend(VD_HANDLE hSemaphore);


/// 增加信号量计数。
/// 
/// \param [in] hSemaphore 信号量句柄。
/// \retval 0  操作成功
/// \retval <0  操作失败
int SemaphorePost(VD_HANDLE hSemaphore);

/// @} end of group

#ifdef __cplusplus
}
#endif

#endif //__SEMAPHORE_API_H__

