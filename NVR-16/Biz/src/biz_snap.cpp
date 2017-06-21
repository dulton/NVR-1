#include <time.h>
#include "MultiTask/Timer.h"
#include "biz.h"
#include "biz_config.h"
#include "biz_encode.h"
#include "biz_record.h"
#include "mod_alarm.h"
#include "biz_snap.h"
#include "ipcamera.h"
#include "sg_platform.h"
#include "System/Locales.h"
#include "mod_encode.h"
#include "circlebuf.h"


#define g_SnapManager (*CSnapManager::instance())

#define TEXTLEN	(32)
#define nSECPERMIN	(60)
#define nSECPERHOUR	(60*60)
const int indextosec[] = {1, 2, 3, 4, 5, 10*nSECPERMIN, 30*nSECPERMIN, nSECPERHOUR, 12*nSECPERHOUR, 24*nSECPERHOUR};

#define SNAPBUFSIZE (64 << 10)


typedef struct {
	time_t StartTime;	//报警触发时间
	time_t EndTime;	//报警解除时间
} sTimeRange;

typedef struct
{
	u16 width;
	u16 height;
}SnapResolutionType;

const SnapResolutionType SnapRes[] = {
	{352, 288},	//AUTO
	{704, 576},	
	{352, 288},
	{176, 144},
};

typedef enum
{
	TypePause,
	TypeResume,
} EM_SNAPMSG_TYPE;


//通道得到抓图后，依据以下信息做后续处理
typedef struct
{
	
	u32 ChnReqTypeMask;	//请求抓图标记，抓图类型
	
	//是否要上传报警触发时刻到前置时间点的该通道所有图片
	//共有m_MaxIpcChn个变量，每一位指示报警源
	u32 UploadPreRec_Sensor;//有效位数: m_MaxSensorNum
	u32 UploadPreRec_IPCMD;
	u32 UploadPreRec_IPCEXT;
	//是否要上传当前时刻该通道所有图片
	//共有m_MaxIpcChn个变量，每一位指示报警源
	u32 UploadCur_Sensor;//有效位数: m_MaxSensorNum
	u32 UploadCur_IPCMD;
	u32 UploadCur_IPCEXT;

	CTimer *p_ChnRequestSnapTimer;	//每个通道一个
	VD_BOOL TimerWorking;	//标示定时器线程是否正在工作中

	//图片BUFFER
	u8 *pSnapData;
	//CMutex *pChnMutex;//通道信息锁
}SnapChnProcessDataType;


class CSnapManager : public CThread
{
public:
	PATTERN_SINGLETON_DECLARE(CSnapManager);
	
	~CSnapManager();
	VD_BOOL Start(u8 MaxSensorNum, u8 MaxIpcChn);
	void ThreadProc();
	//int RequestSnap(u8 chn, EM_SNAP_TYPE type);
	int AlarmStatusChange(u8 chn, EM_SNAP_TYPE type, int status);
	void Snapshot_Register(u8 chn);
	//上传报警触发时刻到前置时间点内多张预录图片
	int UploadPreRecPic(EM_SNAP_TYPE type, u8 AlarmChn, u8 IPCChn);
	//时间必须作为参数，可以指定，因为要上传之前的预录图片
	//一组报警消息的所有GUID编码相同。
	int UploadPic(EM_SNAP_TYPE type, u8 AlarmChn, u8 IPCChn, time_t time, void *SnapData, u32 DataSize, int GUID);

	int requestSnap(u8 chn, EM_SNAP_TYPE type, const char *PoliceID, const char *pswd);
	int alarmStatusChange(u8 chn, EM_ALARM_TYPE type, int status);
	int SnapWriteMsgQueue(EM_SNAPMSG_TYPE msg);
	int SnapReadMsgQueue(EM_SNAPMSG_TYPE *pmsg);

protected:
	CSnapManager();
	
private:
	VD_BOOL  m_Started;	//初始化完成，服务开启
	CMutex *pChnMutex;//通道信息锁
	u8 m_MaxSensorNum;	//本机传感器数目
	u8 m_MaxIpcChn;		//通道数目
	
	//CTimer **pp_ChnRequestSnapTimer;	//每个通道一个
	//volatile u32 TimerWorkingMask;	//每个通道一位，标示定时器线程是否正在工作中
	volatile u32 *p_ChnReqTypeMask;	//请求抓图标记，抓图类型
	
	//是否要上传报警触发时刻到前置时间点的该通道所有图片
	//共有m_MaxIpcChn个变量，每一位指示报警源
	u32 * p_UploadPreRec_Sensor;//有效位数: m_MaxSensorNum
	u32 * p_UploadPreRec_IPCMD;
	u32 * p_UploadPreRec_IPCEXT;
	//是否要上传当前时刻该通道所有图片
	//共有m_MaxIpcChn个变量，每一位指示报警源
	u32 * p_UploadCur_Sensor;//有效位数: m_MaxSensorNum
	u32 * p_UploadCur_IPCMD;
	u32 * p_UploadCur_IPCEXT;

	//一组报警消息的所有GUID编码相同，上传监管平台的一个字段
	s32 *p_GUID_Sensor;//m_MaxSensorNum
	s32 *p_GUID_IPCMD;
	s32 *p_GUID_IPCEXT;
	//CMutex *p_ChnMutex;//通道信息锁
	//
	time_t *p_ChnTimeLast;//通道定时最后操作时间
	time_t *p_ChnPreRec;//通道预录最后操作时间
	
	//一维是IPC通道数目，二维是报警源数目
	time_t **pp_ChnRecSensor;//录像配置-- 事件-- sensor报警，chn最后保存时间[chn][sensor]
	time_t **pp_ChnRecIPCMD;//录像配置-- 事件-- IPCMD报警，chn最后保存时间
	time_t **pp_ChnRecIPCEXT;//录像配置-- 事件-- IPCEXT报警，chn最后保存时间
	time_t **pp_ChnSGSensor;//上传中心-- 报警-- sensor报警，chn最后上传时间
	time_t **pp_ChnSGIPCMD;//上传中心-- 报警-- IPCMD报警，chn最后上传时间
	time_t **pp_ChnSGIPCEXT;//上传中心-- 报警-- IPCEXT报警，chn最后上传时间

	//维数- 报警源数目
	EM_ALARM_STATUS *p_SensorStatus;	//本机报警状态
	EM_ALARM_STATUS *p_IPCMDStatus;	//IPC移动侦测状态
	EM_ALARM_STATUS *p_IPCEXTStatus;	//IPC外部报警状态

	//通道得到抓图后，依据以下信息做后续处理
	volatile SnapChnProcessDataType *pChnData;
	//用于报警
	
	//volatile u32 m_ChnSensorMask;//每一位表示一个传感器
	//volatile u32 m_ChnIPCMDMask;
	//volatile u32 m_ChnIPCEXTMask;
	sTimeRange *p_SensorRange;
	sTimeRange *p_IPCMDRange;
	sTimeRange *p_IPCEXTRange;

	char PoliceID[TEXTLEN];
	char PassWord[TEXTLEN];

	SCircleBufInfo SnapMsgQueue;
	
	int UploadAndRecDeal(EM_ALARM_TYPE type, time_t CurTime);
	int requestResource(void);
	void releaseResource(void);
	
};



/************************************************************/
PATTERN_SINGLETON_IMPLEMENT(CSnapManager);

CSnapManager::CSnapManager():CThread("SnapManager", TP_TIMER)
{
	
	m_Started 		= FALSE;	//初始化完成，服务开启
	m_MaxSensorNum	= 0;		//本机传感器数目
	m_MaxIpcChn		= 0;		//通道数目

	pChnMutex = NULL;
	p_ChnReqTypeMask = NULL;	//请求抓图标记，抓图类型
	
	//是否要上传报警触发时刻到前置时间点的该通道所有图片
	//共有m_MaxIpcChn个变量，每一位指示报警源
	p_UploadPreRec_Sensor	= NULL;//有效位数: m_MaxSensorNum
	p_UploadPreRec_IPCMD	= NULL;
	p_UploadPreRec_IPCEXT	= NULL;
	
	//是否要上传当前时刻该通道所有图片
	//共有m_MaxIpcChn个变量，每一位指示报警源
	p_UploadCur_Sensor	= NULL;//有效位数: m_MaxSensorNum
	p_UploadCur_IPCMD	= NULL;
	p_UploadCur_IPCEXT	= NULL;

	p_GUID_Sensor = NULL;
	p_GUID_IPCMD = NULL;
	p_GUID_IPCEXT = NULL;
	
	p_ChnTimeLast		= NULL;//通道定时最后操作时间
	p_ChnPreRec		= NULL;//通道预录最后操作时间
	pp_ChnRecSensor	= NULL;//录像配置-- 事件-- sensor报警，chn最后保存时间[chn][sensor]
	pp_ChnRecIPCMD	= NULL;//录像配置-- 事件-- IPCMD报警，chn最后保存时间
	pp_ChnRecIPCEXT	= NULL;//录像配置-- 事件-- IPCEXT报警，chn最后保存时间
	pp_ChnSGSensor	= NULL;//上传中心-- 报警-- sensor报警，chn最后上传时间
	pp_ChnSGIPCMD	= NULL;//上传中心-- 报警-- IPCMD报警，chn最后上传时间
	pp_ChnSGIPCEXT	= NULL;//上传中心-- 报警-- IPCEXT报警，chn最后上传时间
	
	p_SensorStatus 	= NULL;	//本机报警状态
	p_IPCMDStatus 	= NULL;	//IPC移动侦测状态
	p_IPCEXTStatus 	= NULL;	//IPC外部报警状态

	//通道得到抓图后，依据以下信息做后续处理
	pChnData 	= NULL;
	//用于报警
	
	p_SensorRange	= NULL;
	p_IPCMDRange	= NULL;
	p_IPCEXTRange	= NULL;

	memset(PoliceID, 0, sizeof(PoliceID));
	memset(PassWord, 0, sizeof(PassWord));	
		
	//printf("CSnapManager::CSnapManager()>>>>>>>>>\n");
}

int CSnapManager::requestResource(void)
{
	u8 i;

	pChnMutex = (CMutex *)operator new(m_MaxIpcChn * sizeof(CMutex));
	for (i=0; i<m_MaxIpcChn; i++)
	{
		new(&pChnMutex[i]) CMutex(MUTEX_FAST); //MUTEX_RECURSIVE
	}
	
	p_ChnReqTypeMask = new (std::nothrow) u32[m_MaxIpcChn];
	if (p_ChnReqTypeMask == NULL)
	{
		printf("%s new p_ChnReqTypeMask failed\n", __func__);
		return 1;
	}
	memset((void *)p_ChnReqTypeMask, 0, sizeof(u32)*m_MaxIpcChn);

	p_UploadPreRec_Sensor = new (std::nothrow) u32 [m_MaxIpcChn];
	if (p_UploadPreRec_Sensor == NULL)
	{
		printf("%s new p_UploadPreRec_Sensor failed\n", __func__);
		return 1;
	}
	memset(p_UploadPreRec_Sensor, 0, sizeof(u32) * m_MaxIpcChn);

	p_UploadPreRec_IPCMD = new (std::nothrow) u32 [m_MaxIpcChn];
	if (p_UploadPreRec_IPCMD == NULL)
	{
		printf("%s new p_UploadPreRec_IPCMD failed\n", __func__);
		return 1;
	}
	memset(p_UploadPreRec_IPCMD, 0, sizeof(u32) * m_MaxIpcChn);
	
	p_UploadPreRec_IPCEXT = new (std::nothrow) u32 [m_MaxIpcChn];
	if (p_UploadPreRec_IPCEXT == NULL)
	{
		printf("%s new p_UploadPreRec_IPCEXT failed\n", __func__);
		return 1;
	}
	memset(p_UploadPreRec_IPCEXT, 0, sizeof(u32) * m_MaxIpcChn);

	p_UploadCur_Sensor = new (std::nothrow) u32 [m_MaxIpcChn];
	if (p_UploadCur_Sensor == NULL)
	{
		printf("%s new p_UploadCur_Sensor failed\n", __func__);
		return 1;
	}
	memset(p_UploadCur_Sensor, 0, sizeof(u32) * m_MaxIpcChn);

	p_UploadCur_IPCMD = new (std::nothrow) u32 [m_MaxIpcChn];
	if (p_UploadCur_IPCMD == NULL)
	{
		printf("%s new p_UploadCur_IPCMD failed\n", __func__);
		return 1;
	}
	memset(p_UploadCur_IPCMD, 0, sizeof(u32) * m_MaxIpcChn);
	
	p_UploadCur_IPCEXT = new (std::nothrow) u32 [m_MaxIpcChn];
	if (p_UploadCur_IPCEXT == NULL)
	{
		printf("%s new p_UploadCur_IPCEXT failed\n", __func__);
		return 1;
	}
	memset(p_UploadCur_IPCEXT, 0, sizeof(u32) * m_MaxIpcChn);

	p_GUID_Sensor = new (std::nothrow) s32 [m_MaxSensorNum];
	if (p_GUID_Sensor == NULL)
	{
		printf("%s new p_GUID_Sensor failed\n", __func__);
		return 1;
	}
	memset(p_GUID_Sensor, 0, sizeof(s32) * m_MaxSensorNum);

	p_GUID_IPCMD = new (std::nothrow) s32 [m_MaxIpcChn];
	if (p_GUID_IPCMD == NULL)
	{
		printf("%s new p_GUID_IPCMD failed\n", __func__);
		return 1;
	}
	memset(p_GUID_IPCMD, 0, sizeof(s32) * m_MaxIpcChn);

	p_GUID_IPCEXT = new (std::nothrow) s32 [m_MaxIpcChn];
	if (p_GUID_IPCEXT == NULL)
	{
		printf("%s new p_GUID_IPCEXT failed\n", __func__);
		return 1;
	}
	memset(p_GUID_IPCEXT, 0, sizeof(s32) * m_MaxIpcChn);

	//time_t *p_ChnTimeLast;//通道定时最后操作时间
	p_ChnTimeLast = new (std::nothrow) time_t [m_MaxIpcChn];
	if (p_ChnTimeLast == NULL)
	{
		printf("%s new p_ChnTimeLast failed\n", __func__);
		return 1;
	}
	memset(p_ChnTimeLast, 0, sizeof(time_t)*m_MaxIpcChn);

	//time_t *p_ChnPreRec;//通道预录最后操作时间
	p_ChnPreRec = new (std::nothrow) time_t [m_MaxIpcChn];
	if (p_ChnPreRec == NULL)
	{
		printf("%s new p_ChnPreRec failed\n", __func__);
		return 1;
	}
	memset(p_ChnPreRec, 0, sizeof(time_t)*m_MaxIpcChn);

	//一维是IPC通道数目，二维是报警源数目
	//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
	pp_ChnRecSensor = new (std::nothrow) time_t*[m_MaxIpcChn];
	if (pp_ChnRecSensor == NULL)
	{
		printf("%s new pp_ChnRecSensor failed\n", __func__);
		return 1;
	}
	memset(pp_ChnRecSensor, 0, sizeof(time_t*) * m_MaxIpcChn);
	
	//time_t *p_ChnRecIPCMD;//录像配置-- 事件-- IPCMD最后操作时间
	pp_ChnRecIPCMD = new (std::nothrow) time_t* [m_MaxIpcChn];
	if (pp_ChnRecIPCMD == NULL)
	{
		printf("%s new pp_ChnRecIPCMD failed\n", __func__);
		return 1;
	}
	memset(pp_ChnRecIPCMD, 0, sizeof(time_t*)*m_MaxIpcChn);
	
	//time_t *p_ChnRecIPCEXT;//录像配置-- 事件-- IPCEXT最后操作时间
	pp_ChnRecIPCEXT = new (std::nothrow) time_t* [m_MaxIpcChn];
	if (pp_ChnRecIPCEXT == NULL)
	{
		printf("%s new pp_ChnRecIPCEXT failed\n", __func__);
		return 1;
	}
	memset(pp_ChnRecIPCEXT, 0, sizeof(time_t*)*m_MaxIpcChn);
	
	//time_t *p_SGSensor;//上传中心-- 报警-- sersor最后操作时间
	pp_ChnSGSensor = new (std::nothrow) time_t* [m_MaxIpcChn];
	if (pp_ChnSGSensor == NULL)
	{
		printf("%s new pp_ChnSGSensor failed\n", __func__);
		return 1;
	}
	memset(pp_ChnSGSensor, 0, sizeof(time_t*)*m_MaxIpcChn);

	//time_t *p_ChnSGIPCMD;//上传中心-- 报警-- IPCMD最后操作时间
	pp_ChnSGIPCMD = new (std::nothrow) time_t* [m_MaxIpcChn];
	if (pp_ChnSGIPCMD == NULL)
	{
		printf("%s new pp_ChnSGIPCMD failed\n", __func__);
		return 1;
	}
	memset(pp_ChnSGIPCMD, 0, sizeof(time_t*)*m_MaxIpcChn);
	
	//time_t *p_ChnSGIPCEXT;//上传中心-- 报警-- IPCEXT最后操作时间
	pp_ChnSGIPCEXT = new (std::nothrow) time_t* [m_MaxIpcChn];
	if (pp_ChnSGIPCEXT == NULL)
	{
		printf("%s new pp_ChnSGIPCEXT failed\n", __func__);
		return 1;
	}
	memset(pp_ChnSGIPCEXT, 0, sizeof(time_t*)*m_MaxIpcChn);

	for (i=0; i<m_MaxIpcChn; i++)
	{
		//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
		pp_ChnRecSensor[i] = new (std::nothrow) time_t [m_MaxSensorNum];
		if (pp_ChnRecSensor[i] == NULL)
		{
			printf("%s new pp_ChnRecSensor[i] failed\n", __func__);
			return 1;
		}
		memset(pp_ChnRecSensor[i], 0, sizeof(time_t)*m_MaxSensorNum);
		
		//time_t *p_ChnRecIPCMD;//录像配置-- 事件-- IPCMD最后操作时间
		pp_ChnRecIPCMD[i] = new (std::nothrow) time_t [m_MaxIpcChn];
		if (pp_ChnRecIPCMD[i] == NULL)
		{
			printf("%s new pp_ChnRecIPCMD[i] failed\n", __func__);
			return 1;
		}
		memset(pp_ChnRecIPCMD[i], 0, sizeof(time_t)*m_MaxIpcChn);
		
		//time_t *p_ChnRecIPCEXT;//录像配置-- 事件-- IPCEXT最后操作时间
		pp_ChnRecIPCEXT[i] = new (std::nothrow) time_t [m_MaxIpcChn];
		if (pp_ChnRecIPCEXT[i] == NULL)
		{
			printf("%s new pp_ChnRecIPCEXT[i] failed\n", __func__);
			return 1;
		}
		memset(pp_ChnRecIPCEXT[i], 0, sizeof(time_t)*m_MaxIpcChn);
		
		//time_t *p_SGSensor;//上传中心-- 报警-- sersor最后操作时间
		pp_ChnSGSensor[i] = new (std::nothrow) time_t [m_MaxSensorNum];
		if (pp_ChnSGSensor[i] == NULL)
		{
			printf("%s new pp_ChnSGSensor[i] failed\n", __func__);
			return 1;
		}
		memset(pp_ChnSGSensor[i], 0, sizeof(time_t)*m_MaxSensorNum);
		
		//time_t *p_ChnSGIPCMD;//上传中心-- 报警-- IPCMD最后操作时间
		pp_ChnSGIPCMD[i] = new (std::nothrow) time_t [m_MaxIpcChn];
		if (pp_ChnSGIPCMD[i] == NULL)
		{
			printf("%s new pp_ChnSGIPCMD[i] failed\n", __func__);
			return 1;
		}
		memset(pp_ChnSGIPCMD[i], 0, sizeof(time_t)*m_MaxIpcChn);
		
		//time_t *p_ChnSGIPCEXT;//上传中心-- 报警-- IPCEXT最后操作时间
		pp_ChnSGIPCEXT[i] = new (std::nothrow) time_t [m_MaxIpcChn];
		if (pp_ChnSGIPCEXT[i] == NULL)
		{
			printf("%s new pp_ChnSGIPCEXT[i] failed\n", __func__);
			return 1;
		}
		memset(pp_ChnSGIPCEXT[i], 0, sizeof(time_t)*m_MaxIpcChn);
	}

	p_SensorStatus= new (std::nothrow) EM_ALARM_STATUS [m_MaxSensorNum];
	if (p_SensorStatus == NULL)
	{
		printf("%s new p_SensorStatus failed\n", __func__);
		return 1;
	}
	memset(p_SensorStatus, 0, sizeof(EM_ALARM_STATUS) * m_MaxSensorNum);

	p_IPCMDStatus= new (std::nothrow) EM_ALARM_STATUS [m_MaxIpcChn];
	if (p_IPCMDStatus == NULL)
	{
		printf("%s new p_IPCMDStatus failed\n", __func__);
		return 1;
	}
	memset(p_IPCMDStatus, 0, sizeof(EM_ALARM_STATUS) * m_MaxIpcChn);

	p_IPCEXTStatus= new (std::nothrow) EM_ALARM_STATUS [m_MaxIpcChn];
	if (p_IPCEXTStatus == NULL)
	{
		printf("%s new p_IPCEXTStatus failed\n", __func__);
		return 1;
	}
	memset(p_IPCEXTStatus, 0, sizeof(EM_ALARM_STATUS) * m_MaxIpcChn);
	
	//SnapChnProcessDataType *pChnData
	pChnData	= new  (std::nothrow) SnapChnProcessDataType[m_MaxIpcChn];
	if (pChnData == NULL)
	{
		printf("%s new pChnData failed\n", __func__);
		return 1;
	}
	memset((void *)pChnData, 0, sizeof(SnapChnProcessDataType) * m_MaxIpcChn);
	for (i=0; i<m_MaxIpcChn; i++)
	{
		pChnData[i].p_ChnRequestSnapTimer = new (std::nothrow) CTimer("Snap");
		if (pChnData[i].p_ChnRequestSnapTimer == NULL)
		{
			printf("%s new pChnData[%d].p_ChnRequestSnapTimer failed\n", __func__, i);
			return 1;
		}

		pChnData[i].pSnapData =  new (std::nothrow) u8[SNAPBUFSIZE];
		if (pChnData[i].pSnapData == NULL)
		{
			printf("%s malloc pSnapData failed\n", __func__);
			return 1;
		}
	}	

	p_SensorRange = new (std::nothrow) sTimeRange [m_MaxSensorNum];
	if (p_SensorRange == NULL)
	{
		printf("%s new p_SensorRange failed\n", __func__);
		return 1;
	}
	memset(p_SensorRange, 0, sizeof(sTimeRange) * m_MaxSensorNum);	

	p_IPCMDRange = new (std::nothrow) sTimeRange [m_MaxIpcChn];
	if (p_IPCMDRange == NULL)
	{
		printf("%s new p_IPCMDRange failed\n", __func__);
		return 1;
	}
	memset(p_IPCMDRange, 0, sizeof(sTimeRange) * m_MaxIpcChn);	

	p_IPCEXTRange = new (std::nothrow) sTimeRange [m_MaxIpcChn];
	if (p_IPCEXTRange == NULL)
	{
		printf("%s new p_IPCEXTRange failed\n", __func__);
		return 1;
	}
	memset(p_IPCEXTRange, 0, sizeof(sTimeRange) * m_MaxIpcChn);

	SnapMsgQueue.nBufId = 0;	
	SnapMsgQueue.nLength = 256;//int: 0 stop 1 resume
	
	if (CreateCircleBuf(&SnapMsgQueue))
	{
		printf("%s SnapMsgQueue init failed\n", __func__);
	}
	
	return 0;
}

void CSnapManager::releaseResource(void)
{
	u8 i;

	if (pChnMutex != NULL)
	{
		for (i=0; i<m_MaxIpcChn; i++)
		{
			pChnMutex[i].~CMutex();
		}
		operator delete(pChnMutex);
		pChnMutex = NULL;
	}

	if (p_ChnReqTypeMask != NULL)
	{
		delete []p_ChnReqTypeMask;
		p_ChnReqTypeMask = NULL;
	}

	if (p_UploadPreRec_Sensor != NULL)
	{
		delete []p_UploadPreRec_Sensor;
		p_UploadPreRec_Sensor = NULL;
	}

	if (p_UploadPreRec_IPCMD != NULL)
	{
		delete []p_UploadPreRec_IPCMD;
		p_UploadPreRec_IPCMD = NULL;
	}
	
	if (p_UploadPreRec_IPCEXT != NULL)
	{
		delete []p_UploadPreRec_IPCEXT;
		p_UploadPreRec_IPCEXT = NULL;
	}

	if (p_UploadCur_Sensor != NULL)
	{
		delete []p_UploadCur_Sensor;
		p_UploadCur_Sensor = NULL;
	}

	if (p_UploadCur_IPCMD != NULL)
	{
		delete []p_UploadCur_IPCMD;
		p_UploadCur_IPCMD = NULL;
	}
	
	if (p_UploadCur_IPCEXT != NULL)
	{
		delete []p_UploadCur_IPCEXT;
		p_UploadCur_IPCEXT = NULL;
	}

	if (p_GUID_Sensor != NULL)
	{
		delete []p_GUID_Sensor;
		p_GUID_Sensor = NULL;
	}
	if (p_GUID_IPCMD != NULL)
	{
		delete []p_GUID_IPCMD;
		p_GUID_IPCMD = NULL;
	}
	if (p_GUID_IPCEXT != NULL)
	{
		delete []p_GUID_IPCEXT;
		p_GUID_IPCEXT = NULL;
	}

	//time_t *p_ChnTimeLast;//通道定时最后操作时间
	if (p_ChnTimeLast != NULL)
	{
		delete []p_ChnTimeLast;
		p_ChnTimeLast = NULL;
	}
	
	//time_t *p_ChnPreRec;//通道预录最后操作时间
	if (p_ChnPreRec != NULL)
	{
		delete []p_ChnPreRec;
		p_ChnPreRec = NULL;
	}

	//一维是IPC通道数目，二维是报警源数目
	//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
	if (pp_ChnRecSensor != NULL)
	{
		for (i=0; i<m_MaxIpcChn; i++)
		{
			//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
			if (pp_ChnRecSensor[i] != NULL)
			{
				delete []pp_ChnRecSensor[i];
				pp_ChnRecSensor[i]  = NULL;
			}			
		}
		
		delete []pp_ChnRecSensor;
		pp_ChnRecSensor = NULL;
	}
	
	//time_t *p_ChnRecIPCMD;//录像配置-- 事件-- IPCMD最后操作时间
	if (pp_ChnRecIPCMD != NULL)
	{
		for (i=0; i<m_MaxIpcChn; i++)
		{
			//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
			if (pp_ChnRecIPCMD[i] != NULL)
			{
				delete []pp_ChnRecIPCMD[i];
				pp_ChnRecIPCMD[i]  = NULL;
			}			
		}
		
		delete []pp_ChnRecIPCMD;
		pp_ChnRecIPCMD = NULL;
	}
	
	//time_t *p_ChnRecIPCEXT;//录像配置-- 事件-- IPCEXT最后操作时间
	if (pp_ChnRecIPCEXT != NULL)
	{
		for (i=0; i<m_MaxIpcChn; i++)
		{
			//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
			if (pp_ChnRecIPCEXT[i] != NULL)
			{
				delete []pp_ChnRecIPCEXT[i];
				pp_ChnRecIPCEXT[i]  = NULL;
			}			
		}
		
		delete []pp_ChnRecIPCEXT;
		pp_ChnRecIPCEXT = NULL;
	}
	
	//time_t *p_SGSensor;//上传中心-- 报警-- sersor最后操作时间
	if (pp_ChnSGSensor != NULL)
	{
		for (i=0; i<m_MaxIpcChn; i++)
		{
			//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
			if (pp_ChnSGSensor[i] != NULL)
			{
				delete []pp_ChnSGSensor[i];
				pp_ChnSGSensor[i]  = NULL;
			}			
		}
		
		delete []pp_ChnSGSensor;
		pp_ChnSGSensor = NULL;
	}

	//time_t *p_ChnSGIPCMD;//上传中心-- 报警-- IPCMD最后操作时间
	if (pp_ChnSGIPCMD != NULL)
	{
		for (i=0; i<m_MaxIpcChn; i++)
		{
			//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
			if (pp_ChnSGIPCMD[i] != NULL)
			{
				delete []pp_ChnSGIPCMD[i];
				pp_ChnSGIPCMD[i]  = NULL;
			}			
		}
		
		delete []pp_ChnSGIPCMD;
		pp_ChnSGIPCMD = NULL;
	}
	
	//time_t *p_ChnSGIPCEXT;//上传中心-- 报警-- IPCEXT最后操作时间
	if (pp_ChnSGIPCEXT != NULL)
	{
		for (i=0; i<m_MaxIpcChn; i++)
		{
			//time_t *p_RecSensor;//录像配置-- 事件-- sersor最后操作时间
			if (pp_ChnSGIPCEXT[i] != NULL)
			{
				delete []pp_ChnSGIPCEXT[i];
				pp_ChnSGIPCEXT[i]  = NULL;
			}			
		}
		
		delete []pp_ChnSGIPCEXT;
		pp_ChnSGIPCEXT = NULL;
	}
	
	if (p_SensorStatus != NULL)
	{
		delete []p_SensorStatus;
		p_SensorStatus = NULL;
	}

	if (p_IPCMDStatus != NULL)
	{
		delete []p_IPCMDStatus;
		p_IPCMDStatus = NULL;
	}

	if (p_IPCEXTStatus != NULL)
	{
		delete []p_IPCEXTStatus;
		p_IPCEXTStatus = NULL;
	}
	
	//SnapChnProcessDataType *pChnData	
	if (pChnData != NULL)
	{
		for (i=0; i<m_MaxIpcChn; i++)
		{			
			if (pChnData[i].p_ChnRequestSnapTimer != NULL)
			{
				delete []pChnData[i].p_ChnRequestSnapTimer;
				pChnData[i].p_ChnRequestSnapTimer = NULL;
			}

			//u8 *pSnapData;
			if (pChnData[i].pSnapData != NULL)
			{
				delete []pChnData[i].pSnapData;
				pChnData[i].pSnapData = NULL;
			}			
		}
		
		delete []pChnData;
		pChnData = NULL;		
	}

	if (p_SensorRange != NULL)
	{
		delete []p_SensorRange;
		p_SensorRange = NULL;
	}

	if (p_IPCMDRange != NULL)
	{
		delete []p_IPCMDRange;
		p_IPCMDRange = NULL;
	}

	if (p_IPCEXTRange != NULL)
	{
		delete []p_IPCEXTRange;
		p_IPCEXTRange = NULL;
	}

	DestroyCircleBuf(&SnapMsgQueue);
}

CSnapManager::~CSnapManager()
{
	DestroyThread();
	releaseResource();
}

int CSnapManager::Start(u8 MaxSensorNum, u8 MaxIpcChn)
{
	//printf("CSnapManager %s 1\n", __func__);
	m_MaxSensorNum = MaxSensorNum;
	m_MaxIpcChn = MaxIpcChn;
	m_Started = FALSE;
	memset(&SnapMsgQueue, 0, sizeof(SnapMsgQueue));
	
	//printf("CSnapManager %s 2\n", __func__);
	if (0 != requestResource())
	{
		releaseResource();
		return 1;
	}
	
	if (FALSE == CreateThread())
	{
		releaseResource();
		return 1;
	}
	
	m_Started = TRUE;
	
	return 0;
}

typedef s32 (* pfuncGetAlarmDispatch)(u32 bDefault, SBizAlarmDispatch *psPara, u8 nId);

int CSnapManager::UploadAndRecDeal(EM_ALARM_TYPE type, time_t CurTime)
{
	u32 i, j;
	SBizSGParam SGParam;	//深广平台参数
	SBizAlarmPicCFG AlarmPicCFG;//联动上传通道
	SBizSnapChnPara SnapChnParam;	//通道抓图参数
	SBizAlarmDispatch AlarmDispatch;	//报警处理--触发通道录像
	
	int Interval;
	u8 *ipcchn = NULL;

	if (ConfigGetSGPara(0, &SGParam) != 0)
	{
		printf("%s ConfigGetSGPara failed\n", __func__);
		return 1;
	}

	u8 MaxAlarmNum = 0;
	u8 offset = 0;
	
	/*
	EM_ALARM_STATUS *p_SensorStatus;	//本机报警状态
	EM_ALARM_STATUS *p_IPCMDStatus;	//IPC移动侦测状态
	EM_ALARM_STATUS *p_IPCEXTStatus;	//IPC外部报警状态
	*/
	EM_ALARM_STATUS *p_AlarmStatus = NULL;
	
	/*
	//是否要上传报警触发时刻到前置时间点的该通道所有图片
	//共有m_MaxIpcChn个变量，每一位指示报警源
	u32 * p_UploadPreRec_Sensor;//有效位数: m_MaxSensorNum
	u32 * p_UploadPreRec_IPCMD;
	u32 * p_UploadPreRec_IPCEXT;
	*/
	u32 *p_UploadPreRec_Alarm = NULL;
	/*
	//是否要上传当前时刻该通道所有图片
	//共有m_MaxIpcChn个变量，每一位指示报警源
	u32 * p_UploadCur_Sensor;//有效位数: m_MaxSensorNum
	u32 * p_UploadCur_IPCMD;
	u32 * p_UploadCur_IPCEXT;
	*/
	u32 *p_UploadCur_Alarm = NULL;
	/*
	sTimeRange *p_SensorRange;
	sTimeRange *p_IPCMDRange;
	sTimeRange *p_IPCEXTRange;
	*/
	sTimeRange *p_AlarmRange = NULL;
	
	/*
	s32 ConfigGetAlarmSensorDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId);
	s32 ConfigGetAlarmIPCExtSensorDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId);
	s32 ConfigGetAlarmVMotionDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId);
	*/
	pfuncGetAlarmDispatch p_GetAlarmDispatch = NULL;

	EM_SNAP_TYPE BitSnapType;
	/*
	time_t **pp_ChnRecSensor;//录像配置-- 事件-- sersor报警，chn最后保存时间
	time_t **pp_ChnRecIPCMD;//录像配置-- 事件-- IPCMD报警，chn最后保存时间
	time_t **pp_ChnRecIPCEXT;//录像配置-- 事件-- IPCEXT报警，chn最后保存时间
	time_t **pp_ChnSGSensor;//上传中心-- 报警-- sersor报警，chn最后上传时间
	time_t **pp_ChnSGIPCMD;//上传中心-- 报警-- IPCMD报警，chn最后上传时间
	time_t **pp_ChnSGIPCEXT;//上传中心-- 报警-- IPCEXT报警，chn最后上传时间
	*/
	time_t **pp_ChnRecAlarm = NULL;
	time_t **pp_ChnSGAlarm = NULL;
	switch (type)
	{
		case TypeSensor:	// 0
		{
			MaxAlarmNum = m_MaxSensorNum;
			offset = 0;
			p_AlarmStatus = p_SensorStatus;
			p_UploadPreRec_Alarm = p_UploadPreRec_Sensor;
			p_UploadCur_Alarm = p_UploadCur_Sensor;
			p_AlarmRange = p_SensorRange;
			p_GetAlarmDispatch = ConfigGetAlarmSensorDispatch;
			BitSnapType = TypeAlarmSensor;
			pp_ChnRecAlarm = pp_ChnRecSensor;
			pp_ChnSGAlarm = pp_ChnSGSensor;
		} break;
		case TypeMD:		// 1
		{
			MaxAlarmNum = m_MaxIpcChn;
			offset = m_MaxSensorNum;
			p_AlarmStatus = p_IPCMDStatus;
			p_UploadPreRec_Alarm = p_UploadPreRec_IPCMD;
			p_UploadCur_Alarm = p_UploadCur_IPCMD;
			p_AlarmRange = p_IPCMDRange;
			p_GetAlarmDispatch = ConfigGetAlarmVMotionDispatch;
			BitSnapType = TypeMotionDet;
			pp_ChnRecAlarm = pp_ChnRecIPCMD;
			pp_ChnSGAlarm = pp_ChnSGIPCMD;
		} break;
		case TypeIPCEXT:	// 2
		{
			MaxAlarmNum = m_MaxIpcChn;
			offset = m_MaxSensorNum;
			p_AlarmStatus = p_IPCEXTStatus;
			p_UploadPreRec_Alarm = p_UploadPreRec_IPCEXT;
			p_UploadCur_Alarm = p_UploadCur_IPCEXT;
			p_AlarmRange = p_IPCEXTRange;
			p_GetAlarmDispatch = ConfigGetAlarmIPCExtSensorDispatch;
			BitSnapType = TypeAlarmIPCEXT;
			pp_ChnRecAlarm = pp_ChnRecIPCEXT;
			pp_ChnSGAlarm = pp_ChnSGIPCEXT;
		} break;
		default:
		{
			printf("%s EM_ALARM_TYPE, type: %d invalid\n", __func__, type);
			return 1;
		}
	}

	//检测预录上传
	//深广运行状态启用且报警联动启用
	if (SGParam.RunEnable && SGParam.AlarmEnable)	
	{
		for (i=0; i<MaxAlarmNum; i++)
		{
		//报警源联动通道
			if (ConfigGetSGAlarmPara(0, &AlarmPicCFG, i + offset) == 0)
			{
				if (AlarmPicCFG.IpcChn)
				{					
					for (j=0; j<m_MaxIpcChn; j++)
					{
						pChnMutex[j].Enter();
						
						if (AlarmPicCFG.IpcChn & (1<<j))//与报警源联动的IPC
						{	
							//对联动的IPC通道加预录标记，以便取得图片后保存
							//该通道可能是多个报警的联动，所以要记录最近预录时间
							//当前时间必须比最近的预录时间大1S
							//printf("%s alarmchn%d, IPCchn%d, curtime: %d, p_ChnPreRec: %d\n", 
								//	__func__, i, j, CurTime, p_ChnPreRec[j]);
								
							if ((CurTime > p_ChnPreRec[j]) && (CurTime -  p_ChnPreRec[j]  >= 1))
							{
								p_ChnPreRec[j] = CurTime;

								p_ChnReqTypeMask[j] |= 1<<TypePreSnap;
							}
							
							//是否要上传报警触发时刻到前置时间点的所有图片
							if ( p_AlarmStatus[i] == EM_ALARM_TIGGER )
							{
								//由传感器i 触发
								//上传当前报警联动j 通道的预录图片
								struct tm now;
								localtime_r(&CurTime,&now);
								printf("%s Alarmchn%d EM_ALARM_TIGGER time: %d - %2d:%2d:%2d\n", \
									__func__, i, CurTime, now.tm_hour+8, now.tm_min, now.tm_sec);
		
								p_UploadPreRec_Alarm[j] = 1<<i;
							}
							
							//是否上传报警联动的通道图片
							if ( p_AlarmStatus[i] != EM_ALARM_NONE )
							{
								Interval = AlarmPicCFG.Interval;
								if ((CurTime > pp_ChnSGAlarm[j][i]) && (CurTime - pp_ChnSGAlarm[j][i]  >= Interval))
								{
									pp_ChnSGAlarm[j][i] = CurTime;

									//由传感器i 触发
									//上传当前报警联动j 通道的图片
									p_UploadCur_Alarm[j] = 1<<i;
								}
							}							
						}

						pChnMutex[j].Leave();
					}
				}
			}
		}
	}

	//检测报警，是否保存图片
	int processflag = 0;
	for (i=0; i<MaxAlarmNum; i++)
	{
	//本机报警-触发录像中是否有通道开启联动
		//状态必须是报警触发、报警中、报警解除
		processflag = 0;
		switch (p_AlarmStatus[i])
		{
			case EM_ALARM_NONE:	//=0,	// 0 无报警，初始状态和整个报警活动结束状态
			{
				processflag = 0;
			}break;
			case EM_ALARM_TIGGER:		// 1 报警触发
			{
				p_AlarmStatus[i] = EM_ALARM_ING;
				p_AlarmRange[i].StartTime = CurTime;
				p_AlarmRange[i].EndTime = 0;
				
				processflag = 1;
			}break;
			case EM_ALARM_ING:		// 2 报警中
			{
				processflag = 1;
			}break;
			case EM_ALARM_END:		// 3 报警解除
			{
				p_AlarmStatus[i] = EM_ALARM_STILL;
				p_AlarmRange[i].EndTime = CurTime;
				
				struct tm now;
				localtime_r(&CurTime,&now);
				printf("%s Alarmchn%d EM_ALARM_END time: %d - %02d:%02d:%02d\n", \
					__func__, i, CurTime, now.tm_hour+8, now.tm_min, now.tm_sec);
				
				processflag = 1;
			}break;
			case EM_ALARM_STILL:	// 4	报警延迟延录上传
			{
				ConfigGetSGAlarmPara(0, &AlarmPicCFG, i + offset);
				
				if (CurTime >= p_AlarmRange[i].EndTime+ AlarmPicCFG.StillTimes)
				{
					p_AlarmStatus[i] = EM_ALARM_NONE;
					p_AlarmRange[i].StartTime = 0;
					p_AlarmRange[i].EndTime = 0;

					struct tm now;
					localtime_r(&CurTime,&now);
					printf("%s Alarmchn%d EM_ALARM_STILL time: %d - %2d:%2d:%2d\n", \
						__func__, i, CurTime, now.tm_hour+8, now.tm_min, now.tm_sec);
				}

				processflag = 0;
			}break;
			default:
			{
				printf("%s EM_ALARM_STATUS, type: %d invalid\n", __func__, p_AlarmStatus[i]);
				return 1;
			}
		}

		//报警触发和解除期间才处理
		if (processflag)//是否记录保存图片
		{
			if ((*p_GetAlarmDispatch)(0, &AlarmDispatch, i) == 0)
			{
				ipcchn = AlarmDispatch.nRecordChn;
				//printf("%s alarm ipcchn: 0x%x\n", __func__, ipcchn);
				
				for (j=0; j<m_MaxIpcChn; j++)
				{
					//是否触发了该录像通道
					if (ipcchn[j] != 0xff)
					{	//该通道是否使能了事件抓图
						if ((ConfigGetSnapChnPara(0, &SnapChnParam, j) == 0) 
							&& (SnapChnParam.EventSnap.Enable))
						{							
							Interval = indextosec[SnapChnParam.EventSnap.Interval];
							//刚触发或是到达间隔点
							if ((CurTime > pp_ChnRecAlarm[j][i]) && (CurTime - pp_ChnRecAlarm[j][i]  >= Interval))
							{
								pp_ChnRecAlarm[j][i] = CurTime;

								pChnMutex[j].Enter();
								p_ChnReqTypeMask[j] |= 1<<BitSnapType;
								pChnMutex[j].Leave();
							}							
						}
					}
				}
			}
		}
	}

	return 0;
}

void CSnapManager::ThreadProc()
{
	u32 i, j, curSecs, speSecs;//当前时间换算成从零点开始的秒数
	SBizSGParam SGParam;	//深广平台参数
	SBizDailyPicCFG para1;
	SBizSnapChnPara SnapChnParam;	//抓图参数
	EM_SNAPMSG_TYPE msg;
	
	time_t CurTime;
	int Interval, ret, running;
	//u32 ReqChnMask;
	
	printf("CSnapManager::ThreadProc: m_MaxSensorNum: %d, m_MaxIpcChn: %d\n", m_MaxSensorNum, m_MaxIpcChn);
	
	running = 1;
	
	while (1)
	{
		//usleep(1000*1000);// 200ms扫一次
		sleep(1);
		
		ret = SnapReadMsgQueue(&msg);
		if (0 == ret)
		{
			printf("%s get msg type: %d, cur statue: %d\n", __func__, msg, running);
			switch(msg)
			{
				case TypePause:
				{
					running = 0;
				} break;
				case TypeResume:
				{
					running = 1;
				} break;
				default:
					printf("%s invalid msg type: %d\n", __func__, msg);
			}
		}

		if (0 == running)//格式化 升级热插拔时暂停
		{
			continue;
		}

		//yaogang modify 20150724 //非预览状态下
		if(!ModPreviewIsOpened())
		{
			//printf("ModPreviewIsOpened == 0\n");
			continue;
		}
		
		if (ConfigGetSGPara(0, &SGParam) != 0)
		{
			printf("%s ConfigGetSGPara failed\n", __func__);
			continue;
		}
		
		time(&CurTime);
		
		//1、报警源对IPC抓图通道掩码的影响
		for (i=0; i<TypeAlarmMax; i++)
		{
			UploadAndRecDeal((EM_ALARM_TYPE)i, CurTime);
		}
	
		//2、日常图像是否启用(每天两个时间点上传深广)
		SBizDateTime DateTime;
		BizSysComplexDTGet_TZ(&DateTime);
		static int flagtime1 = 0;
		static int flagtime2 = 0;
		//printf("RunEnable: %d, DailyPicEnable: %d\n", SGParam.RunEnable, SGParam.DailyPicEnable);
		if (SGParam.RunEnable && SGParam.DailyPicEnable)	//运行状态启用且日常图像启用
		{
			if (ConfigGetSGDailyPara(0, &para1) == 0)
			{
				speSecs = nSECPERHOUR*para1.Time1.hour + nSECPERMIN*para1.Time1.min + para1.Time1.sec;
				curSecs = nSECPERHOUR*DateTime.nHour + nSECPERMIN*DateTime.nMinute + DateTime.nSecond;

				//printf("IpcChn1: 0x%x, Time1Enable: %d\n", para1.IpcChn1, para1.Time1.TimeEnable);
				//printf("IpcChn2: 0x%x, Time2Enable: %d\n", para1.IpcChn2, para1.Time2.TimeEnable);				
				/*
				printf("hour: %d, min: %d, sec: %d\n", 
					DateTime.nHour, 
					DateTime.nMinute, 
					DateTime.nSecond);
				printf("speSecs: %d, curSecs: %d\n", speSecs, curSecs);
				*/
				/*
				printf("hour: %d, min: %d, sec: %d\n", 
					para1.Time2.hour, 
					para1.Time2.min, 
					para1.Time2.sec);
				*/
				if (para1.Time1.TimeEnable && (curSecs >= speSecs) && (curSecs - speSecs < 10))
				{			
					if (flagtime1 == 0)
					{
						flagtime1 = 1;
						//printf("curSecs >= para1.Time1\n");
						//printf("para1.IpcChn1: 0x%x, m_MaxIpcChn: %d\n", para1.IpcChn1, m_MaxIpcChn);
						for (j=0; j<m_MaxIpcChn; j++)
						{
							if (para1.IpcChn1 & (1<<j))//联动
							{
								pChnMutex[j].Enter();
								p_ChnReqTypeMask[j] |= 1<<TypeDaily;
								pChnMutex[j].Leave();
							}
						}
					}					
				}
				else
				{
					flagtime1 = 0;
				}
				
				speSecs = nSECPERHOUR*para1.Time2.hour + nSECPERMIN*para1.Time2.min + para1.Time2.sec;				
				if (para1.Time2.TimeEnable && (curSecs >= speSecs) && (curSecs - speSecs < 10))
				{		
					//printf("curSecs >= para1.Time2\n");
					if (flagtime2 == 0)
					{
						flagtime2 = 1;
						//printf("curSecs >= para1.Time2\n");
						//printf("para1.IpcChn2: 0x%x, m_MaxIpcChn: %d\n", para1.IpcChn2, m_MaxIpcChn);
						for (j=0; j<m_MaxIpcChn; j++)
						{
							if (para1.IpcChn2 & (1<<j))//联动
							{
								pChnMutex[j].Enter();
								p_ChnReqTypeMask[j] |= 1<<TypeDaily;
								pChnMutex[j].Leave();
							}
						}
					}					
				}
				else
				{
					flagtime2 = 0;
				}
			}
		}
		
		for (i=0; i<m_MaxIpcChn; i++) 
		{				
			//3、定时抓图是否启用
			//		录像配置--> 抓图参数
			//	掩码在此设置，收到图片处理后清除
			//if ((ConfigGetSnapChnPara(0, &SnapChnParam, i) == 0)
				//&& (SnapChnParam.TimeSnap.Enable))
			if (ConfigGetSnapChnPara(0, &SnapChnParam, i) == 0)
			{	/*
				//debug
				if (i == 0)
				{
					printf("TimeSnap: enable: %d, Res: %d, Quality: %d, Interval: %d\n", 
						SnapChnParam.TimeSnap.Enable, SnapChnParam.TimeSnap.Res,
						SnapChnParam.TimeSnap.Quality, SnapChnParam.TimeSnap.Interval);

					printf("EventSnap: enable: %d, Res: %d, Quality: %d, Interval: %d\n", 
						SnapChnParam.EventSnap.Enable, SnapChnParam.EventSnap.Res,
						SnapChnParam.EventSnap.Quality, SnapChnParam.EventSnap.Interval);

					printf("CurTime: %u, p_ChnTimeLast[0]: %u, Interval: %u\n", CurTime, p_ChnTimeLast[0], indextosec[SnapChnParam.TimeSnap.Interval]);
				}
				*/
				if (SnapChnParam.TimeSnap.Enable)
				{
					Interval = indextosec[SnapChnParam.TimeSnap.Interval];
					
					if ((CurTime > p_ChnTimeLast[i]) && (CurTime - p_ChnTimeLast[i]  >= Interval))
					{
						pChnMutex[i].Enter();
						p_ChnReqTypeMask[i] |= 1<<TypeTime;
						pChnMutex[i].Leave();
						p_ChnTimeLast[i] = CurTime;
					}
				}
			}

			/*
			if (i==0 && p_ChnReqTypeMask[i])
			{
				printf("%s chn%d p_ChnReqTypeMask: 0x%x\n", __func__, i, p_ChnReqTypeMask[i]);
			}
			*/
			
			//printf("%s chn%d, 1\n", __func__, i);
			
			//如果通道没有接IPC，那么清除通道标记
			if (IPC_GetLinkStatus(i) == 0)
			{
				pChnMutex[i].Enter();
				
				p_ChnReqTypeMask[i] = 0;				
				p_UploadPreRec_Sensor[i] = 0;
				p_UploadPreRec_IPCMD[i] = 0;
				p_UploadPreRec_IPCEXT[i] = 0;
				p_UploadCur_Sensor[i] = 0;
				p_UploadCur_IPCMD[i] = 0;
				p_UploadCur_IPCEXT[i] = 0;

				pChnMutex[i].Leave();
			}

			//printf("%s chn%d, 2\n", __func__, i);
			
			if (p_ChnReqTypeMask[i])
			{
				//printf("%s chn%d, 2\n", __func__, i);
				//if (pp_ChnRequestSnapTimer[i]->IsStarted() == FALSE)
				
				
				
				if (pChnData[i].TimerWorking) //该通道定时器线程正在处理之前的任务，忽略此次任务
				{
					printf("CSnapManager::ThreadProc chn%d is working, ignore once snap\n", i);
				}
				else
				{
					
					pChnMutex[i].Enter();
					
					pChnData[i].TimerWorking = TRUE;
					pChnData[i].ChnReqTypeMask = p_ChnReqTypeMask[i];				
					pChnData[i].UploadPreRec_Sensor = p_UploadPreRec_Sensor[i];
					pChnData[i].UploadPreRec_IPCMD = p_UploadPreRec_IPCMD[i];
					pChnData[i].UploadPreRec_IPCEXT = p_UploadPreRec_IPCEXT[i];
					pChnData[i].UploadCur_Sensor = p_UploadCur_Sensor[i];
					pChnData[i].UploadCur_IPCMD = p_UploadCur_IPCMD[i];
					pChnData[i].UploadCur_IPCEXT = p_UploadCur_IPCEXT[i];

					//清除标记，只有在TimerWorking == FALSE才清除
					p_ChnReqTypeMask[i] = 0;				
					p_UploadPreRec_Sensor[i] = 0;
					p_UploadPreRec_IPCMD[i] = 0;
					p_UploadPreRec_IPCEXT[i] = 0;
					p_UploadCur_Sensor[i] = 0;
					p_UploadCur_IPCMD[i] = 0;
					p_UploadCur_IPCEXT[i] = 0;

					pChnMutex[i].Leave();
					//printf("%s timer start %d\n", __func__, i);
					pChnData[i].p_ChnRequestSnapTimer->Start(this, (VD_TIMERPROC)&CSnapManager::Snapshot_Register, 0, 0, i);
					
				}
				
			}
		}
		//printf("%s chn%d, 3\n", __func__, i);
		
	}
	
}

//上传报警触发时刻到前置时间点内多张预录图片
/*
TypeAlarmSensor,	// 7 本地传感器报警
TypeMotionDet,		// 8 移动侦测
TypeAlarmIPCEXT,	// 9 IPC外部报警
*/
int CSnapManager::UploadPreRecPic(EM_SNAP_TYPE type, u8 AlarmChn, u8 IPCChn)
{
	time_t TriggerTime, FirstSnapTime, UploadTime;
	u8 PreTimes;		//前置时间
	u8 Interval;		//间隔时间
	SBizAlarmPicCFG AlarmPicCFG;//联动上传通道
	SModRecRecordHeader head;
	u8 *pSnapData = NULL;
	struct timeval tv;
	int ret;
	int GUID = 0;//一组报警消息的所有GUID编码相同

	pSnapData = (u8 *)malloc(SNAPBUFSIZE);
	if (pSnapData == NULL)
	{
		printf("%s malloc failed\n", __func__);
		return 1;
	}
	
	if (IPCChn >= m_MaxIpcChn)
	{
		printf("%s IPCChn: %d invalid\n", __func__, IPCChn);
		return 1;
	}

	switch (type)
	{
		case TypeAlarmSensor:
		{			
			if (AlarmChn >= m_MaxSensorNum)
			{
				printf("%s TypeAlarmSensor AlarmChn: %d invalid\n", __func__, AlarmChn);
				return 1;
			}

			GUID = p_GUID_Sensor[AlarmChn];
			TriggerTime = p_SensorRange[AlarmChn].StartTime;
			
			if ( ConfigGetSGAlarmPara(0, &AlarmPicCFG, AlarmChn) )
			{
				printf("%s TypeAlarmSensor AlarmChn%d ConfigGetSGAlarmPara() failed\n", __func__, AlarmChn);
				return 1;
			}
		} break;
		case TypeMotionDet:
		{			
			if (AlarmChn >= m_MaxIpcChn)
			{
				printf("%s TypeMotionDet AlarmChn: %d invalid\n", __func__, AlarmChn);
				return 1;
			}

			GUID = p_GUID_IPCMD[AlarmChn];
			TriggerTime = p_IPCMDRange[AlarmChn].StartTime;
			
			if ( ConfigGetSGAlarmPara(0, &AlarmPicCFG, AlarmChn + m_MaxSensorNum) )
			{
				printf("%s TypeMotionDet AlarmChn%d ConfigGetSGAlarmPara() failed\n", __func__, AlarmChn);
				return 1;
			}
		} break;
		case TypeAlarmIPCEXT:
		{			
			if (AlarmChn >= m_MaxIpcChn)
			{
				printf("%s TypeAlarmIPCEXT AlarmChn: %d invalid\n", __func__, AlarmChn);
				return 1;
			}

			GUID = p_GUID_IPCEXT[AlarmChn];
			TriggerTime = p_IPCEXTRange[AlarmChn].StartTime;
			
			if ( ConfigGetSGAlarmPara(0, &AlarmPicCFG, AlarmChn + m_MaxSensorNum) )
			{
				printf("%s TypeAlarmIPCEXT AlarmChn%d ConfigGetSGAlarmPara() failed\n", __func__, AlarmChn);
				return 1;
			}
		} break;
		default:
		{
			printf("%s snap type: %d invalid\n", __func__, type);
			return 1;
		}
	}

	PreTimes = AlarmPicCFG.PreTimes;
	Interval = AlarmPicCFG.Interval;
	FirstSnapTime = TriggerTime - PreTimes;

	
	//上传前置时间点到报警触发时刻内的多张预录图片
	for (UploadTime = FirstSnapTime; UploadTime < TriggerTime; UploadTime += Interval)
	{
		printf("%s FirstSnapTime: %d, Interval: %d, UploadTime: %d\n", \
			__func__, FirstSnapTime, Interval, UploadTime);
		memset(&head, 0, sizeof(head));
		
		head.nChn = IPCChn;
		head.nStreamType = 2;//Snap
		head.nBitRate = 1<<TypePreSnap;
		head.nDate = pSnapData;
		head.nDataLength = SNAPBUFSIZE;
		tv.tv_sec = UploadTime;
		tv.tv_usec = 0;
		memcpy(&head.nPts, &tv, sizeof(tv));
		
		printf("%s read PreRec time: %d\n", __func__, UploadTime);
		ret = RecordReadOnePreSnap(IPCChn, &head);
		if (ret)
		{
			printf("%s chn%d RecordReadOneSnap failed, ret: %d\n", __func__, IPCChn, ret);
			continue;
		}

		ret = UploadPic(type, AlarmChn, IPCChn, tv.tv_sec, (char *)head.nDate, head.nDataLength, GUID);
		if (ret)
		{
			printf("%s type: %d, AlarmChn: %d, chn%d UploadPic failed, ret: %d\n", \
				__func__, type, AlarmChn, IPCChn, ret);
			continue;
		}
	
	}

/*触发时刻的图片在Snapshot_Register() 上传
	//上传报警触发时刻的图像
	memset(&head, 0, sizeof(head));
	head.nChn = IPCChn;
	head.nStreamType = 2;//Snap
	head.nBitRate = 1<<TypePreSnap;
	head.nDate = pSnapData;
	head.nDataLength = SNAPBUFSIZE;
	
	tv.tv_sec = TriggerTime;
	tv.tv_usec = 0;
	memcpy(&head.nPts, &tv, sizeof(tv));

	ret = RecordReadOnePreSnap(IPCChn, &head);
	if (ret)
	{
		printf("%s chn%d RecordReadOneSnap failed, ret: %d\n", __func__, IPCChn, ret);
		return 1;
	}

	ret = UploadPic(type, AlarmChn, IPCChn, tv.tv_sec, head.nDate, head.nDataLength, GUID);
	if (ret)
	{
		printf("%s type: %d, AlarmChn: %d, chn%d UploadPic failed, ret: %d\n", \
			__func__, type, AlarmChn, IPCChn, ret);
		return 1;
	}
*/
	return 0;
}

//时间必须作为参数，可以指定，因为要上传之前的预录图片
//一组报警消息的所有GUID编码相同。
int CSnapManager::UploadPic(EM_SNAP_TYPE type, u8 AlarmChn, u8 IPCChn, time_t time, void *SnapData, u32 DataSize, int GUID)
{
	SSG_MSG_TYPE msg;
	
	memset(&msg, 0, sizeof(SSG_MSG_TYPE));
	AlarmChn += 1;//0-15-->1-16
	msg.chn = IPCChn+1;

	switch (type)
	{
		case TypeManualTEST:	//测试
		{
			msg.type = EM_PIC_TEST_UPLOAD;
			strcpy(msg.note, GetParsedString("&CfgPtn.TestImageUpload"));
							
		} break;
		case TypeManualMAINTAIN:	//保养
		{
			msg.type = EM_PIC_MAINTAIN_UPLOAD;
			strcpy(msg.note, GetParsedString("&CfgPtn.MaintenanceImageUpload"));
		} break;
		case TypeManualACCEPT:		//验收
		{
			msg.type = EM_PIC_ACCEPT_UPLOAD;
			sprintf(msg.note, "%s: %s %s: %s",
							GetParsedString("&CfgPtn.PoliceID"), PoliceID, \
							GetParsedString("&CfgPtn.KeyWord"), PassWord);
		} break;
		case TypeDaily:	//日常
		{
			//if (IPCChn == 0)
				//printf("%s: chn%d TypeDaily upload\n", __func__, IPCChn);
			
			msg.type = EM_PIC_DAILY_UPLOAD;
			strcpy(msg.note, GetParsedString("&CfgPtn.DayImageUpload"));
		} break;
		case TypeSvrReq:	//监管平台请求上传
		{
			msg.type = EM_PIC_SERVER_GET;
			//strcpy(msg.note, GetParsedString("&CfgPtn.DayImageUpload"));
		} break;
		/*
		本机N报警	通道M联动图像上传
		IPC N报警
		IPC N移动侦测
		*/
	#if 0
		case TypeAlarmSensor:
		case TypeMotionDet:
		case TypeAlarmIPCEXT:
		{
			msg.type = EM_PIC_ALARM_LINK_UPLOAD;
			sprintf(msg.note, "%s[%s%02d]", \
				GetParsedString("&CfgPtn.AlarmLinkPic"),
				GetParsedString("&CfgPtn.Channel"), 
				msg.chn);
			
		} break;
	#else
		case TypeAlarmSensor:
		{
			msg.type = EM_PIC_ALARM_LINK_UPLOAD;
			sprintf(msg.note, "%s%02d%s, %s%02d%s", \
				GetParsedString("&CfgPtn.Local"),
				AlarmChn,
				GetParsedString("&CfgPtn.Alarm1"),
				GetParsedString("&CfgPtn.Channel"), 
				msg.chn,
				GetParsedString("&CfgPtn.LinkPicUpload")
				);
			
		} break;
		case TypeMotionDet:
		{
			msg.type = EM_PIC_ALARM_LINK_UPLOAD;
			sprintf(msg.note, "IPC%02d%s, %s%02d%s", \
				AlarmChn,
				GetParsedString("&CfgPtn.Motion"),
				GetParsedString("&CfgPtn.Channel"), 
				msg.chn,
				GetParsedString("&CfgPtn.LinkPicUpload")
				);
		} break;
		case TypeAlarmIPCEXT:
		{
			msg.type = EM_PIC_ALARM_LINK_UPLOAD;
			sprintf(msg.note, "IPC%02d%s, %s%02d%s", \
				AlarmChn,
				GetParsedString("&CfgPtn.Alarm1"),
				GetParsedString("&CfgPtn.Channel"), 
				msg.chn,
				GetParsedString("&CfgPtn.LinkPicUpload")
				);
		} break;
	#endif
		default:
		{
			printf("%s type %d invalid\n", __func__, type);
			return 1;
		} 
	}

	return upload_sg_proc(&msg, time, SnapData, DataSize, GUID);	
}

void CSnapManager::Snapshot_Register(u8 chn)
{
	int i, cnt, ret, TimeResIdx, EventResIdx;//分辨率索引
	u32 SnapSize;
	SBizSnapChnPara SnapChnParam;	//抓图参数
	SModRecRecordHeader head;
	u16 width, height;
	u8 *pSnapData = pChnData[chn].pSnapData;
	struct timeval tv;
	struct tm curtm;

	#if 0//测试硬盘，暂时关闭预录20150527
	pChnData[chn].TimerWorking = FALSE;
		
	return ;
	#endif

	gettimeofday(&tv, NULL);
	gmtime_r(&tv.tv_sec, &curtm);
	//printf("%s chn%d, time: %d:%d:%d, size: %d, width: %d, height: %d\n", 
		//__func__, chn, curtm.tm_hour+8, curtm.tm_min, curtm.tm_sec, DataSize, width, height);

	//dbg
	//if (chn == 0)
		//printf("chn%d ChnReqTypeMask: 0x%x\n", chn, pChnData[chn].ChnReqTypeMask);
	


#if 1
	//申请抓图参数，分辨率和图片质量(质量还没有加进去)
	if (ConfigGetSnapChnPara(0, &SnapChnParam, chn) == 0)
	{
		TimeResIdx = SnapChnParam.TimeSnap.Res;//定时抓图分辨率
		EventResIdx = SnapChnParam.EventSnap.Res;//事件抓图分辨率
	}
	else
	{
		printf("%s ConfigGetSnapChnPara failed\n", __func__);
		pChnData[chn].TimerWorking = FALSE;
		
		return ;
	}

	//定时和事件图片分辨率不同时，单独先处理
	if ( (TimeResIdx != EventResIdx) &&
		((pChnData[chn].ChnReqTypeMask & (1<<TypeTime)) || (pChnData[chn].ChnReqTypeMask & (1<<TypeDaily))) )
	{

		SnapSize = SNAPBUFSIZE;
		width = SnapRes[TimeResIdx].width;
		height = SnapRes[TimeResIdx].height;
		
		//获取定时触发时的抓图
		ret = ModEncGetSnapshot(chn, &width, &height, pSnapData, &SnapSize);
		if (ret)
		{
			if (SnapSize == 0)
			{
				printf("%s chn%d ModEncGetSnapshot 1 space too small\n", __func__, chn);					
			}
			printf("%s chn%d ModEncGetSnapshot 1 failed\n", __func__, chn);
			pChnData[chn].TimerWorking = FALSE;
			
			return ;
		}

		// 1、定时抓图保存
		if (pChnData[chn].ChnReqTypeMask & (1<<TypeTime))
		{
			//处理后去掉定时属性
			pChnData[chn].ChnReqTypeMask &= ~(1<<TypeTime);
			
			memset(&head, 0, sizeof(head));
			head.nChn = chn;
			head.nStreamType = 2;//Snap
			//head.nBitRate = ChnReqTypeMask & (0x3f<<TypePreSnap);// 6位
			head.nBitRate =  (1<<TypeTime);// 6位
			memcpy(&head.nPts, &tv, sizeof(tv));
			head.nDate = (u8*)pSnapData;
			head.nDataLength = SnapSize;
			head.width = width;
			head.height = height;

			//printf("%s RecordWriteOneSnap chn%d\n", __func__, chn);
			ret = RecordWriteOneSnap(chn, &head);
			if (ret)
			{
				printf("%s chn%d RecordWriteOneSnap 1 failed, ret: %d\n", __func__, chn, ret);
				pChnData[chn].TimerWorking = FALSE;
				return ;
			}
		}

		//2、日常图片上传(定时和事件抓图参数不相同时)
		if (pChnData[chn].ChnReqTypeMask & (1<<TypeDaily))
		{
			//if (chn == 0)
			//	printf("%s: chn%d TypeDaily upload 1\n", __func__, chn);
			pChnData[chn].ChnReqTypeMask &= ~(1<<TypeDaily);
			g_SnapManager.UploadPic(TypeDaily, 0, chn, tv.tv_sec, pSnapData, SnapSize, 0);
		}
	}

	//获取事件触发时的抓图
	SnapSize = SNAPBUFSIZE;
	width = SnapRes[EventResIdx].width;
	height = SnapRes[EventResIdx].height;

	//printf("%s ModEncGetSnapshot chn%d, width: %d, height: %d\n", __func__, chn, width, height);
	ret = ModEncGetSnapshot(chn, &width, &height, pSnapData, &SnapSize);
	if (ret)
	{
		if (SnapSize == 0)
		{
			printf("%s chn%d ModEncGetSnapshot 2 space too small\n", __func__, chn);
		}
		printf("%s chn%d ModEncGetSnapshot 2 failed\n", __func__, chn);
		pChnData[chn].TimerWorking = FALSE;
		
		return ;
	}
	
//一、监管上传报警图片
	for (i=0; i<g_SnapManager.m_MaxSensorNum; i++)
	{
		//1. 上传报警触发时刻到前置时间点内多张预录图片
		if (pChnData[chn].UploadPreRec_Sensor & (1<<i))
		{
			printf("%s Alarmchn%d, ipcchn%d, UploadPreRec_Sensor\n", __func__, i, chn);
			g_SnapManager.UploadPreRecPic(TypeAlarmSensor, i, chn);
		}
		//2. 监管上传一张报警图片，处于报警触发到延录之间
		if (pChnData[chn].UploadCur_Sensor & (1<<i))
		{
			g_SnapManager.UploadPic(TypeAlarmSensor, i, chn, tv.tv_sec, \
				pSnapData, SnapSize, p_GUID_Sensor[i]);
		}
	}
	
	for (i=0; i<g_SnapManager.m_MaxIpcChn; i++)
	{
		if (pChnData[chn].UploadPreRec_IPCMD & (1<<i))
		{
			g_SnapManager.UploadPreRecPic(TypeMotionDet, i, chn);
		}
		
		if (pChnData[chn].UploadCur_IPCMD & (1<<i))
		{
			g_SnapManager.UploadPic(TypeMotionDet, i, chn, tv.tv_sec, \
				pSnapData, SnapSize, p_GUID_IPCMD[i]);
		}

		
		if (pChnData[chn].UploadPreRec_IPCEXT & (1<<i))
		{
			g_SnapManager.UploadPreRecPic(TypeAlarmIPCEXT, i, chn);
		}
		
		if (pChnData[chn].UploadCur_IPCEXT & (1<<i))
		{
			g_SnapManager.UploadPic(TypeAlarmIPCEXT, i, chn, tv.tv_sec, \
				pSnapData, SnapSize, p_GUID_IPCEXT[i]);
		}
	}

	//3.测试图像上传
	cnt = ret = 0;
	if (pChnData[chn].ChnReqTypeMask & (1<<TypeManualTEST))
	{
		do {
			printf("%s TypeManualTEST UploadPic cnt: %d\n", __func__, cnt);
			cnt++;
			ret = g_SnapManager.UploadPic(TypeManualTEST, 0, chn, tv.tv_sec, \
						pSnapData, SnapSize, 0);
		} while ((ret !=0) && (cnt < 2));
	}
	
	//4.保养图像上传
	cnt = ret = 0;
	if (pChnData[chn].ChnReqTypeMask & (1<<TypeManualMAINTAIN))
	{
		do {
			printf("%s TypeManualMAINTAIN UploadPic cnt: %d\n", __func__, cnt);
			cnt++;
			ret = g_SnapManager.UploadPic(TypeManualMAINTAIN, 0, chn, tv.tv_sec, \
						pSnapData, SnapSize, 0);
		} while ((ret !=0) && (cnt < 2));
	}

	//5.验收管理上传
	cnt = ret = 0;
	if (pChnData[chn].ChnReqTypeMask & (1<<TypeManualACCEPT))
	{
		do {
			printf("%s TypeManualACCEPT UploadPic cnt: %d\n", __func__, cnt);
			cnt++;
			ret = g_SnapManager.UploadPic(TypeManualACCEPT, 0, chn, tv.tv_sec, \
						pSnapData, SnapSize, 0);
		} while ((ret !=0) && (cnt < 2));
	}

	//6.监管平台请求上传
	cnt = ret = 0;
	if (pChnData[chn].ChnReqTypeMask & (1<<TypeSvrReq))
	{
		do {
			printf("%s TypeSvrReq UploadPic cnt: %d\n", __func__, cnt);
			cnt++;
			ret = g_SnapManager.UploadPic(TypeSvrReq, 0, chn, tv.tv_sec, \
						pSnapData, SnapSize, 0);
		} while ((ret !=0) && (cnt < 2));
		
	}

	//7、日常图片上传(定时和事件抓图参数相同时)
	if (pChnData[chn].ChnReqTypeMask & (1<<TypeDaily))
	{
		//if (chn == 0)
			//printf("%s: chn%d TypeDaily upload 2\n", __func__, chn);
		pChnData[chn].ChnReqTypeMask &= ~(1<<TypeDaily);
		g_SnapManager.UploadPic(TypeDaily, 0, chn, tv.tv_sec, pSnapData, SnapSize, 0);
	}
	
//二、保存图片
	// 1.预录	
	// 2.定时 
	// 3.手动截图	
	// 4.报警保存	
	/*
	TypePreSnap,		// 4 预录
	TypeTime,			// 5 定时-录像配置--抓图参数
	TypeManual,		// 6 手动截图
	//upload & record
	TypeAlarmSensor,	// 7 本地传感器报警
	TypeMotionDet,		// 8 移动侦测
	TypeAlarmIPCEXT,	// 9 IPC外部报警
	*/
//	int ret;
#if 1
	memset(&head, 0, sizeof(head));
	head.nChn = chn;
	head.nStreamType = 2;//Snap
	head.nBitRate = pChnData[chn].ChnReqTypeMask & (0x3f<<TypePreSnap);//掩码中高 6位的类型才需要保存
	memcpy(&head.nPts, &tv, sizeof(tv));
	head.nDate = pSnapData;
	head.nDataLength = SnapSize;
	head.width = width;
	head.height = height;

	//printf("%s write snap, chn%d, type: 0x%x, size: %d, time: %d\n", 
	//		__func__, chn, head.nBitRate, SnapSize, tv.tv_sec);
	
	if (head.nBitRate)
	{
		ret = RecordWriteOneSnap(chn, &head);
		if (ret)
		{
			printf("%s chn%d RecordWriteOneSnap 2 failed, ret: %d\n", __func__, chn, ret);
			//return 1;
		}
	}
#endif
	pChnData[chn].TimerWorking = FALSE;
	
#else
	//sleep(10);
#endif
	return ;
}

//非报警类型请求抓图
int CSnapManager::requestSnap(u8 chn, EM_SNAP_TYPE type, const char *PoliceID, const char *pswd)
{
	if (!m_Started)
	{
		return 0;
	}

	//printf("%s yg 2 chn%d type: %d\n", __func__, chn, type);
	
	if (type >= TypeSnapMax)
	{
		printf("%s chn%d type: %d invalid\n", __func__, chn, type);
		return 1;
	}
	
	if (chn < 0 || chn > m_MaxIpcChn)
	{
		printf("%s chn%d invalid\n", __func__, chn);
		return 1;
	}
	
	CGuard guard(pChnMutex[chn]);	

	if (TypeManualACCEPT == type)
	{
		if (strlen(PoliceID) +1 > TEXTLEN)
		{
			strncpy(this->PoliceID, PoliceID, TEXTLEN-1);
			this->PoliceID[TEXTLEN-1] = '\0';
		}
		else
		{
			strcpy(this->PoliceID, PoliceID);
		}

		if (strlen(pswd) +1 > TEXTLEN)
		{
			strncpy(PassWord, pswd, TEXTLEN-1);
			PassWord[TEXTLEN-1] = '\0';
		}
		else
		{
			strcpy(PassWord, pswd);
		}
	}

	p_ChnReqTypeMask[chn] |= 1<<type;
	
	return 0;
}

//chn: 指示哪一路报警源(sensor/IPCMD/IPCEXT)
int CSnapManager::alarmStatusChange(u8 chn, EM_ALARM_TYPE type, int status)
{
	SSG_MSG_TYPE msg;

	if (!m_Started)
	{
		return 0;
	}

	//printf("%s yg 2 chn%d type: %d\n", __func__, chn, type);
	
	if (type >= TypeAlarmMax)
	{
		printf("%s chn%d type: %d invalid\n", __func__, chn, type);
		return 1;
	}
	
	if (chn < 0 || chn > m_MaxIpcChn)
	{
		printf("%s chn%d invalid\n", __func__, chn);
		return 1;
	}	
	
	switch (type)
	{
		case TypeSensor:
		{			
			if (status)
			{
				if (p_SensorStatus[chn] == EM_ALARM_NONE)
				{
					printf("%s sensor%d trigger\n", __func__, chn);
					p_SensorStatus[chn] = EM_ALARM_TIGGER;	//报警触发	

					
					
				//发生报警后先发送不包含图片的报警消息
					//生成报警GUID，一组报警消息的所有GUID编码相同
					memset(&msg, 0, sizeof(SSG_MSG_TYPE));
					msg.type = EM_DVR_ALARM_EXT;// 26 DVR外部触发
					msg.chn = chn+1;
					strcpy(msg.note, GetParsedString("&CfgPtn.LocalAlarm"));
					upload_sg_proc(&msg, 0, NULL, 0, 0);
				
					srand(time(NULL));
					do {
						p_GUID_Sensor[chn] = rand();
					} while(p_GUID_Sensor[chn] == 0);

					memset(&msg, 0, sizeof(SSG_MSG_TYPE));
					msg.type = EM_PIC_ALARM_LINK_UPLOAD;// 1 报警联动图像
					msg.chn = chn+1;
					//本机chn报警，联动图像上传
					sprintf(msg.note, "%s%02d%s, %s", \
						GetParsedString("&CfgPtn.Local"),
						chn+1,
						GetParsedString("&CfgPtn.Alarm1"),
						GetParsedString("&CfgPtn.LinkPicUpload")
						);
					upload_sg_proc(&msg, 0, NULL, 0, p_GUID_Sensor[chn]);
					
				}
			}
			else
			{
				if ((p_SensorStatus[chn] == EM_ALARM_TIGGER) \
					|| (p_SensorStatus[chn] == EM_ALARM_ING))
				{
					printf("%s sensor%d over\n", __func__, chn);
					p_SensorStatus[chn] = EM_ALARM_END;	//报警解除
				}
			}

		} break;
		case TypeMD:
		{
			if (status)
			{
				if (p_IPCMDStatus[chn] == EM_ALARM_NONE)
				{
					p_IPCMDStatus[chn] = EM_ALARM_TIGGER;	//报警触发

				//发生报警后先发送不包含图片的报警消息
					//生成报警GUID，一组报警消息的所有GUID编码相同
					srand(time(NULL));
					do {
						p_GUID_IPCMD[chn] = rand();
					} while(p_GUID_IPCMD[chn] == 0);

					memset(&msg, 0, sizeof(SSG_MSG_TYPE));
					msg.type = EM_PIC_ALARM_LINK_UPLOAD;// 1 报警联动图像
					msg.chn = chn+1;
					//IPC chn移动侦测，联动图像上传
					sprintf(msg.note, "IPC%02d%s, %s", \
						chn+1,
						GetParsedString("&CfgPtn.Motion"),
						GetParsedString("&CfgPtn.LinkPicUpload")
						);
					upload_sg_proc(&msg, 0, NULL, 0, p_GUID_IPCMD[chn]);
				}
			}
			else
			{
				if ((p_IPCMDStatus[chn] == EM_ALARM_TIGGER) \
					|| (p_IPCMDStatus[chn] == EM_ALARM_ING))
				{
					p_IPCMDStatus[chn] = EM_ALARM_END;	//报警解除
				}
			}

		} break;
		case TypeIPCEXT:
		{
			if (status)
			{
				if (p_IPCEXTStatus[chn] == EM_ALARM_NONE)
				{
					p_IPCEXTStatus[chn] = EM_ALARM_TIGGER;	//报警触发
					
				//发生报警后先发送不包含图片的报警消息
					//生成报警GUID，一组报警消息的所有GUID编码相同
					srand(time(NULL));
					do {
						p_GUID_IPCEXT[chn] = rand();
					} while(p_GUID_IPCEXT[chn] == 0);

					memset(&msg, 0, sizeof(SSG_MSG_TYPE));
					msg.type = EM_PIC_ALARM_LINK_UPLOAD;// 1 报警联动图像
					msg.chn = chn+1;
					//IPC chn报警，联动图像上传
					sprintf(msg.note, "IPC%02d%s, %s", \
						chn+1,
						GetParsedString("&CfgPtn.Alarm1"),
						GetParsedString("&CfgPtn.LinkPicUpload")
						);
					upload_sg_proc(&msg, 0, NULL, 0, p_GUID_IPCEXT[chn]);
				}
			}
			else
			{
				if ((p_IPCEXTStatus[chn] == EM_ALARM_TIGGER) \
					|| (p_IPCEXTStatus[chn] == EM_ALARM_ING))
				{
					p_IPCEXTStatus[chn] = EM_ALARM_END;	//报警解除
				}
			}

		} break;
		default:
		{
			printf("%s type: %d invalid\n", __func__, type);
			return 1;
		}
	}
	
	return 0;
}

int CSnapManager::SnapWriteMsgQueue(EM_SNAPMSG_TYPE msg)
{
	int ret = 0;
	
	ret = WriteDataToBuf(&SnapMsgQueue, (u8 *)&msg, sizeof(EM_SNAPMSG_TYPE));
	if (ret)
	{
		printf("%s WriteDataToBuf failed\n", __func__);
	}
	
	return ret;
}

int CSnapManager::SnapReadMsgQueue(EM_SNAPMSG_TYPE *pmsg)
{
	return ReadDataFromBuf(&SnapMsgQueue, (u8 *)pmsg, sizeof(EM_SNAPMSG_TYPE));
}


/*************  Export API  *********************/

//服务初始化，启动
int SnapServerStart(u8 MaxSensorNum, u8 MaxIpcChn)
{	
	//测试硬盘，暂时关闭预录20150527
	//return 0;
	
	return g_SnapManager.Start(MaxSensorNum, MaxIpcChn);
}

//非报警类型请求抓图
int RequestSnap(u8 chn, EM_SNAP_TYPE type, const char *PoliceID, const char *pswd)
{
	return g_SnapManager.requestSnap(chn, type, PoliceID, pswd);		
}

//外部告知报警状态(触发和结束)，模块内部处理抓图
//type : 移动侦测、本机报警、IPC外部报警
//每种类型都要有掩码标示，主逻辑来处理向哪个IPC通道取图
int AlarmStatusChange(u8 chn, EM_ALARM_TYPE type, int status)
{	
	return g_SnapManager.alarmStatusChange(chn, type, status);	
}


//yaogang modify 2015715
int BizSnapPause(void)
{
	return g_SnapManager.SnapWriteMsgQueue(TypePause);
}

int BizSnapResume(void)
{
	return g_SnapManager.SnapWriteMsgQueue(TypeResume);
}


