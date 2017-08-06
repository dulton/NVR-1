//** include
#include "ExcuteCmd.h"
#include "biz.h"
#include "biz_types.h"
#include "mod_syscomplex.h"
#include "mod_config.h"
#include "biz_config.h"
#include "diskmanage.h"
#include "biz_user.h"
#include "biz_net.h"
#include "ctrlprotocol.h"
#include "iconv.h"


extern EMBIZPREVIEWMODE GetCurPreviewMode_CW();
extern void SetCurPreviewMode_CW(EMBIZPREVIEWMODE mode);
extern void SwitchPreview(EMBIZPREVIEWMODE emMode, u8 nModePara);


//** macro

#define CRUISE_MAX	TOUR_PATH_MAX
#define PRESETNUM_MAX	PRESET_MAX

//** typedef 

//** local  var

#define nMaxListLen 20
#define nMaxStrLen 64

static s8* g_strList[nMaxListLen];
static s8 strList[nMaxListLen][nMaxStrLen] = {{0}};

typedef struct _sPtzList
{
	u8 idx2_0;
	u8 pro_id;
} SPtzList;

static SPtzList sPtzProList[] = 
{
	{0, NETDVR_PROTOCOLTYPE_HY},
	{1, NETDVR_PROTOCOLTYPE_Panasonic},
	{2, NETDVR_PROTOCOLTYPE_Pelco_9750},
	{3, NETDVR_PROTOCOLTYPE_PELCO_D},
	{4, NETDVR_PROTOCOLTYPE_PelcoD1},
	{5, NETDVR_PROTOCOLTYPE_PelcoD_S1},
	{6, NETDVR_PROTOCOLTYPE_PELCO_P},
	{7, NETDVR_PROTOCOLTYPE_PelcoP1},
	{8, NETDVR_PROTOCOLTYPE_PelcoP5},
	{9, NETDVR_PROTOCOLTYPE_Philips},
	{10, NETDVR_PROTOCOLTYPE_SAMSUNG},
	{11, NETDVR_PROTOCOLTYPE_Sanli},
	{12, NETDVR_PROTOCOLTYPE_Santachi},
	{13, NETDVR_PROTOCOLTYPE_Sharp},
	{14, NETDVR_PROTOCOLTYPE_Sony},
	{15, NETDVR_PROTOCOLTYPE_Yaan}
};

/*
EM_BIZENC_RESOLUTION_QCIF = 0,
EM_BIZENC_RESOLUTION_CIF = 1,
EM_BIZENC_RESOLUTION_HD1 = 2,
EM_BIZENC_RESOLUTION_4CIF = 3,
EM_BIZENC_RESOLUTION_D1 = 4,
		
#define VIDEORESOLU_QCIF 1
#define VIDEORESOLU_CIF 2
#define VIDEORESOLU_HD1 3
#define VIDEORESOLU_D1 4 // 3
*/

#if 0//csp modify
static u8 sOldResoList[] = {VIDEORESOLU_QCIF, VIDEORESOLU_CIF, VIDEORESOLU_HD1, VIDEORESOLU_D1};
static u8 sMainStResoList[] = {VIDEORESOLU_CIF, VIDEORESOLU_D1, VIDEORESOLU_720P, VIDEORESOLU_1080P, VIDEORESOLU_960H};
#endif

static void InitStrList()
{
	int i = 0;
	for(i = 0; i < nMaxListLen; i++)
	{
		g_strList[i] = strList[i];
	}
}

//** global var
//** local functions

static void dealRemoteCmd_ResetNotify(void);

static int SetPtzDeal(AlarmPtzDeal_t* pRemoteDeal, SBizAlarmPtz* pLocalDeal, SBizPtzAdvancedPara* pAdvance)
{
	int err = 0;
	
	if(!pRemoteDeal || !pLocalDeal || !pAdvance) return err;
	
	if(pRemoteDeal->flag_preset)
	{
		int nPresetIdx = pRemoteDeal->preset;
		if(nPresetIdx>0 && nPresetIdx<=PRESET_MAX) 
		{
			if(pAdvance->nIsPresetSet[nPresetIdx-1])
			{
				pLocalDeal->nALaPtzType = EM_BIZALARM_PTZ_PRESET;
				pLocalDeal->nId = nPresetIdx;
			}
		}
	}
	else if(pRemoteDeal->flag_cruise)
	{
		int nCruiseIdx = pRemoteDeal->cruise;
		printf("nCruiseIdx %d \n", nCruiseIdx);
		if(nCruiseIdx>0 && nCruiseIdx<=TOUR_PATH_MAX) 
		{
			if(pAdvance->sTourPath[nCruiseIdx-1].nPathNo > 0)
			{
				pLocalDeal->nALaPtzType = EM_BIZALARM_PTZ_PATROL;
				pLocalDeal->nId = nCruiseIdx;
			}
		}
	}
	else if(pRemoteDeal->flag_track)
	{
		pLocalDeal->nALaPtzType = EM_BIZALARM_PTZ_LOCUS;
	}
	else
	{
		pLocalDeal->nALaPtzType = EM_BIZALARM_PTZ_NULL;
		pLocalDeal->nId 		= 0;
	}

	return err;
}


static void GetPtzDeal(AlarmPtzDeal_t* pRemoteDeal, SBizAlarmPtz* pLocalDeal)
{
	if(!pRemoteDeal || !pLocalDeal) return;
	
	pRemoteDeal->preset 	= 1; // 未选中联动类型对应预置/巡航点置1保证客户端显示值在1-128范围内
	pRemoteDeal->cruise 	= 1; // 未选中联动类型对应预置/巡航点置1保证客户端显示值在1-128范围内
	switch(pLocalDeal->nALaPtzType)
	{
		case EM_BIZALARM_PTZ_PRESET: //联动预置点
		{
			pRemoteDeal->flag_preset = 1;

			if(pLocalDeal->nId>0 && pLocalDeal->nId<PRESETNUM_MAX)
			{
				pRemoteDeal->preset = pLocalDeal->nId;
			}
		} break;
		case EM_BIZALARM_PTZ_PATROL: //巡航线
		{
			pRemoteDeal->flag_cruise = 1;
			
			if(pLocalDeal->nId>0 && pLocalDeal->nId<CRUISE_MAX)
			{
				pRemoteDeal->cruise = pLocalDeal->nId;
			}
		} break;
		case EM_BIZALARM_PTZ_LOCUS: //轨迹
		{
			pRemoteDeal->flag_track = 1;
		} break;
	}
	
	/*
	//printf("get md ptz alarm deal flag %d %d flag %d %d flag %d\n",
		pVideolostparam->AlarmInPtz[i].flag_preset, pVideolostparam->AlarmInPtz[i].preset, 
		pVideolostparam->AlarmInPtz[i].flag_cruise, pVideolostparam->AlarmInPtz[i].cruise, 
		pVideolostparam->AlarmInPtz[i].flag_track 
	);
	*/	
}


static int GetMdRowCol( int* pRow, int* pCol )
{
	SBizDvrInfo sInfo;

	int ret = ConfigGetDvrInfo(0, &sInfo) ;
	if( 0 == ret)
	{
		*pRow = sInfo.nMDAreaRows;
		*pCol = sInfo.nMDAreaCols;
	}
	
	return ret;
}

static int GetVideoMainNum()
{
	SBizDvrInfo sInfo;
	ConfigGetDvrInfo(0, &sInfo);
	
	return sInfo.nVidMainNum;
}

//** global functions

int Excmd_GetSch(SBizSchedule* pSchPara, uint* pTgtSch)
{
	int i, j, k, l, h0, h1;
	
	for(i=0; i<7; i++)
	{
		SBizSchTime* pSch = pSchPara->sSchTime[i];
		
		for(j=0; j<MAX_SCHEDULE_SEGMENTS; j++)
		{
			if(pSch[j].nStartTime<pSch[j].nStopTime)
			{
				h0 = pSch[j].nStartTime/3600;
				h1 = (pSch[j].nStopTime)/3600;
				
				for(k=h0; k<=h1; k++)
				{
					pTgtSch[i] |= (0x1<<k);
				}
			}
			
			////printf("get sch start %d stop %d \n", pSch[j].nStartTime, pSch[j].nStopTime );
		}
		
		////printf("Get sch %d \n", pTgtSch[i]);
	}
	
	return 0;
}

int Excmd_CopyOneDaySch( 
	uint nSch,
	SBizSchTime pSTIns[] )
{
	int i, j;
	u8 bRecStart = 0;
	u8 nSeg = 0;
	
	for(j=0; j<24; j++)
	{
		// check 24 hours of one day
		// if j hour marked, set segment starttime when not set
		// if j not marked, skip and set last segment stoptime		
		if(nSch&(0x1<<j))
		{
			////printf("ch %d sch active !!! \n", j);
			
			// if not set starttime, set
			if(!bRecStart)
			{
				bRecStart = 1;
				pSTIns[nSeg].nStartTime = j*3600;
			}
			//else // if set , skip
			{
				if(j==23)
				{
					pSTIns[nSeg].nStopTime = (j+1)*3600-1;
				}
			}
		}
		else
		{
			// if last hour marked, set last hour endtime, clear setstarttime flag
			if(bRecStart && (nSch&(0x1<<(j-1))))
			{
				bRecStart = 0;
				pSTIns[nSeg].nStopTime = j*3600-1;
				
				nSeg++;
			}			
		}
	}
	
	return 0;
}

//csp modify 20121130
void do_web_link(u8 nLangId)
{
	//csp modify 20121130
	int nLangIdx = nLangId;//0;
	
	printf("The language num is :%d\n",nLangIdx);
	
	remove("/web/pages/home.asp");

	//csp modify 20150110
	symlink("/web/pages/CH/home.asp", "/web/pages/home.asp");
	return;
	
	switch(nLangIdx) 
	{
		//csp modify 20121130
		case 1://case 0:
			symlink("/web/pages/CH/home.asp", "/web/pages/home.asp");
			break;
	#if defined(_JMV_)//csp modify
		case 9:
			symlink("/web/pages/TUR/home.asp", "/web/pages/home.asp");
			break;
		case 8:
			symlink("/web/pages/POR/home.asp", "/web/pages/home.asp");
			break;
	#elif defined(_JUAN_)//csp modify
	#else
		case 2:
			if(0 == access("/web/pages/TCH/home.asp", F_OK))
			{
				symlink("/web/pages/TCH/home.asp", "/web/pages/home.asp");
			}
			else
			{
				symlink("/web/pages/EN/home.asp", "/web/pages/home.asp");
			}
			break;
		case 8:
			if(0 == access("/web/pages/POR/home.asp", F_OK))
			{
				symlink("/web/pages/POR/home.asp", "/web/pages/home.asp");
			}
			else
			{
				symlink("/web/pages/EN/home.asp", "/web/pages/home.asp");
			}
			break;
		case 9:
			if(0 == access("/web/pages/TUR/home.asp", F_OK))
			{
				symlink("/web/pages/TUR/home.asp", "/web/pages/home.asp");
			}
			else
			{
				symlink("/web/pages/EN/home.asp", "/web/pages/home.asp");
			}
			break;
	#endif
		default:
			symlink("/web/pages/EN/home.asp", "/web/pages/home.asp");
			break;
	}
}

void  DoCTRL_CMD_RECFILESEARCH(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	return;
}

void  DoCTRL_CMD_GETSYSTIME(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_sysTime_t *pGetsystemtime;
	SDateTime	sDTIns;
	struct tm tm_time;
	
	pGetsystemtime = &pRslt->sBasicInfo.sysTime;
	
	ModSysComplexDTGet(&sDTIns);
	
	tm_time.tm_year = sDTIns.nYear - 1900;
	tm_time.tm_mon = sDTIns.nMonth - 1;
	tm_time.tm_mday = sDTIns.nDay;
	
	tm_time.tm_hour = sDTIns.nHour;
	tm_time.tm_min = sDTIns.nMinute;
	tm_time.tm_sec = sDTIns.nSecode;
	
	pGetsystemtime->systemtime = mktime(&tm_time);
	
	//printf("get %04d-%02d-%02d\n", tm_time.tm_year, tm_time.tm_mon, tm_time.tm_mday);
}

void  DoCTRL_CMD_SETSYSTIME(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int            i;
	ifly_sysTime_t *pGetsystemtime;
	SDateTime sDTIns;
	struct tm *ptm_time;
	struct tm tm_time;
	
	pGetsystemtime = &pCmd->sReq.sysTime;
	
	//csp modify 20131210
	pGetsystemtime->systemtime += 2;
	
	//ptm_time = gmtime((const time_t*)&pGetsystemtime->systemtime);
	gmtime_r((const time_t*)&pGetsystemtime->systemtime, &tm_time);
	ptm_time = &tm_time;
	
	sDTIns.nYear = 1900+ptm_time->tm_year;
	sDTIns.nMonth = ptm_time->tm_mon+1;
	sDTIns.nDay = ptm_time->tm_mday;
	sDTIns.nHour = ptm_time->tm_hour;
	sDTIns.nMinute = ptm_time->tm_min;
	sDTIns.nSecode = ptm_time->tm_sec;
	
	BizRecordStopAll();
	usleep(2000*1000);//多睡会儿吧，否则结束时间戳容易出问题
	
	sDTIns.emDateTimeFormat = EM_DATETIME_IGNORE;
	ModSysComplexDTSet(&sDTIns);
	
	//csp modify 20140406
	usleep(20*1000);
	//printf("remote set time...\n");
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	BizGetPara(&bizTar, &bizSysPara);
	IPC_SetTimeZone(bizSysPara.nTimeZone, bizSysPara.nSyncTimeToIPC, 1);
	
	for(i = 0; i < GetVideoMainNum(); i++)
	{
		BizRecordResume(i);
	}
	
	//printf("set %04d-%02-%02d\n", sDTIns.nYear, sDTIns.nMonth, sDTIns.nDay);
}

void  DoCTRL_CMD_RECFILESEARCH1(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int sum = 0, i;
	s32 rtn;
	
	//search_param_t localsearch;
	ifly_recsearch_param_t remotesearch;
	ifly_recfile_desc_t desc;
	ifly_recfileinfo_t info;
	
	struct tm tm_time;
	u8 y,m,d, h,min, type, ch;
	char* pFileBuf = NULL;
	
	SSearchPara sSearchParaIns;
	SSearchResult sSearchResultIns;
	int nMaxFileNum;
	
	memcpy(&remotesearch, &pCmd->sReq.recsearch_param, sizeof(remotesearch));

	sSearchParaIns.nMaskType 	= remotesearch.type_mask;
	sSearchParaIns.nMaskChn 	= (remotesearch.channel_mask | (remotesearch.chn17to32mask<<16));
	sSearchParaIns.nStartTime 	= remotesearch.start_time;
	sSearchParaIns.nEndTime 	= remotesearch.end_time;
	
	//printf("mask type %d\n",sSearchParaIns.nMaskType);
	//printf("nMaskChn %8x\n",sSearchParaIns.nMaskChn);
	//printf("nStartTime %d\n",sSearchParaIns.nStartTime);
	//printf("nEndTime %d\n",sSearchParaIns.nEndTime);
	//printf("max_return %d\n",remotesearch.max_return);	
	
	nMaxFileNum = remotesearch.max_return;
	
	SRecfileInfo sRefFileInfo[4000] = {0}; //zlb20111117 去掉部分malloc
	pFileBuf = (char*)sRefFileInfo;
	sSearchResultIns.psRecfileInfo = (SRecfileInfo*)pFileBuf;
	
	//sSearchResultIns.psRecfileInfo = (SRecfileInfo*)pRslt->sBasicInfo.sRecFile.info;
	
	if(!sSearchResultIns.psRecfileInfo) 
	{
		//printf("not enough memory for found files!\n");
		return;
	}

	sSearchResultIns.nFileNum = 0;

	rtn = ModSysComplexDMSearch(EM_FILE_REC, &sSearchParaIns, &sSearchResultIns, 4000);
	//printf("startID: %d, nFileNum: %d\n", remotesearch.startID, sSearchResultIns.nFileNum);
	if(sSearchResultIns.nFileNum > 0)
	{
		ifly_recfileinfo_t* pOldFInfo;
		SRecfileInfo* pNewFInfo;

		for( i=remotesearch.startID-1; i<sSearchResultIns.nFileNum; i++ )
		{
			if(i>=remotesearch.startID-1+nMaxFileNum) break;
			
			pOldFInfo = &pRslt->sBasicInfo.sRecFile.info[i-remotesearch.startID+1];
			pNewFInfo = &sSearchResultIns.psRecfileInfo[i];
			
			pOldFInfo->channel_no 	= pNewFInfo->nChn-1;
			pOldFInfo->type 		= pNewFInfo->nType;
			pOldFInfo->start_time 	= pNewFInfo->nStartTime;
			pOldFInfo->end_time 	= pNewFInfo->nEndTime;
			pOldFInfo->image_format = pNewFInfo->nFormat;
			pOldFInfo->stream_flag 	= pNewFInfo->nStreamFlag;
			pOldFInfo->size 		= pNewFInfo->nSize;
			pOldFInfo->offset 		= pNewFInfo->nOffset;
			
			get_rec_file_name((recfileinfo_t*)pNewFInfo,pOldFInfo->filename,&pOldFInfo->offset);
			
			/*
			//printf("file %d %d %d %d %d %s\n", 
					pNewFInfo->nChn,
					pNewFInfo->nDiskNo, 
					pNewFInfo->nStartTime,
					pNewFInfo->nEndTime,
					pNewFInfo->nFileNo,
					pOldFInfo->filename
			);
			*/
			
			sum++;
		}
	}
	else
	{
		//if(pFileBuf) free(pFileBuf); //zlb20111117 去掉部分malloc
		
		return;
	}
	
	if(remotesearch.startID > sSearchResultIns.nFileNum)
	{
		desc.sum = sSearchResultIns.nFileNum;
		desc.startID = 0;
		desc.endID = 0; 
	}
	else
	{
		desc.startID = remotesearch.startID;
		desc.sum = sSearchResultIns.nFileNum; 
		
		desc.endID = remotesearch.startID+sum-1;
		
		memcpy(&pRslt->sBasicInfo.sRecFile.desc, &desc, sizeof(desc));
	}
	
	//if(pFileBuf) free(pFileBuf); //zlb20111117 去掉部分malloc
	
	//printf("sum %d startID %d endID %d\n", desc.sum, desc.startID, desc.endID);
}

s32 BizNetWriteLoginLog(CPHandle cp, s8 master, s8 slave, char* pUser)
{
	SBizLogInfo info; 
	memset(&info, 0, sizeof(info));
	info.nMasterType = master;
	info.nSlaveType = slave;
	
	struct in_addr addr;
	memcpy(&addr, &cp->ip, 4);
	
	//printf("net log %d  %s\n", __LINE__, inet_ntoa(addr));
	
	sprintf(info.aIp, "%s", inet_ntoa(addr));
	
	strcpy(info.aIp, inet_ntoa(addr));
	
	SNetUser sUser;
	
	sUser.ip = cp->ip;
	sUser.sock = cp->sockfd;
	
	if( NULL == pUser )
		memset(info.aUsername, 0, sizeof(info.aUsername));
	else
		strcpy(info.aUsername, pUser);
	
	return BizWriteLog(&info);	
}

s32 BizNetWriteLog( CPHandle cp, s8 master, s8 slave)
{
	SBizLogInfo info; 
	memset(&info, 0, sizeof(info));
	info.nMasterType = master;
	info.nSlaveType = slave;
	
	struct in_addr addr;
	memcpy(&addr, &cp->ip, 4);

	////printf("net log %d  %s \n", __LINE__, inet_ntoa(addr));
	
	sprintf(info.aIp, "%s", inet_ntoa(addr));
	
	strcpy(info.aIp, inet_ntoa(addr));
	
	SNetUser sUser;
	
	sUser.ip = cp->ip;
	sUser.sock = cp->sockfd;
	
	if( 0!=NetComm_NetUserCtrl(0, &sUser) )
		memset(info.aUsername, 0, sizeof(info.aUsername));
	else
		strcpy(info.aUsername, sUser.name);

	//printf("log write master %d slave %d user %s\n", master, slave, sUser.name);
	
	return BizWriteLog(&info);
}

// todo
void DealRemoteCmd_Login(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i, j;
	s32 rtn;
	ifly_loginpara_t* 	pLoginInfo = &pCmd->sReq.loginpara;
	SRemoteUserInfo*	pUserInfo = &pRslt->sBasicInfo.sUserInfo;
	
	//printf("%d login mac %s\n", __LINE__, pLoginInfo->macAddr);
	
	SBizUserUserPara sUserPara;
	strcpy(sUserPara.UserName, pLoginInfo->username);
	if(0 == UserGetUserInfo(&sUserPara))
	{
		SBizUserLoginPara 	sUserLoginIns;
		memset(&sUserLoginIns, 0, sizeof(SBizUserLoginPara));
		
		sUserLoginIns.emFrom = EM_BIZUSER_LOGINFROM_WEB;
		strcpy(sUserLoginIns.UserName, pLoginInfo->username);	
		strcpy(sUserLoginIns.Password, pLoginInfo->loginpass);
		
		#define MAC_LEN_MAX	18
		
		if(sUserPara.emIsBindPcMac == EM_MODUSER_BIND_PCMAC_YES)
		{
			char tmp[MAC_LEN_MAX];
			memset(tmp, 0, sizeof(tmp));
			
			char mac2[MAC_LEN_MAX] = {0};
			
			//printf("%d  mac %s\n", __LINE__, pLoginInfo->macAddr);
			if(strlen(pLoginInfo->macAddr) >= MAC_LEN_MAX)
			{
				pLoginInfo->macAddr[MAC_LEN_MAX-1] = '\0';
			}
			
			strcpy(mac2, pLoginInfo->macAddr);
			//printf("%d  mac %s\n", __LINE__, mac2);
			
			j = 0;
			for(i = 0; i<MAC_LEN_MAX-1; i++)
			{
				if(((mac2[i] >= '0') && (mac2[i] <= '9')) 
					|| ((mac2[i] >= 'a') && (mac2[i] <= 'f'))
					|| ((mac2[i] >= 'A') && (mac2[i] <= 'F')))
				{
					tmp[i-j] = mac2[i];
				}
				else
				{
					j++;
				}
			}
			
			//printf("%d  mac %s\n", __LINE__, tmp);
			
			if((12 != strlen(tmp)) )
			{
				pRslt->nErrCode = -1;
				return;
			}
			
			sscanf(tmp, "%012llX", &sUserLoginIns.nPcMac);
		}
		
		//printf("%d  mac %llx\n", __LINE__, sUserLoginIns.nPcMac);
		
		sUserLoginIns.nPcIp = pLoginInfo->ipAddr;
		
		SNetUser sUser;
		rtn = UserLogin(&sUserLoginIns, &sUser.id);
		if(rtn == 0)
		{
			pUserInfo->nUserNum = 1;		
			
			pUserInfo->szPrivilege[0] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PTZCTRL];
			pUserInfo->szPrivilege[1] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_RECORD];
			pUserInfo->szPrivilege[2] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PLAYBACK];
			pUserInfo->szPrivilege[3] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_SETUP];
			pUserInfo->szPrivilege[4] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_LOG_VIEW];
			pUserInfo->szPrivilege[5] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DISK_MANAGE];
			pUserInfo->szPrivilege[6] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_REMOTE_LOGIN];
			pUserInfo->szPrivilege[7] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DATA_MANAGE];
			pUserInfo->szPrivilege[8] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_AUDIO_TALK];
			pUserInfo->szPrivilege[9] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_RESET];
			pUserInfo->szPrivilege[10] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_UPDATE];
			pUserInfo->szPrivilege[11] = '0'+sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SHUTDOWN];
			
			//printf("%d  mac %llx\n", __LINE__, sUserLoginIns.nPcMac);
			
			if(pCmd->cph)
			{		
				sUser.bOnline = 1;
				sUser.bUserDel = 0;
				
				strcpy(sUser.name, pLoginInfo->username);
				
				//NetComm_NetUserCtrl(1, &sUser);
				sUser.ip 	= pCmd->cph->ip;
				sUser.sock 	= pCmd->cph->sockfd;
				NetComm_NetUserCtrl(1, &sUser);
				//printf("%d  mac %llx\n", __LINE__, sUserLoginIns.nPcMac);
				
				BizNetWriteLoginLog(pCmd->cph,  BIZ_LOG_MASTER_SYSCONTROL  , BIZ_LOG_SLAVE_LOGIN, pLoginInfo->username);
			}
		}
		else
		{
			pRslt->nErrCode = rtn;
		}
	}
	else
	{
		// 获取不到该用户即清除在线用户相关信息
		SNetUser sUser;
		strcpy(sUser.name, pLoginInfo->username);
		
		sUser.bUserDel = 1;
		
		//csp modify
		//NetComm_NetUserCtrl(1, &sUser);
		NetComm_NetUserCtrl(2, &sUser);
		
		pRslt->nErrCode = -1;
	}
	
	//printf("Login...\n");
	
	return;
}

// todo
void DealRemoteCmd_LoginOff(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_loginpara_t* pLoginInfo = &pCmd->sReq.loginpara;	
	
	SNetUser sUser;
	memset(&sUser, 0, sizeof(sUser));
	strcpy(sUser.name, pLoginInfo->username);
	sUser.ip = pCmd->cph->ip;
	sUser.sock = pCmd->cph->sockfd;
	
	printf("Logoff user:(%s,0x%08x,%d)\n",sUser.name,sUser.ip,sUser.sock);
	
	if(0 == NetComm_NetUserCtrl(0, &sUser))
	{
		printf("Logoff user:(%s,0x%08x,%d,%d)\n",sUser.name,sUser.ip,sUser.sock,sUser.id);
		
		if(0 == UserLogout(sUser.id))
		{
			BizNetWriteLog(pCmd->cph, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_LOGOFF);
			NetComm_NetUserCtrl(2, &sUser);
		}
		else//csp modify
		{
			NetComm_NetUserCtrl(2, &sUser);
		}
	}
	else
	{
		printf("user lost for Logoff\n");
	}
	
	return;
}

// todo
void DealRemoteCmd_GetDeviceInfo(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizDvrInfo sDvrInfo;
	
	ifly_DeviceInfo_t* pDevInfo = &pRslt->sBasicInfo.DeviceInfo;
	
	if(0 == ConfigGetDvrInfo(0, &sDvrInfo))
	{
		pDevInfo->maxChnNum			= sDvrInfo.nVidMainNum;					//最大通道数
		pDevInfo->maxAduioNum		= sDvrInfo.nAudNum;						//最大音频数
		pDevInfo->maxSubstreamNum	= sDvrInfo.nVidSubNum;					//最大子码流数
		pDevInfo->maxPlaybackNum	= 4;									//最大回放数
		pDevInfo->maxAlarmInNum		= sDvrInfo.nSensorNum;					//最大报警输入数
		pDevInfo->maxAlarmOutNum	= sDvrInfo.nAlarmoutNum;				//最大报警输出数
		pDevInfo->maxHddNum			= sDvrInfo.nMaxHddNum;					//最大硬盘数
		
		strcpy(pDevInfo->device_mode, sDvrInfo.strModel);
	}
	
	SBizNetPara sNetInfo;
	if(0 == ConfigGetNetPara(0, &sNetInfo, 0))
	{
		pDevInfo->deviceIP = sNetInfo.HostIP;
		
		//csp modify
		//pDevInfo->deviceIP = sNetInfo.TCPMaxConn;
		pDevInfo->devicePort = htons(sNetInfo.TCPPort);
	}
	
	//pDevInfo->device_name[32];//设备名称
	//pDevInfo->device_mode[32];//设备型号
	
	//csp modify
	SBizSystemPara sSysInfo;
	if(0 == ConfigGetSystemPara(0, &sSysInfo))
	{
		strcpy(pDevInfo->device_name, sSysInfo.strDevName);
	}
}

void DealRemoteCmd_GetVideoPropety(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_Video_Property_t* psIns = &pRslt->sBasicInfo.Video_Property;
	psIns->videoEncType 	= 98;
	psIns->max_videowidth 	= 704;
	psIns->max_videoheight 	= 576;
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$DealRemoteCmd_GetVideoPropety:(%d,%d)\n",704,576);
}

void DealRemoteCmd_GetAudioPropety(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_Audio_Property_t* psIns = &pRslt->sBasicInfo.Audio_Property;
	
	psIns->audioBitPerSample = 16;
	psIns->audioSamplePerSec = 8000;
	
#if 1//def USE_AUDIO_PCMU
	#if 0//csp modify
	psIns->audioFrameSize = 640;
	#else
	psIns->audioFrameSize = 642;
	#endif
	psIns->audioEnctype = 0;//MEDIA_TYPE_PCMU;
#else
	psIns->audioFrameSize = 168;
	psIns->audioEnctype = MEDIA_TYPE_PCMU;
#endif
	
	psIns->audioEnctype = 0;//19;//21;//MEDIA_TYPE_ADPCM;
	
	#if 0//csp modify
	psIns->audioFrameDurTime = 40;
	#else
	psIns->audioFrameDurTime = 40;
	#endif
}

void DealRemoteCmd_Getvoipproperty(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_VOIP_Property_t* psIns = &pRslt->sBasicInfo.VOIP_Property;

	psIns->VOIPBitPerSample = 16;
	psIns->VOIPFrameSize 	= 642;
	psIns->VOIPSamplePerSec = 8000;
}

// todo
void DealRemoteCmd_Getmdproperty(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_MD_Property_t* psIns = &pRslt->sBasicInfo.MD_Property;

	psIns->MDRow = 18;
	psIns->MDCol = 22;
}

//csp modify 20130421
#define FIX_DDNS_BUG

// todo
void DealRemoteCmd_Getnetwork(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_network_t* psIns = &pRslt->sBasicInfo.netInfo;
	SNetPara 	sPIIns;
	
	memset( &sPIIns, 0, sizeof( sPIIns ) );
	
	if( 0 == NetComm_ReadPara( &sPIIns ) )
	{
		memcpy( psIns->mac_address, sPIIns.MAC, sizeof( psIns->mac_address ) );
		
		//printf("ExcuteCmd mac : %s\n", sPIIns.MAC);
		
		psIns->ip_address 	= sPIIns.HostIP;
		psIns->net_mask 	= sPIIns.Submask;
		psIns->net_gateway 	= sPIIns.GateWayIP;
		psIns->dns 			= sPIIns.DNSIP;
	}
	
	SBizNetPara sPIns;
	
	if(0 == ConfigGetNetPara(0, &sPIns, 0))
	{
		psIns->server_port 			= sPIns.TCPPort;
		psIns->multicast_address 	= sPIns.McastIP;
		psIns->http_port			= sPIns.HttpPort;
		//u32 admin_host;
		psIns->host_port 			= sPIns.MobilePort;
		psIns->flag_pppoe 			= sPIns.PPPOEFlag;
		
		psIns->flag_dhcp 			|= (sPIns.McastFlag<<4);
		psIns->flag_dhcp			|= (sPIns.DhcpFlag);
		
		#ifdef FIX_DDNS_BUG
		if(sPIns.DDNSFlag > 0)
		{
			u8 index = 0;
			BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_DDNSDOMAIN, sPIns.DDNSFlag-1, &index);
			psIns->flag_ddns		= index + 1;
		}
		else
		{
			psIns->flag_ddns		= 0;
		}
		#else
		psIns->flag_ddns 			= sPIns.DDNSFlag;
		#endif
		
		if(strcmp(sPIns.PPPOEUser, "0") == 0)
			strcpy( psIns->user_name, "" );
		else
			memcpy( psIns->user_name, sPIns.PPPOEUser, sizeof( psIns->user_name ) );
		
		if(strcmp(sPIns.PPPOEPasswd, "0") == 0)
			strcpy( psIns->passwd, "" );
		else
			memcpy( psIns->passwd, sPIns.PPPOEPasswd, sizeof( psIns->passwd ) );
		
		if(strcmp(sPIns.DDNSDomain, "0") == 0)
			strcpy( psIns->ddns_domain, "" );
		else
			memcpy( psIns->ddns_domain, sPIns.DDNSDomain, sizeof( psIns->ddns_domain ) );
		
		if(strcmp(sPIns.DDNSUser, "0") == 0)
			strcpy( psIns->ddns_user, "" );
		else
			memcpy( psIns->ddns_user, sPIns.DDNSUser, sizeof( psIns->ddns_user ) );
		
		if(strcmp(sPIns.DDNSPasswd, "0") == 0)
			strcpy( psIns->ddns_passwd, "" );
		else
			memcpy( psIns->ddns_passwd, sPIns.DDNSPasswd, sizeof( psIns->ddns_passwd ) );		
	}
}

void* ResetThrxd(void* arg)
{
	static u8 bGotCmd = 0;
	
	if(!bGotCmd)
	{
		bGotCmd = 1;
		
		//printf("ResetNotify\n");
		//fflush(stdout);
		
		dealRemoteCmd_ResetNotify();
		
		//printf("ResetThrxd sleep-1\n");
		//fflush(stdout);
		
		sleep(5);
		
		//printf("ResetThrxd sleep-2\n");
		//fflush(stdout);
		
		BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
		
		//printf("RESTART\n");
		//fflush(stdout);
	}
	
	return NULL;
}

void* DdnsThrxd(void* arg)
{
	pthread_detach(pthread_self());//csp modify 20130322
	
	//printf("$$$$$$$$$$$$$$$$$$DdnsThrxd id:%d\n",getpid());
	
	if(arg)
	{
		sBizNetDdnsPara* pPara = (sBizNetDdnsPara*)arg;
		
		s32 ret = 0;
		
		if(pPara->eProt != 0)//csp modify 20130422
		{
			//csp modify 20130422
			//if(pPara->eProt == EM_BIZDDNS_PROT_TL)
			if(pPara->eProt == EM_BIZDDNS_PROT_TL || pPara->eProt == EM_BIZDDNS_PROT_JMDVR)
			{
				ret = BizNetDdnsCtrl(EM_BIZDDNS_REGIST, pPara);
				if(ret == 0)
				{
					printf("ddns regist ok\n");
				}
				else
				{
					printf("ddns regist failed\n");
					
					ret = 0;//csp modify 20130422
				}
			}
			
			if(ret == 0)
			{
				BizNetDdnsCtrl(EM_BIZDDNS_START, pPara);
			}
		}
		else
		{
			BizNetDdnsCtrl(EM_BIZDDNS_STOP, pPara);//csp modify 20130422
		}
	}
	
	return NULL;
}

// todo
static sBizNetDdnsPara sDdnsPara;

void DealRemoteCmd_Setnetwork(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	u8 nTmpFlag;
	u8 bReset = 0;
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;
	SBizNetPara sPIns;
	
	if(0 == ConfigGetNetPara(0, &sPIns, 0))
	{
		if(sPIns.HttpPort != psIns->http_port)
		{
			BizNetWriteLog(pCmd->cph, BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_HTTP_PORT);
			if(0 == NetComm_WebRestart(psIns->http_port) )
			{
				sPIns.HttpPort = psIns->http_port;
			}
		}
		
		if(sPIns.HostIP != psIns->ip_address)
		{
			BizNetWriteLog(pCmd->cph, BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_IP);
			sPIns.HostIP = psIns->ip_address;
		}
		
		if(sPIns.MobilePort != psIns->mobile_port)
		{
			//printf("MobilePort change!\n");
			bReset = 1;
			//BizNetWriteLog(pCmd->cph, BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_SERVER_PORT);
			sPIns.MobilePort 	=  psIns->mobile_port;
		}
		
		if(sPIns.TCPPort != psIns->server_port)
		{
			//printf("TCPPort change!\n");
			bReset = 1;
			BizNetWriteLog(pCmd->cph, BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_SERVER_PORT);
			sPIns.TCPPort 	=  psIns->server_port;
		}
		
		sPIns.Submask 	=  psIns->net_mask;
		sPIns.GateWayIP =  psIns->net_gateway;
		if(sPIns.DNSIP != psIns->dns)
		{
			BizNetWriteLog(pCmd->cph, BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_DNS_CONFIG);
			sPIns.DNSIP =  psIns->dns;
		}
		
		sPIns.McastIP 	=  psIns->multicast_address;
		
		// 取消远程ppp拨号设置
		/*
		nTmpFlag = psIns->flag_pppoe;
		memcpy( sPIns.PPPOEUser, psIns->pppoe_user_name, sizeof( psIns->pppoe_user_name ) );
		memcpy( sPIns.PPPOEPasswd, psIns->pppoe_passwd, sizeof( psIns->pppoe_passwd ) );
		if(nTmpFlag != sPIns.PPPOEFlag)
		{
			if(nTmpFlag)
				NetComm_PPPoE_Start(sPIns.PPPOEUser, sPIns.PPPOEPasswd);
			else
				NetComm_PPPoE_Stop(0);
				
			sPIns.PPPOEFlag 	=  nTmpFlag;
		}
		*/
		
		sPIns.McastFlag = (psIns->flag_dhcp&0xf0)?1:0;
		
		nTmpFlag = (psIns->flag_dhcp&0x0f)?1:0;
		if(nTmpFlag != sPIns.DhcpFlag)
		{
			if(nTmpFlag)
				NetComm_StartDHCP();
			else
				NetComm_StopDHCP();
			
			sPIns.DhcpFlag  = nTmpFlag;	
		}
		
		#ifdef FIX_DDNS_BUG
		if(psIns->flag_ddns > 0)
		{
			int value = 0;
			BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_DDNSDOMAIN,psIns->ddnsserver,&value);
			nTmpFlag = value + 1;
		}
		else
		{
			nTmpFlag = 0;
		}
		#else
		nTmpFlag = psIns->flag_ddns>0?(psIns->ddnsserver+1):0;
		#endif
		
		memcpy( sPIns.DDNSDomain, psIns->ddns_domain, sizeof( psIns->ddns_domain ) );
		memcpy( sPIns.DDNSUser, psIns->ddns_user, sizeof( psIns->ddns_user ) );
		memcpy( sPIns.DDNSPasswd, psIns->ddns_passwd, sizeof( psIns->ddns_passwd ) );
		
		//printf("domain %s user %s passwd %s \n", sPIns.DDNSDomain, sPIns.DDNSUser, sPIns.DDNSPasswd);
		
		//if(nTmpFlag != sPIns.DDNSFlag)//csp modify 20130422
		{
			//if(nTmpFlag)//csp modify 20130422
			{
				pthread_t ddnsThrxd;
				memset(&sDdnsPara, 0, sizeof(sDdnsPara));
				sDdnsPara.eProt = nTmpFlag;
				strcpy(sDdnsPara.szDomain, sPIns.DDNSDomain);
				strcpy(sDdnsPara.szUser, sPIns.DDNSUser);
				strcpy(sDdnsPara.szPasswd, sPIns.DDNSPasswd);
				
				pthread_create(&ddnsThrxd, NULL, DdnsThrxd, (void*)&sDdnsPara);
				
				//NetComm_DDNSIpUpdate();
			}
			sPIns.DDNSFlag 	= nTmpFlag;
		}
		
		NetComm_WritePara((SNetPara*)&sPIns);
		
		ConfigSetNetPara(&sPIns, 0);
		
		if(bReset)
		{
			pthread_t resetThrxd;
			pthread_create(&resetThrxd, NULL, ResetThrxd, NULL);
		}
		else
		{
			SMultiCastInfo sInfo;
			NetComm_GetMultiCastInfo(&sInfo);
			sInfo.deviceIP 		= sPIns.HostIP;
			sInfo.devicePort	= sPIns.TCPPort;
			NetComm_SetMultiCastInfo(&sInfo);
		}
	}
}

void dealRemoteCmd_ResetNotify(void)
{
	SBizEventPara sBizEventPara;
	sBizEventPara.emType = EM_BIZ_EVENT_NET_CHANGEPARA_RESET;
	SendBizEvent(&sBizEventPara);
}

void DealRemoteCmd_Getvgasollist(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_VGA_Pro vgaList[] = {
		{ 1, 1, 60},//cvbs
		{ 800, 600, 60 },
		{ 1024, 768, 60 },
		{ 1280, 1024, 60 },
		{ 1280, 720, 60 },
		{ 1920, 1080, 60 },
	};
	
	ifly_VGA_Solution* psIns = &pRslt->sBasicInfo.VGA_Solution;
	//memset(psIns, 0, sizeof(vgaList));
	memset(psIns, 0, sizeof(ifly_VGA_Solution));
	
	InitStrList();
	
	//获得 配置 
	u8 real = 0;
	int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_OUTPUT, (s8**)g_strList, (u8*)&real, 20, 64);
	printf("VGAListNum=%d,ret=%d\n",real,ret);
	if(ret == 0)
	{
		//csp modify
		//u8 nResoNum = sizeof(vgaList)/sizeof(vgaList[0]);
		//if(real>0 && real<=nResoNum)
		//{
		//	memset(psIns->vgapro, 0, sizeof(vgaList));
		//	memcpy(psIns->vgapro, vgaList, real*sizeof(ifly_VGA_Pro));
		//}
		
		memset(psIns->vgapro, 0, sizeof(psIns->vgapro));
		
		int i=0,j=0;
		for(i=0;i<real;i++)
		{
			//printf("VGA List index%d:%s\n",i,g_strList[i]);
			if(strcasestr(g_strList[i],"Cvbs"))
			{
				psIns->vgapro[j++] = vgaList[0];
			}
			else if(strcasestr(g_strList[i],"Vga800x600"))
			{
				psIns->vgapro[j++] = vgaList[1];
			}
			else if(strcasestr(g_strList[i],"Vga1024x768"))
			{
				psIns->vgapro[j++] = vgaList[2];
			}
			else if(strcasestr(g_strList[i],"Vga1280x1024"))
			{
				psIns->vgapro[j++] = vgaList[3];
			}
			else if(strcasestr(g_strList[i],"Hdmi720p"))
			{
				psIns->vgapro[j++] = vgaList[4];
			}
			else if(strcasestr(g_strList[i],"Hdmi1080p"))
			{
				psIns->vgapro[j++] = vgaList[5];
			}
		}
	}
}

//csp modify 20130422
void DealRemoteCmd_Getddnslist(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_DDNSList_t* psIns = &pRslt->sBasicInfo.DDNS_List;
	memset(psIns, 0, sizeof(ifly_DDNSList_t));
	
	InitStrList();
	
	//获得 配置 
	u8 real = 0;
	int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_DDNSDOMAIN, (s8**)g_strList, (u8*)&real, 20, 64);
	printf("DDNSListNum=%d,ret=%d\n",real,ret);
	if(ret == 0)
	{
		int i;
		for(i=0;i<real;i++)
		{
			strncpy(psIns->DDNSlist[i],g_strList[i],sizeof(psIns->DDNSlist[i]));
		}
	}
}

void DealRemoteCmd_Getsysparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_SysParam_t* psIns = &pRslt->sBasicInfo.SysParam;
	
	SBizSystemPara bizSysPara;
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	int ret = BizGetPara( &bizTar, &bizSysPara );
	if( 0 == ret )
	{
		memset(psIns->device_name, 0, sizeof(psIns->device_name));
		if(strlen(bizSysPara.strDevName) >= sizeof(bizSysPara.strDevName))
		{
			bizSysPara.strDevName[31] = '\0';
		}
		
		strcpy(psIns->device_name, bizSysPara.strDevName);
		
		psIns->device_id = bizSysPara.nDevId;// = 127;//设备id
		
		/*
		//printf("dev: %s, id: %d lang %d vga %d cr %d\n", 
			psIns->device_name, psIns->device_id, bizSysPara.nLangId, 
			bizSysPara.nOutput, bizSysPara.nCyclingRecord );
		*/
		
		if(psIns->language != bizSysPara.nLangId)
		{
			psIns->language = bizSysPara.nLangId;
		}
		
		psIns->flag_overwrite = bizSysPara.nCyclingRecord;//硬盘满时覆盖标记
		
		switch(bizSysPara.nVideoStandard)
		{
			case EM_BIZ_PAL:
			{
				psIns->video_format = 0;				//视频制式
			} break;
			case EM_BIZ_NTSC:
			{
				psIns->video_format = 1;				//视频制式
			} break;
		}

		//csp modify
		//if(psIns->vga_solution != bizSysPara.nOutput)
		{
			//psIns->vga_solution = bizSysPara.nOutput;
			u8 idx = 0;
			BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_OUTPUT, bizSysPara.nOutput, &idx);
			psIns->vga_solution = idx;
		}
		
		psIns->lock_time = bizSysPara.nLockTime;		//键盘锁定时间		
		psIns->disable_pw = bizSysPara.nAuthCheck;
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
		SBizCfgPreview bizPreCfg;
		int ret = BizGetPara(&bizTar, &bizPreCfg);
		if(ret==0)
		{
			psIns->flag_statusdisp = bizPreCfg.nShowRecState;
		}
		//u16		switch_time;				//切换时间
		//u8		flag_statusdisp;			//状态显示标记
		//u8		transparency;				//菜单透明度
		//u8		disable_pw;					//密码禁用
	}
}

void DealRemoteCmd_Setsysparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	u8 bReset = 0;
	
	ifly_SysParam_t* psIns = &pCmd->sReq.SysParam;

	//csp modify 20150110
	printf("DealRemoteCmd_Setsysparam set language=%d\n", psIns->language);
	psIns->language = 1;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetPara(&bizTar, &bizPreCfg);
	if(ret == 0)
	{
		bizPreCfg.nShowRecState = psIns->flag_statusdisp;
		BizSetPara(&bizTar, &bizPreCfg);
	}
	
	SBizSystemPara bizSysPara;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;	
	ret = BizGetPara(&bizTar, &bizSysPara);
	if(0 == ret)
	{
		memset(bizSysPara.strDevName, 0, sizeof(bizSysPara.strDevName));
		strcpy(bizSysPara.strDevName, psIns->device_name);
		
		SMultiCastInfo sInfo;
		NetComm_GetMultiCastInfo(&sInfo);
		strcpy( sInfo.device_name, psIns->device_name );
		NetComm_SetMultiCastInfo(&sInfo);
		
		bizSysPara.nDevId = psIns->device_id;//设备id
		
		if(bizSysPara.nLangId != psIns->language)
		{
			//printf("nLangId change !\n");
			bReset = 1;
			bizSysPara.nLangId = psIns->language;		
			BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_LANGUAGE  );
		}
		
		bizSysPara.nCyclingRecord = psIns->flag_overwrite;//硬盘满时覆盖标记
		
		int nVs = psIns->video_format?EM_BIZ_NTSC:EM_BIZ_PAL;
		if(bizSysPara.nVideoStandard != nVs)
		{
			BizNetStopEnc();
			BizRecordStopAll();
			
			//printf("nVideoStandard change !\n");
			bReset = 1;
			BizNetWriteLog( pCmd->cph, BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_VIDEOFORMAT );
			bizSysPara.nVideoStandard = nVs;//视频制式
			
			#if 1
			//Modify by Lirl on Dec/07/2011,制式改变时转换到对应制式的满帧
			if(EM_BIZ_NTSC==(EMBIZVIDSTANDARD)BizGetVideoFormatReal())
			{
               SBizEncodePara sBizEncParaMain;
               SBizEncodePara sBizEncParaSub;
			   int i;
               for (i=0; i<BizGetMaxChnNum(); i++)
               {
                    BizGetVMainEncodePara(0,&sBizEncParaMain, i);
                    if (30 == sBizEncParaMain.nFrameRate ||
                        0 == sBizEncParaMain.nFrameRate)
                    {
                        sBizEncParaMain.nFrameRate = 25;
                        BizSetVMainEncodePara(&sBizEncParaMain, i);
                    }

                    BizGetVSubEncodePara(0,&sBizEncParaSub, i);
                    if (30 == sBizEncParaSub.nFrameRate ||
                        0 == sBizEncParaSub.nFrameRate)
                    {
                        sBizEncParaSub.nFrameRate = 25;
                        BizSetVSubEncodePara(&sBizEncParaSub, i);
                    }
               }
			}
			
			if(EM_BIZ_PAL==(EMBIZVIDSTANDARD)BizGetVideoFormatReal())
			{
               SBizEncodePara sBizEncParaMain;
               SBizEncodePara sBizEncParaSub;
			   int i;
               for(i=0; i<BizGetMaxChnNum(); i++)
               {
                    BizGetVMainEncodePara(0,&sBizEncParaMain, i);
                    if (25 == sBizEncParaMain.nFrameRate ||
                       0 == sBizEncParaMain.nFrameRate)
                    {
                        sBizEncParaMain.nFrameRate = 30;
                        BizSetVMainEncodePara(&sBizEncParaMain, i);
                    }
					
                    BizGetVSubEncodePara(0,&sBizEncParaSub, i);
                    if (25 == sBizEncParaSub.nFrameRate ||
                        0 == sBizEncParaSub.nFrameRate)
                    {
                        sBizEncParaSub.nFrameRate = 30;
                        BizSetVSubEncodePara(&sBizEncParaSub, i);
                    }
               }
			}
			//end
            #endif
		}
		
		//printf("nOutput chnum %d vgasolution %d!\n", GetVideoMainNum(), psIns->vga_solution);
		
		#if 0//csp modify
		if(bizSysPara.nOutput != psIns->vga_solution)
		{
			//printf("nOutput change!\n");
			bReset = 1;
			bizSysPara.nOutput = psIns->vga_solution;//VGA分辨率 
		}
		#else
		s32 value = bizSysPara.nOutput;
		BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_OUTPUT, psIns->vga_solution, &value);
		//printf("vga : (%d,%d,%d)\n",psIns->vga_solution,value,bizSysPara.nOutput);
		if(bizSysPara.nOutput != value)
		{
			//printf("nOutput change!\n");
			bReset = 1;
			bizSysPara.nOutput = value;//VGA分辨率 
		}
		#endif
		
		#if 0//csp modify
		bizSysPara.nLockTime = psIns->lock_time;//键盘锁定时间	
		#endif
		
		BizSetPara(&bizTar, &bizSysPara);
		
		//printf("BizSetPara over\n");
		//fflush(stdout);
		
		//u16		switch_time;				//切换时间
		//u8		transparency;				//菜单透明度
		//u8		disable_pw;				//密码禁用
	}
	
	//printf("Remote_Setsysparam end,bReset=%d......\n",bReset);
	//fflush(stdout);
	
	if(bReset)
	{
		pthread_t resetThrxd;
		pthread_create(&resetThrxd, NULL, ResetThrxd, NULL);
	}
}

void DealRemoteCmd_Getrecparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget bizTar;
	SBizRecPara bizRecPara;
	
	int					i;
	u8                  nCh   = pCmd->sReq.nChn;
	ifly_RecordParam_t* psIns = &pRslt->sBasicInfo.RecordParam;
	bizTar.nChn 			= nCh;
	bizTar.emBizParaType 	= EM_BIZ_RECPARA;
	int ret = BizGetPara(&bizTar, &bizRecPara);
	if( 0 == ret )
	{
		psIns->channelno 	= nCh;
		psIns->code_type 	= bizRecPara.nStreamType;
		psIns->pre_record 	= bizRecPara.nPreTime;
		psIns->post_record 	= bizRecPara.nRecDelay;
		psIns->supportResolu = 1;
		
		SBizEncodePara 	sEncIns;
		u8				bValidType = 0;
		
		bValidType			 = 1;
		bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
		
		if( bValidType )
		{
			int ret = BizGetPara( &bizTar, &sEncIns );
			if( 0 == ret )
			{
				switch(sEncIns.nBitRateType)
				{
					case EM_BIZENC_BITRATE_VARIABLE:
					{
						psIns->bit_type 	= 1;
					}break;
					case EM_BIZENC_BITRATE_CONST:
					{
						psIns->bit_type 	= 0;
					}break;
				}
				psIns->bit_max 		= sEncIns.nBitRate;
				psIns->intraRate 	= sEncIns.nGop;
				psIns->frame_rate 	= sEncIns.nFrameRate;
				psIns->quality 		= sEncIns.nPicLevel;
				psIns->minQ 		= sEncIns.nMinQP;
				psIns->maxQ 		= sEncIns.nMaxQP;
				
				int Hnum = 0;//cw_9508S
				EMBIZCFGPARALIST emParaType;
				Hnum=GetBizResolHNum();
				if(psIns->channelno < Hnum)
					emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL;
				else
					emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL2;
				
				BizConfigGetParaListIndex(
					emParaType, 
					sEncIns.nVideoResolution, 
					&psIns->resolutionpos
				);
			}
		}
	}
}

void DealRemoteCmd_Setrecparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget bizTar;
	SBizRecPara bizRecPara;
	
	ifly_RecordParam_t* psIns = &pCmd->sReq.RecordParam;
	
	bizTar.nChn 				= psIns->channelno;
	bizTar.emBizParaType 		= EM_BIZ_RECPARA;
	int ret = BizGetPara(&bizTar, &bizRecPara);
	if( 0 == ret )
	{
		bizRecPara.nStreamType 	= 	psIns->code_type;
		bizRecPara.nPreTime		=	psIns->pre_record;
		bizRecPara.nRecDelay	=	psIns->post_record;
		
		BizSetPara(&bizTar, &bizRecPara);
		
		SBizEncodePara sEncIns;
		
		bizRecPara.nEncChnType = 1;
		
		#if 0//csp modify
		u8 *pnReslist;
		pnReslist			= sMainStResoList;
		#endif
		
		bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
		
		//printf("encchntype %d\n", bizTar.emBizParaType);
		
		int ret = BizGetPara( &bizTar, &sEncIns );
		if(0 == ret)
		{
		#if 1//csp modify
			int Hnum = 0;//cw_9508S
			EMBIZCFGPARALIST emParaType;
			Hnum=GetBizResolHNum();
			if(psIns->channelno < Hnum)
				emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL;
			else
				emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL2;
			
			s32 value = EM_BIZENC_RESOLUTION_CIF;
			BizConfigGetParaListValue(emParaType,psIns->resolutionpos,&value);
			sEncIns.nVideoResolution = value;
			printf("chn%d main stream resolution:%d,Hnum:%d\n",bizTar.nChn,sEncIns.nVideoResolution,Hnum);
		#else
			if(psIns->resolutionpos<(sizeof(sMainStResoList)/sizeof(sMainStResoList[0])))
			{
				switch( pnReslist[psIns->resolutionpos] )
				{
					case VIDEORESOLU_CIF:
					{
						sEncIns.nVideoResolution = EM_BIZENC_RESOLUTION_CIF;
					} break;						
					case VIDEORESOLU_D1:
					{
						sEncIns.nVideoResolution = EM_BIZENC_RESOLUTION_D1;
					} break;
					case VIDEORESOLU_720P:
					{
						sEncIns.nVideoResolution = EM_BIZENC_RESOLUTION_720P;
					} break;						
					case VIDEORESOLU_1080P:
					{
						sEncIns.nVideoResolution = EM_BIZENC_RESOLUTION_1080P;
					} break;
					default:
						return;
				}
			}
		#endif
			
			sEncIns.nBitRateType	=	psIns->bit_type?EM_BIZENC_BITRATE_VARIABLE:EM_BIZENC_BITRATE_CONST;//psIns->bit_type?0:1;
			sEncIns.nBitRate		=	psIns->bit_max;
			sEncIns.nGop			=	psIns->intraRate;
			sEncIns.nFrameRate		=	psIns->frame_rate;
			sEncIns.nPicLevel		=	psIns->quality;
			sEncIns.nMinQP			=	psIns->minQ;
			sEncIns.nMaxQP			=	psIns->maxQ;
			
			BizSetPara( &bizTar, &sEncIns );
		}
	}
	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_REC_CODESTREAM  );
}


void DealRemoteCmd_Getimgparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	SBizParaTarget 		sParaTgtIns;	
	ifly_ImgParam_t* 	pImaginfo;	
	
	pImaginfo 		  			= &pRslt->sBasicInfo.ImgParam;
	
	sParaTgtIns.nChn 			= pCmd->sReq.nChn;
	
	SBizCfgStrOsd 		sStrOds;	
	memset( &sStrOds, 0, sizeof( sStrOds ) );
	sParaTgtIns.emBizParaType 	= EM_BIZ_STROSD;
	int rtn = BizGetPara( &sParaTgtIns, &sStrOds );
	if(0==rtn)
	{	
		//u8		flag_safechn;				//安全通道标记
	
		pImaginfo->channel_no = pCmd->sReq.nChn;
		
		pImaginfo->flag_name = sStrOds.nEncShowChnName;		
		pImaginfo->timepos_x = sStrOds.sEncTimePos.x*2;
		pImaginfo->timepos_y = sStrOds.sEncTimePos.y*2;
		
		pImaginfo->flag_time = sStrOds.nEncShowTime;		
		pImaginfo->chnpos_x = sStrOds.sEncChnNamePos.x*2;
		pImaginfo->chnpos_y = sStrOds.sEncChnNamePos.y*2;

		strcpy(pImaginfo->channelname, sStrOds.strChnName);
#if 0		//无法或者难道较大检测字符串是何种编码
		//yaogang modify 20141103
		/*
		保证之后的编码都是"GB18030"，
		假定pImaginfo->channelname是"UTF-8"，如果"UTF-8" to "GB18030" 失败，
		那么认为之前的编码就是"GB18030" ，无须转换
		*/
		char outbuf[48];
		memset(outbuf, 0, sizeof(outbuf));
		size_t inbytesleft = strlen(pImaginfo->channelname);
		size_t outbytesleft = sizeof (outbuf);
		char *inptr = pImaginfo->channelname;
		char *outptr = (char *) outbuf;

		printf("DealRemoteCmd_Getimgparam before iconv :\n");
		//int i;
		for(i=0; i<strlen(inptr); i++)
			printf("\t 0x%x\n", inptr[i]);
		
	//iconv_t iconv_open (const char* tocode, const char* fromcode);
		iconv_t cd = iconv_open ("GB18030", "UTF-8");
		if (cd == (iconv_t)(-1)) 
		{
			printf("iconv_open failed\n");
		} 
		else 
		{
			size_t r = iconv (cd,
			                  (char **) &inptr, &inbytesleft,
			                  (char **) &outptr, &outbytesleft);

			if (r == (size_t)(-1) )
			{
				
				printf("DealRemoteCmd_Getimgparam iconv() UTF-8 to GB18030 failed r: %d\n", r);
			}
			else
			{
				printf("DealRemoteCmd_Getimgparam iconv UTF-8 to GB18030 success\n");
				printf("DealRemoteCmd_Getimgparam after iconv :\n");
				//int i;
				strcpy(pImaginfo->channelname, outbuf);
				
				for(i=0; i<outbytesleft; i++)
					printf("\t 0x%x\n", outbuf[i]);
				
			}
		}
		iconv_close(cd);
		//yaogang modify 20141103 end
#endif
	}
	
	SBizCfgRectOsd 		sRectOds;
	memset( &sRectOds, 0, sizeof( sRectOds ) );
	sRectOds.nMaxNum = MAX_COVER_REGION_NUM;
	sParaTgtIns.emBizParaType = EM_BIZ_RECTOSd;
	SRect sRect[MAX_COVER_REGION_NUM];//zlb20111117 去掉部分malloc
	sRectOds.psRectOsd = sRect;
	rtn = BizGetPara( &sParaTgtIns, &sRectOds );
	if(0==rtn)
	{
		u8 nOsdRectNum = 0;
		nOsdRectNum = sRectOds.nRealNum;

		pImaginfo->flag_mask = 1; // sRectOds.nShowRectOsd;

		memset(pImaginfo->MaskInfo, 0, sizeof(pImaginfo->MaskInfo));
		for ( i = 0; i < nOsdRectNum; i++ )
		{
			pImaginfo->MaskInfo[i].x = sRectOds.psRectOsd[i].x;
			pImaginfo->MaskInfo[i].y = sRectOds.psRectOsd[i].y;
			pImaginfo->MaskInfo[i].width = sRectOds.psRectOsd[i].w;
			pImaginfo->MaskInfo[i].height = sRectOds.psRectOsd[i].h;
		}
	}
	
	//if(sRectOds.psRectOsd) free(sRectOds.psRectOsd); //zlb20111117 去掉部分malloc
}

//yaogang modify 20170715 简易设置通道名的接口
void DealRemoteCmd_SetChnName(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget 		sParaTgtIns;	
	ifly_ImgParam_t* 	pImaginfo = &pCmd->sReq.ImgParam;
	
	sParaTgtIns.nChn 			= pCmd->sReq.nChn;
	
	SBizCfgStrOsd 		sStrOds;	
	memset( &sStrOds, 0, sizeof( sStrOds ) );
	sParaTgtIns.emBizParaType 	= EM_BIZ_STROSD;
	
	int rtn = BizGetPara( &sParaTgtIns, &sStrOds );
	if(0==rtn)
	{		
		int nChLMax = GetChnNameMax();

		if( strncmp(sStrOds.strChnName, pImaginfo->channelname, nChLMax) != 0 )
		{
			strncpy(sStrOds.strChnName, pImaginfo->channelname, nChLMax);
			BizSetPara( &sParaTgtIns, &sStrOds );
			
			BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_CHN_NAME  );
		}
	}	
}


void DealRemoteCmd_GetChnName(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int rtn = 0;
	int chn = pCmd->sReq.nChn;
	ifly_ImgParam_t* 	pImaginfo= &pRslt->sBasicInfo.ImgParam;
	char ChnName[32];
	int flag_from_ipc = 1;//从IPC获取OSD成功

	//get osd from ipc
	memset(ChnName, 0, sizeof(ChnName));	
	rtn = IPC_CMD_GetOSD(chn, ChnName, sizeof(ChnName));
	if (0 == ChnName[0])//(rtn || 0 == ChnName[0])
	{
		printf("%s IPC_CMD_GetOSD chn%d failed\n", __func__, chn);
		flag_from_ipc = 0;
	}
	else
	{
		strcpy(pImaginfo->channelname, ChnName);
	}

	//get osd from config
	SBizParaTarget 		sParaTgtIns;
	sParaTgtIns.nChn 			= chn;
	
	SBizCfgStrOsd 		sStrOds;	
	memset(&sStrOds, 0, sizeof( sStrOds ) );
	sParaTgtIns.emBizParaType 	= EM_BIZ_STROSD;
	
	rtn = BizGetPara( &sParaTgtIns, &sStrOds );
	if(0 == rtn)
	{
		if (flag_from_ipc)
		{
			if (strcmp(sStrOds.strChnName, ChnName))
			{
				strcpy(sStrOds.strChnName, ChnName);
				BizSetPara( &sParaTgtIns, &sStrOds );
			}
		}
		else
		{
			strcpy(pImaginfo->channelname, sStrOds.strChnName);
		}
	}
	
	//printf("%s chn%d name: %d,%s\n", __func__, sParaTgtIns.nChn, strlen(pImaginfo->channelname), pImaginfo->channelname);
	//if(sRectOds.psRectOsd) free(sRectOds.psRectOsd); //zlb20111117 去掉部分malloc
}


// unicode / utf8 exchange

void Unicode2Utf8(u16 unicode, char* utf8)
{
	utf8[0] = 0xE0|(unicode>>12);
	utf8[1] = 0x80|((unicode>>6)&0x3F);
	utf8[2] = 0x80|(unicode&0x3F);
}

void Utf82Unicode(char* utf8, u16* unicode)
{
	*unicode = (utf8[0] & 0x1F) << 12;
	*unicode |= (utf8[1] & 0x3F) << 6;
	*unicode |= (utf8[2] & 0x3F);
}

void StrUnicode2Utf8( char* pUni, char* pUtf )
{
	char* pTmp = pUtf;
	
	while(*pUni)
	{
		if(*pUni>0x80)
		{
			Unicode2Utf8(*(u16*)pUni, pTmp);
			pUni += 2;
			pTmp += 3;
		}
		else
		{
			*pTmp = *pUni;
			pTmp++; pUni++;			
		}
	}
	*pTmp = '\0';
}

void StrUtf82Unicode( char* pUtf, char* pUni )
{
	char* pTmp = pUni;
	u16 uni;
	
	while(*pUtf)
	{
		if(*pUtf>0x80)
		{
			Utf82Unicode(pUtf, &uni);
			pUtf += 3;
			memcpy( pTmp, &uni, 2 );
			pTmp += 2;
		}
		else
		{
			*pTmp = *pUtf;
			pTmp++; pUtf++;			
		}
	}
	
	*pTmp = '\0';
}

// 15个中文标点自动转换为英文标点
// 其他中文符号一律转为空格
#define CHN_NAME_LEN_MAX	32

void Excmd_Chm2Enm(char* pSrc)
{
	int  i   = 0;
	int  j   = 0;
	u16  uni = 0;
	char tmp[CHN_NAME_LEN_MAX] = {0};

	if(!pSrc) return;

	strcpy(tmp, pSrc);
	int len = strlen(pSrc);
	memset(pSrc, 0, len);
	for(i=0; i<len; )
	{
		if(tmp[i] >= 128)
		{
			Utf82Unicode(tmp+i, &uni);
			if(uni == 0x201C || uni == 0x201D || uni == 0x3002 )
			{
				switch(uni)
				{
					case 0x3002:
					{
						pSrc[j++] = '.';						
					} break;
					case 0x201C:
					case 0x201D:
					{
						pSrc[j++] = '"';
					} break;
				}
			}
			else if(uni>=0xFF01 && uni<=0xFF0F)
			{
				pSrc[j] = uni-0xFF00+0x20;
				j++;
			}
			else
			{
				pSrc[j] = tmp[i];
				pSrc[j+1] = tmp[i+1];
				pSrc[j+2] = tmp[i+2];
				
				j += 3;
			}
			i += 3;
		}
		else
		{
			pSrc[i] = tmp[i];
			i++;
			j++;
		}
	}
}

extern int GetChnNameMax( void );

void DealRemoteCmd_Setimgparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget 		sParaTgtIns;	
	ifly_ImgParam_t* 	pImaginfo;	
	
	pImaginfo 		  			= &pCmd->sReq.ImgParam;
	
	sParaTgtIns.nChn 			= pCmd->sReq.nChn;

	int i;
	for(i=0; i<6; i++)
			printf("0x%x\n",  pImaginfo->channelname[i]);
	
	SBizCfgStrOsd 		sStrOds;	
	memset( &sStrOds, 0, sizeof( sStrOds ) );
	sParaTgtIns.emBizParaType 	= EM_BIZ_STROSD;	
	int rtn = BizGetPara( &sParaTgtIns, &sStrOds );
	if(0==rtn)
	{		
		sStrOds.nEncShowChnName = pImaginfo->flag_name?1:0;
		sStrOds.nShowChnName = sStrOds.nEncShowChnName;
		#if 1
		int nChLMax = GetChnNameMax();
		
		//Excmd_Chm2Enm(pImaginfo->channelname);
#if 0
		//yaogang modify 20141103
		/*
		保证之后的编码都是"UTF-8"，
		假定pImaginfo->channelname是"GB18030"，如果"GB18030" to "UTF-8" 失败，
		那么认为之前的编码就是"UTF-8" ，无须转换
		*/
		char outbuf[48];
		memset(outbuf, 0, sizeof(outbuf));
		size_t inbytesleft = strlen(pImaginfo->channelname);
		size_t outbytesleft = sizeof (outbuf);
		char *inptr = pImaginfo->channelname;
		char *outptr = (char *) outbuf;

		printf("DealRemoteCmd_Setimgparam before iconv :\n");
		//int i;
		for(i=0; i<strlen(inptr); i++)
			printf("\t 0x%x\n", inptr[i]);
		
	//iconv_t iconv_open (const char* tocode, const char* fromcode);
		iconv_t cd = iconv_open ("UTF-8", "GB18030");
		if (cd == (iconv_t)(-1)) 
		{
			printf("iconv_open failed\n");
		} 
		else 
		{
			size_t r = iconv (cd,
			                  (char **) &inptr, &inbytesleft,
			                  (char **) &outptr, &outbytesleft);

			if (r == (size_t)(-1) )
			{
				
				printf("DealRemoteCmd_Setimgparam iconv() GB18030 to UTF-8 failed r: %d\n", r);
				
			}
			else
			{
				printf("DealRemoteCmd_Setimgparam iconv GB18030 to UTF-8 success\n");
				printf("DealRemoteCmd_Setimgparam after iconv :\n");
				//int i;
				strcpy(pImaginfo->channelname, outbuf);
				
				for(i=0; i<outbytesleft; i++)
					printf("\t 0x%x\n", outbuf[i]);
			}
		}
		iconv_close(cd);
		//yaogang modify 20141103 end
#endif
		
		if( strncmp(sStrOds.strChnName, pImaginfo->channelname, nChLMax) != 0 )
		{
			strncpy(sStrOds.strChnName, pImaginfo->channelname, nChLMax);
			BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_CHN_NAME  );
		}
		#else
		char* pUtf8 = malloc(strlen(pImaginfo->channelname)*3/2);
		StrUnicode2Utf8(pImaginfo->channelname, pUtf8);
		if( strcmp(sStrOds.strChnName, pUtf8) != 0 )
		{
			strcpy(sStrOds.strChnName, pUtf8);
			BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_CHN_NAME  );
		}
		if(pUtf8) free(pUtf8);
		#endif

		sStrOds.nEncShowTime = pImaginfo->flag_time?1:0;
		
		sStrOds.sEncTimePos.x = pImaginfo->timepos_x/2;
		sStrOds.sEncTimePos.y = pImaginfo->timepos_y/2;
		
		sStrOds.sEncChnNamePos.x = pImaginfo->chnpos_x/2;
		sStrOds.sEncChnNamePos.y = pImaginfo->chnpos_y/2;

		/*
		printf("ch %d pos %d %d %d %d show %d %d\n",

			pCmd->sReq.nChn,
			sStrOds.sEncTimePos.x, // = pImaginfo->timepos_x;
			sStrOds.sEncTimePos.y, // = pImaginfo->timepos_y;
			
			sStrOds.sEncChnNamePos.x, // = pImaginfo->chnpos_x;
			sStrOds.sEncChnNamePos.y, // = pImaginfo->chnpos_y;
			pImaginfo->flag_name,
			pImaginfo->flag_time

		);
		*/
		
		BizSetPara( &sParaTgtIns, &sStrOds );
	}

	/*
	sParaTgtIns.emBizParaType 	= EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;	
	rtn = BizGetPara( &sParaTgtIns, &bizPreCfg );
	if(0==rtn)
	{
		bizPreCfg.nShowTime = pImaginfo->flag_time?1:0;
		BizSetPara( &sParaTgtIns, &bizPreCfg );
	}
	*/

	#if 1
	SBizCfgRectOsd 		sRectOds;	
	
	memset( &sRectOds, 0, sizeof( sRectOds ) );
	sRectOds.nMaxNum = MAX_COVER_REGION_NUM;
	sParaTgtIns.emBizParaType 	= EM_BIZ_RECTOSd;
	SRect sRect[MAX_COVER_REGION_NUM]; //zlb20111117 去掉部分malloc
	sRectOds.psRectOsd = sRect;
	
	rtn = BizGetPara( &sParaTgtIns, &sRectOds );
	////printf("rect osd num %d rtn %d\n", sRectOds.nRealNum, rtn);
	if(0==rtn)
	{
		int i;
		
		SBizDvrInfo sInfo;
		ConfigGetDvrInfo(0, &sInfo);
		
		u8 nMaxRectOsdNumPerChn = sInfo.nRectOsdNum/sInfo.nVidMainNum;
		if(nMaxRectOsdNumPerChn>MAX_COVER_REGION_NUM)
		{
			nMaxRectOsdNumPerChn = MAX_COVER_REGION_NUM;
		}
		
		sRectOds.nRealNum = 0;
		sRectOds.nMaxNum = nMaxRectOsdNumPerChn;
		for(i = 0; i < sRectOds.nMaxNum; i++)
		{
			sRectOds.psRectOsd[i].x = pImaginfo->MaskInfo[i].x;
			sRectOds.psRectOsd[i].y = pImaginfo->MaskInfo[i].y;
			sRectOds.psRectOsd[i].w = pImaginfo->MaskInfo[i].width;
			sRectOds.psRectOsd[i].h = pImaginfo->MaskInfo[i].height;
			
			/*
			//printf("rect %d %d %d %d %d %d\n", 
				sRectOds.psRectOsd[i].x,// = pImaginfo->MaskInfo[i].x;
				sRectOds.psRectOsd[i].y,// = pImaginfo->MaskInfo[i].y;
				sRectOds.psRectOsd[i].w,// = pImaginfo->MaskInfo[i].width;
				sRectOds.psRectOsd[i].h, // = pImaginfo->MaskInfo[i].height;
				sRectOds.nMaxNum,
				sRectOds.nRealNum
			);
			*/
			
			if(sRectOds.psRectOsd[i].w == 0 || sRectOds.psRectOsd[i].h == 0)
			{
				continue;
			}
			
			sRectOds.nRealNum++;
		}
		
		if(sRectOds.nRealNum > 0)
		{
			sRectOds.nShowRectOsd = 1;
		}
		
		BizSetPara( &sParaTgtIns, &sRectOds );
	}

	//if(sRectOds.psRectOsd) free(sRectOds.psRectOsd); //zlb20111117 去掉部分malloc
	
	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_OSD );
	#endif
}

void DealRemoteCmd_Getpicadj(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget 		sParaTgtIns;
	
	SBizPreviewImagePara sCfgIns;
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	ifly_PicAdjust_t* pGetpicadjustsrc = &pCmd->sReq.PicAdjust;
	sParaTgtIns.nChn 			= pGetpicadjustsrc->channel_no;
	sParaTgtIns.emBizParaType 	= EM_BIZ_IMAGEPARA;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		ifly_PicAdjust_t* pGetpicadjust = &pRslt->sBasicInfo.PicAdjust;

		memcpy(pGetpicadjust, pGetpicadjustsrc, sizeof(ifly_PicAdjust_t));
		
		if(0==pGetpicadjustsrc->flag) pGetpicadjust->val = sCfgIns.nBrightness;
		else if(1==pGetpicadjustsrc->flag) pGetpicadjust->val = sCfgIns.nContrast;
		else if(2==pGetpicadjustsrc->flag) pGetpicadjust->val = sCfgIns.nHue;
		else if(3==pGetpicadjustsrc->flag) pGetpicadjust->val = sCfgIns.nSaturation;
		//else printf("err CTRL_CMD_SETPICADJ\n");
	}
}


void DealRemoteCmd_Setpicadj(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget 		sParaTgtIns;
	
	SBizPreviewImagePara sCfgIns;
	memset( &sCfgIns, 0, sizeof( sCfgIns ) );
	ifly_PicAdjust_t* pGetpicadjustsrc = &pCmd->sReq.PicAdjust;
	sParaTgtIns.nChn 			= pGetpicadjustsrc->channel_no;
	sParaTgtIns.emBizParaType 	= EM_BIZ_IMAGEPARA;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{	
		if(0==pGetpicadjustsrc->flag) sCfgIns.nBrightness 		= pGetpicadjustsrc->val;
		else if(1==pGetpicadjustsrc->flag) sCfgIns.nContrast 	= pGetpicadjustsrc->val;
		else if(2==pGetpicadjustsrc->flag) sCfgIns.nHue 		= pGetpicadjustsrc->val;
		else if(3==pGetpicadjustsrc->flag) sCfgIns.nSaturation 	= pGetpicadjustsrc->val;
		//else //printf("err CTRL_CMD_SETPICADJ\n");

		//printf("pic %d \n", pGetpicadjustsrc->val);
		
		BizSetPara(&sParaTgtIns, &sCfgIns);
		
		BizPreviewSetImage(sParaTgtIns.nChn, &sCfgIns);
	}
	
	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_VO  );
}


void DealRemoteCmd_Getalarminparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i, j;
	ifly_AlarmInParam_t* pAlarminparam;
	
	pAlarminparam = &pRslt->sBasicInfo.AlarmInParam;
	memset( pAlarminparam, 0, sizeof(ifly_AlarmInParam_t) );
	
	u8 nAlarmInIdx = pCmd->sReq.nAlarmInId;
	
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmSensorPara sCfgIns;	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
	sParaTgtIns.nChn = nAlarmInIdx;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		pAlarminparam->inid   		= pCmd->sReq.nAlarmInId;
		pAlarminparam->flag_enable 	= sCfgIns.nEnable;
		pAlarminparam->typein 		= sCfgIns.nType==0?2:1; // pc 1-高 2 低; local 0-低 , 1 high
		
		u8 nIdx = 0;
		
		pAlarminparam->delay  = sCfgIns.nDelay>=0xff?0xffff:sCfgIns.nDelay;
		//printf("alarm in delay %d \n", sCfgIns.nDelay);
		
		pAlarminparam->flag_deal = 1; // 对设备端而言报警处理一直启用
	}
	
	//printf("alarm in en %d id %d typ %d delay %d\n", 
	//	sCfgIns.nEnable, pCmd->sReq.nAlarmInId, sCfgIns.nType, sCfgIns.nDelay);
	
	SBizAlarmDispatch sDsphCfgIns;
	memset(&sDsphCfgIns, 0, sizeof(sDsphCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
	sParaTgtIns.nChn = nAlarmInIdx;
	rtn = BizGetPara(&sParaTgtIns, &sDsphCfgIns);
	if(0==rtn)
	{
		for(i=0; i<GetVideoMainNum(); i++)
		{
			if(sDsphCfgIns.nRecordChn[i]!=0xff)
				pAlarminparam->triRecChn |= (1<<i);
			else
				pAlarminparam->triRecChn &= ~(1<<i);
				
			if(sDsphCfgIns.nAlarmOut[i]!=0xff)
				pAlarminparam->triAlarmoutid |= (1<<i);
			else
				pAlarminparam->triAlarmoutid &= ~(1<<i);
		}
		
		pAlarminparam->flag_buzz = sDsphCfgIns.nFlagBuzz;
		pAlarminparam->flag_email = sDsphCfgIns.nFlagEmail;
		pAlarminparam->flag_showfullscreen = sDsphCfgIns.nZoomChn;

		int nLinkNum = sizeof(pAlarminparam->AlarmInPtz)/sizeof(pAlarminparam->AlarmInPtz[0]);
		for( i=0; i<nLinkNum; i++)
		{
			GetPtzDeal(&pAlarminparam->AlarmInPtz[i], &sDsphCfgIns.sAlarmPtz[i]);
		}	
	}
}


void DealRemoteCmd_Setalarminparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i,j;
	ifly_AlarmInParam_t* pAlarminparam;

	pAlarminparam = &pCmd->sReq.AlarmInParam;
	
	u8 nAlarmInIdx = pCmd->sReq.nAlarmInId;

	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmSensorPara sCfgIns;	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
	sParaTgtIns.nChn = nAlarmInIdx;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);	
	if(0==rtn)
	{
		sCfgIns.nEnable = pAlarminparam->flag_enable;
		pCmd->sReq.nAlarmInId = pAlarminparam->inid;	
		sCfgIns.nType = pAlarminparam->typein==1?1:0; // pc 1-高 2 低; local 0-低 , 1 high
		
		s32 nVal = 0;
		//if(0 == BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_DELAY, pAlarminparam->delay, &nVal ))
			sCfgIns.nDelay = pAlarminparam->delay;
		
		//sCfgIns.nEnable = pAlarminparam->flag_deal;
		BizSetPara(&sParaTgtIns, &sCfgIns);
	}
	
	//printf("alarm in en %d id %d typ %d delay %d\n", 
	//	pAlarminparam->flag_enable, pAlarminparam->inid, pAlarminparam->typein, pAlarminparam->delay);
	
	SBizAlarmDispatch sDsphCfgIns;
	memset(&sDsphCfgIns, 0, sizeof(sDsphCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
	sParaTgtIns.nChn = nAlarmInIdx;
	rtn = BizGetPara(&sParaTgtIns, &sDsphCfgIns);
	if(0==rtn)
	{
		for(i=0; i<GetVideoMainNum(); i++)
		{				
			if(pAlarminparam->triRecChn & (1<<i))
				sDsphCfgIns.nRecordChn[i] = i;
			else
				sDsphCfgIns.nRecordChn[i] = 0xff;
			
			if(pAlarminparam->triAlarmoutid & (1<<i))
				sDsphCfgIns.nAlarmOut[i] = i;
			else
				sDsphCfgIns.nAlarmOut[i] = 0xff;
		}
		
		sDsphCfgIns.nFlagBuzz 	= pAlarminparam->flag_buzz;
		sDsphCfgIns.nFlagEmail = pAlarminparam->flag_email;
		sDsphCfgIns.nZoomChn = pAlarminparam->flag_showfullscreen;

		int nChMax = GetVideoMainNum();
		if(nChMax>sizeof(pAlarminparam->AlarmInPtz)/sizeof(pAlarminparam->AlarmInPtz[0])) 
			nChMax = sizeof(pAlarminparam->AlarmInPtz)/sizeof(pAlarminparam->AlarmInPtz[0]);
		
		for(j=0; j<nChMax; j++)
		{		
			// 设置联动预置点和联动巡航线之前需要确认目标预置点和巡航线是否可用
			//
			SBizParaTarget 	sParaTgtInsc;
			SBizPtzPara 	sCfgInsc;
			
			memset(&sCfgInsc, 0, sizeof(sCfgInsc));
			sParaTgtInsc.emBizParaType 	= EM_BIZ_PTZPARA;
			sParaTgtInsc.nChn 			= j;
			rtn = BizGetPara(&sParaTgtInsc, &sCfgInsc);		
			if(0==rtn)
			{
				rtn = SetPtzDeal(&pAlarminparam->AlarmInPtz[j], &sDsphCfgIns.sAlarmPtz[j], &sCfgInsc.sAdvancedPara);
			}
		}
		
		if(0 == rtn)
			BizSetPara(&sParaTgtIns, &sDsphCfgIns);	
		
		BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_SENSOR_ALARMDEAL);
	}
}

void DealRemoteCmd_Getalarmoutparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_AlarmOutParam_t* psIns = &pRslt->sBasicInfo.AlarmOutParam;
	
	SBizParaTarget 		sParaTgtIns;
	SBizAlarmOutPara 	sCfgIns;
	
	memset( &sCfgIns, 0, sizeof( sCfgIns ) );
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
	sParaTgtIns.nChn = pCmd->sReq.AlarmOutParam.outid;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		psIns->outid = pCmd->sReq.AlarmOutParam.outid;			//报警输出量 
		psIns->typeout = sCfgIns.nType==2?1:2;					//报警输出类型 pc 1 open 2 close; local 2 open 3 close
		//u32 copy2AlarmOutmask;									//复制到其他报警输出。按位
		
		//csp modify
		psIns->alarmoutdelay = sCfgIns.nDelay;
		//psIns->alarmoutdelay = htons(sCfgIns.nDelay);
		//u8 index = 0;
		//BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_DELAY, sCfgIns.nDelay, &index);
		//psIns->alarmoutdelay = htons((u16)index);
		
		SBizAlarmBuzzPara sCfgIns1;
		sParaTgtIns.emBizParaType = EM_BIZ_ALARMBUZZPARA;
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns1);
		if(0==rtn)
		{
			psIns->flag_buzz = sCfgIns1.nEnable;
			
			//csp modify
			psIns->buzzdelay = sCfgIns1.nDelay;
			//psIns->buzzdelay = htons(sCfgIns1.nDelay);
			//u8 index = 0;
			//BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_DELAY, sCfgIns1.nDelay, &index);
			//psIns->buzzdelay = htons((u16)index);
		}
		
		printf("alarmoutdelay %d buzzflag %d buzzdelay %d\n", psIns->alarmoutdelay, psIns->flag_buzz, psIns->buzzdelay);
	}
}

void DealRemoteCmd_Setalarmoutparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_AlarmOutParam_t* psIns = &pCmd->sReq.AlarmOutParam;
	
	SBizParaTarget 		sParaTgtIns;	
	SBizAlarmOutPara 	sCfgIns;
	
	memset( &sCfgIns, 0, sizeof( sCfgIns ) );
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
	sParaTgtIns.nChn = psIns->outid;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		int nType = psIns->typeout==1?2:3;
		sCfgIns.nDelay = (psIns->alarmoutdelay);
		//sCfgIns.nEnable = psIns->flag_buzz;
		
		if(sCfgIns.nType != nType)
		{
			BizSetPara(&sParaTgtIns, &sCfgIns);
			BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_SENSOR_TYPE  );
			sCfgIns.nType = nType;		//报警输出类型
			//u32 copy2AlarmOutmask;		//复制到其他报警输出。按位
		}
	}
	
	SBizAlarmBuzzPara sCfgIns1;
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMBUZZPARA;
	sParaTgtIns.nChn = psIns->outid;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns1);
	if(0==rtn)
	{
		sCfgIns1.nEnable = psIns->flag_buzz;
		sCfgIns1.nDelay = (psIns->buzzdelay);
		BizSetPara(&sParaTgtIns, &sCfgIns1);
	}
}

void DealRemoteCmd_Getrecschparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	
	SBizSchedule sCfgIns;
	SBizParaTarget sParaTgtIns;
	
	SCPRecSch*	psRecSch;
	ifly_RecordSCH_t* pRecordsch;
	
	psRecSch = &pCmd->sReq.sCPRecSch;
	pRecordsch = &pRslt->sBasicInfo.RecordSCH;

	pRecordsch->chn 	= psRecSch->nChn;
	pRecordsch->weekday = psRecSch->nWeek;
	
	sParaTgtIns.nChn 	= psRecSch->nChn;	
	sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECTIMING;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		for(i=0; i<SEGMENTS_OF_DAY; i++)
		{
			pRecordsch->TimeFiled[i].flag_sch = 1;
			u8 week = (psRecSch->nWeek+1)%DAYS_OF_WEEK;
			pRecordsch->TimeFiled[i].starttime = 
				sCfgIns.sSchTime[week][i].nStartTime;
			pRecordsch->TimeFiled[i].endtime = 
				sCfgIns.sSchTime[week][i].nStopTime;

			/*
			//printf("chn %d week %d seg %d start %x end %x \n", 
				psRecSch->nChn, psRecSch->nWeek, i, 
				sCfgIns.sSchTime[week][i].nStartTime,
				sCfgIns.sSchTime[week][i].nStopTime
			);
			*/
		}
	}
}


void DealRemoteCmd_Setrecschparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizSchedule sCfgIns;
	SBizParaTarget sParaTgtIns;
	
	ifly_RecordSCH_t* pRecordsch;
	
	pRecordsch = &pCmd->sReq.RecordSCH;

	sParaTgtIns.nChn 		  = pRecordsch->chn;	
	
	int i;
	for(i=0; i<SEGMENTS_OF_DAY; i++)
	{
		if(pRecordsch->TimeFiled[i].flag_sch)
		{
			BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_TIMER_RECSCHE);
		
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECTIMING;
		}
		else if(pRecordsch->TimeFiled[i].flag_md)
		{
			BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_MD_RECSCHE);
		
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECMOTION;
		}
		else if(pRecordsch->TimeFiled[i].flag_alarm)
		{
			BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_SENSORALARM_RECSCHE);
		
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECSENSOR;
		}
		
		int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			u8 week = (pRecordsch->weekday+1)%DAYS_OF_WEEK;
	
			sCfgIns.sSchTime[week][i].nStartTime = 
				pRecordsch->TimeFiled[i].starttime;
			sCfgIns.sSchTime[week][i].nStopTime = 
				pRecordsch->TimeFiled[i].endtime;

			BizSetPara(&sParaTgtIns, &sCfgIns);	
		}
	}
}


void DealRemoteCmd_Getmdparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget sParaTgtIns;
	
	ifly_MDParam_t* pMdparam, *pMdparamIn;

	pMdparamIn 	= &pCmd->sReq.MDParam;
	pMdparam	= &pRslt->sBasicInfo.MDParam;

	SBizAlarmVMotionPara sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	
	sParaTgtIns.nChn = pMdparamIn->chn;
	sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;	
	int rtn = BizGetPara(&sParaTgtIns, &sPIns);
	if(0==rtn)
	{
		pMdparam->flag_enable = sPIns.nEnable;
		pMdparam->chn 			= pMdparamIn->chn;							//通道
		pMdparam->delay 		= sPIns.nDelay>=0xff?0xffff:sPIns.nDelay;	//延时		
		pMdparam->sense 		= sPIns.nSensitivity - 1;					//灵敏度

		int i, j;
		
		u64 bit = 0x1;
		for(i=0; i<18; i++)
		{
			for(j=0; j<22; j++)
			{
				if( sPIns.nBlockStatus[i]&(bit<<j) )
				{
					pMdparam->block[i*22+j] = 1;
				}
			}
		}
		
		/*
		printf("get md %d %d %d %d %d %d\n", 
			pMdparam->block[0],
			pMdparam->block[1],
			pMdparam->block[2],
			pMdparam->block[3],
			sPIns.nDelay,
			pMdparam->delay
		);
		*/
		
		//pMdparam->block[44*36];				//区域
	}

	SBizAlarmDispatch sDsptPIns;
	sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
	rtn = BizGetPara(&sParaTgtIns, &sDsptPIns);
	if(0==rtn)
	{
		pMdparam->trigRecChn = 0;
		pMdparam->trigAlarmOut = 0;

		int         i;
		SBizDvrInfo sInfo;

		int ret = ConfigGetDvrInfo(0, &sInfo);
		if(ret == 0)
		{		
			for(i=0; i<sInfo.nRecNum; i++)
			{
				if( sDsptPIns.nRecordChn[i] != 0xff )
					pMdparam->trigRecChn |= (0x01<<i);
				else
					pMdparam->trigRecChn &= ~(0x01<<i);
			}
			//printf("pMdparamIn->trigRecChn %x \n", pMdparamIn->trigRecChn);
			
			for(i=0; i<sInfo.nAlarmoutNum; i++)
			{
				if( sDsptPIns.nAlarmOut[i] != 0xff )
					pMdparam->trigAlarmOut |= (0x01<<i);
				else
					pMdparam->trigAlarmOut &= ~(0x01<<i);
			}				
		}
		
		pMdparam->flag_buzz = sDsptPIns.nFlagBuzz;					//蜂鸣器
		pMdparam->flag_email = sDsptPIns.nFlagEmail;				//触发emaill
		//pMdparam->flag_mobile = sDsptPIns.nFlagBuzz;				//触发手机报警
		pMdparam->flag_showfullscreen = sDsptPIns.nZoomChn;
		
		int nLinkNum = sizeof(pMdparamIn->AlarmInPtz)/sizeof(pMdparamIn->AlarmInPtz[0]);
		for(i=0; i<nLinkNum; i++)
		{
			GetPtzDeal(&pMdparam->AlarmInPtz[i], &sDsptPIns.sAlarmPtz[i]);
		}
	}
}


void DealRemoteCmd_Setmdparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i, j;
	SBizParaTarget sParaTgtIns;
	
	ifly_MDParam_t* pMdparamIn;

	pMdparamIn 	= &pCmd->sReq.MDParam;

	SBizAlarmVMotionPara sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	
	sParaTgtIns.nChn = pMdparamIn->chn;
	sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;	
	int rtn = BizGetPara(&sParaTgtIns, &sPIns);
	if(0==rtn)
	{
		sPIns.nEnable = pMdparamIn->flag_enable;
		sPIns.nDelay = pMdparamIn->delay; //延时
		
		sPIns.nSensitivity = pMdparamIn->sense + 1;						//灵敏度
		
		/*
		printf("set md %d %d %d %d %d\n", 
			pMdparamIn->block[0],
			pMdparamIn->block[1],
			pMdparamIn->block[2],
			pMdparamIn->block[3],
			pMdparamIn->delay
		);
		*/
		
		int nRows, nCols;
		
		GetMdRowCol(&nRows, &nCols);
		int nBlockArr = sizeof(sPIns.nBlockStatus)/sizeof(sPIns.nBlockStatus[0]);
		if(nRows > nBlockArr)
		{
			nRows = nBlockArr;
		}
		
		////printf("row %d col %d \n", nRows, nCols);
		
		u64 bit = 0x1;
		for(i=0; i<nRows; i++)
		{
			for(j=0; j<nCols; j++)
			{
				if( pMdparamIn->block[i*nCols+j] )
				{
					sPIns.nBlockStatus[i] |= bit<<j;
				}
				else
				{
					sPIns.nBlockStatus[i] &= ~(bit<<j);
				}
			}
		}
		
		BizSetPara(&sParaTgtIns, &sPIns);
		BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_MD_AREA_SETUP);
	}
	
	SBizAlarmDispatch sDsptPIns;
	sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
	sParaTgtIns.nChn = pMdparamIn->chn;
	rtn = BizGetPara(&sParaTgtIns, &sDsptPIns);
	if(0==rtn)
	{
		SBizDvrInfo sInfo;

		int ret = ConfigGetDvrInfo(0, &sInfo);
		if(ret == 0)
		{	
			////printf("pMdparamIn->trigRecChn %x \n", pMdparamIn->trigRecChn);
			for(i=0; i<sInfo.nRecNum; i++)
			{
				if( pMdparamIn->trigRecChn&(0x01<<i) )
					sDsptPIns.nRecordChn[i] = i;
				else
					sDsptPIns.nRecordChn[i] = 0xff;					
			}
			
			for(i=0; i<sInfo.nAlarmoutNum; i++)
			{
				if( pMdparamIn->trigAlarmOut&(0x01<<i) )
					sDsptPIns.nAlarmOut[i] = i;
				else
					sDsptPIns.nAlarmOut[i] = 0xff;
			}	
		}
		
		sDsptPIns.nFlagBuzz = pMdparamIn->flag_buzz;					//蜂鸣器
		sDsptPIns.nFlagEmail = pMdparamIn->flag_email;					//触发emaill
		//pMdparam->flag_mobile = sDsptPIns.nFlagBuzz;					//触发手机报警
		sDsptPIns.nZoomChn = pMdparamIn->flag_showfullscreen;
				
		// ptz

		int nChMax = GetVideoMainNum();
		if(nChMax>sizeof(pMdparamIn->AlarmInPtz)/sizeof(pMdparamIn->AlarmInPtz[0])) 
			nChMax = sizeof(pMdparamIn->AlarmInPtz)/sizeof(pMdparamIn->AlarmInPtz[0]);
		
		for(i=0; i<nChMax; i++)
		{
			// 设置联动预置点和联动巡航线之前需要确认目标预置点和巡航线是否可用
			//
			SBizParaTarget 	sParaTgtInsc;
			SBizPtzPara 	sCfgInsc;
			
			memset(&sCfgInsc, 0, sizeof(sCfgInsc));
			sParaTgtInsc.emBizParaType 	= EM_BIZ_PTZPARA;
			sParaTgtInsc.nChn 			= i;
			rtn = BizGetPara(&sParaTgtInsc, &sCfgInsc);
			if(0 == rtn)
			{
				rtn = SetPtzDeal(&pMdparamIn->AlarmInPtz[i], &sDsptPIns.sAlarmPtz[i], &sCfgInsc.sAdvancedPara);
			}
		}

		if(0 == rtn)
			BizSetPara(&sParaTgtIns, &sDsptPIns);
		
		BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_MD_ALARMDEAL );
	}
}


void DealRemoteCmd_Getvideolostparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget sParaTgtIns;
	
	ifly_VideoLostParam_t* pVideolostparam, *pVideolostparamIn;

	pVideolostparam 	= &pRslt->sBasicInfo.VideoLostParam;
	pVideolostparamIn 	= &pCmd->sReq.VideoLostParam;

	SBizAlarmVLostPara sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	
	sParaTgtIns.nChn = pVideolostparamIn->chn;
	sParaTgtIns.emBizParaType = EM_BIZ_VLOSTPARA;	
	int rtn = BizGetPara(&sParaTgtIns, &sPIns);
	if(0==rtn)
	{
		pVideolostparam->chn = pVideolostparamIn->chn;				//通道
		pVideolostparam->delay = sPIns.nDelay;						//延时
		//pMdparam->block[44*36];				//区域
	}

	SBizAlarmDispatch sDsptPIns;
	sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
	rtn = BizGetPara(&sParaTgtIns, &sDsptPIns);
	if(0==rtn)
	{	
		pVideolostparam->trigRecChn = 0;
		pVideolostparam->trigAlarmOut = 0;

		int         i;
		SBizDvrInfo sInfo;

		int ret = ConfigGetDvrInfo(0, &sInfo);
		if(ret == 0)
		{		
			for(i=0; i<sInfo.nRecNum; i++)
			{
				if( sDsptPIns.nRecordChn[i] != 0xff )
					pVideolostparam->trigRecChn |= (0x01<<i);
				else
					pVideolostparam->trigRecChn &= ~(0x01<<i);
			}
			
			for(i=0; i<sInfo.nAlarmoutNum; i++)
			{
				if( sDsptPIns.nAlarmOut[i] != 0xff )
					pVideolostparam->trigAlarmOut |= (0x01<<i);
				else
					pVideolostparam->trigAlarmOut &= ~(0x01<<i);
			}				
		}
		
		pVideolostparam->flag_buzz = sDsptPIns.nFlagBuzz;					//蜂鸣器
		pVideolostparam->flag_email = sDsptPIns.nFlagEmail;					//触发emaill
		//pMdparam->flag_mobile = sDsptPIns.nFlagBuzz;				//触发手机报警
		pVideolostparam->flag_showfullscreen = sDsptPIns.nZoomChn;

		int nLinkNum = sizeof(pVideolostparam->AlarmInPtz)/sizeof(pVideolostparam->AlarmInPtz[0]);
		for(i=0; i<nLinkNum; i++)
		{
			GetPtzDeal(&pVideolostparam->AlarmInPtz[i], &sDsptPIns.sAlarmPtz[i]);
		}
	}
}

void DealRemoteCmd_Setvideolostparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget sParaTgtIns;
	
	ifly_VideoLostParam_t* pVideolostparam, *pVideolostparamIn;

	//pVideolostparam 	= &pRslt->sBasicInfo.VideoLostParam;
	pVideolostparamIn 	= &pCmd->sReq.VideoLostParam;

	SBizAlarmVLostPara sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	
	sParaTgtIns.nChn = pVideolostparamIn->chn;
	sParaTgtIns.emBizParaType = EM_BIZ_VLOSTPARA;	
	int rtn = BizGetPara(&sParaTgtIns, &sPIns);
	if(0==rtn)
	{
		sPIns.nDelay = pVideolostparamIn->delay;						//延时
		BizSetPara(&sParaTgtIns, &sPIns);
	}

	SBizAlarmDispatch sDsptPIns;
	sParaTgtIns.nChn = pVideolostparamIn->chn;
	sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
	rtn = BizGetPara(&sParaTgtIns, &sDsptPIns);
	if(0==rtn)
	{
		int i;
		SBizDvrInfo sInfo;

		int ret = ConfigGetDvrInfo(0, &sInfo);
		if(ret == 0)
		{		
			for(i=0; i<sInfo.nRecNum; i++)
			{
				if( pVideolostparamIn->trigRecChn&(0x01<<i) )
					sDsptPIns.nRecordChn[i] = i;
				else
					sDsptPIns.nRecordChn[i] = 0xff;					
			}
			
			for(i=0; i<sInfo.nAlarmoutNum; i++)
			{
				if( pVideolostparamIn->trigAlarmOut&(0x01<<i) )
					sDsptPIns.nAlarmOut[i] = i;
				else
					sDsptPIns.nAlarmOut[i] = 0xff;		
			}				
		}
		
		sDsptPIns.nFlagBuzz = pVideolostparamIn->flag_buzz;			//蜂鸣器
		sDsptPIns.nFlagEmail = pVideolostparamIn->flag_email;		//触发emaill
		//pMdparam->flag_mobile = sDsptPIns.nFlagBuzz;				//触发手机报警
		sDsptPIns.nZoomChn = pVideolostparamIn->flag_showfullscreen;

		int nChMax = GetVideoMainNum();
		int nPtzLinkMax = sizeof(pVideolostparamIn->AlarmInPtz)/sizeof(pVideolostparamIn->AlarmInPtz[0]);
		if(nChMax>nPtzLinkMax) 
			nChMax = nPtzLinkMax;
		
		for(i=0; i<nChMax; i++)
		{ 
			SBizParaTarget 	sParaTgtInsc;
			SBizPtzPara 	sCfgInsc;
			memset(&sCfgInsc, 0, sizeof(sCfgInsc));
			sParaTgtInsc.emBizParaType 	= EM_BIZ_PTZPARA;
			sParaTgtInsc.nChn 			= i;
			rtn = BizGetPara(&sParaTgtInsc, &sCfgInsc);
			if(0 == rtn)
			{
				rtn = SetPtzDeal(&pVideolostparamIn->AlarmInPtz[i], &sDsptPIns.sAlarmPtz[i], &sCfgInsc.sAdvancedPara);			
			}		
		}

		if(0 == rtn)
			BizSetPara(&sParaTgtIns, &sDsptPIns);
	}
	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_VIDEOLOSS_ALARMDEAL);
}


void DealRemoteCmd_Getvideoblockparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget sParaTgtIns;
	ifly_VideoBlockParam_t* pVideoblockparam, *pVideoblockparamIn;

	pVideoblockparam 	= &pRslt->sBasicInfo.VideoBlockParam;			
	pVideoblockparamIn 	= &pCmd->sReq.VideoBlockParam;

	
	SBizAlarmVBlindPara sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	
	sParaTgtIns.nChn = pVideoblockparamIn->chn;
	sParaTgtIns.emBizParaType = EM_BIZ_VBLINDPARA;	
	int rtn = BizGetPara(&sParaTgtIns, &sPIns);
	if(0==rtn)
	{
		pVideoblockparam->chn = pVideoblockparamIn->chn;				//通道
		pVideoblockparam->delay = sPIns.nDelay;						//延时
		//pMdparam->block[44*36];				//区域
	}

	SBizAlarmDispatch sDsptPIns;
	sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VBLIND;
	rtn = BizGetPara(&sParaTgtIns, &sDsptPIns);
	if(0==rtn)
	{	
		pVideoblockparam->trigRecChn = 0;
		pVideoblockparam->trigAlarmOut = 0;

		int         i;
		SBizDvrInfo sInfo;

		int ret = ConfigGetDvrInfo(0, &sInfo);
		if(ret == 0)
		{		
			for(i=0; i<sInfo.nRecNum; i++)
			{
				if( sDsptPIns.nRecordChn[i] != 0xff )
					pVideoblockparam->trigRecChn |= (0x01<<i);
				else
					pVideoblockparam->trigRecChn &= ~(0x01<<i);
			}
			
			for(i=0; i<sInfo.nAlarmoutNum; i++)
			{
				if( sDsptPIns.nAlarmOut[i] != 0xff )
					pVideoblockparam->trigAlarmOut |= (0x01<<i);
				else
					pVideoblockparam->trigAlarmOut &= ~(0x01<<i);
			}				
		}
		
		pVideoblockparam->flag_buzz = sDsptPIns.nFlagBuzz;					//蜂鸣器
		pVideoblockparam->flag_email = sDsptPIns.nFlagEmail;					//触发emaill
		//pMdparam->flag_mobile = sDsptPIns.nFlagBuzz;				//触发手机报警
	}	
}


void DealRemoteCmd_Setvideoblockparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget sParaTgtIns;
	
	ifly_VideoBlockParam_t* pVideoBlockparamIn;

	//pVideolostparam 	= &pRslt->sBasicInfo.VideoLostParam;
	pVideoBlockparamIn 	= &pCmd->sReq.VideoBlockParam;

	SBizAlarmVLostPara sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	
	sParaTgtIns.nChn = pVideoBlockparamIn->chn;
	sParaTgtIns.emBizParaType = EM_BIZ_VBLINDPARA;	
	int rtn = BizGetPara(&sParaTgtIns, &sPIns);
	if(0==rtn)
	{
		sPIns.nDelay = pVideoBlockparamIn->delay;						//延时
		BizSetPara(&sParaTgtIns, &sPIns);
	}

	SBizAlarmDispatch sDsptPIns;
	sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VBLIND;
	rtn = BizGetPara(&sParaTgtIns, &sDsptPIns);
	if(0==rtn)
	{
		int i;
		SBizDvrInfo sInfo;

		int ret = ConfigGetDvrInfo(0, &sInfo);
		if(ret == 0)
		{		
			for(i=0; i<sInfo.nRecNum; i++)
			{
				if( pVideoBlockparamIn->trigRecChn&(0x01<<i) )
					sDsptPIns.nRecordChn[i] = i;
				else
					sDsptPIns.nRecordChn[i] = 0xff;					
			}
			
			for(i=0; i<sInfo.nAlarmoutNum; i++)
			{
				if( pVideoBlockparamIn->trigAlarmOut&(0x01<<i) )
					sDsptPIns.nAlarmOut[i] = i;
				else
					sDsptPIns.nAlarmOut[i] = 0xff;		
			}				
		}
		
		sDsptPIns.nFlagBuzz = pVideoBlockparamIn->flag_buzz;			//蜂鸣器
		sDsptPIns.nFlagEmail = pVideoBlockparamIn->flag_email;		//触发emaill
		//pMdparam->flag_mobile = sDsptPIns.nFlagBuzz;				//触发手机报警

		BizSetPara(&sParaTgtIns, &sDsptPIns);
	}
}

static inline u32 item2baudrate(s32 item)
{
	switch(item)
	{
	case 0:
		return 115200;
	case 1:
		return 57600;
	case 2:
		return 38400;
	case 3:
		return 19200;
	case 4:
		return 9600;
	case 5:
		return 4800;
	case 6:
		return 2400;
	case 7:
		return 1200;
	case 8:
		return 300;	
	}
	return 115200;
}

void DealRemoteCmd_Getptzparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget sParaTgtIns;
	ifly_PTZParam_t* pYuninfo;
	
	pYuninfo = &pRslt->sBasicInfo.PTZParam;
	
	SBizPtzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
	sParaTgtIns.nChn = pCmd->sReq.nChn;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		pYuninfo->chn 			= pCmd->sReq.nChn;
		pYuninfo->address		= sCfgIns.nCamAddr;
		pYuninfo->enableptz		= sCfgIns.nEnable;
		
		u8 nIdx = 0;
		if(0 == BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_BAUDRATE, sCfgIns.nBaudRate, &nIdx))
		{
			pYuninfo->baud_ratesel = nIdx;
		}
		
		//sCfgIns.nDataBit = pYuninfo->data_bitsel;
		//sCfgIns.nStopBit = pYuninfo->stop_bitsel;
		//sCfgIns.nCheckType	= pYuninfo->crccheck;
		//sCfgIns.nFlowCtrlType = pYuninfo->flow_control;
		pYuninfo->protocol = sCfgIns.nProtocol;
		
		//pYuninfo->data_bitsel	= 8-sCfgIns.nDataBit;
		//pYuninfo->stop_bitsel	= sCfgIns.nStopBit;
		//pYuninfo->crccheck		= sCfgIns.nCheckType;
		//pYuninfo->flow_control	= sCfgIns.nFlowCtrlType;
		
		//csp modify
		pYuninfo->enableptz = sCfgIns.nEnable;
		
		printf("get ptz nCamAddr %d nBaudRate %d baud_ratesel %d nProtocol %d\n", sCfgIns.nCamAddr, sCfgIns.nBaudRate, pYuninfo->baud_ratesel, sCfgIns.nProtocol);
	}
}

void DealRemoteCmd_Setptzparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget sParaTgtIns;
	ifly_PTZParam_t* pYuninfo;
	
	pYuninfo = &pCmd->sReq.PTZParam;
	
	SBizPtzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
	sParaTgtIns.nChn = pYuninfo->chn;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		//printf("ptz ch %d enable %d address %d \n", pYuninfo->chn, pYuninfo->enableptz, pYuninfo->address);
		
		sCfgIns.nEnable		= pYuninfo->enableptz;
		sCfgIns.nCamAddr	= pYuninfo->address;
		
		s32 nValue = 0;
		if(0 == BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_BAUDRATE, pYuninfo->baud_ratesel, &nValue))
		{
			sCfgIns.nBaudRate = nValue;
		}
		
		//sCfgIns.nDataBit = pYuninfo->data_bitsel;
		//sCfgIns.nStopBit = pYuninfo->stop_bitsel;
		//sCfgIns.nCheckType	= pYuninfo->crccheck;
		//sCfgIns.nFlowCtrlType= pYuninfo->flow_control;
		
		int i;
		for(i=0; i<sizeof(sPtzProList)/sizeof(sPtzProList[0]); i++)
		{
			if(sPtzProList[i].pro_id == pYuninfo->protocol)
			{
				sCfgIns.nProtocol = i;
				break;
			}
		}
		
		//csp modify
		sCfgIns.nEnable = pYuninfo->enableptz;
		
		printf("set ptz %d %d %d %d\n", pYuninfo->address, pYuninfo->baud_ratesel, pYuninfo->protocol, pYuninfo->enableptz);
		
		BizSetPara(&sParaTgtIns, &sCfgIns);
	}
}

void DealRemoteCmd_Setpreset(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_PtzPresetCtr_t* pPtzpresetctrl;
	
	pPtzpresetctrl = &pCmd->sReq.PtzPresetCtr;
	
	SBizParaTarget 	sParaTgtIns;	
	SBizPtzPara 	sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	sParaTgtIns.emBizParaType 	= EM_BIZ_PTZPARA;
	sParaTgtIns.nChn 			= pPtzpresetctrl->chn;
	u8	nPreset					= pPtzpresetctrl->presetpoint;
	
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if((nPreset<1)||(nPreset>PRESET_MAX))
	{
		pRslt->nErrCode = CTRL_FAILED_PARAM;
		return;
	}
	
	if (pPtzpresetctrl->option == 0)
	{
		//printf("add presetpoint %d !\n", nPreset);
		//if( !sCfgIns.sAdvancedPara.nIsPresetSet[nPreset-1] )
		{
			sCfgIns.sAdvancedPara.nIsPresetSet[nPreset-1] = 1;
			BizPtzCtrl(sParaTgtIns.nChn, EM_BIZPTZ_CMD_PRESET_SET, nPreset);
			BizSetPara(&sParaTgtIns, &sCfgIns);
		}
	}
	else if (pPtzpresetctrl->option == 1)
	{
		//printf("del presetpoint %d !\n", nPreset);
		if( sCfgIns.sAdvancedPara.nIsPresetSet[nPreset-1] )
		{
			sCfgIns.sAdvancedPara.nIsPresetSet[nPreset-1] = 0;
			BizSetPara(&sParaTgtIns, &sCfgIns);
		}
	}
	else if (pPtzpresetctrl->option == 2)
	{
		//printf("goto presetpoint %d !\n", nPreset);
		
		if(sCfgIns.sAdvancedPara.nIsPresetSet[nPreset-1])
			BizPtzCtrl(sParaTgtIns.nChn, EM_BIZPTZ_CMD_PRESET_GOTO, nPreset);
	}
}


void DealRemoteCmd_Getcruiseparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_PtzCruisePathParam_t* pGetptzcruisepathparam, *pGetptzcruisepathparamIn;

	pGetptzcruisepathparam = &pRslt->sBasicInfo.PtzCruisePathParam;
	pGetptzcruisepathparamIn = &pCmd->sReq.PtzCruisePathParam;
	
	SBizParaTarget 	sParaTgtIns;	
	SBizPtzPara 	sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	sParaTgtIns.emBizParaType 	= EM_BIZ_PTZPARA;
	sParaTgtIns.nChn 			= pGetptzcruisepathparamIn->chn;	
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if( rtn==0 )
	{
		pGetptzcruisepathparam->chn 		= sParaTgtIns.nChn;
		pGetptzcruisepathparam->cruise_path = pGetptzcruisepathparamIn->cruise_path;

		int i, j;
		int nTourPointIdx = 0;
		for( i=0; i<CRUISE_MAX; i++ )
		{
			if( sCfgIns.sAdvancedPara.sTourPath[i].nPathNo == pGetptzcruisepathparamIn->cruise_path )
			{
				SBizTourPath* pPath = &sCfgIns.sAdvancedPara.sTourPath[i];
				
				for( j=0; j<MAX_CRUISE_POS_NUM; j++ )
				{
					pGetptzcruisepathparam->Cruise_point[j].preset = 
						pPath->sTourPoint[j].nPresetPos;
					pGetptzcruisepathparam->Cruise_point[j].weeltime = 
						pPath->sTourPoint[j].nDwellTime;
					pGetptzcruisepathparam->Cruise_point[j].rate = 
						pPath->sTourPoint[j].nSpeed;
				}
				
				break;
			}
		}
	}
}

int ExcCmd_SetCruise(SBizTourPath* pPath, ifly_PtzCruisePathParam_t* pOldPara, SBizPtzAdvancedPara* pAdvance)
{
	if(!pPath || !pOldPara || !pAdvance) return -1;
	
	int j, err = 0;
	pPath->nPathNo = pOldPara->cruise_path;
	
	for( j=0; j<CRUISE_POINT_NUM; j++ ) 
	{		
		/*
		printf("flag %d preset %d dwell %d speed %d real %d\n",
			pOldPara->Cruise_point[j].flag_add,
			pOldPara->Cruise_point[j].preset,
			pOldPara->Cruise_point[j].weeltime,
			pOldPara->Cruise_point[j].rate,
			pAdvance->nIsPresetSet[j]
		);
		*/
		
		if(pOldPara->Cruise_point[j].flag_add == 0)
			continue;
			
		if(pOldPara->Cruise_point[j].flag_add == 1) // 1 add
		{
			int nPresetNo = pOldPara->Cruise_point[j].preset;

			if(nPresetNo <= 0)
			{
				return -1;
			}
			
			if(0 == pAdvance->nIsPresetSet[nPresetNo-1]) 
			{
				return -1; // 无预置点则不设置
			}
			
			pPath->sTourPoint[j].nPresetPos = 
				pOldPara->Cruise_point[j].preset;
			pPath->sTourPoint[j].nDwellTime = 
				pOldPara->Cruise_point[j].weeltime;
			pPath->sTourPoint[j].nSpeed = 
				pOldPara->Cruise_point[j].rate;

			/*
			//printf("add preset %d dwell %d speed %d \n",
				pOldPara->Cruise_point[j].preset,
				pOldPara->Cruise_point[j].weeltime,
				pOldPara->Cruise_point[j].rate
			);
			*/
		}
		else if(2 == pOldPara->Cruise_point[j].flag_add) // 0 del
		{
			if(pPath->sTourPoint[j].nPresetPos == 0)
			{
				return -1;
			}
			
			pPath->sTourPoint[j].nPresetPos = 0;
			pPath->sTourPoint[j].nDwellTime	= 0;
			pPath->sTourPoint[j].nSpeed		= 0;
		}
	}
	
	return 0;
}

void DealRemoteCmd_Setcruiseparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_PtzCruisePathParam_t *pGetptzcruisepathparamIn;

	pGetptzcruisepathparamIn = &pCmd->sReq.PtzCruisePathParam;
	
	SBizParaTarget 	sParaTgtIns;	
	SBizPtzPara 	sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	memset(pRslt, 0, sizeof(SRemoteCmdRslt));
	
	sParaTgtIns.emBizParaType 	= EM_BIZ_PTZPARA;
	sParaTgtIns.nChn 			= pGetptzcruisepathparamIn->chn;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if( rtn == 0 )
	{
		int i, j, k;
		int nTourPointIdx = 0;
		int nFirstSlot = -1;	
		
		// 设置巡航线之前需要确认目标巡航点是否可用
		//
		SBizParaTarget 	sParaTgtInsc;
		SBizPtzPara 	sCfgInsc;
		
		memset(&sCfgInsc, 0, sizeof(sCfgInsc));
		sParaTgtInsc.emBizParaType 	= EM_BIZ_PTZPARA;
		sParaTgtInsc.nChn 			= sParaTgtIns.nChn;
		rtn = BizGetPara(&sParaTgtInsc, &sCfgInsc);	
		if(rtn == 0)
		{
			int err = 0;
			
			for( i=0; i<CRUISE_MAX; i++ )
			{
				if(sCfgIns.sAdvancedPara.sTourPath[i].nPathNo == 0 && nFirstSlot < 0)
				{
					nFirstSlot = i;
				}
				else
				{
					if( sCfgIns.sAdvancedPara.sTourPath[i].nPathNo == pGetptzcruisepathparamIn->cruise_path )
					{
						err = ExcCmd_SetCruise(&sCfgIns.sAdvancedPara.sTourPath[i], pGetptzcruisepathparamIn, &sCfgInsc.sAdvancedPara);
						
						break;
					}
				}
			}
			
			if(i == CRUISE_MAX)
				err = ExcCmd_SetCruise(&sCfgIns.sAdvancedPara.sTourPath[nFirstSlot], pGetptzcruisepathparamIn, &sCfgInsc.sAdvancedPara);
			
			if(!err)
				BizSetPara(&sParaTgtIns, &sCfgIns);
			else
				pRslt->nErrCode = -1;
		}
	}
}


void DealRemoteCmd_Ctrlcruisepath(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_CRUISELINE);
#if 1
	ifly_CruisePathCtr_t* pCruisepathctrl;

	pCruisepathctrl = &pCmd->sReq.CruisePathCtr;
	
	SBizParaTarget 	sParaTgtIns;	
	SBizPtzPara 	sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	sParaTgtIns.emBizParaType 	= EM_BIZ_PTZPARA;
	sParaTgtIns.nChn 			= pCruisepathctrl->chn;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if( rtn==0 )
	{
		int nTourPointIdx = 0;
		int i, j;
			
		//printf("cruise %d flag %d chn %d \n", 
		//	pCruisepathctrl->cruisepath, pCruisepathctrl->flagoption, pCruisepathctrl->chn);
			
		for( i=0; i<CRUISE_MAX; i++ )
		{
			if( sCfgIns.sAdvancedPara.sTourPath[pCruisepathctrl->cruisepath-1].nPathNo > 0)
			{
				
				////printf("cruise start %d \n", pCruisepathctrl->cruisepath );
				
				if(pCruisepathctrl->flagoption) // 1 start
				{					
    				BizPtzCtrl((u8)pCruisepathctrl->chn, 
    					EM_BIZPTZ_CMD_START_TOUR, 
    					pCruisepathctrl->cruisepath-1
    				);
				}
				else // 0 stop
				{
    				BizPtzCtrl((u8)pCruisepathctrl->chn, 
    					EM_BIZPTZ_CMD_STOP_TOUR, 
    					pCruisepathctrl->cruisepath-1
    				);					
				}
				
				break;
			}
		}
	}
#endif
}

void DealRemoteCmd_Ctrlptztrack(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_TrackCtr_t* psIns = &pCmd->sReq.TrackCtr;

	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONTROL  , BIZ_LOG_SLAVE_CONTROL_PTZ );

	switch( psIns->flagoption )
	{
		case 0:
		{
			BizPtzCtrl((u8)psIns->chn, EM_BIZPTZ_CMD_START_PATTERN, 0);
		} break;
		case 1:
		{
			BizPtzCtrl((u8)psIns->chn, EM_BIZPTZ_CMD_STOP_PATTERN, 0);
		} break;
		case 2:
		{
			BizPtzCtrl((u8)psIns->chn, EM_BIZPTZ_CMD_STARTREC_PATTERN, 0);			
		} break;
		case 3:
		{
			BizPtzCtrl((u8)psIns->chn, EM_BIZPTZ_CMD_STOPREC_PATTERN, 0);	
		} break;
	}
}

void DealRemoteCmd_Gethddinfo(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	ifly_hddInfo_t* pHddinfo;
	
	int k = pCmd->sReq.nHddIdx;
	
	pHddinfo = &pRslt->sBasicInfo.hddInfo;
	memset(pHddinfo, 0, sizeof(ifly_hddInfo_t));
	
	SStoreDevManage sStDevMgr;
	memset( &sStDevMgr, 0, sizeof(sStDevMgr) );
	
	SDevInfo sDevInfo[64]; //zlb20111117 去掉部分malloc
	sStDevMgr.psDevList = sDevInfo;
	
	memset(sStDevMgr.psDevList, 0, sizeof(SDevInfo)*64);
	
	int rtn = ModSysComplexDMGetInfo(&sStDevMgr, 8);
	if(0==rtn)
	{
		////printf("dev num: %d req hddidx %d\n",sStDevMgr.nDiskNum, k);
		if(sStDevMgr.nDiskNum>0)
		{
			//now just return the first hddinfo
			
			for( i=0; i<sStDevMgr.nDiskNum; i++ )
			{
				/*
				//printf("dev %d nStatus: 	%d\n",i, sStDevMgr.psDevList[i].nStatus);
				//printf("dev %d nType: 		%d\n",i, sStDevMgr.psDevList[i].nType);
				//printf("dev %d nTotal: 		%d\n",i, sStDevMgr.psDevList[i].nTotal);
				//printf("dev %d nFree: 		%d\n",i, sStDevMgr.psDevList[i].nFree);
				//printf("dev %d strDevPath: 	%s\n",i, sStDevMgr.psDevList[i].strDevPath);
				//printf("dev %d nPartitionNo: %d\n",i, sStDevMgr.psDevList[i].nPartitionNo);
				*/
				
				//if(sStDevMgr.psDevList[i].nStatus==0)
				//printf("loop %d logic_idx: %d\n", i, sStDevMgr.psDevList[i].disk_logic_idx);
				{
					if(k == sStDevMgr.psDevList[i].disk_logic_idx-1)
					{
						pHddinfo->capability 	= (sStDevMgr.psDevList[i].nTotal/1000);
						pHddinfo->freesize 	 	= (sStDevMgr.psDevList[i].nFree/1000);
						pHddinfo->hdd_exist 	= 1;
						pHddinfo->hdd_index 	= k;
						pHddinfo->reserved[0] 	= sStDevMgr.psDevList[i].nType;
						////printf("found disk idx %d\n", k);
						break;
					}
				}
			}
		}
	}
	//else
	//	printf("failed to get dev info!\n");
	
	//if(sStDevMgr.psDevList) free(sStDevMgr.psDevList);  //zlb20111117 去掉部分malloc
}

void DealRemoteCmd_Getuserinfo(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	ifly_userNumber_t* pUsernumInfo;
	
	pUsernumInfo = &pRslt->sBasicInfo.userNumber;
	
	s8 szUserInfo[8][USER_GROUP_NAME_MAX_LENGTH];
	s8* paUserInfo[8];
	for(i=0; i<8; i++)
	{
		paUserInfo[i] = szUserInfo[i];
	}
	
	SBizUserListPara sUserListIns;
	
	sUserListIns.list = paUserInfo;
	sUserListIns.nLength = USER_GROUP_NAME_MAX_LENGTH;
	sUserListIns.nRealLength = 8;
	
	if( 0 == UserGetListUserRegisted(&sUserListIns) )
	{
		SBizUserUserPara sUserPara;
		int nMaxUser = sizeof(pUsernumInfo->userNum)/sizeof(pUsernumInfo->userNum[0]);
		int nRealUserNum = sUserListIns.nRealLength>nMaxUser? nMaxUser : sUserListIns.nRealLength;
		for( i=0; i<nRealUserNum; i++ )
		{
			strcpy( sUserPara.UserName, szUserInfo[i] );
			if( 0 == UserGetUserInfo(&sUserPara) )
			{
				strcpy( pUsernumInfo->userNum[i].name, sUserPara.UserName );
				strcpy( pUsernumInfo->userNum[i].passwd, sUserPara.Password );
				
				pUsernumInfo->userNum[i].rcamer = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PTZCTRL];
				pUsernumInfo->userNum[i].rrec = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_RECORD];
				pUsernumInfo->userNum[i].rplay = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PLAYBACK];
				pUsernumInfo->userNum[i].rsetpara = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_SETUP];
				pUsernumInfo->userNum[i].rlog = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_LOG_VIEW];
				pUsernumInfo->userNum[i].rtool = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DISK_MANAGE];
				pUsernumInfo->userNum[i].rpreview = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_REMOTE_LOGIN];
				pUsernumInfo->userNum[i].ralarm = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DATA_MANAGE];
				pUsernumInfo->userNum[i].rvoip = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_AUDIO_TALK];
				pUsernumInfo->userNum[i].lcamer = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_RESET];
				pUsernumInfo->userNum[i].lrec = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_UPDATE];
				pUsernumInfo->userNum[i].lplay = sUserPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SHUTDOWN];
				
				#if 0
				//printf("***UserName: %s\n", pUsernumInfo->userNum[i].name);
				//printf("***Permition:\n");
				//printf("*ptz: %d, rec: %d, pback: %d, sysset: %d, view: %d, disk: %d\n",
					pUsernumInfo->userNum[i].rcamer, pUsernumInfo->userNum[i].rrec,
					pUsernumInfo->userNum[i].rplay, pUsernumInfo->userNum[i].rsetpara,
					pUsernumInfo->userNum[i].rlog, pUsernumInfo->userNum[i].rtool);
				//printf("*login: %d, dataM: %d, audioT: %d, reset: %d, update: %d, shutdown: %d\n\n\n",
					pUsernumInfo->userNum[i].rpreview, pUsernumInfo->userNum[i].ralarm,
					pUsernumInfo->userNum[i].rvoip, pUsernumInfo->userNum[i].lcamer,
					pUsernumInfo->userNum[i].lrec, pUsernumInfo->userNum[i].lplay);
				#endif
				
				if(0 == strcasecmp("Administrator", sUserPara.GroupBelong))
				{
					pUsernumInfo->userNum[i].lsetpara = 1;
				}
				else
					pUsernumInfo->userNum[i].lsetpara = 0;
				
				pUsernumInfo->userNum[i].llog = sUserPara.emIsBindPcMac;
				
				sprintf(pUsernumInfo->userNum[i].macaddr, 
					"%02llX:%02llX:%02llX:%02llX:%02llX:%02llX", 
					(sUserPara.PcMacAddress>>40)&0xFF,
					(sUserPara.PcMacAddress>>32)&0xFF,
					(sUserPara.PcMacAddress>>24)&0xFF,
					(sUserPara.PcMacAddress>>16)&0xFF,
					(sUserPara.PcMacAddress>>8)&0xFF,
					sUserPara.PcMacAddress&0xFF
				);
				
				/*
				//printf("get user info %s %s bindmac %d MAC %s org mac %llx\n", 
					pUsernumInfo->userNum[i].name,
					pUsernumInfo->userNum[i].passwd,
					pUsernumInfo->userNum[i].llog,
					pUsernumInfo->userNum[i].macaddr,
					sUserPara.PcMacAddress
				);
				*/
				
				#if 0
				if(sUserPara.UserAuthor.nPtzCtrl)
				{
					pUsernumInfo->rcamer = 1;
				}
				if(sUserPara.UserAuthor.)
				{
					pUsernumInfo->rcamer = 1;
				}
				if(sUserPara.UserAuthor.nPtzCtrl)
				{
					pUsernumInfo->rcamer = 1;
				}
				if(sUserPara.UserAuthor.nPtzCtrl)
				{
					pUsernumInfo->rcamer = 1;
				}
				#endif
			}
		}
	}
}

void DealRemoteCmd_Setuserinfo(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int j = 0, i;
	ifly_userMgr_t *pUsermanger = &pCmd->sReq.userMgr;
	
	char tmp[20] = {0};
	char mac2[20] = {0};
	strcpy(mac2, pUsermanger->userInfo.macaddr);
	for(i = 0; i<strlen(mac2); i++)
	{
		if(((mac2[i] >= '0') && (mac2[i] <= '9')) 
			|| ((mac2[i] >= 'a') && (mac2[i] <= 'f'))
			|| ((mac2[i] >= 'A') && (mac2[i] <= 'F')))
		{
			tmp[i-j] = mac2[i];
		}
		else
		{
			j++;
		}
	}
	////printf("mac2:%s, tmp:%s\n", mac2, tmp);
	
	if(pUsermanger->userInfo.llog && 12 != strlen(tmp))
	{
		//printf("err mac address ! \n");
		pRslt->nErrCode = -1;
		return;
	}

	u64 nTmpMacAddr;
	sscanf(tmp, "%012llx", &nTmpMacAddr);

	/*
	printf("set user info %s %s flg %d bindmac %d MAC %s tgt mac %llx\n", 
		pUsermanger->userInfo.name,
		pUsermanger->userInfo.passwd,
		pUsermanger->flagOption,
		pUsermanger->userInfo.llog,
		pUsermanger->userInfo.macaddr,
		nTmpMacAddr
	);
	*/
	
	if(pUsermanger->flagOption==0)
	{		
		SBizUserUserPara para;
		memset(&para, 0, sizeof(para));

		strcpy( para.UserName, pUsermanger->userInfo.name );
		strcpy( para.Password, pUsermanger->userInfo.passwd );

		if(pUsermanger->userInfo.lsetpara)
		{
			strcpy((char*)para.GroupBelong, "Administrator");
		}
		else
		{
			strcpy((char*)para.GroupBelong, "Guest");
		}
		
		para.emIsBindPcMac 	= pUsermanger->userInfo.llog;        // mac绑定
		para.PcMacAddress	= nTmpMacAddr;

		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_USER_MANAGE] = 0;
		if(0 == strcasecmp("Administrator", (char*)para.GroupBelong))
		{
			para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_USER_MANAGE] = 1;
		}	

		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PTZCTRL] = pUsermanger->userInfo.rcamer;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_RECORD] = pUsermanger->userInfo.rrec;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PLAYBACK] = pUsermanger->userInfo.rplay;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_SETUP] = pUsermanger->userInfo.rsetpara;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_LOG_VIEW] = pUsermanger->userInfo.rlog;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DISK_MANAGE] = pUsermanger->userInfo.rtool;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_REMOTE_LOGIN] = pUsermanger->userInfo.rpreview;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DATA_MANAGE] = pUsermanger->userInfo.ralarm;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_AUDIO_TALK] = pUsermanger->userInfo.rvoip;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_RESET] = pUsermanger->userInfo.lcamer;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_UPDATE] = pUsermanger->userInfo.lrec;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SHUTDOWN] = pUsermanger->userInfo.lplay;
		
		BizUserAddUser(&para);
				
		BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_ADD_USER);		
	}
	else if(pUsermanger->flagOption==1)
	{
		SBizUserUserPara para;
		memset(&para, 0, sizeof(para));

		strcpy( para.UserName, pUsermanger->userInfo.name );
		
		SBizParaTarget pTarget;
		pTarget.emBizParaType = EM_BIZ_USER_GETUSERINFO;
		if(0 != BizGetPara(&pTarget, &para))
		{
			//printf("Get user info failed\n");
			return;
		}
		if(pUsermanger->userInfo.lsetpara)
		{
			strcpy((char*)para.GroupBelong, "Administrator");
		}
		else
		{
			strcpy((char*)para.GroupBelong, "Guest");
		}
		
		para.emIsBindPcMac 	= pUsermanger->userInfo.llog;        // mac绑定
		para.PcMacAddress	= nTmpMacAddr;

		pTarget.emBizParaType = EM_BIZ_USER_MODIFYUSER;

		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PTZCTRL] = pUsermanger->userInfo.rcamer;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_RECORD] = pUsermanger->userInfo.rrec;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PLAYBACK] = pUsermanger->userInfo.rplay;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_SETUP] = pUsermanger->userInfo.rsetpara;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_LOG_VIEW] = pUsermanger->userInfo.rlog;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DISK_MANAGE] = pUsermanger->userInfo.rtool;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_REMOTE_LOGIN] = pUsermanger->userInfo.rpreview;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DATA_MANAGE] = pUsermanger->userInfo.ralarm;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_AUDIO_TALK] = pUsermanger->userInfo.rvoip;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_RESET] = pUsermanger->userInfo.lcamer;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_UPDATE] = pUsermanger->userInfo.lrec;
		para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SHUTDOWN] = pUsermanger->userInfo.lplay;
		
		strcpy( para.Password, pUsermanger->userInfo.passwd );
		if(0 != BizSetPara(&pTarget, &para))
		{
			//printf("modify user failed\n");
			return;
		}
		//BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_USER_AUTH );
		//BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_PWD );
	}
	else if(pUsermanger->flagOption==2)
	{
		BizUserDeleteUser(pUsermanger->userInfo.name);
		BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_DELETE_USER);
	}
	
	// todo
}

void DealRemoteCmd_Setrestore(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	BizConfigDefault();//恢复默认设置
	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_FACTORY_RESET );
	
	BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
}


void DealRemoteCmd_Clearalarm(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONTROL  , BIZ_LOG_SLAVE_CLEAR_ALARM );

	// todo
}


void DealRemoteCmd_Getsystime(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Setsystime(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	/*
	int i;
	ifly_sysTime_t *pGetsystemtime;
	SDateTime	sDTIns;
	struct tm *tm_time;
	
	pGetsystemtime = &pCmd->sReq.sysTime;
	
	time_t now_time;
	now_time = time(0);
	tm_time = localtime(&now_time);
	
	sDTIns.nYear = tm_time->tm_year + 1900;
	sDTIns.nMonth = tm_time->tm_mon + 1;
	sDTIns.nDay = tm_time->tm_mday;
	sDTIns.nHour = tm_time->tm_hour;
	sDTIns.nMinute = tm_time->tm_min;
	sDTIns.nSecode = tm_time->tm_sec;
	
	//printf("set %04d-%02d-%02d\n", tm_time->tm_year, tm_time->tm_mon, tm_time->tm_mday);
	
	BizRecordStopAll();
	usleep(2000*1000); //多睡会儿吧，否则结束时间戳容易出问题
	
	sDTIns.emDateTimeFormat = EM_DATETIME_IGNORE; 
	printf("dateformat %d \n", sDTIns.emDateTimeFormat);
	printf("dateformat %d \n", sDTIns.emDateTimeFormat);
	printf("dateformat %d \n", sDTIns.emDateTimeFormat);
	printf("dateformat %d \n", sDTIns.emDateTimeFormat);
	ModSysComplexDTSet(&sDTIns);
	
	usleep(20*1000);
	for(i = 0; i < GetVideoMainNum(); i++)
		BizRecordResume(i);
	
	BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONTROL  , BIZ_LOG_SLAVE_DST_CONFIG );
	*/
}

void DealRemoteCmd_Getsysinfo(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{    
	ifly_sysinfo_t *pSysteminfo = &pRslt->sBasicInfo.sysinfo;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret==0)
	{
		strcpy(pSysteminfo->devicename, bizSysPara.strDevName);
	}
	
	SBizDvrInfo sDvrInfo;
	if( 0 == ConfigGetDvrInfo(0, &sDvrInfo) )
	{
		strcpy(pSysteminfo->deviceser, "");
		strcpy(pSysteminfo->devicemodel, sDvrInfo.strModel);
		strcpy(pSysteminfo->version, sDvrInfo.strVersion);
	}	
}

void DealRemoteCmd_Shutdown(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	BizNetWriteLog(pCmd->cph,  BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_POWER_OFF);
	BizSysComplexExit(EM_BIZSYSEXIT_POWEROFF);
}

void DealRemoteCmd_Reboot(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	BizNetWriteLog(pCmd->cph,  BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_REBOOT);
	BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
}

void DealRemoteCmd_Ptzctrl(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_PtzCtrl_t* pPtzCtrl;

	pPtzCtrl = &pCmd->sReq.PtzCtrl;

	u8 curChn = pPtzCtrl->chn;
	static int nLastDirection = 0;

	//printf("ptzctrl channel %d, cmd: %d\n", curChn, pPtzCtrl->cmd);
	switch (pPtzCtrl->cmd)
	{
		case 0:
		{
			//printf("stop\n");

			BizPtzCtrl((u8)curChn, 30+nLastDirection, 0);	
		} break;
		case 1:
		{
			//printf("up\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_TILEUP, 0);
			nLastDirection = EM_BIZPTZ_CMD_START_TILEUP; 		
		} break;
		case 2:
		{
			//printf("down\n");

			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_TILEDOWN, 0); 
			nLastDirection = EM_BIZPTZ_CMD_START_TILEDOWN; 		
		} break;
		case 3:
		{
			//printf("left\n");

			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_PANLEFT, 0);
			nLastDirection = EM_BIZPTZ_CMD_START_PANLEFT; 		
		} break;
		case 4:
		{
			//printf("right\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_PANRIGHT, 0);
			nLastDirection = EM_BIZPTZ_CMD_START_PANRIGHT; 		
		} break;
		case 5:
		{
			//printf("start scan\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_AUTOSCAN_ON, 0);

			nLastDirection = EM_BIZPTZ_CMD_AUTOSCAN_ON;
		}break;
		case 6:
		{
			//printf("zoom ++\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_ZOOMTELE, 0); 
			nLastDirection = EM_BIZPTZ_CMD_START_ZOOMWIDE; 		
		}break;
		case 7:
		{
			//printf("zoom --\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_ZOOMWIDE, 0); 
			nLastDirection = EM_BIZPTZ_CMD_START_ZOOMTELE; 		
		}break;
		case 8:
		{
			//printf("focus ++\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_FOCUSFAR, 0); 
			nLastDirection = EM_BIZPTZ_CMD_START_FOCUSFAR;
			
		}break;
		case 9:
		{
			//printf("focus --\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_FOCUSNEAR, 0);
			nLastDirection = EM_BIZPTZ_CMD_START_FOCUSNEAR; 
		}break;
		case 10:
		{
			//printf("aperture ++\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_IRISLARGE, 0);
			nLastDirection = EM_BIZPTZ_CMD_START_IRISLARGE;
			
		}break;
		case 11:
		{
			//printf("aperture --\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_IRISSMALL, 0); 
			nLastDirection = EM_BIZPTZ_CMD_START_IRISSMALL;
		}break;
		case 14:
		{
			break;
			//printf("wiper open !\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_WIPEROPEN, 0); 
			nLastDirection = EM_BIZPTZ_CMD_START_WIPEROPEN;
		}break;
		case 15:
		{
			break;
			//printf("wiper close !\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_START_WIPERCLOSE, 0); 
			nLastDirection = EM_BIZPTZ_CMD_START_WIPERCLOSE;
		}break;
	}
	
	switch (pPtzCtrl->cmd)
	{
		case 12:
		{
			break;
			//printf("light open !\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_LIGHT_ON, 0);
			nLastDirection = EM_BIZPTZ_CMD_LIGHT_ON;
		}break;
		case 13:
		{
			break;
			//printf("light close !\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_LIGHT_OFF, 0); 
			nLastDirection = EM_BIZPTZ_CMD_LIGHT_OFF;
		}break;
		#if 1
		// 2.0支持速率范围1-10
		case 17:
		{
			//printf("speed normal !\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_SETSPEED, pPtzCtrl->reserved);
			nLastDirection = EM_BIZPTZ_CMD_SETSPEED;
		}break;
		#else
		case 16:
		{
			//printf("speed fast !\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_SETSPEED, 10);
			nLastDirection = EM_BIZPTZ_CMD_SETSPEED;
		}break;
		case 17:
		{
			//printf("speed normal !\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_SETSPEED, 5);
			nLastDirection = EM_BIZPTZ_CMD_SETSPEED;
		}break;
		case 18:
		{
			//printf("speed slow !\n");
			BizPtzCtrl((u8)curChn, EM_BIZPTZ_CMD_SETSPEED, 2);
			nLastDirection = EM_BIZPTZ_CMD_SETSPEED;
		}break;	
		#endif
		default:
			//printf("ptz err cmd= %d!\n",pPtzCtrl->cmd);
			break;
	}
}


void DealRemoteCmd_Getmanualrec(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	SBizRecPara bizRecPara;

	ifly_ManualRecord_t* pManualRecord = &pRslt->sBasicInfo.ManualRecord;
	for( i=0; i<GetVideoMainNum(); i++ )
	{
		bizTar.nChn = i;
		int ret = BizGetPara(&bizTar, &bizRecPara);
		if(ret==0)
		{
			if(bizRecPara.bRecording)
			{
				pManualRecord->chnRecState |= 1<<i;
			}
			else
			{
				pManualRecord->chnRecState &= ~(1<<i);				
			}
		}
	}
}


void DealRemoteCmd_Setmanualrec(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	SBizRecPara bizRecPara;
	ifly_ManualRecord_t* pManualRecord = &pCmd->sReq.ManualRecord;

	for(i=0; i<GetVideoMainNum(); i++)
	{
		bizTar.nChn = i;
		int ret = BizGetPara(&bizTar, &bizRecPara);
		if(ret==0)
		{
			bizRecPara.bRecording = (pManualRecord->chnRecState & (1<<i))?1:0;
			if(bizRecPara.bRecording)
			{
				BizStartManualRec(i);
				BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONTROL  , BIZ_LOG_SLAVE_START_MANUAL_REC );
			}
			else
			{
				BizStopManualRec(i);
				BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONTROL  , BIZ_LOG_SLAVE_STOP_MANUAL_REC);			
			}
			
			BizSetPara(&bizTar, &bizRecPara);
		}
	}
}

void DealRemoteCmd_Logsearch(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	ifly_search_log_t *pPara_infoIn;
	SCPLogInfo	  *pSearchLog;
	
	pSearchLog = &pRslt->sBasicInfo.sLogInfo;
	pPara_infoIn = &pCmd->sReq.search_log;

	SBizLogResult* psLogResult;
	/*
	u8		query_mode;					//查询方式		
	u8		main_type;					//主类型		
	u8		slave_type;					//次类型		
	u16		max_return;					//最大返回数		
	u16		startID;					//返回从第一条记录开始		
	u32		start_time;					//开始时间		
	u32		end_time;					//结束时间
	*/
	psLogResult = (SBizLogResult*)calloc(1, sizeof(SBizLogResult));
	psLogResult->nMaxLogs = 0x4ff;
	psLogResult->psLogList = (SBizLogInfoResult*)calloc(psLogResult->nMaxLogs, sizeof(SBizLogInfoResult));

	if(psLogResult->psLogList)
	{	
		memset(psLogResult->psLogList, 0, sizeof(psLogResult->nMaxLogs*sizeof(SBizLogInfoResult)));
		SBizLogSearchPara para;
		para.nStart = pPara_infoIn->start_time;
		para.nEnd = pPara_infoIn->end_time;
		//printf("net search log main  type %d \n", pPara_infoIn->main_type);
		if(pPara_infoIn->main_type >= 7)
			para.nMasterMask = 0xff;
		else
			para.nMasterMask = (1<<pPara_infoIn->main_type);
		if(0 == BizSearchLog(&para, psLogResult))
		{
			for(i=0; i<psLogResult->nRealNum; i++)
			{
				pSearchLog->para_log[i].startTime = psLogResult->psLogList[i].nTime;
				pSearchLog->para_log[i].main_type = (psLogResult->psLogList[i].nType&0xff00)>>8;
				pSearchLog->para_log[i].slave_type = (psLogResult->psLogList[i].nType&0xff);
				pSearchLog->para_log[i].chn = psLogResult->psLogList[i].nchn;
				sprintf( pSearchLog->para_log[i].loginfo, "log %d", i );
			}
		}

		pSearchLog->Desc.sum = psLogResult->nRealNum;
		
		//printf("time %ld %ld lognum %d \n", para.nStart, para.nEnd, psLogResult->nRealNum);

		free(psLogResult->psLogList);
		free(psLogResult);
	}	
}

//xdc
void DealRemoteCmd_Ipcsearch(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	printf(" DealRemoteCmd_Ipcsearch is start\n");
	ipc_node *m_ipc_head;
	ifly_search_ipc_t* ipcSeach;
	SCPIpcInfo	  *pSearchIpc;
	pSearchIpc = &pRslt->sBasicInfo.sIpcInfo;
	ipcSeach = &pCmd->sReq.search_ipc;	

	printf("yg NVR search protocol_type: 0x%x\n", ipcSeach->protocol_type);
	
	IPC_Search(&m_ipc_head, ipcSeach->protocol_type, 0);
	int count = 0;
	ipc_node *p = m_ipc_head;
	while(p)
	{
		struct in_addr host;
		host.s_addr = p->ipcam.dwIp;
		printf(" %d:%s \n",count,inet_ntoa(host));
		memcpy(&pSearchIpc->para_log[count],&p->ipcam,sizeof(ifly_ipc_info_t));
		count++;
		p = p->next;
	}
	pSearchIpc->Desc.sum = count;
	//printf(" pSearchIpc->Desc.sum = %d \n",pSearchIpc->Desc.sum);
	IPC_Free(m_ipc_head);
	printf(" DealRemoteCmd_Ipcsearch is end\n");
}

void DealRemoteCmd_GetAddIpcList(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_search_ipc_t* ipcSeach;
	SCPIpcInfo	  *pSearchIpc;
	pSearchIpc = &pRslt->sBasicInfo.sIpcInfo;
	ipcSeach = &pCmd->sReq.search_ipc;
	
	int i=0;
	int count = 0;
	for(i=0;i<GetVideoMainNum();i++)
	{
		SBizIPCameraPara ipcam;
		memset(&ipcam, 0, sizeof(ipcam));
		ipcam.channel_no = i;
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
		bizTar.nChn = ipcam.channel_no;
		BizGetPara(&bizTar, (void *)&ipcam);
		
		if(ipcam.enable)
		{	
			pSearchIpc->para_log[count].channel_no = i;
			pSearchIpc->para_log[count].enable = ipcam.enable;
			pSearchIpc->para_log[count].protocol_type = ipcam.protocol_type;
			pSearchIpc->para_log[count].trans_type = ipcam.trans_type;
			pSearchIpc->para_log[count].stream_type = ipcam.stream_type;
			pSearchIpc->para_log[count].ipc_type = ipcam.ipc_type;
			pSearchIpc->para_log[count].dwIp = ipcam.dwIp;
			pSearchIpc->para_log[count].wPort = ipcam.wPort;
			strcpy(pSearchIpc->para_log[count].user, ipcam.user);
			strcpy(pSearchIpc->para_log[count].pwd, ipcam.pwd);
			strcpy(pSearchIpc->para_log[count].name, ipcam.name);
			strcpy(pSearchIpc->para_log[count].uuid, ipcam.uuid);
			struct in_addr host;
			host.s_addr = ipcam.dwIp;
			sprintf(pSearchIpc->para_log[count].address, "http://%s:%d/onvif/device_service", inet_ntoa(host), ipcam.wPort);
			
			struct in_addr host1;
			host1.s_addr = pSearchIpc->para_log[count].dwIp;
			//printf(" %d:%s \n",count,inet_ntoa(host1));
			count++;
		}
		
	}
	pSearchIpc->Desc.sum = count;
}

void DealRemoteCmd_SetIpc(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	printf(" DealRemoteCmd_SetIpc is start\n");
	ifly_ipc_info_t*  ipc_info;
	ipc_info = &pCmd->sReq.ipc_info;
	SBizParaTarget bizTar;
	SBizIPCameraPara ipcam;
	bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
	bizTar.nChn = ipc_info->channel_no;
	ipcam.channel_no = ipc_info->channel_no;
	ipcam.enable = ipc_info->enable;
	ipcam.stream_type = ipc_info->stream_type;
	ipcam.trans_type = ipc_info->trans_type;
	ipcam.protocol_type = ipc_info->protocol_type;
	ipcam.dwIp = ipc_info->dwIp;
	ipcam.wPort = ipc_info->wPort;
	ipcam.force_fps = ipc_info->force_fps;
	ipcam.frame_rate = ipc_info->frame_rate;
	ipcam.ipc_type = ipc_info->ipc_type;
	
	strcpy(ipcam.user, ipc_info->user);
	strcpy(ipcam.pwd, ipc_info->pwd);
	strcpy(ipcam.name, ipc_info->name);
	strcpy(ipcam.uuid, ipc_info->uuid);
	strcpy(ipcam.address , ipc_info->address);
	
	BizSetPara(&bizTar, (void *)&ipcam);
	printf(" DealRemoteCmd_SetIpc is end\n");
}

//channel_no 取值在[1-16]，则添加到指定通道
//= 0，则添加到最小未使用的通道
void DealRemoteCmd_AddIpc(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	printf(" DealRemoteCmd_AddIpc is start\n");
	ifly_ipc_info_t*  camera;
	camera = &pCmd->sReq.ipc_info;
	int i=0;

	if (camera->channel_no > 16)
	{
		printf("yg DealRemoteCmd_AddIpc channelno invaild\n");
		return 0;
	}

	if (camera->channel_no == 0)
	{
		for(i=0;i<GetVideoMainNum();i++)
		{
			SBizIPCameraPara ipcam;
			memset(&ipcam, 0, sizeof(ipcam));
			ipcam.channel_no = i;
			
			SBizParaTarget bizTar;
			bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
			bizTar.nChn = ipcam.channel_no;
			BizGetPara(&bizTar, (void *)&ipcam);
			
			if(ipcam.enable)
			{
				if((ipcam.protocol_type == camera->protocol_type) && (ipcam.dwIp == camera->dwIp))
				{
					if(ipcam.wPort != camera->wPort)
					{
						ipcam.channel_no = i;
						ipcam.enable = 1;
						ipcam.protocol_type = camera->protocol_type;
						ipcam.trans_type = camera->trans_type;
						ipcam.stream_type = camera->stream_type;
						ipcam.ipc_type = camera->ipc_type;
						ipcam.dwIp = camera->dwIp;
						ipcam.wPort = camera->wPort;
						strcpy(ipcam.user, camera->user);
						strcpy(ipcam.pwd, camera->pwd);
						strcpy(ipcam.name, camera->name);
						strcpy(ipcam.uuid, camera->uuid);
						struct in_addr host;
						host.s_addr = camera->dwIp;
						sprintf(ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), camera->wPort);
						
						BizSetPara(&bizTar, (void *)&ipcam);
					}
					
					return TRUE;
				}
			}
		}
		for(i=0;i<GetVideoMainNum();i++)
		{
			SBizIPCameraPara ipcam;
			memset(&ipcam, 0, sizeof(ipcam));
			ipcam.channel_no = i;
			
			SBizParaTarget bizTar;
			bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
			bizTar.nChn = ipcam.channel_no;
			BizGetPara(&bizTar, (void *)&ipcam);
			
			if(!ipcam.enable)
			{
				ipcam.channel_no = i;
				ipcam.enable = 1;
				ipcam.protocol_type = camera->protocol_type;
				ipcam.trans_type = camera->trans_type;
				ipcam.stream_type = camera->stream_type;
				ipcam.ipc_type = camera->ipc_type;
				ipcam.dwIp = camera->dwIp;
				ipcam.wPort = camera->wPort;
				strcpy(ipcam.user, camera->user);
				strcpy(ipcam.pwd, camera->pwd);
				strcpy(ipcam.name, camera->name);
				strcpy(ipcam.uuid, camera->uuid);
				struct in_addr host;
				host.s_addr = camera->dwIp;
				sprintf(ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), camera->wPort);
				
				BizSetPara(&bizTar, (void *)&ipcam);
				
				return TRUE;
			}
		}
	}
	else
	{
		SBizIPCameraPara ipcam;
		memset(&ipcam, 0, sizeof(ipcam));
		ipcam.channel_no = camera->channel_no - 1;//[1-16] ==> [0-15]
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
		bizTar.nChn = ipcam.channel_no;
		BizGetPara(&bizTar, (void *)&ipcam);

		ipcam.enable = 1;
		ipcam.protocol_type = camera->protocol_type;
		ipcam.trans_type = camera->trans_type;
		ipcam.stream_type = camera->stream_type;
		ipcam.ipc_type = camera->ipc_type;
		ipcam.dwIp = camera->dwIp;
		ipcam.wPort = camera->wPort;
		strcpy(ipcam.user, camera->user);
		strcpy(ipcam.pwd, camera->pwd);
		strcpy(ipcam.name, camera->name);
		strcpy(ipcam.uuid, camera->uuid);
		struct in_addr host;
		host.s_addr = camera->dwIp;
		printf("yg DealRemoteCmd_AddIpc ipc addr : %s\n", inet_ntoa(host));
		sprintf(ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), camera->wPort);
		
		BizSetPara(&bizTar, (void *)&ipcam);
	}
	printf(" DealRemoteCmd_AddIpc is end\n");
}

void DealRemoteCmd_DeleteIpc(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	printf(" DealRemoteCmd_DeleteIpc is start\n");
	ifly_ipc_info_t*  ipc_info;
	ipc_info = &pCmd->sReq.ipc_info;

	struct in_addr host;
	host.s_addr = ipc_info->dwIp;
	printf("yg DealRemoteCmd_DelIpc ipc addr : %s\n", inet_ntoa(host));

	int chn = ipc_info->channel_no;
	
	SBizIPCameraPara ipcam;
	memset(&ipcam, 0, sizeof(ipcam));
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
	bizTar.nChn = chn;
	BizGetPara(&bizTar, (void *)&ipcam);
	
	if(ipcam.enable)
	{
		ipcam.channel_no = chn;
		ipcam.enable = 0;
		
		BizSetPara(&bizTar, (void *)&ipcam);
	}
	printf(" DealRemoteCmd_DeleteIpc is start\n");
}

//xdc end

//yaogang modify 20141030
void DealRemoteCmd_GetPatrolPara(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PATROLPARA;
	SBizCfgPatrol bizPara;
	
	int ret = BizGetPara(&bizTar, &bizPara);
	if(ret ==0)
	{
		pRslt->sBasicInfo.patrol_para.nIsPatrol = bizPara.nIsPatrol; //轮巡是否启用(数字:0=否;1=是)
		pRslt->sBasicInfo.patrol_para.nInterval = bizPara.nInterval; //轮巡切换时间(数字:单位秒s)
		pRslt->sBasicInfo.patrol_para.nPatrolMode = bizPara.nPatrolMode; //轮巡时所用的预览模式(数字:参考Mode)
		pRslt->sBasicInfo.patrol_para.nPatrolChnNum = 16;//轮巡通道数16 32 48 64，64路解码器专用
		printf("bizPara.nIsPatrol: %d\n", bizPara.nIsPatrol);
		printf("bizPara.nInterval: %d\n", bizPara.nInterval);
		printf("bizPara.nPatrolMode: %d\n", bizPara.nPatrolMode);
		
	}
}

void DealRemoteCmd_SetPatrolPara(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PATROLPARA;
	SBizCfgPatrol bizPara;
	int ret = BizGetPara(&bizTar, &bizPara);
	if(ret ==0)
	{
		bizPara.nIsPatrol = pCmd->sReq.patrol_para.nIsPatrol; //轮巡是否启用(数字:0=否;1=是)
		bizPara.nInterval = pCmd->sReq.patrol_para.nInterval ; //轮巡切换时间(数字:单位秒s)
		bizPara.nPatrolMode = pCmd->sReq.patrol_para.nPatrolMode; //轮巡时所用的预览模式(数字:参考Mode)
							//轮巡通道数16 32 48 64，64路解码器专用，其他不理会
							//pCmd->sReq.patrol_para.nPatrolChnNum
		BizSetPara(&bizTar, &bizPara);
	}
}
//preview para
void DealRemoteCmd_GetPreviewPara(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPara;
	ifly_preview_para_t *preview_para = &pRslt->sBasicInfo.preview_para;
	EMBIZPREVIEWMODE  PreviewMode;
	u8 ModePara;

	bizData_GetPreviewPara(&PreviewMode, &ModePara);
	printf("GetPreviewPara nMode: %u, ModePara:%d\n", PreviewMode, ModePara);
	
	preview_para->nPreviewMode = PreviewMode; //当前所用的预览模式(数字:参考Mode 1 4 9 16)	
	preview_para->ModePara = ModePara;//预览起始通道
}

void DealRemoteCmd_SetPreviewPara(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPara;
	ifly_preview_para_t *preview_para = &pCmd->sReq.preview_para;

	EMBIZPREVIEWMODE  PreviewMode;
	u8 ModePara;

	bizData_GetPreviewPara(&PreviewMode, &ModePara);
	printf("GetPreviewPara nMode: %u, ModePara:%d\n", PreviewMode, ModePara);

	if ( (PreviewMode != preview_para->nPreviewMode) 
		|| (ModePara != preview_para->ModePara) )
	{
		PreviewMode = preview_para->nPreviewMode;
		ModePara = preview_para->ModePara;//0-15

		if((ModePara + PreviewMode) > 16)
		{
			if(16 != ModePara)
			{
				if(16 > PreviewMode)//cw_9508S
					ModePara = 16 - PreviewMode;
				else
					ModePara = 0;
			}
		}
		
	    	ModePara %= 16;
		printf("SetPreviewPara nMode: %u, ModePara:%d\n", PreviewMode, ModePara);
		SwitchPreview(PreviewMode, ModePara);
	}
	
	/*
	ret = BizGetPara(&bizTar, &bizPara);
	if(ret ==0)
	{
		printf("before SetPreviewPara nMode: %u\n", bizPara.nMode);

		//SetCurPreviewMode_CW(preview_para->nPreviewMode);
		if (bizPara.nMode != preview_para->nPreviewMode)
		{
			bizPara.nMode = preview_para->nPreviewMode;
			//BizSetPara(&bizTar, &bizPara);
			SetCurPreviewMode_CW(bizPara.nMode);//cw_preview
			SwitchPreview(bizPara.nMode, 0);
		}
		else
		{
			SetCurPreviewMode_CW(bizPara.nMode);//cw_preview
			SwitchPreview(bizPara.nMode, 0xff);//0xff模式不切换，通道自动切换
		}
		
	}
	*/
	/*
	ret = bizData_GetPreviewMode();//g_pBizPreviewData.emBizPreviewMode;
	if (ret != preview_para->nPreviewMode)
	{
		ret = preview_para->nPreviewMode;
		//BizSetPara(&bizTar, &bizPara);
		SetCurPreviewMode_CW(ret);//cw_preview
		SwitchPreview(ret, 0);
	}
	else
	{
		SetCurPreviewMode_CW(ret);//cw_preview
		SwitchPreview(ret, 0xff);//0xff模式不切换，通道自动切换
	}
	*/
}

//清除桌面报警三角标（注意只有在当前没有报警源存在时，才会清除）
void DealRemoteCmd_CleanDesktopAlarmIcon(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	printf("yg DealRemoteCmd_CleanDesktopAlarmIcon\n");
	RefreshAllStatus();
}

void DealRemoteCmd_CloseGuide()
{
	Biz_CloseGuide();
}

//yaogang modify 20160122
//获取IPC通道连接状态(一个IPC有两个通道，主、子码流)
void DealRemoteCmd_GetIPCChnLinkStatus(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_ipc_chn_status_t *ipc_chn_status = &pRslt->sBasicInfo.ipc_chn_status;
	int nChMax = GetVideoMainNum();
	int i = 0;

	memset(ipc_chn_status, 0, sizeof(ifly_ipc_chn_status_t));
	ipc_chn_status->max_chn_num = nChMax*2;//主/子码流两路
	
	for (i=0; i<nChMax*2; i++)
	{
		if (IPC_GetLinkStatus(i))
		{
			ipc_chn_status->chn_status[i/8] |= 1<<(i%8);//unsigned char == 8bit
			//printf("%s i: %d, chn_status[%d]: 0x%x\n", 
				//__func__, i, i/8, ipc_chn_status->chn_status[i/8]);
		}
	}
}


//yaogang modify 20141030 end

void DealRemoteCmd_Recfilesearch(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Getspspps(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Alarmuploadcenter(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Serialstop(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Com_Protocol_Get(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}

void DealRemoteCmd_Com_Protocol_Set(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Getsyslanglist(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_SysLangList_t* pSyslanglist = NULL;
	pSyslanglist = &pRslt->sBasicInfo.SysLangList;
	memset(pSyslanglist, 0, sizeof(ifly_SysLangList_t));
	
	int i;
	u8	nReal = 0;
	int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_LANGSUP, (s8**)g_strList, (u8*)&nReal, nMaxListLen, nMaxStrLen);
	if(ret == 0)
	{
		pSyslanglist->max_langnum = nReal;
		for(i=0; i<nReal; i++)
		{
			pSyslanglist->langlist[i] = i;
		}
	}
}

void DealRemoteCmd_Getbitratelist(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	
	ifly_bitRateList_t* pBitRateList;	
	
	pBitRateList = &pRslt->sBasicInfo.bitRateList;
	
	memset(pBitRateList, 0, sizeof(ifly_bitRateList_t));
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = pCmd->sReq.nChn;
	
	SBizRecPara bizRecPara;
	pBitRateList->chn = pCmd->sReq.nChn;
	
	int ret = BizGetPara(&bizTar, &bizRecPara);
	if(ret!=0)
	{
		pBitRateList->videotype = bizRecPara.nEncChnType;//todo
	}
	
	u8 nReal = 0;
	s32 nVal;
	
	InitStrList();
	
	ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_BITRATE, (s8**)g_strList, (u8*)&nReal, nMaxListLen, nMaxStrLen);
	if(ret==0)
	{
		u8 nLimit = sizeof(pBitRateList->bitratelist)/sizeof(pBitRateList->bitratelist[0]);
		nReal = (nReal>nLimit)?nLimit:nReal;
		
		memset(pBitRateList->bitratelist, 0, sizeof(pBitRateList->bitratelist));
		
		for( i=0; i<nReal; i++ )
		{
			BizConfigGetParaListValue(		
				EM_BIZ_CFG_PARALIST_BITRATE, 
				i,
				&nVal
			);
			if(nVal>0)
			{
				pBitRateList->bitratelist[i] = nVal;
				////printf("bitratelist %d %d %d %d \n", nReal, nLimit, i, nVal);
			}
		}
	}
}


void DealRemoteCmd_Getemailsmtp(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_AlarmEmail_SMTP_t* pAlarm_mail_smtp;

	pAlarm_mail_smtp = &pRslt->sBasicInfo.AlarmEmail_SMTP;
	
	SBizNetPara sPIns;
	
	if(0 == ConfigGetNetPara(0, &sPIns, 0))
	{
		/*
		//printf("%s \n %s \n %s \n %s \n %s \n %s \n",
			sPIns.sAdancePara.szReceiveMailBox,
			sPIns.sAdancePara.szReceiveMailBox2,
			sPIns.sAdancePara.szReceiveMailBox3,
			sPIns.sAdancePara.szSmtpServer,
			sPIns.sAdancePara.szSendMailBox,
			sPIns.sAdancePara.szSMailPasswd			
		);
		*/
		
		if(strcmp(sPIns.sAdancePara.szReceiveMailBox, "0") == 0)
			strcpy( pAlarm_mail_smtp->alarm_email1, "" );
		else
			strcpy(pAlarm_mail_smtp->alarm_email1, sPIns.sAdancePara.szReceiveMailBox);
			
		if(strcmp(sPIns.sAdancePara.szReceiveMailBox2, "0") == 0)
			strcpy( pAlarm_mail_smtp->alarm_email2, "" );
		else
			strcpy(pAlarm_mail_smtp->alarm_email2, sPIns.sAdancePara.szReceiveMailBox2);
			
		if(strcmp(sPIns.sAdancePara.szReceiveMailBox3, "0") == 0)
			strcpy( pAlarm_mail_smtp->alarm_email3, "" );
		else
			strcpy(pAlarm_mail_smtp->alarm_email3, sPIns.sAdancePara.szReceiveMailBox3);
			
		if(strcmp(sPIns.sAdancePara.szSmtpServer, "0") == 0)
			strcpy( pAlarm_mail_smtp->SMTP_svr, "" );
		else
			strcpy(pAlarm_mail_smtp->SMTP_svr, sPIns.sAdancePara.szSmtpServer);
			
		if(strcmp(sPIns.sAdancePara.szSendMailBox, "0") == 0)
			strcpy( pAlarm_mail_smtp->username, "" );
		else
			strcpy(pAlarm_mail_smtp->username, sPIns.sAdancePara.szSendMailBox);
			
		if(strcmp(sPIns.sAdancePara.szSMailPasswd, "0") == 0)
			strcpy( pAlarm_mail_smtp->userpw, "" );
		else
			strcpy(pAlarm_mail_smtp->userpw, sPIns.sAdancePara.szSMailPasswd);
		
		pAlarm_mail_smtp->smtp_port = sPIns.SMTPServerPort;
	}
}


void DealRemoteCmd_Setemailsmtp(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_AlarmEmail_SMTP_t* pAlarm_mail_smtp;

	pAlarm_mail_smtp = &pCmd->sReq.AlarmEmail_SMTP;
	
	SBizNetPara sPIns;
	
	if(0 == ConfigGetNetPara(0, &sPIns, 0))
	{
		strcpy(sPIns.sAdancePara.szReceiveMailBox, pAlarm_mail_smtp->alarm_email1);
		strcpy(sPIns.sAdancePara.szReceiveMailBox2, pAlarm_mail_smtp->alarm_email2);
		strcpy(sPIns.sAdancePara.szReceiveMailBox3, pAlarm_mail_smtp->alarm_email3);
		strcpy(sPIns.sAdancePara.szSmtpServer, pAlarm_mail_smtp->SMTP_svr);
		strcpy(sPIns.sAdancePara.szSendMailBox, pAlarm_mail_smtp->username);
		strcpy(sPIns.sAdancePara.szSMailPasswd, pAlarm_mail_smtp->userpw);
		sPIns.SMTPServerPort = pAlarm_mail_smtp->smtp_port;
		
		/*
		printf("%s \n %s \n %s \n %s \n %s \n %s \n, port %d \n",
			sPIns.sAdancePara.szReceiveMailBox,
			sPIns.sAdancePara.szReceiveMailBox2,
			sPIns.sAdancePara.szReceiveMailBox3,
			sPIns.sAdancePara.szSmtpServer,
			sPIns.sAdancePara.szSendMailBox,
			sPIns.sAdancePara.szSMailPasswd,
			sPIns.SMTPServerPort
		);
		*/
		
		ConfigSetNetPara(&sPIns, 0);
		
		BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_OUTBOX_CONFIG);
		BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_INBOX_CONFIG );
	}
}


void DealRemoteCmd_Useutctime(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;
	
	// todo
}


void DealRemoteCmd_Resetpicadjust(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	ifly_reset_picadjust_t* pReset_picadjust;
	
	pReset_picadjust = &pCmd->sReq.reset_picadjust;
	
	SBizPreviewImagePara sPIns, sRealPIns;

	u8 nId = pReset_picadjust->chn;
	if( 0 == ConfigGetPreviewImagePara(1, &sPIns, nId) )
	{
		if(pReset_picadjust->adjtype != 0xf)
		{

			if( 0 == ConfigGetPreviewImagePara(0, &sRealPIns, nId) )
			{
				for(i=0; i<4; i++)
				{
					if( 0 == (pReset_picadjust->adjtype&(1<<i)) )
					{
						switch(i)
						{
							case 0:
							{
								sPIns.nBrightness = sRealPIns.nBrightness;
							} break;
							case 1:
							{
								sPIns.nContrast = sRealPIns.nContrast;
							} break;
							case 2:
							{
								sPIns.nHue = sRealPIns.nHue;
							} break;
							case 3:
							{
								sPIns.nSaturation = sRealPIns.nSaturation;
							} break;
						}
					}
				}
			}
		}

		ConfigSetPreviewImagePara(&sPIns, nId);
		BizPreviewSetImage(nId, &sPIns);
	}
}


void DealRemoteCmd_Getframeratelist(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	u8 i;
	
	ifly_framerate_list_t* pFrameRateList;
	
	pFrameRateList = &pRslt->sBasicInfo.framerate_list;
	memset(pFrameRateList, 0, sizeof(ifly_framerate_list_t));
	
	SBizParaTarget bizTar;
	
	u8 nVideoStand;
	
	pFrameRateList->chn 	= pCmd->sReq.nChn;
	
	bizTar.nChn 			= pCmd->sReq.nChn;	
	bizTar.emBizParaType 	= EM_BIZ_SYSTEMPARA;
	
	SBizSystemPara 			bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret==0)
	{
		nVideoStand = bizSysPara.nVideoStandard;
		
		// get framerate list
		// by videsostand
		u8 nReal = 0;
		u32 eResolist;
		switch(pCmd->sReq.framerate_list.type)
		{
			case VIDEORESOLU_QCIF:
			case VIDEORESOLU_CIF:
			{
				eResolist = (EM_BIZ_NTSC == nVideoStand)?EM_BIZ_CFG_PARALIST_FPSNTSCCIF:EM_BIZ_CFG_PARALIST_FPSPALCIF;
			} break;
			case VIDEORESOLU_HD1:
			case VIDEORESOLU_D1:
			case VIDEORESOLU_720P:
			case VIDEORESOLU_1080P:
			case VIDEORESOLU_960H:
			{
				eResolist = (EM_BIZ_NTSC == nVideoStand)?EM_BIZ_CFG_PARALIST_FPSNTSCD1:EM_BIZ_CFG_PARALIST_FPSPALD1;
			} break;
			default:
			{
				//csp modify 20130525
				eResolist = (EM_BIZ_NTSC == nVideoStand)?EM_BIZ_CFG_PARALIST_FPSNTSCD1:EM_BIZ_CFG_PARALIST_FPSPALD1;
				//return;
			} break;
		}
		
		InitStrList();
		
		int ret = BizConfigGetParaStr(eResolist, (s8**)g_strList, (u8*)&nReal, nMaxListLen, nMaxStrLen);
		if(ret==0)
		{
			u8 nRemoteLimit = 
				sizeof(pFrameRateList->framerate)/sizeof(pFrameRateList->framerate[0]);
			nReal = (nReal>nRemoteLimit)?nRemoteLimit:nReal;
			
			for( i=0; i<nReal; i++ )
			{
				s32 nVal;
				BizConfigGetParaListValue(		
					eResolist, 
					i, 
					&nVal
				);
				
				////printf("net req fr %d\n", nVal);
				
				pFrameRateList->framerate[i] = nVal;
			}
		}
		////printf("net req fr\n");
	}
}

void DealRemoteCmd_Getmax_Imgmasknum(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}

void DealRemoteCmd_Get_Resolution_List(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	
	ifly_videoresolu_list_t* pVideoresolu_list, *pVideoresolu_listIn;
	
	pVideoresolu_listIn = &pCmd->sReq.videoresolu_list;
	pVideoresolu_list = &pRslt->sBasicInfo.videoresolu_list;
	memset( pVideoresolu_list, 0, sizeof(ifly_videoresolu_list_t) );
	
	u8 nReal = 0;
	u32 nType = 0;
	
	if(0 == pVideoresolu_listIn->type)// 主码流
	{
		int Hnum = 0;//cw_9508S
		Hnum = GetBizResolHNum();
		if(pVideoresolu_listIn->chn < Hnum)
			nType = EM_BIZ_CFG_PARALIST_VMAINRESOL;
		else
			nType = EM_BIZ_CFG_PARALIST_VMAINRESOL2;
 	}
	else if(1 == pVideoresolu_listIn->type)// 子码流
	{
		nType = EM_BIZ_CFG_PARALIST_VSUBRESOL;
	}
	else
	{
		//printf("The cmd of video resolution type is error\n");
		return;
	}
	
	InitStrList();
	
	int ret = BizConfigGetParaStr(nType, (s8**)g_strList, (u8*)&nReal, 20, 64);
	if(0 == ret)
	{
		s32 nValue;
		for(i=0; i<nReal; i++)
		{
			if( 0 == BizConfigGetParaListValue(nType, i, &nValue) )
			{
				switch(nValue)
				{
					case EM_BIZENC_RESOLUTION_CIF:
					{
						pVideoresolu_list->videoresolu[i] = VIDEORESOLU_CIF;
					} break;						
					case EM_BIZENC_RESOLUTION_QCIF:
					{
						pVideoresolu_list->videoresolu[i] = VIDEORESOLU_QCIF;
					} break;					
					case EM_BIZENC_RESOLUTION_HD1:
					{
						pVideoresolu_list->videoresolu[i] = VIDEORESOLU_HD1;
					} break;					
					case EM_BIZENC_RESOLUTION_D1:
					{
						pVideoresolu_list->videoresolu[i] = VIDEORESOLU_D1;
					} break;			
					case EM_BIZENC_RESOLUTION_4CIF:
					{
						pVideoresolu_list->videoresolu[i] = VIDEORESOLU_D1;
					} break;
					case EM_BIZENC_RESOLUTION_720P:
					{
						pVideoresolu_list->videoresolu[i] = VIDEORESOLU_720P;
					} break;
					case EM_BIZENC_RESOLUTION_1080P:
					{
						pVideoresolu_list->videoresolu[i] = VIDEORESOLU_1080P;
					} break;
					//csp modify
					case EM_BIZENC_RESOLUTION_960H:
					{
						pVideoresolu_list->videoresolu[i] = VIDEORESOLU_960H;
					} break;
					default:
						return;					
				}
			}
			else
			{
				return;
			}
		}
	}
	else
	{
		return;
	}
}

void DealRemoteCmd_Pppoe_Preup(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Pppoe_Up(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Pppoe_Down(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Pppoe_Disconnect(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}

void DealRemoteCmd_Dhcp_Deconfig(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;

	// todo
}


void DealRemoteCmd_Dhcp_Bound(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	if(pCmd)
	{
		SBizNetPara sNetConfig;

		ConfigGetNetPara(0, &sNetConfig, 0);
		if (0 == strcmp(pCmd->sReq.cp_dhcp.ifrname, "eth0"))
		{
			SNetPara sNetPara;
			
			NetComm_ReadPara(&sNetPara);
			
			sNetConfig.HostIP = sNetPara.HostIP;
			sNetConfig.Submask = sNetPara.Submask;
			sNetConfig.GateWayIP = sNetPara.GateWayIP;
			sNetConfig.DNSIP = sNetPara.DNSIP;
			sNetConfig.DNSIPAlt = sNetPara.DNSIPAlt;
			
			ConfigSetNetPara(&sNetConfig, 0);
		}
	}
}

void DealRemoteCmd_Dhcp_Renew(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	DealRemoteCmd_Dhcp_Bound(pCmd, pRslt);
}

void DealRemoteCmd_Dhcp_Nak(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_NetWork_t* psIns = &pCmd->sReq.NetWork;
	
	//todo
}

//csp modify 20130423
void DealRemoteCmd_SetMonitorInfo(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	
}

//csp modify 20130519
static BOOL IsBigEndian()
{
	const int n = 1;
	if(*(char *)&n)
	{
		return FALSE;
	}
	return TRUE;
}
#define swap64(val) ( \
						(((val) >> 56) & 0xff) |\
						(((val) & 0x00ff000000000000ll) >> 40) |\
						(((val) & 0x0000ff0000000000ll) >> 24) |\
						(((val) & 0x000000ff00000000ll) >> 8)  |\
						(((val) & 0x00000000ff000000ll) << 8)  |\
						(((val) & 0x0000000000ff0000ll) << 24) |\
						(((val) & 0x000000000000ff00ll) << 40) |\
						(((val) & 0x00000000000000ffll) << 56) )
#define hton64(val) IsBigEndian()?(val):swap64(val)
#define ntoh64(val) hton64(val)
void DealRemoteCmd_GetAdvPrivilege(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizUserUserPara sUserPara;
	strcpy(sUserPara.UserName, pCmd->sReq.loginpara.username);
	if(0 == UserGetUserInfo(&sUserPara))
	{
		strcpy(pRslt->sBasicInfo.AdvPrivilege.username, sUserPara.UserName);
		pRslt->sBasicInfo.AdvPrivilege.nRemoteView[0] = hton64(sUserPara.UserAuthor.nRemoteView[0]);
		printf("user[%s] RemoteView privilege:(0x%08x,0x%08x)-(0x%08x,0x%08x)\n",
			sUserPara.UserName,
			(u32)(sUserPara.UserAuthor.nRemoteView[0]),
			(u32)(sUserPara.UserAuthor.nRemoteView[0]>>32),
			(u32)(pRslt->sBasicInfo.AdvPrivilege.nRemoteView[0]),
			(u32)(pRslt->sBasicInfo.AdvPrivilege.nRemoteView[0]>>32));
	}
}

void DealRemoteCmd_Stopvideomonitor(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Stopaudiomonitor(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Stopvoip(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Stopdownload(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Stopfileplay(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Stoptimeplay(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Fastplay(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Slowplay(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Setplayrate(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Pauseplay(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Resumeplay(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Singleplay(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Fastbackplay(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Playprev(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Playnext(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Playseek(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Playmute(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}
void DealRemoteCmd_Playprogress(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}

void DealRemoteCmd_Getalarmnoticyparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{

}

void DealRemoteCmd_Setalarmnoticyparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{}

void DealRemoteCmd_Getsubstreamparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	SBizParaTarget 	bizTar;
	
	int i;
	u8                  	nCh   = pCmd->sReq.nChn;
	ifly_SubStreamParam_t* 	psIns = &pRslt->sBasicInfo.SubStreamParam;
	
	bizTar.nChn 			= nCh;
	bizTar.emBizParaType 	= EM_BIZ_ENCSUBPARA;
	
	SBizEncodePara 			sEncIns;
	int ret = BizGetPara(&bizTar, &sEncIns);
	if( 0 == ret )
	{
		psIns->channelno 	= nCh;
		
		switch(sEncIns.nBitRateType)
		{
			case EM_BIZENC_BITRATE_VARIABLE:
			{
				psIns->sub_bit_type 	= 1;
			} break;
			case EM_BIZENC_BITRATE_CONST:
			{
				psIns->sub_bit_type 	= 0;
			} break;
		}
		
		psIns->sub_bitrate		= sEncIns.nBitRate;
		psIns->sub_intraRate 	= sEncIns.nGop;
		
		BizConfigGetParaListIndex(
			EM_BIZ_CFG_PARALIST_VSUBRESOL, 
			sEncIns.nVideoResolution, 
			&psIns->sub_flag
		);
		
		psIns->sub_framerate 	= sEncIns.nFrameRate;
		
		psIns->sub_quality 		= sEncIns.nPicLevel;
		psIns->sub_minQ 		= sEncIns.nMinQP;
		psIns->sub_maxQ 		= sEncIns.nMaxQP;
	}
}

void DealRemoteCmd_Setsubstreamparam(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{	
	SBizParaTarget 	bizTar;
	
	ifly_SubStreamParam_t* 	psIns = &pCmd->sReq.SubStreamParam;
	
	bizTar.nChn 			= psIns->channelno;
	bizTar.emBizParaType 	= EM_BIZ_ENCSUBPARA;
	
	SBizEncodePara 			sEncIns;
	int ret = BizGetPara(&bizTar, &sEncIns);
	if( 0 == ret )
	{
		sEncIns.nBitRateType	=	psIns->sub_bit_type?EM_BIZENC_BITRATE_VARIABLE:EM_BIZENC_BITRATE_CONST;//psIns->sub_bit_type?0:1;
		sEncIns.nBitRate 		= 	psIns->sub_bitrate;
		sEncIns.nGop			=	psIns->sub_intraRate;
		sEncIns.nFrameRate		=	psIns->sub_framerate;
		
		sEncIns.nPicLevel		=	psIns->sub_quality;
		sEncIns.nMinQP			=	psIns->sub_minQ;
		sEncIns.nMaxQP			=	psIns->sub_maxQ;
		
		#if 1//csp modify
		s32 value = EM_BIZENC_RESOLUTION_CIF;
		BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_VSUBRESOL,psIns->sub_flag,&value);
		sEncIns.nVideoResolution = value;
		printf("chn%d sub stream resolution:%d\n",bizTar.nChn,sEncIns.nVideoResolution);
		#else
		if(psIns->sub_flag<(sizeof(sOldResoList)/sizeof(sOldResoList[0])))
		{
			switch( sOldResoList[psIns->sub_flag] )
			{
				case VIDEORESOLU_CIF:
				{
					sEncIns.nVideoResolution = EM_BIZENC_RESOLUTION_CIF;
				} break;						
				case VIDEORESOLU_QCIF:
				{
					sEncIns.nVideoResolution = EM_BIZENC_RESOLUTION_QCIF;
				} break;
				default:
					return;
			}
		}
		#endif
		
		BizSetPara(&bizTar, &sEncIns);
		BizNetWriteLog( pCmd->cph,  BIZ_LOG_MASTER_SYSCONFIG , BIZ_LOG_SLAVE_CHANGE_NET_CODESTREAM  );
	}
}

void DealRemoteCmd_Getspecdevinfo(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{;}

#define MD_SEND_MAX 	5

void DealRemoteCmd_GetMdSenseList(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_MDSenselist_t* pMdSenseList = &pRslt->sBasicInfo.MDSenselist;

	int i = 0;
	for(i=0; i<MD_SEND_MAX; i++)
	{
		pMdSenseList->mdsenselist[i] = i+1;
	}
}


void DealRemoteCmd_GetMdAlarmDelayList(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_MDAlarmDelaylist_t* pMDAlarmDelaylist = &pRslt->sBasicInfo.MDAlarmDelaylist;
	
	int 	i = 0;
	u8 		nReal;
	
	InitStrList();
	
	if( 0==BizConfigGetParaStr(
				EM_BIZ_CFG_PARALIST_DELAY, 
				g_strList, 
				&nReal, 
				nMaxListLen, 
				nMaxStrLen 
		)
	)
	{
		for(i=0; i<nReal; i++)
		{
			s32 nValue;
			BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_DELAY, i, &nValue);
			pMDAlarmDelaylist->mdalarmdelaylist[i] = htons((u16)nValue);//csp modify
			//printf("md delay %d \n", pMDAlarmDelaylist->mdalarmdelaylist[i]);
		}
	}
}

void DealRemoteCmd_GetBauteRateList(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_BaudRateList_t* pBaudRateList = &pRslt->sBasicInfo.BaudRateList;
	
	int 	i = 0;
	u8 		nReal = 0;
	
	InitStrList();
	
	//printf("GetBauteRateList......\n");
	
	if( 0==BizConfigGetParaStr(
				EM_BIZ_CFG_PARALIST_BAUDRATE, 
				g_strList, 
				&nReal, 
				nMaxListLen, 
				nMaxStrLen 
		)
	)
	{
		for(i=0; i<nReal; i++)
		{
			s32 nValue = 0;
			BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_BAUDRATE, i, &nValue);
			printf("baud rate list %d %d\n", i, nValue);
			pBaudRateList->baudratelist[i] = htonl(nValue);
		}
	}
}

void DealRemoteCmd_GetPtzProtocolList(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	ifly_PTZProtocolList_t* pPTZProtocolList = &pRslt->sBasicInfo.PTZProtocolList;
	
	memset(pPTZProtocolList, 0, sizeof(ifly_PTZProtocolList_t));
	pPTZProtocolList->maxNum = sizeof(sPtzProList)/sizeof(sPtzProList[0]);
	for(i=0; i<pPTZProtocolList->maxNum; i++)
		pPTZProtocolList->list[i] = sPtzProList[i].pro_id;
}

void DealRemoteCmd_GetRecSch(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i;
	ifly_RecSchTime_t* pRecordSchReq = &pCmd->sReq.RecSchTime;
	ifly_RecSchTime_t* pRecSch;
	
	SBizSchedule sCfgIns;
	SBizParaTarget sParaTgtIns;
	
	pRecSch = &pRslt->sBasicInfo.RecSchTime;
	
	pRecSch->chn 		= pRecordSchReq->chn;
	pRecSch->weekday 	= pRecordSchReq->weekday;
	
	sParaTgtIns.nChn 	= pRecordSchReq->chn;
	pRecSch->type 		= pRecordSchReq->type;
	switch(pRecordSchReq->type)
	{
		case 0:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_SCHEDULE_RECTIMING;
		} break;
		case 1:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_SCHEDULE_RECMOTION;
		} break;
		case 2:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_SCHEDULE_RECSENSOR;
		} break;
		default:
			return;
	}

	pRecSch->copy2Chnmask 	= 0;
	pRecSch->copy2Weekday	= 0;
	
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		uint sch[7] = {0};
		u8 week = (pRecordSchReq->weekday+1)%DAYS_OF_WEEK;
		
		Excmd_GetSch(&sCfgIns, sch);
		
		for(i=0; i<sizeof(pRecordSchReq->TimeFiled)/sizeof(pRecordSchReq->TimeFiled[0]); i++)
		{
			if(sch[week] & (0x01<<i))
			{
				pRecSch->TimeFiled[i].starttime = 1;
			}
			else
			{
				pRecSch->TimeFiled[i].starttime = 0;
			}
			////printf("######sch %d %d \n", i, pRecSch->TimeFiled[i].starttime);
			
			pRecSch->TimeFiled[i].starttime = htonl(pRecSch->TimeFiled[i].starttime);
		}
	}
}


void DealRemoteCmd_SetRecSch(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int i, j;
	ifly_RecSchTime_t* pRecordSchReq = &pCmd->sReq.RecSchTime;
	
	SBizSchedule sCfgIns;
	SBizParaTarget sParaTgtIns;
	
	sParaTgtIns.nChn 			= pRecordSchReq->chn;
	switch(pRecordSchReq->type)
	{
		case 0:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_SCHEDULE_RECTIMING;
		} break;
		case 1:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_SCHEDULE_RECMOTION;
		} break;
		case 2:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_SCHEDULE_RECSENSOR;
		} break;
		default:
			return;
	}
	
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		uint sch = 0;
		for(i=0; i<sizeof(pRecordSchReq->TimeFiled)/sizeof(pRecordSchReq->TimeFiled[0]); i++)
		{
			if( pRecordSchReq->TimeFiled[i].starttime )
				sch |= (1<<i);
			else
				sch &= ~(1<<i);

			////printf("sch %d %d \n", i, pRecordSchReq->TimeFiled[i].starttime);			
		}
		
		u8 week = (pRecordSchReq->weekday+1)%DAYS_OF_WEEK;
		
		SBizSchTime* pSTIns;
		
		pSTIns = sCfgIns.sSchTime[week];
		memset(pSTIns, 0, sizeof(SBizSchTime)*MAX_SCHEDULE_SEGMENTS); // 清空一天内所有时间段记录
		Excmd_CopyOneDaySch(sch, pSTIns);

		// 每天
		//printf("sch copy2Weekday %d \n", pRecordSchReq->copy2Weekday);
		for(i=0; i<DAYS_OF_WEEK; i++)
		{
			if ((1 << i) & pRecordSchReq->copy2Weekday)
			{
				u8 nTmpWeek = (i+1)%DAYS_OF_WEEK;
				memcpy(sCfgIns.sSchTime[nTmpWeek], pSTIns, sizeof(SBizSchTime)*MAX_SCHEDULE_SEGMENTS);
			}
		}
		
		BizSetPara(&sParaTgtIns, &sCfgIns);
	}
	
	for (i = 0; i < GetVideoMainNum(); i++)
	{
		if ((1 << i ) & pRecordSchReq->copy2Chnmask)
		{
			SBizSchedule sCfgIns1;

			sParaTgtIns.nChn = i;
			if( BizGetPara(&sParaTgtIns, &sCfgIns1) == 0 )
			{
				memcpy(sCfgIns1.sSchTime, sCfgIns.sSchTime, sizeof(sCfgIns.sSchTime));
			}
			BizSetPara(&sParaTgtIns, &sCfgIns);
		}
	}
}


#if 1
/*
 网络命令处理回调函数指针
*/
SCmdCBPair pNetCmdCB[] = {
	{CTRL_CMD_LOGIN, DealRemoteCmd_Login},
	{CTRL_CMD_LOGOFF, DealRemoteCmd_LoginOff},
	{CTRL_CMD_GETDEVICEINFO, DealRemoteCmd_GetDeviceInfo},
	{CTRL_CMD_GETVIDEOPROPERTY, DealRemoteCmd_GetVideoPropety},
	{CTRL_CMD_GETAUDIOPROPERTY, DealRemoteCmd_GetAudioPropety},
	{CTRL_CMD_GETVOIPPROPERTY,DealRemoteCmd_Getvoipproperty},
	{CTRL_CMD_GETMDPROPERTY,DealRemoteCmd_Getmdproperty},

	{CTRL_CMD_STOPVIDEOMONITOR,DealRemoteCmd_Stopvideomonitor},
	{CTRL_CMD_STOPAUDIOMONITOR,DealRemoteCmd_Stopaudiomonitor},
	{CTRL_CMD_STOPVOIP,DealRemoteCmd_Stopvoip},
	{CTRL_CMD_STOPDOWNLOAD,DealRemoteCmd_Stopdownload},
	{CTRL_CMD_STOPFILEPLAY,DealRemoteCmd_Stopfileplay},
	{CTRL_CMD_STOPTIMEPLAY,DealRemoteCmd_Stoptimeplay},
	{CTRL_CMD_FASTPLAY,DealRemoteCmd_Fastplay},
	{CTRL_CMD_SLOWPLAY,DealRemoteCmd_Slowplay},
	{CTRL_CMD_SETPLAYRATE,DealRemoteCmd_Setplayrate},
	{CTRL_CMD_PAUSEPLAY,DealRemoteCmd_Pauseplay},
	{CTRL_CMD_RESUMEPLAY,DealRemoteCmd_Resumeplay},
	{CTRL_CMD_SINGLEPLAY,DealRemoteCmd_Singleplay},
	{CTRL_CMD_FASTBACKPLAY,DealRemoteCmd_Fastbackplay},
	{CTRL_CMD_PLAYPREV,DealRemoteCmd_Playprev},
	{CTRL_CMD_PLAYNEXT,DealRemoteCmd_Playnext},
	{CTRL_CMD_PLAYSEEK,DealRemoteCmd_Playseek},
	{CTRL_CMD_PLAYMUTE,DealRemoteCmd_Playmute},
	{CTRL_CMD_PLAYPROGRESS,DealRemoteCmd_Playprogress},
	{CTRL_CMD_GETNETWORK,DealRemoteCmd_Getnetwork},
	{CTRL_CMD_SETNETWORK,DealRemoteCmd_Setnetwork},
	
	{CTRL_CMD_GETVGASOLLIST,DealRemoteCmd_Getvgasollist},
	{CTRL_CMD_GETDDNSLIST,DealRemoteCmd_Getddnslist},//csp modify 20130422
	
	{CTRL_CMD_GETSYSPARAM,DealRemoteCmd_Getsysparam},
	{CTRL_CMD_SETSYSPARAM,DealRemoteCmd_Setsysparam},
	{CTRL_CMD_GETRECPARAM,DealRemoteCmd_Getrecparam},
	{CTRL_CMD_SETRECPARAM,DealRemoteCmd_Setrecparam},

	{CTRL_CMD_GETIMGPARAM,DealRemoteCmd_Getimgparam},
	{CTRL_CMD_SETIMGPARAM,DealRemoteCmd_Setimgparam},
	{CTRL_CMD_GETPICADJ,DealRemoteCmd_Getpicadj},
	{CTRL_CMD_SETPICADJ,DealRemoteCmd_Setpicadj},
	{CTRL_CMD_GETALARMINPARAM,DealRemoteCmd_Getalarminparam},
	{CTRL_CMD_SETALARMINPARAM,DealRemoteCmd_Setalarminparam},
	{CTRL_CMD_GETALARMOUTPARAM,DealRemoteCmd_Getalarmoutparam},
	{CTRL_CMD_SETALARMOUTPARAM,DealRemoteCmd_Setalarmoutparam},
	{CTRL_CMD_GETALARMNOTICYPARAM,DealRemoteCmd_Getalarmnoticyparam},
	{CTRL_CMD_SETALARMNOTICYPARAM,DealRemoteCmd_Setalarmnoticyparam},
	{CTRL_CMD_GETRECSCHPARAM,DealRemoteCmd_Getrecschparam},
	{CTRL_CMD_SETRECSCHPARAM,DealRemoteCmd_Setrecschparam},
	{CTRL_CMD_GETMDPARAM,DealRemoteCmd_Getmdparam},
	{CTRL_CMD_SETMDPARAM,DealRemoteCmd_Setmdparam},
	{CTRL_CMD_GETVIDEOLOSTPARAM,DealRemoteCmd_Getvideolostparam},
	{CTRL_CMD_SETVIDEOLOSTPARAM,DealRemoteCmd_Setvideolostparam},
	{CTRL_CMD_GETVIDEOBLOCKPARAM,DealRemoteCmd_Getvideoblockparam},
	{CTRL_CMD_SETVIDEOBLOCKPARAM,DealRemoteCmd_Setvideoblockparam},
	{CTRL_CMD_GETPTZPARAM,DealRemoteCmd_Getptzparam},
	{CTRL_CMD_SETPTZPARAM,DealRemoteCmd_Setptzparam},
	{CTRL_CMD_SETPRESET,DealRemoteCmd_Setpreset},
	{CTRL_CMD_GETCRUISEPARAM,DealRemoteCmd_Getcruiseparam},
	{CTRL_CMD_SETCRUISEPARAM,DealRemoteCmd_Setcruiseparam},
	{CTRL_CMD_CTRLCRUISEPATH,DealRemoteCmd_Ctrlcruisepath},
	{CTRL_CMD_CTRLPTZTRACK,DealRemoteCmd_Ctrlptztrack},
	{CTRL_CMD_GETHDDINFO,DealRemoteCmd_Gethddinfo},
	{CTRL_CMD_GETUSERINFO,DealRemoteCmd_Getuserinfo},
	{CTRL_CMD_SETUSERINFO,DealRemoteCmd_Setuserinfo},
	{CTRL_CMD_SETRESTORE,DealRemoteCmd_Setrestore},
	{CTRL_CMD_CLEARALARM,DealRemoteCmd_Clearalarm},
	{CTRL_CMD_GETSYSTIME,DealRemoteCmd_Getsystime},
	{CTRL_CMD_SETSYSTIME,DealRemoteCmd_Setsystime},
	{CTRL_CMD_GETSYSINFO,DealRemoteCmd_Getsysinfo},
	{CTRL_CMD_SHUTDOWN,DealRemoteCmd_Shutdown},
	{CTRL_CMD_REBOOT,DealRemoteCmd_Reboot},
	{CTRL_CMD_PTZCTRL,DealRemoteCmd_Ptzctrl},
	{CTRL_CMD_GETMANUALREC,DealRemoteCmd_Getmanualrec},
	{CTRL_CMD_SETMANUALREC,DealRemoteCmd_Setmanualrec},
	{CTRL_CMD_LOGSEARCH,DealRemoteCmd_Logsearch},
	{CTRL_CMD_RECFILESEARCH,DealRemoteCmd_Recfilesearch},
	{CTRL_CMD_GETSPSPPS,DealRemoteCmd_Getspspps},
	{CTRL_CMD_ALARMUPLOADCENTER,DealRemoteCmd_Alarmuploadcenter},
#ifdef _TOUMING_COM_
	{CTRL_CMD_SERIALSTOP,DealRemoteCmd_Serialstop},
	{CTRL_CMD_COM_PROTOCOL_GET,DealRemoteCmd_Com_Protocol_Get},
	{CTRL_CMD_COM_PROTOCOL_SET,DealRemoteCmd_Com_Protocol_Set},
#endif
	{CTRL_CMD_GETSYSLANGLIST,DealRemoteCmd_Getsyslanglist},
	{CTRL_CMD_GETBITRATELIST,DealRemoteCmd_Getbitratelist},
	{CTRL_CMD_GETEMAILSMTP,DealRemoteCmd_Getemailsmtp},
	{CTRL_CMD_SETEMAILSMTP,DealRemoteCmd_Setemailsmtp},

	{CTRL_CMD_USEUTCTIME,DealRemoteCmd_Useutctime},

#ifdef IECTRL_NEW
	{CTRL_CMD_RESETPICADJUST,DealRemoteCmd_Resetpicadjust},
	{CTRL_CMD_GETFRAMERATELIST,DealRemoteCmd_Getframeratelist},
	{CTRL_CMD_GETMAX_IMGMASKNUM,DealRemoteCmd_Getmax_Imgmasknum},
	{CTRL_GET_RESOLUTION_LIST,DealRemoteCmd_Get_Resolution_List},
#endif
	{CTRL_CMD_PPPOE_PREUP,DealRemoteCmd_Pppoe_Preup},
	{CTRL_CMD_PPPOE_UP,DealRemoteCmd_Pppoe_Up},
	{CTRL_CMD_PPPOE_DOWN,DealRemoteCmd_Pppoe_Down},
	{CTRL_CMD_PPPOE_DISCONNECT,DealRemoteCmd_Pppoe_Disconnect},

	{CTRL_CMD_DHCP_DECONFIG,DealRemoteCmd_Dhcp_Deconfig},
	{CTRL_CMD_DHCP_BOUND,DealRemoteCmd_Dhcp_Bound},
	{CTRL_CMD_DHCP_RENEW,DealRemoteCmd_Dhcp_Renew},
	{CTRL_CMD_DHCP_NAK,DealRemoteCmd_Dhcp_Nak},

	{CTRL_CMD_GETSUBSTREAMPARAM, DealRemoteCmd_Getsubstreamparam},
	{CTRL_CMD_SETSUBSTREAMPARAM, DealRemoteCmd_Setsubstreamparam},
	{CTRL_CMD_GET_SPECIALDEVICEINFO, DealRemoteCmd_Getspecdevinfo},
	{CTRL_CMD_GETMDSENSELIST, DealRemoteCmd_GetMdSenseList},
	{CTRL_CMD_GETMDALARMDELAYLIST, DealRemoteCmd_GetMdAlarmDelayList},
	{CTRL_CMD_GETBAUDRATELIST, DealRemoteCmd_GetBauteRateList},
	{CTRL_CMD_GETPTZPROTOCOLLIST, DealRemoteCmd_GetPtzProtocolList},
	{CTRL_CMD_GETRECSCHPARAMBYTYPE, DealRemoteCmd_GetRecSch},
	{CTRL_CMD_SETRECSCHPARAMBYTYPE, DealRemoteCmd_SetRecSch},
	
	//csp modify 20130423
	{CTRL_CMD_SETMONITORINFO, DealRemoteCmd_SetMonitorInfo},
	
	//csp modify 20130519
	{CTRL_CMD_GETADVPRIVILEGE,DealRemoteCmd_GetAdvPrivilege},
	//xdc
	{CTRL_CMD_GETSEACHIPCLIST,DealRemoteCmd_Ipcsearch},
	{CTRL_CMD_GETADDIPCLIST,DealRemoteCmd_GetAddIpcList},
	{CTRL_CMD_SETIPC,DealRemoteCmd_SetIpc},
	{CTRL_CMD_ADDIPC,DealRemoteCmd_AddIpc},
	{CTRL_CMD_DELETEIPC,DealRemoteCmd_DeleteIpc},
	{CTRL_CMD_GETTHEOTHER,DealRemoteCmd_Ipcsearch},
	//yaogang modify 20141030
	{CTRL_CMD_GET_PATROL_PARA, DealRemoteCmd_GetPatrolPara},
	{CTRL_CMD_SET_PATROL_PARA, DealRemoteCmd_SetPatrolPara},
	{CTRL_CMD_GET_PREVIEW_PARA, DealRemoteCmd_GetPreviewPara},
	{CTRL_CMD_SET_PREVIEW_PARA, DealRemoteCmd_SetPreviewPara},
	{CTRL_CMD_CLEAN_ALARM_ICON, DealRemoteCmd_CleanDesktopAlarmIcon},
	{CTRL_CMD_CLOSE_GUIDE, DealRemoteCmd_CloseGuide},
	{CTRL_CMD_GET_IPCCHN_LINKSTATUS, DealRemoteCmd_GetIPCChnLinkStatus},
	//yaogang modify 20170715 简易设置通道名的接口
	{CTRL_CMD_GET_CHN_NAME, DealRemoteCmd_GetChnName},
	{CTRL_CMD_SET_CHN_NAME, DealRemoteCmd_SetChnName},	
};
#endif

void bizNet_ExcmdRegCB()
{
	int i;
	for(i=0; i<sizeof(pNetCmdCB)/sizeof(pNetCmdCB[0]); i++)
	{
		if( pNetCmdCB[i].pCB )
			NetCommRegCmdCB( pNetCmdCB[i].nCmdID, pNetCmdCB[i].pCB );
	}
	
	NetCommUnregCmdCB(CTRL_CMD_RECFILESEARCH);
	NetCommRegCmdCB(CTRL_CMD_RECFILESEARCH, DoCTRL_CMD_RECFILESEARCH1);
	NetCommUnregCmdCB(CTRL_CMD_GETSYSTIME);
	NetCommRegCmdCB(CTRL_CMD_GETSYSTIME, DoCTRL_CMD_GETSYSTIME);
	NetCommUnregCmdCB(CTRL_CMD_SETSYSTIME);
	NetCommRegCmdCB(CTRL_CMD_SETSYSTIME, DoCTRL_CMD_SETSYSTIME);
}

int GetChnNameMax( void )
{
	int vLen;
	int ColNum = 0;
	
	switch(GetVideoMainNum())
	{
        case 4:
            ColNum = 2;
            break;
        case 8:
            ColNum = 3;
            break;
        case 16:
            ColNum = 4;
            break;
        case 24:
			ColNum = 5;
            break;
		case 32:
			ColNum = 6;
            break;
        default:
            ColNum = 2;
            break;
    }
	
	#define TEXT_WIDTH 24
	vLen = 720/ColNum/TEXT_WIDTH*3-6;
	vLen = (vLen > 27)?27:vLen;
	
	//printf("GetChnNameMax vLen=%d ColNum=%d\n",vLen,ColNum);
	
	return vLen;
}

