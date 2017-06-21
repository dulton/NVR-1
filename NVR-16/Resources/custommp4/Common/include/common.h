#ifndef _COMMON_H_
#define _COMMON_H_

#define _NVR_
#define _ALL_HD_

#ifdef _NVR_
extern int GetNVRChnNum();
#define SINGLE_PIC_HIDE_TIME
#define NVR_REALTIME
#define NVR_MEM_OPT
#endif

//#define PW_TEST

//model

//#define TL_9508_8M
//#define TL_9504_D1_8M
//#define TL_9504_CIF_8M

#define R9508S//wangcong tw2868 tl-s

//#define R9504S//wangcong lg1702 tl-s 9504D

//#define R9504S_CIF





//#define TL_9508
//#define TL_9508_D1
//#define TL_9508_RTL
//#define TL_9504_CIF_RTL
//#define TL_9504_D1
//#define TL_9504_D1_RTL
//#define TL_9504_CIF_8M_RTL
//#define TL_9508_D1_8M

//#define PREVIEW_6WIN_
//#define CHNS_6

#define DNS_CHANGE     //jflv Aug 24 dns type change bug
#define VGA_RESOL_FIX//zlb20101126
#define PIC_TWINKLE_OPT
#define MSGQ_OPT
#define MSGQ_STAT   //wangcong web 128

#define IECTRL_NEW  //wangcong 16
// FB_LESS_BUF_OPT  //vga分辨率
//#define CHN_ORDER     //逆序

#define  DDNS_LOCK_BUG  //DDNS 账号会锁定bug

//#define R9508_TW2868    //wangcong 2868

//#define R9504_LG1702     //wangcong lg1702  9504D  

//#define  R9508_LG1702   //wangcong lg1702

//#define R9504_RN6266   //wangcong rn6266

//#define R9508_RN6266   //wangcong rn6266



//#define R9504S_2866   //wangcong 2866 tl-s    9504D




#if defined(R9508S) || defined (R9504S) || defined (R9504S_2866)  || defined (R9504S_CIF) 
#define TL_S
#ifndef _NVR_
#define MANU_STANDARD  //手动制式
#endif
#endif

#if defined(R9508S) 
#define TL_9508_8M
#define  R950X_MP2807	//大砌传参数
#endif

#if defined(R9504S) 
#define TL_9504_D1_8M
#define  R950X_MP2807	//大砌传参数
#endif

#if defined(R9508S_CIF) 
#define TL_9504_CIF_8M
#endif

//pw 2010/9/27
#ifdef TL_9504_D1_8M
#define TL_9504_D1
#define CFG_SAVE_IN_FLASH
#endif

//pw 2010/9/6
#ifdef TL_9504_CIF_8M
#define TL_9504_CIF
#define CFG_SAVE_IN_FLASH
#endif

//pw 2010/9/17
#if defined(TL_9508_8M) || defined(TL_9508_D1_8M)
#define TL_9508
#define CFG_SAVE_IN_FLASH
#endif

#if defined(TL_9508_D1)
#define TL_9508
#endif

//pw 2010/9/7
//调整音视频顺序
#if defined(TL_9508_RTL)
#define TL_9508
#define CHN_ORDER
#endif

//pw 2010/10/11
#if defined(TL_9504_CIF_RTL)
#define TL_9504_CIF
#define CHN_ORDER
#endif

//pw 2010/10/11
#if defined(TL_9504_D1_RTL)
#define TL_9504_D1
#define CHN_ORDER
#endif

//pw 2010/10/13
#if defined(TL_9504_CIF_8M_RTL)
#define TL_9504_CIF_8M
#define CHN_ORDER
#endif

//pw 2010/9/13
#if defined(TL_9504_CIF) || defined(TL_9504_D1_8M) || defined(TL_9504_CIF_8M)
#define TL_9504
#endif

#if defined(TL_9508_D1_8M) || defined(TL_9508_D1)//DDR_128M
	#define D1_CHN_NUM 8
	#define D1_8
#else
	#ifdef TL_9504_D1
		#define D1_CHN_NUM 4
	#elif defined(TL_9504_CIF)
		#define D1_CHN_NUM 1
	#else
		#define D1_CHN_NUM 2
	#endif
#endif

#define DDR_128M  //wangcong 120
#define MODEL_9508			0
#define MODEL_9504_D1		1
/*****************************************/
//client
//#define YOUSHENJIA
//#define XINQIGUOJI
//#define HANKE
//#define HANKE_SP1
//#define HANKE_SP2
//#define HANKE_SP3
//#define HANKE_SP4
//#define HANKE_SP7
//#define HANKE_SP10         //黑色背景 德加拉
//#define HAOWEI //豪威
//#define JIUAN
//#define JIUAN_2 //9508 
//#define JINGYANG
//#define LONGANSHI			//
//#define LONGANSHI_SP1		//中英文
//#define LONGANSHI_SP2		//单路音频,中英文
//#define LONGANSHI_SP3		//单路音频
//#define LONGANSHI_Z		//9508
//#define LONGANSHI_Z_SP2    //唯硕定制
//#define LONGANSHI_Z_SP1
//#define LONGANSHI_1A
//#define WANJIAAN
//#define GAOSIBEIER

//#define L_SHOW_TEST   //土耳其语调试





#define JUNMINGSHI     		//增加泰文
//#define JUNMINGSHI_THAI	//默认参数
//#define JUNMINGSHI_SP2		//不同的logo  凯聪
//#define JUNMINGSHI_SP3		 //繁体中文
//#define JUNMINGSHI_SP5        //redrock logo
//#define JUNMINGSHI_sp6        //平安符logo
//#define JUNMINGSHI_SP7        //报警抓图FTP
//#define JUNMINGSHI_SP8       //录像保存30天
//#define JUNMINGSHI_sp9       //波斯语
//#define JUNMINGSHI_sp10     //9504S-CIF
//#define JUNMINGSHI_sp11     //people域名
//#define JUNMINGSHI_KANGLIAN//康联定制
//#define JUNMINGSHI_KAICONG	//凯聪定制

//#define JMV_NEWPANEL





//#define ZHONGWEISHIJI
//#define MEIDIANBEIER_UI
//#define HISHARP
//#define SHENSHIYIN         //增加泰文
//#define SHENSHIYIN_SP1
//#define TIANMIN
//#define KUANGSHIAN
//#define PANIKE
//#define WEIDUOLIYA
//#define KEAN
//#define KEKANGDA
//#define KEKANGDA_SP1
//#define XINWEITU
//#define ANGESHI
//#define TAIJUN
//#define DONGJIA
//#define TAIKANGWEIYE 
//#define TAIKANGWEIYE_sp5   //越南语
//#define TUMIN
//#define TUERKE
//#define YINGSHITIANXIA3
//#define HUANYUWEISHI
//#define ZHAOFENG
//#define LIQIN
//#define LIQIN_SP1
//#define LIQIN_SP2
//#define LIQIN_SP3
//#define YISHI
//#define RUIDASHI

/************************************************/
//#define JIUAN_2_SP1    //enter定制
//#define JIUAN_2_SP2    //布防
//#define JIUAN_2_SP3 //   三国语言
//#define JIUAN_2_SP4    //繁体中文
//#define JIUAN_2_SP5    //无简中

//#define  channel_limit    //通道限制

//#define Web_user_count   //wangcong las web_user
#define ARABIC_SUPPORT  //阿拉伯语  wangcong71

#ifndef JUNMINGSHI_KANGLIAN
#define PLAYBACK_IMAGE_ADJUST		//录像回放画面参数调节
#define ELETRONIC_ZOOM				//电子放大
#endif

#if defined (JUNMINGSHI_sp11)
#define PEOPLE_DDNS
#define JUNMINGSHI
#define JUNMINGSHI_THAI          //默认参数
#endif

#if  defined(JUNMINGSHI_SP3)
#define JUNMINGSHI
#endif

#if defined( JUNMINGSHI_sp9) || defined ( JUNMINGSHI_sp10)
#define JUNMINGSHI
#endif

#ifdef JUNMINGSHI_SP8
#define JUNMINGSHI
#define RECORD_XDAY
#endif

#ifdef JUNMINGSHI_SP7
#define FTP_PIC
#define JUNMINGSHI
#define FTP_DDNS
#define TIME_OSD_FTP
//#define FTP_PIC_2   //遮挡和丢失
#endif

#ifdef JUNMINGSHI_sp6
#define JUNMINGSHI
#endif

#ifdef JUNMINGSHI_THAI
#define JUNMINGSHI
#endif

#if defined( JUNMINGSHI )
//#ifndef PEOPLE_DDNS
//#define JUNMINGSHI_DDNS
//#endif
#if ((!defined(PEOPLE_DDNS))&&(!defined(NO_CUSTOM_DDNS))\
	  &&(!defined(JUNMINGSHI_KANGLIAN))&&(!defined(JUNMINGSHI_SP14)))
#define JUNMINGSHI_DDNS
#endif
#define TAIWEN_TIME
#endif

#ifdef JUNMINGSHI_KANGLIAN
#define ALARM_TOUCH_OFF_FULL_SCREEN		//警报触发全屏 
//#define BACKGROUND_BLACK				//背景黑
#define SHOW_VL_FLAG					//显示视频丢失
#define LANGUAGE_CHANGE
//#define JUNMINGSHI_KANGLIAN_TEST		//颜色调试
#define JUNMINGSHI_KANGLIAN_BLACK		//背景黑   小孔调库代码
#ifdef R9508S
//#define FLAG_CORNER8					// 报警标志移至左下角
#endif
#ifdef R9504S
//#define FLAG_CORNER4					// 报警标志移至左下角
#endif
#endif

#ifndef JUNMINGSHI
#define checkup							//升级验证
#endif

//#if defined( JUNMINGSHI_DDNS)
typedef enum cmd_type
{
	DDNS_LOGIN = 0,
	DDNS_REGIST,
	DDNS_CANCEL,
	DDNS_HEART
}CMD_TYPE;
//#endif

#if defined(JIUAN_2_SP1)
//#define ENTER_LOGO_WIDTH 80
//#define ENTER_LOGO_HEIGHT 25
#define ENTER_LOGO_WIDTH 128
#define ENTER_LOGO_HEIGHT 40
#endif

#ifdef JUNMINGSHI_KANGLIAN
#define NOSIGNAL_LOGO_WIDTH 120
#define NOSIGNAL_LOGO_HEIGHT 32
#endif

#if defined(JIUAN) || defined(JIUAN_2)   //wangcong jiuan
#define HALF_D1    1
#endif

#ifdef  RUIDASHI
#define CHN_ORDER
#endif

#if defined(LIQIN_SP3)  //wangcong
#define LIQIN_SP1
#define JUNMINGSHI
#endif

//pw 2010/10/26
#if defined(HANKE_SP7)
#define HANKE_SP3
#endif
#if defined(TAIKANGWEIYE_sp5)
#define TAIKANGWEIYE
#endif

#if defined(LIQIN_SP1) || defined(LIQIN_SP2)
#define LIQIN
#define MAC_RANDOM
#endif

//pw 2010/10/25
//#if defined(TL_9508) && defined(JUNMINGSHI_SP2)
#if  defined(JUNMINGSHI_SP2)
#define JUNMINGSHI
#endif

#if defined(LONGANSHI_Z_SP2)//wangcong 20101123
#define LONGANSHI_Z
#define BACKGROUND_BLACK
#endif

//pw 2010/10/25
#if defined(TUERKE) || defined(YISHI)
#define CHN_ORDER
#endif

#if defined(HUANYUWEISHI)
#define CHN_ORDER
#endif

//pw 2010/10/12
#if defined(TUMIN)
#define USB_REC
#define USB_FORMAT
//#define D1_8
#endif

//#define USB_REC  //wangcong yp
//#define USB_FORMAT

#define CRUISE_INDEX_PRESETPOS //巡航


#if defined(TIANMIN)

//日光节约时间、时区
#define _DAYTIME_

//pw 2010/9/10
#define USB_ONLY

//手动制式
#define MANU_STANDARD

//录像OSD单独控制
#define REC_OSD

//报警抓拍
#define VIDEO_SNAP

//u盘格式化，只分区格式化并不创建128m文件
#define USB_FORMAT
#endif

//pw 2010/7/30
#if defined(KEKANGDA_SP1)
#define KEKANGDA
#endif

#if defined(HANKE_SP6)
#define HANKE_SP2
#define HANKE_SP5
#endif

//pw 2010/9/10
#if defined(HANKE_SP10)
#define HANKE_SP2
#define BACKGROUND_BLACK
#endif

//pw 2010/8/16
#if defined(HANKE_SP1) || defined(HANKE_SP2) || defined(HANKE_SP4) || defined(HANKE_SP3)  || defined(HANKE_SP5)
#define HANKE
#endif

//#define HOUR_24
//pw 2010/8/4
#if defined(HISHARP) || defined(TIANMIN)
#define REC_RETENTION_DAYS			//录像保留天数
	#ifdef HOUR_24
		#define DATA_NORMAL 100
	#endif
#endif



//PW 2010/6/10
//func module
#define MAC_STATIC
#define FONT_BACKGROUND
#define EMAIL_OPT
#define DEFINITION_NAME
#define REC_D1
#define EFFECT_OPT



//pw 2010/9/29
#define USE_AUDIO_PCMU

//pw 2010/9/10
//#define SUB_STREAM_OPT		//字码流优化
//#define SUB_STREAM_DANCING
#define SUB_GOP 25

//pw 2010/9/29
#if defined(HANKE) || defined(JUNMINGSHI) || defined(WANJIAAN) || defined(XINQIGUOJI) || defined(TAIKANGWEIYE) //wangcong 20101125
//字的背景功能模块
#undef FONT_BACKGROUND
#endif

#ifdef JIUAN_2_SP1
#define SHOW_VL_FLAG
#endif

#ifdef HISHARP
//显示视频丢失信息
#define SHOW_VL_FLAG

//硬盘信息显示
#define SHOW_HDD_INFO

//导入导出功能模块
#define INPUT_OUTPUT_MODULE

//显示客户端连接状态
#define SHOW_CLIENT_CONN_STAT

#define NTP

#define DST	//daylight saving time

//#define BOUNCING_PICTURE
//#define BOUNCING_PICTURE_TEST
#define BOUNCING_PICTURE_MODULE
#endif

#define REC_BUG_FIX//zlb20100928

//pw 2010/7/6
#if defined(TL_9508) && defined(TIANMIN)
#define TIANMIN_VER
//#define NTP
//#define DST
#define HALF_D1				1
#define ALARM_STATUS
#endif

//pw 2010/8/13
#ifdef HANKE
#define ALARM_STATUS
#endif

//pw 2010/6/5
#define FLAGEMAIL

#ifdef SHENSHIYIN_SP1
#define REMOTE_PLAY_MULTI
#endif

//加密芯片的权限认证                     
//#define NO_AUTH

#define HISI_3515
#ifdef HISI_3515
//#define HISI_3515_DEMO
#define HISI_3515_HD
#define FB_LESS_BUF_OPT
//#define HISI_3515_VGA_SCREEN
#define HISI_3515_HD_TDE_RESIZE

//pw 2010/7/5
#define HISI_3515_VGA_MOUSE//20100703 zlb_mouse

//2010/6/8
//#define HISI_3515_HD_TDE_RESIZE_STATE
//#define HISI_3515_VGA_SCREEN_SIMPLE
//#define HISI_3515_DEFLICKER
#endif

#ifdef MEIDIANBEIER_UI
#define MENUPIC_NUM	 6
#endif

//pw 2010/9/20
#if defined(LONGANSHI_SP2) ||defined(LONGANSHI_SP3)
#define LONGANSHI_1A
#endif

//pw 2010/7/30
#if defined(LONGANSHI_SP1) || defined(LONGANSHI_1A) || defined(LONGANSHI_SP2)
#define LONGANSHI
#endif

#if defined(LONGANSHI_Z_SP1) 
#define LONGANSHI_Z
#endif

#if defined(LONGANSHI) ||defined(LONGANSHI_Z) 
#define LONGANSHI_DDNS
#define SHOW_DISK_PROGRSS
#define TIMED_REBOOT
#define LONGANSHI_M		//添加系统维护  yqluo 20101023
#define BACKGROUND_BLACK
#define ALARMSCH
#define ALARM_STATUS
#define LONGANSHI_SET
#endif

//pw 2010/9/20
#if defined(SHENSHIYIN_SP1)
#define SHENSHIYIN
#endif

//pw 2010/8/19
//#if defined(HANKE) ||defined(HAOWEI) || defined(SHENSHIYIN) || defined(JIUAN_2) || defined(XINQIGUOJI) || defined(KEAN) ||defined(LONGANSHI) ||defined(LONGANSHI_Z)
#define BACKUP_AS_AVI     //wangcong 20101125
//#endif


#define NEW_HARDWARE
//#define WATERMARK
//#define __DEBUG
//#define DVR_TEST
//#define NO_AUDIO_TEST
#define SJJK_LANGTAO		//dong	100427
#define MOBILE_SERVER2
#define GOP_KEY_SUPPORT

//pw:2010/6/12
#define SJJK_PORT
//#define NETSND_OPT//zlb20100802

#define ALARM_UPLOAD


//#define VL_VB_DEAL
#ifndef JUNMINGSHI
//#define SHOW_DISK_PROGRSS
#endif

#if defined(JIUAN_2_SP2)
#define ALARMSCH
#define ALARM_STATUS
#endif

// pw 2010/8/26 多路回放倒放功能
#define PLAY_BACKWARD_MULTI

//优化的选项
//#define PLAYBACK_OPT

//pw 2010/8/6
#define CIF					0
#define CIF_DEFINITION_N	"352*240"
#define CIF_DEFINITION_P	"352*288"

//#define HALF_D1				1
#define HD1_DEFINITION_N	"704*240"
#define HD1_DEFINITION_P	"704*288"


#ifdef HALF_D1
#define D1					2
#else
#define D1					1
#endif
#define D1_DEFINITION_N		"704*480"
#define D1_DEFINITION_P		"704*576"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include "iflytype.h"

#ifndef WIN32
#include <unistd.h>
#include <pthread.h>
typedef pthread_t THREADHANDLE;
typedef void* (*LINUXFUNC)(void*);
#else
#include <wtypes.h>
typedef HANDLE THREADHANDLE;
#endif

#define PID_TIMER				0x10
#define PID_MAIN_CONTROL		0x20
#define PID_SYSLOG				0x30
#define PID_PANEL				0x40
#define PID_EVENT				0x50

#define THREAD_PRI_LOW			100
#define THREAD_PRI_MID			80
#define THREAD_PRI_HIGH			60

#define PRI_TIMER				THREAD_PRI_MID
#define PRI_UI					THREAD_PRI_HIGH
#define PRI_CP					THREAD_PRI_HIGH
#define PRI_CPCHECK				THREAD_PRI_LOW
#define PRI_CAPTURE				THREAD_PRI_HIGH

#ifdef _NVR_
#define PRI_DISPLAY				THREAD_PRI_MID
#else
#define PRI_DISPLAY				THREAD_PRI_HIGH
#endif

#define PRI_ENCODE				THREAD_PRI_HIGH
#define PRI_DECODE				THREAD_PRI_HIGH
#define PRI_RECORD				THREAD_PRI_HIGH
#define PRI_MEDIASND			THREAD_PRI_HIGH
#define PRI_MEDIARCV			THREAD_PRI_HIGH
#define PRI_AUDIO				THREAD_PRI_MID
#define PRI_ALARM				THREAD_PRI_MID
#define NTP_CLIENT				THREAD_PRI_MID

#define THREAD_STKSIZE_DEFAULT	(8<<20)

#define STKSIZE_TIMER			(256<<10)
#define STKSIZE_UI				(4<<20)
#define STKSIZE_CP				(512<<10)
#define STKSIZE_CPCHECK			(256<<10)
#define STKSIZE_CAPTURE			(1<<20)
#define STKSIZE_DISPLAY			(1<<20)
#define STKSIZE_ENCODE			(1<<20)
#define STKSIZE_DECODE			(1<<20)
#define STKSIZE_RECORD			(2<<20)
#define STKSIZE_MEDIASND		(512<<10)
#define STKSIZE_MEDIARCV		(512<<10)
#define STKSIZE_AUDIO			(512<<10)
#define STKSIZE_ALARM			(256<<10)
#define STKSIZE_NTPCLIENT		(256<<10)

//#define UPDATE_TYPE_ALL 0
//#define UPDATE_TYPE_PROGRAM 1
//#define UPDATE_TYPE_FS 2
//#define UPDATE_TYPE_UIMAGE 3
//#define UPDATE_TYPE_DTB 4
//#define UPDATE_TYPE_PANEL 5
//find1
#define UPDATE_TYPE_MAINBOARD 0
#define UPDATE_TYPE_PANEL 1

typedef struct _MSGQueue
{
	u32 dwReadID;
	u32 dwWriteID;
	u32 dwMsgNumber;
	u32 dwMsgLength;
}MSGQueue,* MSGQHANDLE;

typedef struct
{
	u16 sender;
	u16 event;
	u32 arg;
}ifly_msg_t;

//pw 2010/7/9
#if defined(TL_9508)
	#ifdef CHNS_6
		#define MAX_CHN_NUM				6//16//fpga:16 6446:4
		#define MAX_AUDIO_CHN_NUM		4
		#define SPLIT_NO				3//分割线的数量
	#else
		#ifdef _NVR_
			#ifdef _ALL_HD_
			#define MAX_CHN_NUM			8//16//fpga:16 6446:4
			#else
			#define MAX_CHN_NUM			9//16//fpga:16 6446:4
			#endif
		#else
		#define MAX_CHN_NUM				8//16//fpga:16 6446:4
		#endif
		#define MAX_AUDIO_CHN_NUM		4
		#define SPLIT_NO				3//分割线的数量
	#endif
#elif defined(TL_9504)
#define MAX_CHN_NUM						4
#define SPLIT_NO						2
#endif

//pw 2010/06/11 将状态图标变成小图标
#define STATUS_PIC_HIGH					16
#define NEW_DDNS_UPDATE
#define NEW_PLAY_PROGRESS
#define SLIDER_DRAG_OPT
#define PLAY_BACKWARD_SUPPORT

#define VIDEO_BLOCK_LUMA_VALUE			0x00f00000
#define STREAM_TCP
#define AUDIO_TCP
//#define SUB_STREAM_TCP

#ifdef STREAM_TCP
#define STREAM_TCP_CHANNELS				MAX_CHN_NUM
#define SUB_STREAM_TCP
#endif

#ifdef SUB_STREAM_TCP
#define EACH_STREAM_TCP_LINKS			5
#endif

#define NEW_REMOTE_PLAYBACK
#define NEW_REMOTE_UPDATE
#define NEW_REMOTE_DOWNLOAD

#define MAX_RGN_NUM						4

#if 0
typedef struct _WndPos
{
	u32 x;/*左上角的x坐标*/
	u32 y;/*左上角的y坐标*/
}WndPos;

typedef struct _WndRect
{
	u32 x;/*左上角的x坐标*/
	u32 y;/*左上角的y坐标*/
	u32 w;/*矩形宽*/
	u32 h;/*矩形高*/
}WndRect;
#endif
#define NO_WAIT			0
#define WAIT_FOREVER    -1

#ifndef WIN32
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#endif

//pw 2010/6/10
#define PRINT(s) printf("FUNC:%s, LINE:%d, %s\n", __FUNCTION__, __LINE__, s);


#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define CONVERT_ENDIAN(x)  ((x)<<24 | (x)>>24 | ((x)&0x00FF0000)>>8 | ((x)&0x0000FF00)<<8)

#ifndef WIN32
/* Enables or disables debug output */
#ifdef __DEBUG
#define DBG(fmt, args...) fprintf(stderr, "Debug: " fmt, ## args)
#else
#define DBG(fmt, args...)
#endif

#define ERR(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
#else
#define DBG
#define ERR
#endif

#define  THREADAFFMASK	1

//FTP错误类型
#define FTP_SUCCESS						0//成功
#define FTP_ERROR_PARAM					1//参数错误
#define FTP_ERROR_SERVER				2//服务器不存在
#define FTP_ERROR_FILE					3//文件不存在
#define FTP_ERROR_USER					4//用户名不存在
#define FTP_ERROR_PASSWD				5//密码错误

#ifdef __cplusplus
extern "C" {
#endif

MSGQHANDLE IFly_CreateMsgQueue(u32 dwMsgNumber, u32 dwMsgLength);
void IFly_CloseMsgQueue(MSGQHANDLE hMsgQ);

int IFly_SndMsg(MSGQHANDLE hMsgQ, char *pchMsgBuf, u32 dwLen, int nTimeout);
int IFly_RcvMsg(MSGQHANDLE hMsgQ, char *pchMsgBuf, u32 dwLen, int nTimeout);

#ifdef WIN32
THREADHANDLE IFly_CreateThread(void* pvTaskEntry, char* szName, u8 byPriority, u32 dwStacksize, u32 dwParam, u16 wFlag, u32 *pdwTaskID);
#else
THREADHANDLE IFly_CreateThread(LINUXFUNC pvTaskEntry, char* szName, u8 byPriority, u32 dwStacksize, u32 dwParam, u16 wFlag, u32 *pdwTaskID);
#endif
BOOL IFly_ThreadExit();
BOOL IFly_ThreadTerminate(THREADHANDLE hTask);

void Dump_Thread_Info(char *name,int tid);

int OpenDev(char *Dev);
int set_Parity(int fd, int databits, int stopbits, int parity);
int set_speed(int fd, int speed);

time_t read_rtc(int utc);
void write_rtc(time_t t, int utc);
int show_clock(int utc);

u32 GetLocalIp();
u32 GetLocalIp2(char * name); //cyl added
int SetLocalIp(u32 dwIp);
int GetHWAddr(char *pBuf);
int SetHWAddr(char *pBuf);// wrchen 090403
u32 GetRemoteIP();// wrchen 091110
u32 GetNetMask();
u32 GetNetMask2(char * name);//cyl added
int SetNetMask(u32 dwIp);
u32 GetBroadcast();
int SetBroadcast(u32 dwIp);
u32 GetDefaultGateway();
int SetDefaultGateway(u32 dwIp);
u32 GetDNSServer();
int	SetDNSServer(u32 dwIp);
int AddDNSServer(u32 dwIp);

int ftpget(char *serverip,char *localfile,char *remotefile);
int ftpput(char *serverip,char *remotefile,char *localfile);

int mount_user(char *mounted_path,char *user_path);
int umount_user(char *user_path);

int wget_user(char *user_name,char *user_passwd,char *hostname,int typeflag);

int contact_with_ddnsserver(char *domain_name,char *passwd, int cmd_no);

//#ifdef JUNMINGSHI_KAICONG
int contact_with_ddnsserver_kaicong(char *domain_name, char *passwd, int cmd_no, char *user, u32 lanip, u32 wanip, u16 wport, u16 dport, u16 mport, u8 language);
//#endif

#ifdef PEOPLE_DDNS
int contact_with_ddnsserver2(char *domain_name,char *passwd,char * hwid);
#endif
BOOL CheckNetLink();

/* the arp packet to be sent*/
struct arp_struct{
	//以太网头
	unsigned char	dst_mac[6];
	unsigned char	src_mac[6];
	unsigned short	pkt_type;
	//arp数据
	unsigned short	hw_type;
	unsigned short	pro_type;
	unsigned char	hw_len;
	unsigned char	pro_len;
	unsigned short	arp_op;
	unsigned char	sender_eth[6];
	unsigned char	sender_ip[4];
	unsigned char	target_eth[6];
	unsigned char	target_ip[4];
};

int open_arp_socket();
int sendarppacket(int s,unsigned int dstIp,unsigned int srcIp);

#ifdef __cplusplus
}
#endif

#ifdef WIN32
typedef struct
{
	__int64 curpos;
	__int64 totalpos;		
}ifly_format_progress_t;
#else
typedef struct
{
	long long curpos;
	long long totalpos;		
}ifly_format_progress_t;
#endif

ifly_format_progress_t tFormat;

extern int g_vga_width;
extern int g_vga_height;
extern THREADHANDLE NTP_pthread;
#endif
