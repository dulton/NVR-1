#ifndef _BIZ_RECORD_H_
#define _BIZ_RECORD_H_

#include "biz.h"
#include "mod_record.h"
#include "biz_manager.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    u8 nChnNum;
    SModRecordRecPara* sModRecPara;
    RecHddInfo hddinfo;
} SBizRecord;

s32 RecordInit(u8 nEnable, SBizRecord* psInitPara);
s32 RecordDeInit();
s32 RecordStartManual(u8 nChn);
s32 RecordStopManual(u8 nChn);
s32 RecordStop(u8 nChn);
s32 RecordStopAll(void);
//yaogang modify 20150314
s32 RecordSnapPause(u8 cause);
s32 RecordSnapResume(u8 cause);

s32 RecordPause(u8 nChn);
s32 RecordResume(u8 nChn);
s32 RecordSetPreTime(u8 nChn, u16 nTime);
s32 RecordSetDelayTime(u8 nChn, u16 nTime);
s32 RecordSetTrigger(u8 nChn, EMRECALARMTYPE emType, u8 nKey);
s32 RecordSetSchedulePara(u8 nChn, SModRecSchPara* psPara);
s32 RecordWriteOneFrame(u8 nChn, SModRecRecordHeader* psHeader);
void RecordSetDealHardDiskFull(u8 nCover); //硬盘满时是否覆盖:0否 1是
void RecordSetWorkingEnable(u8 nEnable);
s32 RecordBufRelease(void);
s32 RecordBufResume(void);
void RecordExceptionCB(u8 nType); //异常回调函数,nType:0,硬盘异常; 1,编码异常; 2,录像文件异常; 3,重启主机

//yaogang modify 20141225
/*抓图类型	bit_mask	value
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

//biz_snap 模块使用的请求抓图类型
typedef enum
{
	//upload
	TypeManualTEST = 0,	// 0 测试图像上传--监管平台--测试保养
	TypeManualMAINTAIN,	// 1 保养图像上传
	TypeManualACCEPT,	// 2 验收管理上传
	TypeDaily,	// 3 日常
	//record
	TypePreSnap,		// 4 预录
	TypeTime,		// 5 定时-录像配置--抓图参数
	TypeManual;		// 6 手动截图
	//upload & record
	TypeAlarmSensor,	// 7 本地传感器报警
	TypeMotionDet,	// 8 移动侦测
	TypeAlarmIPCEXT,	// 9 IPC外部报警
	TypeSnapMax,
} EM_SNAP_TYPE;

typedef struct rec_header
{
	u8	nChn;					//帧的通道号
	u8	nStreamType;				//码流类型（0:视频流、1:视音频流、2: snap）
	u8	nMediaType; 				//编码方式（0:H264、10:PCMU、11:ADPCM）
	u8	nFrameType; 				//是否关键帧(0:否、1:是) FRAME_TYPE_P = 0,FRAME_TYPE_I = 1/3/5
								//nFrameType 在Snap模块用做抓图类型
	u32 nBitRate;					//比特率
	EMRECENCRESOL emResolution; 	//编码分辨率
	u64 nPts;						//时间戳（微妙）
								//nPts 在Snap模块用做struct timeval {time_t tv_sec; suseconds_t tv_usec}; gettimeofday(); )
	u64 nTimeStamp; 				//帧时间戳（毫秒）
	u8* nDate;					//指向帧数据的指针
	u32 nDataLength;				//帧数据的真实长度
	u16 width;
	u16 height;
}SModRecRecordHeader;
*/

//int RecordSnapshotToFile(u8 chn, u8 pic_type, void *pdata, u32 data_size, u32 width, u32 height);
s32 RecordWriteOneSnap(u8 nChn, SModRecRecordHeader* psHeader);
s32 RecordReadOneSnap(u8 nChn, SModRecRecordHeader* psHeader);
s32 RecordReadOnePreSnap(u8 nChn, SModRecRecordHeader* psHeader);


#ifdef __cplusplus
}
#endif


#endif
