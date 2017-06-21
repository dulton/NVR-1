#ifndef _DMS_TYPE_H__
#define _DMS_TYPE_H__

#include <stdlib.h>
#include <sys/vfs.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <mntent.h>
#include <sys/stat.h>
//#include <sys/mount.h>
#define SIG_START
/* player for push button */
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <errno.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char       uint8_t;
typedef unsigned int        uint32_t;
typedef unsigned short      uint16_t;

//#define True            			1
//#define False           			0

typedef enum BooL{
	False = 0,
	True = 1
}Bools;


#ifdef _JBDVS_NET_RELEASE
#define err()
#define dbg(fmt, args...)  //yaogang
#else
#define err()\
{             \
	printf(" %s(%d) - %d , %s \n", __FILE__, __LINE__, errno, strerror(errno));\
}

//yaogang
#if 1
#define dbg(fmt, args...) do {					\
		printf("Anni: " fmt " at %s()\n",		\
			##args, __func__);		\
} while (0)
#else
#define dbg(fmt, args...) do {					\
		printf("Anni: " fmt "\nat %s:%d/%s()\n",		\
			##args, __FILE__, __LINE__, __func__);		\
} while (0)
#endif
#endif


#ifdef RECORD
#define NEW_SERVER_VERSION          0x00030202
#define MODIFY_TICK					0x03
//#define SERVERDESCRIPTION     	"JB_VIDEO_NVR"
#else
#ifdef JB_SECOND_GENERATION
#define NEW_SERVER_VERSION          0x00030107
#define MODIFY_TICK					0x06
//#define SERVERDESCRIPTION     	"JB_VIDEO"
#else
#define NEW_SERVER_VERSION          0x00050101
#define MODIFY_TICK					0x05
//#define SERVERDESCRIPTION     	"JB_VIDEO"
#endif
#endif



#define SERVERDESCRIPTION     		"H264"


#ifndef _HAVE_TYPE_BYTE
#define _HAVE_TYPE_BYTE
	typedef unsigned char       BYTE;
#endif

#ifndef _HAVE_TYPE_SBYTE
#define _HAVE_TYPE_SBYTE
	typedef signed char     	SBYTE;
#endif

#ifndef BOOL
#ifndef _HAVE_TYPE_BOOL
#define _HAVE_TYPE_BOOL
	typedef int					BOOL;
#endif
#endif

#ifndef _HAVE_TYPE_WORD
#define _HAVE_TYPE_WORD
	typedef unsigned short      WORD;
#endif

#ifndef _HAVE_TYPE_SWORD
#define _HAVE_TYPE_SWORD
	typedef signed short    	SWORD;
#endif

#ifndef _HAVE_TYPE_DWORD
#define _HAVE_TYPE_DWORD
	typedef unsigned long		DWORD;
	typedef unsigned long		ULONG;
#endif

#ifndef _HAVE_TYPE_ULONGLONG
#define _HAVE_TYPE_ULONGLONG	
	typedef unsigned long long		ULONGLONG;
#endif

#ifndef _HAVE_TYPE_LONG
#define _HAVE_TYPE_LONG
	typedef long				LONG;
#endif

#ifndef _HAVE_TYPE_UINT
#define _HAVE_TYPE_UINT
	typedef unsigned int		UINT;
#endif

#ifndef _HAVE_TYPE_VOID
#define _HAVE_TYPE_VOID
	typedef void				VOID;
#endif

#ifndef _HAVE_TYPE_DWORD64
#define _HAVE_TYPE_DWORD64
	typedef unsigned long long  DWORD64;
	typedef unsigned long long	QWORD;
#endif

#ifndef _HAVE_TYPE_UINT8
#define _HAVE_TYPE_UINT8
	typedef unsigned char   	UINT8;
#endif

#ifndef _HAVE_TYPE_SINT8
#define _HAVE_TYPE_SINT8
	typedef signed char     	SINT8;
#endif

#ifndef _HAVE_TYPE_UINT16
#define _HAVE_TYPE_UINT16
	typedef unsigned short 		UINT16;
#endif

#ifndef _HAVE_TYPE_SINT16
#define _HAVE_TYPE_SINT16
	typedef signed short    	SINT16;
#endif

#ifndef _HAVE_TYPE_UINT32
#define _HAVE_TYPE_UINT32
	typedef unsigned int    	UINT32;
#endif

#ifndef _HAVE_TYPE_SINT32
#define _HAVE_TYPE_SINT32
	typedef signed int      	SINT32;
#endif

#ifndef _HAVE_TYPE_UINT64
#define _HAVE_TYPE_UINT64
	typedef unsigned long long 	UINT64;
#endif

#ifndef _HAVE_TYPE_SINT64
#define _HAVE_TYPE_SINT64
	typedef signed long long	SINT64;
#endif

#ifndef _HAVE_TYPE_HWND
#define _HAVE_TYPE_HWND
	typedef unsigned int 		HWND;
#endif

#ifndef _HAVE_TYPE_WPARAM
#define _HAVE_TYPE_WPARAM
	typedef unsigned int    	WPARAM;
#endif

#ifndef _HAVE_TYPE_LPARAM
#define _HAVE_TYPE_LPARAM
	typedef unsigned long   	LPARAM;
#endif

typedef enum {
    DMS_FALSE    = 0,
    DMS_TRUE     = 1
}DMS_BOOL;



typedef int (* DMS_WNDPROC)(HWND, int, WPARAM, LPARAM);

#ifndef FALSE
#define FALSE       		0
#endif

#ifndef TRUE
#define TRUE        		1
#endif

#ifndef NULL
#define NULL            	((void *)0)
#endif

typedef LONG				LRESULT;
typedef void  				*PVOID;
typedef void  				*LPVOID;
typedef float				FLOAT;
typedef	LONG				*PLONG;
typedef	DWORD				*LPDWORD;
typedef unsigned long       UInt;
typedef signed long       	Int;
typedef char       			Char;
typedef unsigned char       Byte;

#ifndef _HAVE_TYPE_SINT3212
#define _HAVE_TYPE_SINT3212
typedef unsigned char       Bool;
#endif

typedef unsigned long long 	UInt64;
#ifndef _HAVE_TYPE_SINT3212
#define _HAVE_TYPE_SINT3212
typedef signed long long 	Int64;
#endif
typedef unsigned long      	UInt32;
#ifndef _HAVE_TYPE_SINT3212
#define _HAVE_TYPE_SINT3212
typedef signed long      	Int32;
#endif
typedef unsigned short     	UInt16;
#ifndef _HAVE_TYPE_SINT3212
#define _HAVE_TYPE_SINT3212
typedef signed short     	Int16;
#endif
typedef unsigned char      	UInt8;
#ifndef _HAVE_TYPE_SINT3212
#define _HAVE_TYPE_SINT3212
typedef signed char      	Int8;
#endif
typedef float     			Float;
typedef double    			Double;
typedef void      			Void;
typedef Char     			*String;
typedef const Char     		*constString;
typedef UInt32    			Address;
typedef Void     			*Pointer;
typedef const Void     		*constPointer;
typedef UInt32   	 		BSPAddress;
typedef Void     			*BSPPointer;
typedef UInt64   	 		BusAddress;
typedef char 		 		CHAR;
typedef int 		 		SOCKET;

typedef struct sockaddr_in  SOCKADDR_IN;
typedef struct sockaddr		SOCKADDR;

typedef unsigned long		SESSIONHANDLE;
typedef unsigned long		TALKBACKHANDLE;

typedef Void	  			*HANDLE;
typedef unsigned short 		WCHAR;


/**
 * \def LOBYTE(w)
 * \brief Returns the low byte of the word \a w.
 *
 * \sa MAKEWORD
 */
#define LOBYTE(w)           ((BYTE)(w))
/**
 * \def HIBYTE(w)
 * \brief Returns the high byte of the word \a w.
 *
 * \sa MAKEWORD
 */
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

/**
 * \def MAKEWORD(low, high)
 * \brief Makes a word from \a low byte and \a high byte.
 */
#define MAKEWORD(low, high) ((WORD)(((BYTE)(low)) | (((WORD)((BYTE)(high))) << 8)))

/**
 * \def LOWORD(l)
 * \brief Returns the low word of the double word \a l
 *
 * \sa MAKELONG
 */
#define LOWORD(l)           ((WORD)(DWORD)(l))
/**
 * \def HIWORD(l)
 * \brief Returns the high word of the double word \a l
 *
 * \sa MAKELONG
 */
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))

/**
 * \def LOSWORD(l)
 * \brief Returns the low signed word of the double word \a l
 *
 * \sa MAKELONG
 */
#define LOSWORD(l)          ((SWORD)(DWORD)(l))
/**
 * \def HISWORD(l)
 * \brief Returns the high signed word of the double word \a l
 *
 * \sa MAKELONG
 */
#define HISWORD(l)          ((SWORD)((((DWORD)(l)) >> 16) & 0xFFFF))

/**
 * \def MAKELONG(low, high)
 * \brief Makes a double word from \a low word and \a high word.
 */
#define MAKELONG(low, high) ((DWORD)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

#define max(a,b)		(((a) > (b)) ? (a) : (b))
#define min(a,b)		(((a) < (b)) ? (a) : (b))


#ifdef __cpluscplus
#define DEV_INTER_API extern
#else
#define DEV_INTER_API
#endif


#define SAFE_DELETE(x) do{if ((x) != NULL) free (x); (x) = NULL;}while(0)


/*计算数组下标*/
#define ARRAY_INDEX(index, BitNum) ((index) / (BitNum))
/*计算BIT位*/
#define BIT_INDEX(index, BitNum) ((index) % (BitNum))


#define CLR_BIT_NUM(addr, index, BitNum, type) \
		*(addr + ARRAY_INDEX(index, BitNum)) &= \
			(~(((type)0x01) << BIT_INDEX(index, BitNum)))

#define SET_BIT_NUM(addr, index, BitNum, type) \
		*(addr + ARRAY_INDEX(index, BitNum)) |= \
			(((type)0x01) << BIT_INDEX(index, BitNum))

#define CHK_BIT_NUM(addr, index, BitNum, type) \
		(*(addr + ARRAY_INDEX(index, BitNum)) >> BIT_INDEX(index, BitNum)) & ((type)0x01)


#define CLR_BIT_BYTE(addr, index) (CLR_BIT_NUM(addr, index, 8, unsigned char))
#define SET_BIT_BYTE(addr, index) (SET_BIT_NUM(addr, index, 8, unsigned char))
#define CHK_BIT_BYTE(addr, index) (CHK_BIT_NUM(addr, index, 8, unsigned char))

#define CLR_BIT_SHORT(addr, index) (CLR_BIT_NUM(addr, index, 16, unsigned short))
#define SET_BIT_SHORT(addr, index) (SET_BIT_NUM(addr, index, 16, unsigned short))
#define CHK_BIT_SHORT(addr, index) (CHK_BIT_NUM(addr, index, 16, unsigned short))

#define CLR_BIT_LL(addr, index) (CLR_BIT_NUM(addr, index, 64, unsigned long long))
#define SET_BIT_LL(addr, index) (SET_BIT_NUM(addr, index, 64, unsigned long long))
#define CHK_BIT_LL(addr, index) (CHK_BIT_NUM(addr, index, 64, unsigned long long))


#define CLR_BIT_LONG(addr, index) (CLR_BIT_NUM(addr, index, 32, unsigned long))
#define SET_BIT_LONG(addr, index) (SET_BIT_NUM(addr, index, 32, unsigned long))
#define CHK_BIT_LONG(addr, index) (CHK_BIT_NUM(addr, index, 32, unsigned long))

#define CLR_BIT_UNINT(addr, index) (CLR_BIT_NUM(addr, index, 32, unsigned int))
#define SET_BIT_UNINT(addr, index) (SET_BIT_NUM(addr, index, 32, unsigned int))
#define CHK_BIT_UNINT(addr, index) (CHK_BIT_NUM(addr, index, 32, unsigned int))


#define CLR_BIT(addr, index) (CLR_BIT_BYTE(addr, index))
#define SET_BIT(addr, index) (SET_BIT_BYTE(addr, index))
#define CHK_BIT(addr, index) (CHK_BIT_BYTE(addr, index))

/*BIT位转换成字节数*/
#define BITS_TO_BYTES(bits) ((bits) + 7) / 8



#ifdef __cplusplus
}
#endif

#endif  //#ifndef _DMS_TYPE_H__

