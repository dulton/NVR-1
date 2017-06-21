#ifndef _MOD_COMMON_H_
#define _MOD_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>

/*
typedef char 				s8;
typedef short 				s16;
typedef int 				s32;
typedef long long 			s64;
typedef unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned int 		u32;
typedef unsigned long long 	u64;
*/

enum EMERRORCODE					//函数返回错误码枚举
{
	EM_REC_SUCCESS = 0,				//成功
	EM_REC_INVALID_PARA = 1000,		//无效参数
	EM_REC_SEMINIT_FAILED, 			//sem_init error
	EM_REC_MALLOC_ERR,				//malloc失败
	EM_REC_BUF_NO_SPACE,			//写buf时数据太长，buf剩余空间不足
	EM_REC_BUF_NO_DATA,				//读buf时，要读取的长度大于buf内已存数据的总长
	EM_REC_RECORDCREATEBUF,			//创建录像缓冲失败
	EM_REC_RECORDDESTROYBUF,		//销毁录像缓冲失败
	EM_REC_FRAMETOOBIG,				//帧数据太长，缓冲区放不下了
	EM_REC_NODISK,					//没有硬盘，不进行初始化
	EM_REC_HASNOTINIT,				//还没有执行过初始化
	EM_REC_HASBEENINIT,				//已经执行过初始化
	EM_REC_HASBEENDEINIT,			//已经执行过反初始化
	EM_REC_CANNOTOPENNEWFILE,		//不能打开/创建一个新文件用于录像
	EM_REC_ERRGETRECPATH,			//获得录像文件路径错误
	EM_REC_ERROPENNEWFILE,			//打开文件失败
};

#ifndef PARAOUT
#define PARAOUT
#endif

/**************************旧的头文件需要******************************/
#define PACK_NO_PADDING  __attribute__ ((__packed__)) 
typedef int BOOL;
typedef int BOOL32;

typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef unsigned int		UINT32;
#ifdef WIN32
typedef unsigned __int64	UINT64;
#else
typedef unsigned long long	UINT64;
#endif

typedef char				INT8;
typedef short				INT16;
typedef	int				    INT32;
#ifdef WIN32
typedef __int64				INT64;
#else
typedef long long			INT64;
#endif

#ifndef _MSC_VER
#ifndef LPSTR
#define LPSTR   char *
#endif
#ifndef LPCSTR
#define LPCSTR  const char *
#endif
#endif

#ifndef NULL
#define NULL			0
#endif

#ifndef TRUE
#define TRUE			1
#endif

#ifndef FALSE
#define FALSE			0
#endif

#ifndef SUCCESS
#define SUCCESS         0
#endif

#ifndef FAILURE
#define FAILURE         -1
#endif

#ifndef WIN32
typedef unsigned int	DWORD;
typedef unsigned short	WORD;
typedef unsigned char	BYTE;
#endif
/**************************end******************************/

#endif //_MOD_COMMON_H_
