

#ifndef __SYSTEMM_API_H__
#define __SYSTEMM_API_H__

#include "DVRDEF.H"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup SystemAPI API System
/// 系统时间设置，关机与重启，系统资源状况，升级等API。
/// @{

/// 系统时间结构
typedef struct SYSTEM_TIME{
    int  year;///< 年。   
	int  month;///< 月，January = 1, February = 2, and so on.   
	int  day;///< 日。   
	int  wday;///< 星期，Sunday = 0, Monday = 1, and so on   
	int  hour;///< 时。   
	int  minute;///< 分。   
	int  second;///< 秒。   
	int  isdst;///< 夏令时标识。   
}SYSTEM_TIME;

/// 系统特性结构
typedef struct SYSTEM_CAPS
{
	/// 总共内存大小，以Kilo-Bytes为单位。
	uint MemoryTotal;

	/// 剩余内存大小，以Kilo-Bytes为单位。将关系到应用程序的内存使用策略。
	uint MemoryLeft;
}SYSTEM_CAPS;

/// 衍生版本性能
/*
typedef struct SYSTEM_CAPS_EX
{
	uint ForNRT;			///< 是否为非实时
	BYTE ForATM;			///< 是否为ATM机
	BYTE HasAudioBoard;		///< 带音频板
	BYTE HasLoopBoard;		///< 带环通板
	BYTE HasMatrixBoard;	///< 带矩阵板
}SYSTEM_CAPS_EX;
*/
typedef struct tagSYSTEM_CAPS_EX
{
 uint ForNRT;   //是否为非实时, 按位表示各个通道是否设为非实时.如0xffff表示前16路都是非实时.
 uchar ForATM;   //是否为ATM机, 1表示不带液晶ATM,2表示带液晶ATM
 uchar HasAudioBoard;  //带音频板
 uchar HasLoopBoard;  //带环通板
 uchar HasMatrixBoard; //带矩阵板
 uchar HasPtzBoard;           //带云台板
 uchar HasWlanBoard;        //带无线板 
 uchar reserved[2];
}SYSTEM_CAPS_EX;

/// 系统升级状态
enum system_upgrade_state
{
	SYSTEM_UPGRADE_OVER = 0,
	SYSTEM_UPGRADE_INIT = 1,
	SYSTEM_UPGRADE_RUNNING = 2
};

///系统恢复出厂设置
typedef enum CONFIG_RESUME_E
{
	CONFIG_RESUME_YES,
	CONFIG_RESUME_NO,
	CONFIG_RESUME_INVALID
}CONFIG_RESUME_OPR;

/// 得到当前系统时间。这个函数可能调用的很频繁, 所有调用时应该是从内存中取得
/// 时间,而不是直接从硬件, 在定时器里更新内存中的值.
/// 
/// \param [out] pTime 指向系统时间结构SYSTEM_TIME的指针。
/// \retval 0  成功
/// \retval <0  失败
int SystemGetCurrentTime(SYSTEM_TIME * pTime);


/// 设置当前系统时间。
/// 
/// \param [in] pTime 指向系统时间结构SYSTEM_TIME的指针。
/// \retval 0  成功
/// \retval <0  失败
int SystemSetCurrentTime(SYSTEM_TIME * pTime);
//int SystemGetTimeSecond(int *pTime);


/// 得到系统的毫秒计数。两次调用之间的间隔时间就是得到的计数的差值。
/// 
/// \param 无
/// \return 当前计数。
/// \note 一般情况下，49.7天后得到计数会溢出，使用到的地方要做溢出后处理！
uint SystemGetMSCount(void);

/// 得到系统的毫秒计数。两次调用之间的间隔时间就是得到的计数的差值。
/// 
/// \param 无
/// \return 当前计数。
/// \note 一般情况下，497天后得到计数会溢出，使用到的地方要做溢出后处理！
uint64 SystemGetMSCount64(void);



/// 得到系统的微秒计数, 此函数主要用于模块性能的测试。溢出的更快。
/// 
/// \param 无
/// \return 当前计数。
uint SystemGetUSCount(void);


/// 得到CPU占用率。
/// 
/// \param 无
/// \return CPU占用百分比。
int SystemGetCPUUsage(void);


/// 得到系统特性。
/// 
/// \param [out] pCaps 指向系统特性结构SYSTEM_CAPS的指针。
/// \retval 0  获取成功。
/// \retval <0  获取失败。
int SystemGetCaps(SYSTEM_CAPS * pCaps);



/// 
int SystemGetCapsEx(SYSTEM_CAPS_EX * pCaps);


/// 挂起当前线程，时间到后再唤醒。
/// 
/// \param [in] dwMilliSeconds 挂起的时间。
void SystemSleep(uint dwMilliSeconds);


/// 
void SystemUSleep(uint dwMicroSeconds);


/// 让系统蜂鸣器发出单音调的声音，是同步模式，时间到才返回。
/// 
/// \param [in] dwFrequence 音调频率。
/// \param [in] dwDuration 持续的时间。
void SystemBeep(uint dwFrequence, uint dwDuration );

// 静音接口
// 当isSilence为1时，表示系统需要静音，关闭声音的IO口；
// 当isSilence为0时，表示系统需要取消静音，打开声音的IO口；
void SystemSilence(int isSilence);

/// 系统复位
//void SystemReboot(void);

/// 系统关闭
void SystemShutdown(void);

/// 应用程序收到升级包后，让系统解压。是同步调用模式，直到解压完成才返回。
/// 
/// \param [in] pData 数据包在内存中的指针。
/// \param [in] dwLength 数据长度。
/// \param [out] pProgress 当前进度百分比，函数调用过程时会被定时更新，应用程序
///        不应该修改这个值。
/// \retval 0  成功
/// \retval <0  失败
/// \note 不同的系统数据打包的格式不同，对应的完成数据打包和解压的的程序也不同。
int SystemUpgrade(uchar* pData, uint dwLength, uint *pProgress);

//网络球单片机及IPC单片机升级接口
int SystemUpgradeC8051(char *pData, unsigned int dwLength, unsigned int *pProgress);

/*Begin: Add by chenjianqun 20061024 for New Upgrade method*/
int SystemUpgradeEx(void *iov, uint *pProgress);
//int SystemUpgradeEx(struct iovec *iov, uint *pProgress);

/*End: Add by chenjianqun 20061024 for New Upgrade method*/

/// 系统日志初始化。系统日志用来记录应用程序运行时所有的标准输出。
/// 
/// \retval 0  成功
/// \retval <0  失败
int SystemLogInit(void);


/// 记录系统日志， 同时将内容写到到标准输出缓冲。
/// 
/// \retval 0  成功
/// \retval <0  失败
int SystemLogWrite(char * format, ...);


/// 获取系统序列号
int SystemGetSerialNumber(char *pData, int len);


/// 获取系统硬件版本
uint SystemGetHWVersion(void);


/// 获取设备名称设备类型是一个字符串，长度小于8
///字符串根据设备不同分别为："IPCF4x5"，"IPCA6"，"IPCA8"，"IPCA7"，"IPVM"，"IPSD"
int SystemGetDeviceType(char* type); 

/// 设置u-boot环境变量
/// 例: 设置N制P制, SystemSetUbootParam("videomode", "NTSC"),SystemSetUbootParam("videomode", "PAL")
//int SystemSetUbootParam(const char *para, const char *value);

/// 获取版本号, 格式例如: r9508:2.15:12:13:14:16, 最大长度128字节
int SystemGetVersion(char *version);

/// 获取UBOOT过程中检测到的config_resume 状态，方便上层决定是否需要恢复默认设置
CONFIG_RESUME_OPR SystemGetConfigResume(void);

// 设置语言，获取语言
enum ui_language_t {
UI_LANGUAGE_ENGLISH = 0,                // 英文 
UI_LANGUAGE_CHINESE_SIMPLE =1,          // 中文 
UI_LANGUAGE_CHINESE_TRADITIONAL = 2 , //繁体中文   
UI_LANGUAGE_ITALIAN  = 3 ,             //意大利语    
UI_LANGUAGE_SPANISH  = 4 ,             //西班牙语    
UI_LANGUAGE_JAPANESE  = 5 ,             //日语   
UI_LANGUAGE_RUSSIAN  = 6 ,             //俄语  
UI_LANGUAGE_FRENCH = 7 ,             //法语 
UI_LANGUAGE_GERMAN = 8 ,             //德语 
UI_LANGUAGE_PORTUGUESE = 9 ,         //葡萄牙 
UI_LANGUAGE_TURKISH  = 10,             //土耳其   
UI_LANGUAGE_POLISH = 11,             //波兰文 
UI_LANGUAGE_ROMANIAN  = 12,             //罗马尼亚    
UI_LANGUAGE_HUNGARIAN  = 13,            //匈牙利语 
UI_LANGUAGE_FINNISH  = 14,             //芬兰语 
UI_LANGUAGE_ESTONIAN  = 15,             //爱沙尼亚语  
UI_LANGUAGE_KOREAN = 16,             //韩语 
UI_LANGUAGE_FARSI  = 17,                //波斯文 
UI_LANGUAGE_DANISH = 18,             //丹麦语 
UI_LANGUAGE_BULGARIAN  = 19,            //保加利亚 
UI_LANGUAGE_ARABIC = 20,             //阿拉伯文 
UI_LANGUAGE_CZECH  = 21,                //捷克 
UI_LANGUAGE_SLOVAK = 22,             //斯洛伐克 
UI_LANGUAGE_INVALID = 255

};
enum ui_language_t SystemGetUILanguage(void);
int SystemSetUILanguage(enum ui_language_t lang);
		     

/// wyt: 效验密码放在底层库里面实现，对上层完全透明，上层不做任何事情。2009-7-15
/// 用户程序里面效验密码, 成功是0
//int SystemCheckAT88_1(void);
//int SystemCheckAT88_2(void);

/// @} end of group



#define	SYS_VGAPIXEL_ERR				-1
#define	SYS_VGAPIXEL_800x600			0		// 800 x 600
#define	SYS_VGAPIXEL_1024x768			1		// 1024 x 768



int SystemGetVGAPixelCaps(int *pCaps);
int SystemGetVGAPixel();
int SystemSetVGAPixel(int vgapixel);


#ifdef __cplusplus
}
#endif

#endif //__SYSTEMM_API_H__

