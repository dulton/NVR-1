
// file description
#include "ddns.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>

//** macro

#define PRINT(y)	//printf("[New-DDNS] FUNC %s LINE %d err %x\n", __FUNCTION__, __LINE__, y)
	
//** typedef 

// DDNS配置结构
typedef struct _sDDNS
{
	u8				bInit;
	u8				bSvcInit;
	u8				bInstall;
	u8				bRegist;
	int 			nConnFlag;//登陆失败标志
	EM_DDNS_PROT 	eCurProt;
	u8				nIntvl;//目前不起作用
	
	u8				bActive;
	u8				bStop;
	
	char 			szUser[128];
	char 			szPasswd[128];
	char 			szDomain[128];
	
	PDDNSInitCB 	pInit;
	PDDNSRegistCB 	pReg;
	PDDNSUnRegCB 	pUnreg;
	PDDNSStartCB 	pStart, pStartTry;
	PDDNSStopCB 	pStop;
	PDDNSRefreshCB 	pRefresh;
	PDDNSDeInitCB 	pDeInit;
	PDDNSGetError	pGetErr;
	
} SDDNSCtrl;

//** local var

static SDDNSCtrl sDdnsCtrlIns;


//** global var



//** local functions
static void DDNS_NotifyUpdate(u8 key);
static void *ip_update(void* param);

//** global functions

// DDNS init
s32 DDNS_Init( u8 nFlag, u32 nUpIntvl )
{
	s32 err = 0;
	
	//csp modify 20130322
	static u8 init_flag = 0;
	if(!init_flag)
	{
		init_flag = 1;
		
		memset(&sDdnsCtrlIns, 0, sizeof(sDdnsCtrlIns));
		sDdnsCtrlIns.nConnFlag = 1;
	}
	
	sDdnsCtrlIns.bStop = 1;
	
	printf("DDNS_Init:nFlag=%d,nUpIntvl=%d\n",nFlag,nUpIntvl);
	
	pthread_t ipupdate;
	if( 0 != pthread_create(&ipupdate, NULL, ip_update, NULL) )
	{
		err = -1;
		goto END;
	}
	
	SDDNSProtCfg sCfgIns;
	
	// install saved prot
	switch(nFlag)
	{
		case EM_DDNS_PROT_TL:
		{
			printf("pop dvr ddns...\n");
			sCfgIns.pInit	= ddnstl_Init;
			sCfgIns.pReg	= ddnstl_Regist;
			sCfgIns.pUnreg	= ddnstl_Unreg;
			sCfgIns.pStart	= ddnstl_Start;
			sCfgIns.pStartTry = ddnstl_StartTry;
			sCfgIns.pStop	= ddnstl_Stop;
			sCfgIns.pRefresh= ddnstl_Refresh;
			sCfgIns.pDeInit	= ddnstl_DeInit;
			sCfgIns.pGetErr	= ddnstl_GetErr;	
		} break;
		case EM_DDNS_PROT_3322:
		{
			printf("3322 ddns...\n");
			sCfgIns.pInit	= ddnsother_Init;
			sCfgIns.pReg	= ddnsother_Regist;
			sCfgIns.pUnreg	= ddnsother_Unreg;
			sCfgIns.pStart	= ddns3322_Start;
			sCfgIns.pStartTry	= NULL;
			sCfgIns.pStop	= ddnsother_Stop;
			sCfgIns.pRefresh= ddns3322_Refresh;
			sCfgIns.pDeInit	= ddnsother_DeInit;
			sCfgIns.pGetErr	= ddnsother_GetErr;
		} break;
		case EM_DDNS_PROT_DYNDNS:
		{
			printf("dyndns ddns...\n");
			sCfgIns.pInit	= ddnsother_Init;
			sCfgIns.pReg	= ddnsother_Regist;
			sCfgIns.pUnreg	= ddnsother_Unreg;
			sCfgIns.pStart	= dyndns_Start;
			sCfgIns.pStartTry	= NULL;
			sCfgIns.pStop	= ddnsother_Stop;
			sCfgIns.pRefresh= dyndns_Refresh;
			sCfgIns.pDeInit	= ddnsother_DeInit;
			sCfgIns.pGetErr	= ddnsother_GetErr;
		} break;
		case EM_DDNS_PROT_DVRNET://cw_ddns
		{
			printf("dvrnet ddns...\n");
			sCfgIns.pInit	= dvrnet_Init;
			sCfgIns.pReg	= dvrnet_Regist;
			sCfgIns.pUnreg	= ddnsother_Unreg;
			sCfgIns.pStart	= dvrnet_Start;
			sCfgIns.pStartTry	= NULL;
			sCfgIns.pStop	= dvrnet_Stop;
			sCfgIns.pRefresh= dvrnet_Start;
			sCfgIns.pDeInit	= dvrnet_DeInit;
			sCfgIns.pGetErr	= ddnsother_GetErr;
		} break;
		case EM_DDNS_PROT_NOIP:
		{
			printf("noip ddns...\n");
			sCfgIns.pInit	= ddnsother_Init;
			sCfgIns.pReg	= ddnsother_Regist;
			sCfgIns.pUnreg	= ddnsother_Unreg;
			sCfgIns.pStart	= ddnsnoip_Start;
			sCfgIns.pStartTry	= NULL;
			sCfgIns.pStop	= ddnsother_Stop;
			sCfgIns.pRefresh= ddnsnoip_Start;
			sCfgIns.pDeInit	= ddnsother_DeInit;
			sCfgIns.pGetErr	= ddnsother_GetErr;
		} break;
		case EM_DDNS_PROT_JMDVR:
		{
			printf("jmdvr ddns...\n");
			sCfgIns.pInit	= ddnstl_Init;
			sCfgIns.pReg	= jmdvr_Regist;
			sCfgIns.pUnreg	= ddnstl_Unreg;
			sCfgIns.pStart	= jmdvr_Start;
			sCfgIns.pStartTry	= NULL;
			sCfgIns.pStop	= jmdvr_Stop;
			sCfgIns.pRefresh= jmdvr_Start;
			sCfgIns.pDeInit	= ddnsother_DeInit;
			sCfgIns.pGetErr	= ddnstl_GetErr;
		} break;
		case EM_DDNS_PROT_CHANGEIP:
		{
			printf("changeip ddns...\n");
			sCfgIns.pInit	= ddnsother_Init;
			sCfgIns.pReg	= ddnsother_Regist;
			sCfgIns.pUnreg	= ddnsother_Unreg;
			sCfgIns.pStart	= changeip_Start;
			sCfgIns.pStartTry	= NULL;
			sCfgIns.pStop	= ddnsother_Stop;
			sCfgIns.pRefresh= changeip_Start;
			sCfgIns.pDeInit	= ddnsother_DeInit;
			sCfgIns.pGetErr	= ddnsother_GetErr;
		} break;
		case EM_DDNS_PROT_JSJDVR:
		{
			printf("jsj ddns...\n");
			sCfgIns.pInit	= ddnstl_Init;
			sCfgIns.pReg	= jsjdvr_Regist;
			sCfgIns.pUnreg	= ddnstl_Unreg;
			sCfgIns.pStart	= jsjdvr_Start;
			sCfgIns.pStartTry	= NULL;
			sCfgIns.pStop	= NULL;
			sCfgIns.pRefresh= jsjdvr_Start;
			sCfgIns.pDeInit	= ddnstl_DeInit;
			sCfgIns.pGetErr	= ddnstl_GetErr;
		} break;
		default:
		{
			printf("unknown ddns type:%d\n",nFlag);
			err = 0;
			goto END;
		} break;
	}
	
	sCfgIns.nIntvl = nUpIntvl;
	
	DDNS_Install(nFlag, &sCfgIns);
	
END:
	PRINT(err);
	return 0;
}

// Install prot
s32 DDNS_Install( EM_DDNS_PROT eProt, SDDNSProtCfg* pProt )
{
	s32 nRet = 0;
	SDDNSProtCfg sInst;
	
	if(	eProt < EM_DDNS_PROT_TL 	|| 
		eProt >= EM_DDNS_PROT_MAX 	||
		sDdnsCtrlIns.eCurProt == eProt		
	)
	{
		nRet = -1;
		goto ERROR;
	}
	
	printf("^^^^^^^%s,%d,eProt=%d\n",__func__,__LINE__,eProt);//cw_test
	
	if(pProt == NULL)
	{
		switch(eProt)
		{
			case EM_DDNS_PROT_TL:
			{ 
				sInst.pInit	=	ddnstl_Init;
				sInst.pReg	=	ddnstl_Regist;
				sInst.pUnreg	=	ddnstl_Unreg;
				sInst.pStart	=	ddnstl_Start;
				sInst.pStartTry	=	ddnstl_StartTry;
				sInst.pStop	=	ddnstl_Stop;
				sInst.pRefresh	=	ddnstl_Refresh;
				sInst.pDeInit	=	ddnstl_DeInit;
				sInst.pGetErr	=	ddnstl_GetErr;
			} break;
			case EM_DDNS_PROT_3322:
			{
				sInst.pInit	=	ddnsother_Init;
				sInst.pReg	=	ddnsother_Regist;
				sInst.pUnreg	=	ddnsother_Unreg;
				sInst.pStart	=	ddns3322_Start;
				sInst.pStartTry	=	NULL;
				sInst.pStop	=	ddnsother_Stop;
				sInst.pRefresh	=	ddns3322_Refresh;
				sInst.pDeInit	=	ddnsother_DeInit;
				sInst.pGetErr	=	ddnsother_GetErr;
			} break;
			case EM_DDNS_PROT_DYNDNS:
			{
				sInst.pInit	=	ddnsother_Init;
				sInst.pReg	=	ddnsother_Regist;
				sInst.pUnreg	=	ddnsother_Unreg;
				sInst.pStart	=	dyndns_Start;
				sInst.pStartTry	=	NULL;
				sInst.pStop	=	ddnsother_Stop;
				sInst.pRefresh	=	dyndns_Refresh;
				sInst.pDeInit	=	ddnsother_DeInit;
				sInst.pGetErr	=	ddnsother_GetErr;
			} break;
			case EM_DDNS_PROT_DVRNET:
			{
				sInst.pInit	=	dvrnet_Init;
				sInst.pReg	=	dvrnet_Regist;
				sInst.pUnreg	=	ddnsother_Unreg;
				sInst.pStart	=	dvrnet_Start;
				sInst.pStartTry	=	NULL;
				sInst.pStop	=	dvrnet_Stop;
				sInst.pRefresh	=	dvrnet_Start;
				sInst.pDeInit	=	dvrnet_DeInit;
				sInst.pGetErr	=	ddnsother_GetErr;
			} break;
			case EM_DDNS_PROT_NOIP:
			{
				sInst.pInit	= ddnsother_Init;
				sInst.pReg	= ddnsother_Regist;
				sInst.pUnreg	= ddnsother_Unreg;
				sInst.pStart	= ddnsnoip_Start;
				sInst.pStartTry	= NULL;
				sInst.pStop	= ddnsother_Stop;
				sInst.pRefresh= ddnsnoip_Start;
				sInst.pDeInit	= ddnsother_DeInit;
				sInst.pGetErr	= ddnsother_GetErr;
			} break;
			case EM_DDNS_PROT_JMDVR:
			{
				sInst.pInit	= ddnstl_Init;
				sInst.pReg	= jmdvr_Regist;
				sInst.pUnreg	= ddnstl_Unreg;
				sInst.pStart	= jmdvr_Start;
				sInst.pStartTry	= NULL;
				sInst.pStop	= jmdvr_Stop;
				sInst.pRefresh= jmdvr_Start;
				sInst.pDeInit	= ddnstl_DeInit;
				sInst.pGetErr	= ddnstl_GetErr;
			} break;
			case EM_DDNS_PROT_CHANGEIP:
			{
				sInst.pInit	= ddnsother_Init;
				sInst.pReg	= ddnsother_Regist;
				sInst.pUnreg	= ddnsother_Unreg;
				sInst.pStart	= changeip_Start;
				sInst.pStartTry	= NULL;
				sInst.pStop	= ddnsother_Stop;
				sInst.pRefresh= changeip_Start;
				sInst.pDeInit	= ddnsother_DeInit;
				sInst.pGetErr	= ddnsother_GetErr;
			} break;
			case EM_DDNS_PROT_JSJDVR:
			{
				sInst.pInit	= ddnstl_Init;
				sInst.pReg	= jsjdvr_Regist;
				sInst.pUnreg	= ddnstl_Unreg;
				sInst.pStart	= jsjdvr_Start;
				sInst.pStartTry	= NULL;
				sInst.pStop	= NULL;
				sInst.pRefresh= jsjdvr_Start;
				sInst.pDeInit	= ddnstl_DeInit;
				sInst.pGetErr	= ddnstl_GetErr;
			} break;
			case EM_DDNS_PROT_KONLAN:
			{
				sInst.pInit	= ddnstl_Init;
				sInst.pReg	= konlan_Regist;
				sInst.pUnreg	= ddnstl_Unreg;
				sInst.pStart	= konlan_Start;
				sInst.pStartTry	= NULL;
				sInst.pStop	= konlan_Stop;
				sInst.pRefresh= konlan_Start;
				sInst.pDeInit	= ddnstl_DeInit;
				sInst.pGetErr	= ddnstl_GetErr;
			} break;
			default:
			{
				nRet =  -2;
				goto ERROR;
			} break;
		}
		
		sInst.nIntvl = 15;
	}
	else
	{
		sInst.pInit	=	pProt->pInit;
		sInst.pReg	=	pProt->pReg;
		sInst.pUnreg	=	pProt->pUnreg;
		sInst.pStart	=	pProt->pStart;
		sInst.pStop	=	pProt->pStop;
		sInst.pRefresh	=	pProt->pRefresh;
		sInst.pDeInit	=	pProt->pDeInit;
		sInst.pGetErr	=	pProt->pGetErr;
		sInst.nIntvl 	= 	pProt->nIntvl;
	}
	
	sDdnsCtrlIns.nIntvl 	= sInst.nIntvl;
	sDdnsCtrlIns.pInit 		= sInst.pInit;
	sDdnsCtrlIns.pReg 		= sInst.pReg;	
	sDdnsCtrlIns.pStart 	= sInst.pStart;
	sDdnsCtrlIns.pStop 	= sInst.pStop;
	sDdnsCtrlIns.pUnreg 	= sInst.pUnreg;
	sDdnsCtrlIns.pRefresh 	= sInst.pRefresh;
	sDdnsCtrlIns.pDeInit 	= sInst.pDeInit;
	sDdnsCtrlIns.pGetErr 	= sInst.pGetErr;
	
ERROR:
	
	PRINT(nRet);
	
	if(!nRet)
		sDdnsCtrlIns.bInstall = 1;
	
	return nRet;
}

// Uninstall
s32 DDNS_UnInstall( EM_DDNS_PROT eProt )
{
	if( sDdnsCtrlIns.bInstall )
	{
		sDdnsCtrlIns.bInstall = 0;

		usleep(50*1000);
		
		DDNS_SvcDeInit(eProt);
		
		memset(&sDdnsCtrlIns, 0, sizeof(sDdnsCtrlIns));
		/*
		if(sDdnsCtrlIns.eCurProt==eProt)
		{
			memset(&sDdnsCtrlIns, 0, sizeof(sDdnsCtrlIns));
		}
		else
		{
			PRINT("DDNS_UnInstall", 1);
			
			return -1;
		}
		*/
	}

	PRINT(0);
	
	return 0;
}


// init
s32 DDNS_SvcInit( EM_DDNS_PROT eProt, SDDNSInitPara* pInitPara )
{
	s32 ret = 0;
	
	if(!sDdnsCtrlIns.bInstall)
	{
		ret = -1;
		goto END;
	}
	
	printf("DDNS_SvcInit eProt=%d\n",eProt);
	
	sDdnsCtrlIns.eCurProt = eProt;
	
	if(!sDdnsCtrlIns.bSvcInit)
	{
		if(sDdnsCtrlIns.pInit)
		{
			if(pInitPara)
				sDdnsCtrlIns.pInit( &pInitPara->sTlIns );
			else
				sDdnsCtrlIns.pInit( NULL );
			
			sDdnsCtrlIns.pGetErr(&ret);
		}
	}
	else
	{
		ret = 0;
		goto END;
	}
	
	if(0 == ret)
	{
		printf("DDNS_SvcInit ok!\n");
		sDdnsCtrlIns.bSvcInit = 1;
		sDdnsCtrlIns.bStop = 1;//cw_ddns
	}
	else
	{
		printf("DDNS_SvcInit failed!\n");
	}
	
END:
	PRINT(ret);
	
	return ret;
}

// regist
s32 DDNS_SvcRegist( EM_DDNS_PROT eProt, SDDNSRegPara* pRegPara )
{
	s32 err = 0;
	
	if(sDdnsCtrlIns.bRegist)
	{
		err = -1;
		goto END;
	}
	
	if(sDdnsCtrlIns.eCurProt==eProt)
	{		
		if(sDdnsCtrlIns.pReg) sDdnsCtrlIns.pReg(&pRegPara->sBase);
		if(sDdnsCtrlIns.pGetErr) sDdnsCtrlIns.pGetErr(&err);
	}
	else
	{
		err = -2;
		goto END;
	}
	
END:
	PRINT(err);
	
	if(!err)
		sDdnsCtrlIns.bRegist = 1;
	
	return err;
}

// unreg
s32 DDNS_SvcUnreg( EM_DDNS_PROT eProt, SDDNSUnRegPara* pUnRegPara )
{
	if(sDdnsCtrlIns.eCurProt==eProt)
	{
		if(sDdnsCtrlIns.pUnreg)
			sDdnsCtrlIns.pUnreg( &pUnRegPara->sBase );
	}
	else
	{
		PRINT(-1);
		
		return -1;
	}

	sDdnsCtrlIns.bRegist = 0;
	
	PRINT(0);
	
	return 0;
}

// start
s32 DDNS_SvcStart( EM_DDNS_PROT eProt, SDDNSStartPara* pStartPara )
{
	s32 err = 0;
	
	if(!sDdnsCtrlIns.bSvcInit)
	{
		err = -1;
		goto END;
	}
	
	sDdnsCtrlIns.bStop = 1;
	
	if(sDdnsCtrlIns.eCurProt==eProt)
	{
		if(pStartPara->bTry && eProt == EM_DDNS_PROT_TL)
		{
			if(sDdnsCtrlIns.pStartTry) sDdnsCtrlIns.pStartTry(&pStartPara->sBase);
		}
		else
		{
			if(sDdnsCtrlIns.pStart) sDdnsCtrlIns.pStart(&pStartPara->sBase);
		}
		
		sDdnsCtrlIns.pGetErr(&err);
		//if(0 == err)//csp modify 20130724
		{
			strcpy(sDdnsCtrlIns.szDomain, pStartPara->sBase.szDomain);
			strcpy(sDdnsCtrlIns.szUser, pStartPara->sBase.szUser);
			strcpy(sDdnsCtrlIns.szPasswd, pStartPara->sBase.szPasswd);
		}
		
		//csp modify 20130724
		sDdnsCtrlIns.bStop = 0;
		return 0;
	}
	else
	{
		err = -2;
		goto END;
	}
	
END:
	PRINT(err);
	
	if(0 == err)
	{
		sDdnsCtrlIns.bStop = 0;
	}
	
	return err;
}

// stop
s32 DDNS_SvcStop( EM_DDNS_PROT eProt, SDDNSStopPara* pStopPara )
{
	s32 err = 0;
	
	if(sDdnsCtrlIns.bStop) return 0;
	
	if(sDdnsCtrlIns.eCurProt == eProt)
	{
		sDdnsCtrlIns.bStop = 1;
		
		if(sDdnsCtrlIns.pStop)
			sDdnsCtrlIns.pStop( &pStopPara->sBase );
		
		sDdnsCtrlIns.pGetErr(&err);
	}
	else
	{
		err = -1;
		
		goto END;
	}
	
END:
	PRINT(err);
	return err;
}

// refresh
s32 DDNS_SvcRefresh( EM_DDNS_PROT eProt, SDDNSRegPara* pRefreshPara )
{
	if(sDdnsCtrlIns.eCurProt==eProt)
	{
		;//sDdnsCtrlIns.pRefresh( pRefreshPara );
		DDNS_NotifyUpdate(0);
	}
	else
	{
		PRINT(-1);
		
		return -1;
	}
	
	PRINT(0);
	
	return 0;
}

// deinit
s32 DDNS_SvcDeInit(  EM_DDNS_PROT eProt )
{
	if(sDdnsCtrlIns.eCurProt==eProt)
	{
		sDdnsCtrlIns.pDeInit();
	}
	else
	{
		PRINT(-1);
		return -1;
	}
	
	sDdnsCtrlIns.bSvcInit = 0;
	
	PRINT(0);
	
	return 0;
}

// IP刷新线程同步
static int 				nDdnsFd[2] 	= {-1, -1};
static pthread_mutex_t 	mxDdns 		= PTHREAD_MUTEX_INITIALIZER;

void DDNS_NotifyUpdate(u8 key)
{
	pthread_mutex_lock(&mxDdns);
	
	write(nDdnsFd[1], &key, sizeof(key));
	
	pthread_mutex_unlock(&mxDdns);
	
	printf("DDNS_NotifyUpdate key %d\n", key);
}

// IP刷新线程
void *ip_update(void* param)
{
	struct timeval 	tv;
	fd_set 			sFdSet;
	int 			ret;
	s32 			nErr;
	u8 				nKbuf, bRefresh = 0;
	int 			nCheckDelay = 30;
	SDDNSCtrl 		*pCtrl = &sDdnsCtrlIns;
	
	//csp modify 20130322
	bRefresh 		= 1;
	time_t			last_update_time = time(NULL);
	
	if(pipe(nDdnsFd) < 0)
	{
		return NULL;
	}
	
	printf("$$$$$$$$$$$$$$$$$$ip_update thread id:%d, interval:%d\n", getpid(), pCtrl->nIntvl);
	
	while(1)
	{
		//wait pipe input to check & update ip
		
		FD_ZERO(&sFdSet);
		FD_SET(nDdnsFd[0], &sFdSet);
		
		if(pCtrl->nConnFlag)
		{
			tv.tv_sec = 5;
		}
		else
		{
			//csp modify 20130322
			if(nCheckDelay == 0)
			{
				nCheckDelay = 5;
			}
			
			tv.tv_sec = nCheckDelay;//30+pCtrl->nIntvl*60;
		}
		
		tv.tv_usec = 0;
		
		//printf("ip_update tv.tv_sec:%ld,nCheckDelay:%d\n",tv.tv_sec,nCheckDelay);
		
		ret = select(nDdnsFd[0] + 1, &sFdSet, NULL, NULL, &tv);
		if(ret > 0)
		{
			if(FD_ISSET(nDdnsFd[0], &sFdSet))
			{
				read(nDdnsFd[0], &nKbuf, sizeof(u8));
			}
			
			bRefresh = 1;//csp modify 20130322
		}
		else if(ret == 0)
		{
			#if 1//csp modify 20130322
			if(pCtrl->eCurProt != EM_DDNS_PROT_JMDVR)
			{
				//printf("ddns[%d] select timeout, refresh imediately\n",pCtrl->eCurProt);
				bRefresh = 1;
			}
			else
			{
				time_t curr = time(NULL);
				if(abs(curr-last_update_time) > 3600*24)//大于一天
				{
					printf("jmv ddns select timeout, refresh imediately\n");
					bRefresh = 1;
				}
				else
				{
					//printf("jmv ddns select timeout, Not refresh\n");
				}
			}
			#else
			printf("ddns select timeout, refresh imediately\n");
			bRefresh = 1;
			#endif
		}
		
		//printf("ip_update CheckNetLink:%d,bStop:%d,bSvcInit:%d\n",CheckNetLink(),pCtrl->bStop,pCtrl->bSvcInit);
		
		if(pCtrl->bStop || !pCtrl->bSvcInit)
		{
			continue;
		}
		
		//printf("ddns active... prot=%d,init=%d,stop=%d\n", pCtrl->eCurProt,pCtrl->bSvcInit,pCtrl->bStop);
		
		if(pCtrl->pRefresh && pCtrl->pGetErr)
		{
			//printf("before check_ip_tl...\n");
			
			#if 1//csp modify 20130322
			if(0 != check_ip_tl(&nCheckDelay) || bRefresh)
			#else
			if(1 == check_ip_tl(&nCheckDelay) || bRefresh)
			#endif
			{
				SDDNSRegPara sRefreshIns;
				memset(&sRefreshIns, 0, sizeof(sRefreshIns));
				
				strcpy(sRefreshIns.sBase.szDomain, pCtrl->szDomain);
				strcpy(sRefreshIns.sBase.szUser, pCtrl->szUser);
				strcpy(sRefreshIns.sBase.szPasswd, pCtrl->szPasswd);
				
				pCtrl->pRefresh(&sRefreshIns);
				pCtrl->pGetErr(&nErr);
				if(nErr == 0)
				{
					pCtrl->nConnFlag = 0;
					pCtrl->bActive = 1;
					
					//printf("update ip success!\n");
					
					//csp modify 20130322
					last_update_time = time(NULL);
					bRefresh = 0;
				}
				else
				{
					pCtrl->nConnFlag = 1;
					
					//printf("update ip failed!\n");
					
					//csp modify 20130322
					bRefresh = 1;
				}
				
				//csp modify 20130322
				//bRefresh = 0;
				
				//printf("ddns ip refresh!!!------------------------------------\n");
			}
		}
		else
		{
			// if ddns-flag not active, if last-active , stop
			if(pCtrl->bActive)
			{
				SDDNSStopPara sStopIns;
				memset(&sStopIns, 0, sizeof(sStopIns));
				
				strcpy(sStopIns.sBase.szDomain, pCtrl->szDomain);
				strcpy(sStopIns.sBase.szUser, pCtrl->szUser);
				strcpy(sStopIns.sBase.szPasswd, pCtrl->szPasswd);
				
				if(pCtrl->pStop)//csp modify 20130322
				{
					pCtrl->pStop(&sStopIns);
				}
				
				if(pCtrl->pGetErr)//csp modify 20130322
				{
					// if not error , change status
					pCtrl->pGetErr(&nErr);		
					if(0 == nErr)
					{
						pCtrl->bActive = 0;
					}
				}
				
				//printf("ddns stop!!!------------------------------------\n");
			}
			
			//printf("ip_update not act!\n");
		}
	}
	
	return NULL;
}

