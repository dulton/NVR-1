#ifndef _BIZ_H_
#define _BIZ_H_

#include "common_basetypes.h"
#include "common_geometric.h"
#include "common_mutex.h"
#include "common_semaphore.h"
#include "common_thread.h"
#include "common_debug.h"
#include "public.h"

#include "biz_types.h"
#include "biz_errcode.h"
#include "diskmanage.h"
//#include "../../Biz/inc/biz_playback.h"

#ifdef __cplusplus
extern "C" {
#endif

//////////////////业务层初始化///////////////
//业务层初始化及反初始化
s32 BizInit(SBizInitPara* psBizPara);
s32 BizDeinit(void);

//csp modify
//校验序列号
s32 BizVerifySN();

//////////////////预览模块相关///////////////
//手动开始及停止预览
s32 BizStartPreview(SBizPreviewPara* psBizPreviewPara);
s32 BizStopPreview(void);

//单通道预览
s32 BizChnPreview(u8 nChn);
//电子放大预览
s32 BizElecZoomPreview(int flag, u8 nChn, SBizPreviewElecZoom* stCapRect);
s32 BizPbElecZoomPreview(int flag, u8 nChn, SBizPreviewElecZoom* stCapRect);

//图片预览
s32 BizPreviewSnap(int flag, SBizPreviewElecZoom* stMenuRect);

s32 BizPlayBackRegistFunCB(u8 type, BIZPBREGFUNCCB func);

//单通道预览或者预览停止后恢复多画面
s32 BizResumePreview(u8 nForce);
//预览下一页
s32 BizNextPreview(void);
s32 BizLastPreview(void);
s32 BizNextPreviewMod(void);
s32 BizLastPreviewMod(void);
//单通道放大
s32 BizPreviewZoomChn(u8 chn);

//手动开启或者停止预览静音
s32 BizPreviewMute(u8 nEnable);
//控制音频预览输出通道
s32 BizPreviewAudioOutChn(u8 nChn);//0xff表示自动
//控制音频预览音量
s32 BizPreviewVolume(u8 nVolume);
//直接设置预览图像参数而不保存配置
s32 BizPreviewSetImage(u8 nChn, SBizPreviewImagePara* psImagePara);
s32 BizPlayBackSetImage(SBizVoImagePara* psVoImagePara);
//获取预览图像参数
s32 BizPreviewGetImage(u8 nChn, SBizPreviewImagePara* psImagePara);

//手动开启停止轮巡
s32 BizStartPatrol(void);
s32 BizStopPatrol(void);

//////////////////录像模块相关///////////////
s32 BizStartManualRec(u8 nChn);
s32 BizStopManualRec(u8 nChn);
s32 BizRecordStop(u8 nChn);
s32 BizRecordStopAll(void);
s32 BizRecordPause(u8 nChn);
s32 BizRecordResume(u8 nChn);
void BizRecordSetWorkingEnable(u8 nEnable);

/////////////////报警模块//////////////
s32 BizAlarmWorkingEnable(u8 nEnable); //nEnable: 0,模块停止工作；非0,模块开始工作


/////////////////回放模块相关///////////////
s32 BizStartPlayback(EMBIZPLAYTYPE emBizPlayType, void* para);
s32 BizPlaybackControl(EMBIZPLAYCTRLCMD emBizPlayCtrlCmd, s32 nContext);
s32 BizPlayBackZoom(s32 nKey);
s32 BizPlayBackGetRealPlayChn(u64* pChnMask);
s32 BizPlayBackGetVideoFormat(u8 nChn);
s32 BizPlayBackSetModeVOIP(u8 nVoip);
void SetCurPlayRate(int rate);
void BizSetPlayType(int type);
int GetCurPlayRate();

//yaogang modify 20150112
s32 BizSnapDisplay(SBizRecSnapInfo *pSnapInfo);


/////////////////云台模块相关///////////////
s32 BizPtzCtrl(u8 nChn, EMBIZPTZCMD emCmd, s16 nId); 

//检测云台是否在巡航
s32 BizPtzCheckTouring(u8 nChn);

//如果在杂项模块未初始化之前需要重启就调用该接口
void BizSystemRestart();

/////////////////杂项模块相关///////////////
//	函数功能:系统退出
s32 BizSysComplexExit(EMBIZSYSEXITTYPE emSysExitType);

//函数功能:系统升级
s32 BizSysComplexUpgrade(EMBIZUPGRADETARGET emUpdateTarget, char *pcUpdateFName);

//函数功能:系统升级文件列表
s32 BizSysComplexUpgradeFilelist(EMBIZUPGRADETARGET emUpdateTarget, SBizUpdateFileInfoList* spFileList);

//函数功能:配置文件列表
s32 BizSysComplexConfigFilelist(char* szDir, SBizUpdateFileInfoList* spFileList);
//函数功能:usb设备挂载路径列表
s32 BizSysComplexUsbDevMountDirlist(SBizUpdateFileInfoList* spFileList);

//函数功能:中断文件备份
s32 BizSysCmplexBreakBackup(int type);

//设置升级文件产品验证型号
void BizSysComplexSetProductModel(char* updatemodel);

//获取新磁盘插入时系统检测状态
u8 BizSysGetInsertDiskStatue();

//设置磁盘系统检测状态
void BizSysSetRefreshDiskStatue(u8 flag);

//获取报警上传磁盘检测状态
u8 BizSysGetCheckDiskStatue();
	
//函数功能:文件备份
s32 BizSysComplexBackup(EMBIZFILETYPE emFileType, 
						 PSBizBackTgtAtt	psBackTgtAtt, 
						 SBizSearchCondition* psSearchCondition);

//函数功能:获得存储设备管理信息
s32 BizSysComplexDMGetInfo(SBizStoreDevManage *psStoreDevManage, u8 nMaxDiskNum);

//函数功能:格式化
s32 BizSysComplexDMFormat(char *pstrDevPath, EMBIZFORMATWAY emFormatWay);

//函数功能:文件搜索
s32 BizSysComplexDMSearch(EMBIZFILETYPE emFileType, SBizSearchPara* psSearchPara, SBizSearchResult* psSearchResult, u16 nMaxFileNum);

//函数功能:设置系统日期时间
s32 BizSysComplexDTSet(const SBizDateTime* psDateTime);

//函数功能:获得系统日期时间
s32 BizSysComplexDTGet(SBizDateTime* psDateTime);

//函数功能:获得格式化系统日期时间字符串
s32 BizSysComplexDTGetString(char* pstrDateTime);

//获得编码参数
s32 BizGetVMainEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId);
s32 BizSetVMainEncodePara(SBizEncodePara *psPara, u8 nId);
s32 BizGetVSubEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId);
s32 BizSetVSubEncodePara(SBizEncodePara *psPara, u8 nId);

//获取/设置轮询参数
s32 BizGetIsPatrolPara(u32 bDefault,SBizCfgPatrol *psPara, u8 nId);
s32 BizSetIsPatrolPara(SBizCfgPatrol *psPara, u8 nId);

s32 BizSendBizEvent(SBizEventPara* psBizEventPara);

//获取软件版本号
s32 BizConfigGetDvrInfo(u32 flag, char* sDvr_info, char* sDvr_Model);
//////////////////配置模块相关///////////////
s32 BizConfigDefault(void);//恢复默认设置
s32 BizConfigBackup(char* pszFileName); //备份配置文件
s32 BizConfigResume(EMBIZCONFIGPARATYPE emType, s32 nId, s8* pFilePath); //从配置备份文件中恢复配置
s32 BizConfigResumeDefault(EMBIZCONFIGPARATYPE emType, s32 nId); // 从默认配置文件中恢复配置

//获得参数字符串取值列表
s32 BizConfigGetParaStr(EMBIZCFGPARALIST emParaType, PARAOUT s8* strList[], PARAOUT u8* nRealListLen, u8 nMaxListLen, u8 nMaxStrLen);
//根据参数取值获得字符串列表中的index
s32 BizConfigGetParaListIndex(EMBIZCFGPARALIST emParaType, s32 nValue, PARAOUT u8* pnIndex);
//根据index获得实际参数取值
s32 BizConfigGetParaListValue(EMBIZCFGPARALIST emParaType, u8 nIndex, PARAOUT s32* pnValue);

//////////////////配置及其他所有模块设置相关///////////////
//获得及设置参数
//(设置时如果只改变结构体部分参数先get再set)
s32 BizSetPara(SBizParaTarget* psBizParaTarget, void* pData);
s32 BizGetPara(SBizParaTarget* psBizParaTarget, PARAOUT void* pData);
s32 BizGetDefaultPara(SBizParaTarget* psBizParaTarget, PARAOUT void* pData);

int BizGetVideoFormatReal(void);

//用户
s32 BizUserLogin(SBizUserLoginPara* para, u32* nUserId);
s32 BizUserLogout(u32 nUserId);
EMBIZUSERCHECKAUTHOR BizUserCheckAuthority(EMBIZUSEROPERATION emOperation,u32 * para,u32 nUserId);
s32 BizUserAddUser(SBizUserUserPara* para);
s32 BizUserDeleteUser(s8* pUserName);

//日志
s32 BizWriteLog(SBizLogInfo* psLogInfo);
s32 BizSetUserNameLog(char* pName);//cw_log
s32 BizSearchLog(SBizLogSearchPara* psLogSearch, SBizLogResult* psLogResult);
s32 BizExportLog(SBizLogResult* psLogResult, u8* pPath);
// 网络模块接口

// 获取网络状态 连接/断开
EM_BIZNET_STATE BizNetGetState();
s32 BizNetReadCfg( SBizNetPara* pCfg );
s32 BizNetReadCfgEx( SBizNetPara* pCfg );
s32 BizNetWriteCfg( SBizNetPara* pCfg );
int BizSetHWAddr(char *pBuf);
void BizNetStartDHCP();
void BizNetStopDHCP();

s32 BizNetStartPPPoE(char *pszUser, char* pszPasswd);
s32 BizNetStopPPPoE();

s32 BizNetSendMail(SBizMailContent* pMail);

s32 BizNetDdnsCtrl( EM_BIZ_DDNS_ACT eAct, sBizNetDdnsPara* pPara );

s32 BizGetDvrInfo( SBizDvrInfo* pInfo );

s32 BizGetSysInfo(SBizSystemPara* sysPara);

s32 BizNetReqConnStatus(SBizNetConnStatus* psNCS);

void BizSetZoomMode(u8 flag);//cw_zoom

void BizSetDwellStartFlag(u8 flag);
u8 BizGetDwellStartFlag();
void BizSetUploadAlarmPara(u8 type, u8 id, u8 state);

// return:  0 closed, 1 opened
u8 BizNetGetVoipState( void );

BOOL DiskIsExist(); //add by Lirl on Nov/11/2011
u8 GetHardDiskNum();
s32 BizGetMaxChnNum(void);

void BizSetCurRecStatus(u8 nChn,u8 flag);
s32 BizGetCurRecStatus(u8 nChn);

int GetChnNameMax(void);

typedef void (*CHECKNOAUTH)(void);//cw_auth
	
void BizSetNoAuthFuncCB(CHECKNOAUTH func);
u8 CheckAllHDDFormated();

typedef void (*PREVIEWMODECB)(EMBIZPREVIEWMODE);//cw_preview

typedef void (*LEDDTRLFN)(int flag,int type);//cw_led

void RegLedCtrlFUN(LEDDTRLFN fun);

void SetPrevicwModeFuncCB(PREVIEWMODECB func);

#if 0//csp modify
u64 GetTimeStamp();//cw_time
#endif

void SetBizResolHNum(int num);//cw_9508S
int GetBizResolHNum();

void BizNetUploadAlarm(u32 key);
void BizSetUploadAlarmPara(u8 type, u8 id, u8 state);
u8 GetUpgradeStatue();

#ifdef __cplusplus
}
#endif

#endif

