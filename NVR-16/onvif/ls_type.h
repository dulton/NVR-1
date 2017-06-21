/******************************************************************************
 Copyright (C), 2001-2013
******************************************************************************
File Name     : type.h
Author        : ls
Created       : 2013/7/16
Last Modified :
Description   : The common data type defination
Function List :
History       :
1.Date        : 2013/07/16
  Author      : ls
  Modification: create


  
******************************************************************************/
#ifndef __TYPE_H__
#define __TYPE_H__


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


#define __DEBUG__   //DEBUG switch

#if defined(__WIN32__) || defined(_WIN32)
//#ifndef _WIN32

#else
#ifdef __DEBUG__ //
#define lsprint(fmt,args...) printf(fmt, ##args);
#else
#define lsprint(fmt,args...)
#endif
#endif


/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

		/* LIMIT MACROS */

#ifndef INT8_MIN
#define _C2			1	/* 0 if not 2's complement */
#define INT8_MIN	(-0x7f - _C2)
#define INT16_MIN	(-0x7fff - _C2)
#define INT32_MIN	(-0x7fffffff - _C2)

#define INT8_MAX	0x7f
#define INT16_MAX	0x7fff
#define INT32_MAX	0x7fffffff
#define UINT8_MAX	0xff
#define UINT16_MAX	0xffff
#define UINT32_MAX	0xffffffff
#define INT64_MIN	(-0x7fffffffffffffffLL - _C2)
#define INT64_MAX	0x7fffffffffffffffLL
#define UINT64_MAX	0xffffffffffffffffLL
#define SIZE_MAX      0xffffffff    /* maximum unsigned int value */
#endif 

typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;

typedef signed char             sint8_t;
typedef short                   sint16_t;
typedef int                     sint32_t;


#ifndef _M_IX86                            //_M_IX86 : 32bit processor
    typedef unsigned long long  uint64_t;
    typedef long long           sint64_t;
#else
    typedef __int64             uint64_t;
    typedef __int64             sint64_t;
#endif


//typedef char                    char;
#define LS_VOID                 void


/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
#ifdef __cplusplus


typedef bool Boolean;
#define False false
#define True true

#else
typedef unsigned char Boolean;
#ifndef __MSHTML_LIBRARY_DEFINED__
#ifndef False
const Boolean False = 0;
#endif
#ifndef True
const Boolean True = 1;
#endif
#endif
#endif

#ifndef NULL
    #define NULL    0L
#endif

#define LS_NULL     0L
#define LS_SUCCESS  0
#define LS_FAILURE  (-1)


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __TYPE_H__ */
