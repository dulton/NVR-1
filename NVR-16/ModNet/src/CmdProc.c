#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// file description
#include "CmdProc.h"
#include "Cmdcallback.h"
#include "diskmanage.h"
#include "netcommon.h"
#include "biz.h"


//#include "common_geometric.h"
//#include "biz_types.h"
typedef struct
{
	int nValue;
	char strDisplay[32];
} SValue;


extern void GetDWellSwitchTimeList( SValue* psValueList, int* nRealNum, int nMaxNum);
extern void GetDWellSwitchPictureList( SValue* psValueList, int* nRealNum, int nMaxNum);


//** macro   
#define SUPPORT_LANGUAGE_NUM		20
#define CMD_CB_MAX					128

//** typedef 

typedef enum						//return value from UserCheckLoginPara()
{
	EM_MODUSER_LOGIN_ERR_NOTHISUSER = -2,
	EM_MODUSER_LOGIN_ERR_WRONGPASSWD = -3,
	EM_MODUSER_LOGIN_ERR_NOLOGINTWINCE = -4,
	EM_MODUSER_LOGIN_ERR_IP_NOPERMIT = -5,
	EM_MODUSER_LOGIN_ERR_MAC_NOPERMIT = -6,
	//
} EMMODUSERLOGINERR;

//** local var 

static PNetCommCommandDeal2 	pCB2;
static u8	 					nChnMax;
static SCmdCBPair 				sCmdCBStack[CMD_CB_MAX]={{0,0}};

#define NET_USER_MAX	128

typedef struct 
{
	u8 			nLoginUser;
	SNetUser 	User[NET_USER_MAX];
} SNetUserMgr;

static SNetUserMgr sNetUserMgr;


//** global var

extern SNetCommCtrl sNetCommCtrl;
extern ifly_AlarmUploadCenter_t g_AlarmUploadCenter[MAX_ALARM_UPLOAD_NUM];


//** local functions

//** global functions

s32 remotePlay_Stop(u32 nLnkId);
s32 remotePlay_FastPlay(u32 id);
s32 remotePlay_SlowPlay(u32 id);
s32 remotePlay_SetRate(u32 id, int rate);
s32 remotePlay_Pause(u32 id);
s32 remotePlay_Resume(u32 id);
s32 remotePlay_Step(u32 id);
s32 remotePlay_PrevNext(u32 id, u8 bPrev);
s32 remotePlay_Seek(u32 id, u32 nSeekPos);
s32 remotePlay_Mute(u32 id, u8 bMute);
s32 remoteVoip_Stop(u32 nLinkId);
s32 remotePlay_Progress(u32 id, CPHandle cph, u8 bStatus);

s32 netComm_PPPOE_CmdPreup();
s32 netComm_PPPOE_CmdUp();
s32 netComm_PPPOE_CmdDown();
s32 netComm_PPPOE_CmdDisconnect();
s32 netComm_DHCP_Active( char* pIFName );
extern void net_write_upload_alarm(u32 key);


void cmdPro_Init( PNetCommCommandDeal2 p, u8 nChMax )
{
	pCB2 = p;
	nChnMax = nChMax;
	
	memset(&sNetUserMgr, 0, sizeof(sNetUserMgr));
}

// find cb
PNetCommCommandDeal cmdProc_GetCmdCB(u32 nCmdID)
{
	int i;

	SCmdCBPair* pCmdPair = sCmdCBStack;
			
	for(i=0;i<CMD_CB_MAX;i++)
	{
		if(pCmdPair[i].nCmdID==nCmdID)
		{
			//NETCOMM_DEBUG_STR("Get callback, add ", (u32)pCmdPair[i].pCB);
			
			return pCmdPair[i].pCB;
		}
	}

	return NULL;
}

// NetCommUnregCmdCB
void NetCommUnregCmdCB(u32 nCmdID)
{
	int i;

	SCmdCBPair* pCmdPair = sCmdCBStack;
	
	for(i=0;i<CMD_CB_MAX;i++)
	{
		if(pCmdPair[i].nCmdID==nCmdID)
		{
			pCmdPair[i].nCmdID 	= 0;
			pCmdPair[i].pCB		= NULL;
			
			break;
		}
	}
}

// NetCommRegCmdCB
s32 NetCommRegCmdCB( u32 nCmdID, PNetCommCommandDeal pCB )
{
	int i;

	SCmdCBPair* pCmdPair = sCmdCBStack;
	
	for(i=0;i<CMD_CB_MAX;i++)
	{
		if(pCmdPair[i].nCmdID==0)
		{
			//NETCOMM_DEBUG_STR("Register on callback, id", nCmdID);
			
			pCmdPair[i].nCmdID 	= nCmdID;
			pCmdPair[i].pCB		= pCB;

			break;
		}
	}

	if(i==CMD_CB_MAX)
	{
		NETCOMM_DEBUG_STR("No more position for cmd call back register!!!\n", -1);

		return -1;
	}
	
	return 0;
}

extern u16 DealPlayProgress( 
	CPHandle cph,
	u16 event,
	u8 *pbyMsgBuf,
	int msgLen,
	u8 *pbyAckBuf,
	int *pAckLen,
	void* pContext
);

typedef struct
{
	char	name[12];
	char	password[12];
	char	local_privilege[16];
	char	remote_privilege[16];
	char	mac_address[18];
	u16		startID;
}PACK_NO_PADDING ifly_usermanage_t;

//csp modify 20130423
extern s32 remotePreview_SetMonitorInfo(u32 nLinkId, u32 MonitorInfo);

extern s32 remotePreview_StopPreview(u32 nLinkId, u8 bVideo);
extern s32 remotePlay_Stop(u32 nLnkId);

s32 cmdProc_GetLoginUser(SNetUser* pUser)
{
	int i;
	
	if(pUser)
	{
		for(i=0; i<NET_USER_MAX; i++)
		{
			if(sNetUserMgr.User[i].bOnline)
			{
				//printf("ip %lu, sock %d\n", pUser->ip, pUser->sock);				
			}
			
			#if 1//csp modify
			if( sNetUserMgr.User[i].bOnline &&
				( sNetUserMgr.User[i].ip == pUser->ip &&
				  sNetUserMgr.User[i].sock == pUser->sock )
			)
			#else
			if( sNetUserMgr.User[i].bOnline &&
				( sNetUserMgr.User[i].ip == pUser->ip ||
				  sNetUserMgr.User[i].sock == pUser->sock )
			)
			#endif
			{
				strcpy(pUser->name, sNetUserMgr.User[i].name);
				pUser->id = sNetUserMgr.User[i].id;
				printf("get user:(%s,%d), index:%d, net info:(0x%08x, %d)\n", pUser->name, pUser->id, i, pUser->ip, pUser->sock);
				break;
			}
		}
		
		if(i == NET_USER_MAX)
		{
			return -1;
		}
	}
	
	return 0;
}

s32 cmdProc_AddNetUser(SNetUser* pUser)
{
	int i, iUsrIdx = -1;
	
	if(pUser)
	{
		for(i=0; i<NET_USER_MAX; i++)
		{
			if(iUsrIdx < 0 && !sNetUserMgr.User[i].bOnline)
			{
				iUsrIdx = i;
			}
			
			if(sNetUserMgr.User[i].bOnline &&
				sNetUserMgr.User[i].ip == pUser->ip &&
				sNetUserMgr.User[i].sock == pUser->sock
				#if 0
				//Modify by Lirl on Dec/29/2011
				//登陆时只要为登陆状态,并且再记录下当前用户状态信息
				pUser->bOnline
				#endif
			)
			{
				printf("add user %s\n", pUser->name);
				
				//IE登陆时记录当前用户的状态信息
				strcpy(sNetUserMgr.User[i].name, pUser->name);
				sNetUserMgr.User[i].id = pUser->id;
				sNetUserMgr.User[i].bOnline = pUser->bOnline;
				sNetUserMgr.User[i].ip = pUser->ip;
				sNetUserMgr.User[i].sock = pUser->sock;
				
				return 0;
			}
		}
		
		if((i == NET_USER_MAX) && (iUsrIdx >= 0 && iUsrIdx < NET_USER_MAX))
		{
			printf("add user:(%s,%d), index:%d, net info:(0x%08x, %d)\n", pUser->name, pUser->id, iUsrIdx, pUser->ip, pUser->sock);
			
			memcpy(&sNetUserMgr.User[iUsrIdx], pUser, sizeof(SNetUser));
		}
	}
	
	return 0;
}

s32 cmdProc_DelNetUser(SNetUser* pUser)
{
	int i;
	
	if(pUser)
	{
		if(pUser->bUserDel)
		{
			for(i=0; i<NET_USER_MAX; i++)
			{
				//用户管理删除该用户则删除在线用户信息
				if(strcmp(sNetUserMgr.User[i].name, pUser->name) == 0)
				{
					memset(&sNetUserMgr.User[i], 0, sizeof(SNetUser));
					sNetUserMgr.User[i].bOnline = 0;
					sNetUserMgr.User[i].ip = 0;
					sNetUserMgr.User[i].sock = 0;
				}
			}
		}
		else
		{
			for(i=0; i<NET_USER_MAX; i++)
			{
				if(sNetUserMgr.User[i].bOnline &&
					strcmp(sNetUserMgr.User[i].name, pUser->name) == 0 &&
					sNetUserMgr.User[i].ip == pUser->ip &&
					sNetUserMgr.User[i].sock == pUser->sock
				)
				{
					//IE退出时删除当前用户的状态信息
					memset(&sNetUserMgr.User[i], 0, sizeof(SNetUser));
					sNetUserMgr.User[i].bOnline = 0;
					sNetUserMgr.User[i].ip = 0;
					sNetUserMgr.User[i].sock = 0;
					break;
				}
			}
		}
	}
	
	return 0;
}

//xdc
ifly_ipc_info_t sIpcInfo1[100];
static u16 second = 0;
static u16 startid = 0;
static u16 endid = 0;
static u16 ipc_num = 0;
static u16 max_return = 0;
//xdc end
u16 DealCommand(
	CPHandle 	cph,
	u16 		event,
	u8*			pbyMsgBuf,
	int 		msgLen,
	u8*			pbyAckBuf,
	int*		pAckLen,
	void* 		pContext
)
{
	//printf("event %d\n",event);
	u8 byMute;
	u8 byStatus;
	int i, k, id, rate, seekPos;
	//char temp[2] = {0};
	
	PNetCommCommandDeal pCB = NULL;
	
	SNetCommCtrl* pNetCtrl = &sNetCommCtrl;
	SNetLog		  sNLIns;
	
	// set or get paramters to/from uplayer
	//			
	SRemoteCmdReq sReqIns;
	SRemoteCmdRslt sRslt;
	
	memset(&sReqIns, 0, sizeof(sReqIns));
	memset(&sRslt, 0, sizeof(sRslt));
	
	pCB = cmdProc_GetCmdCB(event);
	if(NULL == pCB)
	{
		if (event != CTRL_NOTIFY_CONNLOST)//yaogang modify for server heart beat check
		{
			NETCOMM_DEBUG_STR("No matched callback function registered!!!\n", -1);
			
			//csp modify 20130527
			if(pAckLen)
			{
				*pAckLen = 0;
			}
			
			//csp modify 20130527
			return CTRL_FAILED_COMMAND;//return 0;//
		}
	}
	
	if(cph)
	{
		sReqIns.cph = cph;
	}
	else
	{
		//csp modify 20130527
		if(pAckLen)
		{
			*pAckLen = 0;
		}
		
		//csp modify 20130527
		return CTRL_FAILED_CONNECT;//return -1;
	}
	
	switch(event)
	{
		case CTRL_CMD_GETRECSCHPARAMBYTYPE://  CTRL_CMD_BASE+133 //按布防类型获得录像布防参数
		{
			ifly_RecSchTime_t* pRecordSCH;
			
			pRecordSCH = &sReqIns.sReq.RecSchTime;
			memcpy(pRecordSCH, pbyMsgBuf, sizeof(ifly_RecSchTime_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_RecSchTime_t);
			}
		
			memcpy(pbyAckBuf, &sRslt.sBasicInfo.RecSchTime, sizeof(ifly_RecSchTime_t));
			
			//printf("acklen %d \n", *pAckLen);
			
		} break;
		case CTRL_CMD_SETRECSCHPARAMBYTYPE: // CTRL_CMD_BASE+134 //按布防类型分开设置录像布防参数
		{
			ifly_RecSchTime_t* pRecordSCH;
			
			pRecordSCH = &sReqIns.sReq.RecSchTime;
			memcpy(pRecordSCH, pbyMsgBuf, sizeof(ifly_RecSchTime_t));

			pRecordSCH->copy2Chnmask = ntohl(pRecordSCH->copy2Chnmask);
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = 0;
			}
		} break;
		case CTRL_CMD_GETPTZPROTOCOLLIST:
		{
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_PTZProtocolList_t);
			}

			memcpy(pbyAckBuf, &sRslt.sBasicInfo.PTZProtocolList, sizeof(ifly_PTZProtocolList_t));
			
			//printf("CTRL_CMD_GETPTZPROTOCOLLIST acklen %d \n", *pAckLen);
		}
		break;
		case CTRL_CMD_GETMDSENSELIST: // CTRL_CMD_BASE+129 //获取移动侦测灵敏度列表
		{
			(*pCB)(&sReqIns, &sRslt); 
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_MDSenselist_t);
			}

			memcpy(pbyAckBuf, &sRslt.sBasicInfo.MDSenselist, sizeof(ifly_MDSenselist_t));
		} break;
		case CTRL_CMD_GETMDALARMDELAYLIST: //   CTRL_CMD_BASE+130 //获取移动侦测报警输出延时列表
		{
			(*pCB)(&sReqIns, &sRslt); 
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_MDAlarmDelaylist_t);
			}
			
			memcpy(pbyAckBuf, &sRslt.sBasicInfo.MDAlarmDelaylist, sizeof(ifly_MDAlarmDelaylist_t));
		} break;
		case CTRL_CMD_GETBAUDRATELIST: //  CTRL_CMD_BASE+131 //获取波特率列表
		{
			(*pCB)(&sReqIns, &sRslt); 
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_BaudRateList_t);
			}
			
			memcpy(pbyAckBuf, &sRslt.sBasicInfo.BaudRateList, sizeof(ifly_BaudRateList_t));
		} break;
		case CTRL_CMD_STOPVOIP:
		{
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			*pAckLen = 0;
			printf("RECV CMD:STOPVOIP id=%d...\n",id);
			return remoteVoip_Stop(id);
		}
		break;
		case CTRL_CMD_LOGIN://		CTRL_CMD_BASE+1	//远程登录命令
		{
			ifly_link_online 	links;
			ifly_loginpara_t* 	pLoginInfo;
			
			memset(&links,0,sizeof(links));
			
			pLoginInfo = &sReqIns.sReq.loginpara;
			memcpy(pLoginInfo,pbyMsgBuf,sizeof(ifly_loginpara_t));
			
			(*pCB)(&sReqIns, &sRslt);  // Get user info from up-layer
			
			if(sRslt.nErrCode != 0)
			{
				if(pAckLen)
				{
					*pAckLen = 0;
				}
				
				//u32 key = (8 << 16) | (0 << 8) | 1;
				u32 key = (10 << 16) | (0 << 8) | 1;
				upload_alarm(key);
				
				switch(sRslt.nErrCode)
				{
					case EM_MODUSER_LOGIN_ERR_WRONGPASSWD:
					{
						return CTRL_FAILED_PASSWORD;
					} break;
					case EM_MODUSER_LOGIN_ERR_MAC_NOPERMIT:
					{
						return CTRL_FAILED_MACADDR;
					} break;
					default:
					{
						return CTRL_FAILED_PARAM;
					} break;
				}
			}
			
			//u32 key = (8 << 16) | (0 << 8) | 0;
			u32 key = (10 << 16) | (0 << 8) | 0;
			upload_alarm(key);
			
			SRemoteUserInfo*	pUserInfo = &sRslt.sBasicInfo.sUserInfo;
			
			links.link_online = 1;
			memcpy(pbyAckBuf,&links,sizeof(links));
			
			ifly_remusr_limit quanxian;
			strcpy( quanxian.usrname, pLoginInfo->username ); 
			
			#if 1
			memcpy( quanxian.remote_privilege, 
					pUserInfo->szPrivilege, 
					sizeof(quanxian.remote_privilege)/sizeof(quanxian.remote_privilege[0]) );
			#else
			memset(quanxian.remote_privilege, '1', 16);
			#endif
			memcpy(pbyAckBuf+sizeof(links),&quanxian,sizeof(ifly_remusr_limit));
			
			if(pAckLen)
			{
				*pAckLen = sizeof(links)+sizeof(ifly_remusr_limit);
			}
		}
		break;
		case CTRL_CMD_LOGOFF://					CTRL_CMD_BASE+2			//注销用户登录
		{
			printf("CTRL_CMD_LOGOFF\n");
			
			ifly_loginpara_t* pLoginInfo;
			
			pLoginInfo = &sReqIns.sReq.loginpara;
			
			memcpy(pLoginInfo, pbyMsgBuf, sizeof(ifly_loginpara_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = 0;
			}		
		}
		break;
		case CTRL_CMD_GETDEVICEINFO://			CTRL_CMD_BASE+3			//获得设备信息
		{
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_DeviceInfo_t);
			}
			
			(*pCB)(&sReqIns, &sRslt);
			
			memcpy(pbyAckBuf,&sRslt.sBasicInfo.DeviceInfo,sizeof(ifly_DeviceInfo_t));
			
			printf("get device info deviceIP:%x,devicePort:%d,device_name:%s,device_mode:%s,chn=%d,maxAlarmInNum:%d\n",
				sRslt.sBasicInfo.DeviceInfo.deviceIP,
				ntohs(sRslt.sBasicInfo.DeviceInfo.devicePort),
				sRslt.sBasicInfo.DeviceInfo.device_name,
				sRslt.sBasicInfo.DeviceInfo.device_mode,
				sRslt.sBasicInfo.DeviceInfo.maxChnNum,
				sRslt.sBasicInfo.DeviceInfo.maxAlarmInNum);
		}
		break;
		case CTRL_CMD_GETVIDEOPROPERTY://		CTRL_CMD_BASE+4			//视频属性信息
		{
			SNetCommCtrl* pNetCtrl = &sNetCommCtrl;
			ifly_Video_Property_t videoproperty;
			videoproperty.videoEncType = 98;
			//csp modify
			//videoproperty.max_videowidth = htons(704);
			//videoproperty.max_videoheight = htons(576);
			videoproperty.max_videowidth = htons(pNetCtrl->sCommonCfg.nVideoMaxWidth);
			videoproperty.max_videoheight = htons(pNetCtrl->sCommonCfg.nVideoMaxHeight);
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_Video_Property_t);
			}
			memcpy(pbyAckBuf,&videoproperty,sizeof(ifly_Video_Property_t));
			printf("here,get video property:(%d,%d)\n",ntohs(videoproperty.max_videowidth),ntohs(videoproperty.max_videoheight));
		}
		break;
		case CTRL_CMD_GETAUDIOPROPERTY://		CTRL_CMD_BASE+5			//音频属性信息
		{
			ifly_Audio_Property_t audioproperty;
			audioproperty.audioBitPerSample = 16;
			audioproperty.audioSamplePerSec = htons(8000);
			#if 0//csp modify
			audioproperty.audioFrameSize = htons(640);
			#else
			audioproperty.audioFrameSize = htons(642);
			#endif
			audioproperty.audioEnctype = 0;//19;//MEDIA_TYPE_PCMU;
			#if 0//csp modify
			audioproperty.audioFrameDurTime = htons(40);
			#else
			audioproperty.audioFrameDurTime = htons(40);
			#endif
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_Audio_Property_t);
			}
			memcpy(pbyAckBuf,&audioproperty,sizeof(ifly_Audio_Property_t));
			printf("get audio property#################\n");
		}
		break;
		case CTRL_CMD_GETVOIPPROPERTY://		CTRL_CMD_BASE+6			//voip属性信息
		{
			ifly_VOIP_Property_t voipproperty;
			voipproperty.VOIPBitPerSample = 16;
			#if 0//csp modify
			voipproperty.VOIPFrameSize = htons(640);
			#else
			voipproperty.VOIPFrameSize = htons(642);
			#endif
			voipproperty.VOIPSamplePerSec = htons(8000);
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_VOIP_Property_t);
			}
			memcpy(pbyAckBuf,&voipproperty,sizeof(ifly_VOIP_Property_t));
			//printf("get voip property\n");
		}
		break;
		case CTRL_CMD_GETMDPROPERTY://			CTRL_CMD_BASE+7			//移动侦测属性信息
		{
 			ifly_MD_Property_t mdproperty;
 			mdproperty.MDRow = 18;
 			mdproperty.MDCol = 22;
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_MD_Property_t);
			}
			memcpy(pbyAckBuf,&mdproperty,sizeof(ifly_MD_Property_t));
			//printf("get md property\n");
		}
		break;
		//csp modify 20130519
		case CTRL_CMD_GETADVPRIVILEGE:
		{
			ifly_loginpara_t* pLoginInfo = &sReqIns.sReq.loginpara;
			memcpy(pLoginInfo,pbyMsgBuf,sizeof(ifly_loginpara_t));
			
			ifly_AdvPrivilege_t *pAdvPrivilege = &sRslt.sBasicInfo.AdvPrivilege;
			memset(pAdvPrivilege, 0, sizeof(ifly_AdvPrivilege_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_AdvPrivilege_t);
			}
			memcpy(pbyAckBuf, pAdvPrivilege, sizeof(ifly_AdvPrivilege_t));
			
			printf("get user[%s] advanced privilege#################\n",pLoginInfo->username);
		}
		break;
		#if 1
		case CTRL_CMD_GETUSERINFO:
		{
			ifly_userNumber_t* pUsernumInfo;
			
			pUsernumInfo = &sRslt.sBasicInfo.userNumber;
			memset(pUsernumInfo, 0, sizeof(ifly_userNumber_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_userNumber_t);
			}
			memcpy(pbyAckBuf, pUsernumInfo, sizeof(ifly_userNumber_t));
			
			//printf("get user info!\n");
		}
		break;
		#else
		case CTRL_CMD_GETUSERINFO:
		{
			ifly_userNumber_t* pUsernumInfo;
			
			pUsernumInfo = &sRslt.sBasicInfo.userNumber;
			memset(pUsernumInfo, 0, sizeof(ifly_userNumber_t));
			
			ifly_usermanage_t tem_user;
			
			memset(tem_user.local_privilege, '1', sizeof(tem_user.local_privilege));
			memset(tem_user.remote_privilege, '1', sizeof(tem_user.remote_privilege));
			
			for(i=0;i<1;++i)
			{
				//GetUserInfo(&tem_user, i);
				strcpy(pUsernumInfo->userNum[i].name,"admin");//tem_user.name,sizeof(pUsernumInfo->userNum[i].name));
				strcpy(pUsernumInfo->userNum[i].passwd,"");//tem_user.password,sizeof(pUsernumInfo->userNum[i].passwd));
				strcpy(pUsernumInfo->userNum[i].macaddr,"");//tem_user.mac_address,sizeof(pUsernumInfo->userNum[i].macaddr));
				
				pUsernumInfo->userNum[i].lcamer = tem_user.local_privilege[0] -48;
				pUsernumInfo->userNum[i].lrec = tem_user.local_privilege[1] -48;
				pUsernumInfo->userNum[i].lplay = tem_user.local_privilege[2] -48;
				pUsernumInfo->userNum[i].lsetpara = tem_user.local_privilege[3] -48;
				pUsernumInfo->userNum[i].llog = tem_user.local_privilege[4] -48;
				pUsernumInfo->userNum[i].ltool = tem_user.local_privilege[5] -48;
				
				pUsernumInfo->userNum[i].rcamer = tem_user.remote_privilege[0] -48;
				pUsernumInfo->userNum[i].rrec = tem_user.remote_privilege[1] -48;
				pUsernumInfo->userNum[i].rplay = tem_user.remote_privilege[2] -48;
				pUsernumInfo->userNum[i].rsetpara = tem_user.remote_privilege[3] -48;
				pUsernumInfo->userNum[i].rlog = tem_user.remote_privilege[4] -48;
				pUsernumInfo->userNum[i].rtool = tem_user.remote_privilege[5] -48;
				pUsernumInfo->userNum[i].rpreview = tem_user.remote_privilege[6] -48;
				pUsernumInfo->userNum[i].ralarm = tem_user.remote_privilege[7] -48;
				pUsernumInfo->userNum[i].rvoip = tem_user.remote_privilege[8] -48;
				//strncpy(pUsernumInfo->userNum[i].local_privilege,tem_user.local_privilege,sizeof(pUsernumInfo->userNum[i].local_privilege));
				//strncpy(pUsernumInfo->userNum[i].remote_privilege,tem_user.remote_privilege,sizeof(pUsernumInfo->userNum[i].remote_privilege));
			}
			
			//(*pCB)(NULL, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_userNumber_t);
			}
			memcpy(pbyAckBuf, pUsernumInfo, sizeof(ifly_userNumber_t));
			
			//printf("get user info!\n");
		}
		break;
		#endif
		case CTRL_CMD_GETSYSTIME:
		{
			ifly_sysTime_t* pGetsystemtime;
			
			pGetsystemtime = &sRslt.sBasicInfo.sysTime;
			
			memset(pGetsystemtime, 0, sizeof(ifly_sysTime_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_sysTime_t);
			}
			//printf("get systime time:%x!\n", pGetsystemtime->systemtime);
			
			pGetsystemtime->systemtime = htonl(pGetsystemtime->systemtime);
			
			memcpy(pbyAckBuf, pGetsystemtime, sizeof(ifly_sysTime_t));
		}
		break;		
	#if 1
		case CTRL_CMD_REBOOT:
		case CTRL_CMD_SETRESTORE:
		case CTRL_CMD_CLEARALARM:
		case CTRL_CMD_SHUTDOWN:
		{
			if(pAckLen) *pAckLen = 0;
			
			//printf("ctrl cmd %d\n", event); 
			(*pCB)(&sReqIns, &sRslt);
			
			//printf("ctrl cmd %d\n", event); 
		}
		break;
		case CTRL_CMD_STOPAUDIOMONITOR:
		{
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			if(pAckLen) *pAckLen = 0;
			printf("RECV CMD:STOPAUDIOMONITOR...\n");
			return remotePreview_StopPreview(id, 0);
		}
		break;
		case CTRL_CMD_STOPVIDEOMONITOR:
		{
			#define CONN_LOST_STOP	1
			if(*(pbyMsgBuf+4) != CONN_LOST_STOP)//???
			{
				memcpy(&id,pbyMsgBuf,sizeof(u32));
				id = ntohl(id);
				if(pAckLen) *pAckLen = 0;
				//printf("RECV CMD:STOPVIDEOMONITOR id=%d...\n",id);
				return remotePreview_StopPreview(id, 1);
			}
			else
			{
				if(pAckLen) *pAckLen = 0;
			}
		}
		break;
		//csp modify 20130423
		case CTRL_CMD_SETMONITORINFO:
		{
			ifly_wndinfo_t wndinfo;
			memcpy(&wndinfo,pbyMsgBuf,sizeof(wndinfo));
			
			id = ntohl(wndinfo.id);
			
			u32 MonitorInfo = wndinfo.wnd_num;//wndinfo.wnd_num | (wndinfo.wnd_index << 8);
			
			//printf("RECV CMD:CTRL_CMD_SETMONITORINFO id=%d MonitorInfo=0x%x\n",id,MonitorInfo);
			
			if(pAckLen) *pAckLen = 0;
			
			SNetThirdStreamProperty property;
			NetComm_GetThirdStreamProperty(&property);
			
			if(property.support)
			{
				s32 chn = remotePreview_SetMonitorInfo(id, MonitorInfo);//低8位,视频窗口数;高8位,窗口索引
				if(chn == -1)
				{
					return CTRL_FAILED_LINKLIMIT;
				}
			}
			
			return CTRL_SUCCESS;
		}
		break;
		#if 1
		case CTRL_CMD_RECFILESEARCH:
		{
			ifly_recsearch_param_t* pRemotesearch;
			SCPRecFile*	pRecFile;
			
			//sNLIns.nLogId = EM_NETLOG_SEARCH_DATA_BYTIME;
			//pNetCtrl->pWriteLogCB(&sNLIns);
			
			pRecFile = &sRslt.sBasicInfo.sRecFile;
			pRemotesearch = &sReqIns.sReq.recsearch_param;
			
			memset(pRecFile, 0, sizeof(SCPRecFile));
			
			memcpy(pRemotesearch, pbyMsgBuf, sizeof(ifly_recsearch_param_t));
			
			pRemotesearch->channel_mask = ntohs(pRemotesearch->channel_mask);
			pRemotesearch->chn17to32mask= ntohs(pRemotesearch->chn17to32mask);
			pRemotesearch->type_mask 	= ntohs(pRemotesearch->type_mask);
			pRemotesearch->start_time 	= ntohl(pRemotesearch->start_time);
			pRemotesearch->end_time 	= ntohl(pRemotesearch->end_time);
			pRemotesearch->startID 		= ntohs(pRemotesearch->startID);
			pRemotesearch->max_return 	= ntohs(pRemotesearch->max_return);
			
			/*
			//printf("pRemotesearch->channel_mask  %x\n", pRemotesearch->channel_mask );
			//printf("pRemotesearch->type_mask     %x\n", pRemotesearch->type_mask );
			//printf("pRemotesearch->start_time    %d\n", pRemotesearch->start_time );
			//printf("pRemotesearch->end_time      %d\n", pRemotesearch->end_time );
			//printf("pRemotesearch->startID       %d\n", pRemotesearch->startID );
			//printf("pRemotesearch->max_return    %d\n", pRemotesearch->max_return );
			*/
			
			ifly_recfileinfo_t sRecFileInfo[pRemotesearch->max_return];//zlb20111117 去掉原先malloc
			pRecFile->info = sRecFileInfo;
			if(NULL != pRecFile->info)
			{
				(*pCB)(&sReqIns, &sRslt);
				
				for(i=pRecFile->desc.startID; i<=pRecFile->desc.endID; i++)
				{
					ifly_recfileinfo_t* pFinfo = &pRecFile->info[i-pRecFile->desc.startID];
					
					/*
					//printf("for sending: \n");
					//printf("for sending, starttime 	%d\n", pFinfo->start_time);
					//printf("for sending, endtime 	%d\n", pFinfo->end_time);
					//printf("for sending, size 		%d\n", pFinfo->size);
					//printf("for sending, offset 	%d\n", pFinfo->offset);
					*/
					
					pFinfo->start_time 	= 	htonl(pFinfo->start_time);
					pFinfo->end_time 	= 	htonl(pFinfo->end_time);
					pFinfo->size		=	htonl(pFinfo->size);
					pFinfo->offset 		= 	htonl(pFinfo->offset);
					
					memcpy(pbyAckBuf+sizeof(ifly_recfile_desc_t)+(i-pRecFile->desc.startID)*sizeof(ifly_recfileinfo_t),
						pFinfo, sizeof(ifly_recfileinfo_t));
				}
				
				/*
				//printf("for sending, sum 		%d\n", pRecFile->desc.sum);
				//printf("for sending, endID 		%d\n", pRecFile->desc.endID);
				*/
				
				*pAckLen = (pRecFile->desc.endID-pRecFile->desc.startID+1)*sizeof(ifly_recfileinfo_t)+sizeof(ifly_search_desc_t);
				
				pRecFile->desc.sum 		= htons(pRecFile->desc.sum);
				pRecFile->desc.endID 	= htons(pRecFile->desc.endID);
				pRecFile->desc.startID 	= htons(pRecFile->desc.startID);
				
				memcpy(pbyAckBuf, &pRecFile->desc, sizeof(ifly_search_desc_t));
				
				//printf("for sending, pAckLen 		%d\n", *pAckLen);
			}
		}
		break;
		#endif
		case CTRL_CMD_PPPOE_PREUP:
		{
			printf("#################CTRL_CMD_PPPOE_PREUP#################\n");
			netComm_PPPOE_CmdPreup();
		} break;
		case CTRL_CMD_PPPOE_UP:
		{
			printf("#################CTRL_CMD_PPPOE_UP#################\n");
			netComm_PPPOE_CmdUp();
		} break;
		case CTRL_CMD_PPPOE_DOWN:
		{
			printf("#################CTRL_CMD_PPPOE_DOWN#################\n");
			netComm_PPPOE_CmdDown();
		} break;
		case CTRL_CMD_PPPOE_DISCONNECT:
		{
			printf("#################CTRL_CMD_PPPOE_DISCONNECT#################\n");
			netComm_PPPOE_CmdDisconnect();
		} break;
		#if 0
		{
			SCmdPPPOE* pCmd;
			pCmd = &sReqIns.sReq.sCmdPoeIns;
			pCmd->nCmdType = EM_CMD_PPPOE_PREUP+(event-CTRL_CMD_PPPOE_PREUP);
			
			(*pCB)(&sReqIns, NULL);
		}
		break;
		#endif
		//csp modify
		case CTRL_CMD_DHCP_DECONFIG:
		{
			ifly_cp_dhcp_t dhcp;
			memcpy(&dhcp,pbyMsgBuf,sizeof(dhcp));
			printf("#################CTRL_CMD_DHCP_DECONFIG interface:%s\n",dhcp.ifrname);
		}
		break;
		//csp modify
		case CTRL_CMD_DHCP_NAK:
		{
			printf("#################CTRL_CMD_DHCP_NAK\n");
		}
		break;
		case CTRL_CMD_DHCP_BOUND:
		case CTRL_CMD_DHCP_RENEW:
		{
			memcpy(&sReqIns.sReq.cp_dhcp, pbyMsgBuf, sizeof(ifly_cp_dhcp_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			netComm_DHCP_Active(sReqIns.sReq.cp_dhcp.ifrname);
		}
		break;
		case CTRL_CMD_STOPDOWNLOAD:
			break;
		case CTRL_CMD_STOPFILEPLAY:
		case CTRL_CMD_STOPTIMEPLAY:
		{
			sNLIns.nLogId = EM_NETLOG_PB_STOP;
			sNLIns.ip=cph->ip;
			pNetCtrl->pWriteLogCB(&sNLIns);
			
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			if(pAckLen) *pAckLen = 0;
			
			return remotePlay_Stop(id);
		}
		break;
		case CTRL_CMD_FASTPLAY:
		{
			
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote fast play:(id:0x%x)\n",id);
			if(pAckLen) *pAckLen = 0;

			return remotePlay_FastPlay(id);
		}
		break;
		case CTRL_CMD_SLOWPLAY:
		{			
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote slow play:(id:0x%x)\n",id);
			if(pAckLen) *pAckLen = 0;
			
			return remotePlay_SlowPlay(id);
		}
		break;		
		case CTRL_CMD_SETPLAYRATE:
		{
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			if(pAckLen) *pAckLen = 0;
			memcpy(&rate,pbyMsgBuf+sizeof(id),sizeof(rate));
			rate = ntohl(rate);
			
			//printf("remote play set rate:(id:0x%x),%d\n",id,rate);

			return remotePlay_SetRate(id, rate);
		}
		break;
		case CTRL_CMD_PAUSEPLAY:
		{
			sNLIns.nLogId = EM_NETLOG_PB_PAUSE;
			sNLIns.ip=cph->ip;
			pNetCtrl->pWriteLogCB(&sNLIns);

			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote pause play:(id:0x%x)\n",id);
			if(pAckLen) *pAckLen = 0;

			return remotePlay_Pause(id);
		}
		break;
		case CTRL_CMD_RESUMEPLAY:
		{
			sNLIns.nLogId = EM_NETLOG_PB_RESUME;
			sNLIns.ip=cph->ip;
			pNetCtrl->pWriteLogCB(&sNLIns);
			
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote resume play:(id:0x%x)\n",id);
			if(pAckLen) *pAckLen = 0;

			return remotePlay_Resume(id);
		}
		break;
		case CTRL_CMD_SINGLEPLAY:
		{
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote sing play:(id:0x%x)\n",id);
			if(pAckLen) *pAckLen = 0;
			
			return remotePlay_Step(id); 
		}
		break;
		case CTRL_CMD_FASTBACKPLAY:
		{
			if(pAckLen) *pAckLen = 0;
			return CTRL_SUCCESS;
		}
		break;
		case CTRL_CMD_PLAYPREV:
		case CTRL_CMD_PLAYNEXT:
		{
			if(event==CTRL_CMD_PLAYNEXT)
				sNLIns.nLogId = EM_NETLOG_PB_FORWARD;
			else
				sNLIns.nLogId = EM_NETLOG_PB_BACKWARD;
				sNLIns.ip=cph->ip;
			pNetCtrl->pWriteLogCB(&sNLIns);
			
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote next/prev play:(id:0x%x)\n",id);
			if(pAckLen) *pAckLen = 0;

			return remotePlay_PrevNext(id, CTRL_CMD_PLAYNEXT-event);
		}
		break;
		case CTRL_CMD_PLAYSEEK:
		{
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote seek play:(id:0x%x)\n",id);
			*pAckLen = 0;
			
			memcpy(&seekPos,pbyMsgBuf+sizeof(id),sizeof(seekPos));
			seekPos = ntohl(seekPos);
			//printf("seekPos=%d\n",seekPos);

			return remotePlay_Seek(id, seekPos);
		}
		break;
		case CTRL_CMD_PLAYMUTE:
		{
			sNLIns.nLogId = EM_NETLOG_MUTE;
			sNLIns.ip = cph->ip;
			pNetCtrl->pWriteLogCB(&sNLIns);
			
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote mute play:(id:0x%x)\n",id);
			if(pAckLen) *pAckLen = 0;
			memcpy(&byMute,pbyMsgBuf+sizeof(id),sizeof(byMute));
			
			return remotePlay_Mute(id, byMute);
		}
		break;
		case CTRL_CMD_PLAYPROGRESS:
		{
			memcpy(&id,pbyMsgBuf,sizeof(u32));
			id = ntohl(id);
			//printf("remote progress play:(id:0x%x)\n",id);
			if(pAckLen) *pAckLen = 0;
			memcpy(&byStatus,pbyMsgBuf+sizeof(id),sizeof(byStatus));

			return remotePlay_Progress(id, cph, byStatus);
		}
		break;
		case CTRL_CMD_GETNETWORK:
		{
			ifly_network_t net;
			ifly_NetWork_t networkparam;
			
			(*pCB)(&sReqIns, &sRslt);
			
			memcpy(&net, &sRslt.sBasicInfo.netInfo, sizeof(net));
			
			memcpy(networkparam.mac_address,net.mac_address,sizeof(net.mac_address));
			
			//printf("CmdProc mac : %s %s\n", networkparam.mac_address, net.mac_address);
			
			networkparam.ip_address = (net.ip_address);
			networkparam.server_port = htons(net.server_port);
			networkparam.net_mask = (net.net_mask);
			networkparam.net_gateway = (net.net_gateway);
			networkparam.dns = (net.dns);
			networkparam.multicast_address = (net.multicast_address);
			//networkparam.admin_host = htonl(net.admin_host);
			//networkparam.host_port = htons(net.host_port);
			//暂时为固定值
			networkparam.mobile_port = htons(net.host_port);
			networkparam.http_port = htons(net.http_port);
			networkparam.flag_pppoe = net.flag_pppoe;
			memcpy(networkparam.pppoe_user_name,net.user_name,sizeof(net.user_name));
			memcpy(networkparam.pppoe_passwd,net.passwd,sizeof(net.passwd));
			networkparam.flag_dhcp = net.flag_dhcp;
			
			networkparam.flag_ddns = net.flag_ddns?1:0;
			networkparam.ddnsserver = net.flag_ddns>0?(net.flag_ddns-1):0;
			memcpy(networkparam.ddns_domain,net.ddns_domain,sizeof(net.ddns_domain));
			memcpy(networkparam.ddns_user,net.ddns_user,sizeof(net.ddns_user));
			memcpy(networkparam.ddns_passwd,net.ddns_passwd,sizeof(net.ddns_passwd));
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_NetWork_t);
			}
			memcpy(pbyAckBuf,&networkparam,sizeof(ifly_NetWork_t));
			
			//printf("get network domain %s %s %s\n", networkparam.ddns_domain, networkparam.ddns_user, networkparam.ddns_passwd);
		}
		break;
		case CTRL_CMD_SETNETWORK:
		{
			ifly_NetWork_t* pNetworkparam;
			
			pNetworkparam = &sReqIns.sReq.NetWork;
			
			memcpy(pNetworkparam,pbyMsgBuf,sizeof(ifly_NetWork_t));
			
			pNetworkparam->server_port = htons(pNetworkparam->server_port);
			pNetworkparam->mobile_port = htons(pNetworkparam->mobile_port);
			pNetworkparam->http_port = htons(pNetworkparam->http_port);
			
			(*pCB)(&sReqIns, &sRslt);
			if(pAckLen) *pAckLen = 0;
			
			//printf("set network\n");
		}
		break;	
		case CTRL_CMD_GETVGASOLLIST:
		{
			ifly_VGA_Solution *pVgasolution = &sRslt.sBasicInfo.VGA_Solution;
			memset(pVgasolution->vgapro, 0, sizeof(pVgasolution->vgapro));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_VGA_Solution);
			}
			
			for(i=0; i<sizeof(pVgasolution->vgapro)/sizeof(pVgasolution->vgapro[0]); i++)
			{
				if(pVgasolution->vgapro[i].width>0 && pVgasolution->vgapro[i].height>0)
				{
					pVgasolution->vgapro[i].width = htons(pVgasolution->vgapro[i].width);
					pVgasolution->vgapro[i].height = htons(pVgasolution->vgapro[i].height);
				}
			}
			
			memcpy(pbyAckBuf,pVgasolution,sizeof(ifly_VGA_Solution));
			
			//printf("get vgasollist\n");
		}
		break;
		//csp modify 20130422
		case CTRL_CMD_GETDDNSLIST:
		{
			ifly_DDNSList_t *pDDNSList = &sRslt.sBasicInfo.DDNS_List;
			memset(pDDNSList, 0, sizeof(ifly_DDNSList_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_DDNSList_t);
			}
			
			memcpy(pbyAckBuf,pDDNSList,sizeof(ifly_DDNSList_t));
			
			//printf("get ddnslist\n");
		}
		break;
	case CTRL_CMD_GETSYSLANGLIST:
		{
			//csp modify 20130422
			//ifly_SysLangList_t syslanglist;
			//memset(&syslanglist,0,sizeof(syslanglist));
			//syslanglist.max_langnum = SUPPORT_LANGUAGE_NUM;
			
			ifly_SysLangList_t* pSyslanglist = NULL;
			pSyslanglist = &sRslt.sBasicInfo.SysLangList;
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_SysLangList_t);
			}
			memcpy(pbyAckBuf, pSyslanglist, sizeof(ifly_SysLangList_t));
			
			//printf("Get sys language %d!\n", pSyslanglist->max_langnum);
		}
		break;
		case CTRL_CMD_GETSYSPARAM:
		{
			////printf("~~~~~~~~~~get sysparam!\n");
			
			ifly_SysParam_t* pSysinfo;

			(*pCB)(&sReqIns, &sRslt);
			pSysinfo = &sRslt.sBasicInfo.SysParam;
			
			////printf("~~~~~~~~~~~~~~~~sys.language:%d\n", pSysinfo->language);

			////printf("get l t=%d,s t=%d\n",ntohs(pSysinfo->lock_time),pSysinfo->switch_time);
			
			pSysinfo->device_id		= htons(pSysinfo->device_id);
			pSysinfo->lock_time		= htons(pSysinfo->lock_time);
			pSysinfo->switch_time	= htons(pSysinfo->switch_time);
			memcpy(pbyAckBuf,pSysinfo,sizeof(ifly_SysParam_t));
			if(pAckLen) *pAckLen = sizeof(ifly_SysParam_t);
			
			return CTRL_SUCCESS;
		}
		break;
		case CTRL_CMD_SETSYSPARAM:
		{
			//printf("set systemparam\n");
			
			ifly_SysParam_t* pSystemparam;
			pSystemparam = &sReqIns.sReq.SysParam;
			
			msgLen = sizeof(ifly_SysParam_t);
			memcpy(pSystemparam, pbyMsgBuf, msgLen);
			
			pSystemparam->device_id		= ntohs(pSystemparam->device_id);
			pSystemparam->lock_time 	= ntohs(pSystemparam->lock_time);
			pSystemparam->switch_time 	= ntohs(pSystemparam->switch_time);
			
			if(pAckLen) *pAckLen = 0;
			
			(*pCB)(&sReqIns, &sRslt);
			
			//printf("set systemparam finish\n");
			
			return CTRL_SUCCESS;
		}
		break;
		case CTRL_CMD_GETRECPARAM:
		{
			//printf("get recparam!\n");
 			
			ifly_RecordParam_t* pRecinfo;
			
			sReqIns.sReq.nChn = *(u8*)pbyMsgBuf;
			
			(*pCB)(&sReqIns, &sRslt);
			
			pRecinfo = &sRslt.sBasicInfo.RecordParam;
			
			pRecinfo->intraRate			= htons(pRecinfo->intraRate);
			pRecinfo->bit_max 			= htonl(pRecinfo->bit_max);
			pRecinfo->pre_record		= htons(pRecinfo->pre_record);
			pRecinfo->post_record		= htons(pRecinfo->post_record);
			
			/*printf("chn=%d,code type=%d,bit type=%d,bit max=%d,frame=%d\n",	
				pRecinfo->channelno,
				pRecinfo->code_type,
				pRecinfo->bit_type,
				pRecinfo->bit_max,
				pRecinfo->frame_rate);	
			*/
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_RecordParam_t);
			}
			
			memcpy(pbyAckBuf,pRecinfo,sizeof(ifly_RecordParam_t));
			
			return CTRL_SUCCESS;
		}
		break;
		
		case CTRL_CMD_SETRECPARAM:
		{
			//ifly_record_t rc;
			ifly_RecordParam_t* pRecinfo;
			
			pRecinfo = &sReqIns.sReq.RecordParam;
			
			memcpy(pRecinfo,pbyMsgBuf,sizeof(ifly_RecordParam_t));
			
			if(pAckLen) *pAckLen = 0;
			
			pRecinfo->bit_max = ntohl(pRecinfo->bit_max);
			pRecinfo->pre_record = ntohs(pRecinfo->pre_record);
			
			//csp modify 20130525
			//pRecinfo->pre_record = ntohs(pRecinfo->post_record);
			pRecinfo->post_record = ntohs(pRecinfo->post_record);
			
			(*pCB)(&sReqIns, &sRslt);
			
			return CTRL_SUCCESS;
		}		
		break;
		case CTRL_CMD_GETBITRATELIST:			//dong	100424
		{	
			ifly_bitRateList_t* pBitRateList;

			pBitRateList = &sRslt.sBasicInfo.bitRateList;
			sReqIns.sReq.nChn = *(u8*)pbyMsgBuf;
			
			memset(pBitRateList,0,sizeof(ifly_bitRateList_t));
			
			(*pCB)(&sReqIns, &sRslt);		

			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_bitRateList_t);
			}
			memcpy(pbyAckBuf,pBitRateList,sizeof(ifly_bitRateList_t));

			//printf("Get Sub sys Bitrate!\n");		
		}
		break;
		case CTRL_CMD_GETSUBSTREAMPARAM:
		{
			ifly_SubStreamParam_t* pSubstreamparam;
			
			pSubstreamparam = &sRslt.sBasicInfo.SubStreamParam;
			sReqIns.sReq.nChn = *(u8*)pbyMsgBuf;
			
			(*pCB)(&sReqIns, &sRslt);
			
			pSubstreamparam->sub_bitrate = htonl(pSubstreamparam->sub_bitrate);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_SubStreamParam_t);
			}
			memcpy(pbyAckBuf,pSubstreamparam,sizeof(ifly_SubStreamParam_t));
			
			//printf("Get Sub Stream Param!\n");
		}
		break;
		case CTRL_CMD_SETSUBSTREAMPARAM:
		{
			ifly_SubStreamParam_t* pRecinfo;
			
			pRecinfo = &sReqIns.sReq.SubStreamParam;
			
			memcpy(pRecinfo,pbyMsgBuf,sizeof(ifly_SubStreamParam_t));
			
			pRecinfo->sub_bitrate = ntohl(pRecinfo->sub_bitrate);
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
		}		
		break;
		case CTRL_CMD_GETIMGPARAM:
		{		
			ifly_ImgParam_t* pImaginfo;
			
			//printf("get imageparam!\n");
			
			pImaginfo = &sRslt.sBasicInfo.ImgParam;
			sReqIns.sReq.nChn = *(u8*)pbyMsgBuf;
			printf("CTRL_CMD_GETIMGPARAM chn: %d\n", sReqIns.sReq.nChn);
			(*pCB)(&sReqIns, &sRslt);
			
			pImaginfo->timepos_x = htons(pImaginfo->timepos_x);
			pImaginfo->timepos_y = htons(pImaginfo->timepos_y);
			
			pImaginfo->chnpos_x	= htons(pImaginfo->chnpos_x);
			pImaginfo->chnpos_y	= htons(pImaginfo->chnpos_y);

			for (i = 0; i < MAX_COVER_REGION_NUM; ++i)
			{
				pImaginfo->MaskInfo[i].x = htons(pImaginfo->MaskInfo[i].x);
				pImaginfo->MaskInfo[i].y = htons(pImaginfo->MaskInfo[i].y);
				pImaginfo->MaskInfo[i].width = htons(pImaginfo->MaskInfo[i].width);
				pImaginfo->MaskInfo[i].height = htons(pImaginfo->MaskInfo[i].height);
			}

			//printf("pImaginfo->channelnames %s\n", pImaginfo->channelname);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_ImgParam_t);
			}
			memcpy(pbyAckBuf,pImaginfo,sizeof(ifly_ImgParam_t));
		}
		break;
		case CTRL_CMD_SETIMGPARAM:
		{
			ifly_ImgParam_t* pSetimgparam;

			pSetimgparam = &sReqIns.sReq.ImgParam;
			memset(pSetimgparam, 0, sizeof(ifly_ImgParam_t));
			memcpy(pSetimgparam, pbyMsgBuf, sizeof(ifly_ImgParam_t));
		
			for (k = 0; k < MAX_COVER_REGION_NUM; ++k)
			{
				pSetimgparam->MaskInfo[k].x 		= ntohs(pSetimgparam->MaskInfo[k].x);
				pSetimgparam->MaskInfo[k].width 	= ntohs(pSetimgparam->MaskInfo[k].width);
				pSetimgparam->MaskInfo[k].height 	= ntohs(pSetimgparam->MaskInfo[k].height);
				pSetimgparam->MaskInfo[k].y 		= ntohs(pSetimgparam->MaskInfo[k].y);
			}

			pSetimgparam->timepos_x 		= ntohs(pSetimgparam->timepos_x);
			pSetimgparam->timepos_y 		= ntohs(pSetimgparam->timepos_y);
			
			pSetimgparam->chnpos_x 		= ntohs(pSetimgparam->chnpos_x);
			pSetimgparam->chnpos_y 		= ntohs(pSetimgparam->chnpos_y);

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
		}
		break;
		case CTRL_CMD_GETPICADJ:
		{
			ifly_PicAdjust_t* pGetpicadjust;
			pGetpicadjust = &sReqIns.sReq.PicAdjust;
			memset(pGetpicadjust, 0, sizeof(ifly_PicAdjust_t));
			memcpy(pGetpicadjust, pbyMsgBuf, sizeof(ifly_PicAdjust_t));

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_PicAdjust_t);
			}
			pGetpicadjust = &sRslt.sBasicInfo.PicAdjust;
			memcpy(pbyAckBuf,pGetpicadjust,sizeof(ifly_PicAdjust_t));
			
			//printf("get pic adjust\n");
		}
		break;
		case CTRL_CMD_SETPICADJ:
		{
			ifly_PicAdjust_t* pSetpicadjust;

			pSetpicadjust = &sReqIns.sReq.PicAdjust;
			memset(pSetpicadjust, 0, sizeof(ifly_PicAdjust_t));
			memcpy(pSetpicadjust, pbyMsgBuf, sizeof(ifly_PicAdjust_t));

			(*pCB)(&sReqIns, &sRslt);
			
			*pAckLen = 0;
			//printf("set pic adjust\n");
		}
		break;
		case CTRL_CMD_GETALARMINPARAM:
		{
			ifly_AlarmInParam_t* pAlarminparam;
			
			pAlarminparam = &sRslt.sBasicInfo.AlarmInParam;
			sReqIns.sReq.nAlarmInId = *(u8*)pbyMsgBuf;
			
			(*pCB)(&sReqIns, &sRslt);
			
			//printf("pAlarminparam->typein %d\n", pAlarminparam->typein );
			//printf("pAlarminparam->triRecChn %d\n", pAlarminparam->triRecChn );
			//printf("pAlarminparam->triAlarmoutid %d\n", pAlarminparam->triAlarmoutid );
			
			pAlarminparam->delay		 = htons(pAlarminparam->delay);
			pAlarminparam->triRecChn 	 = htonl(pAlarminparam->triRecChn);
			pAlarminparam->triAlarmoutid = htonl(pAlarminparam->triAlarmoutid);
			pAlarminparam->copy2AlarmInmask = htonl(pAlarminparam->copy2AlarmInmask);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_AlarmInParam_t);
			}
			
			memcpy(pbyAckBuf,pAlarminparam,sizeof(ifly_AlarmInParam_t));
			
			//printf("get alarm in param!\n");
		}
		break;		
		case CTRL_CMD_SETALARMINPARAM:
		{
			ifly_AlarmInParam_t* pAlarminparam;

			pAlarminparam = &sReqIns.sReq.AlarmInParam;
			
			memset(pAlarminparam, 0, sizeof(ifly_AlarmInParam_t));
			memcpy(pAlarminparam, pbyMsgBuf, sizeof(ifly_AlarmInParam_t));

			pAlarminparam->triRecChn 	 = ntohl(pAlarminparam->triRecChn);
			pAlarminparam->delay		 = ntohs(pAlarminparam->delay);
			pAlarminparam->triAlarmoutid = ntohl(pAlarminparam->triAlarmoutid);
			pAlarminparam->copy2AlarmInmask = ntohl(pAlarminparam->copy2AlarmInmask);

			(*pCB)(&sReqIns, &sRslt);
				
			if(pAckLen) *pAckLen = 0;
			
			//printf("set alarm in param!\n");
			
			if(sRslt.nErrCode != 0)
				return -1;
		}
		break;
		case CTRL_CMD_GETALARMOUTPARAM:
		{
			ifly_AlarmOutParam_t* pAlarmoutparam, *pAlarmoutparamIn;
			
			pAlarmoutparamIn = &sReqIns.sReq.AlarmOutParam;
			pAlarmoutparam = &sRslt.sBasicInfo.AlarmOutParam;
			
			memset(pAlarmoutparam,0,sizeof(ifly_AlarmOutParam_t));
			
			pAlarmoutparamIn->outid = *pbyMsgBuf;
			
			(*pCB)(&sReqIns, &sRslt);
			
			pAlarmoutparam->copy2AlarmOutmask = ntohl(pAlarmoutparam->copy2AlarmOutmask);
			pAlarmoutparam->alarmoutdelay = ntohs(pAlarmoutparam->alarmoutdelay);
			pAlarmoutparam->buzzdelay = ntohs(pAlarmoutparam->buzzdelay);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_AlarmOutParam_t);
			}
			
			memcpy(pbyAckBuf,pAlarmoutparam,sizeof(ifly_AlarmOutParam_t));
			
			//printf("get alarm out param!\n");
		}
		break;
		case CTRL_CMD_SETALARMOUTPARAM:
		{
			ifly_AlarmOutParam_t* pSetalarmoutparam;
			pSetalarmoutparam = &sReqIns.sReq.AlarmOutParam;
			
			memset(pSetalarmoutparam, 0, sizeof(ifly_AlarmOutParam_t));
			memcpy(pSetalarmoutparam, pbyMsgBuf ,sizeof(ifly_AlarmOutParam_t));
			
			pSetalarmoutparam->copy2AlarmOutmask = 
				htonl(pSetalarmoutparam->copy2AlarmOutmask);
			pSetalarmoutparam->alarmoutdelay 	= htons(pSetalarmoutparam->alarmoutdelay);
			pSetalarmoutparam->buzzdelay 		= htons(pSetalarmoutparam->buzzdelay);
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			//printf("set alarm out pro!\n");
		}
		break;
		case CTRL_CMD_GETALARMNOTICYPARAM:
		{
			ifly_AlarmNoticeParam_t* pAlarmnoticeparam;
			pAlarmnoticeparam = &sRslt.sBasicInfo.AlarmNoticeParam;

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_AlarmNoticeParam_t);
			}
			memcpy(pbyAckBuf,pAlarmnoticeparam,sizeof(ifly_AlarmNoticeParam_t));
			
			//printf("get alarm noticy param:%s!\n",pAlarmnoticeparam->alarm_email);
		}
		break;
		case CTRL_CMD_SETALARMNOTICYPARAM:
		{
			ifly_AlarmNoticeParam_t* pAlarmnoticeparam;
			pAlarmnoticeparam = &sReqIns.sReq.AlarmNoticeParam;
			
			memcpy(pAlarmnoticeparam,pbyMsgBuf,sizeof(ifly_AlarmNoticeParam_t));
			
			(*pCB)(&sReqIns, &sRslt);
			if(pAckLen) *pAckLen = 0;
			
			//printf("set alarm noticy papam:%s!\n",pAlarmnoticeparam->alarm_email);
		}
		break;
#ifdef IECTRL_NEW
		case CTRL_CMD_RESETPICADJUST:
		{
			ifly_reset_picadjust_t* pReset_picadjust;

			pReset_picadjust = &sReqIns.sReq.reset_picadjust;
			
			memset(pReset_picadjust, 0, sizeof(ifly_reset_picadjust_t));
			memcpy(pReset_picadjust, pbyMsgBuf, sizeof(ifly_reset_picadjust_t));

			(*pCB)(&sReqIns, &sRslt);
			
			//printf("reset pic adjust\n");
			if(pAckLen) *pAckLen = 0;
		}
		break;
		case CTRL_CMD_GETFRAMERATELIST:
		{
			ifly_framerate_list_t* pFramerate_list, *pFramerate_listIn;
			
			pFramerate_listIn = &sReqIns.sReq.framerate_list;
			pFramerate_list = &sRslt.sBasicInfo.framerate_list;
			
			pFramerate_listIn->chn = *pbyMsgBuf;
			pFramerate_listIn->type = *(pbyMsgBuf + 1);
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_framerate_list_t);
			}
			memcpy(pbyAckBuf,pFramerate_list,sizeof(ifly_framerate_list_t));
			
			//printf("Get frame list!\n");
		}
		break;
		//获得支持的最大遮盖数
		case CTRL_CMD_GETMAX_IMGMASKNUM:
		{
			*pbyAckBuf = 1;
			
			if(pAckLen) *pAckLen = sizeof(u8);
			
			////printf("Get max ingmask number!\n");
		}
		break;	
		//主/子码流分辨率列表 CTRL_GET_RESOLUTION_LIST
		case CTRL_GET_RESOLUTION_LIST:
		{
			ifly_videoresolu_list_t* pVideoresolu_list, *pVideoresolu_listIn;
			
			pVideoresolu_listIn = &sReqIns.sReq.videoresolu_list;
			pVideoresolu_list = &sRslt.sBasicInfo.videoresolu_list;
			
			pVideoresolu_listIn->chn = *pbyMsgBuf;
			pVideoresolu_listIn->type = *(pbyMsgBuf + 1);
			
			(*pCB)(&sReqIns, &sRslt);
			
			/*printf("%d,sub chn=%d,bit type=%d\n",
				__LINE__,
				pVideoresolu_listIn->chn, 
				pVideoresolu_listIn->type );
			*/
			
			memcpy(pbyAckBuf,pVideoresolu_list,sizeof(ifly_videoresolu_list_t));
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_videoresolu_list_t);
			}
			
			//printf("Get video resolution list! %d \n", *pbyAckBuf);
		}
		break;
#endif
		case CTRL_CMD_GETEMAILSMTP:
		{
			ifly_AlarmEmail_SMTP_t* pAlarm_mail_smtp;

			pAlarm_mail_smtp = &sRslt.sBasicInfo.AlarmEmail_SMTP;
			memset(pAlarm_mail_smtp, 0, sizeof(ifly_AlarmEmail_SMTP_t));

			(*pCB)(&sReqIns, &sRslt);

			pAlarm_mail_smtp->smtp_port = htons(pAlarm_mail_smtp->smtp_port);
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_AlarmEmail_SMTP_t);
			}
			memcpy(pbyAckBuf,pAlarm_mail_smtp,sizeof(ifly_AlarmEmail_SMTP_t));
			
			//printf("get alarm email smtp param\n");
		}
		break;	
		case CTRL_CMD_SETEMAILSMTP:
		{
			ifly_AlarmEmail_SMTP_t* pAlarm_mail_smtp;

			pAlarm_mail_smtp = (ifly_AlarmEmail_SMTP_t*)&sReqIns.sReq.AlarmEmail_SMTP;
			memcpy(pAlarm_mail_smtp, pbyMsgBuf, sizeof(ifly_AlarmEmail_SMTP_t));

			pAlarm_mail_smtp->smtp_port = ntohs(pAlarm_mail_smtp->smtp_port);

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			//printf("Set alarm email smtp param\n");
		}
		break;
		case CTRL_CMD_GETRECSCHPARAM: 
		{
			SCPRecSch*	psRecSch;
			ifly_RecordSCH_t* pRecordsch;
			
			psRecSch = &sReqIns.sReq.sCPRecSch;
			pRecordsch = &sRslt.sBasicInfo.RecordSCH;
			
			psRecSch->nChn = pbyMsgBuf[0];
			psRecSch->nWeek = pbyMsgBuf[1];

			(*pCB)(&sReqIns, &sRslt);

			for(i=0;i<SEGMENTS_OF_DAY;i++)
			{
				pRecordsch->TimeFiled[i].starttime = htonl(pRecordsch->TimeFiled[i].starttime);
				pRecordsch->TimeFiled[i].endtime = htonl(pRecordsch->TimeFiled[i].endtime);
			}

			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_RecordSCH_t);
			}
			memcpy(pbyAckBuf,pRecordsch,sizeof(ifly_RecordSCH_t));
		}
		break;
		case CTRL_CMD_SETRECSCHPARAM:
		{
			ifly_RecordSCH_t* pSetrecordsch;

			pSetrecordsch = &sReqIns.sReq.RecordSCH;
			
			memset(pSetrecordsch,0,sizeof(ifly_RecordSCH_t));
			memcpy(pSetrecordsch,pbyMsgBuf,sizeof(ifly_RecordSCH_t));

			for(i=0;i<SEGMENTS_OF_DAY;i++)
			{
				pSetrecordsch->TimeFiled[i].starttime = ntohl(pSetrecordsch->TimeFiled[i].starttime);
				pSetrecordsch->TimeFiled[i].endtime = ntohl(pSetrecordsch->TimeFiled[i].endtime);
			}

			(*pCB)(&sReqIns, NULL);
			
			*pAckLen = 0;
		}
		break;
		case CTRL_CMD_GETMDPARAM:
		{
			ifly_MDParam_t* pMdparam, *pMdparamIn;

			pMdparamIn 	= &sReqIns.sReq.MDParam;
			pMdparam	= &sRslt.sBasicInfo.MDParam;
			
			pMdparamIn->chn = *pbyMsgBuf;

			(*pCB)(&sReqIns, &sRslt);

			pMdparam->delay 		= htons(pMdparam->delay);
			pMdparam->trigRecChn 	= htonl(pMdparam->trigRecChn);
			pMdparam->trigAlarmOut 	= htonl(pMdparam->trigAlarmOut);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_MDParam_t);
			}
			memcpy(pbyAckBuf,pMdparam,sizeof(ifly_MDParam_t));
			
			//printf("get md param!\n");
		}
		break;
		case CTRL_CMD_SETMDPARAM:
		{
			ifly_MDParam_t* pSetmdparam;

			pSetmdparam = &sReqIns.sReq.MDParam;
			
			memset(pSetmdparam, 0, sizeof(ifly_MDParam_t));
			memcpy(pSetmdparam, pbyMsgBuf, sizeof(ifly_MDParam_t));

			pSetmdparam->delay 			= ntohs(pSetmdparam->delay);
			pSetmdparam->trigRecChn 	= ntohl(pSetmdparam->trigRecChn);
			pSetmdparam->trigAlarmOut 	= ntohl(pSetmdparam->trigAlarmOut);

			(*pCB)(&sReqIns, &sRslt);
			
			*pAckLen = 0;

			if(sRslt.nErrCode != 0)
				return -1;
			
			//printf("set md param!\n");
		}
		break;
		case CTRL_CMD_GETVIDEOLOSTPARAM:
		{
			ifly_VideoLostParam_t* pVideolostparam, *pVideolostparamIn;

			pVideolostparam 	= &sRslt.sBasicInfo.VideoLostParam;
			pVideolostparamIn 	= &sReqIns.sReq.VideoLostParam;

			pVideolostparamIn->chn = *pbyMsgBuf;
			
			(*pCB)(&sReqIns, &sRslt);
			
			pVideolostparam->delay		  = htons(pVideolostparam->delay);
			pVideolostparam->trigRecChn   = htonl(pVideolostparam->trigRecChn);
			pVideolostparam->trigAlarmOut = htonl(pVideolostparam->trigAlarmOut);

			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_VideoLostParam_t);
			}
			memcpy(pbyAckBuf, pVideolostparam, sizeof(ifly_VideoLostParam_t));
			
			//printf("get video lost param!\n");
		}
		break;
		case CTRL_CMD_SETVIDEOLOSTPARAM: 	
		{
			ifly_VideoLostParam_t* pSetvideolostparam;

			pSetvideolostparam = &sReqIns.sReq.VideoLostParam;

			memset(pSetvideolostparam, 0, sizeof(ifly_VideoLostParam_t));
			memcpy(pSetvideolostparam, pbyMsgBuf, sizeof(ifly_VideoLostParam_t));

			pSetvideolostparam->delay			= ntohs(pSetvideolostparam->delay);
			pSetvideolostparam->trigRecChn 		= ntohl(pSetvideolostparam->trigRecChn);
			pSetvideolostparam->trigAlarmOut 	= ntohl(pSetvideolostparam->trigAlarmOut);
			
			(*pCB)(&sReqIns, &sRslt);			

			if(sRslt.nErrCode != 0)
				return -1;
				
			*pAckLen = 0;
			//printf("set video lost param!\n");
		}
		break;
		case CTRL_CMD_GETVIDEOBLOCKPARAM:
		{
            ifly_VideoBlockParam_t* pVideoblockparam, *pVideoblockparamIn;

			pVideoblockparam 	= &sRslt.sBasicInfo.VideoBlockParam;			
			pVideoblockparamIn 	= &sReqIns.sReq.VideoBlockParam;

			pVideoblockparamIn->chn = *pbyMsgBuf;
			
			(*pCB)(&sReqIns, &sRslt);
			
			pVideoblockparam->trigRecChn = htonl(pVideoblockparam->trigRecChn);
			pVideoblockparam->trigAlarmOut = htonl(pVideoblockparam->trigAlarmOut);

			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_VideoBlockParam_t);
			}
			
			memcpy(pbyAckBuf, pVideoblockparam, sizeof(ifly_VideoBlockParam_t));			          
		}
		break;
		case CTRL_CMD_SETVIDEOBLOCKPARAM:
		{
			ifly_VideoBlockParam_t* pSetvideoblockparam;

			pSetvideoblockparam = &sReqIns.sReq.VideoBlockParam;

			memset(pSetvideoblockparam, 0, sizeof(ifly_VideoBlockParam_t));
			memcpy(pSetvideoblockparam, pbyMsgBuf, sizeof(ifly_VideoBlockParam_t));

            for(i=0;i<nChnMax;i++)
            {
            	pSetvideoblockparam->trigRecChn = ntohl(pSetvideoblockparam->trigRecChn);
            }
            for(i=0;i<MAX_ALARM_OUT_NUM;i++)
            {
            	pSetvideoblockparam->trigAlarmOut = ntohl(pSetvideoblockparam->trigAlarmOut);     
            }
            
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			
			//printf("set video block param!\n");
		}
		break;
		case CTRL_CMD_GETPTZPARAM:
		{
			ifly_PTZParam_t* pYuninfo;
			
			pYuninfo = &sRslt.sBasicInfo.PTZParam;
			sReqIns.sReq.nChn = *(u8*)pbyMsgBuf;
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_PTZParam_t);
			}
			
			//printf("address %d\n", pYuninfo->address);
			//printf("baud_ratesel %d\n", pYuninfo->baud_ratesel);
			//printf("data_bitsel %d\n", pYuninfo->data_bitsel);
			//printf("stop_bitsel %d\n", pYuninfo->stop_bitsel);
			//printf("crccheck %d\n", pYuninfo->crccheck);
			//printf("flow_control %d\n", pYuninfo->flow_control);
			//printf("protocol %d\n", pYuninfo->protocol);
			
			pYuninfo->address = htons(pYuninfo->address);
			
			memcpy(pbyAckBuf, pYuninfo, sizeof(ifly_PTZParam_t));
			
			//printf("get ptz pro!\n");
		}
		break;
		case CTRL_CMD_SETPTZPARAM:
		{
			ifly_PTZParam_t* pSetptzparam;

			pSetptzparam = &sReqIns.sReq.PTZParam;
			
			memset(pSetptzparam, 0, sizeof(ifly_PTZParam_t));
			memcpy(pSetptzparam, pbyMsgBuf, sizeof(ifly_PTZParam_t));

			pSetptzparam->address = ntohs(pSetptzparam->address);
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
		}
		break;
		case CTRL_CMD_SETPRESET:
		{
			ifly_PtzPresetCtr_t* pPtzpresetctrl;

			pPtzpresetctrl = &sReqIns.sReq.PtzPresetCtr;
			
			memset(pPtzpresetctrl, 0, sizeof(ifly_PtzPresetCtr_t));
			memcpy(pPtzpresetctrl, pbyMsgBuf, sizeof(ifly_PtzPresetCtr_t));

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
		}
		break;
		case CTRL_CMD_GETCRUISEPARAM:
		{
			ifly_PtzCruisePathParam_t* pGetptzcruisepathparam, *pGetptzcruisepathparamIn;

			pGetptzcruisepathparam = &sRslt.sBasicInfo.PtzCruisePathParam;
			pGetptzcruisepathparamIn = &sReqIns.sReq.PtzCruisePathParam;

			pGetptzcruisepathparamIn->chn		 = *(u8*)pbyMsgBuf;
			pGetptzcruisepathparamIn->cruise_path = *(u8*)(pbyMsgBuf+1);
			
			memset(pGetptzcruisepathparam, 0, sizeof(ifly_PtzCruisePathParam_t));

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_PtzCruisePathParam_t);
			}
			memcpy(pbyAckBuf, pGetptzcruisepathparam, sizeof(ifly_PtzCruisePathParam_t));
			
			//printf("get cruise param!\n");
		} break;
		case CTRL_CMD_SETCRUISEPARAM:	
		{
			ifly_PtzCruisePathParam_t* pSetptzcruisepathparam;

			pSetptzcruisepathparam = &sReqIns.sReq.PtzCruisePathParam;
			memset(pSetptzcruisepathparam, 0, sizeof(ifly_PtzCruisePathParam_t));
			msgLen = sizeof(ifly_PtzCruisePathParam_t);
			memcpy(pSetptzcruisepathparam, pbyMsgBuf, sizeof(ifly_PtzCruisePathParam_t));

			(*pCB)(&sReqIns, &sRslt);

			if(pAckLen) *pAckLen = 0;
				
			if(sRslt.nErrCode != 0)
			{
				return -1;
			}
			
			//printf("set cruise param!\n");
		}
		break;
		case CTRL_CMD_CTRLCRUISEPATH:
		{
			ifly_CruisePathCtr_t* pCruisepathctrl;

			pCruisepathctrl = &sReqIns.sReq.CruisePathCtr;
			
			memcpy(pCruisepathctrl, pbyMsgBuf, sizeof(ifly_CruisePathCtr_t));
			msgLen = sizeof(ifly_CruisePathCtr_t);

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
		}
		break;
		case CTRL_CMD_CTRLPTZTRACK:
		{
			ifly_TrackCtr_t* pTrackctrl;
			
			pTrackctrl = &sReqIns.sReq.TrackCtr;
			msgLen = sizeof(ifly_TrackCtr_t);
			memcpy(pTrackctrl, pbyMsgBuf, sizeof(ifly_TrackCtr_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
		}
		break;
		case CTRL_CMD_GETHDDINFO:
		{
			//printf("CTRL_CMD_GETHDDINFO-1\n");
			
			ifly_hddInfo_t* pHddinfo;
			pHddinfo = &sRslt.sBasicInfo.hddInfo;
			memset(pHddinfo, 0, sizeof(ifly_hddInfo_t));
			
			sReqIns.sReq.nHddIdx = *pbyMsgBuf;
			pHddinfo->hdd_index = sReqIns.sReq.nHddIdx;
			
			if(sReqIns.sReq.nHddIdx<MAX_HDD_NUM)
			{
				//printf("CTRL_CMD_GETHDDINFO-2,nHddIdx=%d\n",sReqIns.sReq.nHddIdx);
				
				(*pCB)(&sReqIns, &sRslt);
				
				//printf("CTRL_CMD_GETHDDINFO-3,idx %d total %d free %d exist %d\n", pHddinfo->hdd_index, pHddinfo->capability, pHddinfo->freesize, pHddinfo->hdd_exist);
				
				pHddinfo->capability = htonl(pHddinfo->capability);
				pHddinfo->freesize = htonl(pHddinfo->freesize);		
				
				if(pAckLen)
				{
					*pAckLen = sizeof(ifly_hddInfo_t);
				}
				
				memcpy(pbyAckBuf, pHddinfo, sizeof(ifly_hddInfo_t));
				
				//printf("CTRL_CMD_GETHDDINFO-4\n");
			}
			else
			{
				if(pAckLen) *pAckLen = 0;
				
				return CTRL_FAILED_PARAM;
			}
		}
		break;
		case CTRL_CMD_SETUSERINFO:
		{
			ifly_userMgr_t* pUsermanger;

			pUsermanger = &sReqIns.sReq.userMgr;
			memset(pUsermanger, 0, sizeof(ifly_userMgr_t));
			msgLen = sizeof(ifly_userMgr_t);
			memcpy(pUsermanger, pbyMsgBuf, sizeof(ifly_userMgr_t));

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;

			if(sRslt.nErrCode != 0)
			{
				return -1;
			}
			
			//printf("set user info !\n");
		}
		break;
		case CTRL_CMD_SETSYSTIME:
		{
			ifly_sysTime_t* pSetsystemtime;
			pSetsystemtime = &sReqIns.sReq.sysTime;
			
			memset(pSetsystemtime, 0, sizeof(ifly_sysTime_t));
			memcpy(pSetsystemtime, pbyMsgBuf, sizeof(ifly_sysTime_t));
			pSetsystemtime->systemtime = ntohl(pSetsystemtime->systemtime);
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			
			////printf("set system time:%d!\n", pSetsystemtime->systemtime);
		}
		break;
		case CTRL_CMD_USEUTCTIME:
		{
			ifly_utctime_t* pIsUseUTC;
			pIsUseUTC = &sRslt.sBasicInfo.utctime;
			
			memset(pIsUseUTC,0,sizeof(ifly_utctime_t));
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_utctime_t);
			}
			memcpy(pbyAckBuf, pIsUseUTC, sizeof(ifly_utctime_t));
			
			//printf("use UTC time\n");
		}
		break;
		case CTRL_CMD_GETSYSINFO:
		{
			ifly_sysinfo_t* pSys;
			pSys = &sRslt.sBasicInfo.sysinfo;
			
			memset(pSys, 0, sizeof(ifly_sysinfo_t));
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_sysinfo_t);
			}
			memcpy(pbyAckBuf, pSys, sizeof(ifly_sysinfo_t));
			
			//printf("get systime info!\n");
		}
		break;
		case CTRL_CMD_PTZCTRL:
		{
			ifly_PtzCtrl_t* pPtzCtrl;

			pPtzCtrl = &sReqIns.sReq.PtzCtrl;
			
			memcpy(pPtzCtrl, pbyMsgBuf, sizeof(ifly_PtzCtrl_t));
			if(pAckLen) *pAckLen = 0;

			(*pCB)(&sReqIns, &sRslt);
		}
		break;
		case CTRL_CMD_GETMANUALREC:
		{
			//printf("get record state ok!\n");
			
			ifly_ManualRecord_t* pManualrecord;

			pManualrecord = &sRslt.sBasicInfo.ManualRecord;
			memset(pManualrecord, 0, sizeof(ifly_ManualRecord_t));

			(*pCB)(&sReqIns, &sRslt);
			pManualrecord->chnRecState = htonl(pManualrecord->chnRecState);
			
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_ManualRecord_t);
			}
			memcpy(pbyAckBuf, pManualrecord, sizeof(ifly_ManualRecord_t));
		}
		break;
		case CTRL_CMD_SETMANUALREC:		
		{
			ifly_ManualRecord_t* pManualrecord;

			pManualrecord = &sReqIns.sReq.ManualRecord;
			
			memcpy(pManualrecord, pbyMsgBuf, sizeof(ifly_ManualRecord_t));

			pManualrecord->chnRecState = ntohl(pManualrecord->chnRecState);
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			
			//printf("set record state ok!\n");
		}
		break;
		case CTRL_CMD_LOGSEARCH:
		{
			#if 1
			ifly_search_log_t *pPara_infoIn;
			SCPLogInfo	  *pSearchLog;
			
			pSearchLog = &sRslt.sBasicInfo.sLogInfo;
			pPara_infoIn = &sReqIns.sReq.search_log;

			memset(pSearchLog, 0, sizeof(SCPLogInfo));

			memcpy(pPara_infoIn, pbyMsgBuf, sizeof(ifly_search_log_t));
			
			pPara_infoIn->max_return= ntohs(pPara_infoIn->max_return);
			pPara_infoIn->start_time = ntohl(pPara_infoIn->start_time);
			pPara_infoIn->end_time = ntohl(pPara_infoIn->end_time);

			pPara_infoIn->startID = ntohs(pPara_infoIn->startID);
			pSearchLog->Desc.startID = pPara_infoIn->startID;
			
			ifly_logInfo_t sLogInfo[0x4ff]; //zlb20111117 去掉malloc
			pSearchLog->para_log = sLogInfo;

			if(NULL != pSearchLog->para_log)
			{
				(*pCB)(&sReqIns, &sRslt);

				if(pSearchLog->Desc.startID > pSearchLog->Desc.sum)
				{
					pSearchLog->Desc.sum = htons(pSearchLog->Desc.sum);
					pSearchLog->Desc.startID = htons(0);
					pSearchLog->Desc.endID = htons(0);
					memcpy(pbyAckBuf,&pSearchLog->Desc,sizeof(pSearchLog->Desc));
					*pAckLen = sizeof(pSearchLog->Desc);

					//free(pSearchLog->para_log);//zlb20111117 去掉malloc
					
					return CTRL_SUCCESS;
				}

				int sum = 0;

				//printf("pSearchLog->Desc.sum %d \n", pSearchLog->Desc.sum);

				if(pSearchLog->Desc.startID<1) return -1;
				
				for(i=pSearchLog->Desc.startID; i<(pSearchLog->Desc.sum+1); ++i)
				{
					if(i>=pSearchLog->Desc.startID+pPara_infoIn->max_return) break;
					
					ifly_logInfo_t* pLogInfo = &pSearchLog->para_log[i-1];

					/*printf("idx %d %d %d %d %s\n",
						i,
						pLogInfo->startTime,
						pLogInfo->main_type,
						pLogInfo->slave_type,
						pLogInfo->loginfo
					);
					*/
					
					pLogInfo->startTime = htonl(pLogInfo->startTime);
					memcpy(	pbyAckBuf + sizeof(ifly_search_desc_t) + 
						(i-pSearchLog->Desc.startID)*sizeof(ifly_logInfo_t), 
						pLogInfo,
						sizeof(ifly_logInfo_t));

					sum++;
				}
				
				*pAckLen = sum*sizeof(ifly_logInfo_t)+sizeof(ifly_search_desc_t);
				
				pSearchLog->Desc.endID = htons(sum-1+pSearchLog->Desc.startID);
				pSearchLog->Desc.sum = htons(pSearchLog->Desc.sum);
				pSearchLog->Desc.startID = htons(pSearchLog->Desc.startID);
				
				memcpy(pbyAckBuf, &pSearchLog->Desc, sizeof(ifly_search_desc_t));

				//free(pSearchLog->para_log);//zlb20111117 去掉malloc
			}
			#endif
			
			//printf("log search!\n");
		}
		break;	
#ifdef SPECIAL_DEV_INFO
		case CTRL_CMD_GET_SPECIALDEVICEINFO:
		{
			ifly_specialinfo_t sp_info;
			memset(&sp_info, 0, sizeof(sp_info));
			sp_info.characterset = 2; //字符集: 0-ascii, 1-GB2312, 2-UTF8, 3-UTF16
			if(pAckLen)
			{
				*pAckLen = sizeof(ifly_specialinfo_t);
			}
			memcpy(pbyAckBuf,&sp_info,sizeof(sp_info));
			//printf("Get SPECIALDEVICEINFO!\n");
			return CTRL_SUCCESS;
		}
		break;
#endif
		case CTRL_NOTIFY_CONNLOST:
		{
			struct in_addr in;
			in.s_addr = cph->ip;
			
			for(i=0;i<MAX_ALARM_UPLOAD_NUM;i++)
			{
				if(cph == g_AlarmUploadCenter[i].g_cph)
				{
					printf("client %s out AlarmUploadCenter\n", inet_ntoa(in));
					net_write_upload_alarm(0xffff0000+i);//set 0
					break;
				}
			}		
		} break;
		case CTRL_CMD_ALARMUPLOADCENTER:
		{
			#if 1
			msgLen = sizeof(unsigned char);
			u8 tmp = 0;
			memcpy(&tmp,pbyMsgBuf,sizeof(unsigned char));
			if(pAckLen) *pAckLen = 0;

			struct in_addr in;
			in.s_addr = cph->ip;
			
			for(i=0;i<MAX_ALARM_UPLOAD_NUM;i++)
			{
				if(cph == g_AlarmUploadCenter[i].g_cph)
				{
					if (1 == tmp) {
						return CTRL_SUCCESS;
					} 
					else 
					{
						printf("client %s out AlarmUploadCenter\n", inet_ntoa(in));
						net_write_upload_alarm(0xffff0000+i);//set 0
						return CTRL_SUCCESS;
					}
				}
			}
			
			if(0 == tmp) return CTRL_SUCCESS;
			
			for(i=0;i<MAX_ALARM_UPLOAD_NUM;i++)
			{
				if(NULL == g_AlarmUploadCenter[i].g_cph)
				{
					printf("client %s in AlarmUploadCenter\n", inet_ntoa(in));
					g_AlarmUploadCenter[i].g_cph = cph;
					net_write_upload_alarm(0xffff0100+i);//set 1
					break;
				}
			}
			
			if(i>=MAX_ALARM_UPLOAD_NUM)
			{
				printf("open close alarm upload center err!\n");
				return CTRL_FAILED_RESOURCE;
			}
			#else
			msgLen = sizeof(unsigned char);
			u8 tmp = 0;
			memcpy(&tmp,pbyMsgBuf,sizeof(unsigned char));
			//printf("%s tmp: %d\n", __func__, tmp);
			
			if(pAckLen) *pAckLen = 0;
			
			for(i=0;i<MAX_ALARM_UPLOAD_NUM;i++)
			{
				if(cph == g_AlarmUploadCenter[i].g_cph)
				{
					if (1 == tmp)
					{
						return CTRL_SUCCESS;
					}
					else
					{
						net_write_upload_alarm(0xffff0000+i);//set 0
						return CTRL_SUCCESS;
					}
				}
			}
			
			if(0 == tmp) return CTRL_SUCCESS;

			#if 0
				struct in_addr in;
				in.s_addr = cph->ip;
				printf("%s cph->ip: %s, conntype: %d\n", __func__, inet_ntoa(in), cph->conntype);
			#endif
			
			for(i=0;i<MAX_ALARM_UPLOAD_NUM;i++)
			{
				if(NULL == g_AlarmUploadCenter[i].g_cph)
				{
					g_AlarmUploadCenter[i].g_cph = cph;
					//printf("%s MAX_ALARM_UPLOAD_NUM: %d\n", __func__, MAX_ALARM_UPLOAD_NUM);
					net_write_upload_alarm(0xffff0100+i);//set 1
					break;
				}
				else
				{
					#if 0
						//struct in_addr in;
						in.s_addr = g_AlarmUploadCenter[i].g_cph->ip;
						printf("%s g_AlarmUploadCenter[%d] ip: %s, conntype: %d\n",
							__func__, i, inet_ntoa(in), g_AlarmUploadCenter[i].g_cph->conntype);
					#endif
					
					if (cph->ip == g_AlarmUploadCenter[i].g_cph->ip \
						&& CTRL_CONNECTION_TCPSERVER == g_AlarmUploadCenter[i].g_cph->conntype)
					{
						CleanCPHandle(g_AlarmUploadCenter[i].g_cph);
					
						g_AlarmUploadCenter[i].g_cph = cph;
						//printf("%s MAX_ALARM_UPLOAD_NUM: %d\n", __func__, MAX_ALARM_UPLOAD_NUM);
						net_write_upload_alarm(0xffff0100+i);//set 1
						break;
					}
				}
			}
			
			if(i>=MAX_ALARM_UPLOAD_NUM)
			{
				printf("%s ***Error*** open close alarm upload center err!\n", __func__);
				return CTRL_FAILED_RESOURCE;
			}
			
			//printf("open close alarm upload center ok!\n");
			#endif
		}
		break;
	//xdc
		case CTRL_CMD_GETSEACHIPCLIST:
		{
			printf(" CTRL_CMD_GETSEACHIPCLIST CTRL_CMD_GETSEACHIPCLIST is start\n");
			ifly_search_ipc_t* ipcSeach;
			SCPIpcInfo	  *pSearchIpc;
			int j = 0;
			
			pSearchIpc = &sRslt.sBasicInfo.sIpcInfo;
			ipcSeach = &sReqIns.sReq.search_ipc;
			memcpy(ipcSeach, pbyMsgBuf, sizeof(ifly_search_ipc_t));
			ipcSeach->protocol_type = ntohl(ipcSeach->protocol_type);
			ipcSeach->max_return = ntohs(ipcSeach->max_return);
			
			pSearchIpc->para_log = sIpcInfo1;
			
			if(NULL != pSearchIpc->para_log)
			{
				(*pCB)(&sReqIns, &sRslt);//回调
				
				if(pSearchIpc->Desc.sum > 0)
				{
					second = (pSearchIpc->Desc.sum - 1) / ipcSeach->max_return;
					printf("yg second: %d\n", second);
				}
				max_return = ipcSeach->max_return;
				for(j=0;j < 1;j++)
				{
					printf(" pSearchIpc->Desc.sum = %d ,ipcSeach->max_return = %d ,second = %d \n",pSearchIpc->Desc.sum,ipcSeach->max_return,second);
				
					if(pSearchIpc->Desc.startID > pSearchIpc->Desc.sum)
					{
						pSearchIpc->Desc.sum = htons(pSearchIpc->Desc.sum);
						pSearchIpc->Desc.startID = htons(0);
						pSearchIpc->Desc.endID = htons(0);
						memcpy(pbyAckBuf,&pSearchIpc->Desc,sizeof(pSearchIpc->Desc));
						*pAckLen = sizeof(pSearchIpc->Desc);
						
						return CTRL_SUCCESS;
					}
					int sum = 0,num = 0;
					
					printf("pSearchIpc->Desc.startID = %d \n", pSearchIpc->Desc.startID);

					if(pSearchIpc->Desc.startID<0) return -1;

					if(j == second)
					{
						num = pSearchIpc->Desc.sum - second * ipcSeach->max_return;
					}
					else
					{
						num = ipcSeach->max_return;
					}

					for(i=pSearchIpc->Desc.startID; i< pSearchIpc->Desc.startID + num; ++i)//(pSearchIpc->Desc.sum+1)
					{
						if(i>=pSearchIpc->Desc.startID+ipcSeach->max_return) break;
						
						ifly_ipc_info_t* pIpcInfo = &sIpcInfo1[i];//pSearchIpc->para_log
						
						memcpy(	pbyAckBuf + sizeof(ifly_search_desc_t) + (i-pSearchIpc->Desc.startID)*sizeof(ifly_ipc_info_t), 
							pIpcInfo,
							sizeof(ifly_ipc_info_t)
							);

						struct in_addr host;
						host.s_addr = pIpcInfo->dwIp;
						printf(" %d:%s \n",sum,inet_ntoa(host));

						sum++;
					}
					*pAckLen = sum*sizeof(ifly_ipc_info_t)+sizeof(ifly_search_desc_t);
					
					pSearchIpc->Desc.endID = htons(sum-1+pSearchIpc->Desc.startID);
					endid = sum-1+pSearchIpc->Desc.startID;
					pSearchIpc->Desc.sum = htons(pSearchIpc->Desc.sum);
					pSearchIpc->Desc.startID = htons(pSearchIpc->Desc.startID);

					printf("\t startid: %d\n", ntohs(pSearchIpc->Desc.startID));
					printf("\t endid: %d\n", ntohs(pSearchIpc->Desc.endID));
					printf("\t ipc_num: %d\n", ntohs(pSearchIpc->Desc.sum));
					
					memcpy(pbyAckBuf, &pSearchIpc->Desc, sizeof(ifly_search_desc_t));
					pSearchIpc->Desc.startID = sum ;
					startid = sum;
					pSearchIpc->Desc.sum = ntohs(pSearchIpc->Desc.sum);
					ipc_num = pSearchIpc->Desc.sum;
					
				}
			}
			printf(" CTRL_CMD_GETSEACHIPCLIST CTRL_CMD_GETSEACHIPCLIST is end pAckLen = %d\n",*pAckLen);
		}
		break;
	case CTRL_CMD_GETTHEOTHER:
		{
		static int j = 0;//上传次数
		ifly_search_desc_t *Desc = (ifly_search_desc_t *)malloc(sizeof(ifly_search_desc_t));
		
		printf("CTRL_CMD_GETTHEOTHER second = %d \n",second);
		if(second <1) break;

		//for(j=1;j < second +1;j++)
		{
			
			int sum = 0,num = 0;
			
			//printf("1 startid = %d \n", startid);

			if(startid<0) return -1;

			if(j == second-1)
			{
				num = ipc_num - second * max_return;
			}
			else
			{
				num = max_return;
			}
			//printf(" num = %d \n",num);
			for(i=startid; i< startid + num; ++i)//(pSearchIpc->Desc.sum+1)
			{
				if(i >= startid + max_return) break;
				ifly_ipc_info_t* pIpcInfo = &sIpcInfo1[i];
				memcpy(	pbyAckBuf + sizeof(ifly_search_desc_t) + (i-startid)*sizeof(ifly_ipc_info_t), 
					pIpcInfo,
					sizeof(ifly_ipc_info_t)
					);
				struct in_addr host;
				host.s_addr = pIpcInfo->dwIp;
				printf(" %d:%s \n",sum,inet_ntoa(host));

				sum++;
			}
			
			
			
			*pAckLen = sum*sizeof(ifly_ipc_info_t)+sizeof(ifly_search_desc_t);
			Desc->endID = htons(sum-1+startid);
			endid = sum-1+startid;
			Desc->sum = htons(ipc_num);
			Desc->startID = htons(startid);

			printf("\t j = %d\n", j);
			printf("\t startid: %d\n", ntohs(Desc->startID));
			printf("\t endid: %d\n", ntohs(Desc->endID));
			printf("\t ipc_num: %d\n", ntohs(Desc->sum));
			
			memcpy(pbyAckBuf, Desc, sizeof(ifly_search_desc_t));
			free(Desc);
			startid = startid + sum;
			
			j++;
			if (j >= second)
				j = 0;
			
		}
		printf(" CTRL_CMD_GETTHEOTHER \n");
		}
		break;
	case CTRL_CMD_GETADDIPCLIST:
		{printf(" CTRL_CMD_GETADDIPCLIST CTRL_CMD_GETADDIPCLIST is start\n");
			ifly_search_ipc_t* ipcSeach;
			SCPIpcInfo	  *pSearchIpc;
			int j;
			
			pSearchIpc = &sRslt.sBasicInfo.sIpcInfo;
			ipcSeach = &sReqIns.sReq.search_ipc;
			memcpy(ipcSeach, pbyMsgBuf, sizeof(ifly_search_ipc_t));
			ipcSeach->max_return = ntohs(ipcSeach->max_return);
			
			//ifly_ipc_info_t sIpcInfo1[0xff];
			pSearchIpc->para_log = sIpcInfo1;
			
			if(NULL != pSearchIpc->para_log)
			{
				(*pCB)(&sReqIns, &sRslt);//执行回调
				
				if(pSearchIpc->Desc.sum > 0)
				{
					second = (pSearchIpc->Desc.sum - 1) / ipcSeach->max_return;
					printf("yg second: %d\n", second);
				}
				max_return = ipcSeach->max_return;
				for(j=0;j < 1;j++)
				{
					printf(" pSearchIpc->Desc.sum = %d ,ipcSeach->max_return = %d ,second = %d \n",pSearchIpc->Desc.sum,ipcSeach->max_return,second);
				
					if(pSearchIpc->Desc.startID > pSearchIpc->Desc.sum)
					{
						pSearchIpc->Desc.sum = htons(pSearchIpc->Desc.sum);
						pSearchIpc->Desc.startID = htons(0);
						pSearchIpc->Desc.endID = htons(0);
						memcpy(pbyAckBuf,&pSearchIpc->Desc,sizeof(pSearchIpc->Desc));
						*pAckLen = sizeof(pSearchIpc->Desc);
						
						return CTRL_SUCCESS;
					}
					int sum = 0,num = 0;
					
					printf("pSearchIpc->Desc.startID = %d \n", pSearchIpc->Desc.startID);

					if(pSearchIpc->Desc.startID<0) return -1;

					if(j == second)
					{
						num = pSearchIpc->Desc.sum - second * ipcSeach->max_return;
					}
					else
					{
						num = ipcSeach->max_return;
					}

					for(i=pSearchIpc->Desc.startID; i< pSearchIpc->Desc.startID + num; ++i)//(pSearchIpc->Desc.sum+1)
					{
						if(i>=pSearchIpc->Desc.startID+ipcSeach->max_return) break;
						
						ifly_ipc_info_t* pIpcInfo = &sIpcInfo1[i];//pSearchIpc->para_log
						
						memcpy(	pbyAckBuf + sizeof(ifly_search_desc_t) + (i-pSearchIpc->Desc.startID)*sizeof(ifly_ipc_info_t), 
							pIpcInfo,
							sizeof(ifly_ipc_info_t)
							);

						struct in_addr host;
						host.s_addr = pIpcInfo->dwIp;
						printf(" %d:%s \n",sum,inet_ntoa(host));

						sum++;
					}
					*pAckLen = sum*sizeof(ifly_ipc_info_t)+sizeof(ifly_search_desc_t);
					
					pSearchIpc->Desc.endID = htons(sum-1+pSearchIpc->Desc.startID);
					endid = sum-1+pSearchIpc->Desc.startID;
					pSearchIpc->Desc.sum = htons(pSearchIpc->Desc.sum);
					pSearchIpc->Desc.startID = htons(pSearchIpc->Desc.startID);

					printf("\t startid: %d\n", ntohs(pSearchIpc->Desc.startID));
					printf("\t endid: %d\n", ntohs(pSearchIpc->Desc.endID));
					printf("\t ipc_num: %d\n", ntohs(pSearchIpc->Desc.sum));
					
					memcpy(pbyAckBuf, &pSearchIpc->Desc, sizeof(ifly_search_desc_t));
					pSearchIpc->Desc.startID = sum ;
					startid = sum ;
					pSearchIpc->Desc.sum = ntohs(pSearchIpc->Desc.sum);
					ipc_num = pSearchIpc->Desc.sum;
					
					
				}
				
			}
			printf(" CTRL_CMD_GETSEACHIPCLIST CTRL_CMD_GETSEACHIPCLIST is end pAckLen = %d\n",*pAckLen);
			
		}
		break;
	case CTRL_CMD_SETIPC:
		{printf(" CTRL_CMD_SETIPC CTRL_CMD_SETIPC is start \n");
			ifly_ipc_info_t*  ipc_info;
			ipc_info = &sReqIns.sReq.ipc_info;
			
			memset(ipc_info, 0, sizeof(ifly_ipc_info_t));
			memcpy(ipc_info, pbyMsgBuf ,sizeof(ifly_ipc_info_t));

			//ipc_info->protocol_type = htonl(ipc_info->protocol_type);
			//ipc_info->wPort = htons(ipc_info->wPort);
			//ipc_info->net_mask = htonl(ipc_info->net_mask);
			//ipc_info->net_gateway = htonl(ipc_info->net_gateway);
			//ipc_info->dns1 = htonl(ipc_info->dns1);
			//ipc_info->dns2 = htonl(ipc_info->dns2);
			
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			printf(" CTRL_CMD_SETIPC CTRL_CMD_SETIPC is end \n");
		}
		break;
	case CTRL_CMD_ADDIPC:
		{printf(" CTRL_CMD_ADDIPC CTRL_CMD_ADDIPC is start \n");
			ifly_ipc_info_t*  ipc_info;
			ipc_info = &sReqIns.sReq.ipc_info;
			
			memset(ipc_info, 0, sizeof(ifly_ipc_info_t));
			memcpy(ipc_info, pbyMsgBuf ,sizeof(ifly_ipc_info_t));
			
			ipc_info->protocol_type = ntohl(ipc_info->protocol_type);
			ipc_info->dwIp = ntohl(ipc_info->dwIp);
			ipc_info->wPort = ntohs(ipc_info->wPort);
			ipc_info->net_mask = ntohl(ipc_info->net_mask);
			ipc_info->net_gateway = ntohl(ipc_info->net_gateway);
			ipc_info->dns1 = ntohl(ipc_info->dns1);
			ipc_info->dns2 = ntohl(ipc_info->dns2);

			struct in_addr in;
			in.s_addr = ipc_info->dwIp;
			printf("yg NVR addipc dwIP= %s \n", inet_ntoa(in));
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			printf(" CTRL_CMD_ADDIPC CTRL_CMD_ADDIPC is end \n");
		}
		break;
	case CTRL_CMD_DELETEIPC:
		{printf(" CTRL_CMD_DELETEIPC CTRL_CMD_DELETEIPC is start \n");
			ifly_ipc_info_t*  ipc_info;
			ipc_info = &sReqIns.sReq.ipc_info;
			
			memset(ipc_info, 0, sizeof(ifly_ipc_info_t));
			memcpy(ipc_info, pbyMsgBuf ,sizeof(ifly_ipc_info_t));
			
			ipc_info->protocol_type = ntohl(ipc_info->protocol_type);
			ipc_info->dwIp = ntohl(ipc_info->dwIp);
			ipc_info->wPort = ntohs(ipc_info->wPort);
			ipc_info->net_mask = ntohl(ipc_info->net_mask);
			ipc_info->net_gateway = ntohl(ipc_info->net_gateway);
			ipc_info->dns1 = ntohl(ipc_info->dns1);
			ipc_info->dns2 = ntohl(ipc_info->dns2);

			struct in_addr in;
			in.s_addr = ipc_info->dwIp;
			printf("yg NVR del dwIP= %s \n", inet_ntoa(in));
			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			printf(" CTRL_CMD_DELETEIPC CTRL_CMD_DELETEIPC is end \n");
		}
		break;
	case CTRL_CMD_GET_PATROL_PARA:
		{
			printf(" CTRL_CMD_GET_PATROL_PARA is start \n");
			ifly_patrol_para_t *patrol_para = &sRslt.sBasicInfo.patrol_para;
			
			(*pCB)(&sReqIns, &sRslt);
			
			memcpy(pbyAckBuf, patrol_para, sizeof(ifly_patrol_para_t));
			printf("patrol_para->nIsPatrol: %d\n", patrol_para->nIsPatrol);
			printf("patrol_para->nInterval: %d\n", patrol_para->nInterval);
			printf("patrol_para->nPatrolMode: %d\n", patrol_para->nPatrolMode);

			patrol_para = (ifly_patrol_para_t *)pbyAckBuf;//再次赋值

			int nDWellTime, i;
			SValue dWellTimeList[10];
			GetDWellSwitchTimeList(dWellTimeList, &nDWellTime, 10);

			patrol_para->nInterval_num = nDWellTime;

			printf("yg nDWellTime: %u\n", patrol_para->nInterval_num);
			for (i=0; i<nDWellTime; i++)
			{
				patrol_para->value[i] = dWellTimeList[i].nValue;
				printf("\t patrol_para->value[i]: %u\n", patrol_para->value[i]);
			}

			SValue dWellSwitchPicture[10];//轮巡画面选择: 单画面，四画面，九画面
			int nPicturesReal;
			GetDWellSwitchPictureList(dWellSwitchPicture, &nPicturesReal, 10);

			patrol_para->nPatrolMode_num = nPicturesReal;
			printf("yg nPicturesReal: %u\n", patrol_para->nPatrolMode_num);
			for(i=0; i < nPicturesReal; i++)
			{
				BizConfigGetParaListValue(
							EM_BIZ_CFG_PARALIST_SWITCHPICTURE, 
							i, 
							&dWellSwitchPicture[i].nValue);
				
				patrol_para->value[i+nDWellTime] = dWellSwitchPicture[i].nValue;
				printf("yg DWellSwitchPicture[%d]: %d\n", i, dWellSwitchPicture[i].nValue);
				//printf("yg DWellSwitchPicture[%d]: %s\n", i, dWellSwitchPicture[i].strDisplay);
			}
				
			*pAckLen = sizeof(ifly_patrol_para_t) + patrol_para->nInterval_num \
				+ patrol_para->nPatrolMode_num - 1;//patrol_para->value 数组有一个字节
			printf("CTRL_CMD_GET_PATROL_PARA is end\n");
	
		}break;
	case CTRL_CMD_SET_PATROL_PARA:
		{
			printf(" CTRL_CMD_SET_PATROL_PARA is start \n");
			ifly_patrol_para_t*  patrol_para = &sReqIns.sReq.patrol_para;
			
			memset(patrol_para, 0, sizeof(ifly_patrol_para_t));
			memcpy(patrol_para, pbyMsgBuf ,sizeof(ifly_patrol_para_t));

			(*pCB)(&sReqIns, &sRslt);
			
			if(pAckLen) *pAckLen = 0;
			
			printf(" CTRL_CMD_SET_PATROL_PARA is end \n");
		}break;
	case CTRL_CMD_GET_PREVIEW_PARA:
		{
			printf(" CTRL_CMD_GET_PREVIEW_PARA is start \n");
			ifly_preview_para_t *preview_para = &sRslt.sBasicInfo.preview_para;
			
			(*pCB)(&sReqIns, &sRslt);
			
			memcpy(pbyAckBuf, preview_para, sizeof(ifly_preview_para_t));
			*pAckLen = sizeof(ifly_preview_para_t) ;//patrol_para->value 数组有一个字节
			printf("CTRL_CMD_GET_PREVIEW_PARA is end\n");
		}break;
	case CTRL_CMD_SET_PREVIEW_PARA:
		{
			printf(" CTRL_CMD_SET_PREVIEW_PARA is start \n");
			ifly_preview_para_t*  preview_para = &sReqIns.sReq.preview_para;
			
			memset(preview_para, 0, sizeof(ifly_preview_para_t));
			memcpy(preview_para, pbyMsgBuf ,sizeof(ifly_preview_para_t));

			(*pCB)(&sReqIns, &sRslt);
			printf("yg CTRL_CMD_SET_PREVIEW_PARA pAckLen %s NULL\n", pAckLen? "!=":"==");
			if(pAckLen) *pAckLen = 0;
			
			printf(" CTRL_CMD_SET_PREVIEW_PARA is end \n");
		}break;
	case CTRL_CMD_CLEAN_ALARM_ICON:
		{
			printf(" CTRL_CMD_CLEAN_ALARM_ICON is start \n");
			ifly_preview_para_t *preview_para = &sRslt.sBasicInfo.preview_para;
			
			(*pCB)(&sReqIns, &sRslt);

			printf("yg CTRL_CMD_CLEAN_ALARM_ICON pAckLen %s NULL\n", pAckLen? "!=":"==");
			if(pAckLen) *pAckLen = 0;
			printf("CTRL_CMD_CLEAN_ALARM_ICON is end\n");
		}break;
	case CTRL_CMD_CLOSE_GUIDE:
		{
			printf(" CTRL_CMD_CLOSE_GUIDE is start \n");
			ifly_preview_para_t *preview_para = &sRslt.sBasicInfo.preview_para;
			
			(*pCB)(&sReqIns, &sRslt);

			//printf("yg CTRL_CMD_CLOSE_GUIDE pAckLen %s NULL\n", pAckLen? "!=":"==");
			if(pAckLen) *pAckLen = 0;
			//printf("CTRL_CMD_CLOSE_GUIDE is end\n");
		}break;
	case CTRL_CMD_GET_IPCCHN_LINKSTATUS:
		{
			//printf(" CTRL_CMD_GET_IPCCHN_LINKSTATUS is start \n");
			ifly_ipc_chn_status_t *ipc_chn_status = &sRslt.sBasicInfo.ipc_chn_status;
			
			(*pCB)(&sReqIns, &sRslt);
			
			memcpy(pbyAckBuf, ipc_chn_status, sizeof(ifly_ipc_chn_status_t));
			*pAckLen = sizeof(ifly_ipc_chn_status_t);
			//printf("CTRL_CMD_GET_IPCCHN_LINKSTATUS is end\n");
		}break;
	//xdc end 
	}
	return CTRL_SUCCESS;
}

#else

u16 DealCommand(
	CPHandle cph,
	u16 event,
	u8 *pbyMsgBuf,
	int msgLen,
	u8 *pbyAckBuf,
	int *pAckLen,
	void* pContext
)
{
	PNetCommCommandDeal pCB;
	SRemoteCmdReq sReqIns;
	
	NETCOMM_DEBUG_STR("get cmd111 ", event);

	if(event==CTRL_CMD_PLAYPROGRESS)
	{
		return DealPlayProgress(
					cph,
					event,
					pbyMsgBuf,
					msgLen,
					pbyAckBuf,
					pAckLen,
					pContext);
	}
	
	sReqIns.cph = cph;
	if( NULL!=pbyMsgBuf&&msgLen>0 )
		memcpy(&sReqIns.sReq, pbyMsgBuf, msgLen);

	if(CTRL_CMD_RECFILESEARCH==event)
	{
		NETCOMM_DEBUG_STR("get cmd CTRL_CMD_RECFILESEARCH", event);
		if(pCB2)
			return (*pCB2)(&sReqIns,pbyAckBuf,pAckLen);
	}
	else
	{
#if 1
		NETCOMM_DEBUG_STR("get cmd ", event);
		
		pCB = cmdProc_GetCmdCB(event);
		if(pCB)
		{

			// set or get paramters to/from uplayer
			//			
			SRemoteCmdReq sReqIns;
			SRemoteCmdRslt sRslt;
			
			memset(&sReqIns, 0, sizeof(sReqIns));
			memset(&sRslt, 0, sizeof(sRslt));
			
			sReqIns.cph = cph;

			if( NULL!=pbyMsgBuf&&msgLen>0 )
				memcpy(&sReqIns.sReq, pbyMsgBuf, msgLen);

			(*pCB)(&sReqIns,&sRslt);
			
			//printf("result basic len %d\n", sRslt.nBasicInfoLen);
			//printf("result basic %x %x\n", sRslt.sBasicInfo);
			
			// fill ack buff
			if(sRslt.nErrCode==CTRL_SUCCESS)
			{
				// fill basic info
				if(sRslt.nBasicInfoLen>0)
				{
					if(NULL!=pbyAckBuf)
					{
						memcpy(pbyAckBuf, &sRslt.sBasicInfo, sRslt.nBasicInfoLen);
						*pAckLen = sRslt.nBasicInfoLen;
						
						// fill ext
						if(sRslt.bUseExt)
						{
							if(sRslt.nExtInfoLen)
							{
								memcpy(pbyAckBuf+sRslt.nBasicInfoLen, &sRslt.sExtInfo, sRslt.nExtInfoLen);
								*pAckLen += sRslt.nExtInfoLen;
							}
						}
					}
				}
			}
			else
			{
				return sRslt.nErrCode;
			}
		}

		return CTRL_SUCCESS;
#endif
	}
}

#endif

