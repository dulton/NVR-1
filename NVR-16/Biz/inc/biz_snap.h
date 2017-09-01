#ifndef _BIZ_SNAP_H_
#define _BIZ_SNAP_H_

typedef enum
{
	TypeSensor = 0,	// 0
	TypeMD,			// 1
	TypeIPCEXT,		// 2
	TypeAlarmMax,
} EM_ALARM_TYPE;

typedef enum
{
	EM_ALARM_NONE = 0,	// 0 无报警，初始状态和整个报警活动结束状态
	EM_ALARM_TIGGER,	// 1 报警触发
	EM_ALARM_ING,		// 2 报警中
	EM_ALARM_END,		// 3 报警解除
	EM_ALARM_STILL,	// 4	报警延迟延录
	EM_ALARM_MAX,
} EM_ALARM_STATUS;


/* record模块中图片类型掩码
抓图类型	bit_mask	value
定时			0		
移动侦测	1
报警			2 //海康只有外部报警和IPC外部报警才抓图
动测或报警			(1<<1) |(1<<2)			用于备份
动测且报警			(1<<1) |(1<<2) |(1<<7)	同上
手动			3
/////////////////智能侦测	4
手动截图	4
////////////////////////////////回放截图	6
全部类型			0xff
预录			5		因为报警后要上传发生报警前n秒的图片
日常			6		(每天两个时间点上传深广)
全部类型	0xff
*/

//本模块中图片类型
typedef enum
{
//upload
	TypeManualTEST,		// 0 测试图像上传--监管平台--测试保养
	TypeManualMAINTAIN,	// 1 保养图像上传
	TypeManualACCEPT,	// 2 验收管理上传
	TypeDaily,			// 3 日常图片上传
	TypeSvrReq,			// 4 服务器请求上传通道图片
//record
	TypePreSnap,		// 5 预录
	TypeTime,			// 6 定时-录像配置--抓图参数
	TypeManual,			// 7 手动截图
//upload & record
	TypeAlarmSensor,	// 8 本地传感器报警
	TypeMotionDet,		// 9 移动侦测
	TypeAlarmIPCEXT,	// 10 IPC外部报警
	TypeSnapMax,
} EM_SNAP_TYPE;


#ifdef __cplusplus
extern "C" {
#endif

//服务初始化，启动
int SnapServerStart(u8 MaxSensorNum, u8 MaxIpcChn);

//非报警类型请求抓图
int RequestSnap(u8 chn, EM_SNAP_TYPE type, const char *PoliceID, const char *pswd);


//外部告知报警状态(触发和结束)，模块内部处理抓图
//type : 移动侦测、本机报警、IPC外部报警
int AlarmStatusChange(u8 chn, EM_ALARM_TYPE type, int status);

int BizSnapPause(void);
int BizSnapResume(void);



#ifdef __cplusplus
}
#endif

#endif

