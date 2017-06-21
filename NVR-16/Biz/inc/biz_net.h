#ifndef __BIZNET_H_
#define __BIZNET_H_

#include "biz.h"
#include "netcomm.h"
#include "biz_manager.h"
#include "mediastream.h"
#include "CmdCallback.h"
#include "sg_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define 

#define PAL 12
#define NTSC 10

#define AUDIO_SAMPLE_BITWIDTH		16

#define CHN_MAX						16

#define BIZ_NET_ONLINE_MAX 			1024
#define BIZ_NET_USER_NAME_MAX		15

//#define MAX_FRAME_SIZE				(s32)256*1024//zlb20100802

#define CUR_STARDMODE	PAL

// typdef 

typedef struct
{
	u8 		nChNum;
	
	//csp modify 20121130
	//int 	nVs;
	u16		reserved;
	u8		nLangId;
	u8		nVs;
	
	u32 	nFrameSizeMax;
	
	char 	device_name[32];				//设备名称
	char 	device_mode[32];				//设备型号
	u8		maxChnNum;						//最大通道数
	u8		maxAduioNum;					//最大音频数
	u8		maxPlaybackNum;					//最大回放数
	u8		maxAlarmInNum;					//最大报警输入数
	u8		maxAlarmOutNum;					//最大报警输出数
	u8		maxHddNum;						//最大硬盘数
	u8 		nNVROrDecoder;	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
} SNetSysPara;

// extern ..

s32 NetInit(void* pHddMgr, SNetSysPara* pPara);

s32 bizNet_Init();
void bizNet_RequestPreviewCB(STcpStreamReq* param);
void bizNet_remoteplayCB(
	void* pFileHandle,
	PSRmtPlayFileOp pParam
);

typedef struct _sBizNetOnlineUsr
{
	u32  nId;
	u32  nIP;
	u64  nMac;
	char szName[BIZ_NET_USER_NAME_MAX];
	char szPass[BIZ_NET_USER_NAME_MAX];
} SBNOnlineUser;

s32 BizNetAddLoginUser( SBNOnlineUser* pUser );
s32 BizNetDelLoginUser( u32 nId );

s32 BizNetGetLoginUserID( SBNOnlineUser* pUser );
void BizNetUpdateProgressNoitfyCB(u8 nProgress);
void BizNetStopEnc(void);

#ifdef __cplusplus
}
#endif


#endif // __A_H_


