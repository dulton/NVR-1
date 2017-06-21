
#ifndef __DVRDEF_H__
#define __DVRDEF_H__

#include <stdio.h>
#include "Types.h"
#include <stdarg.h>

typedef void *				VD_HANDLE;
typedef int					VD_BOOL;

typedef uint				VD_COLORREF;//0x00bbggrr
typedef uint				VD_COLORDEV;//0x????????

typedef unsigned char			BYTE;
typedef unsigned short			WORD;
typedef signed int				LONG;
typedef unsigned int			PARAM;
typedef unsigned int			DWORD;
typedef void *					HANDLE;

//typedef int						BOOL;
#ifndef BOOL
#define BOOL int
#endif

typedef unsigned long long		UQWORD;
typedef long long				SQWORD;

#define FOR(i, max)				for (i = 0; i < (max); i++)
#define BITMSK(bit)				(int)(1 << (bit))

#define INTEGER_MIN (int)(0x80000000)
#define INTEGER_MAX (int)(0x7FFFFFFF)
#define UINTEGER_MAX (unsigned int)0xFFFFFFFF

#define VD_MAKEWORD(a, b)      ((ushort)(((uchar)(a)) | ((ushort)((uchar)(b))) << 8))
#define VD_MAKELONG(a, b)      ((int)(((ushort)(a)) | ((uint)((ushort)(b))) << 16))
#define VD_MAKEDWORD(a, b, c, d) ((uint)(((uchar)(a)) | ((uchar)(b) << 8) | ((uchar)(c) << 16) | ((uchar)(d) << 24)))
#define VD_LOWORD(l)           ((ushort)(l))
#define VD_HIWORD(l)           ((ushort)(((uint)(l) >> 16) & 0xFFFF))
#define VD_LOBYTE(w)           ((uchar)(w))
#define VD_HIBYTE(w)           ((uchar)(((ushort)(w) >> 8) & 0xFF))

#define TRUE					1
#define FALSE					0

#ifndef NULL
#define NULL					0
#endif

#define ON						1
#define OFF						0

#define MPG1					1
#define MPG4					4

#define CIF1					1	//1 CIF
#define CIF2					2	//2 CIF
#define CIF4					4	//4 CIF

#define NC						0	//常关

#define NO						1	//常开

#ifndef MAX
#define MAX(a,b)				(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)				(((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS//csp modify
#define ABS(x) ((x)>=0?(x):-(x))
#endif

#ifdef FUNC_BUILD_CHANNEL
#define channel_utol(x) (2 * x - x % 2)
#define channel_ltou(x) ((x + 1) / 2)
#else
#define channel_utol(x) (x)
#define channel_ltou(x) (x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

//#########################################################################
// use the unified 'DEBUG' macro
#if (!defined(NDEBUG)) && !defined(DEBUG)
#	define DEBUG
#endif

//////////////////////////////////////////////////////////////////////////
// print message, controlled by messsage level
int logTrace(const char* fmt, ...);
int logDebug(const char* fmt, ...);
int logInfo(const char* fmt, ...);
int logWarn(const char* fmt, ...);
int logError(const char* fmt, ...);
int logFatal(const char* fmt, ...);

#ifdef __linux__ // perform complete xxxxf macro replacement
#ifdef DEBUG
#	define tracef(format, ...) logTrace(format, ## __VA_ARGS__)
#	define debugf(format, ...) logDebug(format, ## __VA_ARGS__)
#	define infof(format, ...) logInfo(format, ## __VA_ARGS__)
#	define warnf(format, ...) logWarn(format, ## __VA_ARGS__)
#	define errorf(format, ...) logError(format, ## __VA_ARGS__)
#	define fatalf(format, ...) logFatal(format, ## __VA_ARGS__)
#else
#	define tracef(format, ...) 
#	define debugf(format, ...) 
#	define infof(format, ...) logInfo(format, ## __VA_ARGS__)
#	define warnf(format, ...) logWarn(format, ## __VA_ARGS__)
#	define errorf(format, ...) logError(format, ## __VA_ARGS__)
#	define fatalf(format, ...) logFatal(format, ## __VA_ARGS__)
#endif
#else
#ifdef DEBUG
#	define tracef logTrace
#	define debugf logDebug
#	define infof logInfo
#	define warnf logWarn
#	define errorf logError
#	define fatalf logFatal
#else
#	define tracef if(0)logTrace
#	define debugf if(0)logDebug
#	define infof logInfo
#	define warnf logWarn
#	define errorf logError
#	define fatalf logFatal
#endif
#endif

#define tracepoint()  debugf("tracepoint: %s, %d.\n",__FILE__,__LINE__)
#define trace tracef

//#define __trip 			do {trace("-W-%d::%s(%d)\n", (int)time(NULL), __FILE__, __LINE__);}while(0)
//#define __fline 			tracepoint() 
//#########################################################################

#ifdef __cplusplus
}
#endif

#define NAME_LEN		32
#define CHANNEL_NAME_SIZE		64
#define LABEL_LEN		32
#define VD_MAX_PATH		240

#ifndef N_SYS_CH
#define N_SYS_CH		16
#endif
//#ifndef MAX_DDNS_TYPE 
//#define MAX_DDNS_TYPE 5
//#endif

#ifndef MAX_ALARMSERVER_TYPE
#define MAX_ALARMSERVER_TYPE 3
#endif
#define N_ALM_IN		16
#define N_ALM_OUT		6
#define N_PLY_CH		4
#define N_AUDIO_IN		16
#define N_PTZ_ALARM		16
#define N_WEEKS			7
#define N_TSECT			6
#define N_SPLIT			10
#define N_VIDEO_OUT		4
#define N_GUI_CORE		2

#define AUTHORITY_DISP_ITEMS 100
#define RELATIVE_MAX_X 0x7FF
#define RELATIVE_MAX_Y 0x7FF
#define TOUCHSCREEN_MAX_X 0x7FF
#define TOUCHSCREEN_MAX_Y 0x7FF

#define N_MATRIX_OUT	4

typedef struct {
   int left;
   int top;
   int right;
   int bottom;
}VD_RECT,*VD_PRECT;

typedef struct {
	int x;
	int y;
}VD_POINT,*VD_PPOINT;

typedef struct {
	int w;
	int h;
}VD_SIZE,*VD_PSIZE;
#ifndef __SMALL_FILE_TIME_DEFINED__
#define __SMALL_FILE_TIME_DEFINED__

//时间结构
typedef struct _DHTIME								
{
	uint second		:6;					//	秒	1-60		
	uint minute		:6;					//	分	1-60		
	uint hour			:5;					//	时	1-24		
	uint day			:5;					//	日	1-31		
	uint month			:4;					//	月	1-12		
	uint year			:6;					//	年	2000-2063	

}DHTIME,*pDHTIME;
#endif
typedef union {	//IP addr
	uchar	c[4];
	ushort	s[2];
	uint	l;
} IPADDR;

typedef struct { // Serial number
	int		iNumber[4];
}SERIALNUMBER;

typedef struct {	// 定时时段
	uchar	StartHour;
	uchar	StartMin;
	uchar	StartSec;
	uchar	EndHour;
	uchar    EndMin;
	uchar	EndSec;
	uchar	State;			//第二位是定时，第三位是动态检测，第四位是报警
	uchar	Reserve;		/*!< Reserve已经被使用，更改的话请通知录像模块 */
} TSECT;

typedef struct tagTIMESECTION
{
	//!使能
	VD_BOOL enable;
	//!开始时间:小时
	int startHour;
	//!开始时间:分钟
	int	startMinute;
	//!开始时间:秒钟
	int	startSecond;
	//!结束时间:小时
	int	endHour;
	//!结束时间:分钟
	int	endMinute;
	//!结束时间:秒钟
	int	endSecond;
}TIMESECTION;	

/*-----------------------------------------------------------------------
	配置文件版本结构 zhouwei
-----------------------------------------------------------------------*/
typedef struct 
{
	uint dwVersion;
	uchar dbReserve[4];
}VER_CONFIG;

enum {	//user status ...
	S_NO_USR = -100,
	S_PSW_ERR,
	S_FREE = -2,
	S_ACTIVE,
	S_GUEST = 0,
	S_USER,
	S_ADMIN,
	S_SUPER
};

typedef union {
	uint dw;
	char ch[4];
}INPUT_CHAR;

enum xm_msg_t {
	XM_MSG_SYSTEM = 0x000000,
		XM_QUIT,

	XM_MSG_LOC = 0x010000,
		XM_DHFILE_CREATE,	XM_DHFILE_OPEN,		XM_DHFILE_CLOSE,	XM_DHFILE_PACK,	
		XM_DHFILE_READ,		XM_DHFILE_WRITE,	XM_DHFILE_DATA,		XM_DHFILE_SYNC,
		XM_DHFILE_WAKEUP,
		XM_CAP_START,		XM_CAP_STOP,		XM_CAP_DATA,		XM_CAP_FLUSH, 
		XM_PLY_START,		XM_PLY_STOP,		XM_PLY_DATA,		XM_PLY_SYNC, 
		XM_BKP_DATA,
		
	XM_MSG_GUI = 0x020000,
		XM_KEEPALIVE,
		XM_KEYDOWN,		XM_KEYUP,
		XM_MOUSEFOUND,  XM_MOUSELOST,
		XM_MOUSEMOVE,	XM_MOUSEWHEEL,
		XM_LBUTTONDOWN,	XM_LBUTTONUP,	XM_LBUTTONDBLCLK,
		XM_RBUTTONDOWN,	XM_RBUTTONUP,	XM_RBUTTONDBLCLK,
		XM_MBUTTONDOWN,	XM_MBUTTONUP,	XM_MBUTTONDBLCLK,
		XM_DRAW,		XM_SCROLL,		XM_LIST,
		XM_NUMPAD,		XM_CARET,		XM_CHAR,
		XM_TIMER,		XM_INPUTTYPE,	XM_ACTIVATE,
		XM_DEACTIVATE,
		XM_KEYDOWNDELAY,
		XM_TIMETICK,

	XM_MSG_NET = 0x030000,
		XM_NET_START, XM_NET_STOP, XM_NET_DATA, XM_NET_FLUSH,
		XM_NMN_START, XM_NMN_STOP, XM_NMN_DATA, XM_NMN_FLUSH,
		XM_NPY_START, XM_NPY_STOP, XM_NPY_DATA, XM_NPY_FLUSH,
		XM_NUSR_STOP,
	
	XM_SYSTEM_END
};

//应用程序事件
typedef enum app_event_code
{
	appEventInit = 0,
	appEventAlarmLocal = 1,
	appEventAlarmNet,
	appEventAlarmDecoder,
	appEventAlarmManual,
	appEventVideoMotion,
	appEventVideoLoss,
	appEventVideoBlind,
	appEventVideoTitle,
	appEventVideoSplit,
	appEventVideoTour,
	appEventStorageNotExist,
	appEventStorageFailure,
	appEventStorageLowSpace,
	appEventNetAbort,
	appEventEncoderError,
	appEventCDNoSpace,
	appEventComm,
	appEventStorageReadErr,
	appEventStorageWriteErr,
	appEventNetArp,
	appEventAlarmInfrared,
	appEventAlarmEmergency,
	appEventAlarmEncoder,
	appEventDecConnect,
	appEventUpgrade,
	appEventAll
}appEventCode;

//thread priority fifo 0-29
#define TP_COM			 1		//1
#define TP_TIMER		 2		//1
#define TP_ALARM		 3		//1
#define TP_PLAY			 4		//4

#define TP_REC			 6
#define TP_CAPTURE	20		//16
#define TP_DHFILE		24		//1
#define TP_GUI			25		//1
#define TP_PTZ			26

//thread priority other 30-99
#define TP_SMTP     30
#define WATCH_DOG   31

#define TP_BACKUP		35		//1
#define TP_SVR			37		//2
#define TP_NET			39		//16
#define TP_TEL			55		//1
#define TP_FRONTBOARD	56		//1
#define TP_MOUSE		57		//1
#define TP_FTP			58
#define TP_WEB          58
#define TP_DEFAULT		64
#define TP_RTSP			65
#define TP_PRE			 127

enum dh_msgbox_t {
	MT_OKCANCEL = 0,
	MT_OK
};

enum fmt_time_flag{
	FT_NORMAL = 0,
	FT_HALF_YEAR = 0x0001,
	FT_NO_SECOND = 0x0002,
	FT_NO_SIGNS  = 0x0004,
	FT_ONLY_DATE = 0x0008,
	FT_ONLY_TIME = 0x0010,
	FT_FULL_AMPM = 0x0020
};

#ifndef RGB
#define VD_RGB(r,g,b)		((VD_COLORREF)(((uchar)(r)|((ushort)((uchar)(g))<<8))|(((uint)(uchar)(b))<<16)))
#endif

#define VD_RGBA(r,g,b,a)	((VD_COLORREF)(((uchar)(r)|((ushort)((uchar)(g))<<8))|(((uint)(uchar)(b))<<16)|(((uint)(uchar)(a))<<24)))
#define VD_GetRValue(c)	((uchar)(c))
#define VD_GetGValue(c)	((uchar)(((ushort)(c)) >> 8))
#define VD_GetBValue(c)	((uchar)((c)>>16))
#define VD_GetAValue(c)	((uchar)((c)>>24))

#define PRINT_TIME(s,x) \
	do{printf("%s[%d-%d-%d %d:%d:%d]\n", \
	(s), (x).year, (x).month, (x).day, (x).hour, (x).minute, (x).second);}while(0)
//例如
//1.PRINT_TIME("",dhtime);
//2.PRINT_TIME("systimeX:",systimeX);

typedef const VD_RECT* VD_PCRECT;
typedef const VD_POINT* VD_PCPOINT;
typedef const VD_SIZE* VD_PCSIZE;
typedef const char* VD_PCSTR;

#ifndef WIN32
#define LIBDVR_PRINT(fmt, args ... )   fprintf(stderr, "LIBDVR : "  fmt, ## args)
#define ERR_PRINT(fmt, args ... )    do {fprintf(stderr, "LIBDVR : FILE -> %s, LINE -> %d" ": "   fmt, __FILE__, __LINE__, ## args);} while(0)
#else

#endif//WIN32

typedef struct tagAlarmPattern
{
	int pulsewid;
}AlarmPattern;

struct erase_info_user
{
        unsigned int start;
        unsigned int length;
};

#define MEMERASE	_IOW('M', 2, struct erase_info_user)



typedef enum cmos_offs_t {   //0x08 - 0x3f 56个字节可用
 	CMOS_RTC   = 0x00,   //RTC内部使用  [0x00 - 0x07]
	CMOS_LABAL   = 0x08,   //幻数(固定为XC) [0x08 - 0x09]
	CMOS_IP   = 0x0A,   //静态IP   [0x0A - 0x0D]
	CMOS_MASK   = 0x0E,   //静态掩码   [0x0E - 0x11]
	CMOS_GATE   = 0x12,   //静态网关   [0x12 - 0x15]
	CMOS_DHCP   = 0x16,   //DHCP标志   [0x16 - 0x16]
	CMOS_PPPOE_ENABLE = 0x17,   //PPPOE使能标志  [0x17 - 0x17]
	CMOS_PPPOE_NAME  = 0x18,   //PPPOE用户名  [0x18 - 0x27]
	CMOS_PPPOE_PWD  = 0x28,   //PPPOE密码   [0x28 - 0x37]
	CMOS_FLAGS   = 0x38,   //     [0x38 - 0x39]
	CMOS_RESERVED   = 0x3a,   //保留    [0x3a - 0x3a]
	CMOS_EXIT_TIME  = 0x3b,   //程序退出时间  [0x3b - 0x3e]
	CMOS_EXIT_STATE  = 0x3f,   //程序退出状态  [0x3f - 0x3f]
	CMOS_MANUAL_REC  = 0x40,   //手动录象标志  [0x40 - 0x47]
	//add by zhangxuewu 20090921,add function for upnp
	CMOS_UPNP_ENABLE        =  0x48,   //UPNP 使能标识[0x48 - 0x48]
	CMOS_AREA_END   = 0x49, //modify by zhangxuewu 20090921,add function for upnp
} CMOS_OFFS;




struct I2C_Data
{
	unsigned char chip;
	unsigned char page;
	unsigned char addr;
	unsigned char value;
};




struct boot_env_data
{
	unsigned char name[24];
	unsigned char value[32];
};


enum date_fmt {
	DF_YYMMDD = 0,		
	DF_MMDDYY,			
	DF_DDMMYY			
};

enum date_spec {
	DS_DOT	= 0,		
	DS_DASH,			
	DS_SLASH			
};

enum time_fmt {
	TF_24	= 0,		
	TF_12				
};

#endif //__DVRDEF_H__

