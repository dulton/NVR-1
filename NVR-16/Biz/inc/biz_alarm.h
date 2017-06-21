#ifndef _BIZ_ALARM_H_
#define _BIZ_ALARM_H_

#include "biz.h"
#include "biz_config.h"
#include "biz_encode.h"
#include "mod_alarm.h"
#include "biz_manager.h"

typedef struct
{
    u8 nVideoChnNum; //视频通道数
    u8 nAlarmSensorNum; //传感器通道数
    u8 nAlarmOutNum; //报警输出通道数
    u8 nBuzzNum; //蜂鸣器个数;暂时只支持1个
    u32 nVBlindLuma; //遮挡报警亮度阀值
    u32 nDiskNum;//硬盘数目
    //yaogang modify 20150324 
    //跃天: 1 nvr，2 轮巡解码器，3 切换解码器
    u8 nNVROrDecoder;
} SBizAlarm;

s32 AlarmInit(u8 nEnable, SBizAlarm* psInitPara);
s32 AlarmSetPara(u8 nChn, EMBIZPARATYPE emType, void* pData);
s32 AlarmGetPara(u8 nChn, EMBIZPARATYPE emType, PARAOUT void* pData);
s32 AlarmWorkingEnable(u8 nEnable); //nEnable: 0,模块停止工作；非0,模块开始工作

#endif
