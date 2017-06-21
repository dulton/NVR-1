#include "biz_alarm.h"

void AlarmDeal(SAlarmCbData* psAlarmCbData);

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

s32 AlarmInit(u8 nEnable, SBizAlarm* psInitPara)
{
	if(nEnable && psInitPara)
	{
		//csp modify 20131213
		SBizSystemPara sysPara;
		memset(&sysPara, 0, sizeof(SBizSystemPara));
		ConfigGetSystemPara(0, &sysPara);
		int nTimeZone = sysPara.nTimeZone;
		ModAlarmSetTimeZoneOffset(GetTZOffset(nTimeZone));
		
		SAlarmInitPara sAlarmInitPara;
		SBizManager* psBizManager = &g_sBizManager;
		
		sAlarmInitPara.nAlarmOutNum = psInitPara->nAlarmOutNum;
		sAlarmInitPara.nAlarmSensorNum = psInitPara->nAlarmSensorNum;
		sAlarmInitPara.nBuzzNum = psInitPara->nBuzzNum;
		sAlarmInitPara.nVBlindLuma = psInitPara->nVBlindLuma;
		sAlarmInitPara.nVideoChnNum = psInitPara->nVideoChnNum;
		sAlarmInitPara.nDiskNum = psInitPara->nDiskNum;
		sAlarmInitPara.pfnAlarmCb = AlarmDeal;
		sAlarmInitPara.nNVROrDecoder = psInitPara->nNVROrDecoder;
		
		if(0 != ModAlarmInit(&sAlarmInitPara, &psBizManager->hAla))
		{
			return -1;
		}
	}
	else
	{
		//处理返回多画面
		return 0;//csp modify
	}
	
	printf("Alarm init OK!!!\n");
	
	return 0;
}

//csp modify 20130326
#define	CMD_H8_LED8			(1 << 3)	//控制CH8-16的灯
#define	CMD_H8_LED0			(1 << 4)	//控制CH0-7的灯

static u16 chnstatus 		= 0x0;

void ChnLedInit(u32 status)
{
#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	chnstatus = status & 0xffff;
	
	char buf[2]={0};
	buf[0] = CMD_H8_LED0;
	buf[1] = (chnstatus & 0xff);
	FrontboardWrite(buf,2);
	
	buf[0] = CMD_H8_LED8;
	buf[1] = (chnstatus >> 8) & 0xff;
	FrontboardWrite(buf,2);
#endif
}

void ChnLedCtrl(int flag,int chn)
{
#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	if(chn >= 16)
	{
		return;
	}
	flag = flag?1:0;
	
	char buf[2]={0};
	u16 newstatus = (chnstatus & ~(1<<chn)) | (flag<<chn);
	if(chn < 8)
	{
		if((newstatus & 0xff) != (chnstatus & 0xff))
		{
			buf[0] = CMD_H8_LED0;
			buf[1] = (newstatus & 0xff);
			FrontboardWrite(buf,2);
		}
	}
	else
	{
		if((newstatus & 0xff00) != (chnstatus & 0xff00))
		{
			buf[0] = CMD_H8_LED8;
			buf[1] = (newstatus >> 8) & 0xff;
			FrontboardWrite(buf,2);
		}
	}
	
	chnstatus = newstatus;
#endif
}

//yaogang modify 20141118
/*
0		//硬盘未格式化报警
1		//硬盘丢失报警
2		//硬盘读写出错报警
3		//硬盘满报警
4		//信号量报警
5		//视频丢失报警
6		//移动侦测报警
7		//IPC遮盖报警
8		//制式不匹配报警
9		//非法访问报警
10		//IPC外部报警
11		//485扩展报警

*/
void AlarmDeal(SAlarmCbData* psAlarmCbData)
{
	//printf("AlarmDeal......\n");
	SBizManager* psBizManager = &g_sBizManager;
	
	if(psAlarmCbData)
	{
		//printf("AlarmDeal... SAlarmCbData:event:%d chn:%d, nData:%d\n", psAlarmCbData->emAlarmEvent, psAlarmCbData->nChn, psAlarmCbData->nData);
		
		SBizEventPara sBizEventPara;
		
		sBizEventPara.emType = EM_BIZ_EVENT_UNKNOW;
		sBizEventPara.sBizAlaStatus.nChn = psAlarmCbData->nChn;
		sBizEventPara.sBizAlaStatus.nLastStatus = sBizEventPara.sBizAlaStatus.nCurStatus;//why???
		sBizEventPara.sBizAlaStatus.nCurStatus = psAlarmCbData->nData;
		sBizEventPara.sBizAlaStatus.nTime = psAlarmCbData->nTime;
		switch(psAlarmCbData->emAlarmEvent)
		{
		case EM_ALARM_EVENT_SENSOR:  //传感器报警事件
			{
				sBizEventPara.emType = EM_BIZ_EVENT_ALARM_SENSOR;
				BizSetUploadAlarmPara(5, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
			}
		case EM_ALARM_EVENT_DISK_LOST:
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					//printf("%s EM_ALARM_EVENT_DISK_LOST\n", __func__);
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_DISK_LOST;
					BizSetUploadAlarmPara(1, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
			}
		case EM_ALARM_EVENT_DISK_ERR:
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					//printf("%s EM_ALARM_EVENT_DISK_ERR\n", __func__);
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_DISK_ERR;
					BizSetUploadAlarmPara(2, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
			}
		case EM_ALARM_EVENT_DISK_NONE:
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					//printf("%s EM_ALARM_EVENT_DISK_NONE\n", __func__);
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_DISK_NONE;
					BizSetUploadAlarmPara(4, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
			}
		case EM_ALARM_EVENT_485EXT:
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					//printf("%s EM_ALARM_EVENT_485EXT\n", __func__);
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_485EXT;
					BizSetUploadAlarmPara(12, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
			}
		case EM_ALARM_EVENT_IPCEXT:
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					//printf("%s EM_ALARM_EVENT_IPCEXT\n", __func__);
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_IPCEXT;
					BizSetUploadAlarmPara(11, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
			}
		case EM_ALARM_EVENT_IPCCOVER:
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					//printf("%s EM_ALARM_EVENT_IPCCOVER\n", __func__);
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_IPCCOVER;
					BizSetUploadAlarmPara(8, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
			}
		case EM_ALARM_EVENT_VLOST: //视频丢失报警事件
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					//csp modify 20130326
					//printf("%s EM_ALARM_EVENT_VLOST chn%02d\n", __func__, sBizEventPara.sBizAlaStatus.nChn);
					ChnLedCtrl(!GetAlarmCheckStaue(),sBizEventPara.sBizAlaStatus.nChn);
					
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_VLOSS;
					BizSetUploadAlarmPara(6, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
			}
		case EM_ALARM_EVENT_VBLIND:  //遮挡报警事件
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_VBLIND;
					BizSetUploadAlarmPara(8, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
			}
		case EM_ALARM_EVENT_VMOTION:  //移动侦测报警事件
			{
				if(EM_BIZ_EVENT_UNKNOW == sBizEventPara.emType)
				{
					sBizEventPara.emType = EM_BIZ_EVENT_ALARM_VMOTION;
					BizSetUploadAlarmPara(7, sBizEventPara.sBizAlaStatus.nChn, GetAlarmCheckStaue());
				}
				
				//SendBizEvent(&sBizEventPara);
				//SendBizEvent2(&sBizEventPara);
				//事件回调****************************************************************
				psBizManager->sBizPara.pfnBizEventCb(&sBizEventPara);
			}
			break;
		case EM_ALARM_EVENT_DISPATCH_ZOOMCHN: //触发通道放大事件;
		case EM_ALARM_EVENT_DISPATCH_EMAIL: //触发email事件;暂不支持
		case EM_ALARM_EVENT_DISPATCH_SNAP: //触发抓图事件;暂不支持
		case EM_ALARM_EVENT_DISPATCH_REC_SENSOR: //传感器触发录像事件
		case EM_ALARM_EVENT_DISPATCH_REC_IPCEXTSENSOR:
		case EM_ALARM_EVENT_DISPATCH_REC_IPCCOVER:
		case EM_ALARM_EVENT_DISPATCH_REC_VMOTION: //传感器触发录像事件
		case EM_ALARM_EVENT_DISPATCH_REC_VBLIND: //传感器触发录像事件
		case EM_ALARM_EVENT_DISPATCH_REC_VLOST: //传感器触发录像事件
		case EM_ALARM_EVENT_DISPATCH_PTZ_PRESET: //触发云台联动预置点事件
		case EM_ALARM_EVENT_DISPATCH_PTZ_PATROL: //触发云台联动巡航事件
		case EM_ALARM_EVENT_DISPATCH_PTZ_LOCUS: //触发云台联动轨迹事件
			//SendBizDispatch(psAlarmCbData);
			AlarmDispatchEvent(psAlarmCbData);
			break;
		//csp modify 20130326
		case EM_ALARM_EVENT_CTRL_CHN_LED:
			ChnLedInit(psAlarmCbData->reserved[0]);
			break;
		default:
			break;
		}
	}
	
	//printf("AlarmDeal......Over!!!!\n");
}

s32 AlarmSetPara(u8 nChn, EMBIZPARATYPE emType, void* pData)
{
	//return 0;
	
	if(pData)
    {
        SAlarmPara sAlaPara;
        EMALARMPARATYPE emAlaType = EM_ALARM_PARA_NULL;
        SBizManager* psBizManager = &g_sBizManager;
        
        switch(emType)
        {
            case EM_BIZ_SENSORPARA:  //传感器参数
                emAlaType = EM_ALARM_PARA_SENSOR;
                //memcpy(&sAlaPara.sAlaSensorPara , pData, sizeof(SBizAlarmSensorPara));
                SBizAlarmSensorPara* SensorPara = (SBizAlarmSensorPara*)pData;
				sAlaPara.sAlaSensorPara.emType = SensorPara->nType;
				sAlaPara.sAlaSensorPara.nEnable = SensorPara->nEnable;
				sAlaPara.sAlaSensorPara.nDelay = SensorPara->nDelay;
				strcpy(sAlaPara.sAlaSensorPara.name, SensorPara->name);
				//printf("chn:%d, type:%d, enable:%d\n", nChn, SensorPara->nType, SensorPara->nEnable);
	    case EM_BIZ_IPCEXTSENSORPARA:
		if (EM_ALARM_PARA_NULL == emAlaType)
		{
			emAlaType = EM_ALARM_PARA_IPCEXTSENSOR;
	                //memcpy(&sAlaPara.sAlaSensorPara , pData, sizeof(SBizAlarmSensorPara));
	                SBizAlarmIPCExtSensorPara* SensorPara = (SBizAlarmIPCExtSensorPara* )pData;
					sAlaPara.sAlaIPCExtSensorPara.nEnable = SensorPara->nEnable;
					sAlaPara.sAlaIPCExtSensorPara.nDelay = SensorPara->nDelay;
		}
	 case EM_BIZ_485EXTSENSORPARA:
		if (EM_ALARM_PARA_NULL == emAlaType)
		{
			emAlaType = EM_ALARM_PARA_485EXTSENSOR;
	                //memcpy(&sAlaPara.sAlaSensorPara , pData, sizeof(SBizAlarmSensorPara));
	                SBizAlarmIPCExtSensorPara* SensorPara = (SBizAlarmIPCExtSensorPara* )pData;
					sAlaPara.sAla485ExtSensorPara.nEnable = SensorPara->nEnable;
					sAlaPara.sAla485ExtSensorPara.nDelay = SensorPara->nDelay;
		}
	case EM_BIZ_IPCCOVERPARA:
		if (EM_ALARM_PARA_NULL == emAlaType)
		{
			emAlaType = EM_ALARM_PARA_IPCCOVER;
	                //memcpy(&sAlaPara.sAlaSensorPara , pData, sizeof(SBizAlarmSensorPara));
	                SBizAlarmIPCExtSensorPara* SensorPara = (SBizAlarmIPCExtSensorPara* )pData;
					sAlaPara.sAlaIPCCoverPara.nEnable = SensorPara->nEnable;
					sAlaPara.sAlaIPCCoverPara.nDelay = SensorPara->nDelay;
		}
	case EM_BIZ_HDDPARA:
		if (EM_ALARM_PARA_NULL == emAlaType)
		{
			emAlaType = EM_ALARM_PARA_HDD;
	                //memcpy(&sAlaPara.sAlaSensorPara , pData, sizeof(SBizAlarmSensorPara));
	                SBizAlarmHDDPara* SensorPara = (SBizAlarmHDDPara* )pData;
					sAlaPara.sAlaHDDPara.nEnable = SensorPara->nEnable;
					//sAlaPara.sAla485ExtSensorPara.nDelay = SensorPara->nDelay;
		}
            case EM_BIZ_VMOTIONPARA:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_VMOTION;
                    //memcpy(&sAlaPara.sAlaVMotionPara, pData, sizeof(SBizAlarmVMotionPara));
                    SEncodeParam sEncPara;
					SBizDvrInfo sDvr;
					EMCHNTYPE emChnType = EM_CHN_VIDEO_MAIN;
					ModEncodeGetParam(nChn, emChnType, &sEncPara);
					ConfigGetDvrInfo(0,&sDvr);
					
					sAlaPara.sAlaVMotionPara.emSetType = EM_ALMARM_VMOTION_AREA_SET;
					sAlaPara.sAlaVMotionPara.emResol = sEncPara.sVideoMainParam.emVideoResolution;
					sAlaPara.sAlaVMotionPara.nCols = sDvr.nMDAreaCols;
					sAlaPara.sAlaVMotionPara.nRows = sDvr.nMDAreaRows;
					
                    SBizAlarmVMotionPara* VMotionPara = (SBizAlarmVMotionPara*)pData;
					sAlaPara.sAlaVMotionPara.nEnable = VMotionPara->nEnable;
					sAlaPara.sAlaVMotionPara.nSensitivity = VMotionPara->nSensitivity;
					sAlaPara.sAlaVMotionPara.nDelay = VMotionPara->nDelay;
					memcpy(sAlaPara.sAlaVMotionPara.nBlockStatus, VMotionPara->nBlockStatus, sizeof(VMotionPara->nBlockStatus));
					
					if(VMotionPara->nEnable)
					{
						EncodeRequestStartWithoutRec(nChn, EM_CHN_VIDEO_MAIN);
					}	
					else
					{
						EncodeRequestStopWithoutRec(nChn, EM_CHN_VIDEO_MAIN);
					}
                }
            case EM_BIZ_VBLINDPARA:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_VBLIND;
                    //memcpy(&sAlaPara.sAlaVBlindPara, pData, sizeof(SBizAlarmVBlindPara));
                    SBizAlarmVBlindPara* VBlindPara = (SBizAlarmVBlindPara*)pData;
					sAlaPara.sAlaVBlindPara.nDelay = VBlindPara->nDelay;
					sAlaPara.sAlaVBlindPara.nEnable = VBlindPara->nEnable;
                }
            case EM_BIZ_VLOSTPARA:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_VLOST;
                    //memcpy(&sAlaPara.sAlaVLostPara, pData, sizeof(SBizAlarmVLostPara));
                    SBizAlarmVLostPara* VLostPara = (SBizAlarmVLostPara*)pData;
					sAlaPara.sAlaVLostPara.nDelay = VLostPara->nDelay;
					sAlaPara.sAlaVLostPara.nEnable = VLostPara->nEnable;
                }
            case EM_BIZ_ALARMOUTPARA: //报警输出参数
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_ALARMOUT;
                    //memcpy(&sAlaPara.sAlaOutPara, pData, sizeof(SBizAlarmOutPara));
                    SBizAlarmOutPara* AlarmOutPara = (SBizAlarmOutPara*)pData;
					//sAlaPara.sAlaOutPara.emType = AlarmOutPara->nType;
					if(AlarmOutPara->nType == EM_BIZ_ALARM_NORMAL_CLOSE)
					{
						sAlaPara.sAlaOutPara.emType = EM_ALARM_SENSOR_NORMALCLOSE;
					}
					else
					{
						sAlaPara.sAlaOutPara.emType = EM_ALARM_SENSOR_NORMALOPEN;
					}
					sAlaPara.sAlaOutPara.nDelay = AlarmOutPara->nDelay;
					sAlaPara.sAlaOutPara.nEnable = AlarmOutPara->nEnable;
					strcpy(sAlaPara.sAlaOutPara.name, AlarmOutPara->name);
                }
            case EM_BIZ_ALARMBUZZPARA: //蜂鸣器参数输出
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_BUZZ;
                    //memcpy(&sAlaPara.sAlaBuzzPara, pData, sizeof(SBizAlarmBuzzPara));
                    SBizAlarmBuzzPara* BuzzPara = (SBizAlarmBuzzPara*)pData;
					sAlaPara.sAlaBuzzPara.nDelay = BuzzPara->nDelay;
					sAlaPara.sAlaBuzzPara.nEnable = BuzzPara->nEnable;
					//printf("buzz enable: %d\n", BuzzPara->nEnable);
					sAlaPara.sAlaBuzzPara.nDuration = 0;
					sAlaPara.sAlaBuzzPara.nInterval = 0;					
                }
             //一般报警事件触发参数设置
            case EM_BIZ_DISPATCH_SENSOR:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_SENSOR;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
	    case EM_BIZ_DISPATCH_IPCEXTSENSOR:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_IPCEXTSENSOR;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
	case EM_BIZ_DISPATCH_485EXTSENSOR:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_485EXTSENSOR;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
	case EM_BIZ_DISPATCH_IPCCOVER:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_IPCCOVER;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
            case EM_BIZ_DISPATCH_VMOTION:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_VMOTION;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
            case EM_BIZ_DISPATCH_VBLIND:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_VBLIND;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
            case EM_BIZ_DISPATCH_VLOST:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_VLOST;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
	case EM_BIZ_DISPATCH_HDD:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_HDD;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
            //一般报警事件布防设置
            case EM_BIZ_SCHEDULE_SENSOR:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_SENSOR;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
	case EM_BIZ_SCHEDULE_IPCEXTSENSOR:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_IPCEXTSENSOR;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            case EM_BIZ_SCHEDULE_VMOTION:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_VMOTION;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            case EM_BIZ_SCHEDULE_VBLIND:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_VBLIND;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            case EM_BIZ_SCHEDULE_VLOST:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_VLOST;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            //报警输出事件布防设置
            case EM_BIZ_SCHEDULE_ALARMOUT:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_ALARMOUT;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            case EM_BIZ_SCHEDULE_BUZZ: //暂时不支持
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_BUZZ;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
                
                return ModAlarmSetParam(psBizManager->hAla, emAlaType, nChn, &sAlaPara);
            default:
                break;
        }
    }
	
    return -1;
}

s32 AlarmGetPara(u8 nChn, EMBIZPARATYPE emType, void* pData)
{
    if (pData)
    {
       SAlarmPara sAlaPara;
       EMALARMPARATYPE emAlaType = EM_ALARM_PARA_NULL;
       SBizManager* psBizManager = &g_sBizManager;
   #if 0
        switch (emType)
        {
            case EM_BIZ_SENSORPARA:  //传感器参数
                emAlaType = EM_ALARM_PARA_SENSOR;
                memcpy(&sAlaPara.sAlaSensorPara , pData, sizeof(SBizAlarmSensorPara));
            case EM_BIZ_VMOTIONPARA:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_VMOTION;
                    memcpy(&sAlaPara.sAlaVMotionPara, pData, sizeof(SBizAlarmVMotionPara));
                }
            case EM_BIZ_VBLINDPARA:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_VBLIND;
                    memcpy(&sAlaPara.sAlaVBlindPara, pData, sizeof(SBizAlarmVBlindPara));
                }
            case EM_BIZ_VLOSTPARA:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_VLOST;
                    memcpy(&sAlaPara.sAlaVLostPara, pData, sizeof(SBizAlarmVLostPara));
                }
            case EM_BIZ_ALARMOUTPARA: //报警输出参数
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_ALARMOUT;
                    memcpy(&sAlaPara.sAlaOutPara, pData, sizeof(SBizAlarmOutPara));
                }
            case EM_BIZ_ALARMBUZZPARA: //蜂鸣器参数输出
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_PARA_BUZZ;
                    memcpy(&sAlaPara.sAlaBuzzPara, pData, sizeof(SBizAlarmBuzzPara));
                }
             //一般报警事件触发参数设置
            case EM_BIZ_DISPATCH_SENSOR:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_SENSOR;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
            case EM_BIZ_DISPATCH_VMOTION:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_VMOTION;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
            case EM_BIZ_DISPATCH_VBLIND:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_VBLIND;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
            case EM_BIZ_DISPATCH_VLOST:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_DISPATCH_VLOST;
                    memcpy(&sAlaPara.sAlaDispatch, pData, sizeof(SBizAlarmDispatch));
                }
            //一般报警事件布防设置
            case EM_BIZ_SCHEDULE_SENSOR:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_SENSOR;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            case EM_BIZ_SCHEDULE_VMOTION:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_VMOTION;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            case EM_BIZ_SCHEDULE_VBLIND:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_VBLIND;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            case EM_BIZ_SCHEDULE_VLOST:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_VLOST;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            //报警输出事件布防设置
            case EM_BIZ_SCHEDULE_ALARMOUT:
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_ALARMOUT;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }
            case EM_BIZ_SCHEDULE_BUZZ: //暂时不支持
                if (EM_ALARM_PARA_NULL == emAlaType)
                {
                    emAlaType = EM_ALARM_SCHEDULE_BUZZ;
                    memcpy(&sAlaPara.sAlaSchedule, pData, sizeof(SBizAlarmSchedule));
                }

                return ModAlarmGetParam(psBizManager->hAla, emAlaType, nChn, &sAlaPara);
            default:
                break;
        }
#endif
    }

    return -1;
}

s32 AlarmWorkingEnable(u8 nEnable)
{
	SBizManager* psBizManager = &g_sBizManager;
	return ModAlarmWorkingEnable(psBizManager->hAla, nEnable);
}

