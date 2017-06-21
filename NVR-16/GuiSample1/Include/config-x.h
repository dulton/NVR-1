


#ifndef __config_x_h__
#define __config_x_h__

#ifdef WIN32
	#define PRTSC_DIR "Win32/PrtSc"		//抓图文件目录
	#define CONFIG_DIR "Win32/Config"		//配置文件目录
	#define LOG_DIR "Win32/Log"			//日志文件目录
	#define PROFILE_DIR "Common/Lua"		//Lua文件目录
	#define FONT_BIN "Common/FontBin"	//解析字体的工具
	#define DATA_DIR "Win32/Pics"  //图片目录
	#define FONT_DIR "Win32/Log" //字体
	#define	FONT_DEFAULT_DIR "Win32/Strings" //字体
	#define LOGO_DIR "Win32/Logo"	//Logo目录
	#define WEB_DIR	"Win32/Web"		//Web目录
//#elif defined(DVR_HI)
#else
// 	#define PRTSC_DIR ""
// 	#define CONFIG_DIR "/mnt/mtd/Config"
// 	#define LOG_DIR "/mnt/mtd/Log"
// 	#define PROFILE_DIR "/usr/bin/lua"
// 	#define DATA_DIR "/usr/data/Data"
// 	#define FONT_BIN "/usr/data/Data"
// 	#define	FONT_DIR "/mnt/custom" //字体
// 	#define FONT_DEFAULT_DIR "/usr/data/Data"	//字体
// 	#define LOGO_DIR "/mnt/logo"	//Logo目录
// 	#define WEB_DIR "/mnt/web"
	#define PRTSC_DIR ""
	#define CONFIG_DIR "/tmp/ui/mtd/Config"
	#define LOG_DIR "/tmp/ui/mtd/Log"
	#define PROFILE_DIR "/tmp/ui/bin/lua"
	#define DATA_DIR "/tmp/ui/data/Data"
	#define FONT_BIN "/tmp/ui/data/Data"
	#define	FONT_DIR "/tmp/ui/data/Data" //字体
	#define FONT_DEFAULT_DIR "/tmp/ui/data/Data"	//字体
	#define LOGO_DIR "/tmp/ui/logo"	//Logo目录
	#define WEB_DIR "/tmp/ui/web"
#endif

#ifdef _lint
#define SHBELL
#endif

/*!< 模块宏 */

/*! <GUI模块 */
//#define GRAPHICS_SUPPORT_SETRESOLUTION 1	// 可以设置分辨率

/*!< 备份模块 */
//#define BAK_EJECT_DISK			1 // 弹出盘片功能
//#define BAK_BURN_DISK				1 //实时刻录功能
//#define BAK_SUPPORT_CHANNEL		2 //支持刻录的录像通道数
//#define BAK_SUPPORT_DIRECTORY		1 //支持多级目录备份

/*!< 云台相关 */
//#define PTZ_SUPPORT_BANKNOTE	//特殊点钞机的宏 //#define PTZ_BANKNOTE					//通过485实现一台点钞机功能，之前必须打开COM_SUPPORT_GUAO宏
//#define PTZ_RW_SEMIDUPLEX  //485半双工通信 //#define PTZ_SEMIDUPLEX				//485半双工通信
//#define PTZ_CONTROL_IRIS_DEEPLY   //东北办光圈无法控制问题  //#define IRISNOTCTRL					//东北办光圈无法控制问题
//#define PTZ_SUPPORT_ANNUNCIATOR	//报警解码器功能设置 //#define PTZ_ALARM_SUPPORTED			//报警解码器功能设置
//#define PTZ_WATCH_NIGHT //夜视仪权限控制 //#define USE_NIGHTVISION        	 //夜视仪权限控制

/*!< ALARM */
//#define ALM_SUPPORT_SNAPSHOT			//使用报警触发抓图和取数据

/*!< 编码模块 */
//#define ENC_SHOW_SMALL_FONT 1 //叠加小字体

//#define ENC_CHANGE_PARAM 		//支持动检和报警以不同的分辨率或帧率录像 1:设置界面 2自动调整

//#define ENC_SUPPORT_SNAPSHOT //支持抓图功能
//#define ENC_ENCODE_MULTI 1

//#define ENC_SUB_COMPRESS // 128:-H264 1:-MPEG4
//#define ENC_ADD_MP4_COMPRESS //128:MPEG4+H264

//#define ENC_SUB_IMG //1:-D1 2:-HD1 3:-D1-HD1 4:-BCIF 5:-D1-BCIF 6:-HD1-BCIF 7:-D1-HD1-BCIF 8:-CIF
//#define ENC_ADD_CIF_IMG  //1:CIF+D1 2:CIF+HD1 3:CIF+D1+HD1 4:CIF+BCIF 5:CIF+D1+BCIF 6:CIF+HD1+BCIF 7:CIF+D1+HD1+BCIF
//#define ENC_ADD_BCIF_IMG //1:BCIF+D1 2:BCIF+HD1 3:BCIF+D1+HD1
//#define ENC_ADD_HD1_IMG //1:HD1+D1

//! 抓图模块
//#define SNAP_REQUIRE_ISTC //ISTC抓图功能
//#define SNAP_REQUIRE_JQBAR //金桥网吧抓图
//#define	SNAP_REQUIRE_SHCL	//上海长岭抓图

/// 录像模块
//#define	REC_SUPPORT_HEAD  //录像文件里含64K的头信息,卡号录像不需要打开这个宏，打开也没有问题


//#define MON_PROTECT_SCREEN //屏幕保护宏
//#define  MON_SUPPORT_SPLIT6 //六画面的支持，1大五小
//开启网络键盘的轮训键
//#define MON_NETKEY_TOUR
//#define MON_OPEN_TVMONITORBYPOWER //液晶关掉后只有按电源键才能唤醒
/*!< 回放模块 */
//#define FUNC_SYN4_PLAY 1

/*不要加在此来打开宏，
需要在编译器添加，
linux下在makefile里，win32 在属性设置中*/

//#define FTP_TRAN_SNAPSHOT //FTP传输图片,打开此宏时,必须打开ENC_SUPPORT_SNAPSHOT 宏

//#define FUNC_AUTOTEST_TVVGA //1500自动检测切换TV/VGA切换

//#define FUNC_MONITOR_RECORD

//#define  LOG_SUPPORT_PLAYBACK
//#define FUNC_SUPPORT_ALM_BACKTO_SPLIT16 //报警延迟结束后弹回16画面
//#define AUTO_MAINTAIN_TUS_TWO //默认每周二凌晨两点自动维护开启
//#define FUNC_SHOW_TZTIP 1  //显示NTP界面上时区选择的提示
//#define FUNCTION_SUPPORT_BLIND_LATCHDELAY	//支持遮挡检测去抖动功能,目前在ATM上使用
//#define MOTION_LATCHDELAY_NOSHOW //支持视频检测去抖动功能是否开启,目前在ATM上使用
//#define MON_ADDLOGOUT_FOR_LOGINUSR 1//用户登陆后右键菜单中增加注销选项，本地默认用户的右键菜单则不显示该选项
#endif //__config_x_h__

