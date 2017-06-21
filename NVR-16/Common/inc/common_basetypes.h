#ifndef _BASE_TYPES_H_
#define _BASE_TYPES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifndef _IFLY_TYPE_H_

typedef char s8;
typedef unsigned char u8;

typedef short s16;
typedef unsigned short u16;

typedef int s32;
typedef unsigned int u32;

#ifdef _MSC_VER
typedef __int64				s64;
typedef unsigned __int64	u64;
#else
typedef long long			s64;
typedef unsigned long long	u64;
#endif

#endif

#ifndef WIN32
typedef void *HANDLE;
#endif

#ifndef TRUE
#define TRUE				1
#endif

#ifndef FALSE
#define FALSE				0
#endif

#ifndef BOOL
#define BOOL int
#endif

typedef struct
{
    u8 nR;
    u8 nG;
    u8 nB;
    u8 nTrans;
} SRgb;

#define PARAOUT
#ifdef WIN32
#define PACK_NO_PADDING
#else
#define PACK_NO_PADDING  __attribute__ ((__packed__))
#endif

#endif

