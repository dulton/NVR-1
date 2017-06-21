#ifndef _UI_H_
#define _UI_H_

#define USE_DEL_KEY
//#define _OLD_KERNEL_

#include "common.h"
#include "mouse.h"

//定义通道数
//#define _16CHANNELS_

//pw:2010/6/1
#ifdef TL_9508
#define _8CHANNELS_
#endif

//pw 2010/7/30
#ifdef TL_9504
#define _4CHANNELS_
#endif
#include "simplewidget.h"

#ifdef NEW_HARDWARE
//#define _SSY_
#endif

#define NO_NUMKEY
//#define NO_AUDIO

#ifdef USE_TTF
#define OSD_FONT_HEIGHT	16
#define OSD_FONT_WIDTH	(OSD_FONT_HEIGHT*3/4)
#else
#define OSD_FONT_HEIGHT	20
#define OSD_FONT_WIDTH	12
#endif

//cyl added
#define INPUT_3
#define INPUT_CH
//
typedef struct mouse {
    int x;
    int y;
    int w;
    int h;
    unsigned short *bufp;
    char * trans_mem;//cyl added for save transparency
} mouse;

typedef enum 
{
#ifdef NEW_HARDWARE
	IFLY_KEYCODE_1_4             = 30,
	IFLY_KEYCODE_5_8             = 31,
	#ifdef HANKE
	IFLY_KEYCODE_9_12            = 32,
	IFLY_KEYCODE_13_16           = 33,
	IFLY_KEYCODE_1_9             = 34,
	IFLY_KEYCODE_8_16            = 35,
	IFLY_KEYCODE_PIC8            = 36,
	IFLY_KEYCODE_PIC16           = 37,
	IFLY_KEYCODE_DISPLAY_CTL     = 38,
	IFLY_KEYCODE_PIC1_11         = 39,
	IFLY_KEYCODE_PIC1_12         = 40,
	IFLY_KEYCODE_PIC1_13         = 41,
	IFLY_KEYCODE_PIC1_14         = 42,
	IFLY_KEYCODE_PIC1_15         = 43,
	IFLY_KEYCODE_PIC1_16         = 45,//44 心跳
	#endif
	#ifdef _SSY_
	IFLY_KEYCODE_NUMERIC         = 47,
	#endif
	IFLY_KEYCODE_0               = 48,
	IFLY_KEYCODE_1               = 49,
	IFLY_KEYCODE_2               = 50,
	IFLY_KEYCODE_3               = 51,
	IFLY_KEYCODE_4               = 52,
	IFLY_KEYCODE_5               = 53,
	IFLY_KEYCODE_6               = 54,
	IFLY_KEYCODE_7               = 55,
	IFLY_KEYCODE_8               = 56,
	IFLY_KEYCODE_9               = 57,
	
	IFLY_KEYCODE_MENU            = 58,
	IFLY_KEYCODE_FN				 = 59,/*编辑*/
	IFLY_KEYCODE_SHIFT           = 60,/*输入法*/
	IFLY_KEYCODE_PLAY            = 61,
	IFLY_KEYCODE_RECORD          = 62,
	IFLY_KEYCODE_PTZ             = 63,/*云镜*/
	IFLY_KEYCODE_MULTI           = 64,/*多画面*/
	IFLY_KEYCODE_VOIP            = 65,
	
	IFLY_KEYCODE_UP              = 66,
	IFLY_KEYCODE_DOWN            = 67,
	IFLY_KEYCODE_LEFT            = 68,
	IFLY_KEYCODE_RIGHT           = 69,
	
	IFLY_KEYCODE_ENTER           = 70,
	IFLY_KEYCODE_ESC             = 71,
	
	IFLY_KEYCODE_MUTE            = 72,
	IFLY_KEYCODE_CLEAR           = 73,
	IFLY_KEYCODE_VGA             = 74,
	
	IFLY_KEYCODE_PAUSE           = 75,
	
	IFLY_KEYCODE_FASTFORWARD     = 76,
	IFLY_KEYCODE_SLOWFORWARD     = 77,
	IFLY_KEYCODE_FASTBACK        = 78,
	
	IFLY_KEYCODE_PREV			 = 79,
	IFLY_KEYCODE_NEXT            = 80,
	
	IFLY_KEYCODE_POWER           = 81,

	IFLY_KEYCODE_VOL_MINUS                             =    84,             //声音－
	IFLY_KEYCODE_VOL_PLUS                             =      85,            //声音＋

	IFLY_KEYCODE_STOP  = 91,   	//stop键 
	IFLY_KEYCODE_STEP  = 28, 

	IFLY_KEYCODE_PTZ_ZOOM_PLUS                    =    92,             //云台zoom＋
	IFLY_KEYCODE_PTZ_ZOOM_MINUS                    =  93,              //云台zoom－
	IFLY_KEYCODE_PTZ_FOCUS_PLUS             =        94,            //云台focus＋
	IFLY_KEYCODE_PTZ_FOCUS_MINUS           =        95,             //云台focus－
	IFLY_KEYCODE_PTZ_IRIS_PLUS    			=	96,              //云台光圈＋
	IFLY_KEYCODE_PTZ_IRIS_MINUS  			=	97,              //云台光圈－
	IFLY_KEYCODE_PTZ_CRUISE  = 27,

	IFLY_KEYCODE_PLAYOVER        = 100,
	IFLY_KEYCODE_PROGRESS_RATE	 = 101,
	IFLY_KEYCODE_PROGRESS_RATIO  = 102,
	IFLY_KEYCODE_PROGRESS_HOURS  = 103,
	IFLY_KEYCODE_PROGRESS_MINUTES= 104,
	IFLY_KEYCODE_PROGRESS_SECONDS= 105,
	IFLY_KEYCODE_PROGRESS_TOTAL	 = 106,
	IFLY_KEYCODE_PLAYSTART       = 107,
	IFLY_KEYCODE_PLAYOVER_TIME   = 108,
	
	#ifdef _NVR_
	IFLY_KEYCODE_NVRCHN_STATUSUPDATE = 109,
	#endif
	
	IFLY_KEYCODE_FS_OL   = 112,   	//外圈逆时针，定义为退放
	IFLY_KEYCODE_FS_OR   = 113,   	//外圈顺时针，定义为进放
	IFLY_KEYCODE_FS_IR   = 114,  	//内圈逆时针，定义为上段
	IFLY_KEYCODE_FS_IL   = 115,  	//内圈顺时针，定义为下段
	IFLY_KEYCODE_10      = 116,     //数字键10
	IFLY_KEYCODE_10PLUS  = 117,    	//数字键10+
	IFLY_KEYCODE_YKENTER = 135,     //wangcong jiuan 遥控器enter键
	//pw 2010/8/26
	IFLY_KEYCODE_LOCKOUT = 132,
	IFLY_KEYCODE_REBOOT = 140,//wangcong reboot
	IFLY_KEYCODE_DATA_SCREEN = 141,//wangcong ysj
#else
	IFLY_KEYCODE_0               = 0x373008ff,
	IFLY_KEYCODE_1               = 0x383108ff,
	IFLY_KEYCODE_2               = 0x393208ff,
	IFLY_KEYCODE_3               = 0x3a3308ff,
	IFLY_KEYCODE_4               = 0x3b3408ff,
	IFLY_KEYCODE_5               = 0x3c3508ff,
	IFLY_KEYCODE_6               = 0x3d3608ff,
	IFLY_KEYCODE_7               = 0x3e3708ff,
	IFLY_KEYCODE_8               = 0x3f3808ff,
	IFLY_KEYCODE_9               = 0x403908ff,
	
	IFLY_KEYCODE_MENU            = 0x413a08ff,
	IFLY_KEYCODE_FN				 = 0x423b08ff,/*编辑*/
	IFLY_KEYCODE_SHIFT           = 0x433c08ff,/*输入法*/
	IFLY_KEYCODE_PLAY            = 0x443d08ff,
	IFLY_KEYCODE_RECORD          = 0x453e08ff,
	IFLY_KEYCODE_PTZ             = 0x463f08ff,/*云镜*/
	IFLY_KEYCODE_MULTI           = 0x474008ff,/*多画面*/
	IFLY_KEYCODE_VOIP            = 0x484108ff,
	
	IFLY_KEYCODE_UP              = 0x4a4308ff,
	IFLY_KEYCODE_DOWN            = 0x494208ff,
	IFLY_KEYCODE_LEFT            = 0x4b4408ff,
	IFLY_KEYCODE_RIGHT           = 0x4c4508ff,
	
	IFLY_KEYCODE_ENTER           = 0x4d4608ff,
	IFLY_KEYCODE_ESC             = 0x4e4708ff,
	
	IFLY_KEYCODE_MUTE            = 0x4f4808ff,
	IFLY_KEYCODE_CLEAR           = 0x504908ff,
	IFLY_KEYCODE_VGA             = 0x514a08ff,
	
	IFLY_KEYCODE_PAUSE           = 0x524b08ff,
	
	IFLY_KEYCODE_FASTFORWARD     = 0x534c08ff,
	IFLY_KEYCODE_SLOWFORWARD     = 0x544d08ff,
	IFLY_KEYCODE_FASTBACK        = 0x554e08ff,
	
	IFLY_KEYCODE_PREV			 = 0x564f08ff,
	IFLY_KEYCODE_NEXT            = 0x575008ff,
	
	IFLY_KEYCODE_POWER           = 0x585108ff
#endif
}ifly_keycode;

#define RUN_LIGHT_CHN1		0x1
#define RUN_LIGHT_CHN2		0x2
#define RUN_LIGHT_CHN3		0x4
#define RUN_LIGHT_CHN4		0x8
#ifdef NEW_HARDWARE
#define RUN_LIGHT_VGA		0x10
#else
#define RUN_LIGHT_VGA		0x40
#endif
#define RUN_LIGHT_ALARM		0x20
#ifdef _SSY_
#define RUN_LIGHT_NUMERIC	(u16)(~RUN_LIGHT_CHN1)
#endif

#define MAX_WND_NAME_LEN	64
#define MAX_CTRL_NUM        256

/* Global data structure */
typedef struct _GlobalData
{
    //u16 *display; /* Pointer to the screen frame buffer */
    s32 yFactor;    /* Vertical scaling factor (PAL vs. NTSC) */
}GlobalData;

extern GlobalData gbl;

int getOsdYFactor();
int setOsdYFactor(int yFac);

/* Screen dimensions */
#define yScale(x)						(((x) * gbl.yFactor) / 10)
#define SCREEN_WIDTH					720
#define SCREEN_HEIGHT					yScale(480)
#define SCREEN_SIZE						(SCREEN_WIDTH * SCREEN_HEIGHT * SCREEN_BPP / 8)

#define OSD_WIDTH						720
#define OSD_HEIGHT						((480 * gbl.yFactor) / 10)

//数字
#define INPUTMODE_NUM					0x0001
//小写字母 
#define INPUTMODE_LOWER				0x0002
//大写字母
#define INPUTMODE_UPPER				0x0004
//符号
#define INPUTMODE_SIGN					0x0008
//中文
#define INPUTMODE_CHINESE				0x0010
//MAC
#define INPUTMODE_MAC					0x0020
//#define INPUTMODE_SECTION				0x0010

#define IDD_DIALOG_LOGIN				1000
#define IDD_DIALOG_MAINMENU				1100
#ifdef MEIDIANBEIER_UI
#define IDC_PICTURE_MAINMENU_MENUPIC_BASE	 IDD_DIALOG_MAINMENU+20
#endif

#define IDD_DIALOG_SYSTEM				1300

#define IDD_DIALOG_TOOL					1600
#define IDD_DIALOG_SUPPORT				1650

#define IDD_DIALOG_RESUME				1900
#define IDD_DIALOG_HDD					2200
#define IDD_DIALOG_ONLINE				2500
#define IDD_DIALOG_SYSTEMPARAM			2800
#define IDD_DIALOG_UPDATE				3100
#define IDD_DIALOG_UPDATESUCC			3200
#define IDD_DIALOG_UPDATEPROG			3300			
#define IDD_DIALOG_PASSWD				3400
#define IDD_DIALOG_RESTART				3700
#define IDD_DIALOG_SYSINFO				4000
#define IDD_DIALOG_LOG					4300
#define IDD_DIALOG_USERMANAGE			4600
#define IDD_DIALOG_DELUSER				4900
#define IDD_DIALOG_EDITUSER				5200
#define IDD_DIALOG_LAYOUT				5500
#define IDD_DIALOG_NETWORK				5800
#define IDD_DIALOG_EXCEPTION			6100
#define IDD_DIALOG_YUNTAI				6400
#define IDD_DIALOG_ALARMTIME			6700
#define IDD_DIALOG_PTZ					7000
#define IDD_DIALOG_ALARMDISPOSE			7300
#define IDD_DIALOG_ALARMIN				7600

#define IDD_DIALOG_ALARMOUT			7700

#define IDD_DIALOG_IMAGEALARM			7900
#define IDD_DIALOG_IMAGEDETECTTIME		8200
#define IDD_DIALOG_PLAY					8500

#define IDD_DIALOG_SEARCH_RESULT				8600
#ifdef BACKUP_AS_AVI
#define IDD_DIALOG_BACKUPFORMAT			8700
#endif
#define IDD_DIALOG_IMAGESET				8800
#define IDD_DIALOG_CHNSTATE				9100

//yqluo 20101023
#define IDD_DIALOG_MAINTAIN				21000
//#define IDC_BUTTON_MAINTAINTIME_OK		24000
//#define IDC_BUTTON_MAINTAINTIME_CANCEL		27000

#define IDD_DIALOG_RECORDSET			9400
#define IDD_DIALOG_RECORDCONF			9700
#define IDD_DIALOG_FORMAT				9900

#define IDD_STATIC_IMAGESETBASE				9000
#define IDD_STATIC_IMAGESETBRIGHTNESS   IDD_STATIC_IMAGESETBASE+1
#define IDD_STATIC_IMAGESETCONTRAST   IDD_STATIC_IMAGESETBASE+2
#define IDD_STATIC_IMAGESETHUE   IDD_STATIC_IMAGESETBASE+3
#define IDD_STATIC_IMAGESETSATURATION   IDD_STATIC_IMAGESETBASE+4

#define IDD_DIALOG_LOCATION				10000
#define IDC_LOCATION_STR				IDD_DIALOG_LOCATION+1

#define IDD_DIALOG_ENVELOP				10300
#define IDC_REGION_ENVELOP				IDD_DIALOG_ENVELOP+1

#define IDD_DIALOG_MDAREA				10600
#define IDC_PARTITION_MDAREA			IDD_DIALOG_MDAREA+1

/*comment by lshu 20070320*/
#define IDD_DIALOG_YTPRESET             10900
#define IDD_DIALOG_YTTRACK				11200
#define IDD_DIALOG_YTCRUISE             11500
#define IDD_DIALOG_YUNTAICTRLBASE		11800
#define IDD_DIALOG_YUNTAICTRL			IDD_DIALOG_YUNTAICTRLBASE+0
#define IDD_DIALOG_YUNTAICTRLMOVE		IDD_DIALOG_YUNTAICTRLBASE+16
#define IDD_DIALOG_YUNTAICTRLZOOM		IDD_DIALOG_YUNTAICTRLBASE+32
#define IDD_DIALOG_YUNTAICTRLFOCUS		IDD_DIALOG_YUNTAICTRLBASE+48
#define IDD_DIALOG_YUNTAICTRLIRIS		IDD_DIALOG_YUNTAICTRLBASE+64
#define IDD_DIALOG_YUNTAICTRLWIPER		IDD_DIALOG_YUNTAICTRLBASE+80
#define IDD_DIALOG_YUNTAICTRLLIGHT		IDD_DIALOG_YUNTAICTRLBASE+96
#define IDD_DIALOG_YUNTAICTRLAUX		IDD_DIALOG_YUNTAICTRLBASE+112
#define IDD_DIALOG_YUNTAICTRLPRESET		IDD_DIALOG_YUNTAICTRLBASE+128
#define IDD_DIALOG_YUNTAICTRLCRUISE		IDD_DIALOG_YUNTAICTRLBASE+144
/*comment end*/

/*comment by lshu 20070521*/
#define IDD_DIALOG_PASSWDSET			12100
#define IDD_DIALOG_SYSDATETIME			1240
#ifdef channel_limit
#define IDD_DIALOG_CHNLIMIT             12400
#endif
//pw: 2010/6/24 DST
#define IDD_DIALOG_DST					12500

/*comment end*/

/***************************/
//added by kmli 2007-05-21
#define IDD_DIALOG_MODIFYUSER			12700
#define IDD_DIALOG_DELETEUSER			13000
/***************************/

//added by wrchen 2007-11-12
#define IDC_CAP_BASE					14000
#define IDC_TEXT_BASE					14500

//cyl added 系统设置
#define IDD_DIALOG_SYSTEMCONF			19000

#define IDD_DIALOG_TIMEDREBOOT			19500

#ifdef _NVR_
#define IDD_DIALOG_IPNC					19800
#define IDD_DIALOG_NVRCHN				19850
#define IDD_DIALOG_IPNCNET				19900
#define IDD_DIALOG_IPNCSET				19950
#define IDD_DIALOG_IPNCINFO				19970
#endif

#define	IDC_CAP_ABNOR					IDC_CAP_BASE+1
#define	IDC_CAP_MANULREC				IDC_CAP_BASE+2
#define IDC_CAP_NETSET					IDC_CAP_BASE+3
#define IDC_CAP_PTZSET					IDC_CAP_BASE+4
#define IDC_CAP_HDFORMAT				IDC_CAP_BASE+5
#define IDC_CAP_RECSET					IDC_CAP_BASE+6
#define IDC_CAP_RECTIMESET				IDC_CAP_BASE+7
#define IDC_CAP_SEARCHFAIL				IDC_CAP_BASE+8
#define IDC_CAP_CHANGQUERYCON			IDC_CAP_BASE+9
#define IDC_CAP_TIMEPLAYBAK				IDC_CAP_BASE+10
#define IDC_CAP_JUMPFAIL				IDC_CAP_BASE+11
#define IDC_CAP_BAKCURREC				IDC_CAP_BASE+12
#define IDC_CAP_REBOOT					IDC_CAP_BASE+13
#define IDC_CAP_HDMANA					IDC_CAP_BASE+14
#define IDC_CAP_MANATOOL				IDC_CAP_BASE+15
#define IDC_CAP_BAKREC					IDC_CAP_BASE+16
#define IDC_CAP_CURCHOOSEALL			IDC_CAP_BASE+17
#define IDC_CAP_SOFTSHUTDOWN			IDC_CAP_BASE+18
#define IDC_CAP_LOGIN					IDC_CAP_BASE+19
#define IDC_CAP_LOCALPLAY				IDC_CAP_BASE+20
#define IDC_CAP_MANNLREC				IDC_CAP_BASE+21
#define IDC_CAP_CURALLNOCHOOSE			IDC_CAP_BASE+22
#define IDC_CAP_PTZCONTROL				IDC_CAP_BASE+23
#define IDC_CAP_CLEANCALL				IDC_CAP_BASE+24
#define IDC_CAP_SOFTUPDATE				IDC_CAP_BASE+25
#define IDC_CAP_DISPOSETIMESET			IDC_CAP_BASE+26
#define IDC_CAP_IMAGESET				IDC_CAP_BASE+27
#define IDC_CAP_CALLSET					IDC_CAP_BASE+28
#define IDC_CAP_CALLINSET				IDC_CAP_BASE+29
#define IDC_CAP_CALLOUTSET				IDC_CAP_BASE+30
#define IDC_CAP_CALLTIMESET				IDC_CAP_BASE+31
#define IDC_CAP_LOGQUERY				IDC_CAP_BASE+32
#define IDC_CAP_USERMANA				IDC_CAP_BASE+33
#define IDC_CAP_SYSTIME					IDC_CAP_BASE+34
#define IDC_CAP_MODIPWD					IDC_CAP_BASE+35
#define IDC_CAP_SEARCHLOG				IDC_CAP_BASE+36
#define IDC_CAP_MODIUSER				IDC_CAP_BASE+37
#define IDC_CAP_DELUSER					IDC_CAP_BASE+38
#define IDC_CAP_ADDUSER					IDC_CAP_BASE+39
#define IDC_CAP_PRENODSET				IDC_CAP_BASE+40
#define IDC_CAP_CRUISEPATHRANG			IDC_CAP_BASE+41
#define IDC_CAP_CRUISENODRANG			IDC_CAP_BASE+42
#define IDC_CAP_PRENODRANG				IDC_CAP_BASE+43
#define IDC_CAP_SYSPARA					IDC_CAP_BASE+44
#define IDC_CAP_ONLINEUSER				IDC_CAP_BASE+45
#define IDC_CAP_RESUMESET				IDC_CAP_BASE+46
#define IDC_CAP_EXPORTINPORT			IDC_CAP_BASE+47
//#define IDC_CAP_REBOOTING				IDC_CAP_BASE+48
#define IDC_CAP_SYSINFO					IDC_CAP_BASE+49
#define IDC_CAP_KEYLOCK					IDC_CAP_BASE+50

#define IDC_CAP_CRUISESET				IDC_CAP_BASE+51
#define IDC_CAP_TRACKSET				IDC_CAP_BASE+52
#define IDC_CAP_DELNOD					IDC_CAP_BASE+53
#define IDC_CAP_DATETIME				IDC_CAP_BASE+54

#define IDC_CAP_INIT					IDC_CAP_BASE+54

#define IDC_CAP_CHECKDISK				IDC_CAP_BASE+56

#if 0
//pw 2010/6/21
#define IDC_CAP_IMPORTINPORT			IDC_CAP_BASE+57
#endif

#define IDC_TEXT_NOSETPOWER				IDC_TEXT_BASE+1
#define IDC_TEXT_HDERROR				IDC_TEXT_BASE+2
#define IDC_TEXT_PPWDERROR				IDC_TEXT_BASE+3
#define IDC_TEXT_NOPOWER				IDC_TEXT_BASE+4
#define IDC_TEXT_FINISHCHCP				IDC_TEXT_BASE+5
#define IDC_TEXT_NOHD					IDC_TEXT_BASE+6
#define IDC_TEXT_RECODING				IDC_TEXT_BASE+7
#define IDC_TEXT_HDBUSY					IDC_TEXT_BASE+8
#define IDC_TEXT_FORMATING				IDC_TEXT_BASE+9
#define IDC_TEXT_FMTFAIL				IDC_TEXT_BASE+10
#define IDC_TEXT_FMTSUC					IDC_TEXT_BASE+11
#define IDC_TEXT_TIMEOVERLAP			IDC_TEXT_BASE+12
#define IDC_TEXT_TIMEMULLITY			IDC_TEXT_BASE+13
#define IDC_TEXT_FINISHSETCP			IDC_TEXT_BASE+14
#define IDC_TEXT_FILETOOMANY			IDC_TEXT_BASE+15
#define IDC_TEXT_NOSEARCH				IDC_TEXT_BASE+16
#define IDC_TEXT_SEARCHFIRAT			IDC_TEXT_BASE+17
#define IDC_TEXT_RIGHTPAGE				IDC_TEXT_BASE+18
#define IDC_TEXT_NOUSB					IDC_TEXT_BASE+19
#define IDC_TEXT_BAKTIMEERROR			IDC_TEXT_BASE+20
#define IDC_TEXT_NOFILE					IDC_TEXT_BASE+21
#define IDC_TEXT_BAKUPING				IDC_TEXT_BASE+22
#define IDC_TEXT_EQUIPERROR				IDC_TEXT_BASE+23
#define IDC_TEXT_BAKUPFAIL				IDC_TEXT_BASE+24
#define IDC_TEXT_BAKUPSUC				IDC_TEXT_BASE+25
#define IDC_TEXT_LINKUSBERR				IDC_TEXT_BASE+26
#define IDC_TEXT_BAKRECING				IDC_TEXT_BASE+27
#define IDC_TEXT_NOCHOFILE				IDC_TEXT_BASE+28
#define IDC_TEXT_NORECFILE				IDC_TEXT_BASE+29
#define IDC_TEXT_LOGFIRST				IDC_TEXT_BASE+30
#define IDC_TEXT_SYSSHUTDOWNING			IDC_TEXT_BASE+31
#define IDC_TEXT_PWDERROR				IDC_TEXT_BASE+32
#define IDC_TEXT_CONFFILEERR			IDC_TEXT_BASE+33
#define IDC_TEXT_CALLCLEANSUC			IDC_TEXT_BASE+34
#define IDC_TEXT_RECINGNOUPDATE			IDC_TEXT_BASE+35
#define IDC_TEXT_FIRSTSTOPMO			IDC_TEXT_BASE+36
#define IDC_TEXT_FIRSTSTOPSHE			IDC_TEXT_BASE+37
#define IDC_TEXT_FIRSTSTOPLOSE			IDC_TEXT_BASE+38
#define IDC_TEXT_NOROOM					IDC_TEXT_BASE+39
#define IDC_TEXT_SYSUPDATEING			IDC_TEXT_BASE+40
#define IDC_TEXT_FILECHECKERR			IDC_TEXT_BASE+41
#define IDC_TEXT_UPSUCREBOOOT			IDC_TEXT_BASE+42
#define IDC_TEXT_NOSYSROOM				IDC_TEXT_BASE+43
#define IDC_TEXT_OPENFLASHFAIL			IDC_TEXT_BASE+44
#define IDC_TEXT_GETFLASHFAIL			IDC_TEXT_BASE+45
#define IDC_TEXT_ERASEFLASHFAIL			IDC_TEXT_BASE+46
#define IDC_TEXT_WRITEFLASHFAIL			IDC_TEXT_BASE+47
#define IDC_TEXT_FLASHCHECKFAIL			IDC_TEXT_BASE+48
#define IDC_TEXT_FILEERR				IDC_TEXT_BASE+49
#define IDC_TEXT_CONNSERVERERR			IDC_TEXT_BASE+50
//#define IDC_TEXT_NOFILE				IDC_TEXT_BASE+51
#define IDC_TEXT_USBNOFILE				IDC_TEXT_BASE+52
#define IDC_TEXT_FINISHCALLINSETCP		IDC_TEXT_BASE+53
#define IDC_TEXT_FINISHCALLOUTSETCP		IDC_TEXT_BASE+54
#define IDC_TEXT_NOTOOLPOWER			IDC_TEXT_BASE+55
#define IDC_TEXT_NOLOGPOWER				IDC_TEXT_BASE+56
#define IDC_TEXT_RECONDINGNOSETTIME		IDC_TEXT_BASE+57
#define IDC_TEXT_PWDERR					IDC_TEXT_BASE+58
#define IDC_TEXT_NOLOG					IDC_TEXT_BASE+59
#define IDC_TEXT_CURISHEAD				IDC_TEXT_BASE+60
#define IDC_TEXT_CURISTRAIL				IDC_TEXT_BASE+61
#define IDC_TEXT_INPUTRINGTPAGE			IDC_TEXT_BASE+62
#define IDC_TEXT_NOUSERMODIFILY			IDC_TEXT_BASE+63
#define IDC_TEXT_NOUSERDEL				IDC_TEXT_BASE+64
#define IDC_TEXT_CUREXIST				IDC_TEXT_BASE+65
#define IDC_TEXT_USERNAMENULL			IDC_TEXT_BASE+66
#define IDC_TEXT_ISMAX					IDC_TEXT_BASE+67
#define IDC_TEXT_PRESETRAGEERR			IDC_TEXT_BASE+68
#define IDC_TEXT_PRESETNUMERR			IDC_TEXT_BASE+69
#define IDC_TEXT_ADDNOD					IDC_TEXT_BASE+70
#define IDC_TEXT_DELNOD					IDC_TEXT_BASE+71
#define IDC_TEXT_CRUISESTART			IDC_TEXT_BASE+72
#define IDC_TEXT_CRUISEEND				IDC_TEXT_BASE+73
#define IDC_TEXT_OLDPWDERR				IDC_TEXT_BASE+74
#define IDC_TEXT_PWDDIFF				IDC_TEXT_BASE+75
#define IDC_TEXT_PWDMODISUC				IDC_TEXT_BASE+76
#define IDC_TEXT_NOCONNUSB				IDC_TEXT_BASE+77
#define IDC_TEXT_NOCONFFILE				IDC_TEXT_BASE+78
#define IDC_TEXT_EQUIPNODRAW			IDC_TEXT_BASE+79
#define IDC_TEXT_EXPORTSUC				IDC_TEXT_BASE+80
#define IDC_TEXT_EQUIPNUMOVERFLOW		IDC_TEXT_BASE+81
#define IDC_TEXT_TYPESUCROBOOT			IDC_TEXT_BASE+82
#define IDC_TEXT_KEYBLOCK				IDC_TEXT_BASE+83
#define IDC_TEXT_OK						IDC_TEXT_BASE+84
#define IDC_TEXT_BITRATE				IDC_TEXT_BASE+85
#define IDC_TEXT_BITRATEMAX				IDC_TEXT_BASE+86
#define IDC_TEXT_SETSUCREBOOT			IDC_TEXT_BASE+87
#define IDC_TEXT_FINISHCONFREBOOT		IDC_TEXT_BASE+88
#define IDC_TEXT_INPORTEFAIL			IDC_TEXT_BASE+89
#define IDC_TEXT_LOWERCASE				IDC_TEXT_BASE+90
#define IDC_TEXT_CAPITAL				IDC_TEXT_BASE+91
#define IDC_TEXT_SIGN					IDC_TEXT_BASE+92
#define IDC_TEXT_FIGURE					IDC_TEXT_BASE+93
#define IDC_TEXT_CHTEXT					IDC_TEXT_BASE+94
#define IDC_TEXT_CHTEXT1				IDC_TEXT_BASE+95
#define IDC_TEXT_CHTEXT2				IDC_TEXT_BASE+96
#define IDC_TEXT_CHTEXT3				IDC_TEXT_BASE+97
#define IDC_TEXT_CHTEXT4				IDC_TEXT_BASE+98
#define IDC_TEXT_MONDAY					IDC_TEXT_BASE+99
#define IDC_TEXT_TUESDAY				IDC_TEXT_BASE+100
#define IDC_TEXT_WEDNESDAY				IDC_TEXT_BASE+101
#define IDC_TEXT_THURSDAY				IDC_TEXT_BASE+102
#define IDC_TEXT_FRIDAY					IDC_TEXT_BASE+103
#define IDC_TEXT_SATURDAY				IDC_TEXT_BASE+104
#define IDC_TEXT_SUNDAY					IDC_TEXT_BASE+105
#define IDC_TEXT_FORMAT					IDC_TEXT_BASE+106
#define IDC_TEXT_RECBAKUP				IDC_TEXT_BASE+107
#define IDC_TEXT_SYSUPDATE				IDC_TEXT_BASE+108

#define IDC_TEXT_PRESETSAVE				IDC_TEXT_BASE+109
#define IDC_TEXT_PRESETDEL				IDC_TEXT_BASE+110
#define IDC_TEXT_CRUISEADD				IDC_TEXT_BASE+111
#define IDC_TEXT_CRUISEDEL				IDC_TEXT_BASE+112
#define IDC_TEXT_TRACKREC				IDC_TEXT_BASE+113
#define IDC_TEXT_TRACKRECEND			IDC_TEXT_BASE+114
#define IDC_TEXT_TRACKSTART				IDC_TEXT_BASE+115
#define IDC_TEXT_TRACKEND				IDC_TEXT_BASE+116
#define IDC_TEXT_ONLINEUSERBREAK		IDC_TEXT_BASE+117
#define IDC_TEXT_SYSTIMECHANGED			IDC_TEXT_BASE+118

#define IDC_TEXT_CRUISENODRANG			IDC_TEXT_BASE+119				//ddz
#define IDC_TEXT_CRUISEPATHRANG			IDC_TEXT_BASE+120			//ddz

#define IDC_TEXT_DVRINIT				IDC_TEXT_BASE+121

#define IDC_TEXT_RECINFOSWITCH			IDC_TEXT_BASE+123	//06-11

#define IDC_TEXT_PLAYCHN				IDC_TEXT_BASE+124


#define IDC_TEXT_CHTEXT1_2				IDC_TEXT_BASE+127
#define IDC_TEXT_UPDATE_HDD				IDC_TEXT_BASE+128
#define IDC_CAP_SOFTUPDATE_FTP		IDC_TEXT_BASE+129

#define IDC_TEXT_NODISK				IDC_TEXT_BASE+130

#define IDC_TEXT_SOUND				IDC_TEXT_BASE+131//zlb20090831
#define IDC_TEXT_MUTE				IDC_TEXT_BASE+132//zlb20090831

#define IDC_TEXT_NORECTYPE			IDC_TEXT_BASE+133

#define IDC_TEXT_NORECCHN			IDC_TEXT_BASE+134

#define IDC_TEXT_SETPORTREBOOT		IDC_TEXT_BASE+135

//pw 2010/6/21
#define IDC_TEXT_IMPORTSUC				IDC_TEXT_BASE+136

//pw 2010/7/31
#define IDC_TEXT_APPLYING		IDC_TEXT_BASE+140//20100621 zlb_las
#define IDC_TEXT_APPLY_SUCCESS		IDC_TEXT_BASE+141
#define IDC_TEXT_APPLY_FAILED		IDC_TEXT_BASE+142
#define IDC_TEXT_DISKERROR				IDC_TEXT_BASE+143			//dong	HANKE_HDD_ERROR

//pw 2010/8/24
#define IDC_TEXT_RESOLUTIONSWITCH 	IDC_TEXT_BASE+144

#define IDC_TEXT_TYPERATIOREBOOT 	IDC_TEXT_BASE+145 //wangcong 201114

#ifdef JUNMINGSHI_DDNS
#define IDC_TEXT_CANCEL_SUCCESS       IDC_TEXT_BASE+146
#define IDC_TEXT_CANCEL_FAILED		   IDC_TEXT_BASE+147	
#define IDC_TEXT_CANCELING  			IDC_TEXT_BASE+148
#endif

#ifdef JUNMINGSHI
#define IDC_TEXT_ONOFFKEY  			IDC_TEXT_BASE+149
#endif

#ifdef _NVR_
#define IDC_TEXT_NOIPNCSELECT			IDC_TEXT_BASE+150
#define IDC_TEXT_IPNC_SETIP_FAILED		IDC_TEXT_BASE+151
#define IDC_TEXT_IPNC_SETIP_SUCC		IDC_TEXT_BASE+152
#define IDC_TEXT_IPNC_IPERROR			IDC_TEXT_BASE+153
#endif

#define IDD_DIALOG_BACKUPPROG   		15000

#define IDD_DIALOG_START				15400
#define IDD_DIALOG_TASKBAR				15500

#define IDD_DIALOG_START_TV				15600
#define IDD_DIALOG_TASKBAR_TV			15700

#define IDD_DIALOG_HDDFORMATPROG   		16000
#define IDD_DIALOG_PASSWDSET2			17000

#define IDD_DIALOG_PLAY_LOADFILE		17500
#define IDD_DIALOG_PLAY_PLAYING			17600
#define IDD_DIALOG_PLAY_BAKUP_CHN					17800

#define IDD_DIALOG_PLAY_CDRECORD					17900

#define IDD_DIALOG_PLAYTIME_CHN						18000		//08731
#ifdef ALARMSCH
#define IDD_DIALOG_ALARMSCH 15100		//ALARMSCH hxd09102010
#define IDD_DIALOG_MDSCH 15200
#endif
#define IDD_DIALOG_SCHEDULE	19100

#define IDD_DIALOG_VIDEODETECT	19200

#define IDD_DIALOG_ETHER	19300
#define IDD_DIALOG_PPPOE	19400
#define IDD_DIALOG_DDNS	19600
#define IDD_DIALOG_APPLY_DDNS	19650	
#define OSD_WIDTH						720
#define OSD_HEIGHT						((480 * gbl.yFactor) / 10)
#define OSD_SIZE						(OSD_WIDTH * OSD_HEIGHT * SCREEN_BPP / 8)

typedef enum _WndType
{
	TYPE_DIALOG=0,/*对话框*/
	TYPE_PICTURE,/*可加载jpg和png文件*/
	TYPE_STATICTEXT,/*静态控件*/
	TYPE_EDITBOX,/*编辑框*/
	TYPE_BUTTON,/*按钮*/
	TYPE_LISTBOX,/*列表框*/
	TYPE_COMBOBOX,/*组合框*/
	TYPE_CHECKBOX,/*复选框*/
	TYPE_PROGRESS,/*进度条*/
	TYPE_SLIDER,/*滑动条*/
	TYPE_BAR,/*任务条*/
	TYPE_MESSAGEBOX,/*消息框*/
	TYPE_PARTITION,/*移动侦测区域*/
	TYPE_LOCATION,/*资源定位*/
	TYPE_REGION,/*遮盖区域*/
	TYPE_INPUTBOX,
	TYPE_DATETIMECTRL,
	TYPE_PWEDITBOX,/*PASSWORD编辑框20070410*/
	TYPE_IPMACADDRCTRL,/*ip*/
	TYPE_MENUITEM,//菜单按钮
}WndType;

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

typedef struct _message_event
{
	u32 event;//消息 (如果是面板消息.坐标无效)
	u16 x; //鼠标事件的坐标X的值.(经过转化的坐标.不是1024X768的坐标)
	u16 y;//鼠标事件的坐标X的值.(经过转化的坐标.不是1024X768的坐标)
}Message_event;
typedef struct _Wnd Wnd;

typedef BOOL (*FuncShowWindow)(Wnd* pWnd,BOOL bShow);
typedef s32  (*FuncGetFocus)(Wnd* pWnd);
typedef s32  (*FuncLoseFocus)(Wnd* pWnd);
typedef BOOL (*FuncDestroyWindow)(Wnd* pWnd);

struct _Wnd
{
	WndType type;/*窗体类型，如button或editbox等*/
	u32  id;/*控件id,唯一标识本控件*/
	char name[MAX_WND_NAME_LEN];/*窗体名称*/
	WndRect rect;/*坐标*/
	Wnd* parent;/*父窗体*/
	
	/*通用属性*/
	BOOL visible;/*可见性*/
	BOOL tabstop;/*该控件是否接受焦点*/
	
	/*公共消息处理函数*/
	FuncShowWindow ShowWindow;/*显示窗体*/
	FuncGetFocus GetFocus;/*得到焦点*/
	FuncLoseFocus LoseFocus;/*失去焦点*/
	FuncDestroyWindow DestroyWindow;/*删除窗体*/
};

typedef void (*MsgFxn)(Wnd* pWnd,Message_event event,s32 param);
typedef void (*initFxn)(Wnd* pWnd);
//typedef void (*MsgFxn)(s32 param);

//第一个参数为每一个控件的id
#define BEGIN_MSG_MAP(nID,msg,fxn,param) SetMsgMap(nID,msg,fxn,param)
#define BEGIN_MSG_UNMAP(nID,msg,fxn,param) SetMsgUnMap(nID,msg,fxn,param)
//#define BEGIN_MSG_MAP(nID,msg,fxn,param)
//#define BEGIN_MSG_UNMAP(nID,msg,fxn,param)

#ifdef __cplusplus
extern "C"
{
#endif


	int GetDaysNum(int year,int mon);
	
	/********************/
	//07-09-03
	void SetEditboxDisable(u32 nID,BOOL bEnable);
	/********************/

	void SetListboxDisable(u32 nID,BOOL bEnable);//07-11-12
	void SetPicDisable(u32 nID,BOOL bEnable);

	void SetButtonDisable(u32 nID,BOOL bEnable);
	void SetInputboxDisable(u32 nID,BOOL bEnable);
	
	u32  GetCurPage();
	
	BOOL ShowListBoxWnd(u32 nID,BOOL bShow);
	
	BOOL CreateWnd(WndType type,u32 nID,char* pchWndName,WndRect* pRect,u32 nParentID,BOOL visible);
	BOOL ShowWnd(u32 nID,BOOL bShow);
	BOOL DestroyWnd(u32 nID);
	
	BOOL GetWndText(u32 nID,char *pch,int len);
	BOOL SetWndText(u32 nID,char *pch);
	BOOL SetWndText2(u32 nID,s32 nID2);
	BOOL SetWndText3(u32 nID,char *pch);
	
	BOOL SetWndTabStop(u32 nID,BOOL bStop);
	
	Wnd* GetWndItem(u32 nID);
	
	BOOL GetCheckBoxSelect(u32 nID,BOOL *pbSel);
	BOOL SetCheckBoxSelect(u32 nID,BOOL bSel);
	
	BOOL AddComboBoxItem(s32 nID,char *pchStr);
	BOOL AddComboBoxItem2(s32 nID,s32 nID2);
	BOOL ClearComboBoxItem(s32 nID);
	BOOL GetComboBoxSelect(s32 nID,s32 *pnSel);
	BOOL GetComboBoxPreSelect(s32 nID,s32 *pnSel);
	BOOL SetComboBoxSelect(s32 nID,s32 nSel);
	s32  GetComboBoxItemCount(s32 nID);
	
	BOOL AddListBoxItem(s32 nID,char *pchStr);
	BOOL ClearListBoxItem(s32 nID);
	BOOL DelListBoxItem(s32 nID,s32 nIndex);
	BOOL GetListBoxSelect(s32 nID,s32 *pnSel);
	BOOL GetListBoxStatus(s32 nID,s32 *pnSel);
	BOOL GetListBoxItemContext(s32 nID,s32 nIndex,char *pStr);
	BOOL SetListBoxItemContext(s32 nID,s32 nIndex,char *pStr);
	BOOL SetListBoxSelect(s32 nID,s32 nSel);
	s32  GetListBoxItemCount(s32 nID);
	
	BOOL SetProgressRange(s32 nID,s32 nLower, s32 nUpper);
	BOOL GetProgressRange(s32 nID,s32 *pnLower, s32 *pnUpper);
	s32  GetProgressPos(s32 nID);
	s32  SetProgressPos(s32 nID,s32 nPos);
	s32  OffsetProgressPos(s32 nID,s32 nPos);
	s32  SetProgressStep(s32 nID,s32 nStep);
	s32  ProgressStepIt(s32 nID);
	
	BOOL SetSliderRange(s32 nID,s32 nLower, s32 nUpper);
	BOOL GetSliderRange(s32 nID,s32 *pnLower, s32 *pnUpper);
	s32  GetSliderPos(s32 nID);
	s32  SetSliderPos(s32 nID,s32 nPos);
	s32 SetSliderPos2(s32 nID,s32 nPos);
	s32  OffsetSliderPos(s32 nID,s32 nPos);
	s32  SetSliderStep(s32 nID,s32 nStep);
	s32  SliderStepIt(s32 nID);
	
	BOOL SetPicToBar(s32 nID,char *file,s32 x,s32 y);
	BOOL SetTextToBar(s32 nID,char *txt,s32 x,s32 y);
	
	BOOL GetLocationPos(s32 nID,WndPos *pPos);
	BOOL SetLocationPos(s32 nID,WndPos *pPos);
	void SetLocationFontHeight(s32 nID,int font_h);
	
	BOOL GetPartitionArea(s32 nID,char *pflag,int num);
	void SetPartitionArea(s32 nID,char *pflag,int num);
	
	BOOL GetRegionRect(s32 nID,WndRect *pRect,u8 *pNum);
	BOOL SetRegionRect(s32 nID,WndRect *pRect,u8 num);
	
	BOOL ShowMsgBox(s32 nID,s32 nID2,WndRect* pRect,BOOL bSure);
	void setTmpDlgId(u32 id);
	BOOL SetMsgBoxText(char *pchText);
	
	BOOL GetKeyValue(ifly_keycode *key);
	BOOL SetMsgMap(u32 nID,ifly_keycode msg,MsgFxn fxn,s32 param);
	BOOL SetMsgUnMap(u32 nID,ifly_keycode msg,MsgFxn fxn,s32 param);
	
	BOOL uiInit();
	BOOL uiInit_state();
	BOOL uiRelease();
	
	BOOL setOsdTransparency(u8 trans);
	BOOL setOsdStateShow(u8 bShow);//wrchen 090107
	BOOL setRegionOsdTransparency(u8 trans,int x,int y,int w,int h);
	BOOL setRegionOsdTransparency2(u8 trans,int x,int y,int w,int h,Wnd *pWnd);
	int setOsdalpha(simplewidget_screen sws,int x, int y, int w, int h,int a);//wrchen 090313
	
	BOOL CreateLocationPage();
	BOOL ShowLocationPage();
	void SetLocationFlag(u8 flag);
	u8   GetLocationFlag();
	
	BOOL CreateEnvelopPage();
	BOOL ShowEnvelopPage();
	
	BOOL CreateMDAreaPage();
	BOOL ShowMDAreaPage();
	
	#ifdef NEW_HARDWARE
	int  SendToPanel(u16 code);
	#else
	int  SendToPanel(u32 code);
	#endif
	#if defined(LANGUAGE_ARA) || defined(LANGUAGE_FAR) || defined(LANGUAGE_HEB)
	int get_language_ara_width(char ch);
	#endif

	BOOL AddInputBoxItem(s32 nID,char *pchStr);
	BOOL GetInputBoxSelect(s32 nID, s32 nIndex, char *pDeno);
	BOOL SetInputBoxSelect(s32 nID, s32 nIndex);
	
	void GetDateTimeCtrlTime(s32 param, u32 *pYear, u32 *pMonth, u32 *pDate, u32 *pHour, u32 *pMinute, u32 *pSecond);
    void SetDateTimeCtrlTime(s32 param, u32 nYear, u32 nMonth, u32 nDate, u32 nHour, u32 nMinute, u32 nSecond);
	void SetDateTimeCtrlEnable(s32 param, BOOL bYearEnable, BOOL bMonthEnable, BOOL bDateEnable, BOOL bHourEnable, BOOL bMinuteEnble, BOOL bSecondEnable);
	
	/*comment by lshu 20070416*/
	BOOL SetEditBoxInputLen(s32 nID,s32 nLen);
	/*comment end*/
	void SetInputMode(u32 nID, u32 nInputMode);//comment by lshu 20070508
	u32  GetInpurMode(u32 nID);//comment by lshu 20070508
	
	void SetShiftPromptID(u32 nID,u32 nShiftPromptID);
	
	void SetIPMacAddrCtrlType(s32 param, u32 type);
	
	BOOL SetDialogEscFxn(s32 nID,MsgFxn fxn);
	MsgFxn GetDialogEscFxn(s32 nID);
	
	BOOL LockPanel();
	BOOL UnlockPanel();
	
	int GetComFd();//07-11-06
	
	void control_light(u16 light,u8 status);
	
	void SetLockTime(u8 lock_time);
	void SetLockPolicy(MsgFxn fxn);
	
	void SetStopSeriesPolicy(MsgFxn fxn);
	
	u8	 GetAuthFlag();
	void SetAuthFlag(u8 byAuthFlag);
	
	/*******************/
	//07-12-13
	int  GetFbShow();
	int  GetFbShow2();
	int  SetFbShow(int fb_show_type);
	int  RedrawWnd();
	/*******************/
	
	int GetSupLangType();
	u8 GetLanguageType();
	u8 SetLanguageType(u8 type);
	
	inline int GetAsciiCharIndex(char ch);
	inline int ADD_OSD_ON_IMAGE(u8 *start,int x,int y,char *pch);
	inline int ADD_OSD_ON_VIDEO(u8 *vobuf,int w,int h,char *pch);//wrchen 090105
	inline int ADD_OSD_FONT_ONTO_IMAGE(int x,int y,char *pch);//wrchen 090107
	inline int ADD_FONT_ONTO_IMAGE(u8 *start,int x,int y,char *pch);//wrchen 090107
	void draw_line_on_state(int x1, int y1, int x2, int y2,u8 r,u8 g,u8 b);
	inline int ADD_STATE_ONTO_IMAGE(int x,int y,char *pbuf);//wrchen 090113
	 inline int ADD_STATE_ONTO_IMAGE_PIC(int x,int y,int pic_height,int pic_width,char *pbuf); //jflv 2011/11/18
	#ifdef HISI_3515_HD
	inline int ADD_FONT_ONTO_IMAGE_VGA(u8 *start,int x,int y,char *pch);
	void draw_line_on_state_vga(int x1, int y1, int x2, int y2,u8 r,u8 g,u8 b);
	inline int ADD_STATE_ONTO_IMAGE_VGA(int x,int y,char *pbuf);
	#endif
	inline int myfont_text_load();
	inline u16 myfont_width();
	inline u16 myfont_height();
	
	inline int myfont_text_show(simplewidget_text swt,simplewidget_screen sws,int font_width);
	inline int myfont_text_getsize(char *text, int font_width, int font_height, int *string_width, int *string_height);

	inline int myfont_text_show2(simplewidget_text swt,simplewidget_screen sws,int font_width);

	int disable_Region();
	void SetPicDisable(u32 nID,BOOL bEnable);
#ifdef __cplusplus
}
#endif

#define FONT_WIDTH 11
#define FONT_HEIGHT 16

//获得焦点状态
#define STATUS_FOCUS 0x01
//空闲状态
#define STATUS_IDLE  0x00
//按下状态
#define STATUS_DOWN 0x02
//用来标记listbox的选中条目是否改变
#define STATUS_SELECT_CHANGE 0x80
//定义最大的显示层数
#define MAX_SHOW_WND_NUM 15

extern mouse *input_box_screenRegion; //全局变量..用来备份被软键盘覆盖的区域.以备取消软键盘的时候恢复回去
extern Wnd *g_Inputbox;//全局指向输入框
extern u8 g_Inputbox_show;//标记输入框是否是显示状态
u8 set_intputbox_inputmethod(u8 new_method);//设置新的输入方法
//正常状态时候的控件边框的颜色
extern u8 g_test_r;
extern u8 g_test_g;
extern u8 g_test_b;
//控件获得焦点的时候的颜色
extern u8 g_focus_r;
extern u8 g_focus_g;
extern u8 g_focus_b;
//页面的背景颜色
extern u8 g_bg_color_r;
extern u8 g_bg_color_g;
extern u8 g_bg_color_b;

//pw 2010/7/7
extern u8 g_tbg_color_r;
extern u8 g_tbg_color_g; 
extern u8 g_tbg_color_b;
extern u16 g_tf_color ;

//pw 2010/7/21
extern u16 g_font_color;
extern u16 g_hl_font_color;

extern u8 g_static_text_color_r;
extern u8 g_static_text_color_g;
extern u8 g_static_text_color_b;


//pw 2010/9/30
extern u8 g_button_color_r;
extern u8 g_button_color_g;
extern u8 g_button_color_b;

extern u8 g_show_start;
//全局保存当前的透明度
extern u8 gOsdTransParency;
//全局是否有控件进入输入状态(输入状态是指面板或者遥控器输入.已经按了一次按键)
//比如 要输入小写字母c 面板按键对应的按键0 为abc.当第一次按下0的时候.该控件就进入了编辑状态.
//该值会再按键超时的时候自动清除(按键超时..是指在一个等待时间片(现在使用200ms)内没有收到任何按键)
//
//该值为0表示没有进入编辑状态的控件.非零为进入编辑状态了
extern u8 g_have_element_edit_mode;
//相同的键.击键次数.
//如果有控件进入编辑状态后.连续第几次击键.
//比如 要输入小写字母c 面板按键对应的按键0 为abc.当第一次按下0的时候.g_keybord_hit_num=0,对应的字母为a,第二次按下0的时候g_keybord_hit_num=1,对应的字母就为b了
//
//
//该值在按键超时和前后两次按键不相同情况下必须.清除为0
extern u8 g_keybord_hit_num;

extern s32 have_combox_show;
extern mouse * combox_option_screenRegion;

extern char * py_ime(char *strInput_py_str);
//保存页面的堆栈.一次保存每个显示的窗体ID
//如果一个窗口退出显示的话..会从中删除.
//
extern u32 g_ShowWND_ID[MAX_SHOW_WND_NUM];
//标记显示的页面数.
extern u8 g_ShowWND_num;

extern s32 g_show_sub_menu_count;

s32  pop_dialog();
void push_dialog(s32 id);
s32 pop_all_dialog();
//显示字符
int show_font(int x,int y,char *pstr,u16 color,simplewidget_screen sws);
int get_text_width(char *pstr);	//dong	010109	mult language
int show_font_mult(int x,int y,char *pstr,u16 color,simplewidget_screen sws,u16 width);	//dong	010105	mult language
extern u8 dateformat;
//显示软键盘
extern void  display_inputbox(Wnd * pWnd,Message_event event,s32 param);

void set_menu_item_parent_id(s32 nID,s32 nPID);
void set_menu_item_sub_flag(s32 nID,u8 flag);
void Set_Checkbox_Type(s32 nid,u8 type);
void move_element(Wnd* pWnd,Message_event event,s32 arg);

extern u8 g_show_twice;

extern void draw_total_schedule_view();
extern void OnDialogCancel(Wnd * pWnd,Message_event msg_ev,s32 param);
u8 SetStaticTextType(s32 nid,u8 type);
#ifdef MEIDIANBEIER_UI
int deal_menu_pic(s32 nID,BOOL bshow,int flag);
#endif

void set_fun_keychange(MsgFxn p_fun);//20100618 zlb_las

int utf82unicode(unsigned char *_s, unsigned int offset[]);

BOOL ShowInputBox(Wnd* pWnd,BOOL bShow);

#endif

