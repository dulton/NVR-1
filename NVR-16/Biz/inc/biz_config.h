#ifndef _BIZ_CONFIG_H_
#define _BIZ_CONFIG_H_

#include "biz.h"
#include "mod_config.h"
#include "biz_preview.h"
#include "biz_encode.h"
#include "biz_record.h"
#include "biz_syscomplex.h"
#include "biz_alarm.h"
#include "biz_ptz.h"
#include "biz_manager.h"
#include "biz_net.h"

typedef struct
{
    
} SBizConfig;

#ifdef  __cplusplus
extern "C" {
#endif


s32 ConfigInit(u8 nEnable, SBizConfig* psInitPara);
s32 DefaultConfigRelease(void);//cw_update
s32 DefaultConfigResume(void);
s32 ConfigGetDvrInfo(u32 bDefault,SBizDvrInfo *psPara);
s32 ConfigGetSystemPara(u32 bDefault,SBizSystemPara *psPara);
s32 ConfigGetCfgPreview(u32 bDefault,SBizCfgPreview *psPara);
s32 ConfigGetCfgPatrol(u32 bDefault,SBizCfgPatrol *psPara);
s32 ConfigGetPreviewImagePara(u32 bDefault,SBizPreviewImagePara *psPara, u8 nId);
s32 ConfigGetPlayBackImagePara(u32 bDefault,SBizPreviewImagePara *psPara, u8 nId);
s32 ConfigGetCfgStrOsd(u32 bDefault,SBizCfgStrOsd *psPara, u8 nId);
s32 ConfigGetCfgRectOsd(u32 bDefault,SBizCfgRectOsd *psPara, u8 nId);
s32 ConfigGetVMainEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId);
s32 ConfigGetIsPatrolPara(u32 bDefault,SBizCfgPatrol *psPara, u8 nId);
s32 ConfigGetVSubEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId);
s32 ConfigGetVMobEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId);
s32 ConfigGetRecPara(u32 bDefault,SBizRecPara *psPara, u8 nId);
s32 ConfigGetAlarmSensorPara(u32 bDefault,SBizAlarmSensorPara *psPara, u8 nId);
//yaogang modify 20141010
s32 ConfigGetAlarmIPCExtSensorPara(u32 bDefault, SBizAlarmIPCExtSensorPara *psPara, u8 nId);

s32 ConfigGetAlarmVMotionPara(u32 bDefault,SBizAlarmVMotionPara *psPara, u8 nId);
s32 ConfigGetAlarmVLostPara(u32 bDefault,SBizAlarmVLostPara *psPara, u8 nId);
s32 ConfigGetAlarmVBlindPara(u32 bDefault,SBizAlarmVBlindPara *psPara, u8 nId);
s32 ConfigGetAlarmOutPara(u32 bDefault,SBizAlarmOutPara *psPara, u8 nId);
s32 ConfigGetAlarmBuzzPara(u32 bDefault,SBizAlarmBuzzPara *psPara, u8 nId);
//一般报警事件触发参数设置
s32 ConfigGetAlarmSensorDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId);
s32 ConfigGetAlarmIPCExtSensorDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId);

s32 ConfigGetAlarmVMotionDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId);
s32 ConfigGetAlarmVBlindDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId);
s32 ConfigGetAlarmVLostDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId);
//一般报警事件布防设置
s32 ConfigGetAlarmSensorSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId);
s32 ConfigGetAlarmVMotionSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId);
s32 ConfigGetAlarmVBlindSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId);
s32 ConfigGetAlarmVLostSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId);
//报警输出事件布防设置
s32 ConfigGetAlarmOutSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId);
s32 ConfigGetAlarmBuzzSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId);
//录像布防
s32 ConfigGetSchRecSchedule(u32 bDefault,SBizSchedule *psPara, u8 nId);
s32 ConfigGetVMotionAlaRecSchedule(u32 bDefault,SBizSchedule *psPara, u8 nId);
s32 ConfigGetSensorAlaRecSchedule(u32 bDefault,SBizSchedule *psPara, u8 nId);
//云台参数
s32 ConfigGetPtzPara(u32 bDefault,SBizPtzPara *psPara, u8 nId);
//用户
s32 ConfigGetUserPara(u32 bDefault,SBizUserPara *psPara);

//NVR used
s32 ConfigGetIPCameraPara(u32 bDefault, SBizIPCameraPara *psPara, u8 nId);

s32 ConfigSetSystemPara(SBizSystemPara *psPara);
s32 ConfigSetCfgPreview(SBizCfgPreview *psPara);
s32 ConfigSetCfgPatrol(SBizCfgPatrol *psPara);
s32 ConfigSetPreviewImagePara(SBizPreviewImagePara *psPara, u8 nId);
s32 ConfigSetCfgStrOsd(SBizCfgStrOsd *psPara, u8 nId);
s32 ConfigSetCfgRectOsd(SBizCfgRectOsd *psPara, u8 nId);
s32 ConfigSetVMainEncodePara(SBizEncodePara *psPara, u8 nId);
s32 ConfigSetIsPatrolPara(SBizCfgPatrol *psPara, u8 nId);
s32 ConfigSetVSubEncodePara(SBizEncodePara *psPara, u8 nId);
s32 ConfigSetRecPara(SBizRecPara *psPara, u8 nId);
s32 ConfigSetAlarmSensorPara(SBizAlarmSensorPara *psPara, u8 nId);
s32 ConfigSetAlarmIPCExtSensorPara(SBizAlarmIPCExtSensorPara *psPara, u8 nId);

s32 ConfigSetAlarmVMotionPara(SBizAlarmVMotionPara *psPara, u8 nId);
s32 ConfigSetAlarmVLostPara(SBizAlarmVLostPara *psPara, u8 nId);
s32 ConfigSetAlarmVBlindPara(SBizAlarmVBlindPara *psPara, u8 nId);
s32 ConfigSetAlarmOutPara(SBizAlarmOutPara *psPara, u8 nId);
s32 ConfigSetAlarmBuzzPara(SBizAlarmBuzzPara *psPara, u8 nId);
//一般报警事件触发参数设置
s32 ConfigSetAlarmSensorDispatch(SBizAlarmDispatch *psPara, u8 nId);
s32 ConfigSetAlarmVMotionDispatch(SBizAlarmDispatch *psPara, u8 nId);
s32 ConfigSetAlarmVBlindDispatch(SBizAlarmDispatch *psPara, u8 nId);
s32 ConfigSetAlarmVLostDispatch(SBizAlarmDispatch *psPara, u8 nId);
//一般报警事件布防设置
s32 ConfigSetAlarmSensorSchedule(SBizAlarmSchedule *psPara, u8 nId);
s32 ConfigSetAlarmVMotionSchedule(SBizAlarmSchedule *psPara, u8 nId);
s32 ConfigSetAlarmVBlindSchedule(SBizAlarmSchedule *psPara, u8 nId);
s32 ConfigSetAlarmVLostSchedule(SBizAlarmSchedule *psPara, u8 nId);
//报警输出事件布防设置
s32 ConfigSetAlarmOutSchedule(SBizAlarmSchedule *psPara, u8 nId);
s32 ConfigSetAlarmBuzzSchedule(SBizAlarmSchedule *psPara, u8 nId);
//录像布防
s32 ConfigSetSchRecSchedule(SBizSchedule *psPara, u8 nId);
s32 ConfigSetVMotionAlaRecSchedule(SBizSchedule *psPara, u8 nId);
s32 ConfigSetSensorAlaRecSchedule(SBizSchedule *psPara, u8 nId);
//云台参数
s32 ConfigSetPtzPara(SBizPtzPara *psPara, u8 nId);
//用户
s32 ConfigSetUserPara(SBizUserPara *psPara);

//NVR used
s32 ConfigSetIPCameraPara(SBizIPCameraPara *psPara, u8 nId);

//获得字符串取值列表
s32 ConfigGetParaStr(SBizConfigParaList *psBizConfigParaList);
//根据数字取值获得字符串列表中的index
s32 ConfigGetParaListIndex(SBizConfigParaIndex *psBizConfigParaIndex);
//根据index获得实际参数取值
s32 ConfigGetParaListValue(SBizConfigParaValue *psBizConfigParaValue);

s32 ConfigBackup(char* pszFileName);

s32 ConfigSyncFileToFlash(u8 nMode);

s32 ConfigGetMaxChnNum(void);

s32 ConfigDefault(void);

//yaogang modify 20141209 深广平台参数
s32 ConfigGetSGPara(u32 bDefault, SBizSGParam *psPara);
s32 ConfigSetSGPara(SBizSGParam *psPara);

//日常图片配置
s32 ConfigGetSGDailyPara(u32 bDefault,SBizDailyPicCFG *psPara);
s32 ConfigSetSGDailyPara(SBizDailyPicCFG *psPara);

//报警图片配置
s32 ConfigGetSGAlarmPara(u32 bDefault, SBizAlarmPicCFG *psPara, u8 nId);
s32 ConfigSetSGAlarmPara(SBizAlarmPicCFG *psPara, u8 nId);


//抓图参数
s32 ConfigGetSnapChnPara(u32 bDefault, SBizSnapChnPara *psPara, u8 nId);
s32 ConfigSetSnapChnPara(SBizSnapChnPara *psPara, u8 nId);

#ifdef  __cplusplus
}
#endif



#endif

