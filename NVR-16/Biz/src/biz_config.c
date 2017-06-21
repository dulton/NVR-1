#include "biz_config.h"

s32 ConfigInit(u8 nEnable, SBizConfig* psInitPara)
{
   if (nEnable)
   {
        if (0 != ModConfigInit())
        {
            return -1;
        }
    }

    return 0;
}

s32 DefaultConfigRelease(void)
{
    if (0 != ModDefaultConfigRelease())
    {
        return -1;
    }
    return 0;
}

s32 DefaultConfigResume(void)
{
    if (0 != ModDefaultConfigResume())
    {
        return -1;
    }
    return 0;
}

s32 ConfigGetDvrInfo(u32 bDefault,SBizDvrInfo *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigDvrProperty sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_DVR_PROPERTY, &sConfig, 0);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_DVR_PROPERTY, &sConfig, 0);
	}
	
	if (0 == ret)
	{
		strcpy(psPara->strVersion, sConfig.nVersion);
		strcpy(psPara->strModel, sConfig.nModel);
		strcpy(psPara->sproductnumber, sConfig.nproductnumber);
		psPara->nPreviewNum = sConfig.nPreviewNum;
		psPara->nVidMainNum = sConfig.nVidMainNum;
		psPara->nVidSubNum = sConfig.nVidSubNum;
		psPara->nAudNum = sConfig.nAudNum;
		psPara->nSnapNum = sConfig.nSnapNum;
		psPara->nVoipNum = sConfig.nVoipNum;
		psPara->nSensorNum = sConfig.nSensorNum;
		psPara->nRecNum = sConfig.nRecNum;
		psPara->nAlarmoutNum = sConfig.nAlarmoutNum;
		psPara->nBuzzNum = sConfig.nBuzzNum;
		psPara->nMaxHddNum = sConfig.nMaxHddNum;
		psPara->nMaxMainFrameSize = sConfig.nMaxMainFrameSize;
		psPara->nMaxSubFrameSize = sConfig.nMaxSubFrameSize;
		psPara->nMaxAudFrameSize = sConfig.nMaxAudFrameSize;
		psPara->nMaxSnapFrameSize = sConfig.nMaxSnapFrameSize;

		psPara->nOsdOperator = sConfig.nOsdOperator;
		psPara->nStrOsdNum = sConfig.nStrOsdNum;
		psPara->nImgOsdNum = sConfig.nImgOsdNum;
		psPara->nRectOsdNum = sConfig.nRectOsdNum;
		psPara->sRectOsdRgb.nR = sConfig.nRectOsdRgb[0];
		psPara->sRectOsdRgb.nG = sConfig.nRectOsdRgb[1];
		psPara->sRectOsdRgb.nB = sConfig.nRectOsdRgb[2];
		psPara->nSplitLineNum = sConfig.nLineOsdNum;
		psPara->nRefWidth = sConfig.nRefWidth;
		psPara->nRefHeight = sConfig.nRefHeight;

		psPara->nAudioEncType = sConfig.nAudioEncType;
		psPara->nSampleRate = sConfig.nAudioSampleRate;
		psPara->nSampleDuration = sConfig.nAudioSampleDuration;
		psPara->nSampleSize = sConfig.nAudioSampleSize;
		psPara->nBitWidth = sConfig.nAudioBitWidth;

		psPara->nVoipEncType = sConfig.nVoipEncType;
		psPara->nVoipSampleRate = sConfig.nVoipSampleRate;
		psPara->nVoipSampleDuration = sConfig.nVoipSampleDuration;
		psPara->nVoipSampleSize = sConfig.nVoipSampleSize;
		psPara->nVoipBitWidth = sConfig.nVoipBitWidth;

		psPara->nMDAreaRows = sConfig.nMdAreaRows;
		psPara->nMDAreaCols = sConfig.nMdAreaCols;
		psPara->nMDSenseMin = sConfig.nMdSenseMin;
		psPara->nMDSenseMax = sConfig.nMdSenseMax;
		    
		psPara->nSchSegments = sConfig.nSchSegments;
		psPara->nOutputSync = sConfig.nOutputSync;
		//psPara->BuzzDuration = sConfig.BuzzDuration; //wait
		//psPara->nBuzzInterval = sConfig.nBuzzInterval; //wait
		psPara->nNVROrDecoder = sConfig.nNVROrDecoder;	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	}
	
	return ret;
}

s32 ConfigGetMaxChnNum(void)
{
	SBizDvrInfo sInf;
	s32         ret;
	u8 			nChMax = 4;

	ret = BizGetDvrInfo(&sInf);
	if(ret == 0)
	{
		nChMax = sInf.nVidMainNum;
	}
	
	return nChMax;
}

u8 GetChnNum()
{
	SBizDvrInfo para;
	ConfigGetDvrInfo(0,&para);
	return para.nPreviewNum;
}

s32 ConfigGetMainTainPara(u32 bDefault,SBizMainTainPara *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigMainTainParam sConfig;
	memset(&sConfig, 0, sizeof(sConfig));

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_MAINTAIN, &sConfig, 0);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_MAINTAIN, &sConfig, 0);
	}
	
	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnable;
		psPara->nMainType = sConfig.nMainType;
		psPara->nSubType = sConfig.nSubType;
		psPara->ntime = sConfig.ntime;
	}
	
	return ret;
}

s32 ConfigGetSystemPara(u32 bDefault,SBizSystemPara *psPara)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSystemParam sConfig;
	memset(&sConfig, 0, sizeof(sConfig));
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SYSTEM, &sConfig, 0);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SYSTEM, &sConfig, 0);
	}
	
	if(0 == ret)
	{
		strcpy(psPara->strDevName, sConfig.nDevName);
		psPara->nDevId = sConfig.nDevId;
		psPara->nManual = sConfig.nManual;
		psPara->nVideoStandard = sConfig.nVideoStandard;
		psPara->nAuthCheck = sConfig.nAuthCheck;
		psPara->nOutput = sConfig.nOutput;
		psPara->nLangId = sConfig.nLangId;
		psPara->nShowGuide= sConfig.nShowGuide;
		psPara->nLockTime = sConfig.nLockTime;
		psPara->nDateFormat = sConfig.nDateFormat;
		psPara->nTimeStandard = sConfig.nTimeStandard;
		psPara->nCyclingRecord = sConfig.nCyclingRecord;
		
		//csp modify
		//psPara->nShowFirstMenue = sConfig.nShowFirstMenue;
		psPara->nShowFirstMenue = 0;
		
		//csp modify
		strcpy(psPara->sn, sConfig.sn);
		strcpy(psPara->reserved, sConfig.reserved);
		
		strcpy(psPara->strNtpServerAdress, sConfig.nNtpServerAdress);//QYJ
		psPara->nSyscCheck = sConfig.nSyscCheck;//QYJ
		psPara->nTimeZone = sConfig.nTimeZone;//QYJ
		
		psPara->nSyncTimeToIPC = sConfig.nSyncTimeToIPC;
		psPara->P2PEnable = sConfig.P2PEnable;
	}
	
	return ret;
}

s32 ConfigGetCfgPreview(u32 bDefault,SBizCfgPreview *psPara)
{
	int ret = 0;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigPreviewParam sConfig;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
	}
	
	if(0 == ret)
	{
		psPara->nMode = sConfig.nPreviewMode;
		psPara->nModePara = sConfig.nModePara;
		psPara->nAudioOutChn = sConfig.nAdioChn;
		psPara->nVolume = sConfig.nVolume;
		psPara->nMute = sConfig.nMute;
		
		SModConfigMainOsdParam sConfig2;
		ret = ModConfigGetParam(EM_CONFIG_PARA_MAIN_OSD, &sConfig2, 0);
		if(0 == ret)
		{
			psPara->nShowRecState = sConfig2.nShowRecState;
			psPara->nShowTime = sConfig2.nShowTime;
			//yaogang modify 20140918
			psPara->nShowChnKbps = sConfig2.nShowChnKbps;
			psPara->sTimePos.x = sConfig2.nTimePosX;
			psPara->sTimePos.y = sConfig2.nTimePosY;
		}
		
		memcpy(psPara->nVideoSrcType, sConfig.nVideoSrcType, sizeof(psPara->nVideoSrcType));
	}
	
	return ret;
}

s32 ConfigGetCfgPatrol(u32 bDefault,SBizCfgPatrol *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigPreviewParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
	}
	
	if (0 == ret)
	{
		psPara->nIsPatrol = sConfig.nIsPatrol;
		psPara->nInterval = sConfig.nInterval;
		psPara->nPatrolMode = sConfig.nPatrolMode;
		psPara->nStops = sConfig.nStops;
		memcpy(psPara->pnStopModePara, sConfig.pnStopModePara, psPara->nStops);
	}
	
	return ret;
}

s32 ConfigGetPreviewImagePara(u32 bDefault,SBizPreviewImagePara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigImagePara sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_IMAGE, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_IMAGE, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		psPara->nHue = sConfig.nHue;
		psPara->nSaturation = sConfig.nSaturation;
		psPara->nContrast = sConfig.nContrast;
		psPara->nBrightness = sConfig.nBrightness;
	}
	
	return ret;
}

s32 ConfigGetPlayBackImagePara(u32 bDefault,SBizPreviewImagePara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigImagePara sConfig;
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VO_IMAGE, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VO_IMAGE, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		psPara->nHue = sConfig.nHue;
		psPara->nSaturation = sConfig.nSaturation;
		psPara->nContrast = sConfig.nContrast;
		psPara->nBrightness = sConfig.nBrightness;
	}
	return ret;
}

s32 ConfigGetCfgStrOsd(u32 bDefault,SBizCfgStrOsd *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigStrOsdpara sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_STR_OSD, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_STR_OSD, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		strcpy(psPara->strChnName, sConfig.nChnName);
		psPara->nShowChnName = sConfig.nShowChnName;
		psPara->sChnNamePos.x = sConfig.nChnNamePosX;
		psPara->sChnNamePos.y = sConfig.nChnNamePosY;
		psPara->nEncShowChnName = sConfig.nEncShowChnName;
		psPara->sEncChnNamePos.x = sConfig.nEncChnNamePosX;
		psPara->sEncChnNamePos.y = sConfig.nEncChnNamePosY;
		psPara->nEncShowTime = sConfig.nEncShowTime;
		psPara->sEncTimePos.x = sConfig.nEncTimePosX;
		psPara->sEncTimePos.y = sConfig.nEncTimePosY;
	}
	
	return ret;
}

s32 ConfigGetCfgRectOsd(u32 bDefault,SBizCfgRectOsd *psPara, u8 nId)
{
	s32 ret, i;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigRectOsdpara sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_RECT_OSD, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_RECT_OSD, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nShowRectOsd = sConfig.nShowRectOsd;
		psPara->nRealNum = sConfig.nRectOsdNum;
		if (psPara->nRealNum > psPara->nMaxNum)
		{
			psPara->nRealNum = psPara->nMaxNum;
		}

		//printf("###########max = %d  , real = %d ret %d\n", psPara->nRealNum, psPara->nRealNum, ret);
	
		if (NULL == psPara->psRectOsd)
		{
			return -3;
		}
		
		for (i = 0; i < psPara->nRealNum; i++)
		{
			psPara->psRectOsd[i].x = sConfig.nRectOsdX[i];
			psPara->psRectOsd[i].y = sConfig.nRectOsdY[i];
			psPara->psRectOsd[i].w = sConfig.nRectOsdW[i];
			psPara->psRectOsd[i].h = sConfig.nRectOsdH[i];
		}
	}
	//printf("###########max = %d  , real = %d ret %d\n", psPara->nRealNum, psPara->nRealNum, ret);
	
	return ret;
}

s32 ConfigSetIsPatrolPara(SBizCfgPatrol *psPara, u8 nId)
{
	int ret = 0;
	
	if (NULL == psPara)
	{
		return -1;
	}
	
 	u8 nIsChanged = 0;
	
	SModConfigPreviewParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_PATROL, &sConfig, nId);
	
	if (sConfig.nIsPatrol != psPara->nIsPatrol)
	{
		sConfig.nIsPatrol = psPara->nIsPatrol;
		nIsChanged += 1;
	}
	
	if (nIsChanged)
	{
		ret = ModConfigSetParam(EM_CONFIG_PARA_PATROL, &sConfig, nId);
	}
	
	return ret;//csp modify
}

s32 ConfigGetIsPatrolPara(u32 bDefault,SBizCfgPatrol *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigPreviewParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_PATROL, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_PATROL, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nIsPatrol = sConfig.nIsPatrol;
	}
	
	return ret;
}

s32 ConfigGetVMainEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVideoParam sConfig;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_ENC_MAIN, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_ENC_MAIN, &sConfig, nId);
	}
	
	if(0 == ret)
	{
		psPara->nBitRate = sConfig.nBitRate;
		psPara->nFrameRate = sConfig.nFrameRate;
		psPara->nGop = sConfig.nGop;
		psPara->nMinQP = sConfig.nMinQP;
		psPara->nMaxQP = sConfig.nMaxQP;
		psPara->nVideoResolution = sConfig.nResolution;
		psPara->nEncodeType = sConfig.nEncodeType;
		psPara->nBitRateType = sConfig.nBitRateType;
		psPara->nPicLevel = sConfig.nPicLevel;
	}
	
	return ret;
}

s32 ConfigGetVMobEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVideoParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_ENC_MOB, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_ENC_MOB, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nBitRate = sConfig.nBitRate;
		psPara->nFrameRate = sConfig.nFrameRate;
		psPara->nGop = sConfig.nGop;
		psPara->nMinQP = sConfig.nMinQP;
		psPara->nMaxQP = sConfig.nMaxQP;
		psPara->nVideoResolution = sConfig.nResolution;
		psPara->nEncodeType = sConfig.nEncodeType;
		psPara->nBitRateType = sConfig.nBitRateType;
		psPara->nPicLevel = sConfig.nPicLevel;
	}
	
	return ret;
}


s32 ConfigGetVSubEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVideoParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_ENC_SUB, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_ENC_SUB, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nBitRate = sConfig.nBitRate;
		psPara->nFrameRate = sConfig.nFrameRate;
		psPara->nGop = sConfig.nGop;
		psPara->nMinQP = sConfig.nMinQP;
		psPara->nMaxQP = sConfig.nMaxQP;
		psPara->nVideoResolution = sConfig.nResolution;
		psPara->nEncodeType = sConfig.nEncodeType;
		psPara->nBitRateType = sConfig.nBitRateType;
		psPara->nPicLevel = sConfig.nPicLevel;
	}
	
	return ret;
}

s32 ConfigGetRecPara(u32 bDefault,SBizRecPara *psPara, u8 nId)
{
	int ret;
	
	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigRecordParam sConfig;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_RECORD, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_RECORD, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		psPara->bRecording = sConfig.bRecording;
		//printf("11111  psPara->bRecording = %d \n",psPara->bRecording);
		psPara->nEnable = sConfig.nEnable;
		psPara->nStreamType = sConfig.nStreamType;
		psPara->nEncChn = sConfig.nEncChn;
		psPara->nEncChnType = sConfig.nChnEncType;
		psPara->nPreTime = sConfig.nPreTime;
		psPara->nRecDelay = sConfig.nDelayTime;
		psPara->nRecExpire = sConfig.nExpireDays;
		psPara->nSchRecSchtype = sConfig.nTimerRecSchType;
		psPara->nMDRecSchtype = sConfig.nMDRecSchType;
		psPara->nSensorRecSchtype = sConfig.nSensorRecSchType;
	}
	
	return ret;
}

s32 ConfigGetAlarmSensorPara(u32 bDefault,SBizAlarmSensorPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SENSOR, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SENSOR, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nType = sConfig.nSensorType;
		psPara->nDelay = sConfig.nDelay;
		strcpy(psPara->name, sConfig.name);
		psPara->nSchType = sConfig.nSchType;
	}
	
	return ret;
}
//yaogang modify 20141010
s32 ConfigGetAlarmIPCCoverPara(u32 bDefault, SBizAlarmIPCExtSensorPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	//if (nId == 0)
		//printf("yg ConfigGetAlarmIPCExtSensorPara chn0\n");

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_IPCCOVER, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_IPCCOVER, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nDelay = sConfig.nDelay;
	}
	
	return ret;
}


s32 ConfigGetAlarmIPCExtSensorPara(u32 bDefault, SBizAlarmIPCExtSensorPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	//if (nId == 0)
		//printf("yg ConfigGetAlarmIPCExtSensorPara chn0\n");

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_IPCEXTSENSOR, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_IPCEXTSENSOR, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nDelay = sConfig.nDelay;
	}
	
	return ret;
}
s32 ConfigGetAlarm485ExtSensorPara(u32 bDefault, SBizAlarmIPCExtSensorPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	//if (nId == 0)
		//printf("yg ConfigGetAlarmIPCExtSensorPara chn0\n");

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_485EXTSENSOR, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_485EXTSENSOR, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nDelay = sConfig.nDelay;
	}
	
	return ret;
}

s32 ConfigGetAlarmHDDPara(u32 bDefault, SBizAlarmHDDPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigHDDParam sConfig;
	//if (nId == 0)
		//printf("yg ConfigGetAlarmIPCExtSensorPara chn0\n");

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_HDD, &sConfig, nId);
	}
	else
	{
		
		ret = ModConfigGetParam(EM_CONFIG_PARA_HDD, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		//psPara->nDelay = sConfig.nDelay;
	}
	
	return ret;
}


s32 ConfigGetAlarmVMotionPara(u32 bDefault,SBizAlarmVMotionPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVMotionParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VMOTION, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VMOTION, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nSensitivity = sConfig.nSensitivity;
		psPara->nDelay = sConfig.nDelay;
		psPara->nSchType = sConfig.nSchType;
		memcpy(psPara->nBlockStatus, sConfig.nBlockStatus, sizeof(psPara->nBlockStatus));
	}
	
	return ret;
}

s32 ConfigGetAlarmVLostPara(u32 bDefault,SBizAlarmVLostPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVLostParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VLOST, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VLOST, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nDelay = sConfig.nDelay;
		psPara->nSchType = sConfig.nSchType;
	}
	
	return ret;
}

s32 ConfigGetAlarmVBlindPara(u32 bDefault,SBizAlarmVBlindPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVBlindParam sConfig;

	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VBLIND, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VBLIND, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nDelay = sConfig.nDelay;
		psPara->nSchType = sConfig.nSchType;
	}
	
	return ret;
}

s32 ConfigGetAlarmOutPara(u32 bDefault,SBizAlarmOutPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigAlarmOutParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_ALARMOUT, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_ALARMOUT, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nType = sConfig.nAlarmOutType;
		psPara->nDelay = sConfig.nDelay;
		strcpy(psPara->name, sConfig.name);
		psPara->nSchType = sConfig.nSchType;
	}
	
	return ret;
}

s32 ConfigGetAlarmBuzzPara(u32 bDefault,SBizAlarmBuzzPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigBuzzParam sConfig;

	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_BUZZ, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_BUZZ, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nEnable = sConfig.nEnalbe;
		psPara->nDelay = sConfig.nDelay;
	}
	
	return ret;
}

//一般报警事件触发参数设置
s32 ConfigGetAlarmSensorDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorParam sConfig;

	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SENSOR, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SENSOR, &sConfig, nId);
	}


	if (0 == ret)
	{
		psPara->nFlagBuzz = sConfig.psDispatch.nFlagBuzz;
		psPara->nZoomChn = sConfig.psDispatch.nZoomChn;
		psPara->nFlagEmail = sConfig.psDispatch.nFlagEmail;

		int i;
		for (i = 0; i < sizeof(psPara->nSnapChn); i++)
		{
			if (sConfig.psDispatch.nSnapChn & (1 <<i))
			{
				psPara->nSnapChn[i] = i;
			}
			else
			{
				psPara->nSnapChn[i] = 0xff;
			}

			if (sConfig.psDispatch.nAlarmOut & (1 <<i))
			{
				psPara->nAlarmOut[i] = i;
			}
			else
			{
				psPara->nAlarmOut[i] = 0xff;
			}
			if (sConfig.psDispatch.nRecordChn & (1 <<i))
			{
				psPara->nRecordChn[i] = i;
			}
			else
			{
				psPara->nRecordChn[i] = 0xff;
			}
		}
		memcpy(psPara->sAlarmPtz, sConfig.psDispatch.sAlarmPtz, sizeof(psPara->sAlarmPtz));
	}
	
	return ret;
}

//yaogang modify 20141010
s32 ConfigGetAlarmIPCCoverDispatch(u32 bDefault, SBizAlarmDispatch * psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;

	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_IPCCOVER, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_IPCCOVER, &sConfig, nId);
	}


	if (0 == ret)
	{
		psPara->nFlagBuzz = sConfig.psDispatch.nFlagBuzz;
		psPara->nZoomChn = sConfig.psDispatch.nZoomChn;
		psPara->nFlagEmail = sConfig.psDispatch.nFlagEmail;

		int i;
		for (i = 0; i < sizeof(psPara->nSnapChn); i++)
		{
			if (sConfig.psDispatch.nSnapChn & (1 <<i))
			{
				psPara->nSnapChn[i] = i;
			}
			else
			{
				psPara->nSnapChn[i] = 0xff;
			}

			if (sConfig.psDispatch.nAlarmOut & (1 <<i))
			{
				psPara->nAlarmOut[i] = i;
			}
			else
			{
				psPara->nAlarmOut[i] = 0xff;
			}
			if (sConfig.psDispatch.nRecordChn & (1 <<i))
			{
				psPara->nRecordChn[i] = i;
			}
			else
			{
				psPara->nRecordChn[i] = 0xff;
			}
		}
		memcpy(psPara->sAlarmPtz, sConfig.psDispatch.sAlarmPtz, sizeof(psPara->sAlarmPtz));
	}
	
	return ret;
}

s32 ConfigGetAlarmIPCExtSensorDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;

	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_IPCEXTSENSOR, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_IPCEXTSENSOR, &sConfig, nId);
	}


	if (0 == ret)
	{
		psPara->nFlagBuzz = sConfig.psDispatch.nFlagBuzz;
		psPara->nZoomChn = sConfig.psDispatch.nZoomChn;
		psPara->nFlagEmail = sConfig.psDispatch.nFlagEmail;

		int i;
		for (i = 0; i < sizeof(psPara->nSnapChn); i++)
		{
			if (sConfig.psDispatch.nSnapChn & (1 <<i))
			{
				psPara->nSnapChn[i] = i;
			}
			else
			{
				psPara->nSnapChn[i] = 0xff;
			}

			if (sConfig.psDispatch.nAlarmOut & (1 <<i))
			{
				psPara->nAlarmOut[i] = i;
			}
			else
			{
				psPara->nAlarmOut[i] = 0xff;
			}
			if (sConfig.psDispatch.nRecordChn & (1 <<i))
			{
				psPara->nRecordChn[i] = i;
			}
			else
			{
				psPara->nRecordChn[i] = 0xff;
			}
		}
		memcpy(psPara->sAlarmPtz, sConfig.psDispatch.sAlarmPtz, sizeof(psPara->sAlarmPtz));
	}
	
	return ret;
}
s32 ConfigGetAlarm485ExtSensorDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;

	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_485EXTSENSOR, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_485EXTSENSOR, &sConfig, nId);
	}


	if (0 == ret)
	{
		psPara->nFlagBuzz = sConfig.psDispatch.nFlagBuzz;
		psPara->nZoomChn = sConfig.psDispatch.nZoomChn;
		psPara->nFlagEmail = sConfig.psDispatch.nFlagEmail;

		int i;
		for (i = 0; i < sizeof(psPara->nSnapChn); i++)
		{
			if (sConfig.psDispatch.nSnapChn & (1 <<i))
			{
				psPara->nSnapChn[i] = i;
			}
			else
			{
				psPara->nSnapChn[i] = 0xff;
			}

			if (sConfig.psDispatch.nAlarmOut & (1 <<i))
			{
				psPara->nAlarmOut[i] = i;
			}
			else
			{
				psPara->nAlarmOut[i] = 0xff;
			}
			if (sConfig.psDispatch.nRecordChn & (1 <<i))
			{
				psPara->nRecordChn[i] = i;
			}
			else
			{
				psPara->nRecordChn[i] = 0xff;
			}
		}
		memcpy(psPara->sAlarmPtz, sConfig.psDispatch.sAlarmPtz, sizeof(psPara->sAlarmPtz));
	}
	
	return ret;
}
s32 ConfigGetAlarmHDDDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigHDDParam sConfig;

	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_HDD, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_HDD, &sConfig, nId);
	}


	if (0 == ret)
	{
		psPara->nFlagBuzz = sConfig.psDispatch.nFlagBuzz;
		psPara->nZoomChn = sConfig.psDispatch.nZoomChn;
		psPara->nFlagEmail = sConfig.psDispatch.nFlagEmail;

		int i;
		for (i = 0; i < sizeof(psPara->nSnapChn); i++)
		{
			if (sConfig.psDispatch.nSnapChn & (1 <<i))
			{
				psPara->nSnapChn[i] = i;
			}
			else
			{
				psPara->nSnapChn[i] = 0xff;
			}

			if (sConfig.psDispatch.nAlarmOut & (1 <<i))
			{
				psPara->nAlarmOut[i] = i;
			}
			else
			{
				psPara->nAlarmOut[i] = 0xff;
			}
			if (sConfig.psDispatch.nRecordChn & (1 <<i))
			{
				psPara->nRecordChn[i] = i;
			}
			else
			{
				psPara->nRecordChn[i] = 0xff;
			}
		}
		memcpy(psPara->sAlarmPtz, sConfig.psDispatch.sAlarmPtz, sizeof(psPara->sAlarmPtz));
	}
	
	return ret;
}


s32 ConfigGetAlarmVMotionDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVMotionParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VMOTION, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VMOTION, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nFlagBuzz = sConfig.psDispatch.nFlagBuzz;
		psPara->nZoomChn = sConfig.psDispatch.nZoomChn;
		psPara->nFlagEmail = sConfig.psDispatch.nFlagEmail;
		int i;
		for (i = 0; i < sizeof(psPara->nSnapChn); i++)
		{
			if (sConfig.psDispatch.nSnapChn & (1 <<i))
			{
				psPara->nSnapChn[i] = i;
			}
			else
			{
				psPara->nSnapChn[i] = 0xff;
			}

			if (sConfig.psDispatch.nAlarmOut & (1 <<i))
			{
				psPara->nAlarmOut[i] = i;
			}
			else
			{
				psPara->nAlarmOut[i] = 0xff;
			}
			if (sConfig.psDispatch.nRecordChn & (1 <<i))
			{
				psPara->nRecordChn[i] = i;
			}
			else
			{
				psPara->nRecordChn[i] = 0xff;
			}
		}
		memcpy(psPara->sAlarmPtz, sConfig.psDispatch.sAlarmPtz, sizeof(psPara->sAlarmPtz));
	}
	
	return ret;
}

s32 ConfigGetAlarmVBlindDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVBlindParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VBLIND, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VBLIND, &sConfig, nId);
	}
	

	if (0 == ret)
	{
		psPara->nFlagBuzz = sConfig.psDispatch.nFlagBuzz;
		psPara->nZoomChn = sConfig.psDispatch.nZoomChn;
		psPara->nFlagEmail = sConfig.psDispatch.nFlagEmail;
		int i;
		for (i = 0; i < sizeof(psPara->nSnapChn); i++)
		{
			if (sConfig.psDispatch.nSnapChn & (1 <<i))
			{
				psPara->nSnapChn[i] = i;
			}
			else
			{
				psPara->nSnapChn[i] = 0xff;
			}

			if (sConfig.psDispatch.nAlarmOut & (1 <<i))
			{
				psPara->nAlarmOut[i] = i;
			}
			else
			{
				psPara->nAlarmOut[i] = 0xff;
			}
			if (sConfig.psDispatch.nRecordChn & (1 <<i))
			{
				psPara->nRecordChn[i] = i;
			}
			else
			{
				psPara->nRecordChn[i] = 0xff;
			}
		}
		memcpy(psPara->sAlarmPtz, sConfig.psDispatch.sAlarmPtz, sizeof(psPara->sAlarmPtz));
	}
	
	return ret;
}

s32 ConfigGetAlarmVLostDispatch(u32 bDefault,SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVLostParam sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VLOST, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VLOST, &sConfig, nId);
	}

	if (0 == ret)
	{
		psPara->nFlagBuzz = sConfig.psDispatch.nFlagBuzz;
		psPara->nZoomChn = sConfig.psDispatch.nZoomChn;
		psPara->nFlagEmail = sConfig.psDispatch.nFlagEmail;
		int i;
		for (i = 0; i < sizeof(psPara->nSnapChn); i++)
		{
			if (sConfig.psDispatch.nSnapChn & (1 <<i))
			{
				psPara->nSnapChn[i] = i;
			}
			else
			{
				psPara->nSnapChn[i] = 0xff;
			}

			if (sConfig.psDispatch.nAlarmOut & (1 <<i))
			{
				psPara->nAlarmOut[i] = i;
			}
			else
			{
				psPara->nAlarmOut[i] = 0xff;
			}
			if (sConfig.psDispatch.nRecordChn & (1 <<i))
			{
				psPara->nRecordChn[i] = i;
			}
			else
			{
				psPara->nRecordChn[i] = 0xff;
			}
		}
		memcpy(psPara->sAlarmPtz, sConfig.psDispatch.sAlarmPtz, sizeof(psPara->sAlarmPtz));
	}
	
	return ret;
}

//一般报警事件布防设置
s32 ConfigGetAlarmSensorSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorSch sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SENSOR_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SENSOR_SCH, &sConfig, nId);
	}

	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psSensorSch, sizeof(SBizAlarmSchedule));
	}
	
	return ret;
}

s32 ConfigGetAlarmIPCExtSensorSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorSch sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_IPCEXTSENSOR_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_IPCEXTSENSOR_SCH, &sConfig, nId);
	}

	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psSensorSch, sizeof(SBizAlarmSchedule));
	}
	
	return ret;
}


s32 ConfigGetAlarmVMotionSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVMotionSch sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VMOTION_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VMOTION_SCH, &sConfig, nId);
	}

	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psVMSch, sizeof(SBizAlarmSchedule));
	}
	
	return ret;
}

s32 ConfigGetAlarmVBlindSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVBlindSch sConfig;

	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VBLIND_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VBLIND_SCH, &sConfig, nId);
	}

	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psVBlindSch, sizeof(SBizAlarmSchedule));
	}
	
	return ret;
}

s32 ConfigGetAlarmVLostSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId)
{
	int ret = 0;
	
	if (NULL == psPara)
	{
		return -1;
	}
	
	#if 0
	SModConfigVLostSch sConfig;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VLOST_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VLOST_SCH, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psVLostSch, sizeof(SBizAlarmSchedule));
	}
	#else
	//强制全部布防
	memset(psPara, 0, sizeof(SBizAlarmSchedule));
	psPara->nSchType = 0;
	int i = 0;
	for(i = 0; i < 7; i++)
	{
		psPara->sSchTime[i][0].nStartTime = 0;
		psPara->sSchTime[i][0].nStopTime = 3600 * 24 - 1;
	}
	return 0;
	//end
	#endif
	
	return ret;
}

//报警输出事件布防设置
s32 ConfigGetAlarmOutSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigAlarmOutSch sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_ALARMOUT_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_ALARMOUT_SCH, &sConfig, nId);
	}

	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psAlarmOutSch, sizeof(SBizAlarmSchedule));
	}
	
	return ret;
}

s32 ConfigGetAlarmBuzzSchedule(u32 bDefault,SBizAlarmSchedule *psPara, u8 nId)
{
	return 0;
}

//录像布防
s32 ConfigGetSchRecSchedule(u32 bDefault,SBizSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigRecTimerSch sConfig;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_TIMER_REC_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_TIMER_REC_SCH, &sConfig, nId);
	}

	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psRecSch, sizeof(SBizSchedule));

		/*
		int i, j;
		for(i=0; i<7; i++)
		{
			for(j=0; j<12; j++)
			{
				printf("sch %d %d get \n",
					psPara->sSchTime[i][j].nStartTime, psPara->sSchTime[i][j].nStopTime,
					sConfig.psRecSch.nSchTime[i][j].nStartTime, sConfig.psRecSch.nSchTime[i][j].nStopTime
				);
			}
		}
		*/
	}
	
	return ret;
}

s32 ConfigGetVMotionAlaRecSchedule(u32 bDefault,SBizSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigRecVMSch sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_VMOTION_REC_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_VMOTION_REC_SCH, &sConfig, nId);
	}

	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psRecSch, sizeof(SBizSchedule));
/*		int i = 0 ;
		while(i++ <7)
		{
			printf("start%d:%d, stop%d:%d\n", i,psPara->sSchTime[i-1][0].nStartTime,i,psPara->sSchTime[i-1][0].nStopTime);
		}
*/	}
	
	return ret;
}

s32 ConfigGetSensorAlaRecSchedule(u32 bDefault,SBizSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigRecAlarmInSch sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SENSOR_REC_SCH, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SENSOR_REC_SCH, &sConfig, nId);
	}

	if (0 == ret)
	{
		memcpy(psPara, &sConfig.psRecSch, sizeof(SBizSchedule));
	}
	
	return ret;
}

//云台参数
s32 ConfigGetPtzPara(u32 bDefault,SBizPtzPara *psPara, u8 nId)
{
	int ret;
	SBizManager* psBizManager = &g_sBizManager;
	
	if(NULL == psPara && psBizManager->pProtocolList)
	{
		return -1;
	}
	
	//printf("ptz nId=%d\n", nId);
	
	SModConfigPtzParam sConfig;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_PTZ, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_PTZ, &sConfig, nId);
	}
	
	if(0 == ret)
	{
		int i, j, k;
		
		psPara->nEnable = sConfig.nEnable;
		psPara->nCamAddr = sConfig.nCamAddr;
		psPara->nDataBit = sConfig.nDataBit;
		psPara->nStopBit = sConfig.nStopBit;
		psPara->nBaudRate = sConfig.nBaudRate;
		psPara->nCheckType = sConfig.nCheckType;
		psPara->nFlowCtrlType = sConfig.nFlowCtrlType;
		psPara->nProtocol = 0;
		for(i = 0; i < psBizManager->nPtzProtocolNum; i++)
		{
			if(0 == strcasecmp(psBizManager->pProtocolList[i], sConfig.nProtocolName))
			{
				psPara->nProtocol = i;
				break;
			}
		}
		
		for(i = 0; i <sizeof(psPara->sAdvancedPara.nIsPresetSet); i++)
		{
			j = i / 64;
			k = i % 64;
			//printf("***pos:[%d]\n", psPara->sAdvancedPara.nIsPresetSet[i]);
			psPara->sAdvancedPara.nIsPresetSet[i] = ((sConfig.sTourPresetPara.nPresetId[j] >> k) & 1);
			//printf("***posID:%d = [%d]\n",i, psPara->sAdvancedPara.nIsPresetSet[i]);
		}

		for(i = 0; i <sizeof(psPara->sAdvancedPara.sTourPath) / sizeof(SBizTourPath); i++)
		{
			j = i / 64;
			k = i % 64;
			
			if((sConfig.sTourPresetPara.nTourId[j] >> k) & 1)
			{
				psPara->sAdvancedPara.sTourPath[i].nPathNo = i+1;
				for(j = 0; j < sizeof(psPara->sAdvancedPara.sTourPath[i].sTourPoint) / sizeof(SBizTourPoint); j++)
				{
					psPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nPresetPos = sConfig.sTourPresetPara.nTourPresetId[i][j].nPresetId;
					psPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nDwellTime = sConfig.sTourPresetPara.nTourPresetId[i][j].nStayTime;
					psPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nSpeed = sConfig.sTourPresetPara.nTourPresetId[i][j].nSpeed;
				}
			}
		}
	}
	
	return ret;
}

//用户
s32 ConfigGetUserPara(u32 bDefault,SBizUserPara *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigUserPara sConfig;

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_USER, &sConfig, 0);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_USER, &sConfig, 0);
	}
	
	if (0 == ret)
	{
		psPara->nFlagGroupSetted = sConfig.nFlagGroupSetted;
		psPara->nFlagUserSetted[0] = sConfig.nFlagUserSetted[0];
		psPara->nFlagUserSetted[1] = sConfig.nFlagUserSetted[1];

		int i;
		for(i = 0; i < MOD_BIZ_MAX_USER_NUM; i++)
		{
			if((sConfig.nFlagUserSetted[i/64] >> (i%64)) & 1)
			{
				strcpy(psPara->sUserPara[i].UserName, sConfig.sUserPara[i].UserName);
				strcpy(psPara->sUserPara[i].Password, sConfig.sUserPara[i].Password);
				strcpy(psPara->sUserPara[i].GroupBelong, sConfig.sUserPara[i].GroupBelong);
				strcpy(psPara->sUserPara[i].UserDescript, sConfig.sUserPara[i].UserDescript);
				psPara->sUserPara[i].emLoginTwince = sConfig.sUserPara[i].emLoginTwince;
				psPara->sUserPara[i].emIsBindPcMac = sConfig.sUserPara[i].emIsBindPcMac;
				psPara->sUserPara[i].PcMacAddress = sConfig.sUserPara[i].PcMacAddress;
				int j;
				for(j = 0; j < sizeof(sConfig.sUserPara[i].UserAuthor.nAuthor); j++)
				{
					psPara->sUserPara[i].UserAuthor.nAuthor[j] = sConfig.sUserPara[i].UserAuthor.nAuthor[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nLiveView[j] = sConfig.sUserPara[i].UserAuthor.nLiveView[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nRemoteView[j] = sConfig.sUserPara[i].UserAuthor.nRemoteView[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nRecord[j] = sConfig.sUserPara[i].UserAuthor.nRecord[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nPlayback[j] = sConfig.sUserPara[i].UserAuthor.nPlayback[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nBackUp[j] = sConfig.sUserPara[i].UserAuthor.nBackUp[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sUserPara[i].UserAuthor.nPtzCtrl[j] = sConfig.sUserPara[i].UserAuthor.nPtzCtrl[j];
				}				
			}
		}
		
		for(i = 0; i < MOD_BIZ_MAX_GROUP_NUM; i++)
		{
			if((sConfig.nFlagGroupSetted >> i) & 1)
			{
				strcpy(psPara->sGroupPara[i].GroupName, sConfig.sGroupPara[i].GroupName);
				strcpy(psPara->sGroupPara[i].GroupDescript, sConfig.sGroupPara[i].GroupDescript);
				psPara->sGroupPara[i].nDoWithUser = sConfig.sGroupPara[i].nDoWithUser;
				int j;
				for(j = 0; j < sizeof(sConfig.sGroupPara[i].GroupAuthor.nAuthor); j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nAuthor[j] = sConfig.sGroupPara[i].GroupAuthor.nAuthor[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nLiveView[j] = sConfig.sGroupPara[i].GroupAuthor.nLiveView[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nRemoteView[j] = sConfig.sGroupPara[i].GroupAuthor.nRemoteView[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nRecord[j] = sConfig.sGroupPara[i].GroupAuthor.nRecord[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nPlayback[j] = sConfig.sGroupPara[i].GroupAuthor.nPlayback[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nBackUp[j] = sConfig.sGroupPara[i].GroupAuthor.nBackUp[j];
				}
				for(j = 0; j < 4; j++)
				{
					psPara->sGroupPara[i].GroupAuthor.nPtzCtrl[j] = sConfig.sGroupPara[i].GroupAuthor.nPtzCtrl[j];
				}				
			}
		}		
	}
	
	return ret;
}

#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <errno.h>

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
#else
#include "mkp_vd.h"
#endif

#include "hifb.h"
#include "lib_common.h"
#include "lib_misc.h"
#include <sys/syscall.h>
#include <unistd.h>

//************************

s32 ConfigSetMainTainPara(SBizMainTainPara *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}

	SModConfigMainTainParam sConfig;
	memset(&sConfig, 0, sizeof(sConfig));
	
	sConfig.nEnable = psPara->nEnable;
	sConfig.nMainType = psPara->nMainType;
	sConfig.nSubType = psPara->nSubType;
	sConfig.ntime = psPara->ntime;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_MAINTAIN, &sConfig, 0);
	
	return ret;
}

s32 ConfigSetSystemPara(SBizSystemPara *psPara)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	// byspliang, 修改分辨率只保存配置，并不实时调整分辨率避免可能出现的osd错乱问题
	/*
	if (EM_BIZ_NTSC == psPara->nVideoStandard)
	{
		//NTSC
		switch(psPara->nOutput)
		{
			//9624 nonsupport tl_vga_resolution
			case EM_BIZ_OUTPUT_VGA_1024X768:
				tl_vga_resolution(VGA_RESOL_N_1024_60);
				break;

			case EM_BIZ_OUTPUT_VGA_1280X1024:
				tl_vga_resolution(VGA_RESOL_N_1280_60);
				break;

			default:
				tl_vga_resolution(VGA_RESOL_N_800_60);
				break;
		}
	}
	else
	{
		//PAL
		switch(psPara->nOutput)
		{
			case EM_BIZ_OUTPUT_VGA_1024X768:
				tl_vga_resolution(VGA_RESOL_P_1024_60);
				break;

			case EM_BIZ_OUTPUT_VGA_1280X1024:
				tl_vga_resolution(VGA_RESOL_P_1280_60);
				break;
			default:
				tl_vga_resolution(VGA_RESOL_P_800_60);
				break;
		}
	}
	*/
	
	SModConfigSystemParam sConfig;
	memset(&sConfig, 0, sizeof(sConfig));
	
	strcpy(sConfig.nDevName, psPara->strDevName);
	
	{
		SMultiCastInfo sInfo;
		NetComm_GetMultiCastInfo(&sInfo);
		strcpy(sInfo.device_name, sConfig.nDevName);
		NetComm_SetMultiCastInfo(&sInfo);						
	}
	
	strcpy(sConfig.nNtpServerAdress, psPara->strNtpServerAdress);//QYJ
	sConfig.nSyscCheck = psPara->nSyscCheck;//QYJ
	sConfig.nTimeZone = psPara->nTimeZone;//QYJ
	//printf("nNtpServerAdress IN ConfigSetSystemPara sConfig = %s\n",sConfig.nNtpServerAdress);//QYJ
	//printf("nSyscCheck IN ConfigSetSystemPara = %d\n",psPara->nSyscCheck);//QYJ
	
	sConfig.nSyncTimeToIPC = psPara->nSyncTimeToIPC;
	sConfig.P2PEnable = psPara->P2PEnable;
	
	sConfig.nDevId = psPara->nDevId;
	sConfig.nManual = psPara->nManual;
	sConfig.nVideoStandard = psPara->nVideoStandard;
	sConfig.nAuthCheck = psPara->nAuthCheck;
	sConfig.nOutput = psPara->nOutput;
	sConfig.nLangId = psPara->nLangId;
	sConfig.nShowGuide = psPara->nShowGuide;
	sConfig.nLockTime = psPara->nLockTime;
	sConfig.nDateFormat = psPara->nDateFormat;
	sConfig.nTimeStandard = psPara->nTimeStandard;
	sConfig.nCyclingRecord = psPara->nCyclingRecord;
	RecordSetDealHardDiskFull(sConfig.nCyclingRecord);
	//printf("设置是否循环录像: [%s]\n", sConfig.nCyclingRecord?"是":"否");
	sConfig.nShowFirstMenue = psPara->nShowFirstMenue;
	
	//csp modify
	strcpy(sConfig.sn, psPara->sn);
	strcpy(sConfig.reserved, psPara->reserved);
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_SYSTEM, &sConfig, 0);
	
	return ret;
}

s32 ConfigSetCfgPreview(SBizCfgPreview *psPara)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigPreviewParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
	sConfig.nPreviewMode = psPara->nMode;
	sConfig.nModePara = psPara->nModePara;
	sConfig.nAdioChn = psPara->nAudioOutChn;
	sConfig.nVolume = psPara->nVolume;
	sConfig.nMute = psPara->nMute;
	memcpy(sConfig.nVideoSrcType, psPara->nVideoSrcType, sizeof(sConfig.nVideoSrcType));
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
	if(0 == ret)
	{
		ModPreviewMute(sConfig.nMute);
		ModPreviewVolume(sConfig.nVolume);
		ModPreviewSetAudioOut(sConfig.nAdioChn);
		
		SModConfigMainOsdParam sConfig2;
		sConfig2.nShowRecState = psPara->nShowRecState;
		sConfig2.nShowTime = psPara->nShowTime;
		sConfig2.nTimePosX = psPara->sTimePos.x;
		sConfig2.nTimePosY = psPara->sTimePos.y;
		sConfig2.nShowChnKbps= psPara->nShowChnKbps;
		ret = ModConfigSetParam(EM_CONFIG_PARA_MAIN_OSD, &sConfig2, 0);
		
		SBizEventPara sBizEventPara;
		memset(&sBizEventPara,0,sizeof(SBizEventPara));
		
		sBizEventPara.emType = EM_BIZ_EVENT_SHOWTIME_REFRESH;
	    SendBizEvent(&sBizEventPara);
		
		///*
		int i;
		for(i = 0; i < GetChnNum(); i++)
		{
			sBizEventPara.sBizRecStatus.nChn = i;
			sBizEventPara.emType = EM_BIZ_EVENT_LIVE_REFRESH;
		    SendBizEvent(&sBizEventPara);
		}
		//*/
	}
	
	//printf("ConfigSetCfgPreview - ModPreviewStart mode=%d\n",psPara->nMode);
	
	SPreviewPara sPreview;
	sPreview.emPreviewMode = (EMPREVIEWMODE)psPara->nMode;
	sPreview.nModePara = psPara->nModePara;
	ret = ModPreviewStart(&sPreview);
	
	return ret;
}

s32 ConfigSetCfgPatrol(SBizCfgPatrol *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigPreviewParam sConfig;

	ret = ModConfigGetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
	if (ret == 0)
	{
		sConfig.nIsPatrol = psPara->nIsPatrol;
		sConfig.nInterval = psPara->nInterval;
		sConfig.nPatrolMode = psPara->nPatrolMode;
		sConfig.nStops = psPara->nStops;
		memcpy(sConfig.pnStopModePara, psPara->pnStopModePara, psPara->nStops);
		
		ret = ModConfigSetParam(EM_CONFIG_PARA_PREVIEW, &sConfig, 0);
		if (ret == 0)
		{
			SPreviewPatrolPara sPatrolPara;
			sPatrolPara.emPreviewMode = psPara->nPatrolMode;
			sPatrolPara.nInterval = psPara->nInterval;
			sPatrolPara.nIsPatrol = psPara->nIsPatrol;
			sPatrolPara.nStops = psPara->nStops;
			memcpy(sPatrolPara.pnStopModePara, psPara->pnStopModePara, psPara->nStops);
			PreviewSetPatrol(&sPatrolPara);
		}
	}
	return ret;
}

s32 ConfigSetPlayBackImagePara(SBizPreviewImagePara *psPara, u8 nId)
{
	int ret;
	
	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigImagePara sConfig;
	memset(&sConfig,0,sizeof(SModConfigImagePara));
	
	sConfig.nHue = psPara->nHue;
	sConfig.nSaturation = psPara->nSaturation;
	sConfig.nContrast = psPara->nContrast;
	sConfig.nBrightness = psPara->nBrightness;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_VO_IMAGE, &sConfig, nId);
	
	//ret = ModPreviewSetImage(nId, (SPreviewImagePara *)psPara);
	
	return ret;
}

s32 ConfigSetPreviewImagePara(SBizPreviewImagePara *psPara, u8 nId)
{
	int ret;
	
	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigImagePara sConfig;
	memset(&sConfig,0,sizeof(SModConfigImagePara));
	
	sConfig.nHue = psPara->nHue;
	sConfig.nSaturation = psPara->nSaturation;
	sConfig.nContrast = psPara->nContrast;
	sConfig.nBrightness = psPara->nBrightness;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_IMAGE, &sConfig, nId);
	
	ret = ModPreviewSetImage(nId, (SPreviewImagePara *)psPara);
	
	return ret;
}

s32 ConfigSetCfgStrOsd(SBizCfgStrOsd *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}	
	
	#if 1 //zlb20111117 去掉原先malloc
	u8 nIsChanged = 0;
	
	SModConfigStrOsdpara sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_STR_OSD, &sConfig, nId);
	
	if (strcmp(sConfig.nChnName, psPara->strChnName))
	{
		strcpy(sConfig.nChnName, psPara->strChnName);
		EncodeStrOsdUpdate(nId, (u8 *)psPara->strChnName);
		
		nIsChanged = 1;
	}
	
	if (sConfig.nShowChnName != psPara->nShowChnName)
	{
		sConfig.nShowChnName = psPara->nShowChnName;
		nIsChanged = 1;
	}
	
	if ((sConfig.nChnNamePosX != psPara->sChnNamePos.x) || (sConfig.nChnNamePosY != psPara->sChnNamePos.y))
	{
		if (sConfig.nChnNamePosX != psPara->sChnNamePos.x)
		{
			sConfig.nChnNamePosX = psPara->sChnNamePos.x;
			nIsChanged = 1;
		}

		if (sConfig.nChnNamePosY != psPara->sChnNamePos.y)
		{
			sConfig.nChnNamePosY = psPara->sChnNamePos.y;
			nIsChanged = 1;
		}
		//EncodeChangeOsdPosition(nId, EM_ENCODE_OSD_CHNNAME, psPara->sEncChnNamePos.x, psPara->sEncChnNamePos.y);
	}

	if ((sConfig.nEncChnNamePosX != psPara->sEncChnNamePos.x) || (sConfig.nEncChnNamePosY != psPara->sEncChnNamePos.y))
	{
		u16 tmpX = sConfig.nEncChnNamePosX;
		u16 tmpY = sConfig.nEncChnNamePosY;
		if (sConfig.nEncChnNamePosX != psPara->sEncChnNamePos.x)
		{
			sConfig.nEncChnNamePosX = psPara->sEncChnNamePos.x;
			//nIsChanged = 1;
		}

		if (sConfig.nEncChnNamePosY != psPara->sEncChnNamePos.y)
		{
			sConfig.nEncChnNamePosY = psPara->sEncChnNamePos.y;
			//nIsChanged = 1;
		}
		s32 ret = EncodeChangeOsdPosition(nId, EM_BIZ_ENCODE_OSD_CHNNAME, psPara->sEncChnNamePos.x, psPara->sEncChnNamePos.y);
		if(0 != ret)
		{
			sConfig.nEncChnNamePosX = tmpX;
			sConfig.nEncChnNamePosY = tmpY;
		}
	}
	if (sConfig.nEncShowChnName != psPara->nEncShowChnName)
	{
		sConfig.nEncShowChnName = psPara->nEncShowChnName;
		EncodeOsdShow(nId, EM_BIZ_ENCODE_OSD_CHNNAME, psPara->nEncShowChnName);
		//nIsChanged = 1;
	}
	if(sConfig.nEncShowChnName) //通道名为空时会存在黑色背景
	{
		if(0 == strlen(sConfig.nChnName))
		{
			EncodeOsdShow(nId, EM_BIZ_ENCODE_OSD_CHNNAME, 0);
		}
		else
		{
			EncodeOsdShow(nId, EM_BIZ_ENCODE_OSD_CHNNAME, 1);
		}
	}
	
	//时间和通道名设在同一行会出问题 yzw
	psPara->sEncTimePos.y = ((psPara->sEncTimePos.y - psPara->sEncChnNamePos.y <= 26) && (psPara->sEncChnNamePos.y - psPara->sEncTimePos.y <= 26)) ? ((psPara->sEncTimePos.y >= psPara->sEncChnNamePos.y) ? ((psPara->sEncTimePos.y > 238) ? (psPara->sEncChnNamePos.y - 26) : (psPara->sEncChnNamePos.y + 26)) : ((psPara->sEncChnNamePos.y <= 26) ? (psPara->sEncChnNamePos.y + 26) : (psPara->sEncChnNamePos.y - 26))) : (psPara->sEncTimePos.y);
	//printf("[%d,%d],[%d,%d]\n", psPara->sEncChnNamePos.x, psPara->sEncChnNamePos.y, psPara->sEncTimePos.x, psPara->sEncTimePos.y);
	//
	
	if ((sConfig.nEncTimePosX != psPara->sEncTimePos.x) || (sConfig.nEncTimePosY != psPara->sEncTimePos.y))
	{
		u16 tmpX = sConfig.nEncTimePosX;
		u16 tmpY = sConfig.nEncTimePosY;
		if (sConfig.nEncTimePosX != psPara->sEncTimePos.x)
		{
			sConfig.nEncTimePosX = psPara->sEncTimePos.x;
			//nIsChanged = 1;
		}
		
		if (sConfig.nEncTimePosY != psPara->sEncTimePos.y)
		{
			sConfig.nEncTimePosY = psPara->sEncTimePos.y;
			//nIsChanged = 1;
		}
		s32 ret = EncodeChangeOsdPosition(nId, EM_BIZ_ENCODE_OSD_TIME, psPara->sEncTimePos.x, psPara->sEncTimePos.y);
		if(0 != ret)
		{
			sConfig.nEncTimePosX = tmpX;
			sConfig.nEncTimePosY = tmpY;
		}
	}
	if (sConfig.nEncShowTime != psPara->nEncShowTime)
	{
		sConfig.nEncShowTime = psPara->nEncShowTime;
		EncodeOsdShow(nId, EM_BIZ_ENCODE_OSD_TIME, psPara->nEncShowTime);
		//nIsChanged = 1;
	}

	/*printf("ConfigSetCfgStrOsd: \n");
	int i;
	for(i=0; i<6; i++)
		printf("0x%x\n", sConfig.nChnName[i]);
	*/
	ret = ModConfigSetParam(EM_CONFIG_PARA_STR_OSD, &sConfig, nId);
	
	if (nIsChanged)
	{
		SBizEventPara sBizEventPara;		
		memset(&sBizEventPara,0,sizeof(SBizEventPara));
		sBizEventPara.sBizRecStatus.nChn = nId;
		sBizEventPara.emType = EM_BIZ_EVENT_LIVE_REFRESH;		
	    //SendBizEvent(&sBizEventPara);
	    DealCmdWithoutMsg(&sBizEventPara);
	}
	//printf("yg ConfigSetCfgStrOsd chn%d, name: %s\n", nId, sConfig.nChnName);
	IPC_CMD_SetOSD(nId, sConfig.nChnName);
	
	#else
	static u8* nLastShowChName = NULL;
	if(nLastShowChName == NULL)
	{
		nLastShowChName = malloc(sizeof(u8)*GetChnNum());
		if(nLastShowChName)
		{
			memset(nLastShowChName, 0, sizeof(u8)*GetChnNum());
		}
	}

	static u8* nLastShowTime = NULL;
	if(nLastShowTime == NULL)
	{
		nLastShowTime = malloc(sizeof(u8)*GetChnNum());
		if(nLastShowTime)
		{
			memset(nLastShowTime, 0, sizeof(u8)*GetChnNum());
		}
	}

	static u16* nLastChnX = NULL;
	if(nLastChnX == NULL)
	{
		nLastChnX = malloc(sizeof(u16)*GetChnNum());
		if(nLastChnX)
		{
			memset(nLastChnX, 0, sizeof(u16)*GetChnNum());
		}
	}

	static u16* nLastChnY = NULL;
	if(nLastChnY == NULL)
	{
		nLastChnY = malloc(sizeof(u16)*GetChnNum());
		if(nLastChnY)
		{
			memset(nLastChnY, 0, sizeof(u16)*GetChnNum());
		}
	}

	static u16* nLastTimeX = NULL;
	if(nLastTimeX == NULL)
	{
		nLastTimeX = malloc(sizeof(u16)*GetChnNum());
		if(nLastTimeX)
		{
			memset(nLastTimeX, 0, sizeof(u16)*GetChnNum());
		}
	}

	static u16* nLastTimeY = NULL;
	if(nLastTimeY == NULL)
	{
		nLastTimeY = malloc(sizeof(u16)*GetChnNum());
		if(nLastTimeY)
		{
			memset(nLastTimeY, 0, sizeof(u16)*GetChnNum());
		}
	}
		
	SModConfigStrOsdpara sConfig;
	
	strcpy(sConfig.nChnName, psPara->strChnName);
	sConfig.nShowChnName = psPara->nShowChnName;
	sConfig.nChnNamePosX = psPara->sChnNamePos.x;
	sConfig.nChnNamePosY = psPara->sChnNamePos.y;
	sConfig.nEncShowChnName = psPara->nEncShowChnName;
	sConfig.nEncChnNamePosX = psPara->sEncChnNamePos.x;
	sConfig.nEncChnNamePosY = psPara->sEncChnNamePos.y;
	sConfig.nEncShowTime = psPara->nEncShowTime;
	sConfig.nEncTimePosX = psPara->sEncTimePos.x;
	sConfig.nEncTimePosY = psPara->sEncTimePos.y;	
	ret = ModConfigSetParam(EM_CONFIG_PARA_STR_OSD, &sConfig, nId);

	//printf("file:%s, line:%d, chn[%d]\n", __FILE__, __LINE__,nId);
	EncodeStrOsdUpdate(nId,psPara->strChnName);
	//printf("file:%s, line:%d, chn[%d]\n", __FILE__, __LINE__,nId);
	
	if(nLastChnX[nId] != psPara->sEncChnNamePos.x
		|| nLastChnY[nId] != psPara->sEncChnNamePos.y)
	{
		EncodeChangeOsdPosition(nId, EM_ENCODE_OSD_CHNNAME, psPara->sEncChnNamePos.x, psPara->sEncChnNamePos.y);
		//printf("chn[%d], X:%ld, Y:%ld\n",nId, psPara->sEncChnNamePos.x,psPara->sEncChnNamePos.y);
		nLastChnX[nId] = psPara->sEncChnNamePos.x;
		nLastChnY[nId] = psPara->sEncChnNamePos.y;
	}
	//printf("file:%s, line:%d, chn[%d]\n", __FILE__, __LINE__,nId);
	
	if(nLastTimeX[nId] != psPara->sEncTimePos.x
		|| nLastTimeY[nId] != psPara->sEncTimePos.y)
	{
		EncodeChangeOsdPosition(nId, EM_ENCODE_OSD_TIME, psPara->sEncTimePos.x, psPara->sEncTimePos.y);

		nLastTimeX[nId] = psPara->sEncTimePos.x;
		nLastTimeY[nId] = psPara->sEncTimePos.y;
	}

	//printf("nEncShowChnName %d \n", psPara->nEncShowChnName);
	
	//if(nLastShowChName[nId] != psPara->nEncShowChnName)
	//{
		//show chn name or not
	//	printf("psPara->nEncShowChnName = %d \n",psPara->nEncShowChnName);
		EncodeOsdShow(nId, EM_BIZ_ENCODE_OSD_CHNNAME, psPara->nEncShowChnName);

	//	nLastShowChName[nId] = psPara->nEncShowChnName;
	//}

	//if(nLastShowTime[nId] != psPara->nEncShowTime)
	//{
		//show time or not
	//	printf("psPara->nEncShowTime = %d \n",psPara->nEncShowTime);
		EncodeOsdShow(nId, EM_BIZ_ENCODE_OSD_TIME, psPara->nEncShowTime);

	//	nLastShowTime[nId] = psPara->nEncShowTime;
	//}
	SBizEventPara sBizEventPara;
	memset(&sBizEventPara,0,sizeof(SBizEventPara));
	sBizEventPara.sBizRecStatus.nChn = nId;
	
	sBizEventPara.emType = EM_BIZ_EVENT_LIVE_REFRESH;
    SendBizEvent(&sBizEventPara);
	#endif
	
	return ret;
}

int BizGetVideoFormatConfig()//cw_mask
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		printf("GetVideoFormat\n");
	}
	
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_VIDEOSTANDARD, 
							bizSysPara.nVideoStandard, 
							&index);
	
	if(ret!=0)
	{
		printf("GetVideoFormat error\n");
	}
	
	return index;
}

s32 ConfigSetCfgRectOsd(SBizCfgRectOsd *psPara, u8 nId)
{
	s32 ret, i;
	
	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigRectOsdpara sConfig;
	
	SBizDvrInfo sDvrInfor;
	ConfigGetDvrInfo(0, &sDvrInfor);
	
	sConfig.nShowRectOsd = psPara->nShowRectOsd;
	sConfig.nRectOsdNum = psPara->nRealNum;
	
	if (NULL == psPara->psRectOsd)
	{
		return -3;
	}
	
	int Realheight=(BizGetVideoFormatConfig()?480:576);//cw_mask
	
	if((0 == strcmp("R9624T",sDvrInfor.sproductnumber))
		|| (0 == strcmp("R9632S",sDvrInfor.sproductnumber))
		 || (0 == strcmp("R9624SL",sDvrInfor.sproductnumber))
		 || (0 == strcmp("R9516S",sDvrInfor.sproductnumber))
		 || (0 == strcmp("R9608S",sDvrInfor.sproductnumber))
		 	/*|| (0 == strcmp("R9616S",sDvrInfor.sproductnumber))*/)
	{
		for (i = 0; i < psPara->nRealNum; i++)
		{
			sConfig.nRectOsdX[i] = psPara->psRectOsd[i].x /2 * 2;
			sConfig.nRectOsdY[i] = psPara->psRectOsd[i].y / 2 * 2;
			sConfig.nRectOsdW[i] = psPara->psRectOsd[i].w / 2 * 2;
			sConfig.nRectOsdH[i] = psPara->psRectOsd[i].h / 2 * 2;
			
			psPara->psRectOsd[i].x = (psPara->psRectOsd[i].x / 2);
			psPara->psRectOsd[i].y = (psPara->psRectOsd[i].y / 2);
			psPara->psRectOsd[i].w = (psPara->psRectOsd[i].w / 2);
			psPara->psRectOsd[i].h = (psPara->psRectOsd[i].h / 2);
			
			#if 0
			if ( sConfig.nRectOsdX[i] == 8)   //cw_mask
			{
				psPara->psRectOsd[i].x = 0;
				sConfig.nRectOsdX[i] = 0;
			}
			#endif
			
			if (sConfig.nRectOsdX[i] + sConfig.nRectOsdW[i] >= 688)
			{
				sConfig.nRectOsdW[i] = 704 - sConfig.nRectOsdX[i];
				psPara->psRectOsd[i].w = 352 - sConfig.nRectOsdX[i] / 2;
			}
			
			if (sConfig.nRectOsdY[i] + sConfig.nRectOsdH[i] >= Realheight-6)
			{
				sConfig.nRectOsdH[i] = Realheight - sConfig.nRectOsdY[i];
				psPara->psRectOsd[i].h = Realheight/2 - sConfig.nRectOsdY[i] / 2;
			}
			
			PreviewRectOsdShow(nId, i, psPara->nShowRectOsd, &psPara->psRectOsd[i]);
		}
	}
	else if((0 == strcasecmp("R3104HD",sDvrInfor.sproductnumber))
			|| (0 == strcasecmp("R3110HDW",sDvrInfor.sproductnumber))
			|| (0 == strcasecmp("R3106HDW",sDvrInfor.sproductnumber)))
	{
		for (i = 0; i < psPara->nRealNum; i++)
		{
			sConfig.nRectOsdX[i] = psPara->psRectOsd[i].x;
			sConfig.nRectOsdY[i] = psPara->psRectOsd[i].y;
			sConfig.nRectOsdW[i] = psPara->psRectOsd[i].w;
			sConfig.nRectOsdH[i] = psPara->psRectOsd[i].h;
			
			psPara->psRectOsd[i].x = (psPara->psRectOsd[i].x * 1920 / 704);
			psPara->psRectOsd[i].y = (psPara->psRectOsd[i].y * 1080 / Realheight);
			psPara->psRectOsd[i].w = (psPara->psRectOsd[i].w * 1920 / 704);
			psPara->psRectOsd[i].h = (psPara->psRectOsd[i].h * 1080 / Realheight);
			
			if (sConfig.nRectOsdX[i] + sConfig.nRectOsdW[i] >= 688)
			{
				sConfig.nRectOsdW[i] = 704 - sConfig.nRectOsdX[i];
				psPara->psRectOsd[i].w = 1920 - psPara->psRectOsd[i].x;
			}
			
			if (sConfig.nRectOsdY[i] + sConfig.nRectOsdH[i] >= Realheight-6)
			{
				sConfig.nRectOsdH[i] = Realheight - sConfig.nRectOsdY[i];
				psPara->psRectOsd[i].h = 1080 - psPara->psRectOsd[i].y;
			}
			
			PreviewRectOsdShow(nId, i, psPara->nShowRectOsd, &psPara->psRectOsd[i]);
		}
	}
	else
	{
		for (i = 0; i < psPara->nRealNum; i++)
		{
			sConfig.nRectOsdX[i] = psPara->psRectOsd[i].x;
			sConfig.nRectOsdY[i] = psPara->psRectOsd[i].y;
			sConfig.nRectOsdW[i] = psPara->psRectOsd[i].w;
			sConfig.nRectOsdH[i] = psPara->psRectOsd[i].h;
			if (sConfig.nRectOsdX[i] + sConfig.nRectOsdW[i] >= 688)
			{
				sConfig.nRectOsdW[i] = 704 - sConfig.nRectOsdX[i];
				psPara->psRectOsd[i].w = 704-sConfig.nRectOsdX[i];
			}
			
			if (sConfig.nRectOsdY[i] + sConfig.nRectOsdH[i] >= Realheight-6)
			{
				sConfig.nRectOsdH[i] = Realheight - sConfig.nRectOsdY[i];
				psPara->psRectOsd[i].h = Realheight - sConfig.nRectOsdY[i];
			}
			PreviewRectOsdShow(nId, i, psPara->nShowRectOsd, &psPara->psRectOsd[i]);
		}
	}	
	SRect sRect;
	memset(&sRect, 0, sizeof(sRect));
	for (;i < 10; i++)
	{
		PreviewRectOsdShow(nId, i, 0, &sRect);
	}
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_RECT_OSD, &sConfig, nId);
	
	#if 0
	SBizEventPara sBizEventPara;
	memset(&sBizEventPara,0,sizeof(SBizEventPara));
	sBizEventPara.sBizRecStatus.nChn = nId;
	
	sBizEventPara.emType = EM_BIZ_EVENT_LIVE_REFRESH;
    SendBizEvent(&sBizEventPara);
	#endif
	
	return ret;
}

s32 ConfigSetVMainEncodePara(SBizEncodePara *psPara, u8 nId)
{
	int ret = 0;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	#if 1  //zlb20111117 去掉原先malloc
 	u8 nIsChanged = 0;
	
	SModConfigVideoParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_ENC_MAIN, &sConfig, nId);
	
	if (sConfig.nBitRate != psPara->nBitRate)
	{
		sConfig.nBitRate = psPara->nBitRate;
		nIsChanged += 1;
	}

	if (sConfig.nFrameRate != psPara->nFrameRate)
	{
		sConfig.nFrameRate = psPara->nFrameRate;
		nIsChanged += 1;
	}
	
	if (sConfig.nGop != psPara->nGop)
	{
		sConfig.nGop = psPara->nGop;
		nIsChanged += 1;
	}
	
	if (sConfig.nMinQP != psPara->nMinQP)
	{
		sConfig.nMinQP = psPara->nMinQP;
		nIsChanged += 1;
	}

	if (sConfig.nMaxQP != psPara->nMaxQP)
	{
		sConfig.nMaxQP = psPara->nMaxQP;
		nIsChanged += 1;
	}
	
	#if 1//csp modify
	if(sConfig.nResolution != psPara->nVideoResolution)
	{
		sConfig.nResolution = psPara->nVideoResolution;
		nIsChanged += 100;
		
		RecordPause(nId);
		usleep(20000);
	}
	#endif
	
	if (sConfig.nEncodeType != psPara->nEncodeType)
	{
		sConfig.nEncodeType = psPara->nEncodeType;
		nIsChanged += 1;
	}

	if (sConfig.nBitRateType != psPara->nBitRateType)
	{
		sConfig.nBitRateType = psPara->nBitRateType;
		nIsChanged += 1;
	}

	if (sConfig.nPicLevel != psPara->nPicLevel)
	{
		sConfig.nPicLevel = psPara->nPicLevel;
		nIsChanged += 1;
	}
	
	if(nIsChanged)
	{
		ret = ModConfigSetParam(EM_CONFIG_PARA_ENC_MAIN, &sConfig, nId);
		
		SEncodeParam sEncPara;
		sEncPara.nForce = 0;
		sEncPara.sVideoMainParam.nBitRate = sConfig.nBitRate; 
		sEncPara.sVideoMainParam.nFrameRate = sConfig.nFrameRate; 
		sEncPara.sVideoMainParam.nGop = sConfig.nGop;
		sEncPara.sVideoMainParam.nMinQP = sConfig.nMinQP;
		sEncPara.sVideoMainParam.nMaxQP = sConfig.nMaxQP;
		sEncPara.sVideoMainParam.emVideoResolution = sConfig.nResolution;
		sEncPara.sVideoMainParam.emEncodeType = sConfig.nEncodeType;
		sEncPara.sVideoMainParam.emBitRateType = sConfig.nBitRateType;
		sEncPara.sVideoMainParam.emPicLevel = sConfig.nPicLevel;
		EMCHNTYPE emChnType = EM_CHN_VIDEO_MAIN;
		ret |= ModEncodeSetParam(nId, emChnType, &sEncPara);
		
		if(nIsChanged >= 100)
		{
			EncodeChangeOsdResolution(nId);
			//printf("waitting**********\n");
			//usleep(10000000);
			RecordResume(nId);
		}
	}
	//printf("func:%s, set ok! chn[%d]\n", __FUNCTION__, nId);
	
	#else
	static u8* nLastReso = NULL;
	if(nLastReso == NULL)
	{
		nLastReso = malloc(sizeof(u8)*GetChnNum());
		if(nLastReso == NULL)
		{
			return -1;
		}
		int i;
		SModConfigVideoParam para;
		for(i=0;i<GetChnNum();i++)
		{
			ModConfigGetParam(EM_CONFIG_PARA_ENC_MAIN, &para, i);
			nLastReso[i] = para.nResolution;
		}
	}
	
	SModConfigVideoParam sConfig;

	sConfig.nBitRate = psPara->nBitRate;
	sConfig.nFrameRate = psPara->nFrameRate;
	//sConfig.nFrameRate = 25;
	//printf("sConfig.nFrameRate = %d  \n",sConfig.nFrameRate );
	sConfig.nGop = psPara->nGop;
	sConfig.nMinQP = psPara->nMinQP;
	sConfig.nMaxQP = psPara->nMaxQP;
	sConfig.nResolution = psPara->nVideoResolution;
	sConfig.nEncodeType = psPara->nEncodeType;
	sConfig.nBitRateType = psPara->nBitRateType;
	sConfig.nPicLevel = psPara->nPicLevel;
	ret = ModConfigSetParam(EM_CONFIG_PARA_ENC_MAIN, &sConfig, nId);

	SEncodeParam sEncPara;
	sEncPara.sVideoMainParam.nBitRate = sConfig.nBitRate; 
	sEncPara.sVideoMainParam.nFrameRate = sConfig.nFrameRate; 
	sEncPara.sVideoMainParam.nGop = sConfig.nGop;
	sEncPara.sVideoMainParam.nMinQP = sConfig.nMinQP;
	sEncPara.sVideoMainParam.nMaxQP = sConfig.nMaxQP;
	sEncPara.sVideoMainParam.emVideoResolution = sConfig.nResolution;
	sEncPara.sVideoMainParam.emEncodeType = sConfig.nEncodeType;
	sEncPara.sVideoMainParam.emBitRateType = sConfig.nBitRateType;
	sEncPara.sVideoMainParam.emPicLevel = sConfig.nPicLevel;
	EMCHNTYPE emChnType = EM_CHN_VIDEO_MAIN;

	if(nLastReso[nId] != psPara->nVideoResolution)
	{
		RecordPause(nId);
		usleep(20000);
	}
	
	ret  |= ModEncodeSetParam(nId, emChnType, &sEncPara);
	
	if(nLastReso[nId] != psPara->nVideoResolution)
	{
		//encode osd position change
		EncodeChangeOsdResolution(nId);
		RecordResume(nId);				

		nLastReso[nId] = psPara->nVideoResolution;
	}
	#endif
	
	return ret;
}
//yaogang modify 20150402 mobile stream
s32 ConfigSetVMobEncodePara(SBizEncodePara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVideoParam sConfig;

	sConfig.nBitRate = psPara->nBitRate;
	sConfig.nFrameRate = psPara->nFrameRate;
	sConfig.nGop = psPara->nGop;
	sConfig.nMinQP = psPara->nMinQP;
	sConfig.nMaxQP = psPara->nMaxQP;
	sConfig.nResolution = psPara->nVideoResolution;
	sConfig.nEncodeType = psPara->nEncodeType;
	sConfig.nBitRateType = psPara->nBitRateType;
	sConfig.nPicLevel = psPara->nPicLevel;
	ret = ModConfigSetParam(EM_CONFIG_PARA_ENC_MOB, &sConfig, nId);

	SEncodeParam sEncPara;
	sEncPara.sVideoMainParam.nBitRate = sConfig.nBitRate; 
	sEncPara.sVideoMainParam.nFrameRate = sConfig.nFrameRate; 
	sEncPara.sVideoMainParam.nGop = sConfig.nGop;
	sEncPara.sVideoMainParam.nMinQP = sConfig.nMinQP;
	sEncPara.sVideoMainParam.nMaxQP = sConfig.nMaxQP;
	sEncPara.sVideoMainParam.emVideoResolution = sConfig.nResolution;
	sEncPara.sVideoMainParam.emEncodeType = sConfig.nEncodeType;
	sEncPara.sVideoMainParam.emBitRateType = sConfig.nBitRateType;
	sEncPara.sVideoMainParam.emPicLevel = sConfig.nPicLevel;
	
	////yaogang modify 20150402 mobile stream
	EMCHNTYPE emChnType = EM_CHN_VIDEO_THIRD;
	ret = ModEncodeSetParam(nId, emChnType, &sEncPara);
	
	return ret;
}


s32 ConfigSetVSubEncodePara(SBizEncodePara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVideoParam sConfig;

	sConfig.nBitRate = psPara->nBitRate;
	sConfig.nFrameRate = psPara->nFrameRate;
	sConfig.nGop = psPara->nGop;
	sConfig.nMinQP = psPara->nMinQP;
	sConfig.nMaxQP = psPara->nMaxQP;
	sConfig.nResolution = psPara->nVideoResolution;
	sConfig.nEncodeType = psPara->nEncodeType;
	sConfig.nBitRateType = psPara->nBitRateType;
	sConfig.nPicLevel = psPara->nPicLevel;
	ret = ModConfigSetParam(EM_CONFIG_PARA_ENC_SUB, &sConfig, nId);

	SEncodeParam sEncPara;
	sEncPara.sVideoMainParam.nBitRate = sConfig.nBitRate; 
	sEncPara.sVideoMainParam.nFrameRate = sConfig.nFrameRate; 
	sEncPara.sVideoMainParam.nGop = sConfig.nGop;
	sEncPara.sVideoMainParam.nMinQP = sConfig.nMinQP;
	sEncPara.sVideoMainParam.nMaxQP = sConfig.nMaxQP;
	sEncPara.sVideoMainParam.emVideoResolution = sConfig.nResolution;
	sEncPara.sVideoMainParam.emEncodeType = sConfig.nEncodeType;
	sEncPara.sVideoMainParam.emBitRateType = sConfig.nBitRateType;
	sEncPara.sVideoMainParam.emPicLevel = sConfig.nPicLevel;
	EMCHNTYPE emChnType = EM_CHN_VIDEO_SUB;
	
	ret = ModEncodeSetParam(nId, emChnType, &sEncPara);
	
	return ret;
}

s32 ConfigSetRecPara(SBizRecPara *psPara, u8 nId)
{
	int ret;
	
	if (NULL == psPara)
	{
		return -1;
	}
	
	#if 1//zlb20111117 去掉原先malloc
	u8 nIsChanged = 0;
	SModConfigRecordParam sConfig;

	ret = ModConfigGetParam(EM_CONFIG_PARA_RECORD, &sConfig, nId);
	
	//printf("#######sConfig.bRecording = %d\n",sConfig.bRecording);
	if (sConfig.nEnable != psPara->nEnable)//必须放在第一个
	{
		sConfig.nEnable = psPara->nEnable;
		if(psPara->nEnable)
		{
			RecordResume(nId);
		}
		else
		{
			RecordPause(nId);
		}
		nIsChanged = 1;
	}

	if(sConfig.nStreamType != psPara->nStreamType)
	{
		sConfig.nStreamType = psPara->nStreamType;

		if ((0 == nIsChanged) && psPara->nEnable)
		{
			RecordPause(nId);
			RecordResume(nId);
		}
	}
	sConfig.bRecording = psPara->bRecording;
	sConfig.nStreamType = psPara->nStreamType;
	sConfig.nEncChn = psPara->nEncChn;
	sConfig.nChnEncType = psPara->nEncChnType;
	sConfig.nPreTime = psPara->nPreTime;
	sConfig.nDelayTime = psPara->nRecDelay;
	sConfig.nExpireDays = psPara->nRecExpire;
	sConfig.nTimerRecSchType = psPara->nSchRecSchtype;
	sConfig.nMDRecSchType = psPara->nMDRecSchtype;
	sConfig.nSensorRecSchType = psPara->nSensorRecSchtype;
	ret = ModConfigSetParam(EM_CONFIG_PARA_RECORD, &sConfig, nId);
	
	RecordSetPreTime(nId,(u16)psPara->nPreTime);
	RecordSetDelayTime(nId,psPara->nRecDelay);
	#else
	static u8* last_nEnable = NULL;
	static u8* last_nStreamType = NULL;
	if(last_nEnable == NULL)
	{
		last_nEnable = (u8*)malloc(GetChnNum());
		if(NULL == last_nEnable)
		{
			return ret;
		}
		last_nStreamType = (u8*)malloc(GetChnNum());
		if(NULL == last_nStreamType)
		{
			free(last_nEnable);
			last_nEnable = NULL;
			return ret;
		}
		SBizRecPara para;
		u8 i;
		for(i = 0; i < GetChnNum(); i++)
		{
			ConfigGetRecPara(0,&para, i);
			last_nEnable[i] = para.nEnable;
			last_nStreamType[i] = para.nStreamType;
		}
	}
	
	SModConfigRecordParam sConfig;
	//printf("#######sConfig.bRecording = %d\n",sConfig.bRecording);
	sConfig.bRecording = psPara->bRecording;
	sConfig.nEnable = psPara->nEnable;
	sConfig.nStreamType = psPara->nStreamType;
	sConfig.nEncChn = psPara->nEncChn;
	sConfig.nChnEncType = psPara->nEncChnType;
	sConfig.nPreTime = psPara->nPreTime;
	sConfig.nDelayTime = psPara->nRecDelay;
	sConfig.nExpireDays = psPara->nRecExpire;
	sConfig.nTimerRecSchType = psPara->nSchRecSchtype;
	sConfig.nMDRecSchType = psPara->nMDRecSchtype;
	sConfig.nSensorRecSchType = psPara->nSensorRecSchtype;
	ret = ModConfigSetParam(EM_CONFIG_PARA_RECORD, &sConfig, nId);
	
	//printf("chn:%d, video enable---past:%d, now:%d\n", nId, last_nEnable[nId], psPara->nEnable);
	if(last_nEnable[nId] != psPara->nEnable)
	{
		if(psPara->nEnable)// || psPara->nStreamType)
		{
			RecordResume(nId);
		}
		else
		{
			RecordPause(nId);
		}
	}
	else if((psPara->nEnable == 1) && (last_nStreamType[nId] != psPara->nStreamType))
	{
		RecordPause(nId);
		RecordResume(nId);		
	}
	last_nEnable[nId] = psPara->nEnable;
	last_nStreamType[nId] = psPara->nStreamType;
	
	RecordSetPreTime(nId,(u16)psPara->nPreTime);
	RecordSetDelayTime(nId,psPara->nRecDelay);
	#endif
	
	return ret;
}

s32 ConfigSetAlarmSensorPara(SBizAlarmSensorPara *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_SENSOR, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nSensorType = psPara->nType;
	sConfig.nDelay = psPara->nDelay;
	strcpy(sConfig.name, psPara->name);
	sConfig.nSchType = psPara->nSchType;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_SENSOR, &sConfig, nId);
	
	SAlarmPara sAlarmPara;
	
	sAlarmPara.sAlaSensorPara.emType = psPara->nType;
	sAlarmPara.sAlaSensorPara.nEnable= psPara->nEnable;
	sAlarmPara.sAlaSensorPara.nDelay= psPara->nDelay;
	strcpy(sAlarmPara.sAlaSensorPara.name, psPara->name);
	ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_SENSOR, nId, &sAlarmPara);
	//printf("chn:%d, type:%d, enable:%d\n", nId, psPara->nType, psPara->nEnable);
	return ret;
}
//yaogang modify 20141010

s32 ConfigSetAlarmIPCCoverPara(SBizAlarmIPCExtSensorPara *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_IPCCOVER, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nDelay = psPara->nDelay;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_IPCCOVER, &sConfig, nId);

	SAlarmPara sAlarmPara;
	//yaogang modify 20141010
	//SAlarmIPCExtSensorPara sAlaIPCExtSensorPara;
	
	sAlarmPara.sAlaIPCCoverPara.nEnable= psPara->nEnable;
	sAlarmPara.sAlaIPCCoverPara.nDelay= psPara->nDelay;
	ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_IPCCOVER, nId, &sAlarmPara);
	//printf("chn:%d, type:%d, enable:%d\n", nId, psPara->nType, psPara->nEnable);
	return ret;
}
s32 ConfigSetAlarmIPCExtSensorPara(SBizAlarmIPCExtSensorPara *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_IPCEXTSENSOR, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nDelay = psPara->nDelay;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_IPCEXTSENSOR, &sConfig, nId);

	SAlarmPara sAlarmPara;
	//yaogang modify 20141010
	//SAlarmIPCExtSensorPara sAlaIPCExtSensorPara;
	
	sAlarmPara.sAlaIPCExtSensorPara.nEnable= psPara->nEnable;
	sAlarmPara.sAlaIPCExtSensorPara.nDelay= psPara->nDelay;
	ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_IPCEXTSENSOR, nId, &sAlarmPara);
	//printf("chn:%d, type:%d, enable:%d\n", nId, psPara->nType, psPara->nEnable);
	return ret;
}

s32 ConfigSetAlarm485ExtSensorPara(SBizAlarmIPCExtSensorPara *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_485EXTSENSOR, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nDelay = psPara->nDelay;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_485EXTSENSOR, &sConfig, nId);

	SAlarmPara sAlarmPara;
	//yaogang modify 20141010
	//SAlarmIPCExtSensorPara sAlaIPCExtSensorPara;
	
	sAlarmPara.sAla485ExtSensorPara.nEnable= psPara->nEnable;
	sAlarmPara.sAla485ExtSensorPara.nDelay= psPara->nDelay;
	ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_485EXTSENSOR, nId, &sAlarmPara);
	//printf("chn:%d, type:%d, enable:%d\n", nId, psPara->nType, psPara->nEnable);
	return ret;
}
s32 ConfigSetAlarmHDDPara(SBizAlarmHDDPara *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigHDDParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_HDD, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	//sConfig.nDelay = psPara->nDelay;
	printf("yg ConfigSetAlarmHDDPara enable: %d\n", sConfig.nEnalbe);
	ret = ModConfigSetParam(EM_CONFIG_PARA_HDD, &sConfig, nId);

	SAlarmPara sAlarmPara;
	//yaogang modify 20141010
	//SAlarmIPCExtSensorPara sAlaIPCExtSensorPara;
	
	sAlarmPara.sAlaHDDPara.nEnable= psPara->nEnable;
	//sAlarmPara.sAla485ExtSensorPara.nDelay= psPara->nDelay;
	
	//一个保存的参数供检测硬盘丢失的8个通道,
	//检测硬盘读写错误的8个通道,还有检测开机无硬盘的单通道
	//这三方共用
	int i;
	for (i = 0; i < MAX_HDD_NUM; i++)
	{
		ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_HDD, i, &sAlarmPara);
	}
	//printf("chn:%d, type:%d, enable:%d\n", nId, psPara->nType, psPara->nEnable);
	return ret;
}



s32 ConfigSetAlarmVMotionPara(SBizAlarmVMotionPara *psPara, u8 nId)
{
	int ret = 0;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVMotionParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_VMOTION, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nSensitivity = psPara->nSensitivity;
	sConfig.nDelay = psPara->nDelay;
	sConfig.nSchType = psPara->nSchType;
	memcpy(sConfig.nBlockStatus, psPara->nBlockStatus, sizeof(psPara->nBlockStatus));
	ret |= ModConfigSetParam(EM_CONFIG_PARA_VMOTION, &sConfig, nId);
	
	SAlarmPara sAlarmPara;
	SEncodeParam sEncPara;
	SBizDvrInfo sDvr;
	
	EMCHNTYPE emChnType = EM_CHN_VIDEO_MAIN;
	ret |= ModEncodeGetParam(nId, emChnType, &sEncPara);
	
	ConfigGetDvrInfo(0,&sDvr);
	
	sAlarmPara.sAlaVMotionPara.emSetType = EM_ALMARM_VMOTION_AREA_SET;
	sAlarmPara.sAlaVMotionPara.emResol = sEncPara.sVideoMainParam.emVideoResolution;
	memcpy(sAlarmPara.sAlaVMotionPara.nBlockStatus, sConfig.nBlockStatus, sizeof(psPara->nBlockStatus));
	sAlarmPara.sAlaVMotionPara.nCols = sDvr.nMDAreaCols;
	sAlarmPara.sAlaVMotionPara.nRows = sDvr.nMDAreaRows;
	sAlarmPara.sAlaVMotionPara.nDelay  = sConfig.nDelay;
	sAlarmPara.sAlaVMotionPara.nEnable  = sConfig.nEnalbe;
	sAlarmPara.sAlaVMotionPara.nSensitivity  = sConfig.nSensitivity;
	ret |= ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_VMOTION, nId, &sAlarmPara);
	
	if(sConfig.nEnalbe)
	{
		EncodeRequestStartWithoutRec(nId, EM_CHN_VIDEO_MAIN);
	}	
	else
	{
		EncodeRequestStopWithoutRec(nId, EM_CHN_VIDEO_MAIN);
	}
	
	return ret;
}

s32 ConfigSetAlarmVLostPara(SBizAlarmVLostPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVLostParam sConfig;

	ret = ModConfigGetParam(EM_CONFIG_PARA_VLOST, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nDelay = psPara->nDelay;
	sConfig.nSchType = psPara->nSchType;
	ret = ModConfigSetParam(EM_CONFIG_PARA_VLOST, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaVLostPara.nDelay = sConfig.nDelay;
	sAlarmPara.sAlaVLostPara.nEnable = sConfig.nEnalbe;
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_VLOST, nId, &sAlarmPara);
	
	return ret;
}

s32 ConfigSetAlarmVBlindPara(SBizAlarmVBlindPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVBlindParam sConfig;

	ret = ModConfigGetParam(EM_CONFIG_PARA_VBLIND, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nDelay = psPara->nDelay;
	sConfig.nSchType = psPara->nSchType;
	ret = ModConfigSetParam(EM_CONFIG_PARA_VBLIND, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaVBlindPara.nDelay = sConfig.nDelay;
	sAlarmPara.sAlaVBlindPara.nEnable = sConfig.nEnalbe;
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_VBLIND, nId, &sAlarmPara);
	
	return ret;
}

s32 ConfigSetAlarmOutPara(SBizAlarmOutPara *psPara, u8 nId)
{
	int ret;
	
	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigAlarmOutParam sConfig;
	
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nAlarmOutType = psPara->nType;
	sConfig.nDelay = psPara->nDelay;
	strcpy(sConfig.name, psPara->name);
	sConfig.nSchType = psPara->nSchType;
	ret = ModConfigSetParam(EM_CONFIG_PARA_ALARMOUT, &sConfig, nId);
	
	SAlarmPara sAlarmPara;
	
	//sAlarmPara.sAlaOutPara.emType = psPara->nType;
	if(psPara->nType == EM_BIZ_ALARM_NORMAL_CLOSE)
	{
		sAlarmPara.sAlaOutPara.emType = EM_ALARM_SENSOR_NORMALCLOSE;
	}
	else
	{
		sAlarmPara.sAlaOutPara.emType = EM_ALARM_SENSOR_NORMALOPEN;
	}
	strcpy(sAlarmPara.sAlaOutPara.name, psPara->name);
	sAlarmPara.sAlaOutPara.nDelay = sConfig.nDelay;
	sAlarmPara.sAlaOutPara.nEnable = sConfig.nEnalbe;
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_ALARMOUT, nId, &sAlarmPara);
	
	return ret;
}

s32 ConfigSetAlarmBuzzPara(SBizAlarmBuzzPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigBuzzParam sConfig;

	ret = ModConfigGetParam(EM_CONFIG_PARA_BUZZ, &sConfig, nId);
	sConfig.nEnalbe = psPara->nEnable;
	sConfig.nDelay = psPara->nDelay;
	ret = ModConfigSetParam(EM_CONFIG_PARA_BUZZ, &sConfig, nId);
	
	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaBuzzPara.nDelay = sConfig.nDelay;
	sAlarmPara.sAlaBuzzPara.nEnable = sConfig.nEnalbe;
	sAlarmPara.sAlaBuzzPara.nDuration = 0;//sConfig.nDuration;
	sAlarmPara.sAlaBuzzPara.nInterval = 0;//sConfig.nInterval;
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_PARA_BUZZ, nId, &sAlarmPara);
		
	return ret;
}

//一般报警事件触发参数设置
s32 ConfigSetAlarmSensorDispatch(SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_SENSOR, &sConfig, nId);
	sConfig.psDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sConfig.psDispatch.nZoomChn = psPara->nZoomChn;
	sConfig.psDispatch.nFlagEmail = psPara->nFlagEmail;
	sConfig.psDispatch.nSnapChn = 0;
	sConfig.psDispatch.nAlarmOut = 0;
	sConfig.psDispatch.nRecordChn = 0;
	int i;
	for (i = 0; i < sizeof(psPara->nSnapChn); i++)
	{
		if (psPara->nSnapChn[i] < 64)
		{
			 sConfig.psDispatch.nSnapChn |= (1 <<psPara->nSnapChn[i]);
		}

		if (psPara->nAlarmOut[i] < 64)
		{
			 sConfig.psDispatch.nAlarmOut |= (1 <<psPara->nAlarmOut[i]);
			 //printf("sConfig.psDispatch.nAlarmOut = 0x%08llu\n", sConfig.psDispatch.nAlarmOut);
		}
		
		if (psPara->nRecordChn[i] < 64)
		{
			 sConfig.psDispatch.nRecordChn |= (1 <<psPara->nRecordChn[i]);
		}
	}
	memcpy(sConfig.psDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModConfigSetParam(EM_CONFIG_PARA_SENSOR, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sAlarmPara.sAlaDispatch.nFlagEmail = psPara->nFlagEmail;
	sAlarmPara.sAlaDispatch.nZoomChn = psPara->nZoomChn;
	memcpy(sAlarmPara.sAlaDispatch.nSnapChn, psPara->nSnapChn, sizeof(psPara->nSnapChn));
	memcpy(sAlarmPara.sAlaDispatch.nAlarmOut, psPara->nAlarmOut, sizeof(psPara->nAlarmOut));
	memcpy(sAlarmPara.sAlaDispatch.nRecordChn, psPara->nRecordChn, sizeof(psPara->nRecordChn));
	memcpy(sAlarmPara.sAlaDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_DISPATCH_SENSOR, nId, &sAlarmPara);
	//printf("chn:%d, flagbuzz:%d\n", nId, psPara->nFlagBuzz);
	return ret;
}
//yaogang modify 20141010
s32 ConfigSetAlarmIPCCoverDispatch(SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_IPCCOVER, &sConfig, nId);
	sConfig.psDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sConfig.psDispatch.nZoomChn = psPara->nZoomChn;
	sConfig.psDispatch.nFlagEmail = psPara->nFlagEmail;
	sConfig.psDispatch.nSnapChn = 0;
	sConfig.psDispatch.nAlarmOut = 0;
	sConfig.psDispatch.nRecordChn = 0;
	int i;
	for (i = 0; i < sizeof(psPara->nSnapChn); i++)
	{
		if (psPara->nSnapChn[i] < 64)
		{
			 sConfig.psDispatch.nSnapChn |= (1 <<psPara->nSnapChn[i]);
		}

		if (psPara->nAlarmOut[i] < 64)
		{
			 sConfig.psDispatch.nAlarmOut |= (1 <<psPara->nAlarmOut[i]);
			 //printf("sConfig.psDispatch.nAlarmOut = 0x%08llu\n", sConfig.psDispatch.nAlarmOut);
		}
		
		if (psPara->nRecordChn[i] < 64)
		{
			 sConfig.psDispatch.nRecordChn |= (1 <<psPara->nRecordChn[i]);
		}
	}
	memcpy(sConfig.psDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModConfigSetParam(EM_CONFIG_PARA_IPCCOVER, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sAlarmPara.sAlaDispatch.nFlagEmail = psPara->nFlagEmail;
	sAlarmPara.sAlaDispatch.nZoomChn = psPara->nZoomChn;
	memcpy(sAlarmPara.sAlaDispatch.nSnapChn, psPara->nSnapChn, sizeof(psPara->nSnapChn));
	memcpy(sAlarmPara.sAlaDispatch.nAlarmOut, psPara->nAlarmOut, sizeof(psPara->nAlarmOut));
	memcpy(sAlarmPara.sAlaDispatch.nRecordChn, psPara->nRecordChn, sizeof(psPara->nRecordChn));
	memcpy(sAlarmPara.sAlaDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_DISPATCH_IPCCOVER, nId, &sAlarmPara);
	//printf("chn:%d, flagbuzz:%d\n", nId, psPara->nFlagBuzz);
	return ret;
}

s32 ConfigSetAlarmIPCExtSensorDispatch(SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_IPCEXTSENSOR, &sConfig, nId);
	sConfig.psDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sConfig.psDispatch.nZoomChn = psPara->nZoomChn;
	sConfig.psDispatch.nFlagEmail = psPara->nFlagEmail;
	sConfig.psDispatch.nSnapChn = 0;
	sConfig.psDispatch.nAlarmOut = 0;
	sConfig.psDispatch.nRecordChn = 0;
	int i;
	for (i = 0; i < sizeof(psPara->nSnapChn); i++)
	{
		if (psPara->nSnapChn[i] < 64)
		{
			 sConfig.psDispatch.nSnapChn |= (1 <<psPara->nSnapChn[i]);
		}

		if (psPara->nAlarmOut[i] < 64)
		{
			 sConfig.psDispatch.nAlarmOut |= (1 <<psPara->nAlarmOut[i]);
			 //printf("sConfig.psDispatch.nAlarmOut = 0x%08llu\n", sConfig.psDispatch.nAlarmOut);
		}
		
		if (psPara->nRecordChn[i] < 64)
		{
			 sConfig.psDispatch.nRecordChn |= (1 <<psPara->nRecordChn[i]);
		}
	}
	memcpy(sConfig.psDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModConfigSetParam(EM_CONFIG_PARA_IPCEXTSENSOR, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sAlarmPara.sAlaDispatch.nFlagEmail = psPara->nFlagEmail;
	sAlarmPara.sAlaDispatch.nZoomChn = psPara->nZoomChn;
	memcpy(sAlarmPara.sAlaDispatch.nSnapChn, psPara->nSnapChn, sizeof(psPara->nSnapChn));
	memcpy(sAlarmPara.sAlaDispatch.nAlarmOut, psPara->nAlarmOut, sizeof(psPara->nAlarmOut));
	memcpy(sAlarmPara.sAlaDispatch.nRecordChn, psPara->nRecordChn, sizeof(psPara->nRecordChn));
	memcpy(sAlarmPara.sAlaDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_DISPATCH_IPCEXTSENSOR, nId, &sAlarmPara);
	//printf("chn:%d, flagbuzz:%d\n", nId, psPara->nFlagBuzz);
	return ret;
}
s32 ConfigSetAlarm485ExtSensorDispatch(SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigIPCExtSensorParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_485EXTSENSOR, &sConfig, nId);
	sConfig.psDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sConfig.psDispatch.nZoomChn = psPara->nZoomChn;
	sConfig.psDispatch.nFlagEmail = psPara->nFlagEmail;
	sConfig.psDispatch.nSnapChn = 0;
	sConfig.psDispatch.nAlarmOut = 0;
	sConfig.psDispatch.nRecordChn = 0;
	int i;
	for (i = 0; i < sizeof(psPara->nSnapChn); i++)
	{
		if (psPara->nSnapChn[i] < 64)
		{
			 sConfig.psDispatch.nSnapChn |= (1 <<psPara->nSnapChn[i]);
		}

		if (psPara->nAlarmOut[i] < 64)
		{
			 sConfig.psDispatch.nAlarmOut |= (1 <<psPara->nAlarmOut[i]);
			 //printf("sConfig.psDispatch.nAlarmOut = 0x%08llu\n", sConfig.psDispatch.nAlarmOut);
		}
		
		if (psPara->nRecordChn[i] < 64)
		{
			 sConfig.psDispatch.nRecordChn |= (1 <<psPara->nRecordChn[i]);
		}
	}
	memcpy(sConfig.psDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModConfigSetParam(EM_CONFIG_PARA_485EXTSENSOR, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sAlarmPara.sAlaDispatch.nFlagEmail = psPara->nFlagEmail;
	sAlarmPara.sAlaDispatch.nZoomChn = psPara->nZoomChn;
	memcpy(sAlarmPara.sAlaDispatch.nSnapChn, psPara->nSnapChn, sizeof(psPara->nSnapChn));
	memcpy(sAlarmPara.sAlaDispatch.nAlarmOut, psPara->nAlarmOut, sizeof(psPara->nAlarmOut));
	memcpy(sAlarmPara.sAlaDispatch.nRecordChn, psPara->nRecordChn, sizeof(psPara->nRecordChn));
	memcpy(sAlarmPara.sAlaDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_DISPATCH_485EXTSENSOR, nId, &sAlarmPara);
	//printf("chn:%d, flagbuzz:%d\n", nId, psPara->nFlagBuzz);
	return ret;
}
s32 ConfigSetAlarmHDDDispatch(SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigHDDParam sConfig;
	
	ret = ModConfigGetParam(EM_CONFIG_PARA_HDD, &sConfig, nId);
	sConfig.psDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sConfig.psDispatch.nZoomChn = psPara->nZoomChn;
	sConfig.psDispatch.nFlagEmail = psPara->nFlagEmail;
	sConfig.psDispatch.nSnapChn = 0;
	sConfig.psDispatch.nAlarmOut = 0;
	sConfig.psDispatch.nRecordChn = 0;
	int i;
	for (i = 0; i < sizeof(psPara->nSnapChn); i++)
	{
		if (psPara->nSnapChn[i] < 64)
		{
			 sConfig.psDispatch.nSnapChn |= (1 <<psPara->nSnapChn[i]);
		}

		if (psPara->nAlarmOut[i] < 64)
		{
			 sConfig.psDispatch.nAlarmOut |= (1 <<psPara->nAlarmOut[i]);
			 //printf("sConfig.psDispatch.nAlarmOut = 0x%08llu\n", sConfig.psDispatch.nAlarmOut);
		}
		
		if (psPara->nRecordChn[i] < 64)
		{
			 sConfig.psDispatch.nRecordChn |= (1 <<psPara->nRecordChn[i]);
		}
	}
	memcpy(sConfig.psDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModConfigSetParam(EM_CONFIG_PARA_HDD, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sAlarmPara.sAlaDispatch.nFlagEmail = psPara->nFlagEmail;
	sAlarmPara.sAlaDispatch.nZoomChn = psPara->nZoomChn;
	memcpy(sAlarmPara.sAlaDispatch.nSnapChn, psPara->nSnapChn, sizeof(psPara->nSnapChn));
	memcpy(sAlarmPara.sAlaDispatch.nAlarmOut, psPara->nAlarmOut, sizeof(psPara->nAlarmOut));
	memcpy(sAlarmPara.sAlaDispatch.nRecordChn, psPara->nRecordChn, sizeof(psPara->nRecordChn));
	memcpy(sAlarmPara.sAlaDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));

	//一个保存的参数供检测硬盘丢失的8个通道,
	//检测硬盘读写错误的8个通道,还有检测开机无硬盘的单通道
	//这三方共用
	for (i = 0; i < MAX_HDD_NUM; i++)
	{
		ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_DISPATCH_HDD, i, &sAlarmPara);
	}
	//printf("chn:%d, flagbuzz:%d\n", nId, psPara->nFlagBuzz);
	return ret;
}



s32 ConfigSetAlarmVMotionDispatch(SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVMotionParam sConfig;
	ret = ModConfigGetParam(EM_CONFIG_PARA_VMOTION, &sConfig, nId);
	sConfig.psDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sConfig.psDispatch.nZoomChn = psPara->nZoomChn;
	sConfig.psDispatch.nFlagEmail = psPara->nFlagEmail;
	sConfig.psDispatch.nSnapChn = 0;
	sConfig.psDispatch.nAlarmOut = 0;
	sConfig.psDispatch.nRecordChn = 0;
	int i;
	for (i = 0; i < sizeof(psPara->nSnapChn); i++)
	{
		if (psPara->nSnapChn[i] < 64)
		{
			 sConfig.psDispatch.nSnapChn |= (1 <<psPara->nSnapChn[i]);
		}

		if (psPara->nAlarmOut[i] < 64)
		{
			 sConfig.psDispatch.nAlarmOut |= (1 <<psPara->nAlarmOut[i]);
		}
		
		if (psPara->nRecordChn[i] < 64)
		{
			 sConfig.psDispatch.nRecordChn |= (1 <<psPara->nRecordChn[i]);
		}
	}
	memcpy(sConfig.psDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModConfigSetParam(EM_CONFIG_PARA_VMOTION, &sConfig, nId);
	
	SAlarmPara sAlarmPara;
	sAlarmPara.sAlaDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sAlarmPara.sAlaDispatch.nFlagEmail = psPara->nFlagEmail;
	sAlarmPara.sAlaDispatch.nZoomChn = psPara->nZoomChn;
	memcpy(sAlarmPara.sAlaDispatch.nSnapChn, psPara->nSnapChn, sizeof(psPara->nSnapChn));
	memcpy(sAlarmPara.sAlaDispatch.nAlarmOut, psPara->nAlarmOut, sizeof(psPara->nAlarmOut));
	memcpy(sAlarmPara.sAlaDispatch.nRecordChn, psPara->nRecordChn, sizeof(psPara->nRecordChn));
	memcpy(sAlarmPara.sAlaDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_DISPATCH_VMOTION, nId, &sAlarmPara);
	
	return ret;
}

s32 ConfigSetAlarmVBlindDispatch(SBizAlarmDispatch *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVBlindParam sConfig;

	ret = ModConfigGetParam(EM_CONFIG_PARA_VBLIND, &sConfig, nId);
	sConfig.psDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sConfig.psDispatch.nZoomChn = psPara->nZoomChn;
	sConfig.psDispatch.nFlagEmail = psPara->nFlagEmail;
	sConfig.psDispatch.nSnapChn = 0;
	sConfig.psDispatch.nAlarmOut = 0;
	sConfig.psDispatch.nRecordChn = 0;
	int i;
	for (i = 0; i < sizeof(psPara->nSnapChn); i++)
	{
		if (psPara->nSnapChn[i] < 64)
		{
			 sConfig.psDispatch.nSnapChn |= (1 <<psPara->nSnapChn[i]);
		}

		if (psPara->nAlarmOut[i] < 64)
		{
			 sConfig.psDispatch.nAlarmOut |= (1 <<psPara->nAlarmOut[i]);
		}
		
		if (psPara->nRecordChn[i] < 64)
		{
			 sConfig.psDispatch.nRecordChn |= (1 <<psPara->nRecordChn[i]);
		}
	}
	memcpy(sConfig.psDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModConfigSetParam(EM_CONFIG_PARA_VBLIND, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sAlarmPara.sAlaDispatch.nFlagEmail = psPara->nFlagEmail;
	sAlarmPara.sAlaDispatch.nZoomChn = psPara->nZoomChn;
	memcpy(sAlarmPara.sAlaDispatch.nSnapChn, psPara->nSnapChn, sizeof(psPara->nSnapChn));
	memcpy(sAlarmPara.sAlaDispatch.nAlarmOut, psPara->nAlarmOut, sizeof(psPara->nAlarmOut));
	memcpy(sAlarmPara.sAlaDispatch.nRecordChn, psPara->nRecordChn, sizeof(psPara->nRecordChn));
	memcpy(sAlarmPara.sAlaDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_DISPATCH_VBLIND, nId, &sAlarmPara);

	return ret;
}

s32 ConfigSetAlarmVLostDispatch(SBizAlarmDispatch *psPara, u8 nId)
{
	int ret = 0;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVLostParam sConfig;

	
	ret |= ModConfigGetParam(EM_CONFIG_PARA_VLOST, &sConfig, nId);
	sConfig.psDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sConfig.psDispatch.nZoomChn = psPara->nZoomChn;
	sConfig.psDispatch.nFlagEmail = psPara->nFlagEmail;
	sConfig.psDispatch.nSnapChn = 0;
	sConfig.psDispatch.nAlarmOut = 0;
	sConfig.psDispatch.nRecordChn = 0;
	int i;
	for (i = 0; i < sizeof(psPara->nSnapChn); i++)
	{
		if (psPara->nSnapChn[i] < 64)
		{
			 sConfig.psDispatch.nSnapChn |= (1 <<psPara->nSnapChn[i]);
		}

		if (psPara->nAlarmOut[i] < 64)
		{
			 sConfig.psDispatch.nAlarmOut |= (1 <<psPara->nAlarmOut[i]);
		}
		
		if (psPara->nRecordChn[i] < 64)
		{
			 sConfig.psDispatch.nRecordChn |= (1 <<psPara->nRecordChn[i]);
		}
	}
	memcpy(sConfig.psDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret |= ModConfigSetParam(EM_CONFIG_PARA_VLOST, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaDispatch.nFlagBuzz = psPara->nFlagBuzz;
	sAlarmPara.sAlaDispatch.nFlagEmail = psPara->nFlagEmail;
	sAlarmPara.sAlaDispatch.nZoomChn = psPara->nZoomChn;
	memcpy(sAlarmPara.sAlaDispatch.nSnapChn, psPara->nSnapChn, sizeof(psPara->nSnapChn));
	memcpy(sAlarmPara.sAlaDispatch.nAlarmOut, psPara->nAlarmOut, sizeof(psPara->nAlarmOut));
	memcpy(sAlarmPara.sAlaDispatch.nRecordChn, psPara->nRecordChn, sizeof(psPara->nRecordChn));
	memcpy(sAlarmPara.sAlaDispatch.sAlarmPtz, psPara->sAlarmPtz, sizeof(psPara->sAlarmPtz));
	ret |= ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_DISPATCH_VLOST, nId, &sAlarmPara);
	
	return ret;
}

//一般报警事件布防设置
s32 ConfigSetAlarmSensorSchedule(SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorSch sConfig;

	memcpy(&sConfig.psSensorSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_SENSOR_SCH, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaSchedule.nSchType = psPara->nSchType;
	memcpy(sAlarmPara.sAlaSchedule.nSchTime, psPara->sSchTime, sizeof(psPara->sSchTime));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_SCHEDULE_SENSOR, nId, &sAlarmPara);
	
	return ret;
}
s32 ConfigSetAlarmIPCExtSensorSchedule(SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorSch sConfig;

	memcpy(&sConfig.psSensorSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_IPCEXTSENSOR_SCH, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaSchedule.nSchType = psPara->nSchType;
	memcpy(sAlarmPara.sAlaSchedule.nSchTime, psPara->sSchTime, sizeof(psPara->sSchTime));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_SCHEDULE_IPCEXTSENSOR, nId, &sAlarmPara);
	
	return ret;
}	

s32 ConfigSetAlarmIPCCoverSchedule(SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSensorSch sConfig;

	memcpy(&sConfig.psSensorSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_IPCCOVER_SCH, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaSchedule.nSchType = psPara->nSchType;
	memcpy(sAlarmPara.sAlaSchedule.nSchTime, psPara->sSchTime, sizeof(psPara->sSchTime));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_SCHEDULE_IPCCOVER, nId, &sAlarmPara);
	
	return ret;
}	


s32 ConfigSetAlarmVMotionSchedule(SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVMotionSch sConfig;

	memcpy(&sConfig.psVMSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_VMOTION_SCH, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaSchedule.nSchType = psPara->nSchType;
	memcpy(sAlarmPara.sAlaSchedule.nSchTime, psPara->sSchTime, sizeof(psPara->sSchTime));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_SCHEDULE_VMOTION, nId, &sAlarmPara);
	
	return ret;
}

s32 ConfigSetAlarmVBlindSchedule(SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVBlindSch sConfig;

	memcpy(&sConfig.psVBlindSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_VBLIND_SCH, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaSchedule.nSchType = psPara->nSchType;
	memcpy(sAlarmPara.sAlaSchedule.nSchTime, psPara->sSchTime, sizeof(psPara->sSchTime));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_SCHEDULE_VBLIND, nId, &sAlarmPara);
	
	return ret;
}

s32 ConfigSetAlarmVLostSchedule(SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigVLostSch sConfig;

	memcpy(&sConfig.psVLostSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_VLOST_SCH, &sConfig, nId);
	
	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaSchedule.nSchType = psPara->nSchType;
	memcpy(sAlarmPara.sAlaSchedule.nSchTime, psPara->sSchTime, sizeof(psPara->sSchTime));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_SCHEDULE_VLOST, nId, &sAlarmPara);
		
	return ret;
}

//报警输出事件布防设置
s32 ConfigSetAlarmOutSchedule(SBizAlarmSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigAlarmOutSch sConfig;
	
	memcpy(&sConfig.psAlarmOutSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_ALARMOUT_SCH, &sConfig, nId);

	SAlarmPara sAlarmPara;

	sAlarmPara.sAlaSchedule.nSchType = psPara->nSchType;
	memcpy(sAlarmPara.sAlaSchedule.nSchTime, psPara->sSchTime, sizeof(psPara->sSchTime));
	ret = ModAlarmSetParam(g_sBizManager.hAla, EM_ALARM_SCHEDULE_ALARMOUT, nId, &sAlarmPara);
	
	return ret;
}

s32 ConfigSetAlarmBuzzSchedule(SBizAlarmSchedule *psPara, u8 nId)
{
	return 0;
}

//录像布防
s32 ConfigSetSchRecSchedule(SBizSchedule *psPara, u8 nId)
{
	s32 ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigRecTimerSch sConfig;
	memcpy(&sConfig.psRecSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_TIMER_REC_SCH, &sConfig, nId);

	SModRecSchPara sRecSch;
	sRecSch.nChn = nId;
	sRecSch.nValue = 0x7f;
	sRecSch.nTimeType = psPara->nSchType;
	sRecSch.nRecType = EM_REC_SCH_REC_TIMER;
	memcpy(&sRecSch.nSchTime[0][0], &psPara->sSchTime[0][0], sizeof(SModRecSchTime)*31*MAX_TIME_SEGMENTS);
/*	printf("chn:%d, TimeType:%d\n", nId, sRecSch.nTimeType);
	int i = 0, j;
	for(i=0; i<7; i++)
	{
		printf("schTime[%d]: ", i);
		for(j = 0; j< MAX_SCHEDULE_SEGMENTS; j++)
		{
			printf("0x%04u~0x%04u ", sRecSch.nSchTime[i][j].nStartTime,sRecSch.nSchTime[i][j].nStopTime);
		}
		printf("\n");
	}
*/
	ret = ModRecordSetSchedulePara(nId, &sRecSch);
	
	return ret;
}

s32 ConfigSetVMotionAlaRecSchedule(SBizSchedule *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigRecVMSch sConfig;
	memcpy(&sConfig.psRecSch, psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_VMOTION_REC_SCH, &sConfig, nId);

	SModRecSchPara sRecSch;
	sRecSch.nChn = nId;
	sRecSch.nValue = 0x7f;
	sRecSch.nTimeType = psPara->nSchType;
	sRecSch.nRecType = EM_REC_SCH_REC_VMOTION;
	memcpy(&sRecSch.nSchTime[0][0], &psPara->sSchTime[0][0], sizeof(SModRecSchTime)*31*MAX_TIME_SEGMENTS);
/*	printf("chn:%d, TimeType:%d\n", nId, sRecSch.nTimeType);
	int i = 0, j;
	for(i=0; i<7; i++)
	{
		printf("schTime[%d]: ", i);
		for(j = 0; j< MAX_SCHEDULE_SEGMENTS; j++)
		{
			printf("0x%04u~0x%04u ", sRecSch.nSchTime[i][j].nStartTime,sRecSch.nSchTime[i][j].nStopTime);
		}
		printf("\n");
	}
*/
	ret = ModRecordSetSchedulePara(nId, &sRecSch);
	
	return ret;
}

s32 ConfigSetSensorAlaRecSchedule(SBizSchedule *psPara, u8 nId)
{
	int ret;
	
	if(NULL == psPara)
	{
		return -1;
	}
	
	//printf("11112222\n");
	
	SModConfigRecAlarmInSch sConfig;
	memcpy(&sConfig.psRecSch,psPara, sizeof(SBizAlarmSchedule));
	ret = ModConfigSetParam(EM_CONFIG_PARA_SENSOR_REC_SCH, &sConfig, nId);
	
	//printf("11133333\n");
	
	SModRecSchPara sRecSch;
	sRecSch.nChn = nId;
	sRecSch.nValue = 0x7f;
	sRecSch.nTimeType = psPara->nSchType;
	sRecSch.nRecType = EM_REC_SCH_REC_ALARM;
	memcpy(&sRecSch.nSchTime[0][0], &psPara->sSchTime[0][0], sizeof(SModRecSchTime)*31*MAX_TIME_SEGMENTS);
	ret = ModRecordSetSchedulePara(nId, &sRecSch);
	
	//printf("11144444\n");
	
	return ret;
}

//云台参数
s32 ConfigSetPtzPara(SBizPtzPara *psPara, u8 nId)
{
	int ret = 0;
	SBizManager* psBizManager = &g_sBizManager;
	
	if(NULL == psPara && psPara->nProtocol < psBizManager->nPtzProtocolNum && psBizManager->pProtocolList)
	{
		return -1;
	}
	
	SModConfigPtzParam sConfig;
	
	memset(&sConfig, 0, sizeof(sConfig));
	
	sConfig.nEnable = psPara->nEnable;
	sConfig.nCamAddr = psPara->nCamAddr;
	sConfig.nDataBit = psPara->nDataBit;
	sConfig.nStopBit = psPara->nStopBit;
	sConfig.nBaudRate = psPara->nBaudRate;
	sConfig.nCheckType = psPara->nCheckType;
	sConfig.nFlowCtrlType = psPara->nFlowCtrlType;
	strcpy(sConfig.nProtocolName, psBizManager->pProtocolList[psPara->nProtocol]);

	int i, j, k;
	for (i = 0; i <sizeof(psPara->sAdvancedPara.nIsPresetSet); i++)
	{
		j = i / 64;
		k = i % 64;
		sConfig.sTourPresetPara.nPresetId[j]  |= (((u64) (psPara->sAdvancedPara.nIsPresetSet[i]?1:0)) << k);
	}

	for (i = 0; i <sizeof(psPara->sAdvancedPara.sTourPath) / sizeof(SBizTourPath); i++)
	{
		if  (psPara->sAdvancedPara.sTourPath[i].nPathNo > 0)
		{
			j = (psPara->sAdvancedPara.sTourPath[i].nPathNo - 1) / 64;
			k = (psPara->sAdvancedPara.sTourPath[i].nPathNo - 1) % 64;

			sConfig.sTourPresetPara.nTourId[j]  |= ((u64)1 << k);

			k = psPara->sAdvancedPara.sTourPath[i].nPathNo - 1;
			
			for (j = 0; j < sizeof(psPara->sAdvancedPara.sTourPath[i].sTourPoint) / sizeof(SBizTourPoint); j++)
			{
				sConfig.sTourPresetPara.nTourPresetId[k][j].nPresetId = psPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nPresetPos;
				sConfig.sTourPresetPara.nTourPresetId[k][j].nStayTime = psPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nDwellTime;
				sConfig.sTourPresetPara.nTourPresetId[k][j].nSpeed = psPara->sAdvancedPara.sTourPath[i].sTourPoint[j].nSpeed;
			}
		}
	}
	
	ret = PtzSetPara(nId, psPara);
	
	if(ret == 0)
	{
		ret = ModConfigSetParam(EM_CONFIG_PARA_PTZ, &sConfig, nId);
	}
	
	return ret;
}

//用户
s32 ConfigSetUserPara(SBizUserPara *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigUserPara sConfig;
	
	sConfig.nFlagGroupSetted = psPara->nFlagGroupSetted;
	sConfig.nFlagUserSetted[0] = psPara->nFlagUserSetted[0];
	sConfig.nFlagUserSetted[1] = psPara->nFlagUserSetted[1];

	int i;
	for(i = 0; i < MOD_BIZ_MAX_USER_NUM; i++)
	{
		if((sConfig.nFlagUserSetted[i/64] >> (i%64)) & 1)
		{
			strcpy(sConfig.sUserPara[i].UserName, psPara->sUserPara[i].UserName);
			strcpy(sConfig.sUserPara[i].Password, psPara->sUserPara[i].Password);
			strcpy(sConfig.sUserPara[i].GroupBelong, psPara->sUserPara[i].GroupBelong);
			strcpy(sConfig.sUserPara[i].UserDescript, psPara->sUserPara[i].UserDescript);
			sConfig.sUserPara[i].emLoginTwince = psPara->sUserPara[i].emLoginTwince;
			sConfig.sUserPara[i].emIsBindPcMac = psPara->sUserPara[i].emIsBindPcMac;
			sConfig.sUserPara[i].PcMacAddress = psPara->sUserPara[i].PcMacAddress;
			int j;
			for(j = 0; j < sizeof(sConfig.sUserPara[i].UserAuthor.nAuthor); j++)
			{
				sConfig.sUserPara[i].UserAuthor.nAuthor[j] = psPara->sUserPara[i].UserAuthor.nAuthor[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sUserPara[i].UserAuthor.nLiveView[j] = psPara->sUserPara[i].UserAuthor.nLiveView[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sUserPara[i].UserAuthor.nRemoteView[j] = psPara->sUserPara[i].UserAuthor.nRemoteView[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sUserPara[i].UserAuthor.nRecord[j] = psPara->sUserPara[i].UserAuthor.nRecord[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sUserPara[i].UserAuthor.nPlayback[j] = psPara->sUserPara[i].UserAuthor.nPlayback[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sUserPara[i].UserAuthor.nBackUp[j] = psPara->sUserPara[i].UserAuthor.nBackUp[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sUserPara[i].UserAuthor.nPtzCtrl[j] = psPara->sUserPara[i].UserAuthor.nPtzCtrl[j];
			}				
		}
	}
	
	for(i = 0; i < MOD_BIZ_MAX_GROUP_NUM; i++)
	{
		if((sConfig.nFlagGroupSetted >> i) & 1)
		{
			strcpy(sConfig.sGroupPara[i].GroupName, psPara->sGroupPara[i].GroupName);
			strcpy(sConfig.sGroupPara[i].GroupDescript, psPara->sGroupPara[i].GroupDescript);
			sConfig.sGroupPara[i].nDoWithUser = psPara->sGroupPara[i].nDoWithUser;
			int j;
			for(j = 0; j < sizeof(sConfig.sGroupPara[i].GroupAuthor.nAuthor); j++)
			{
				sConfig.sGroupPara[i].GroupAuthor.nAuthor[j] = psPara->sGroupPara[i].GroupAuthor.nAuthor[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sGroupPara[i].GroupAuthor.nLiveView[j] = psPara->sGroupPara[i].GroupAuthor.nLiveView[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sGroupPara[i].GroupAuthor.nRemoteView[j] = psPara->sGroupPara[i].GroupAuthor.nRemoteView[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sGroupPara[i].GroupAuthor.nRecord[j] = psPara->sGroupPara[i].GroupAuthor.nRecord[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sGroupPara[i].GroupAuthor.nPlayback[j] = psPara->sGroupPara[i].GroupAuthor.nPlayback[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sGroupPara[i].GroupAuthor.nBackUp[j] = psPara->sGroupPara[i].GroupAuthor.nBackUp[j];
			}
			for(j = 0; j < 4; j++)
			{
				sConfig.sGroupPara[i].GroupAuthor.nPtzCtrl[j] = psPara->sGroupPara[i].GroupAuthor.nPtzCtrl[j];
			}				
		}
	}
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_USER, &sConfig, 0);
	
	return ret;
}

s32 ConfigGetPara(EMCONFIGPARATYPE emType, u8 nId, void* para)
{
    return ModConfigGetParam(emType, para, nId);
}

s32 ConfigSetPara(EMCONFIGPARATYPE emType, u8 nId, void* para)
{
    return ModConfigSetParam(emType, para, nId);
}

#if 0
//NVR used
static unsigned char g_ipc_init_flag = 0;
static SBizIPCameraPara g_ipc_list[64];
#endif

//NVR used
s32 ConfigGetIPCameraPara(u32 bDefault, SBizIPCameraPara *psPara, u8 nId)
{
	#if 0
	if(!g_ipc_init_flag)
	{
		memset(g_ipc_list, 0, sizeof(g_ipc_list));
		
		int i = 0;
		for(i = 0; i < sizeof(g_ipc_list)/sizeof(g_ipc_list[0]); i++)
		{
			g_ipc_list[i].channel_no = i;
			g_ipc_list[i].enable = 0;
			g_ipc_list[i].dwIp = inet_addr("192.168.1.88");
			g_ipc_list[i].wPort = 8080;
			g_ipc_list[i].protocol_type = PRO_TYPE_ONVIF;
			g_ipc_list[i].trans_type = TRANS_TYPE_TCP;
			g_ipc_list[i].stream_type = STREAM_TYPE_MAIN;
			g_ipc_list[i].ipc_type = IPC_TYPE_720P;
			strcpy(g_ipc_list[i].user, "admin");
			strcpy(g_ipc_list[i].pwd, "admin");
			sprintf(g_ipc_list[i].address, "http://%s:%d/onvif/device_service", "192.168.1.88", 8080);
		}
		
		g_ipc_init_flag = 1;
	}
	#endif
	
	if(psPara == NULL)
	{
		return -1;
	}
	
	#if 0
	*psPara = g_ipc_list[nId];
	#endif
	
	int ret = -1;
	SModConfigIPCameraParam sConfig;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_IPCAMERA, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_IPCAMERA, &sConfig, nId);
	}
	
	if(ret == 0)
	{
		psPara->channel_no = sConfig.channel_no;
		psPara->enable = sConfig.enable;
		psPara->protocol_type = sConfig.protocol_type;
		psPara->trans_type = sConfig.trans_type;
		psPara->stream_type = sConfig.stream_type;
		psPara->ipc_type = sConfig.ipc_type;
		psPara->dwIp = sConfig.dwIp;
		psPara->wPort = sConfig.wPort;
		strcpy(psPara->user, sConfig.user);
		strcpy(psPara->pwd, sConfig.pwd);
		strcpy(psPara->address, sConfig.address);
		
		//yaogang modify for yuetian private
		psPara->main_audio_sw = sConfig.main_audio_sw;//主码流音频开关
		psPara->sub_audio_sw = sConfig.sub_audio_sw;//次码流音频开关
	}
	
	return ret;
}

//NVR used
s32 ConfigSetIPCameraPara(SBizIPCameraPara *psPara, u8 nId)
{
	if(psPara == NULL)
	{
		return -1;
	}
	
	#if 0
	g_ipc_list[nId] = *psPara;
	#endif
	
	ipc_unit ipcam;
	ipcam.channel_no = psPara->channel_no;
	ipcam.enable = psPara->enable;
	//printf("ipcam.enable: %d\n", ipcam.enable);
	ipcam.protocol_type = psPara->protocol_type;
	ipcam.trans_type = psPara->trans_type;
	ipcam.stream_type = psPara->stream_type;
	ipcam.ipc_type = psPara->ipc_type;
	ipcam.dwIp = psPara->dwIp;
	ipcam.wPort = psPara->wPort;
	strcpy(ipcam.user, psPara->user);
	strcpy(ipcam.pwd, psPara->pwd);
	strcpy(ipcam.address, psPara->address);
//yaogang modify for yuetian private
	ipcam.main_audio_sw = psPara->main_audio_sw;//主码流音频开关
	ipcam.sub_audio_sw   = psPara->sub_audio_sw;//次码流音频开关
	
	SModConfigIPCameraParam sConfig;
	sConfig.channel_no = psPara->channel_no;
	sConfig.enable = psPara->enable;
	sConfig.protocol_type = psPara->protocol_type;
	sConfig.trans_type = psPara->trans_type;
	sConfig.stream_type = psPara->stream_type;
	sConfig.ipc_type = psPara->ipc_type;
	sConfig.dwIp = psPara->dwIp;
	sConfig.wPort = psPara->wPort;
	strcpy(sConfig.user, psPara->user);
	strcpy(sConfig.pwd, psPara->pwd);
	strcpy(sConfig.address, psPara->address);

//yaogang modify for yuetian private
	sConfig.main_audio_sw = psPara->main_audio_sw;//主码流音频开关
	sConfig.sub_audio_sw   = psPara->sub_audio_sw;//次码流音频开关

	
	int ret = ModConfigSetParam(EM_CONFIG_PARA_IPCAMERA, &sConfig, nId);
	
	ret |= tl_set_ipcamera(nId, &ipcam);

	return ret;
}

//获得字符串取值列表
s32 ConfigGetParaStr(SBizConfigParaList *psBizConfigParaList)
{
    EMCONFIGPARALISTTYPE emParaType = psBizConfigParaList->emParaType;
    u8* pnRealListLen = psBizConfigParaList->pnRealListLen;
    u8 nMaxListLen = psBizConfigParaList->nMaxListLen;
    u8 nMaxStrLen = psBizConfigParaList->nMaxStrLen;
	SBizManager* psBizManager = &g_sBizManager;
	
	if (emParaType == EM_BIZ_CFG_PARALIST_PTZPROTOCOL && pnRealListLen && psBizManager->pProtocolList)
	{
		int i;
		
		*pnRealListLen = psBizManager->nPtzProtocolNum;
		if (nMaxListLen < *pnRealListLen)
		{
			*pnRealListLen = nMaxListLen;
		}
		
		for (i = 0; i < *pnRealListLen; i++)
		{
			strcpy(psBizConfigParaList->strList[i], psBizManager->pProtocolList[i]);
		}
		
		return 0;
	}
	
    return ModConfigGetParaStr(emParaType, psBizConfigParaList->strList, pnRealListLen, nMaxListLen, nMaxStrLen);
}

//根据数字取值获得字符串列表中的index
s32 ConfigGetParaListIndex(SBizConfigParaIndex *psBizConfigParaIndex)
{
    EMCONFIGPARALISTTYPE emParaType = psBizConfigParaIndex->emParaType;
    s32 nValue = psBizConfigParaIndex->nValue;
    u8* pnIndex = psBizConfigParaIndex->pnIndex;
	
	if(emParaType == EM_BIZ_CFG_PARALIST_PTZPROTOCOL)
	{
		*pnIndex = nValue;
		return 0;
	}
	
	//u32 start = GetTimeTick();
	s32 ret = ModConfigGetParaListIndex(emParaType, nValue, pnIndex);
	//u32 end = GetTimeTick();
	//printf("ConfigGetParaListIndex: end - start = %d \n",end-start);
	
    return ret;
}

//根据index获得实际参数取值
s32 ConfigGetParaListValue(SBizConfigParaValue *psBizConfigParaValue)
{
    EMCONFIGPARALISTTYPE emParaType = psBizConfigParaValue->emParaType;
    u8 nIndex = psBizConfigParaValue->nIndex;
    s32* pnValue = psBizConfigParaValue->pnValue;
	
	if (emParaType == EM_BIZ_CFG_PARALIST_PTZPROTOCOL)
	{
		*pnValue = nIndex;
		return 0;
	}
	
    return ModConfigGetParaListValue(emParaType, nIndex, pnValue);
}

s32 ConfigBackup(char* pszFileName)
{
	//s8* fileName = "config_backup.ini";
	
	SUsbDirList sUsbDirList;
	
	s8 pathName[64] = {0};

	if(pszFileName)
	{
		// now just sInfo[0] is valid
		sprintf(pathName, "%s", pszFileName); 
	}
	else
	{
		if( 0==ModSysComplexGetUsbDirInfo(&sUsbDirList) )
		{
			// now just sInfo[0] is valid
			sprintf(pathName, "%s/config_backup.ini", sUsbDirList.sInfo[0].szDir); 
		}
		else
			return -1;
	}

	return ModConfigBackup(pathName);
}

s32 ConfigResumeDefault(EMBIZCONFIGPARATYPE emType, s32 nId)
{
	return ModConfigResumeDefault(emType, nId);
}

s32 ConfigResume(SBizCfgResume* spCfgResume)
{
	EMBIZCONFIGPARATYPE emType = spCfgResume->emType;
	s32 nId = spCfgResume->nId;
	s8 pFilePath[64] = {0};
	strcpy(pFilePath,spCfgResume->pFilePath);
	
	return ModConfigResume(emType, nId, pFilePath);
}

s32 ConfigDefault()
{
	return ModConfigDefault();
}

s32 ConfigSyncFileToFlash(u8 nMode)
{
	return ModConfigSyncFileToFlash(nMode);
}

// network config get/set
s32 ConfigGetNetPara(u32 bDefault, SBizNetPara *psPara, u8 nId)
{
	SModConfigNetParam sPIns;
	int ret;
	
	//printf("ConfigGetNetPara line %d\n", __LINE__);
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_NETWORK, &sPIns, nId);		
		
		printf("ConfigGetNetPara default DDNSFlag=%d,UpdateIntvl=%d\n",sPIns.DDNSFlag,sPIns.UpdateIntvl);
		
		// 恢复默认网络配置不包括MAC地址
		SModConfigNetParam sPIns2;
		if(0 == ModConfigGetParam(EM_CONFIG_PARA_NETWORK, &sPIns2, nId))
		{
			memcpy(sPIns.MAC, sPIns2.MAC, sizeof(sPIns.MAC));
		}
	}
	else
	{
		ret = ModConfigGetParam( EM_CONFIG_PARA_NETWORK, &sPIns, nId );
		
		//printf("ConfigGetNetPara DDNSFlag=%d,UpdateIntvl=%d\n",sPIns.DDNSFlag,sPIns.UpdateIntvl);
	}
	
	if(0 == ret)
	{		
		memcpy( psPara, &sPIns, sizeof(sPIns) );
		
		//printf("ConfigGetNetPara line %d\n", __LINE__);
		
		return 0;
	}
	else
	{
		return -1;
	}
}

s32 ConfigSetNetPara(SBizNetPara *psPara, u8 nId)
{
	s32 ret = -1;
	
	SModConfigNetParam sPIns;
	ret = ModConfigGetParam( EM_CONFIG_PARA_NETWORK, &sPIns, nId );
	if(0 == ret)
	{
		//csp modify 20130321
		if(psPara->UPNPFlag != sPIns.UPNPFlag)
		{
			if(psPara->UPNPFlag)
			{
				BizNetStartUPnP(1);
			}
			else
			{
				BizNetStartUPnP(0);
			}
		}
		
		//printf("ConfigSetNetPara DDNSFlag=(%d,%d) UpdateIntvl=(%d,%d)\n",sPIns.DDNSFlag,psPara->DDNSFlag,sPIns.UpdateIntvl,psPara->UpdateIntvl);
		
		memcpy( &sPIns, psPara, sizeof(sPIns) );
		ret = ModConfigSetParam( EM_CONFIG_PARA_NETWORK, &sPIns, nId );
		{
			SMultiCastInfo sInfo;
			NetComm_GetMultiCastInfo(&sInfo);
			sInfo.deviceIP	= sPIns.HostIP;
			sInfo.devicePort= sPIns.TCPPort;
			NetComm_SetMultiCastInfo(&sInfo);
		}
	}
	
	return ret;
}

s32 ConfigGetSGPara(u32 bDefault,SBizSGParam *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModConfigSGParam sConfig;
	s32 nId = 0;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SG, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SG, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		memcpy(psPara, &sConfig, sizeof(SBizSGParam));
	}
	
	return ret;
}

s32 ConfigSetSGPara(SBizSGParam *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	//SModConfigSGParam sConfig;
	s32 nId = 0;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_SG, (SModConfigSGParam *)psPara, nId);
	
	return ret;
}
//日常图片配置
s32 ConfigGetSGDailyPara(u32 bDefault,SBizDailyPicCFG *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModDailyPicCFG sConfig;
	s32 nId = 0;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SG_DAILY, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SG_DAILY, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		memcpy(psPara, &sConfig, sizeof(SModDailyPicCFG));
	}
	
	return ret;
}

s32 ConfigSetSGDailyPara(SBizDailyPicCFG *psPara)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	s32 nId = 0;
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_SG_DAILY, (SModDailyPicCFG *)psPara, nId);
	
	return ret;
}

//报警图片配置
s32 ConfigGetSGAlarmPara(u32 bDefault, SBizAlarmPicCFG *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	SModAlarmPicCFG sConfig;
	
	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SG_ALARM, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SG_ALARM, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		memcpy(psPara, &sConfig, sizeof(SModAlarmPicCFG));
	}
	
	return ret;
}

s32 ConfigSetSGAlarmPara(SBizAlarmPicCFG *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_SG_ALARM, (SModAlarmPicCFG *)psPara, nId);
	
	return ret;
}


//抓图参数
s32 ConfigGetSnapChnPara(u32 bDefault, SBizSnapChnPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}

	 SModConfigSnapChnPara sConfig;
	//if (nId == 0)
		//printf("yg ConfigGetAlarmIPCExtSensorPara chn0\n");

	if(bDefault)
	{
		ret = ModConfigGetDefaultParam(EM_CONFIG_PARA_SNAP_CHN, &sConfig, nId);
	}
	else
	{
		ret = ModConfigGetParam(EM_CONFIG_PARA_SNAP_CHN, &sConfig, nId);
	}
	
	if (0 == ret)
	{
		memcpy(psPara, &sConfig, sizeof(SBizSnapChnPara));
	}
	
	return ret;
}
s32 ConfigSetSnapChnPara(SBizSnapChnPara *psPara, u8 nId)
{
	int ret;

	if (NULL == psPara)
	{
		return -1;
	}
	
	ret = ModConfigSetParam(EM_CONFIG_PARA_SNAP_CHN, (SModConfigSnapChnPara *)psPara, nId);
	
	return ret;
}



