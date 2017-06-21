#ifndef _IFLY_TYPE_H_
#define _IFLY_TYPE_H_

#ifdef WIN32
#define PACK_NO_PADDING
#else
#define PACK_NO_PADDING  __attribute__ ((__packed__)) //wrchen 090206
#endif

typedef int BOOL;
typedef int BOOL32;

//typedef char				s8;
typedef short				s16;
typedef int					s32;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;

#ifdef _MSC_VER
typedef __int64				s64;
typedef unsigned __int64	u64;
#else
typedef long long			s64;
typedef unsigned long long	u64;
#endif

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

#endif

