// include
#include <pthread.h>
#include <sys/reboot.h>
#include "mod_syscomplex.h"
#include "update.h"
#include "search.h"
#include "Backup.h"
#include "format.h"
#include "syscomplxtime.h"

//** macro

//csp modify
//#define RECV_CMD_ERR_DELAY_US	(1000*1000)
#define RECV_CMD_ERR_DELAY_US		(10*1000)

static s32 g_sysCmplxError = 0;

//csp modify
static u8 g_modeworking = 0;

extern unsigned int nMaxSupportDiskNum;

//** typedef

typedef struct _sBackup
{
	EMFILETYPE 	  emFileType;
	SBackTgtAtt   sBackTgtAtt;
	SSearchResult sSearchResult;
} SBackup, *PSBackup;

typedef enum
{
	EM_MODSYS_CMD_INIT = 0,
	EM_MODSYS_CMD_DEINIT,
	EM_MODSYS_CMD_UPGRADE_USB,
	EM_MODSYS_CMD_UPGRADE_FTP,
	EM_MODSYS_CMD_BACKUP,
	EM_MODSYS_CMD_RECSEARCH,
	EM_MODSYS_CMD_DMHdrGet,
	EM_MODSYS_CMD_DMGetInfo,
	EM_MODSYS_CMD_DMFormat,
	EM_MODSYS_CMD_InfoSet,
	EM_MODSYS_CMD_InfoGet,
	EM_MODSYS_CMD_DTSet,
	EM_MODSYS_CMD_DTGet,
	EM_MODSYS_CMD_DTGetString,
	EM_MODSYS_CMD_Exit
} EM_MODSYSCPLX_CMD;

typedef struct _sUpdate
{
	EMUPGRADETARGET target;
	EMUPGRADEWAY 	way;
	s8 path[MODSYS_PATH_LEN_MAX]; // upgrade
} SUpdate, *PSupdate;

typedef struct _sSysExit
{
	EMSYSEXITTYPE emSysExitType;
	u8 nDelay;
} SSysExit, *PSysExit;


// cmd data struct
typedef struct _sCmdData
{
	EM_MODSYSCPLX_CMD cmd;

	union
	{
		// upgrade
		SUpdate sUpdate;

		// search
		SSearch	sSearch;
		// backup
		SBackup sBackup;
		// format
		SFormat sFormat;
		// exit
		SSysExit sExit;

		// datatime info
		SDateTime* psDateTime;
		// strorage dev
		SStoreDevMgr sStDevMgr;
	};
} SCmdData, *PSCmdSta;


//** local variables
extern SModSyscplx sModSysIns;


//** global variables


//** local functions

static s32 modSysCmplx_SysExit( SSysExit* psExit );
static s32 modSysComplex_CheckPowerOff();

//** global functions

extern void modsys_SetDskAct( u8 op, BOOL state );
extern s32 modSysCmplx_GetTime(SDateTime* psDateTime);
extern s32 modSysCmplx_Backup_USB( u32 nFileNum, SRecfileInfo* pRecInfoList );
extern s32 modsys_DskInfoUpdateInit();

PFNSysResetCB GetRecStopCB()//cw_9508S
{
	return sModSysIns.pStopRecCB;
}

s32 modSysCmplx_DmInfoNotify(
	SDMInfo *psInfo,
	s32 	evt,
	s32 	emRslt
)
{
	PFNSysNotifyCB 	pCb;
	SEventPara		sEvPara;
	
	if(NULL != psInfo)
	{
		memcpy(&sEvPara.sDmInfo, psInfo, sizeof(sEvPara.sDmInfo));
	}
	else
	{
		sEvPara.emResult = emRslt;
	}
	pCb = sModSysIns.pFNSysNotifyCB;
	EvNotify(pCb, evt, &sEvPara);
	
	return 0;	
}

s32 modSysCmplx_StorgeNotify(
	SStoreDevManage *psMgr,
	s32 	evt,
	s32 	emRslt
)
{
	PFNSysNotifyCB 	pCb;
	SEventPara		sEvPara;
	
	if(NULL != psMgr)
	{
		memcpy(&sEvPara.sStoreMgr, psMgr, sizeof(sEvPara.sStoreMgr));
	}
	else
	{
		sEvPara.emResult = emRslt;
	}
	pCb = sModSysIns.pFNSysNotifyCB;
	EvNotify(pCb, evt, &sEvPara);
	
	return 0;	
}

s32 modSysCmplx_UpdateNotify(
	SProgress *sProg,
	s32 	evt,
	s32 	emRslt )
{
	PFNSysNotifyCB 	pCb;
	SEventPara		sEvPara;
	
	if(NULL!=sProg)
	{
		//printf("+++++++++++++ %d\n", sProg->nProgress);
		if(sProg->lTotalSize>(u64)0)
		{
			//printf("total %d cur %d\n", sProg->lTotalSize, sProg->lCurSize);
			sProg->nProgress = (sProg->lCurSize*(u64)100/sProg->lTotalSize);
			//printf("---------------------prog %d\n", sProg->nProgress);
		}
		
		memcpy( &sEvPara.sProgress, sProg, sizeof(sEvPara.sProgress) );
	}
	else
	{
		sEvPara.emResult = emRslt;
	}
	
	pCb = sModSysIns.pFNSysNotifyCB;
	EvNotify(pCb, evt, &sEvPara); // init success
	
	// deal with remote update progress 
	if(evt == EM_SYSEVENT_UPGRADE_RUN || evt == EM_SYSEVENT_FORMAT_RUN)
	{
		if(sModSysIns.pRemoteUpdateNotifyCB)
		{
			sModSysIns.pRemoteUpdateNotifyCB(sProg->nProgress);
		}
	}
	
	return 0;
}

/*
1.1 s32 ModSysComplexInit(PFNSysNotifyCB pSysNotifyCB)

	函数功能:模块初始化
	输入参数：
		pSysNotifyCB:
			类型:PFNSysNotifyCB
			含义:系统杂项事件通知回调函数
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/

static s32 modSysCmplx_Upgrade_Do( EMUPGRADETARGET emUpdateTarget, s8* path )
{
	s32 errCode;
	SProgress sProg;
	
	//MODSYS_ASSERT(path);
	
	// start
	sProg.lTotalSize	= 0llu;
	sProg.lCurSize 		= 0llu;
	sProg.nProgress		= 0;
	
	modSysCmplx_UpdateNotify(&sProg, EM_SYSEVENT_UPGRADE_INIT, EM_UPGRADE_SUCCESS);
	
	// init
	errCode = modSysCmplx_Update_USB_Do(emUpdateTarget, path, NULL);

	//printf("%s over, errCode: %d\n", __func__, errCode);
	//fflush(stdout);
	
	MODSYS_DEBUG(errCode);
	
	return errCode;
}

static void* modSysCmplx_ThreadFunc(void* param)
{
	printf("$$$$$$$$$$$$$$$$$$modSysCmplx_ThreadFunc id:%d\n",getpid());
	
	SCmdData data;
	s32 rtn = -1, errCode;
	SMsgCmdHdr pCmd;
	SModSyscplx* psModSysCplx;
	
	psModSysCplx = &sModSysIns;
	
	while(1)
	{
		memset(&data, 0, sizeof(data));
		
		pCmd = psModSysCplx->pMsgCmd;
		rtn = ReadMsgCmd(pCmd, &data);
		if(0!=rtn)
		{
			//MODSYS_DEBUG(EM_MODSYS_THREAD_READ_CMD_ERR);
			
			usleep(RECV_CMD_ERR_DELAY_US); // cmd err, sleep 1 seconds to continue
			
			continue;
		}
		else
		{
			MODSYS_DEBUG_STR("\n Recv this cmd, check the ID:");
			MODSYS_DEBUG(data.cmd);
		}
		
		// select (&cmd)
		switch(data.cmd)
		{
			case EM_MODSYS_CMD_UPGRADE_FTP:
				//sModSysIns.pFNSysNotifyCB(EM_SYSEVENT_REMOTEUP_START,-1);
			case EM_MODSYS_CMD_UPGRADE_USB:
			{
				PSupdate 	pUpdt 	= &data.sUpdate;
				EMUPGRADETARGET upTgt = pUpdt->target;
				
				MODSYS_DEBUG(data.cmd);
				
				//if(data.cmd==EM_MODSYS_CMD_UPGRADE_FTP)
				{
					u32 len = strlen(pUpdt->path);
					
					if( len>0 && len<MODSYS_PATH_LEN_MAX )
					{
						s8* pPath = psModSysCplx->path;
						memset(pPath, 0, sizeof(psModSysCplx->path));
						strcpy(pPath, pUpdt->path);
						
						printf("~~~~~~~path:%s\n", pUpdt->path);
						rtn = modSysCmplx_Upgrade_Do(upTgt, pPath);
						
						//printf("%s over, rtn: %d\n", __func__, rtn);
						//fflush(stdout);
						
						MODSYS_DEBUG(rtn);
					}
					else
					{
						rtn = -1;
						errCode = EM_MODSYS_THREAD_FILE_PATH_ERR;// file path invalid
						MODSYS_DEBUG(EM_MODSYS_THREAD_FILE_PATH_ERR);
					}
				}
				//else
				//{
				//	rtn = modSysCmplx_Upgrade_Do(upTgt, NULL);
				//	MODSYS_DEBUG(rtn);
				//}
				
				g_sysCmplxError = rtn;
				
				rtn = AckMsgCmd(pCmd);
				if(rtn)
				{
					MODSYS_DEBUG_STR("ack fail!\n");
					MODSYS_DEBUG(rtn);
				}
			} break;
			case EM_MODSYS_CMD_RECSEARCH:
			{
				PSSearch psSearch;

				MODSYS_DEBUG(EM_MODSYS_CMD_RECSEARCH);

				psSearch = (PSSearch)&data.sSearch;

				// parse cmd and get search para
				if( 0==modSysComplex_Search_do( psSearch ) )
				{
					MODSYS_DEBUG_STR("search do success!\n");
				}
				else
				{
					MODSYS_DEBUG_STR("search do failed!\n");
				}

				// ack cmd
				rtn = AckMsgCmd(pCmd);
				if(rtn)
				{
					MODSYS_DEBUG_STR("ack fail!\n");
					MODSYS_DEBUG(rtn);
				}

				MODSYS_DEBUG_STR("EM_MODSYS_CMD_RECSEARCH ack!\n");

				break;
			}
			case EM_MODSYS_CMD_BACKUP:
			{
				MODSYS_DEBUG(EM_MODSYS_CMD_BACKUP);

				PSBackup psBackup;

				psBackup = (PSBackup)&data.sBackup;

				if(psBackup->sSearchResult.psRecfileInfo)
				{
					modsysCmplx_ClrBreakBackup();
					
					g_sysCmplxError = 0;//zlbfix20110715 backup
					
					rtn = AckMsgCmd(pCmd);
					if(rtn)
					{
						MODSYS_DEBUG_STR("format ack fail!\n");
						MODSYS_DEBUG(rtn);
					}
					
					//if(filetype==SNAP)
					// do
					// else
					// do
					if(psBackup->sBackTgtAtt.devt==EM_BACKUP_USB)
					{
						MODSYS_DEBUG(psBackup->sBackTgtAtt.fmt);

						switch(psBackup->sBackTgtAtt.fmt)
						{
							case EM_FORMAT_ORIGINAL:
							{
								MODSYS_DEBUG_STR("EM_FORMAT_ORIGINAL!\n\n");
								MODSYS_DEBUG(psBackup->sSearchResult.nFileNum);
								//printf("%s yg 1\n", __func__);
								rtn = modSysCmplx_Backup_USB(
										psBackup->sSearchResult.nFileNum,
										psBackup->sSearchResult.psRecfileInfo );
								if(rtn==0)
								{
									MODSYS_DEBUG_STR("EM_FORMAT_ORIGINAL backup finish!\n\n");
								}
								else
								{
									// not enter this route
									MODSYS_DEBUG_STR("EM_FORMAT_ORIGINAL backup failed!\n\n");
								}

								break;
							}
							case EM_FORMAT_AVI:
							{
								MODSYS_DEBUG_STR("EM_FORMAT_AVI!\n\n");
								MODSYS_DEBUG(psBackup->sSearchResult.nFileNum);

								rtn = modSysCmplx_Backup_USB_Avi(
										psBackup->sSearchResult.nFileNum,
										psBackup->sSearchResult.psRecfileInfo );
								if(rtn==0)
								{
									MODSYS_DEBUG_STR("EM_FORMAT_AVI backup finish!\n\n");
								}
								else
								{
									// not enter this route
									MODSYS_DEBUG_STR("EM_FORMAT_AVI backup failed!\n\n");
								}

								break;
							}
							case EM_FORMAT_JPEG: //yaogang modify 20150123 for snap backup
							{
								MODSYS_DEBUG_STR("EM_FORMAT_JPEG!\n\n");
								MODSYS_DEBUG(psBackup->sSearchResult.nFileNum);
								printf("%s EM_FORMAT_JPEG\n", __func__);
								rtn = modSysCmplx_Backup_USB_JPEG(
										psBackup->sSearchResult.nFileNum,
										psBackup->sSearchResult.psRecSnapInfo);
								if(rtn==0)
								{
									MODSYS_DEBUG_STR("EM_FORMAT_ORIGINAL backup finish!\n\n");
								}
								else
								{
									// not enter this route
									MODSYS_DEBUG_STR("EM_FORMAT_ORIGINAL backup failed!\n\n");
								}
							}break;
							default:
							{
								MODSYS_DEBUG_STR("not supported backup type!!!\n");

								break;
							}
						}
					}
					else
					{
						MODSYS_DEBUG_STR("DVDR EM_FORMAT_ORIGINAL!\n\n");
						MODSYS_DEBUG(psBackup->sSearchResult.nFileNum);

						rtn = modSysCmplx_Backup_DVDR(
										psBackup->sSearchResult.nFileNum,
										psBackup->sSearchResult.psRecfileInfo,
										psBackup->sBackTgtAtt.mode );
						if(rtn==0)
						{
							MODSYS_DEBUG_STR("DVDR EM_FORMAT_ORIGINAL backup finish!\n\n");
						}
						else
						{
							// not enter this route
							MODSYS_DEBUG_STR("DVDR EM_FORMAT_ORIGINAL backup failed!\n\n");
						}

						//break;
					}
					
					MODSYS_DEBUG_STR("backup done!!!\n");
				}
				else
				{
					MODSYS_DEBUG_STR("DVDR no valid files to backup!\n\n");
				}
				
				modsys_SetDskAct( 3, FALSE );
				
				break;
			}
			case EM_MODSYS_CMD_DMFormat:
			{
				MODSYS_DEBUG(EM_MODSYS_CMD_DMFormat);
				
				SFormat* psFormat;
				
				psFormat = (PSFormat)&data.sFormat;
				
				// format
				// do
				//
				rtn = modSysCmplx_DMFormat(psFormat->strDevPath, psFormat->emFormatWay);
				if(rtn)
				{
					MODSYS_DEBUG_STR("ModSysComplexDMFormat fail!1\n");
				}
				else
				{
					MODSYS_DEBUG_STR("ModSysComplexDMFormat success!1\n");
				}
				
				g_sysCmplxError = rtn;
				
				rtn = AckMsgCmd(pCmd);
				if(rtn)
				{
					MODSYS_DEBUG_STR("format ack fail!\n");
					MODSYS_DEBUG(rtn);
				}
				
				break;
			}
			case EM_MODSYS_CMD_Exit:
			{
				MODSYS_DEBUG(EM_MODSYS_CMD_Exit);
				
				SSysExit sSysExitIns;
				
				memcpy( &sSysExitIns, &data.sExit, sizeof(SSysExit) );
				
				rtn = AckMsgCmd(pCmd);
				if(rtn)
				{
					MODSYS_DEBUG_STR("EM_MODSYS_CMD_Exit ack fail!\n");
					MODSYS_DEBUG(rtn);
				}
				rtn = modSysCmplx_SysExit(&sSysExitIns);
				if(rtn)
				{
					MODSYS_DEBUG_STR("EM_MODSYS_CMD_Exit fail!1\n");
				}
				else
				{
					MODSYS_DEBUG_STR("EM_MODSYS_CMD_Exit success!1\n");
				}
				
				break;
			}
			case EM_MODSYS_CMD_DTSet:
			{
				MODSYS_DEBUG(EM_MODSYS_CMD_DTSet);
				
				SDateTime* psDateTime;
				psDateTime = (SDateTime*)data.psDateTime;
				if(NULL!=psDateTime)
				{
					rtn = modSysCmplx_SetTime(psDateTime);
					if(rtn)
					{
						MODSYS_DEBUG_STR("EM_MODSYS_CMD_DTSet fail!1\n");
					}
					else
					{
						MODSYS_DEBUG_STR("EM_MODSYS_CMD_DTSet success!1\n");
					}
				}
				else
				{
					MODSYS_DEBUG_STR("NULL datetime str!!!\n");
				}
				
				rtn = AckMsgCmd(pCmd);
				if(rtn)
				{
					MODSYS_DEBUG_STR("EM_MODSYS_CMD_DTSet ack fail!\n");
					MODSYS_DEBUG(rtn);
				}
				
				break;
			}
			case EM_MODSYS_CMD_DMGetInfo:
			{
				MODSYS_DEBUG(EM_MODSYS_CMD_DMGetInfo);
				//printf("yg modSysCmplx_ThreadFunc-EM_MODSYS_CMD_DMGetInfo\n");
				SStoreDevMgr* psStDevMgrIns;
				psStDevMgrIns = &data.sStDevMgr;
				if(NULL!=psStDevMgrIns->pDevMgr)
				{
					rtn = modSysCmplx_GetDMInfo(psStDevMgrIns);
					if(rtn)
					{
						MODSYS_DEBUG_STR("EM_MODSYS_CMD_DMGetInfo fail!1\n");
					}
					else
					{
						MODSYS_DEBUG_STR("EM_MODSYS_CMD_DMGetInfo success!1\n");
					}
				}
				else
				{
					MODSYS_DEBUG_STR("NULL storage dev str!!!\n");
				}
				
				rtn = AckMsgCmd(pCmd);
				if(rtn)
				{
					MODSYS_DEBUG_STR("EM_MODSYS_CMD_DMGetInfo ack fail!\n");
					MODSYS_DEBUG(rtn);
				}
				
				break;
			}
			default:
			{
				rtn = AckMsgCmd(pCmd);
				if(rtn)
				{
					MODSYS_DEBUG_STR("EM_MODSYS_CMD_DMGetInfo ack fail!\n");
					MODSYS_DEBUG(rtn);
				}
				
				MODSYS_DEBUG_STR("\nget a invalid cmd request!\n");
				
				//AckMsgCmd(pCmd);
				
				break;
			}
		}
		
		MODSYS_DEBUG(rtn);
	}
	
	return 0;
}

#define UPTIME_INTVL	(20) //(300)太大会导致编码时间戳不均匀
extern void tl_venc_update_basetime(void);

static void* modSysCmplx_UpTimeThreadFunc(void* param)
{
	printf("$$$$$$$$$$$$$$$$$$modSysCmplx_UpTimeThreadFunc id:%d\n",getpid());
	
	SModSyscplx* psModSysCplx;
	struct timeval val;
	char szDateTime[128], *pstrDateTime = NULL;
	SDateTime sDTIns;
	//int cnr = 0;
	int rtn = 0;
	
	SEventPara		sEvPara;
	
	psModSysCplx = &sModSysIns;
	
	PFNSysNotifyCB pCB = sModSysIns.pFNSysNotifyCB;
	
	u8 nPastSecond = 255;
	
	//printf("Enter modSysUpTimeThxd OK!\n");
	//printf("now:%d\n", time(0));
	tl_venc_update_basetime();
	
	usleep(2*1000*1000);
	
	while(1)
	{
		//csp modify
		if(!g_modeworking)
		{
			usleep(10*1000);
			continue;
		}
		
		//wait by time
		if(!psModSysCplx->bSetTime)
		{
			val.tv_sec  = 0;//UPTIME_INTVL;
			val.tv_usec = UPTIME_INTVL*1000;
			select(0,NULL,NULL,NULL,&val);
		}
		
		psModSysCplx->bSetTime = FALSE;
		
		//printf("update time !!!\n");
		
		//uptime
		//if(psModSysCplx->pUpdateTimeCB)
		if(pCB)
		{
			//printf("update time ...\n");
			memset(szDateTime, 0, sizeof(szDateTime));
			//ModSysComplexDTGetString(szDateTime);
			
			//csp modify 20131213
			//ModSysComplexDTGet(&sDTIns);
			ModSysComplexDTGet_TZ(&sDTIns);
			
			if(sDTIns.nSecode == nPastSecond)//yzw 时间完全相同，没必要更新
			{
				continue;
			}
			
			nPastSecond = sDTIns.nSecode;
			
			if((0 == (sDTIns.nMinute % 10))
				&& (0 == sDTIns.nSecode))//yzw 长时间观察发现编码时间戳会偏慢，1小时慢100毫秒，故10分钟更新一次编码时间戳
			{
				tl_venc_update_basetime();
			}
			
			pstrDateTime = szDateTime;
			switch(sDTIns.emDateTimeFormat)
			{
				case EM_DATETIME_YYYYMMDDHHNNSS:
				{
					if(!sDTIns.emTimeFormat)
					{
						sprintf(pstrDateTime, 
							"%04d-%02d-%02d %02d:%02d:%02d",						
							sDTIns.nYear,
							sDTIns.nMonth,
							sDTIns.nDay,
							sDTIns.nHour,
							sDTIns.nMinute,
							sDTIns.nSecode
						);
					}
					else
					{
						char ampm[4] = {0};
						
						if(!sDTIns.nHour)
						{
							sDTIns.nHour = 12;
							strcpy(ampm, "AM");
						}
						else if(sDTIns.nHour < 12)
						{
							strcpy(ampm, "AM");
						}
						else if(sDTIns.nHour == 12)
						{
							strcpy(ampm, "PM");
						}
						else if(sDTIns.nHour > 12)
						{
							sDTIns.nHour -= 12;
							strcpy(ampm, "PM");
						}
						
						sprintf(pstrDateTime, 
							"%04d-%02d-%02d %02d:%02d:%02d %s",						
							sDTIns.nYear,
							sDTIns.nMonth,
							sDTIns.nDay,
							sDTIns.nHour,
							sDTIns.nMinute,
							sDTIns.nSecode,
							ampm
						);
					}
					break;
				}
				case EM_DATETIME_MMDDYYYYHHNNSS:
				{
					if (!sDTIns.emTimeFormat)
					{
						sprintf(pstrDateTime, 
							"%02d-%02d-%04d %02d:%02d:%02d",
							sDTIns.nMonth,
							sDTIns.nDay,
							sDTIns.nYear,
							sDTIns.nHour,
							sDTIns.nMinute,
							sDTIns.nSecode
						);
					}
					else
					{						
						char ampm[4] = {0};
						
						if(!sDTIns.nHour)
						{
							sDTIns.nHour = 12;
							strcpy(ampm, "AM");
						}
						else if(sDTIns.nHour < 12)
						{
							strcpy(ampm, "AM");
						}
						else if(sDTIns.nHour == 12)
						{
							strcpy(ampm, "PM");
						}
						else if(sDTIns.nHour > 12)
						{
							sDTIns.nHour -= 12;
							strcpy(ampm, "PM");
						}
						
						sprintf(pstrDateTime, 
							"%02d-%02d-%04d %02d:%02d:%02d %s",
							sDTIns.nMonth,
							sDTIns.nDay,
							sDTIns.nYear,
							sDTIns.nHour,
							sDTIns.nMinute,
							sDTIns.nSecode,
							ampm
						);
					}
					break;
				}
				case EM_DATETIME_DDMMYYYYHHNNSS:
				{
					if (!sDTIns.emTimeFormat)
					{
						sprintf(pstrDateTime, 
							"%02d-%02d-%04d %02d:%02d:%02d",
							sDTIns.nDay,
							sDTIns.nMonth,
							sDTIns.nYear,
							sDTIns.nHour,
							sDTIns.nMinute,
							sDTIns.nSecode
						);
					}
					else
					{
						char ampm[4] = {0};
						
						if(!sDTIns.nHour)
						{
							sDTIns.nHour = 12;
							strcpy(ampm, "AM");
						}
						else if(sDTIns.nHour < 12)
						{
							strcpy(ampm, "AM");
						}
						else if(sDTIns.nHour == 12)
						{
							strcpy(ampm, "PM");
						}
						else if(sDTIns.nHour > 12)
						{
							sDTIns.nHour -= 12;
							strcpy(ampm, "PM");
						}

						sprintf(pstrDateTime, 
							"%02d-%02d-%04d %02d:%02d:%02d %s",
							sDTIns.nDay,
							sDTIns.nMonth,
							sDTIns.nYear,
							sDTIns.nHour,
							sDTIns.nMinute,
							sDTIns.nSecode,
							ampm
						);
					}
					break;
				}
				default:
				{
					MODSYS_DEBUG_STR("not supported datetime format!!!\n");
					printf("not supported datetime format!!!\n");
					rtn = -1;
					break;
				}
			}
			
			if(!rtn)
			{
				memset(&sEvPara, 0, sizeof(sEvPara));
				strcpy(sEvPara.szTime, pstrDateTime);
				(*pCB)(EM_SYSEVENT_DATETIME_STR, &sEvPara);
				//EvNotify(, EM_SYSEVENT_DATETIME_STR, &sEvPara);
				memcpy(&sEvPara.sDateTime,&sDTIns,sizeof(sDTIns));
				(*pCB)(EM_SYSEVENT_DATETIME_YMD, &sEvPara);
				
				//MODSYS_DEBUG_STR("check this!!!\n");
				
				//EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_DATETIME_YMD, &sEvPara);
			}
			
			//if(0==(cnr++%5))
			//	MODSYS_DEBUG_STR(szDateTime);
		}
	}
	
	return NULL;
}

s32 ModSysComplexInit( PSModSysCmplxInit pSysCmplxInit )
{
	s32 rtn = 0;//, i;
	s32 errCode = EM_MODSYSINIT_SUCCESS;
	pthread_t modSysThxd, modSysUpTimeThxd;
	SMsgCmdHdr pCmd;
	SModSyscplx* psModSysCplx = NULL;
	
//#define NO_RTC	//zlbfix20110715 time
#ifndef NO_RTC
	/* read time from rtc and set time to linux system */
	time_t cur;
	cur = time(NULL);
	cur = read_rtc(1);
	printf("utc : system current time : %d, date : %s\n", (int)cur, ctime(&cur));	
	stime(&cur);
	//cur = time(NULL);
#else
	char times[20];
	char *ptime;
	char tmptime[2];
	sprintf(times,"%s",__TIME__);
	ptime = times;
	
	time_t cur;
	struct tm tm_now;
	tm_now.tm_year	= 2011 - 1900;
	tm_now.tm_mon	= 7 - 1;
	tm_now.tm_mday	= 15;
	
	strncpy(tmptime,ptime,2);
	ptime +=3;
	tm_now.tm_hour	= atoi(tmptime);
	strncpy(tmptime,ptime,2);
	ptime +=3;
	tm_now.tm_min	= atoi(tmptime);
	strncpy(tmptime,ptime,2);
	tm_now.tm_sec	= atoi(tmptime);
	cur = mktime(&tm_now);
	stime(&cur);
	cur = time(NULL);
	printf("after set:system current time:%d,date:%s",(int)cur,ctime(&cur));
	printf("buildtime : %s %s\n",__DATE__,__TIME__);
#endif
	
	psModSysCplx = &sModSysIns;
	
	if(psModSysCplx->bInit)
	{
		errCode = -1; // init have done
		goto MODSYS_ERR;
	}
	else
	{
		psModSysCplx->bInit = TRUE;
	}
	
	//csp modify 20131213
	psModSysCplx->nTimeZone = pSysCmplxInit->nTimeZone;
	
	// init hddmanager
	if(NULL==pSysCmplxInit->pDMHdr)
	{
		errCode = -1; // null dev handle pointer
		
		goto MODSYS_ERR;
	}
	
	psModSysCplx->pDiskManager = (disk_manager*)pSysCmplxInit->pDMHdr;
	
	// 创建可用空闲命令
	pCmd = CreateMsgCmd( sizeof(SCmdData) );
	if(NULL==pCmd)
	{
		errCode = -1;// create cmmand fail
		
		goto MODSYS_ERR;
	}
	
	psModSysCplx->pMsgCmd = pCmd;
	
	psModSysCplx->pFNSysNotifyCB = NULL;
	if(!pSysCmplxInit->pSysNotifyCB)
	{
		errCode = EM_MODSYSINIT_NULL;
	}
	else
	{
		psModSysCplx->pFNSysNotifyCB = pSysCmplxInit->pSysNotifyCB;
	}
	
	// register pUpTimeCB
	psModSysCplx->pUpdateTimeCB = NULL;
	if(!pSysCmplxInit->pUpTimeCB)
	{
		errCode = EM_MODSYSINIT_NULL;
	}
	else
	{
		psModSysCplx->pUpdateTimeCB = pSysCmplxInit->pUpTimeCB;
	}	
	
	psModSysCplx->pSysResetCB = NULL;
	if(!pSysCmplxInit->pSysResetCB)
	{
		errCode = EM_MODSYSINIT_NULL;
	}
	else
	{
		psModSysCplx->pSysResetCB = pSysCmplxInit->pSysResetCB;
	}
	
	psModSysCplx->pDisablePanelCB = NULL;
	if(!pSysCmplxInit->pDisablePanelCB)
	{
		errCode = EM_MODSYSINIT_NULL;
	}
	else
	{
		psModSysCplx->pDisablePanelCB = pSysCmplxInit->pDisablePanelCB;
	}
	
	psModSysCplx->pStopRecCB = NULL;
	if(!pSysCmplxInit->pStopRecCB)
	{
		errCode = EM_MODSYSINIT_NULL;
	}
	else
	{
		psModSysCplx->pStopRecCB = pSysCmplxInit->pStopRecCB;
	}
	
	psModSysCplx->pResumeRecCB = NULL;
	if(!pSysCmplxInit->pResumeRecCB)
	{
		errCode = EM_MODSYSINIT_NULL;
	}
	else
	{
		psModSysCplx->pResumeRecCB = pSysCmplxInit->pResumeRecCB;
	}
	
	psModSysCplx->pGetUpdateSizeCB = NULL;
	if(!pSysCmplxInit->pGetLogoSizeCB)
	{
		errCode = EM_MODSYSINIT_NULL;
	}
	else
	{
		psModSysCplx->pGetUpdateSizeCB = pSysCmplxInit->pGetLogoSizeCB;
	}
	
	// 创建定时提供时间字符串线程
	if(0 != (rtn = pthread_create(&modSysUpTimeThxd,
			NULL,
			modSysCmplx_UpTimeThreadFunc,
			NULL )))
	{
		//pthread_exit(&modSysUpTimeThxd);
		
		errCode=EM_MODSYSINIT_FAIL;
		
		goto MODSYS_ERR;
	}
	
	printf("Create modSysUpTimeThxd OK!\n");
	
	// 创建处理线程
	if(0 != (rtn = pthread_create(&modSysThxd,
			NULL,
			modSysCmplx_ThreadFunc,
			NULL )))
	{
		//pthread_exit(&modSysThxd);
		
		errCode=EM_MODSYSINIT_FAIL;
		
		goto MODSYS_ERR;
	}
	
	// 磁盘信息维护
	modsys_DskInfoUpdateInit();
	
MODSYS_ERR:
	MODSYS_DEBUG(errCode);
	
	if(errCode!=EM_MODSYSINIT_SUCCESS
		&&errCode!=EM_MODSYSINIT_NULL)
		rtn = errCode;
	
	return rtn;
}

/*
1.2	s32 ModSysComplexDeinit(void)
	函数功能:模块反初始化
	输入参数：无
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDeinit(void)
{
	return 0;
}

/*
3.	s32 ModSysComplexExit(EMSYSEXITTYPE emSysExitType, u8 nDelay)
  需要两个参数：类型枚举（锁定/重起/关闭等）、延时时间(s)（u8 delay）；
	函数功能:系统退出
	输入参数：
		emSysExitType:
			类型:EMSYSEXITTYPE
			含义:退出类型锁定/重起/关闭系统
		nDelay
			类型:u8
			含义:延时时间(s)
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexExit(EMSYSEXITTYPE emSysExitType, u8 nDelay)
{
	s32 		rtn = 0; 
	s32 		errCode = 0;
	SMsgCmdHdr 	pCmd;
	SCmdData	sCmdDataIns;
	PSysExit	pSysExitIns;
	
	//EMSYSEVENT emSysEvent;
	//SEventPara sEventParaIns;
	
	pCmd = sModSysIns.pMsgCmd;
	if(NULL==pCmd)
	{
		errCode = -1;//null cmd struct
		goto EIXT_ERR;
	}
	memset( &sCmdDataIns, 0, sizeof(sCmdDataIns) );
	sCmdDataIns.cmd = EM_MODSYS_CMD_Exit;
	pSysExitIns = &sCmdDataIns.sExit;
	
	pSysExitIns->nDelay = nDelay;
	pSysExitIns->emSysExitType = emSysExitType;
	
	rtn = WriteMsgCmd( pCmd, (void*)&sCmdDataIns );
	if(0!=rtn)
	{
		errCode = -1;// write cmd fail
		goto EIXT_ERR;
	}
	
EIXT_ERR:
	if(0!=errCode)
	{
		rtn = -1;
		modSysCmplx_UpdateNotify(NULL, -1, -1);//exit request failed!!!
	}
	
	MODSYS_DEBUG(errCode);
	
	return rtn;
}

/*
4.	s32 ModSysComplexUpgrade(EMUPGRADEWAY emUpgradeWay, char *pcUpdateFName)
	函数功能:系统升级
	输入参数：
		emUpgradeWay:
			类型:EMUPGRADEWAY
			含义:升级途径（本地/通立协议/FTP）
		pcUpdateFName：
			类型:char *
			含义:升级文件路径（本地升级可以为空，其他不能为空）
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/

s32 ModSysComplexUpgrade(
	EMUPGRADETARGET	 	emUpdateTarget,
	EMUPGRADEWAY 		emUpgradeWay,
	char*				pcUpdateFName )
{
	s32 rtn = 0, errCode=0;
	SMsgCmdHdr 	pCmd;
	SCmdData	sCmdDataIns;
	//EMSYSEVENT emSysEvent;
	//SEventPara sEventParaIns;
	
	//MODSYS_ASSERT(pcUpdateFName);
	
	pCmd = sModSysIns.pMsgCmd;
	if(NULL==pCmd)
	{
		errCode = -1; // null cmd struct
		
		goto UPDATE_ERR;
	}
	
//	sModSysIns.pStopRecCB();//cw_9508S
	
	memset(&sCmdDataIns, 0, sizeof(sCmdDataIns));
	
	printf("Update request ..., by way %d\n", emUpgradeWay);
	switch(emUpgradeWay)
	{
		case EM_UPGRADE_USB: // 	, 	EM_UPGRADE_NET
		{
			modsys_SetDskAct(5, 1);
			
			sCmdDataIns.cmd = EM_MODSYS_CMD_UPGRADE_USB;
		}
		break;
		case EM_UPGRADE_NET:
		{
			sCmdDataIns.cmd = EM_MODSYS_CMD_UPGRADE_FTP;
			break;
		}
		default:
			errCode = -1; // not supported upgrade way
			break;
	}
	
	if( pcUpdateFName )
	{
		printf("update file %s\n", pcUpdateFName);
		if(strlen(pcUpdateFName) > MODSYS_PATH_LEN_MAX)
		{
			errCode = -1; // too long path
			
			goto UPDATE_ERR;
		}
		
		strcpy( sCmdDataIns.sUpdate.path, pcUpdateFName );
	}
	sCmdDataIns.sUpdate.target = emUpdateTarget;
	
	g_sysCmplxError = 0;
	
	rtn = WriteMsgCmd( pCmd, (void*)&sCmdDataIns );
	printf("%s over, rtn: %d\n", __func__, rtn);
	
	if(0!=rtn)
	{
		errCode = -1; // write cmd fail
		
		goto UPDATE_ERR;
	}
	
UPDATE_ERR:

	if(0!=errCode || g_sysCmplxError)
	{
		SProgress sProg;

		sProg.lTotalSize = 0;
		sProg.lCurSize = 0;
		sProg.nProgress = 0;

		//modSysCmplx_UpdateNotify(&sProg, EM_SYSEVENT_UPGRADE_DONE, EM_UPGRADE_FAILED_UNKNOW);

		rtn = -1;
	}
	
	MODSYS_DEBUG(rtn);
	
	if(emUpgradeWay == EM_UPGRADE_USB)
		modsys_SetDskAct(5, 0);
	
	if(g_sysCmplxError != 0)
		sModSysIns.pResumeRecCB();
	
	return rtn;
}

/*
void ModsysCmplexBreakBackup( void )
函数功能:中断文件备份
*/
s32 ModSysComplexBreakBackup( int type )
{
	modsysCmplx_SetBreakBackup(type);

	return 0;
}

/*
5.	s32 ModSysComplexBackup(EMFILETYPE emFileType, PSBackTgtAtt	sBackTgtAtt, SSearchCondition* psSearchCondition)
	函数功能:文件备份
	输入参数：
		emFileType:
			类型:EMFILETYPE
			含义:文件类型
		emFormat:
			类型:EMFILEFORMAT
			含义:输出的文件格式
		psSearchCondition：
			类型:SSearchCondition*
			含义:搜索条件, 搜索结果buff都需要调用方提供,并且需要保证在备份结束之前不能释放该buff
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexBackup( EMFILETYPE 	emFileType,
						 PSBackTgtAtt	psBackTgtAtt,
						 SSearchCondition* psSearchCondition )
{
	s32 		errCode	= 0; 
	s32			rtn		= 0;
	SCmdData 	sCmdDataIns;
	SMsgCmdHdr	pCmd;
	PSBackup	psBackupIns;
	//SEventPara 	sEventParaIns;
	SSearchResult* psSRIns;
	
	MODSYS_ASSERT(NULL!=psBackTgtAtt);
	MODSYS_ASSERT(NULL!=psSearchCondition);
	//MODSYS_ASSERT(NULL!=psSearchCondition->sSearchResult.psRecfileInfo);
	
	//sEventParaIns.emResult = 0xee;
	//EvNotify(sModSysIns.pFNSysNotifyCB, 0xff, &sEventParaIns); // start backup

	//printf("%s yg 1\n", __func__);
	modsys_SetDskAct( 3, TRUE );
	
	pCmd = sModSysIns.pMsgCmd;
	if(NULL==pCmd)
	{
		errCode = -1; // null cmd struct
		
		goto BACKUP_ERR;
	}

	// clear the conditions
	memset(&sCmdDataIns, 0, sizeof(sCmdDataIns));

	psBackupIns = &sCmdDataIns.sBackup;
	psSRIns = &psBackupIns->sSearchResult;

	if(EM_SEARCH_BY_TIME == psSearchCondition->emSearchWay)
	{
		// search file by the assigned conditions
		// now support by time or file
		// return filenum and fileinfo list
		
		// pSearchBuf = malloc(sizeof(SRecfileInfo)*SEARCH_FILE_MAX);
		//psSRIns->psRecfileInfo = (SRecfileInfo *)pSearchBuf;
		
		// 搜索buff由外部提供
		if(psSRIns->psRecfileInfo==NULL)
		{
			MODSYS_DEBUG_STR("no free memory for backup files list!\n");
			
			goto BACKUP_ERR;
		}
		rtn = ModSysComplexDMSearch( emFileType,
									&psSearchCondition->sSearchPara,
									psSRIns,
									SEARCH_FILE_MAX );
		if( 0!=rtn )
		{
			errCode = -1;// search error
			MODSYS_DEBUG(errCode);

			goto BACKUP_ERR;
		}
		
		memcpy(&psBackupIns->sSearchResult, psSRIns, sizeof(SSearchResult));
	}
	else //EM_SEARCH_BY_FILE
	{
		//printf("%s yg 2\n", __func__);
		memcpy(&psBackupIns->sSearchResult, &psSearchCondition->sSearchResult, sizeof(SSearchResult));
	}
	
	if(psSRIns->psRecfileInfo)//psRecSnapInfo
	{
		printf("file list is ok! file count %d\n", psSRIns->nFileNum);
	}
	else
	{
		printf("file list null!\n");
		exit(1);
	}
	
	if( psSRIns->nFileNum==0 )
	{
		errCode = -1; // no file found, not continue
		goto BACKUP_ERR;
	}
	
	// now only as found too many  files over max-limit, the got filenum is negtive,
	if( psSRIns->nFileNum>SEARCH_FILE_MAX )
	{
		psSRIns->nFileNum = SEARCH_FILE_MAX;
		
		MODSYS_DEBUG_STR("too many files fount!!!!\n");
		
		errCode = -1; // search no files
		
		// goto BACKUP_ERR;
	}
	
	psBackupIns->emFileType = emFileType;//EM_FILE_SNAP
	memcpy( &psBackupIns->sBackTgtAtt, psBackTgtAtt, sizeof(SBackTgtAtt) );
	
	sCmdDataIns.cmd = EM_MODSYS_CMD_BACKUP;
	
	MODSYS_DEBUG_STR("\nwait backup action complete....!\n");
	
	rtn = WriteMsgCmd( pCmd, (void*)&sCmdDataIns );
	if(0!=rtn)
	{
		errCode = -1; // write cmd fail
		
		goto BACKUP_ERR;
	}
	
BACKUP_ERR:
	
	/*
	if (EM_SEARCH_BY_TIME == psSearchCondition->emSearchWay)
	{
		if(pSearchBuf) free(pSearchBuf);
	}
	*/
	
	MODSYS_DEBUG(errCode);
	
	//sEventParaIns.emResult = 0xdd;
	//EvNotify(sModSysIns.pFNSysNotifyCB, 0xcc, &sEventParaIns); // init success
	
	modsys_SetDskAct( 2, TRUE );
	
	return errCode;
}

/*
6.	DMHDR ModSysComplexDMHdrGet(void)
	函数功能:获得录像设备管理句柄（供业务层调度给其他模块用）
	输入参数：无
	返回：
		类型:MHDR
		含义:句柄或者NULL
		取值:非NULL成功;NUL失败
*/
//DMHDR ModSysComplexDMHdrGet(void)
//{
//	return (void*)&hddmanager;
//}

/*
7.	s32 ModSysComplexDMGetInfo(SStoreDevManage *psStoreDevManage)
	函数功能:获得存储设备管理信息，一个参数，设备管理信息结构体（数量及各设备信息）
	输出参数：
		psStoreDevManage:
			类型:SStoreDevManage
			含义:存储设备管理信息
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/

s32 ModSysComplexDMGetInfo(SStoreDevManage *psStoreDevManage, u8 MaxHddNum)
{
	s32 rtn = 0;
	s32 errCode = 0;
	SMsgCmdHdr pCmd;
	SCmdData sCmdDataIns;
	
	//SEventPara		sEvPara;
	//EMSYSEVENT	emEvent;
	
	MODSYS_ASSERT(psStoreDevManage);
	
	modsys_SetDskAct(4, 1);//pModSys->bRefreshSata = 1;
	
	pCmd = sModSysIns.pMsgCmd;
	if(NULL==pCmd)
	{
		errCode = -1; // null cmd struct
		
		goto EIXT_ERR;
	}
	
	MODSYS_DEBUG_STR("Enter ModSysComplexDMGetInfo");
	
	memset(&sCmdDataIns, 0, sizeof(sCmdDataIns));
	sCmdDataIns.cmd = EM_MODSYS_CMD_DMGetInfo;
	
	if(MaxHddNum == 0 || MaxHddNum > nMaxSupportDiskNum)
	{
		printf("%s MaxHddNum(%d) > nMaxSupportDiskNum(%d)\n", 
			__func__, MaxHddNum, nMaxSupportDiskNum);
		
		sCmdDataIns.sStDevMgr.nMax = nMaxSupportDiskNum;
	}
	else
	{
		sCmdDataIns.sStDevMgr.nMax = MaxHddNum;
	}
	
	sCmdDataIns.sStDevMgr.pDevMgr = psStoreDevManage;

	//printf("yg WriteMsgCmd\n");
	rtn = WriteMsgCmd(pCmd, (void*)&sCmdDataIns);
	if(0!=rtn)
	{
		errCode = -1; // write cmd fail
		goto EIXT_ERR;
	}
	
EIXT_ERR:	
	//memset(&sEvPara, 0, sizeof(sEvPara));
	
	//sEvPara.nDelay = psExit->nDelay;
	//sEvPara.emResult = (errCode==0)?0:1; // 0 success, 1 failed
	//emEvent = -1;	
	//EvNotify(sModSysIns.pFNSysNotifyCB, emEvent, &sEvPara);
	
	MODSYS_DEBUG(errCode);
	
	if(errCode!=0)
	{
		rtn = -1;
	}
	
	return rtn;
}

/*
8.	s32 ModSysComplexDMFormat(char *pstrDevPath, EMFORMATWAY emFormatWay)
	函数功能:格式化
	输入参数：
		pstrDevPath:
			类型:char *
			含义:设备路径
		emFormatWay:
			类型:EMFORMATWAY
			含义:格式化格式及方式
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDMFormat(char *pstrDevPath, EMFORMATWAY emFormatWay)
{
	s32 errCode=0, rtn=0;
	SCmdData sCmdDataIns;
	SMsgCmdHdr	pCmd;
	SFormat*	psFormatIns;
	//SEventPara sEventParaIns;

	MODSYS_ASSERT(NULL!=pstrDevPath);

	//memset(&sEventParaIns, 0, sizeof(sEventParaIns));

	//sEventParaIns.emResult = 0xee;
	//EvNotify(sModSysIns.pFNSysNotifyCB, 0xff, &sEventParaIns); // init success

	modsys_SetDskAct( 1, TRUE );
	//yg modify
	sModSysIns.nFormatdiskIdx = pstrDevPath[strlen(pstrDevPath)-1] - 'a' + 1;//sda--1 sdb--2

	pCmd = sModSysIns.pMsgCmd;
	if(NULL==pCmd)
	{
		errCode = -1; // null cmd struct
		
		goto FORMAT_ERR;
	}
	
	// search file by the assigned conditions
	// now support by time or file
	// return filenum and fileinfo array
	
	//errCode = -1; // start search
	
	// parse the conditions
	memset(&sCmdDataIns, 0, sizeof(sCmdDataIns));
	
	psFormatIns = &sCmdDataIns.sFormat;
	psFormatIns->emFormatWay = emFormatWay;
	psFormatIns->strDevPath = pstrDevPath;
	
	sCmdDataIns.cmd = EM_MODSYS_CMD_DMFormat;
	
	g_sysCmplxError = 0;
	
	rtn = WriteMsgCmd( pCmd, (void*)&sCmdDataIns );
	if(0!=rtn)
	{
		errCode = -1; // write cmd fail
		
		goto FORMAT_ERR;
	}
	
FORMAT_ERR:

	MODSYS_DEBUG(errCode);

	//sEventParaIns.emResult = 0xdd;
	//EvNotify(sModSysIns.pFNSysNotifyCB, 0xcc, &sEventParaIns); // init success

	if(errCode!=0) rtn = -1;
	if(g_sysCmplxError!=0) rtn = -1;

	modsys_SetDskAct( 1, FALSE );	
	modsys_SetDskAct( 0, TRUE );
	
	sModSysIns.pResumeRecCB();
	
	return rtn;
}

/*
9.	s32 ModSysComplexDMSearch(EMFILETYPE emFileType, SSearchPara* psSearchPara, SSearchResult* psSearchResult, u16 nMaxFileNum;)
	函数功能:文件搜索
	输入参数：
		emFileType:
			类型:EMFILETYPE
			含义:文件类型
		psSearchPara:
			类型:SSearchPara
			含义:搜索条件
		psSearchResul:
			类型:SSearchResult
			含义:传入用于存放搜索结果结构体
		nMaxFileNum:
			类型:u16
			含义:psRecfileList的最大个数
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDMSearch(	EMFILETYPE emFileType,
							SSearchPara* psSearchPara,
							SSearchResult* psSearchResult,
							u16 nMaxFileNum )
{
	s32 errCode=0, rtn=0;
	SCmdData sCmdDataIns;
	SMsgCmdHdr	pCmd;
	SSearch		sSearchIns;
	//SEventPara sEventParaIns;

	//sEventParaIns.emResult = 0xee;
	//EvNotify(sModSysIns.pFNSysNotifyCB, 0xff, &sEventParaIns); // init success

	pCmd = sModSysIns.pMsgCmd;
	if(NULL==pCmd)
	{
		errCode = -1; // null cmd struct

		goto SEARCH_ERR;
	}

	// search file by the assigned conditions
	// now support by time or file
	// return filenum and fileinfo array

	MODSYS_ASSERT(NULL!=psSearchPara && NULL!=psSearchResult);
	MODSYS_ASSERT(psSearchResult->psRecfileInfo!=NULL);
	
	//errCode = -1; // start search

	// parse the conditions
	memset(&sSearchIns, 0, sizeof(sSearchIns));
	memset(&sCmdDataIns, 0, sizeof(sCmdDataIns));

	sSearchIns.emFileType = emFileType;
	sSearchIns.psSearchPara = psSearchPara;
	sSearchIns.nMaxFileNum= nMaxFileNum;

	sSearchIns.psSearchResult = psSearchResult;

	sCmdDataIns.cmd = EM_MODSYS_CMD_RECSEARCH;
	memcpy(&sCmdDataIns.sSearch, &sSearchIns, sizeof(sSearchIns));

	rtn = WriteMsgCmd( pCmd, (void*)&sCmdDataIns );
	if(0!=rtn)
	{
		errCode = -1; // write cmd fail

		goto SEARCH_ERR;
	}

SEARCH_ERR:

	MODSYS_DEBUG(errCode);

	//sEventParaIns.emResult = 0xdd;
	//EvNotify(sModSysIns.pFNSysNotifyCB, 0xcc, &sEventParaIns); // init success

	if(errCode!=0) rtn = -1;
	
	return rtn;
}

/*
12.	s32 ModSysComplexDTSet(const SDateTime* psDateTime)
	函数功能:设置系统日期时间
	输入参数：
		psDateTime:
			类型:const SDateTime*
			含义:用于设置日期时间
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDTSet(SDateTime* psDateTime)
{
	s32 rtn = 0;
	s32 errCode=0;
	SMsgCmdHdr 	pCmd;
	SCmdData sCmdDataIns;
	SModSyscplx* psModSysCplx;
	
	//SEventPara		sEvPara;
	//EMSYSEVENT	emEvent;
	
	MODSYS_ASSERT(psDateTime);
	
	psModSysCplx = &sModSysIns;
	
	pCmd = psModSysCplx->pMsgCmd;
	if(NULL==pCmd)
	{
		errCode = -1;// null cmd struct
		
		goto EIXT_ERR;
	}
	
	memset( &sCmdDataIns, 0, sizeof(sCmdDataIns) );
	sCmdDataIns.cmd = EM_MODSYS_CMD_DTSet;
	sCmdDataIns.psDateTime = (SDateTime*)psDateTime;
	rtn = WriteMsgCmd( pCmd, (void*)&sCmdDataIns );
	if(0!=rtn)
	{
		errCode = -1;// write cmd fail
		
		goto EIXT_ERR;
	}
	
EIXT_ERR:
	
	//memset(&sEvPara, 0, sizeof(sEvPara));
	
	//sEvPara.nDelay = psExit->nDelay;
	//sEvPara.emResult = (errCode==0)?0:1; // 0 success, 1 failed
	//emEvent = -1;	
	//EvNotify(sModSysIns.pFNSysNotifyCB, emEvent, &sEvPara);
	
	MODSYS_DEBUG(errCode);
	
	if(errCode!=0) rtn = -1;
	else psModSysCplx->bSetTime = TRUE;
	
	return rtn;
}

s32 ModSysComplexSetTimeZone(int nTimeZone)
{
	sModSysIns.nTimeZone = nTimeZone;
	return 0;
}

/*
13.	s32 ModSysComplexDTGet(SDateTime* psDateTime)
	函数功能:获得系统日期时间
	输入参数：
		psDateTime:
			类型:SDateTime*
			含义:用于获得日期时间
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDTGet(SDateTime* psDateTime)
{
	s32 rtn = 0;
	//SEventPara		sEvPara;
	//EMSYSEVENT	emEvent;
	
	if(NULL==psDateTime)
	{
		MODSYS_DEBUG_STR("null datetime pointer!\n");
		rtn = -1;
		goto DTSET_ERR;
	}
	
	modSysCmplx_GetTime(psDateTime);
	
DTSET_ERR:
	
	//memset(&sEvPara, 0, sizeof(sEvPara));
	
	//sEvPara.nDelay = psExit->nDelay;
	//sEvPara.emResult = (rtn==0)?0:1; // 0 success, 1 failed
	//emEvent = -1;	
	//EvNotify(sModSysIns.pFNSysNotifyCB, emEvent, &sEvPara);
	
	return 0;
}

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
	if(index < 0 || index >= sizeof(TimeZoneOffset)/sizeof(TimeZoneOffset[0]))
	{
		return 0;
	}
	
	return TimeZoneOffset[index];
}
//转换时间time_t to SDateTime
s32 ModSysComplexConvertToDT(time_t time, SDateTime* psDateTime)
{
	if(NULL==psDateTime)
	{
		MODSYS_DEBUG_STR("null datetime pointer!\n");
		return -1;
	}
	
	time += GetTZOffset(sModSysIns.nTimeZone);
	
	struct tm tm_time;
	localtime_r(&time, &tm_time);
	struct tm *ptm = &tm_time;
	
	psDateTime->nYear 	= ptm->tm_year+1900;
	psDateTime->nMonth	= ptm->tm_mon+1;
	psDateTime->nDay	= ptm->tm_mday;
	psDateTime->nHour	= ptm->tm_hour;
	psDateTime->nMinute = ptm->tm_min;
	psDateTime->nSecode = ptm->tm_sec;
	psDateTime->nWday	= ptm->tm_wday;
	psDateTime->emDateTimeFormat = (EMDATETIMEFORMAT)sModSysIns.eTimeFmt;
	psDateTime->emTimeFormat = (EMTIMEFORMAT)sModSysIns.eTmFormat;
	
	return 0;
}

s32 ModSysComplexDTGet_TZ(SDateTime* psDateTime)
{
	if(NULL==psDateTime)
	{
		MODSYS_DEBUG_STR("null datetime pointer!\n");
		return -1;
	}
	
	time_t currtime = time(NULL);
	currtime += GetTZOffset(sModSysIns.nTimeZone);
	
	struct tm tm_time;
	localtime_r(&currtime, &tm_time);
	struct tm *ptm = &tm_time;
	
	psDateTime->nYear 	= ptm->tm_year+1900;
	psDateTime->nMonth	= ptm->tm_mon+1;
	psDateTime->nDay	= ptm->tm_mday;
	psDateTime->nHour	= ptm->tm_hour;
	psDateTime->nMinute = ptm->tm_min;
	psDateTime->nSecode = ptm->tm_sec;
	psDateTime->nWday	= ptm->tm_wday;
	psDateTime->emDateTimeFormat = (EMDATETIMEFORMAT)sModSysIns.eTimeFmt;
	psDateTime->emTimeFormat = (EMTIMEFORMAT)sModSysIns.eTmFormat;
	
	return 0;
}

/*
14.	s32 ModSysComplexDTGetString(char* pstrDateTime)
	函数功能:获得格式化系统日期时间字符串
	输入参数：
		pstrDateTime:
			类型:char*
			含义:用于获得日期时间字符串(先按1.0默认YYYY-MM-DD HH:NN:SS)
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 ModSysComplexDTGetString(char* pstrDateTime)
{
	s32 rtn = 0;
	SDateTime sDTIns;
	
	MODSYS_ASSERT(NULL!=pstrDateTime);
	
	memset( &sDTIns, 0, sizeof(sDTIns) );
	
	rtn = ModSysComplexDTGet(&sDTIns);
	if(0==rtn)
	{
		switch(sDTIns.emDateTimeFormat)
		{
			case EM_DATETIME_YYYYMMDDHHNNSS:
			{
				if (!sDTIns.emTimeFormat)
				{
					sprintf(pstrDateTime, 
						"%04d-%02d-%02d %02d:%02d:%02d",
						sDTIns.nYear,
						sDTIns.nMonth,
						sDTIns.nDay,
						sDTIns.nHour,
						sDTIns.nMinute,
						sDTIns.nSecode
					);
				}
				else
				{
					char ampm[4] = {0};
						
					if(!sDTIns.nHour)
					{
						sDTIns.nHour = 12;
						strcpy(ampm, "AM");
					}
					else if(sDTIns.nHour < 12)
					{
						strcpy(ampm, "AM");
					}
					else if(sDTIns.nHour == 12)
					{
						strcpy(ampm, "PM");
					}
					else if(sDTIns.nHour > 12)
					{
						sDTIns.nHour -= 12;
						strcpy(ampm, "PM");
					}

					sprintf(pstrDateTime, 
					"%04d-%02d-%02d %02d:%02d:%02d %s",
					sDTIns.nYear,
					sDTIns.nMonth,
					sDTIns.nDay,
					sDTIns.nHour,
					sDTIns.nMinute,
					sDTIns.nSecode,
					ampm
					);
				}
				break;
			}
			case EM_DATETIME_MMDDYYYYHHNNSS:
			{
				if (!sDTIns.emTimeFormat)
				{
					sprintf(pstrDateTime, 
						"%02d-%02d-%04d %02d:%02d:%02d",
						sDTIns.nMonth,
						sDTIns.nDay,
						sDTIns.nYear,
						sDTIns.nHour,
						sDTIns.nMinute,
						sDTIns.nSecode
					);
				}
				else
				{
					char ampm[4] = {0};
					
					if(!sDTIns.nHour)
					{
						sDTIns.nHour = 12;
						strcpy(ampm, "AM");
					}
					else if(sDTIns.nHour < 12)
					{
						strcpy(ampm, "AM");
					}
					else if(sDTIns.nHour == 12)
					{
						strcpy(ampm, "PM");
					}
					else if(sDTIns.nHour > 12)
					{
						sDTIns.nHour -= 12;
						strcpy(ampm, "PM");
					}

					sprintf(pstrDateTime, 
						"%02d-%02d-%04d %02d:%02d:%02d %s",
						sDTIns.nMonth,
						sDTIns.nDay,
						sDTIns.nYear,
						sDTIns.nHour,
						sDTIns.nMinute,
						sDTIns.nSecode,
						ampm
					);
				}
				break;
			}
			case EM_DATETIME_DDMMYYYYHHNNSS:
			{
				if (!sDTIns.emTimeFormat)
				{
					sprintf(pstrDateTime, 
						"%02d-%02d-%04d %02d:%02d:%02d",
						sDTIns.nDay,
						sDTIns.nMonth,
						sDTIns.nYear,
						sDTIns.nHour,
						sDTIns.nMinute,
						sDTIns.nSecode
					);
				}
				else
				{
					char ampm[4] = {0};
					
					if(!sDTIns.nHour)
					{
						sDTIns.nHour = 12;
						strcpy(ampm, "AM");
					}
					else if(sDTIns.nHour < 12)
					{
						strcpy(ampm, "AM");
					}
					else if(sDTIns.nHour == 12)
					{
						strcpy(ampm, "PM");
					}
					else if(sDTIns.nHour > 12)
					{
						sDTIns.nHour -= 12;
						strcpy(ampm, "PM");
					}

					sprintf(pstrDateTime, 
						"%02d-%02d-%04d %02d:%02d:%02d %s",
						sDTIns.nDay,
						sDTIns.nMonth,
						sDTIns.nYear,
						sDTIns.nHour,
						sDTIns.nMinute,
						sDTIns.nSecode,
						ampm
					);
				}
				break;
			}
			default:
			{
				MODSYS_DEBUG_STR("not supported datetime format!!!\n");
				rtn = -1;
				break;
			}

		}
	}
	
	//SEventPara		sEvPara;
	//EMSYSEVENT		emEvent;
	
	//memset(&sEvPara, 0, sizeof(sEvPara));
	
	//sEvPara.nDelay = psExit->nDelay;
	//sEvPara.emResult = (rtn==0)?0:1; // 0 success, 1 failed
	//emEvent = -1;	
	//EvNotify(sModSysIns.pFNSysNotifyCB, emEvent, &sEvPara);

	return rtn;
}

/*
	函数功能:请求系统退出锁定/关机/重启
	输入参数：
		psExit 重启参数 (延时和退出类型)
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
static s32 modSysCmplx_SysExit( SSysExit* psExit )
{
	s32 rtn = 0;
	
	MODSYS_ASSERT(psExit);

	SEventPara		sEvPara;
	EMSYSEVENT		emEvent = 0;
	
	memset(&sEvPara, 0, sizeof(sEvPara));
	
	sEvPara.nDelay = psExit->nDelay;
	//sEvPara.emResult = 0; // 0 success, 1 failed
	//emEvent = EM_SYSEVENT_LOCK;	
	//EvNotify(sModSysIns.pFNSysNotifyCB, emEvent, &sEvPara);
	
	SModSyscplx* psModSysCplx = &sModSysIns;
	
	switch(psExit->emSysExitType)
	{
		case EM_SYSEXIT_LOCK:
		{
			emEvent = EM_SYSEVENT_LOCK;
			break;
		}
		case EM_SYSEXIT_RESTART:
		{
			struct timeval val;
			
			emEvent = EM_SYSEVENT_RESTART;
			
			val.tv_sec  = psExit->nDelay;
			val.tv_usec = 0;
			select(0,NULL,NULL,NULL,&val);
			
			printf("system reboot\n");
			usleep(2*1000*1000);
			
			if(psModSysCplx->pSysResetCB)
			{
				(*psModSysCplx->pSysResetCB)();
			}
			
			//system(reboot); //9624调用下行代码无效
			//reboot(RB_AUTOBOOT);
			
			break;
		}
		case EM_SYSEXIT_POWEROFF:
		{
			if(modSysComplex_CheckPowerOff())
			{
				struct timeval val;
				
				emEvent = EM_SYSEVENT_RESTART;
				
				val.tv_sec  = psExit->nDelay;
				val.tv_usec = 0;
				select(0,NULL,NULL,NULL,&val);

				// shutdown
				// todo...
			}
			else // cannot shutdown power, so call cb to notify biz
			{
				//sModSysIns.pDisablePanelCB(0);//csp modify 20121022
				
				emEvent = EM_SYSEVENT_POWEROFF_MANUAL;
				
				printf("EM_SYSEVENT_POWEROFF_MANUAL\n");
				modSysCmplx_UpdateNotify(NULL, emEvent, 0);
			}
			break;
		}
		default:
		{
			rtn = -1;
			MODSYS_DEBUG_STR("not supported exit type!!!!\n");
			break;
		}
	}

	//if(0!=rtn)
	//{
	//	sEvPara.emResult = 1; // 0 success, 1 failed
	//}
	//else
	//{
	//	sEvPara.emResult = 0; // 0 success, 1 failed
	//}

	//EvNotify(sModSysIns.pFNSysNotifyCB, emEvent, &sEvPara);
	
	MODSYS_DEBUG(rtn);

//	printf("EM_SYSEVENT_POWEROFF_MANUAL \n");

	return rtn;
}

s32 ModSysComplexGetUpdateFileInfo(EMUPGRADETARGET emTgt, SUpdateFileInfoList* pInfo)
{
	s32 rtn = 0;
	
	if(pInfo)
		modSysComplex_GetUpdateFileInfo(emTgt, pInfo);
	else
		rtn = -1;
	
	return rtn;
}

s32 modSysComplex_CheckPowerOff()
{
	return 0;
}

s32 ModSysComplexGetUsbDirInfo(SUsbDirList* pInfo)
{
	return modSysComplex_GetUsbDevDir(pInfo);
}

u8 ModSysGetInsertDiskStatue()
{
	return modsys_GetInsertDiskStatue();
}

s32 ModSysComplexGetConfigFileInfo(char* szDir, SUpdateFileInfoList* pInfo)
{
	s32 rtn = 0;
	
	if(pInfo&&szDir)
		modSysComplex_GetConfigFileInfo(szDir, pInfo);
	else
		rtn = -1;

	return rtn;
}

void ModSysComplexRegNetUpdateProgCB(PFNRemoteUpdateProgressCB pCB)
{
	if(pCB)
		sModSysIns.pRemoteUpdateNotifyCB = pCB;
}

void ModSysComplexSetProductModel(char* updatemodel)
{
	modsysCmplx_SetProductModel(updatemodel);
	
	//csp modify
	g_modeworking = 1;
}

