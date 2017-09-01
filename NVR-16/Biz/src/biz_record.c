#include "biz_record.h"
#include "biz_log.h"
#include "biz_syscomplex.h"
#include "biz_snap.h"

void RecordDeal(void* status);

static int TimeZoneOffset[] = 
{
	-12*3600,
	-11*3600,
	-10*3600,
	-9*3600,
	-8*3600,
	-7*3600,
	-6*3600,
	-5*3600,
	-4*3600-1800,
	-4*3600,
	-3*3600-1800,
	-3*3600,
	-2*3600,
	-1*3600,
	0,
	1*3600,
	2*3600,
	3*3600,
	3*3600+1800,
	4*3600,
	4*3600+1800,
	5*3600,
	5*3600+1800,
	5*3600+2700,
	6*3600,
	6*3600+1800,
	7*3600,
	8*3600,
	9*3600,
	9*3600+1800,
	10*3600,
	11*3600,
	12*3600,
	13*3600,
};

static int GetTZOffset(int index)
{
	if(index < 0 || index >= (int)(sizeof(TimeZoneOffset)/sizeof(TimeZoneOffset[0])))
	{
		return 0;
	}
	
	return TimeZoneOffset[index];
}

s32 RecordInit(u8 nEnable, SBizRecord* psInitPara)
{
	printf("%s nEnable: %d\n", __func__, nEnable);
	if(nEnable && psInitPara)
	{
		//csp modify 20131213
		SBizSystemPara sysPara;
		memset(&sysPara, 0, sizeof(SBizSystemPara));
		ConfigGetSystemPara(0, &sysPara);
		int nTimeZone = sysPara.nTimeZone;
		ModRecordSetTimeZoneOffset(GetTZOffset(nTimeZone));
		
		if(0 != ModRecordInit(psInitPara->nChnNum, psInitPara->sModRecPara, RecordDeal, psInitPara->hddinfo, RecordExceptionCB))
		{
			return -1;
		}
	}
	
	return 0;
}

s32 RecordDeInit()
{
	printf("^^^^^^^^^^^ModeRecordDeinit\n");
	return ModRecordDeInit();
}

void RecordDeal(void* status)
{
	//printf("RecordDeal [0x%08x]......\n", status);
	
	if(status)
	{
		SBizManager* psBizManager = &g_sBizManager;
		
		u8* pnStatus;
		pnStatus = (u8 *)status;
		
		int i;
		for(i = 0; i < psBizManager->nVideoMainNum; i++)
		{
			if(pnStatus[i] != psBizManager->pnVideoMainRecStatus[i])
			{
				u32 nChnStatus = ((i << 8) | pnStatus[i]);//高8位通道，低8位状态
				//printf("chn:%d, RecordDeal :0x%04x\n", i, nChnStatus);
				
				SendBizInnerMsgCmd(EM_BIZMSG_VIDEOMAIN_RECSTATUS, (void *)nChnStatus);
			}
			else
			{
				//if(i == 0) printf("chn:%d ,RecordDeal status[%d] no change\n", i, psBizManager->pnVideoMainRecStatus[i]);
			}
		}
	}
	else
	{
		PUBPRT("NULL == status");
	}
}

s32 RecordStartManual(u8 nChn)
{
	//printf("chn %d , RecordStartManual\n",nChn);
    return ModRecordStartManual(nChn);
}

s32 RecordStopManual(u8 nChn)
{
	//printf("chn %d , RecordStopManual\n",nChn);
    return ModRecordStopManual(nChn);
}

s32 RecordStop(u8 nChn)
{
    return ModRecordStop(nChn);
}
	
s32 RecordStopAll(void)
{
    return ModRecordStopAll();
}

//yaogang modify 20150314
//0: format 1: hotplugup 2:update
s32 RecordSnapPause(u8 cause)
{
    return ModRecordSnapPause(cause);
}

s32 RecordSnapResume(u8 cause)
{
     return ModRecordSnapResume(cause);
}

s32 RecordPause(u8 nChn)
{
    return ModRecordPause(nChn);
}

s32 RecordResume(u8 nChn)
{
     return ModRecordResume(nChn);
}

s32 RecordSetPreTime(u8 nChn, u16 nTime)
{
    return ModRecordSetPreTime(nChn, (u32)nTime);
}

s32 RecordSetDelayTime(u8 nChn, u16 nTime)
{
    return ModRecordSetDelayTime(nChn, (u32)nTime);
}

s32 RecordSetTrigger(u8 nChn, EMRECALARMTYPE emType, u8 nKey)
{
    return ModRecordSetTrigger(nChn, emType, nKey);
}

s32 RecordSetSchedulePara(u8 nChn, SModRecSchPara* psPara)
{
    return ModRecordSetSchedulePara(nChn, psPara);
}

s32 RecordWriteOneFrame(u8 nChn, SModRecRecordHeader* psHeader)
{
    return ModRecordWriteOneFrame(nChn, psHeader);
}

void RecordSetDealHardDiskFull(u8 nCover) //硬盘满时是否覆盖:0否 1是
{
	ModRecordSetDealHardDiskFull(nCover);
}

void RecordSetWorkingEnable(u8 nEnable)
{
	ModRecordSetWorkingEnable(nEnable);
}

s32 RecordBufRelease(void)
{
	return ModRecordBufRelease();
}

s32 RecordBufResume(void)
{
	return ModRecordBufResume();
}

extern char* BizGetUserNameLog();

void RecordExceptionCB(u8 nType)
{
	SBizLogInfo sLogInfo;
	memset(&sLogInfo, 0, sizeof(sLogInfo));
	sLogInfo.nMasterType = BIZ_LOG_MASTER_ERROR;
	strcpy(sLogInfo.aUsername, BizGetUserNameLog());
	strcpy(sLogInfo.aIp, "GUI");
	
	switch(nType)
	{
		case 0:
			sLogInfo.nSlaveType = BIZ_LOG_SLAVE_ERROR_HDD_ABNORMAL;
			break;

		case 1:
			sLogInfo.nSlaveType = BIZ_LOG_SLAVE_ERROR_ENC_ABNORMAL;
			break;

		case 2:
			sLogInfo.nSlaveType = BIZ_LOG_SLAVE_ERROR_FILE_ABNORMAL;
			break;

		case 3:
			sLogInfo.nMasterType = BIZ_LOG_MASTER_SYSCONTROL;
			sLogInfo.nSlaveType = BIZ_LOG_SLAVE_REBOOT;
			break;

		default:
			return;
	}
	WriteLog(&sLogInfo);
	
	if(3 == nType) //硬盘已经异常，日志可能就写不进去了
	{
		sysComplex_resetcb();
	}
	
	return;
}

//yaogang modify 20141225
/*保存抓图类型	bit_mask	value
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

typedef enum
{
	//upload
	TypeManualTEST = 0,	// 0 测试图像上传--监管平台--测试保养
	TypeManualMAINTAIN,	// 1 保养图像上传
	TypeManualACCEPT,		// 2 验收管理上传
	TypeDaily,				// 3 日常
	//record
	TypePreSnap,			// 4 预录
	TypeTime,				// 5 定时-录像配置--抓图参数
	TypeManual,			// 6 手动截图
	//upload & record
	TypeAlarmSensor,		// 7 本地传感器报警
	TypeMotionDet,			// 8 移动侦测
	TypeAlarmIPCEXT,		// 9 IPC外部报警
	TypeSnapMax,
} EM_SNAP_TYPE;

typedef struct rec_header
{
	u8	nChn;					//帧的通道号
	u8	nStreamType;				//码流类型（0:视频流、1:视音频流、2: snap）
	u8	nMediaType; 				//编码方式（0:H264、10:PCMU、11:ADPCM）
	u8	nFrameType; 				//是否关键帧(0:否、1:是) FRAME_TYPE_P = 0,FRAME_TYPE_I = 1/3/5
								//nFrameType 在Snap模块用做抓图类型EM_SNAP_TYPE
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
//psHeader->nFrameType(EM_SNAP_TYPE) 抓图图片类型Convert to 保存图片类型
//EM_SNAP_TYPE: 5 预录6 定时7 手动截图8910 报警
//index-5
int SnapToSave[] = {5, 0, 4, 2, 1, 2};

s32 RecordReadOnePreSnap(u8 nChn, SModRecRecordHeader* psHeader)
{
	struct timeval tv;
	if (nChn != psHeader->nChn)
	{
		printf("%s nChn%d != psHeader->nChn%d\n", __func__, \
			nChn, psHeader->nChn);
		return 1;
	}

	if (psHeader->nStreamType != 2) //nStreamType = 2;//Snap
	{
		printf("%s nStreamType: %d invalid\n", __func__, psHeader->nStreamType);
		return 1;
	}

	memcpy(&tv, &psHeader->nPts, sizeof(tv));

	return ModRecordPreSnapFromFile(nChn, tv.tv_sec, psHeader->nDate, &psHeader->nDataLength);
}

s32 RecordWriteOneSnap(u8 nChn, SModRecRecordHeader* psHeader)
{
	u32 snap_type = psHeader->nBitRate;
	int i, index;
	u8 pic_type;
	
	struct timeval tv;

	if (nChn != psHeader->nChn)
	{
		printf("%s nChn%d != psHeader->nChn%d\n", __func__, \
			nChn, psHeader->nChn);
		return 1;
	}
	
	if (psHeader->nStreamType != 2) //nStreamType = 2;//Snap
	{
		printf("%s nStreamType: %d invalid\n", __func__, psHeader->nStreamType);
		return 1;
	}

	//转换图片记录类型
	pic_type = 0;
	for (i =(int)TypePreSnap; i <= (int)TypeAlarmIPCEXT; i++)
	{
		if (snap_type & (1<<i))
		{
			index = i - (int)TypePreSnap;
			pic_type |= 1<<SnapToSave[index];
		}
	}

	memcpy(&tv, &psHeader->nPts, sizeof(tv));

	//printf("%s chn%d, snap_type: 0x%x convert to save_type: 0x%x, size: %d, time: %d\n", 
		//__func__, nChn, snap_type, pic_type, psHeader->nDataLength, tv.tv_sec);
	
	return ModRecordSnapshotToBuf(nChn, pic_type, tv.tv_sec, tv.tv_usec, \
		psHeader->nDate, psHeader->nDataLength, psHeader->width, psHeader->height);
}

s32 RecordReadOneSnap(u8 nChn, SModRecRecordHeader* psHeader)
{
	if (psHeader->nStreamType != 2) //nStreamType = 2;//Snap
	{
		printf("%s nStreamType: %d invalid\n", __func__, psHeader->nStreamType);
		return 1;
	}
}



