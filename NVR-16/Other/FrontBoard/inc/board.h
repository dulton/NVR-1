#ifndef _BOARD_H_
#define _BOARD_H_

enum KEYCODE_FROM_PANEL
{
	KEYCODE_FROM_PANEL_0               = 48,//new
	KEYCODE_FROM_PANEL_1               = 49,//new
	KEYCODE_FROM_PANEL_2               = 50,//new
	KEYCODE_FROM_PANEL_3               = 51,//new
	KEYCODE_FROM_PANEL_4               = 52,//new
	KEYCODE_FROM_PANEL_5               = 53,//new
	KEYCODE_FROM_PANEL_6               = 54,//new
	KEYCODE_FROM_PANEL_7               = 55,//new
	KEYCODE_FROM_PANEL_8               = 56,//new
	KEYCODE_FROM_PANEL_9               = 57,//new
	KEYCODE_FROM_PANEL_10			   = 116,
	KEYCODE_FROM_PANEL_11   	       = 39,
	KEYCODE_FROM_PANEL_12   	       = 40,
	KEYCODE_FROM_PANEL_13   	       = 41,
	KEYCODE_FROM_PANEL_14   	       = 42,
	KEYCODE_FROM_PANEL_15   	       = 43,
	KEYCODE_FROM_PANEL_16    	       = 45,
	KEYCODE_FROM_PANEL_10PLUS		   = 117,
	KEYCODE_FROM_PANEL_1_4             = 30,
	KEYCODE_FROM_PANEL_5_8             = 31,
	KEYCODE_FROM_PANEL_9_12            = 32,
	KEYCODE_FROM_PANEL_13_16           = 33,
	KEYCODE_FROM_PANEL_1_9             = 34,
	KEYCODE_FROM_PANEL_8_16            = 35,
	KEYCODE_FROM_PANEL_PIC8            = 36,
	KEYCODE_FROM_PANEL_PIC16           = 37,
	
	KEYCODE_FROM_PANEL_MULTI           = 64,//new
	
	KEYCODE_FROM_PANEL_CHN_PLUS        = 254,//new
	KEYCODE_FROM_PANEL_CHN_MINUS       = 255,//new	
	//
	KEYCODE_FROM_PANEL_UP              = 66,//new
	KEYCODE_FROM_PANEL_DOWN            = 67,//new
	KEYCODE_FROM_PANEL_LEFT            = 68,//new
	KEYCODE_FROM_PANEL_RIGHT           = 69,//new	
	KEYCODE_FROM_PANEL_ENTER           = 70,//new
	KEYCODE_FROM_PANEL_ESC             = 71,//new
	//
	KEYCODE_FROM_PANEL_MENU            = 58,//new
	KEYCODE_FROM_PANEL_PLAY            = 61,//new
	KEYCODE_FROM_PANEL_RECORD          = 62,//new
	KEYCODE_FROM_PANEL_PTZ             = 63,//new
	KEYCODE_FROM_PANEL_VOIP            = 65,
	//
	KEYCODE_FROM_PANEL_STOP			   = 91,//new
	KEYCODE_FROM_PANEL_STEP  		   = 28,//new
	KEYCODE_FROM_PANEL_PAUSE           = 75,//new
	KEYCODE_FROM_PANEL_FAST		       = 76,//new
	KEYCODE_FROM_PANEL_SLOW 	       = 77,//new
	KEYCODE_FROM_PANEL_FASTB           = 78,//new
	KEYCODE_FROM_PANEL_PREV			   = 79,//new
	KEYCODE_FROM_PANEL_NEXT            = 80,//new
	//
	KEYCODE_FROM_PANEL_PTZ_ZOOM_PLUS	= 92,
	KEYCODE_FROM_PANEL_PTZ_ZOOM_MINUS	= 93,
	KEYCODE_FROM_PANEL_PTZ_FOCUS_PLUS	= 94,
	KEYCODE_FROM_PANEL_PTZ_FOCUS_MINUS	= 95,
	KEYCODE_FROM_PANEL_PTZ_IRIS_PLUS	= 96,
	KEYCODE_FROM_PANEL_PTZ_IRIS_MINUS	= 97,
	KEYCODE_FROM_PANEL_PTZ_CRUISE		= 27,
	//
	KEYCODE_FROM_PANEL_MUTE				= 72,//new
	KEYCODE_FROM_PANEL_VOL_MINUS		= 84,
	KEYCODE_FROM_PANEL_VOL_PLUS			= 85,
	//
	KEYCODE_FROM_PANEL_FS_OL			= 112,
	KEYCODE_FROM_PANEL_FS_OR			= 113,
	KEYCODE_FROM_PANEL_FS_IR			= 114,
	KEYCODE_FROM_PANEL_FS_IL			= 115,
	//
	KEYCODE_FROM_PANEL_FN				= 59,//new
	KEYCODE_FROM_PANEL_SHIFT			= 60,//new
	KEYCODE_FROM_PANEL_CLEAR			= 73,//new
	KEYCODE_FROM_PANEL_POWER			= 81,//new
	//
	////////////////////////////////////////////////////
	KEYCODE_FROM_PANEL_DISPLAY_CTL		= 38,
	//
	KEYCODE_FROM_PANEL_NUMERIC			= 47,
	KEYCODE_FROM_PANEL_VGA				= 74,
	//	
	KEYCODE_FROM_PANEL_PLAYOVER			= 100,
	KEYCODE_FROM_PANEL_PROGRESS_RATE	= 101,
	KEYCODE_FROM_PANEL_PROGRESS_RATIO	= 102,
	KEYCODE_FROM_PANEL_PROGRESS_HOURS	= 103,
	KEYCODE_FROM_PANEL_PROGRESS_MINUTES	= 104,
	KEYCODE_FROM_PANEL_PROGRESS_SECONDS	= 105,
	KEYCODE_FROM_PANEL_PROGRESS_TOTAL	= 106,
	KEYCODE_FROM_PANEL_PLAYSTART		= 107,
	KEYCODE_FROM_PANEL_PLAYOVER_TIME	= 108,
	//
	KEYCODE_FROM_PANEL_LOCKOUT			= 99,//132,//csp modify
	KEYCODE_FROM_PANEL_MUTE_OFF			= 133,
	KEYCODE_FROM_PANEL_FARCTL_NUM1		= 134,
	KEYCODE_FROM_PANEL_MENU_ESC			= 136,
	//
	KEYCODE_FROM_PANEL_DEV				= 82,//200,//csp modify
	//csp modify
	KEYCODE_FROM_PANEL_SEARCH			= 98,// 搜索
};

enum KEYCODE_FOR_UI
{
	KEYCODE_FOR_UI_0 = 0x01,
	KEYCODE_FOR_UI_1,	KEYCODE_FOR_UI_2,	KEYCODE_FOR_UI_3,	KEYCODE_FOR_UI_4,	KEYCODE_FOR_UI_5,
	KEYCODE_FOR_UI_6,	KEYCODE_FOR_UI_7,	KEYCODE_FOR_UI_8,	KEYCODE_FOR_UI_9,	KEYCODE_FOR_UI_10, 
	KEYCODE_FOR_UI_11, KEYCODE_FOR_UI_12, KEYCODE_FOR_UI_13, KEYCODE_FOR_UI_14, KEYCODE_FOR_UI_15, KEYCODE_FOR_UI_16,
	KEYCODE_FOR_UI_CHNPLUS, KEYCODE_FOR_UI_CHNMINUS,
	/*! 0x12 */
	KEYCODE_FOR_UI_UP,		KEYCODE_FOR_UI_DOWN,	KEYCODE_FOR_UI_LEFT,	KEYCODE_FOR_UI_RIGHT,
	KEYCODE_FOR_UI_PGUP,	KEYCODE_FOR_UI_PGDN, 	KEYCODE_FOR_UI_RET,	KEYCODE_FOR_UI_ESC,

	KEYCODE_FOR_UI_PLAY,	KEYCODE_FOR_UI_STOP,	KEYCODE_FOR_UI_FAST,	KEYCODE_FOR_UI_SLOW,
	KEYCODE_FOR_UI_PREV,	KEYCODE_FOR_UI_NEXT,	/*! 0x20 */KEYCODE_FOR_UI_SPLIT,	KEYCODE_FOR_UI_ADDR,
	KEYCODE_FOR_UI_REC,	KEYCODE_FOR_UI_FUNC,	KEYCODE_FOR_UI_ALARM,	KEYCODE_FOR_UI_SHUT,	KEYCODE_FOR_UI_SEARCH,

	KEYCODE_FOR_UI_LOCK,	KEYCODE_FOR_UI_10PLUS,	KEYCODE_FOR_UI_SHIFT,	KEYCODE_FOR_UI_BACK,
	KEYCODE_FOR_UI_STEPX,	KEYCODE_FOR_UI_MENU,	KEYCODE_FOR_UI_INFO,	KEYCODE_FOR_UI_SPLIT1,	
	KEYCODE_FOR_UI_SPLIT4,	/*! 0x30 */KEYCODE_FOR_UI_SPLIT8,	KEYCODE_FOR_UI_SPLIT9,	KEYCODE_FOR_UI_SPLIT16,
	KEYCODE_FOR_UI_STEPF,	KEYCODE_FOR_UI_STEPB,  KEYCODE_FOR_UI_PAUSE,

	//add by xie 
	KEYCODE_FOR_UI_MAIN,            //右键菜单	
	//end

	KEYCODE_FOR_UI_MUTE, //静音
	KEYCODE_FOR_UI_VOIP, //呼叫

	KEYCODE_FOR_UI_AUTOTUR = 0xa0, /*!< 自动轮巡 */

	//新增加A系列的遥控键值
	//紧急呼叫
	KEYCODE_FOR_UI_EMERGENCY_CALL = 0xa8,
	
	//安全模式
	KEYCODE_FOR_UI_SAFE_MODE = 0xa9,
	
	//布防,
	KEYCODE_FOR_UI_ARMING = 0xaa,

	//撤防
	KEYCODE_FOR_UI_DISARMING = 0xab,


	KEYCODE_FOR_UI_PTZ = 0xb0, /*!< 云台控制菜单 */
	KEYCODE_FOR_UI_TELE, KEYCODE_FOR_UI_WIDE, /*!< 变倍 */
	KEYCODE_FOR_UI_IRIS_CLOSE, KEYCODE_FOR_UI_IRIS_OPEN, /*!< 光圈 */
	KEYCODE_FOR_UI_FOCUS_NEAR,	KEYCODE_FOR_UI_FOCUS_FAR, /*!< 聚焦 */
	KEYCODE_FOR_UI_BRUSH, /*!< 雨刷 */
	KEYCODE_FOR_UI_LIGHT, /*!< 灯光 */
	KEYCODE_FOR_UI_AUTOPAN, /*!< 线扫*/
	KEYCODE_FOR_UI_SCAN, /*!< 点间巡航 */
	KEYCODE_FOR_UI_PATTERN, /*!< 模式 */
	KEYCODE_FOR_UI_SPRESET, /*!< 设置预置点 */
	KEYCODE_FOR_UI_GPRESET, /*!< 转到预置点 */
	KEYCODE_FOR_UI_DPRESET, /*!< 删除预置点 */

	//球机菜单(0x30 - 0x32)
	KEYCODE_FOR_UI_MACHINE_MENU = 0xc0, KEYCODE_FOR_UI_MACHINE_PREV, KEYCODE_FOR_UI_MACHINE_NEXT,

	KEYCODE_FOR_UI_BACKUP = 0xdf,

	// 恢复出厂设置
	KEYCODE_FOR_UI_DEFAULT_CFG = 0xee,
	KEYCODE_FOR_UI_ALARMIN = 0xfe,
};

#if defined(CHIP_HISI3531) || (defined(CHIP_HISI3521) && !defined(HI3520D))
#define PANEL_GET_CRYPT		0x12

#define	CMD_H8_EXTEND		(1 << 0)	//扩展命令
#define	CMD_H8_EXTEND2		(1 << 1)	//扩展命令
#define	CMD_H8_RESET		(1 << 2)	//系统重启(软关机)
#define	CMD_H8_IO_CTRL		(1 << 3)	//控制IO的值
#define	CMD_H8_HDD_LOCK		(1 << 4)	//是否启用硬盘锁功能
#define	CMD_H8_MORELED		(1 << 5)	//控制其它的灯
#define	CMD_H8_KEY			(1 << 6)	//设置连发键
#define	CMD_H8_DEV			(1 << 7)	//设置设备号

#define	CMD_L8_VERSION		(1 << 0)	//获得面板版本号，固定为3位数，比如1.21
#define	CMD_L8_START		(1 << 1)	//设置开机计划
#define	CMD_L8_SYSTIME		(1 << 2)	//设置系统时间
#define	CMD_L8_FAIL			(1 << 3)	//加密芯片验证失败通知（目前没有用）
#define	CMD_L8_CIPHER		(1 << 4)	//加密
#define	CMD_L8_STATUS		(1 << 5)	//获得系统状态(如ACC状态等)
#define	CMD_L8_REOPEN		(1 << 6)	//断电关机重启
#define	CMD_L8_UPDATE		(1 << 7)	//面板升级

#define	CMD2_L8_ACC			(1 << 0)	//使用ACC开机方式，同时支持定时开机方式
#define	CMD2_L8_SCHED		(1 << 1)	//使用定时开机方式，不支持ACC开机方式
#else
//往面板单片机写数据时，命令字如下，一般是两个字节，第一字节是命令，第二字节是参数
#define FRONTBOARD_CMD_H8_EXTEND 		(1 << 0) //扩展命令
#define FRONTBOARD_CMD_H8_RESERVE 		(1 << 1) //保留位
#define FRONTBOARD_CMD_H8_RESET 		(1 << 2) //系统重启(软关机)
#define FRONTBOARD_CMD_H8_LED8 			(1 << 3) //控制CH8-16的灯
#define FRONTBOARD_CMD_H8_LED0 			(1 << 4) //控制CH0-7的灯
#define FRONTBOARD_CMD_H8_MORELED 		(1 << 5) //控制其它的灯
#define FRONTBOARD_CMD_H8_PANEL_POWER 	(1 << 6) //设置面板电源键值
#define FRONTBOARD_CMD_H8_IR_POWER 		(1 << 7) //设置红外电源键值
//结束
#endif

typedef struct
{
	u8 nLogicKey;
	u16 nPhysicKeyFront;
	u16 nPhysicKeyRemote;
	s8 nDescription[20];//csp modify
}SFrontBoardKeyInfo;

typedef struct
{
	s32* nBoardFd;	//面板设备打开后的文件描述符，机器可能同时存在多个不同类型的面板设备，比如前面板、键盘设备等
	u8 nBoardNum;	//已经连接到主机的面板设备的数目
	u8 nKeyIn;		//是否有按键数据//cw_panel
	s8 nData[4];	//存放从面板设备读到或将要写入面板设备的数据；
	sem_t nDataSem;	//读写数据nData时同步
}SBoardManager;

typedef void* BoardCenter;

BoardCenter BoardInit(void);
s32 AddBoardDevice(BoardCenter center, s8* pDevPath);
void ReadBoardDeviceFxn(BoardCenter* center);
//void UpdateDvrAddress(BoardCenter center);
s32 WriteBoardDevice(BoardCenter center, s32 nBytes);
void InitKeyTablePanelLogicToPhysic();

#endif //_BOARD_H_

