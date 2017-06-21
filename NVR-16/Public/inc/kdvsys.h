/*****************************************************************************
   模块名      : KDV system
   文件名      : kdvsys.h
   相关文件    : 
   文件实现功能: KDV公共函数及宏定义
   作者        : 李屹
   版本        : V0.9  Copyright(C) 2001-2002 KDC, All rights reserved.
-----------------------------------------------------------------------------
   修改记录:
   日  期      版本        修改人      修改内容
   2002/01/30  0.9         李屹        创建
******************************************************************************/
#ifndef _KDV_SYS_H_
#define _KDV_SYS_H_

/*#include "osp.h"*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "unicodedef.h"

#ifndef WIN32
/*#include <windows.h>*/
#define min(a,b) (a>b ? b : a)
#endif

#ifdef WIN32
/*    #ifdef KDVSYS_EXPORTS*/
        #define DllExport __declspec(dllexport)
/*    #else                  
        #define DllExport __declspec(dllimport)
    #endif*/
#else            /* VxWorks*/
    #define DllExport
#endif

#ifdef _LINUX_
#define min(a,b) (a>b ? b : a)
#endif

/* 定义回车 */
#if defined(WIN32) || defined(_LINUX_)
    #define     STR_RETURN          (LPCSTR)"\r\n"
#elif defined( VXWORKS_SIMU )
    #define     STR_RETURN          (LPCSTR)"\r\n"
#else            /* VxWorks */
    #define     STR_RETURN          (LPCSTR)"\n"
#endif

/* SNMP拷贝FTP文件分割符 */
#define CHAR_FTPFILE_SEPARATOR		(char)'|'	/* 不同文件之间分隔符 */
#define CHAR_FTPFILETYPE_SEPARATOR	(char)':'	/* 文件名和类型之间分隔符 */
#define CHAR_EXE_TYPE				(char)'E'	/* 二进制文件类型 */
#define CHAR_CFG_TYPE				(char)'C'	/* 配置文件类型 */
#define CHAR_RES_TYPE				(char)'R'   /* 资源文件类型 */

/* 定义Profile文件中表格数据相关字符串常量 */
#define STR_ENTRY_NUM			(LPCSTR)"EntryNum"
#define STR_ENTRY				(LPCSTR)"Entry"
#define SECTION_ipRouteTable	(LPCSTR)"ipRouteTable"
#define FIELD_ipRouteDest		(LPCSTR)"ipRouteDest"
#define FIELD_ipRouteNextHop	(LPCSTR)"ipRouteNextHop"
#define FIELD_ipRouteMask		(LPCSTR)"ipRouteMask"

/* OID length */
#define OID_IP_ADDR_LEN     4 
#define OID_ID_LEN          1 

/* maximum size of physical address */
#define MAXSIZE_PHYADDR     16

/* Type definition */
#ifndef BOOL
#define BOOL	int
#endif
#ifndef BYTE
#define BYTE	unsigned char
#endif
#ifndef UCHAR
#define UCHAR	unsigned char
#endif
#ifndef WORD
#define WORD	unsigned short
#endif

#ifndef __H264__
#ifndef DWORD
#define DWORD	unsigned long
#endif
#else
#ifndef DWORD
#define DWORD	unsigned int
#endif
#endif

#ifndef WORD16
#define WORD16	WORD
#endif

//#ifndef WORD32
//#define WORD32	DWORD
//#endif

#ifndef __H264__
#ifndef WORD32
#define WORD32	unsigned long
#endif
#else
#ifndef WORD32
#define WORD32	unsigned int
#endif	
#endif

#ifndef SWORD
#define SWORD	short
#endif
#ifndef SDWORD
#define SDWORD	long
#endif
#ifndef UINT8
#define UINT8	BYTE
#endif
#ifndef UINT16
#define UINT16	WORD
#endif
#ifndef LPSTR
#define LPSTR   char *
#endif
#ifndef LPCSTR
#define LPCSTR  const char *
#endif
#ifndef UINT64
#define UINT64  unsigned long long
#endif


typedef UCHAR   MAC_TYPE[6];

//#ifndef _LINUX_
/* Value definition */
#ifndef TRUE
#define TRUE    (BOOL)1
#endif
#ifndef FALSE
#define FALSE   (BOOL)0
#endif
//#endif //_LINUX_

/* 路径定义 */
#if defined( WIN32 )    /* WIN32 */
#define PATH_FTP            (LPCSTR)"ftp"
#define PATH_CONFIG         (LPCSTR)"conf"
#define PATH_BIN			(LPCSTR)"."
#define PATH_RES			(LPCSTR)"res"
#elif defined( _LINUX_ )
#define PATH_FTP            (LPCSTR)"ftp"
//#define PATH_CONFIG         (LPCSTR)"conf"
#define PATH_CONFIG         (LPCSTR)"config"
#define PATH_BIN			(LPCSTR)"."
#define PATH_RES			(LPCSTR)"res"
#elif defined( _VXWORKS_ )
#define PATH_FTP            (LPCSTR)"/ftp"
#define PATH_CONFIG         (LPCSTR)"/conf"
#define PATH_BIN			(LPCSTR)""
#define PATH_RES			(LPCSTR)"res"
#endif

/* semTake超时 */
#define WAIT_SEM_TIMEOUT    2000    /* ticks */

/* SNMP PORT macro */
#define  SNMP_PORT          1161         /* port for snmp */
#define  TRAP_PORT          162         /* port for snmp traps */

/* SNMP TRAP自定义specificTrap */
#define SPECIFICTRAP_COLD_RESTART		1L
#define SPECIFICTRAP_POWERON			1L
#define SPECIFICTRAP_POWEROFF			2L
#define SPECIFICTRAP_ALARM_GENERATE     3L
#define SPECIFICTRAP_ALARM_RESTORE      4L

/* PrintMsg() type取值 */
#define MESSAGE_INFO            (BYTE)0x1
#define MESSAGE_WARNING         (BYTE)0x2
#define MESSAGE_ERROR           (BYTE)0x3

#define MESSAGE_BUFFER_LEN      (WORD)1024   /* 消息缓冲的最大长度 */

#ifndef MAX_PATH
    /* 定义最大路径长度 */
    #define MAX_PATH            (WORD)256
#endif


#ifndef MAX_LINE_LEN
    /* 定义一行的最大长度 */
    #define MAX_LINE_LEN        (WORD)1024*3
#endif

#define MAX_SECTIONNAME_LEN     (WORD)50
#define MAX_KEYNAME_LEN         (WORD)50
#define MAX_VALUE_LEN           (WORD)255

/* OpenProfile() byOpenMode 取值 */
#define PROFILE_READONLY        (BYTE)0x1       /*只读*/
#define PROFILE_WRITE           (BYTE)0x2       /*读写*/
#define PROFILE_READWRITE       PROFILE_WRITE   /*读写*/
#define PROFILE_WRITEONLY       (BYTE)0x4       /*只写*/

#define PROFILE_MAX_OPEN        (WORD)20        /* 打开文件的最大数目 */

typedef DWORD                   tKeyHandle;

#define INVALID_KEYHANDLE       (WORD)0xFFFF    /* 无效的文件句柄 */

/* Profile文件注释符定义 */
#define PROFILE_COMMENT1        (LPCSTR)";"
#define PROFILE_COMMENT2        (LPCSTR)"//"
#define PROFILE_COMMENT3        (LPCSTR)"--"
#define PROFILE_COMMENT4        (LPCSTR)"#"

/* struct definition */
typedef struct
{
    WORD    dwAddrLen;                      /* Length of address */
    UCHAR   phyAddress[MAXSIZE_PHYADDR];    /* physical address value */
} PHYADDR_STRUCT;

typedef struct
{
    UCHAR   byDtiNo;
    UCHAR   byE1No;
} MCU_E1_STRUCT;

typedef struct
{
    UCHAR   byDtiNo;
    UCHAR   byE1No;
    UCHAR   byTsNo;
} MCU_TS_STRUCT;

typedef struct
{
    DWORD   dwTsNumber;                 /* TS number */
    MCU_TS_STRUCT   *ptMcuTs;           /* MCU TS array */
} MCU_TS_ARRAY_STRUCT;

typedef struct
{
    UCHAR   byE1No;
    UCHAR   byTsNo;
} MT_TS_STRUCT;

/* -----------------------  系统公用函数  ----------------------------- */
#define LOWORD16(l)     ((WORD16)(l))
#define HIWORD16(l)     ((WORD16)(((WORD32)(l) >> 16) & 0xFFFF))
#define MAKEDWORD(l,h)  ((WORD32)(((WORD)(l)) | ((WORD32)((WORD)(h))) << 16))

#ifndef _WINDEF_  /* 为了能够有Windows下使用 */
    #define LOBYTE(w)       ((BYTE)(w))
    #define HIBYTE(w)       ((BYTE)(((WORD16)(w) >> 8) & 0xFF))
    #define MAKEWORD(l,h)  ((WORD)(((BYTE)(l)) | ((WORD)((BYTE)(h))) << 8))
#endif

#ifdef _DEBUG 
	#undef ASSERT
	#define ASSERT(exp)    \
        { \
	      if ( !( exp ) ) \
		  { \
            printf((LPSTR)"Assert File %s, Line %d (%s)\n", \
	                 __FILE__, __LINE__, (LPSTR)#exp ); \
		  } \
        }
#else
    #undef ASSERT
    #define ASSERT( exp )    {}
#endif

/* 读取配置文件类函数 */
DllExport BOOL GetRegKeyInt( LPCSTR lpszProfileName,    /* 文件名（含绝对路径）*/
                   LPCSTR lpszSectionName,      /* Profile中的段名   */
                   LPCSTR lpszKeyName,          /* 段内的关键字名    */
                   const SDWORD sdwDefault,     /* 失败时返回的默认值*/
                   SDWORD  *sdwReturnValue      /* 返回值            */
                 );
    /* 从配置文件中读取一整数型值 */

DllExport BOOL GetRegKeyStringTable( LPCSTR lpszProfileName,    /* 文件名（含绝对路径）*/
                   LPCSTR lpszSectionName,      /* Profile中的段名   */        
                   LPCSTR lpszDefault,          /* 失败时返回的默认值*/
                   LPSTR *lpszEntryArray,       /* 返回字符串数组指针 */
                   DWORD *dwEntryNum,           /* 字符串数组数目，成功后返回字符串数组实际数
                                                目，如传入的数目不够则多余的记录不予传回 */
                   DWORD dwBufSize              /* 返回字串的长度，如不够则截断，最
                                                后一个字节填上'\0'    */
                 );
    /* 从配置文件中读取一张表里的若干行数据
 */

DllExport BOOL GetRegKeyString( LPCSTR lpszProfileName, /* 文件名（含绝对路径）*/
                      LPCSTR lpszSectionName,   /* Profile中的段名   */
                      LPCSTR lpszKeyName,       /* 段内的关键字名    */
                      LPCSTR lpszDefault,       /* 失败时返回的默认值*/
                      LPSTR lpszReturnValue,    /* 返回字串          */
                      DWORD dwBufSize           /* 返回字串的长度，如不够则截断，最
                                                后一个字节填上'\0'    */
                    );
    /* 从配置文件中读取一字符串型值 */
    
DllExport BOOL GetRegKeyString2( FILE *stream,  /* 文件句柄 */
                      LPCSTR lpszSectionName,   /* Profile中的段名   */
                      LPCSTR lpszKeyName,       /* 段内的关键字名    */
                      LPCSTR lpszDefault,       /* 失败时返回的默认值*/
                      LPSTR lpszReturnValue,    /* 返回字串          */
                      DWORD dwBufSize           /* 返回字串的长度，如不够则截断，最
                                                后一个字节填上'\0'    */
                    );
    /* 根据文件句柄从配置文件中读取一字符串值，该文件需以"rb"方式打开 */
    
DllExport BOOL SetRegKeyInt( LPCSTR lpszProfileName,    /* 文件名（含绝对路径）*/
                   LPCSTR lpszSectionName,      /* Profile中的段名   */
                   LPCSTR lpszKeyName,          /* 段内的关键字名    */
                   const SDWORD  sdwValue       /* 新的整数值        */
                 );
    /* 往配置文件中写入一新的整数型的值，如果不存在则自动创建 */

DllExport BOOL SetRegKeyString( LPCSTR lpszProfileName, /* 文件名（含绝对路径）*/
                      LPCSTR lpszSectionName,   /* Profile中的段名   */
                      LPCSTR lpszKeyName,       /* 段内的关键字名    */
                      LPCSTR lpszValue          /* 新的字串值, NULL则表示删除改行 */
                    );
    /* 往配置文件中写入一新的字符串型的值，如果不存在则自动创建 */

#if 0//csp modify
DllExport BOOL DeleteRegKey( LPCSTR lpszProfileName,    /* 文件名（含绝对路径）*/
                      LPCSTR lpszSectionName,   /* Profile中的段名   */
                      LPCSTR lpszKeyName        /* 段内的关键字名    */
                    );
    /* 从配置文件中删除一行Key */
#endif

//ljl add
char* GetTableRegKeyString( LPCSTR lpszProfileName, 
									LPCSTR lpszSectionName, 
									BYTE   arossCoordinate, 
									BYTE   verticalCoordinate,
									LPSTR  lpszValue
									);

DllExport BOOL SetTableRegKeyString( LPCSTR lpszProfileName,   // 文件名（含绝对路径）   
							LPCSTR lpszSectionName,   // Profile中的段名
							LPCSTR lpszLineName,
							BYTE   arossCoordinate,   // 要设置的新的值在水平方向的位置。
							BYTE   verticalCoordinate, //要设置的新的值在垂直方向的位置。
							LPCSTR lpszValue        //新的字串值, NULL则表示删除改行
							);

DllExport BOOL SetTableRegKeyInt( LPCSTR lpszProfileName, // 文件名（含绝对路径）   
								 LPCSTR lpszSectionName,  // Profile中的段名
								 LPCSTR lpszLineName,
								 BYTE   arossCoordinate,  // 要设置的新的值在水平方向的位置。
								 BYTE   verticalCoordinate, //要设置的新的值在垂直方向的位置。
								 const SDWORD sdwValue    //新的整数值, NULL则表示删除改行
								 );
//ljl add

//yzw add
DllExport BOOL GetRegKeyU64( LPCSTR lpszProfileName, 
                   LPCSTR lpszSectionName, 
                   LPCSTR lpszKeyName, 
                   UINT64  sdwDefault,
                   UINT64  *sdwReturnValue 
                 );

DllExport BOOL SetRegKeyU64( LPCSTR lpszProfileName,
				  LPCSTR lpszSectionName, 
				  LPCSTR lpszKeyName, 
				  const UINT64 sdwValue   
				);

DllExport BOOL GetStringFromTableWithKey( LPCSTR lpszProfileName, 
									LPCSTR lpszSectionName, 
									LPCSTR lpszKeyName,          /* 段内的关键字名    */
                  					WORD   verticalCoordinate,
									LPSTR  lpszValue,
									DWORD dwBufSize
									);
									
DllExport BOOL GetValueFromTableWithKey( LPCSTR lpszProfileName, 
									LPCSTR lpszSectionName, 
									LPCSTR lpszKeyName,          /* 段内的关键字名    */
                  					WORD   verticalCoordinate,
									SDWORD  *sdwReturnValue 
									);

DllExport BOOL GetU64ValueFromTableWithKey( LPCSTR lpszProfileName, 
									LPCSTR lpszSectionName, 
									LPCSTR lpszKeyName,          /* 段内的关键字名    */
                  					WORD   verticalCoordinate,
									UINT64 *sdwReturnValue 
									);
									
DllExport BOOL SetStringToTableWithKey( LPCSTR lpszProfileName, 
									LPCSTR lpszSectionName, 
									LPCSTR lpszKeyName,          /* 段内的关键字名    */
                 					WORD   verticalCoordinate,
									LPSTR  lpszValue
									);
									
DllExport BOOL SetValueToTableWithKey( LPCSTR lpszProfileName, 
									LPCSTR lpszSectionName, 
									LPCSTR lpszKeyName,          /* 段内的关键字名    */
                  					WORD   verticalCoordinate,
									SDWORD sdwReturnValue 
									);

DllExport BOOL SetU64ValueToTableWithKey( LPCSTR lpszProfileName, 
									LPCSTR lpszSectionName, 
									LPCSTR lpszKeyName,          /* 段内的关键字名    */
                  					WORD   verticalCoordinate,
									UINT64 sdwReturnValue 
									);

//end

#if defined(_VXWORKS_) || defined(_LINUX_)
DllExport BOOL ReadRouteTable( const TCHAR * lpszProfileName );
#endif
	/* 从配置文件中读取路由表并设置 */

/* 字符串处理类函数 */
DllExport void Trim( LPSTR lpszSrc );        /* 去除字符串的前导及后续空格以及TAB键 */
DllExport void StrUpper( LPSTR lpszSrc );    /* 字符串转换成大写 */
DllExport void StrLower( LPSTR lpszSrc );    /* 字符串转换成小写 */

/* 内存处理函数*/
/*====================================================================
    功能        ：释放掉动态申请（malloc）的表结构内存
    输入参数说明：void **ppMem, 指针数组指针
                  DWORD dwEntryNum, 指针数组大小
    返回值说明  ：void
====================================================================*/
/* DllExport void TableMemoryFree( void **ppMem, DWORD dwEntryNum ); */

/*====================================================================
    功能        ：验证是否合法日期
    输入参数说明：WORD wYear, 年
                  UCHAR byMonth, 月
                  UCHAR byDay, 日
                  UCHAR byHour, 时
                  UCHAR byMin, 分
                  UCHAR bySec, 秒
    返回值说明  ：合法返回TRUE，负责返回FALSE
====================================================================*/
DllExport BOOL IsValidTime( WORD wYear, UCHAR byMonth, UCHAR byDay, 
                           UCHAR byHour, UCHAR byMin, UCHAR bySec );


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _KDV_SYS_H_ */

/* end of file sys.h */

