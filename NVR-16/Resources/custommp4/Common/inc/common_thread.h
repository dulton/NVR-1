#ifndef _THREAD_H_
#define _THREAD_H_

#include "common_basetypes.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup Thread API 
/// 线程API，封装了不同操作系统的线程对象。
///	\n 调用流程图:
/// \code
///    ===========================
///                |
///           DvrThreadCreate
///                |------------+
///        DvrThreadShareSocket    |
///           DvrThreadGetID       |
///                |------------+
///           DvrThreadExit
///          DvrThreadDestory
///                |
///    ===========================
/// \endcode
/// @{


/// 创建线程。
/// 
/// \param [out] phThread 线程创建成功时，传出的线程句柄。
/// \param [in] pStartAddress 线程函数的地址。
/// \param [in] pParameter 线程函数的参数。
/// \param [in] nPriority 线程优先级，取值0-127，值越小表示优先级越高，会被转化
///        成对应操作系统的优先级。
/// \param [in] dwStatckSize 为线程指定的堆栈大小，如果等于0或者小于必须的值，
///        则使用缺省值。
/// \param [in] pName 线程名称字符串。
/// \retval 0  创建成功
/// \retval <0  创建失败
s32 DvrThreadCreate(HANDLE* phThread,
				 void(* pStartAddress)(void* p),
				 void* pParameter,
				 s32 nPriority,
				 u32 dwStatckSize,
				 const char* pName);


/// 销毁线程。在指定的线程退出前，阻塞调用线程。
/// 
/// \param [in] hThread 指定待销毁线程的句柄。
/// \retval 0  销毁成功
/// \retval <0 销毁失败
s32 DvrThreadDestory(HANDLE* hThread);


/*/// 获得线程的共享套结字。某些操作系统不同的线程不能使用同一个套结字，需要经过转换。
/// 如果可以使用同一个套结字，则直接返回传入的描述符。
/// 
/// \param [in] hThread 指定要使用套接字的线程句柄。
/// \param [in] nSocket 在其他线程中创建的套结字的描述符。
/// \return 共享后的套结字描述符
S32 DvrThreadShareSocket(HANDLE hThread, S32 nSocket);*/


/// 退出调用线程。
void DvrThreadExit(void);


/*/// 返回调用线程的ID。
/// 
/// \return 线程ID
S32 DvrThreadGetID(void);*/

/// @} end of group

#ifdef __cplusplus
}
#endif

#endif //_THREAD_H_
