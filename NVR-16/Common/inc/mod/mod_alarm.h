#ifndef _MOD_ALARM_H_
#define _MOD_ALARM_H_

#include "common_basetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_LEN	16

#define ALARM_PTZ_NUM	64

typedef void * AlaHdr;

typedef enum
{
	EM_ALARM_SENSOR_LOW = 0, //传感器低电平报警类型
	EM_ALARM_SENSOR_HIGH, //传感器高电平报警类型
} EMALARMSENSORTYPE;

typedef enum
{
	EM_ALARM_SENSOR_NORMALOPEN = 1, //常开报警输出
	EM_ALARM_SENSOR_NORMALCLOSE, //常闭报警输出
} EMALARMOUTTYPE;

typedef enum
{
	EM_ALMARM_VMOTION_AREA_SELECTALL, //移动侦测区域全选
	EM_ALMARM_VMOTION_AREA_CLEAR, //移动侦测区域全部清除
	EM_ALMARM_VMOTION_AREA_SET, //移动侦测区域设置
} EMALARMVMOTIONAREASET;

typedef enum
{
	EM_ALARM_EVENT_SENSOR = 0, //传感器报警事件
	EM_ALARM_EVENT_VLOST, //视频丢失报警事件
	EM_ALARM_EVENT_VBLIND, //遮挡报警事件
	EM_ALARM_EVENT_VMOTION, //移动侦测报警事件
	EM_ALARM_EVENT_IPCEXT,//IPC外部传感器报警事件
	EM_ALARM_EVENT_IPCCOVER, //IPC 遮盖
	EM_ALARM_EVENT_DISK_LOST,//硬盘丢失
	EM_ALARM_EVENT_DISK_ERR,//硬盘读写错误
	EM_ALARM_EVENT_DISK_NONE,//开机检测无硬盘
	EM_ALARM_EVENT_485EXT,
	
	EM_ALARM_EVENT_DISPATCH_ZOOMCHN = 100, //触发通道放大事件;
	EM_ALARM_EVENT_DISPATCH_EMAIL, //触发email事件;暂不支持
	EM_ALARM_EVENT_DISPATCH_SNAP, //触发抓图事件;暂不支持
	EM_ALARM_EVENT_DISPATCH_REC_SENSOR,//传感器触发录像事件
	EM_ALARM_EVENT_DISPATCH_REC_IPCEXTSENSOR,
	EM_ALARM_EVENT_DISPATCH_REC_IPCCOVER,
	EM_ALARM_EVENT_DISPATCH_REC_VMOTION,//移动侦测触发录像事件
	EM_ALARM_EVENT_DISPATCH_REC_VBLIND,//视频遮挡触发录像事件
	EM_ALARM_EVENT_DISPATCH_REC_VLOST,//视频丢失触发录像事件
	EM_ALARM_EVENT_DISPATCH_PTZ_PRESET, //触发云台联动预置点事件
	EM_ALARM_EVENT_DISPATCH_PTZ_PATROL, //触发云台联动巡航事件
	EM_ALARM_EVENT_DISPATCH_PTZ_LOCUS, //触发云台联动轨迹事件
	
	//csp modify 20130326
	EM_ALARM_EVENT_CTRL_CHN_LED = 200,//控制通道灯
} EMALARMEVENT;

typedef struct
{
	EMALARMEVENT emAlarmEvent; //回调事件
	u8 nChn; //表示各类型的通道号
	u8 nData; //传感器/移动侦测/视频丢失/遮挡表示报警事件状态;云台联动时表示预置点号、巡航路径或巡航停止；轨迹时表示开始/停止
	time_t nTime; //表示各事件最后发生的时间(延时情况下表示最后触发的时间,不是延时后的时间)
	//csp modify 20130326
	u32 reserved[1];
} SAlarmCbData;

typedef void (*FNALARMCB)(SAlarmCbData* psAlarmCbData);

typedef struct
{
	u8 nVideoChnNum;//视频通道数
	u8 nAlarmSensorNum;//传感器通道数
	u8 nAlarmOutNum;//报警输出通道数
	u8 nBuzzNum;//蜂鸣器个数;暂时只支持1个
	u32 nVBlindLuma;//遮挡报警亮度阀值
	u32 nDiskNum;//硬盘数目
	
	FNALARMCB pfnAlarmCb; //报警事件触发回调函数
	//yaogang modify 20150324 
	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	u8 nNVROrDecoder;
}SAlarmInitPara;

typedef struct
{
	EMALARMSENSORTYPE emType;//常开/常闭
	u8 nEnable;//是否启用
	u16 nDelay;//延时	
	char name[MAX_NAME_LEN + 1];//名称设置
	char reserved[8];
} SAlarmSensorPara;
//yaogang modify 20141010
typedef struct
{
	u8 nEnable;//是否启用
	u16 nDelay;//延时
} SAlarmIPCExtSensorPara;
typedef struct
{
	u8 nEnable;//是否启用
	//u16 nDelay;//延时
} SAlarmHDDPara;


typedef enum
{
	EM_RESOL_CIF = 1, //通道分辨率
	EM_RESOL_D1 = 4, //通道分辨率
} EMCHNRESOL;

typedef struct
{
	EMALARMVMOTIONAREASET emSetType; //设置方式 全选/清除/部分设置
	EMCHNRESOL emResol;
	u8 nEnable; //是否启用
	u8 nSensitivity; //灵敏度
	u16 nDelay; //延时	
	char reserved[6];
	u8 nRows; //区域划分总行数
	u8 nCols; //区域划分总列数
	u64 nBlockStatus[64]; //对应区域块状态
} SAlarmVMotionPara;

typedef struct
{
	u8 nEnable;
	u16 nDelay;
} SAlarmVLostPara;

typedef struct
{
	u8 nEnable;
	u16 nDelay;
} SAlarmVBlindPara;

typedef struct
{
	EMALARMOUTTYPE emType;
	u8 nEnable;	//是否启用
	u16 nDelay; //延时	
	char name[MAX_NAME_LEN + 1];//以及名称设置
	char reserved[8];
} SAlarmOutPara;

typedef struct
{
	u8 nEnable; //是否启用
	u8 nDuration;//蜂鸣时间时长非零将是间歇式蜂鸣(单位s) nDuration和nInterval同时设非零有效
	u8 nInterval; //蜂鸣时间间隔非零将是间歇式蜂鸣(单位s)
	u16 nDelay; //延时	
} SAlarmBuzzPara;

typedef enum
{
	EM_ALARM_PTZ_NULL = 0, //不联动
	EM_ALARM_PTZ_PRESET, //联动预置点
	EM_ALARM_PTZ_PATROL, //巡航线
	EM_ALARM_PTZ_LOCUS, //轨迹
} EMALARMPTZTYPE;

typedef struct
{
	u8 emALaPtzType; //EMALARMPTZTYPE
	u8 nChn; //通道号
	u8 nId; //表示预置点id/巡航线路id/轨迹(无效)
} SAlarmPtz;

typedef struct
{
	u8 nFlagBuzz; //蜂鸣器
	u8 nZoomChn; //大画面报警通道号 0xff 表示无效
	u8 nFlagEmail; //邮件
	u8 nSnapChn[64]; //存放触发抓图的通道号 最多同时64个;0xff表示不起用
	u8 nAlarmOut[64]; //触发报警 最多同时触发64个，放id;0xff表示不起用
	u8 nRecordChn[64]; //触发录像通道;0xff表示不起用
	SAlarmPtz sAlarmPtz[ALARM_PTZ_NUM]; //最多同时触发64个联动
} SAlarmDispatch;

typedef enum//报警布防类型枚举
{
	EM_ALARM_SCH_WEEK_DAY = 0,//每周的哪几天
	EM_ALARM_SCH_MONTH_DAY,//每月的哪几天
	EM_ALARM_SCH_EVERY_DAY,//每天
	EM_ALARM_SCH_ONCE_DAY,//只一次有效 / 今天
} EMALARMSCHTYPE;

typedef struct 
{
	u32 nStartTime;//每个时间段的起始时间
	u32 nStopTime;//结束时间
} SAlarmSchTime;

#define MAX_ALARM_TIME_SEGMENTS	12//每天可设置的时间段的最大数目

typedef struct 
{
	u8 nSchType;//报警布防的时间类型EMALARMSCHTYPE	
	SAlarmSchTime nSchTime[31][MAX_ALARM_TIME_SEGMENTS];//各段布防时间
} SAlarmSchedule;

typedef struct
{
	union
	{
		SAlarmSensorPara sAlaSensorPara;
		//yaogang modify 20141010
		SAlarmIPCExtSensorPara sAlaIPCExtSensorPara;
		SAlarmIPCExtSensorPara sAlaIPCCoverPara;
		SAlarmIPCExtSensorPara sAla485ExtSensorPara;
		SAlarmHDDPara sAlaHDDPara;
		
		SAlarmVMotionPara sAlaVMotionPara;
		SAlarmVLostPara sAlaVLostPara;
		SAlarmVBlindPara sAlaVBlindPara;
		SAlarmOutPara sAlaOutPara;
		SAlarmBuzzPara sAlaBuzzPara;
		SAlarmDispatch sAlaDispatch;
		SAlarmSchedule sAlaSchedule;
	};	
} SAlarmPara;

//报警模块参数命令
typedef enum
{
	//一般报警事件参数设置
	EM_ALARM_PARA_SENSOR = 0,
	//yaogang modify 20141010
	EM_ALARM_PARA_IPCCOVER,
	EM_ALARM_PARA_IPCEXTSENSOR,
	EM_ALARM_PARA_485EXTSENSOR,
	EM_ALARM_PARA_HDD,
	EM_ALARM_PARA_VMOTION,
	EM_ALARM_PARA_VBLIND,
	EM_ALARM_PARA_VLOST,
	
	//其他报警事件参数设置
	EM_ALARM_PARA_DISKFULL = 50,
	EM_ALARM_PARA_NETBROKEN,
	EM_ALARM_PARA_NETADDRCONFLICT,
	
	//报警输出参数设置
	EM_ALARM_PARA_ALARMOUT = 100,
	EM_ALARM_PARA_BUZZ,
	
	//一般报警事件处理设置
	EM_ALARM_DISPATCH_SENSOR = 200,
	EM_ALARM_DISPATCH_IPCEXTSENSOR,//yaogang modify 20141010
	EM_ALARM_DISPATCH_IPCCOVER,
	EM_ALARM_DISPATCH_485EXTSENSOR,
	EM_ALARM_DISPATCH_HDD,
	
	EM_ALARM_DISPATCH_VMOTION,
	EM_ALARM_DISPATCH_VBLIND,
	EM_ALARM_DISPATCH_VLOST,
	
	//一般报警事件布防设置
	EM_ALARM_SCHEDULE_SENSOR = 300,
	EM_ALARM_SCHEDULE_IPCEXTSENSOR,
	EM_ALARM_SCHEDULE_IPCCOVER,
	EM_ALARM_SCHEDULE_VMOTION,
	EM_ALARM_SCHEDULE_VBLIND,
	EM_ALARM_SCHEDULE_VLOST,
	
	//报警输出布防设置
	EM_ALARM_SCHEDULE_ALARMOUT = 400,
	EM_ALARM_SCHEDULE_BUZZ,//暂时不支持
	
	EM_ALARM_PARA_NULL = 0xffffffff,
}EMALARMPARATYPE;

s32 ModAlarmInit(SAlarmInitPara* psAlarmInitPara, PARAOUT AlaHdr* pAlaHdr);
s32 ModAlarmDeinit(AlaHdr AlaHdr);
s32 ModAlarmSetParam(AlaHdr AlaHdr, EMALARMPARATYPE emAlarmParaType, u8 id, const SAlarmPara* psAlarmPara);
s32 ModAlarmGetParam(AlaHdr AlaHdr, EMALARMPARATYPE emAlarmParaType, u8 id, PARAOUT SAlarmPara* psAlarmPara);
s32 ModAlarmWorkingEnable(AlaHdr AlaHdr, u8 nEnable);//nEnable: 0,模块停止工作；非0,模块开始工作
u8 GetAlarmCheckStaue();

s32 ModAlarmSetTimeZoneOffset(int nOffset);
s32 ModAlarmGetTimeZoneOffset();

#ifdef __cplusplus
}
#endif

#endif //_MOD_ALARM_H_

