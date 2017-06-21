#ifndef _MOD_PLAYBACK_H_
#define _MOD_PLAYBACK_H_

#ifndef PACK_NO_PADDING
#define PACK_NO_PADDING  __attribute__ ((__packed__))
#endif

typedef void* PbMgrHandle;

typedef struct 
{
	u8   nMaskType;
	u32  nMaskChn;
	u32  nStartTime;
	u32  nEndTime;
	void* pReserverInfo;
} SPBSearchPara;

typedef enum
{
	EM_CTL_NULL=0,
	EM_CTL_STOP,
	EM_CTL_PAUSE,
	EM_CTL_RESUME,
	EM_CTL_STEP,
	EM_CTL_PRE_SECT,
	EM_CTL_NXT_SECT,
	EM_CTL_SPEED_DOWN,
	EM_CTL_SPEED_UP,
	EM_CTL_SET_SPEED,
	EM_CTL_SEEK,
	EM_CTL_FORWARD,
	EM_CTL_BACKWARD,
	EM_CTL_MUTE,
	EM_CTL_PBTIME,
	EM_CTL_PBFILE
} EmPBCtlCmd;

#if 0
typedef struct
{
	u8   channel_no;//u32  channel_no;//csp modify
	u8   type;
	u32  start_time;
	u32  end_time;
	u8   image_format;//3:cif;4:4cif
	u8   stream_flag;//0:视频流;1:音频流
	u32	 size;
	u32  offset;
	u8   disk_no;
	u8   ptn_no;
	u16  file_no;
}PACK_NO_PADDING recfileinfo_t;
#endif

typedef struct
{
    u8 nChn;
    u8 nType;
    u32 nStartTime;
    u32 nEndTime;
    u8 nFormat;  //N制式下 10 = D1，13 = CIF  P制式下 0 = D1， 3 = CIF
    u8 nStreamFlag; //0:视频流;1:音频流
    u32 nSize;
    u32 nOffset;
    u8 nDiskNo;
    u8 nPtnNo;
    u16 nFileNo;
} PACK_NO_PADDING SPBRecfileInfo;

typedef struct
{
	u8 nChn;
	u8 nType;
	u8 nFormat;  //格式,0-jpeg,1-bmp,默认jpeg 
	u8 nReserver;
	u16 	width;
	u16	height;
	u32 nSize;
	u32 nOffset;
	u8 nDiskNo;
	u8 nPtnNo;
	u16 nFileNo;
	u32 nStartTime;
	u32 nStartTimeus;
} PACK_NO_PADDING SPBRecSnapInfo;



typedef enum										//播放速率枚举
{
	EM_PLAYRATE_1 = 1,								//正常播放
	EM_PLAYRATE_2 = 2,								//2x快放
	EM_PLAYRATE_4 = 4,								//4x快放
	
	//csp modify 20130429
	//EM_PLAYRATE_MAX = 8,							//8x快放
	EM_PLAYRATE_8 = 8,								//8x快放
	EM_PLAYRATE_16 = 16,							//16x快放
	EM_PLAYRATE_32 = 32,							//32x快放
	EM_PLAYRATE_64 = 64,							//64x快放
	
	EM_PLAYRATE_1_2 = -2,							//1/2x慢放
	EM_PLAYRATE_1_4 = -4,							//1/4x慢放
	EM_PLAYRATE_MIN = -8,							//1/8x慢放
	EM_PLAYRATE_SINGLE = 0,							//帧进
	EM_PLAYRATE_BACK_1 = -1,						//1x退放
}EmPlayRate;

typedef struct
{
	BOOL bStop;
	u32 nProg;
	u32 nTotalTime;
	u64 nCurTime;
}SPBCallbackInfo;

typedef struct
{
	u32 nMaxFrameSize;
	u64  nChnMaskOfD1;	
}SPBInitPara;

typedef void (* PBPROGFUNC)(SPBCallbackInfo* cbInfo);
typedef void (* PBREGFUNCCB)(void);

#ifdef __cplusplus
extern "C" 
{
#endif

//初始化回放模块
PbMgrHandle ModPlayBackInit(u32 nMaxChn, void *pContent, u64 nChnMaskOfD1);

//开始按时间回放 
s32 ModPlayBackByTime(PbMgrHandle hPbMgr, SPBSearchPara* pSearchParam);

//按文件回放
s32 ModPlayBackByFile(PbMgrHandle hPbMgr, SPBRecfileInfo* pFileInfo);

//回放控制 
s32 ModPlayBackControl(PbMgrHandle hPbMgr, EmPBCtlCmd emPBCtlcmd, s32 nContext);

//释放回放模块
s32 ModPlayBackDeinit(PbMgrHandle hPbMgr);

//回放进度
s32 ModPlayBackProgress(PBPROGFUNC pbProgFunc);

//注册回调函数,参数type: 0,关闭电子放大
s32 ModPlayBackRegistFunCB(u8 type, PBREGFUNCCB func); 

//获取回放时各通道是否有文件要回放
s32 ModPlayBackGetRealPlayChn(PbMgrHandle hPbMgr, u64* pChnMask);

//设置当前是否在语音对讲，需要屏蔽回放的音频
s32 ModPlayBackSetModeVOIP(PbMgrHandle hPbMgr, u8 nVoip);

/*回放时单通道放大/恢复 (说明:nKey<0,自动依次放大下一个通道; 
							  nKey>=最大通道数,恢复多通道回放;
							  nKey其它,放大第nKey个窗口)*/
s32 ModPlayBackZoom(PbMgrHandle hPbMgr, s32 nKey);

/*获取某通道文件的格式 ， 返回值: 0,D1; 3,cif*/
s32 ModPlayBackGetVideoFormat(PbMgrHandle hPbMgr, u8 nChn);

//yaogang modify 20150112
//s32 ModSnapDisplay(PbMgrHandle hPbMgr, SPBRecSnapInfo* pSnapInfo);
s32 ModSnapDisplay(SPBRecSnapInfo* pSnapInfo);


#ifdef __cplusplus
}
#endif

#endif  //_PLAYBACK_H_

